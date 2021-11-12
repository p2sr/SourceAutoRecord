#include "Renderer.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Hook.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Utils/SDK.hpp"

#include <atomic>
#include <cinttypes>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <string>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
};

// Stuff pulled from the engine
static void **g_videomode;
static int *g_snd_linear_count;
static int **g_snd_p;
static int *g_snd_vol;
static MovieInfo_t *g_movieInfo;

// The demoplayer tick this segment ends on (we'll stop recording at
// this tick if sar_render_autostop is set)
int Renderer::segmentEndTick = -1;

// Whether a demo is loading; used to detect whether we should autostart
bool Renderer::isDemoLoading = false;

// We reference this so we can make sure the output is stereo.
// Supporting other output options is quite tricky; because Source
// movies only support stereo output, the functions for outputting
// surround audio don't have the necessary code for writing it to
// buffers like we need. This is *possible* to get around, but would
// require some horrible mid-function detours.
static Variable snd_surround_speakers;

// h264, hevc, vp8, vp9, dnxhd
// aac, ac3, vorbis, opus, flac
static Variable sar_render_vbitrate("sar_render_vbitrate", "40000", 1, "Video bitrate used in renders (kbit/s)\n");
static Variable sar_render_abitrate("sar_render_abitrate", "160", 1, "Audio bitrate used in renders (kbit/s)\n");
static Variable sar_render_vcodec("sar_render_vcodec", "h264", "Video codec used in renders (h264, hevc, vp8, vp9, dnxhd)\n", 0);
static Variable sar_render_acodec("sar_render_acodec", "aac", "Audio codec used in renders (aac, ac3, vorbis, opus, flac)\n", 0);
static Variable sar_render_quality("sar_render_quality", "35", 0, 50, "Render output quality, higher is better (50=lossless)\n");
static Variable sar_render_fps("sar_render_fps", "60", 1, "Render output FPS\n");
static Variable sar_render_sample_rate("sar_render_sample_rate", "44100", 1000, "Audio output sample rate\n");
static Variable sar_render_blend("sar_render_blend", "0", 0, "How many frames to blend for each output frame; 1 = do not blend, 0 = automatically determine based on host_framerate\n");
static Variable sar_render_autostart("sar_render_autostart", "0", "Whether to automatically start when demo playback begins\n");
static Variable sar_render_autostart_extension("sar_render_autostart_extension", "mp4", "The file extension (and hence container format) to use for automatically started renders.\n", 0);
static Variable sar_render_autostop("sar_render_autostop", "1", "Whether to automatically stop when __END__ is seen in demo playback\n");
static Variable sar_render_shutter_angle("sar_render_shutter_angle", "180", 30, 360, "The shutter angle to use for rendering in degrees.\n");
static Variable sar_render_merge("sar_render_merge", "0", "When set, merge all the renders until sar_render_finish is entered\n");

// g_videomode VMT wrappers {{{

static inline int GetScreenWidth() {
	return Memory::VMT<int(__rescall *)(void *)>(*g_videomode, Offsets::GetModeWidth)(*g_videomode);
}

static inline int GetScreenHeight() {
	return Memory::VMT<int(__rescall *)(void *)>(*g_videomode, Offsets::GetModeHeight)(*g_videomode);
}

static inline void ReadScreenPixels(int x, int y, int w, int h, void *buf, ImageFormat fmt) {
	return Memory::VMT<void(__rescall *)(void *, int, int, int, int, void *, ImageFormat)>(*g_videomode, Offsets::ReadScreenPixels)(*g_videomode, x, y, w, h, buf, fmt);
}

// }}}

struct Stream {
	AVStream *stream;
	AVCodecContext *enc;
	AVCodec *codec;
	AVFrame *frame, *tmpFrame;
	SwsContext *swsCtx;
	SwrContext *swrCtx;
	int nextPts;
};

enum class WorkerMsg {
	NONE,
	VIDEO_FRAME_READY,
	AUDIO_FRAME_READY,
	STOP_RENDERING_ERROR,
	STOP_RENDERING_REQUESTED,
};

// The global renderer state
static struct
{
	std::atomic<bool> isRendering;
	std::atomic<bool> isPaused;
	int fps;
	int samplerate;
	int channels;
	Stream videoStream;
	Stream audioStream;
	std::string filename;
	AVFormatContext *outCtx;
	int width, height;

	// The audio stream's temporary frame needs nb_samples worth of
	// audio before we resample and submit, so we keep track of how far
	// in we are with this
	int16_t *audioBuf[8];  // Temporary buffer - contains the planar audio info from the game
	size_t audioBufSz;
	size_t audioBufIdx;

	// This buffer stores the image data
	uint8_t *imageBuf;  // Temporary buffer - contains the raw pixel data read from the screen

	int toBlend;
	int toBlendStart;       // Inclusive
	int toBlendEnd;         // Exclusive
	int nextBlendIdx;       // How many frames in this blend have we seen so far?
	uint16_t *blendSumBuf;  // Blending buffer - contains the sum of the pixel values during blending (we only divide at the end of the blend to prevent rounding errors). Not allocated if toBlend == 1.

	// Synchronisation
	std::thread worker;
	std::mutex workerUpdateLock;
	std::condition_variable workerUpdate;
	std::atomic<WorkerMsg> workerMsg;
	std::mutex imageBufLock;
	std::mutex audioBufLock;
	std::atomic<bool> workerFailedToStart;
} g_render;

static inline void msgStopRender(bool error) {
	std::lock_guard<std::mutex> lock(g_render.workerUpdateLock);
	g_render.workerMsg.store(error ? WorkerMsg::STOP_RENDERING_ERROR : WorkerMsg::STOP_RENDERING_REQUESTED);
	g_render.workerUpdate.notify_all();
}

// Utilities {{{

static AVCodecID videoCodecFromName(const char *name) {
	if (!strcmp(name, "h264")) return AV_CODEC_ID_H264;
	if (!strcmp(name, "hevc")) return AV_CODEC_ID_HEVC;
	if (!strcmp(name, "vp8")) return AV_CODEC_ID_VP8;
	if (!strcmp(name, "vp9")) return AV_CODEC_ID_VP9;
	if (!strcmp(name, "dnxhd")) return AV_CODEC_ID_DNXHD;
	return AV_CODEC_ID_NONE;
}

static AVCodecID audioCodecFromName(const char *name) {
	if (!strcmp(name, "aac")) return AV_CODEC_ID_AAC;
	if (!strcmp(name, "ac3")) return AV_CODEC_ID_AC3;
	if (!strcmp(name, "vorbis")) return AV_CODEC_ID_VORBIS;
	if (!strcmp(name, "opus")) return AV_CODEC_ID_OPUS;
	if (!strcmp(name, "flac")) return AV_CODEC_ID_FLAC;
	return AV_CODEC_ID_NONE;
}

// }}}

// Movie command hooks {{{

// We want to stop use of the normal movie system while a SAR render is
// active. This is because we exploit g_movieInfo to make the game think
// it's recording a movie, so if both were running at the same time,
// they'd interact in weird and undefined ways.

static _CommandCallback startmovie_origCbk;
static _CommandCallback endmovie_origCbk;
static void startmovie_cbk(const CCommand &args) {
	if (g_render.isRendering.load()) {
		console->Print("Cannot startmovie while a SAR render is active! Stop the render with sar_render_finish.\n");
		return;
	}
	startmovie_origCbk(args);
}
static void endmovie_cbk(const CCommand &args) {
	if (g_render.isRendering.load()) {
		console->Print("Cannot endmovie while a SAR render is active! Did you mean sar_render_finish?\n");
		return;
	}
	endmovie_origCbk(args);
}

// }}}

// AVFrame allocators {{{

static AVFrame *allocPicture(AVPixelFormat pixFmt, int width, int height) {
	AVFrame *picture = av_frame_alloc();
	if (!picture) {
		return NULL;
	}

	picture->format = pixFmt;
	picture->width = width;
	picture->height = height;

	if (av_frame_get_buffer(picture, 32) < 0) {
		console->Print("Failed to allocate frame data\n");
		av_frame_free(&picture);
		return NULL;
	}

	return picture;
}

static AVFrame *allocAudioFrame(AVSampleFormat sampleFmt, uint64_t channelLayout, int sampleRate, int nbSamples) {
	AVFrame *frame = av_frame_alloc();
	if (!frame) {
		return NULL;
	}

	frame->format = sampleFmt;
	frame->channel_layout = channelLayout;
	frame->sample_rate = sampleRate;
	frame->nb_samples = nbSamples;

	if (nbSamples) {
		if (av_frame_get_buffer(frame, 0) < 0) {
			return NULL;
		}
	}

	return frame;
}

// }}}

// Stream creation and destruction {{{

// framerate is sample rate for audio streams
static bool addStream(Stream *out, AVFormatContext *outputCtx, AVCodecID codecId, int64_t bitrate, int framerate, int ptsOff, int width = 0, int height = 0) {
	out->codec = avcodec_find_encoder(codecId);
	if (!out->codec) {
		console->Print("Failed to find encoder for '%s'!\n", avcodec_get_name(codecId));
		return false;
	}

	// dnxhd bitrate selection {{{

	if (codecId == AV_CODEC_ID_DNXHD) {
		// dnxhd is fussy and won't just allow any bitrate or
		// resolution, so check our resolution is supported and find the
		// closest bitrate to what was requested

		// rates here are in Mbps
		int64_t *rates;
		size_t nrates;

		if (width == 1920 && height == 1080) {  // 1080p 16:9
			static int64_t rates1080[] = {
				36,
				45,
				75,
				90,
				115,
				120,
				145,
				175,
				180,
				190,
				220,
				240,
				365,
				440,
			};
			rates = rates1080;
			nrates = sizeof rates1080 / sizeof rates1080[0];
		} else if (width == 1280 && height == 720) {  // 720p 16:9
			static int64_t rates720[] = {
				60,
				75,
				90,
				110,
				120,
				145,
				180,
				220,
			};
			rates = rates720;
			nrates = sizeof rates720 / sizeof rates720[0];
		} else if (width == 1440 && height == 1080) {  // 1080p 4:3
			static int64_t rates1080[] = {
				63,
				84,
				100,
				110,
			};
			rates = rates1080;
			nrates = sizeof rates1080 / sizeof rates1080[0];
		} else if (width == 960 && height == 720) {  // 720p 4:3
			static int64_t rates720[] = {
				42,
				60,
				75,
				115,
			};
			rates = rates720;
			nrates = sizeof rates720 / sizeof rates720[0];
		} else {
			console->Print("Resolution not supported by dnxhd\n");
			return false;
		}

		int64_t realBitrate = -1;
		int64_t lastDelta = INT64_MAX;

		for (size_t i = 0; i < nrates; ++i) {
			int64_t rate = rates[i] * 1000000;
			int64_t delta = rate > bitrate ? rate - bitrate : bitrate - rate;
			if (delta < lastDelta) {
				realBitrate = rate;
			}
		}

		if (realBitrate != bitrate) {
			console->Print("dnxhd does not support the given bitrate; encoding at %d kb/s instead\n", realBitrate / 1000);
			bitrate = realBitrate;
		}
	}

	// }}}

	out->stream = avformat_new_stream(outputCtx, NULL);
	if (!out->stream) {
		console->Print("Failed to allocate stream\n");
		return false;
	}

	out->enc = avcodec_alloc_context3(out->codec);
	if (!out->enc) {
		console->Print("Failed to allocate an encoding context\n");
		return false;
	}

	out->enc->bit_rate = bitrate;
	out->stream->time_base = {1, framerate};

	switch (out->codec->type) {
	case AVMEDIA_TYPE_AUDIO:
		out->enc->sample_fmt = out->codec->sample_fmts ? out->codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;  // don't really care about sample format
		out->enc->sample_rate = framerate;
		out->enc->channel_layout = AV_CH_LAYOUT_STEREO;
		out->enc->channels = av_get_channel_layout_nb_channels(out->enc->channel_layout);

		if (out->codec->channel_layouts) {
			bool foundStereo = false;
			for (const uint64_t *layout = out->codec->channel_layouts; *layout; ++layout) {
				if (*layout == AV_CH_LAYOUT_STEREO) {
					foundStereo = true;
					break;
				}
			}

			if (!foundStereo) {
				console->Print("Stereo not supported by audio encoder\n");
				avcodec_free_context(&out->enc);
				return false;
			}
		}

		if (out->codec->supported_samplerates) {
			bool foundRate = false;
			for (const int *rate = out->codec->supported_samplerates; *rate; ++rate) {
				if (*rate == framerate) {
					foundRate = true;
					break;
				}
			}

			if (!foundRate) {
				console->Print("Sample rate %d not supported by audio encoder\n", framerate);
				avcodec_free_context(&out->enc);
				return false;
			}
		}

		break;

	case AVMEDIA_TYPE_VIDEO:
		out->enc->codec_id = codecId;
		out->enc->width = width;
		out->enc->height = height;
		out->enc->time_base = out->stream->time_base;
		out->enc->gop_size = 12;
		out->enc->pix_fmt = codecId == AV_CODEC_ID_DNXHD ? AV_PIX_FMT_YUV422P : AV_PIX_FMT_YUV420P;

		break;

	default:
		break;
	}

	if (outputCtx->oformat->flags & AVFMT_GLOBALHEADER) {
		out->enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	out->nextPts = ptsOff;

	return true;
}

static void closeStream(Stream *s) {
	avcodec_free_context(&s->enc);
	av_frame_free(&s->frame);
	av_frame_free(&s->tmpFrame);
	sws_freeContext(s->swsCtx);
	swr_free(&s->swrCtx);
}

// }}}

// Flushing streams {{{

static bool flushStream(Stream *s, bool isEnd = false) {
	if (isEnd) {
		avcodec_send_frame(s->enc, NULL);
	}

	while (true) {
		AVPacket pkt = {0};
		int ret = avcodec_receive_packet(s->enc, &pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return true;
		} else if (ret < 0) {
			return false;
		}
		av_packet_rescale_ts(&pkt, s->enc->time_base, s->stream->time_base);
		pkt.stream_index = s->stream->index;
		av_interleaved_write_frame(g_render.outCtx, &pkt);
		av_packet_unref(&pkt);
	}
}

// }}}

// Opening streams {{{

static bool openVideo(AVFormatContext *outputCtx, Stream *s, AVDictionary **options) {
	if (avcodec_open2(s->enc, s->codec, options) < 0) {
		console->Print("Failed to open video codec\n");
		return false;
	}

	s->frame = allocPicture(s->enc->pix_fmt, s->enc->width, s->enc->height);
	if (!s->frame) {
		console->Print("Failed to allocate video frame\n");
		return false;
	}

	s->tmpFrame = allocPicture(AV_PIX_FMT_BGR24, s->enc->width, s->enc->height);
	if (!s->tmpFrame) {
		console->Print("Failed to allocate intermediate video frame\n");
		return false;
	}

	if (avcodec_parameters_from_context(s->stream->codecpar, s->enc) < 0) {
		console->Print("Failed to copy stream parameters\n");
		return false;
	}

	s->swsCtx = sws_getContext(s->enc->width, s->enc->height, AV_PIX_FMT_BGR24, s->enc->width, s->enc->height, s->enc->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
	if (!s->swsCtx) {
		console->Print("Failed to initialize conversion context\n");
		return false;
	}

	return true;
}

static bool openAudio(AVFormatContext *outputCtx, Stream *s, AVDictionary **options) {
	if (avcodec_open2(s->enc, s->codec, options) < 0) {
		console->Print("Failed to open audio codec\n");
		return false;
	}

	s->frame = allocAudioFrame(s->enc->sample_fmt, s->enc->channel_layout, s->enc->sample_rate, s->enc->frame_size);
	s->tmpFrame = allocAudioFrame(AV_SAMPLE_FMT_S16P, s->enc->channel_layout, s->enc->sample_rate, s->enc->frame_size);

	if (avcodec_parameters_from_context(s->stream->codecpar, s->enc) < 0) {
		console->Print("Failed to copy stream parameters\n");
		return false;
	}

	s->swrCtx = swr_alloc();
	if (!s->swrCtx) {
		console->Print("Failed to allocate resampler context\n");
		return false;
	}

	av_opt_set_int(s->swrCtx, "in_channel_count", s->enc->channels, 0);
	av_opt_set_int(s->swrCtx, "in_sample_rate", 44100, 0);
	av_opt_set_sample_fmt(s->swrCtx, "in_sample_fmt", AV_SAMPLE_FMT_S16P, 0);
	av_opt_set_int(s->swrCtx, "out_channel_count", s->enc->channels, 0);
	av_opt_set_int(s->swrCtx, "out_sample_rate", s->enc->sample_rate, 0);
	av_opt_set_sample_fmt(s->swrCtx, "out_sample_fmt", s->enc->sample_fmt, 0);

	if (swr_init(s->swrCtx) < 0) {
		console->Print("Failed to initialize resampler context\n");
		return false;
	}

	return true;
}

// }}}

// Worker thread {{{

// workerStartRender {{{

static void workerStartRender(AVCodecID videoCodec, AVCodecID audioCodec, int64_t videoBitrate, int64_t audioBitrate, AVDictionary *options) {
	if (avformat_alloc_output_context2(&g_render.outCtx, NULL, NULL, g_render.filename.c_str()) == AVERROR(EINVAL)) {
		console->Print("Failed to deduce output format from file extension - using MP4\n");
		avformat_alloc_output_context2(&g_render.outCtx, NULL, "mp4", g_render.filename.c_str());
	}

	if (!g_render.outCtx) {
		console->Print("Failed to allocate output context\n");
		return;
	}

	if (!addStream(&g_render.videoStream, g_render.outCtx, videoCodec, videoBitrate, g_render.fps, 0, g_render.width, g_render.height)) {
		console->Print("Failed to create video stream\n");
		avformat_free_context(g_render.outCtx);
		return;
	}

	if (!addStream(&g_render.audioStream, g_render.outCtx, audioCodec, audioBitrate, g_render.samplerate, g_render.samplerate / 10)) {  // offset the start by 0.1s because idk
		console->Print("Failed to create audio stream\n");
		closeStream(&g_render.videoStream);
		avformat_free_context(g_render.outCtx);
		return;
	}

	if (!openVideo(g_render.outCtx, &g_render.videoStream, &options)) {
		console->Print("Failed to open video stream\n");
		closeStream(&g_render.audioStream);
		closeStream(&g_render.videoStream);
		avformat_free_context(g_render.outCtx);
		return;
	}

	if (!openAudio(g_render.outCtx, &g_render.audioStream, &options)) {
		console->Print("Failed to open audio stream\n");
		closeStream(&g_render.audioStream);
		closeStream(&g_render.videoStream);
		avformat_free_context(g_render.outCtx);
		return;
	}

	if (avio_open(&g_render.outCtx->pb, g_render.filename.c_str(), AVIO_FLAG_WRITE) < 0) {
		console->Print("Failed to open output file\n");
		closeStream(&g_render.audioStream);
		closeStream(&g_render.videoStream);
		avformat_free_context(g_render.outCtx);
		return;
	}

	if (avformat_write_header(g_render.outCtx, &options) < 0) {
		console->Print("Failed to write output file\n");
		closeStream(&g_render.audioStream);
		closeStream(&g_render.videoStream);
		avio_closep(&g_render.outCtx->pb);
		avformat_free_context(g_render.outCtx);
		return;
	}

	g_render.audioBufIdx = 0;
	g_render.audioBufSz = g_render.audioStream.tmpFrame->nb_samples;

	g_render.channels = g_render.audioStream.enc->channels;

	g_render.imageBuf = (uint8_t *)malloc(3 * g_render.width * g_render.height);
	for (int i = 0; i < g_render.channels; ++i) {
		g_render.audioBuf[i] = (int16_t *)malloc(g_render.audioBufSz * sizeof g_render.audioBuf[i][0]);
	}

	g_render.nextBlendIdx = 0;
	if (g_render.toBlend > 1) {
		g_render.blendSumBuf = (uint16_t *)calloc(3 * g_render.width * g_render.height, sizeof g_render.blendSumBuf[0]);
	}

	g_movieInfo->moviename[0] = 'a';  // Just something nonzero to make the game think there's a movie in progress
	g_movieInfo->moviename[1] = 0;
	g_movieInfo->movieframe = 0;
	g_movieInfo->type = 0;  // Should stop anything actually being output

	g_render.isRendering.store(true);

	console->Print("Started rendering to '%s'\n", g_render.filename.c_str());

	console->Print(
		"    video: %s (%dx%d @ %d fps, %" PRId64 " kb/s, %s)\n",
		g_render.videoStream.codec->name,
		g_render.width,
		g_render.height,
		g_render.fps,
		g_render.videoStream.enc->bit_rate / 1000,
		av_get_pix_fmt_name(g_render.videoStream.enc->pix_fmt));

	console->Print(
		"    audio: %s (%d Hz, %" PRId64 " kb/s, %s)\n",
		g_render.audioStream.codec->name,
		g_render.audioStream.enc->sample_rate,
		g_render.audioStream.enc->bit_rate / 1000,
		av_get_sample_fmt_name(g_render.audioStream.enc->sample_fmt));
}

// }}}

// workerFinishRender {{{

static void workerFinishRender(bool error) {
	if (error) {
		console->Print("A fatal error occurred; stopping render early\n");
	} else {
		console->Print("Stopping render...\n");
	}

	g_render.isRendering.store(false);

	flushStream(&g_render.videoStream, true);
	flushStream(&g_render.audioStream, true);

	av_write_trailer(g_render.outCtx);

	console->Print("Rendered %d frames to '%s'\n", g_render.videoStream.nextPts, g_render.filename.c_str());

	g_render.imageBufLock.lock();
	g_render.audioBufLock.lock();

	closeStream(&g_render.videoStream);
	closeStream(&g_render.audioStream);
	avio_closep(&g_render.outCtx->pb);
	avformat_free_context(g_render.outCtx);
	free(g_render.imageBuf);
	for (int i = 0; i < g_render.channels; ++i) {
		free(g_render.audioBuf[i]);
	}
	if (g_render.toBlend > 1) {
		free(g_render.blendSumBuf);
	}

	g_render.imageBufLock.unlock();
	g_render.audioBufLock.unlock();

	// Reset all the Source movieinfo struct to its default values
	g_movieInfo->moviename[0] = 0;
	g_movieInfo->movieframe = 0;
	g_movieInfo->type = MovieInfo_t::FMOVIE_TGA | MovieInfo_t::FMOVIE_WAV;
	g_movieInfo->jpeg_quality = 50;
}

// }}}

// workerHandleVideoFrame {{{

static bool workerHandleVideoFrame() {
	g_render.imageBufLock.lock();
	g_render.workerMsg.store(WorkerMsg::NONE);  // It's important that we do this *after* locking the image buffer
	size_t size = g_render.width * g_render.height * 3;
	if (g_render.toBlend == 1) {
		// We can just copy the data directly
		memcpy(g_render.videoStream.tmpFrame->data[0], g_render.imageBuf, size);
		g_render.imageBufLock.unlock();
	} else {
		if (g_render.nextBlendIdx >= g_render.toBlendStart && g_render.nextBlendIdx < g_render.toBlendEnd) {
			for (size_t i = 0; i < size; ++i) {
				g_render.blendSumBuf[i] += g_render.imageBuf[i];
			}
		}
		g_render.imageBufLock.unlock();

		if (++g_render.nextBlendIdx != g_render.toBlend) {
			// We've added in this frame, but not done blending yet
			return true;
		}

		int shift = g_render.toBlendEnd - g_render.toBlendStart;

		for (size_t i = 0; i < size; ++i) {
			g_render.videoStream.tmpFrame->data[0][i] = g_render.blendSumBuf[i] / shift;
			g_render.blendSumBuf[i] = 0;
		}

		g_render.nextBlendIdx = 0;
	}

	// tmpFrame is now our final frame; convert to the output format and
	// process it

	sws_scale(g_render.videoStream.swsCtx, (const uint8_t *const *)g_render.videoStream.tmpFrame->data, g_render.videoStream.tmpFrame->linesize, 0, g_render.height, g_render.videoStream.frame->data, g_render.videoStream.frame->linesize);

	g_render.videoStream.frame->pts = g_render.videoStream.nextPts;

	if (avcodec_send_frame(g_render.videoStream.enc, g_render.videoStream.frame) < 0) {
		console->Print("Failed to send video frame for encoding!\n");
		return false;
	}

	if (!flushStream(&g_render.videoStream)) {
		console->Print("Failed to flush video stream!\n");
		return false;
	}

	++g_render.videoStream.nextPts;

	return true;
}

// }}}

// workerHandleAudioFrame {{{

static bool workerHandleAudioFrame() {
	g_render.audioBufLock.lock();
	g_render.workerMsg.store(WorkerMsg::NONE);  // It's important that we do this *after* locking the audio buffer

	Stream *s = &g_render.audioStream;
	for (int i = 0; i < g_render.channels; ++i) {
		memcpy(s->tmpFrame->data[i], g_render.audioBuf[i], g_render.audioBufSz * sizeof g_render.audioBuf[i][0]);
	}
	g_render.audioBufLock.unlock();


	s->tmpFrame->pts = s->nextPts;
	s->nextPts += s->frame->nb_samples;

	if (av_frame_make_writable(s->frame) < 0) {
		console->Print("Failed to make audio frame writable!\n");
		return false;
	}

	if (swr_convert(s->swrCtx, s->frame->data, s->frame->nb_samples, (const uint8_t **)s->tmpFrame->data, s->tmpFrame->nb_samples) < 0) {
		console->Print("Failed to resample audio frame!\n");
		return false;
	}

	s->frame->pts = av_rescale_q(s->tmpFrame->pts, {1, s->enc->sample_rate}, s->enc->time_base);

	if (avcodec_send_frame(s->enc, s->frame) < 0) {
		console->Print("Failed to send audio frame for encoding!\n");
		return false;
	}

	if (!flushStream(s)) {
		console->Print("Failed to flush audio stream!\n");
		return false;
	}

	return true;
}

// }}}

static void worker(AVCodecID videoCodec, AVCodecID audioCodec, int64_t videoBitrate, int64_t audioBitrate, AVDictionary *options) {
	workerStartRender(videoCodec, audioCodec, videoBitrate, audioBitrate, options);
	if (!g_render.isRendering.load()) {
		g_render.workerFailedToStart.store(true);
		return;
	}
	std::unique_lock<std::mutex> lock(g_render.workerUpdateLock);
	while (true) {
		g_render.workerUpdate.wait(lock);
		switch (g_render.workerMsg.load()) {
		case WorkerMsg::VIDEO_FRAME_READY:
			if (!workerHandleVideoFrame()) {
				workerFinishRender(true);
				return;
			}
			break;
		case WorkerMsg::AUDIO_FRAME_READY:
			if (!workerHandleAudioFrame()) {
				workerFinishRender(true);
				return;
			}
			break;
		case WorkerMsg::STOP_RENDERING_ERROR:
			workerFinishRender(true);
			return;
		case WorkerMsg::STOP_RENDERING_REQUESTED:
			workerFinishRender(false);
			return;
		case WorkerMsg::NONE:
			break;
		}
	}
}

// }}}

// startRender {{{

static void startRender() {
	// We can't start rendering if we haven't stopped yet, so make sure
	// the worker thread isn't running
	if (g_render.worker.joinable()) {
		g_render.worker.join();
	}

	AVDictionary *options = NULL;

	if (g_render.isRendering.load()) {
		console->Print("Already rendering\n");
		return;
	}

	if (!sv_cheats.GetBool()) {
		console->Print("sv_cheats must be enabled\n");
		return;
	}

	g_render.samplerate = sar_render_sample_rate.GetInt();
	g_render.fps = sar_render_fps.GetInt();

	if (sar_render_blend.GetInt() == 0 && host_framerate.GetInt() == 0) {
		console->Print("host_framerate or sar_render_blend must be nonzero\n");
		return;
	} else if (sar_render_blend.GetInt() != 0) {
		g_render.toBlend = sar_render_blend.GetInt();
		int framerate = g_render.toBlend * g_render.fps;
		if (host_framerate.GetInt() != 0 && host_framerate.GetInt() != framerate) {
			console->Print("Warning: overriding host_framerate to %d based on sar_render_fps and sar_render_blend\n", framerate);
		}
		host_framerate.SetValue(framerate);
	} else {  // host_framerate nonzero
		int framerate = host_framerate.GetInt();
		if (framerate % g_render.fps != 0) {
			console->Print("host_framerate must be a multiple of sar_render_fps\n");
			return;
		}
		g_render.toBlend = framerate / g_render.fps;
	}

	{
		float shutter = (float)sar_render_shutter_angle.GetInt() / 360.0f;
		int toExclude = (int)round(g_render.toBlend * (1 - shutter) / 2);
		if (toExclude * 2 >= g_render.toBlend) {
			toExclude = g_render.toBlend / 2 - 1;
		}
		g_render.toBlendStart = toExclude;
		g_render.toBlendEnd = g_render.toBlend - toExclude;
	}

	if (snd_surround_speakers.GetInt() != 2) {
		console->Print("Note: setting speaker configuration to stereo. You may wish to reset it after the render\n");
		snd_surround_speakers.SetValue(2);
	}

	AVCodecID videoCodec = videoCodecFromName(sar_render_vcodec.GetString());
	AVCodecID audioCodec = audioCodecFromName(sar_render_acodec.GetString());

	if (videoCodec == AV_CODEC_ID_NONE) {
		console->Print("Unknown video codec '%s'\n", sar_render_vcodec.GetString());
		return;
	}

	if (audioCodec == AV_CODEC_ID_NONE) {
		console->Print("Unknown audio codec '%s'\n", sar_render_acodec.GetString());
		return;
	}

	float videoBitrate = sar_render_vbitrate.GetFloat();
	float audioBitrate = sar_render_abitrate.GetFloat();

	int quality = sar_render_quality.GetInt();
	if (quality < 0) quality = 0;
	if (quality > 50) quality = 50;

	// Quality options
	{
		// TODO: CRF scales non-linearly. Do we make this conversion
		// non-linear to better reflect that?

		int min = 0;
		int max = 63;
		if (videoCodec == AV_CODEC_ID_H264 || videoCodec == AV_CODEC_ID_HEVC) {
			max = 51;
		} else if (videoCodec == AV_CODEC_ID_VP8) {
			min = 4;
		}
		int crf = min + (max - min) * (50 - quality) / 50;
		std::string crfStr = std::to_string(crf);
		av_dict_set(&options, "crf", crfStr.c_str(), 0);

		if (videoCodec == AV_CODEC_ID_H264 || videoCodec == AV_CODEC_ID_HEVC) {
			av_dict_set(&options, "preset", "slower", 0);
		}
	}

	g_render.width = GetScreenWidth();
	g_render.height = GetScreenHeight();

	g_render.workerFailedToStart.store(false);

	g_render.workerMsg.store(WorkerMsg::NONE);
	g_render.worker = std::thread(worker, videoCodec, audioCodec, videoBitrate * 1000, audioBitrate * 1000, options);

	// Busy-wait until the rendering has started so that we don't miss
	// any frames
	while (!g_render.isRendering.load() && !g_render.workerFailedToStart.load())
		;
}

// }}}

// Audio output {{{

static inline short clip16(int x) {
	if (x < -32768) return -32768;  // Source uses -32767, but that's, like, not how two's complement works
	if (x > 32767) return 32767;
	return x;
}

static void (*SND_RecordBuffer)();
static void SND_RecordBuffer_Hook();
static Hook g_RecordBufferHook(&SND_RecordBuffer_Hook);
static void SND_RecordBuffer_Hook() {
	if (!g_render.isRendering.load()) {
		g_RecordBufferHook.Disable();
		SND_RecordBuffer();
		g_RecordBufferHook.Enable();
		return;
	}

	if (g_render.isPaused.load())
		return;

	if (engine->ConsoleVisible()) return;

	// If the worker has received a message it hasn't yet handled,
	// busy-wait; this is a really obscure race condition that should
	// never happen
	while (g_render.isRendering.load() && g_render.workerMsg.load() != WorkerMsg::NONE)
		;

	if (snd_surround_speakers.GetInt() != 2) {
		console->Print("Speaker configuration changed!\n");
		msgStopRender(true);
		return;
	}

	g_render.audioBufLock.lock();

	for (int i = 0; i < *g_snd_linear_count; i += g_render.channels) {
		for (int c = 0; c < g_render.channels; ++c) {
			g_render.audioBuf[c][g_render.audioBufIdx] = clip16(((*g_snd_p)[i + c] * *g_snd_vol) >> 8);
		}

		++g_render.audioBufIdx;
		if (g_render.audioBufIdx == g_render.audioBufSz) {
			g_render.audioBufLock.unlock();

			g_render.workerUpdateLock.lock();
			g_render.workerMsg.store(WorkerMsg::AUDIO_FRAME_READY);
			g_render.workerUpdate.notify_all();
			g_render.workerUpdateLock.unlock();

			// Busy-wait until the worker locks the audio buffer; not
			// ideal, but shouldn't take long
			while (g_render.isRendering.load() && g_render.workerMsg.load() != WorkerMsg::NONE)
				;

			if (!g_render.isRendering.load()) {
				// We shouldn't write to that buffer anymore, it's been
				// invalidated
				// We've already unlocked audioBufLock so just return
				return;
			}

			g_render.audioBufLock.lock();
			g_render.audioBufIdx = 0;
		}
	}

	g_render.audioBufLock.unlock();

	return;
}

// }}}

// Video output {{{

void Renderer::Frame() {
	if (Renderer::isDemoLoading && sar_render_autostart.GetBool()) {
		bool start = engine->demoplayer->IsPlaybackFixReady();
		g_render.isPaused.store(!start);
		if (!start) return;

		Renderer::isDemoLoading = false;

		if (!g_render.isRendering.load()) {
			g_render.filename = std::string(engine->GetGameDirectory()) + "/" + std::string(engine->demoplayer->DemoName) + "." + sar_render_autostart_extension.GetString();
			startRender();
		}
	} else {
		Renderer::isDemoLoading = false;
	}

	if (!g_render.isRendering.load()) return;

	// autostop: if it's the __END__ tick, or the demo is over, stop
	// rendering


	if (sar_render_autostop.GetBool() && Renderer::segmentEndTick != -1 && engine->demoplayer->IsPlaying() && engine->demoplayer->GetTick() > Renderer::segmentEndTick) {
		if (!sar_render_merge.GetBool())
			msgStopRender(false);
		else
			g_render.isPaused.store(true);
		return;
	}

	// Don't render if the console is visible
	if (engine->ConsoleVisible()) return;

	// If the worker has received a message it hasn't yet handled,
	// busy-wait; this is a really obscure race condition that should
	// never happen
	while (g_render.isRendering.load() && g_render.workerMsg.load() != WorkerMsg::NONE)
		;

	if (GetScreenWidth() != g_render.width) {
		console->Print("Screen resolution has changed!\n");
		msgStopRender(true);
		return;
	}

	if (GetScreenHeight() != g_render.height) {
		console->Print("Screen resolution has changed!\n");
		msgStopRender(true);
		return;
	}

	if (av_frame_make_writable(g_render.videoStream.frame) < 0) {
		console->Print("Failed to make video frame writable!\n");
		msgStopRender(true);
		return;
	}

	g_render.imageBufLock.lock();

	// Double check the buffer hasn't at some point between the start of
	// the function and now been invalidated
	if (!g_render.isRendering.load()) return;

	ReadScreenPixels(0, 0, g_render.width, g_render.height, g_render.imageBuf, IMAGE_FORMAT_BGR888);

	g_render.imageBufLock.unlock();

	// Signal to the worker thread that there is image data for it to
	// process
	{
		std::lock_guard<std::mutex> lock(g_render.workerUpdateLock);
		g_render.workerMsg.store(WorkerMsg::VIDEO_FRAME_READY);
		g_render.workerUpdate.notify_all();
	}
}

// }}}

ON_EVENT(DEMO_STOP) {
	if (g_render.isRendering.load() && sar_render_autostop.GetBool() && !sar_render_merge.GetBool()) {
		msgStopRender(false);
	}
}

// Init {{{

void Renderer::Init(void **videomode) {
	g_videomode = videomode;

	snd_surround_speakers = Variable("snd_surround_speakers");

#ifdef _WIN32
	SND_RecordBuffer = (void (*)())Memory::Scan(engine->Name(), "55 8B EC 80 3D ? ? ? ? 00 53 56 57 0F 84 15 01 00 00 E8 ? ? ? ? 84 C0 0F 85 08 01 00 00 A1 ? ? ? ? 3B 05");
	g_movieInfo = *(MovieInfo_t **)((uintptr_t)SND_RecordBuffer + 5);
#else
	if (sar.game->Is(SourceGame_Portal2)) {
		SND_RecordBuffer = (void (*)())Memory::Scan(engine->Name(), "55 89 E5 57 56 53 E8 ? ? ? ? 81 C3 65 EE 76 00 83 EC 3C 89 5D D0");
	} else if (sar.game->Is(SourceGame_PortalReloaded) || sar.game->Is(SourceGame_PortalStoriesMel)) {
		SND_RecordBuffer = (void (*)())Memory::Scan(engine->Name(), "55 89 E5 57 56 53 83 EC 3C 65 A1 ? ? ? ? 89 45 E4 31 C0 E8 ? ? ? ? 84 C0 75 1B");
	} else {  // Pre-update engine
		SND_RecordBuffer = (void (*)())Memory::Scan(engine->Name(), "55 89 E5 57 56 53 83 EC 2C E8 ? ? ? ? 84 C0 75 0E 8D 65 F4 5B 5E 5F 5D C3");
	}

	if (sar.game->Is(SourceGame_Portal2)) {
		uintptr_t SND_IsRecording = Memory::Read((uintptr_t)SND_RecordBuffer + 35);
		g_movieInfo = (MovieInfo_t *)(SND_IsRecording + 6 + *(uint32_t *)(SND_IsRecording + 8) + *(uint32_t *)(SND_IsRecording + 17));
	} else if (sar.game->Is(SourceGame_PortalReloaded) || sar.game->Is(SourceGame_PortalStoriesMel)) {
		uintptr_t SND_IsRecording = Memory::Read((uintptr_t)SND_RecordBuffer + 21);
		g_movieInfo = *(MovieInfo_t **)(SND_IsRecording + 2);
	} else {  // Pre-update engine
		uintptr_t SND_IsRecording = Memory::Read((uintptr_t)SND_RecordBuffer + 10);
		g_movieInfo = *(MovieInfo_t **)(SND_IsRecording + 11);
	}
#endif

	g_RecordBufferHook.SetFunc(SND_RecordBuffer);

#ifndef _WIN32
	if (sar.game->Is(SourceGame_Portal2)) {
		uint32_t fn = (uint32_t)SND_RecordBuffer;
		uint32_t base = fn + 11 + *(uint32_t *)(fn + 13);
		g_snd_linear_count = (int *)(base + *(uint32_t *)(fn + 49));
		g_snd_p = (int **)(base + *(uint32_t *)(fn + 96));
		g_snd_vol = (int *)(base + *(uint32_t *)(fn + 102));
	} else
#endif
	{
		g_snd_linear_count = *(int **)((uintptr_t)SND_RecordBuffer + Offsets::snd_linear_count);
		g_snd_p = *(int ***)((uintptr_t)SND_RecordBuffer + Offsets::snd_p);
		g_snd_vol = *(int **)((uintptr_t)SND_RecordBuffer + Offsets::snd_vol);
	}

	Command::Hook("startmovie", &startmovie_cbk, startmovie_origCbk);
	Command::Hook("endmovie", &endmovie_cbk, endmovie_origCbk);
}

void Renderer::Cleanup() {
	g_RecordBufferHook.Disable();
	Command::Unhook("startmovie", startmovie_origCbk);
	Command::Unhook("endmovie", endmovie_origCbk);
}

// }}}

// Commands {{{

CON_COMMAND(sar_render_start, "sar_render_start <file> - start rendering frames to the given video file\n") {
	if (args.ArgC() != 2) {
		console->Print(sar_render_start.ThisPtr()->m_pszHelpString);
		return;
	}

	g_render.filename = std::string(args[1]);

	startRender();
}

CON_COMMAND(sar_render_finish, "sar_render_finish - stop rendering frames\n") {
	if (args.ArgC() != 1) {
		console->Print(sar_render_finish.ThisPtr()->m_pszHelpString);
		return;
	}

	if (!g_render.isRendering.load()) {
		console->Print("Not rendering!\n");
		return;
	}

	msgStopRender(false);
}

// }}}
