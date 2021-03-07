#include "Renderer.hpp"
#include "Modules/Engine.hpp"
#include "Command.hpp"
#include "Utils/SDK.hpp"
#include "Modules/Server.hpp"
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cinttypes>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/dnxhddata.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
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

// We reference this so we can make sure the output is stereo.
// Supporting other output options is quite tricky; because Source
// movies only support stereo output, the functions for outputting
// surround audio don't have the necessary code for writing it to
// buffers like we need. This is *possible* to get around, but would
// require some horrible mid-function detours.
static Variable snd_surround_speakers;

// h264, hevc, vp8, vp9, dnxhd
// aac, ac3, mp3, vorbis, opus, flac
static Variable sar_render_vbitrate("sar_render_vbitrate", "40000", 1, "Video bitrate used in renders (kbit/s)\n");
static Variable sar_render_abitrate("sar_render_abitrate", "160", 1, "Audio bitrate used in renders (kbit/s)\n");
static Variable sar_render_vcodec("sar_render_vcodec", "h264", "Video codec used in renders (h264, hevc, vp8, vp9, dnxhd)\n", 0);
static Variable sar_render_acodec("sar_render_acodec", "vorbis", "Audio codec used in renders (aac, ac3, mp3, vorbis, opus, flac)\n", 0);
static Variable sar_render_quality("sar_render_quality", "35", 0, 50, "Render output quality, higher is better (50=lossless)\n");
static Variable sar_render_fps("sar_render_fps", "60", 1, "Render output FPS\n");
static Variable sar_render_blend("sar_render_blend", "0", 0, "How many frames to blend for each output frame; 1 = do not blend, 0 = automatically determine based on host_framerate\n");
static Variable sar_render_autostop("sar_render_autostop", "1", "Whether to automatically stop when __END__ is seen in demo playback\n");

// g_videomode VMT wrappers {{{

static inline int GetScreenWidth()
{
    return Memory::VMT<int __rescall (*)(void *)>(*g_videomode, Offsets::GetModeWidth)(*g_videomode);
}

static inline int GetScreenHeight()
{
    return Memory::VMT<int __rescall (*)(void *)>(*g_videomode, Offsets::GetModeHeight)(*g_videomode);
}

static inline void ReadScreenPixels(int x, int y, int w, int h, void *buf, ImageFormat fmt)
{
    return Memory::VMT<void __rescall (*)(void *, int, int, int, int, void *, ImageFormat)>(*g_videomode, Offsets::ReadScreenPixels)(*g_videomode, x, y, w, h, buf, fmt);
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

// The global renderer state
static struct {
    bool isRendering = false;
    int fps;
    Stream videoStream;
    Stream audioStream;
    std::string filename;
    AVFormatContext *outCtx;
    int width, height;

    // The audio stream's temporary frame needs nb_samples worth of
    // audio before we resample and submit, so we keep track of how far
    // in we are with this
    size_t audioBufSz;
    size_t audioBufIdx;

    // Frameblending! The buffers below aren't allocated if toBlend == 1.
    int toBlend;
    int nextBlendIdx; // How many frames in this blend have we seen so far?
    uint8_t *blendTmpBuf; // Temporary buffer - contains the raw pixel data read from the screen
    uint16_t *blendSumBuf; // Blending buffer - contains the sum of the pixel values during blending (we only divide at the end of the blend to prevent rounding errors)
} g_render;

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
  if (!strcmp(name, "mp3")) return AV_CODEC_ID_MP3;
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
    if (g_render.isRendering) {
        console->Print("Cannot startmovie while a SAR render is active! Stop the render with sar_render_finish.\n");
        return;
    }
    startmovie_origCbk(args);
}
static void endmovie_cbk(const CCommand &args) {
    if (g_render.isRendering) {
        console->Print("Cannot endmovie while a SAR render is active! Did you mean sar_render_finish?\n");
        return;
    }
    endmovie_origCbk(args);
}

// }}}

// AVFrame allocators {{{

static AVFrame *allocPicture(AVPixelFormat pixFmt, int width, int height)
{
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

static AVFrame *allocAudioFrame(AVSampleFormat sampleFmt, uint64_t channelLayout, int sampleRate, int nbSamples)
{
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
static bool addStream(Stream *out, AVFormatContext *outputCtx, AVCodecID codecId, int64_t bitrate, int framerate, int ptsOff, int width = 0, int height = 0)
{
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

        int64_t realBitrate = -1;
        int64_t lastDelta = INT64_MAX;
        
        for (int cid = 1235; cid <= 1274; ++cid) {
            const CIDEntry *e = ff_dnxhd_get_cid_table(cid);
            if (!e) continue;

            if (e->width != width) continue;
            if (e->height != height) continue;
            if (e->flags & DNXHD_INTERLACED) continue;
            if (e->flags & DNXHD_444) continue;
            if (e->bit_depth != 8) continue;

            for (size_t j = 0; j < FF_ARRAY_ELEMS(e->bit_rates); ++j) {
                int64_t rate = e->bit_rates[j] * 1000000;
                if (rate == 0) continue;

                int64_t delta = rate - bitrate;
                if (delta < 0) {
                    delta = -delta;
                }

                if (delta < lastDelta) {
                    lastDelta = delta;
                    realBitrate = rate;
                }
            }
        }

        if (realBitrate == -1) {
            console->Print("Resolution not supported by dnxhd\n");
            return false;
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
    out->stream->time_base = (AVRational){ 1, framerate };

    switch (out->codec->type) {
    case AVMEDIA_TYPE_AUDIO:
        out->enc->sample_fmt = out->codec->sample_fmts ? out->codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP; // don't really care about sample format
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

static void closeStream(Stream *s)
{
    avcodec_free_context(&s->enc);
    av_frame_free(&s->frame);
    av_frame_free(&s->tmpFrame);
    sws_freeContext(s->swsCtx);
    swr_free(&s->swrCtx);
}

// }}}

// Flushing streams {{{

static bool flushStream(Stream *s, bool isEnd = false)
{
    if (isEnd) {
        avcodec_send_frame(s->enc, NULL);
    }

    while (true) {
        AVPacket pkt = { 0 };
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

static bool openVideo(AVFormatContext *outputCtx, Stream *s, AVDictionary **options)
{
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

static bool openAudio(AVFormatContext *outputCtx, Stream *s, AVDictionary **options)
{
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
    av_opt_set_int(s->swrCtx, "in_sample_rate", s->enc->sample_rate, 0);
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

// startRender, finishRender {{{

static void startRender() {
    AVDictionary *options = NULL;

    if (g_render.isRendering) {
        console->Print("Already rendering\n");
        return;
    }

    if (!sv_cheats.GetBool()) {
        console->Print("sv_cheats must be enabled\n");
        return;
    }

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
    } else { // host_framerate nonzero
        int framerate = host_framerate.GetInt();
        if (framerate % g_render.fps != 0) {
            console->Print("host_framerate must be a multiple of sar_render_fps\n");
            return;
        }
        g_render.toBlend = framerate / g_render.fps;
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

    // All data gotten; start actually doing things!

    if (avformat_alloc_output_context2(&g_render.outCtx, NULL, NULL, g_render.filename.c_str()) == AVERROR(EINVAL)) {
        console->Print("Failed to deduce output format from file extension - using MP4\n");
        avformat_alloc_output_context2(&g_render.outCtx, NULL, "mp4", g_render.filename.c_str());
    }

    if (!g_render.outCtx) {
        console->Print("Failed to allocate output context\n");
        return;
    }

    if (!addStream(&g_render.videoStream, g_render.outCtx, videoCodec, videoBitrate * 1000, g_render.fps, 0, g_render.width, g_render.height)) {
        console->Print("Failed to create video stream\n");
        avformat_free_context(g_render.outCtx);
        return;
    }

    if (!addStream(&g_render.audioStream, g_render.outCtx, audioCodec, audioBitrate * 1000, 44100, 4410)) { // offset the start by 0.1s because idk
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

    g_render.nextBlendIdx = 0;
    if (g_render.toBlend > 1) {
        g_render.blendTmpBuf = (uint8_t *)malloc(3 * g_render.width * g_render.height);
        g_render.blendSumBuf = (uint16_t *)calloc(3 * g_render.width * g_render.height, sizeof g_render.blendSumBuf[0]);
    }

    g_movieInfo->moviename[0] = 'a'; // Just something nonzero to make the game think there's a movie in progress
    g_movieInfo->moviename[1] = 0;
    g_movieInfo->movieframe = 0;
    g_movieInfo->type = 0; // Should stop anything actually being output

    g_render.isRendering = true;

    console->Print("Started rendering to '%s'\n", g_render.filename.c_str());

    console->Print(
        "    video: %s (%dx%d @ %d fps, %" PRId64 " kb/s, %s)\n",
        g_render.videoStream.codec->name,
        g_render.width,
        g_render.height,
        g_render.fps,
        g_render.videoStream.enc->bit_rate / 1000,
        av_get_pix_fmt_name(g_render.videoStream.enc->pix_fmt)
    );

    console->Print(
        "    audio: %s (%d Hz, %" PRId64 " kb/s, %s)\n",
        g_render.audioStream.codec->name, g_render.audioStream.enc->sample_rate,
        g_render.audioStream.enc->bit_rate / 1000,
        av_get_sample_fmt_name(g_render.audioStream.enc->sample_fmt)
    );
}

static void finishRender(bool wasError = true) {
    if (!g_render.isRendering) {
        console->Print("Not rendering!\n");
        return;
    }

    if (wasError) {
        console->Print("A fatal error occurred; finishing render early\n");
    }

    flushStream(&g_render.videoStream, true);
    flushStream(&g_render.audioStream, true);

    av_write_trailer(g_render.outCtx);

    closeStream(&g_render.videoStream);
    closeStream(&g_render.audioStream);
    avio_closep(&g_render.outCtx->pb);
    avformat_free_context(g_render.outCtx);
    if (g_render.toBlend > 1) {
        free(g_render.blendTmpBuf);
        free(g_render.blendSumBuf);
    }

    g_render.isRendering = false;
    console->Print("Rendered %d frames to '%s'\n", g_render.videoStream.nextPts, g_render.filename.c_str());

    // Reset all the Source movieinfo struct to its default values
    g_movieInfo->moviename[0] = 0;
    g_movieInfo->movieframe = 0;
    g_movieInfo->type = MovieInfo_t::FMOVIE_TGA | MovieInfo_t::FMOVIE_WAV;
    g_movieInfo->jpeg_quality = 50;
}

// }}}


// Audio output {{{

static short clip16(int x) {
    if (x < -32768) return -32768; // Source uses -32767, but that's, like, not how two's complement works
    if (x > 32767) return 32767;
    return x;
}

static bool flushAudioBuf(void) {
    Stream *s = &g_render.audioStream;

    s->tmpFrame->pts = s->nextPts;
    s->nextPts += s->frame->nb_samples;

    if (av_frame_make_writable(s->frame) < 0) {
        console->Print("Failed to make audio frame writable!\n");
        finishRender();
        return false;
    }

    if (swr_convert(s->swrCtx, s->frame->data, s->frame->nb_samples, (const uint8_t **)s->tmpFrame->data, s->tmpFrame->nb_samples) < 0) {
        console->Print("Failed to resample audio frame!\n");
        finishRender();
        return false;
    }

    s->frame->pts = av_rescale_q(s->tmpFrame->pts, (AVRational){ 1, s->enc->sample_rate }, s->enc->time_base);

    if (avcodec_send_frame(s->enc, s->frame) < 0) {
        console->Print("Failed to send audio frame for encoding!\n");
        finishRender();
        return false;
    }

    if (!flushStream(s)) {
        console->Print("Failed to flush audio stream!\n");
        finishRender();
        return false;
    }

    return true;
}

// returns whether to run the og function
static bool SND_RecordBuffer_Hook(void) {
    if (!g_render.isRendering) {
        return true;
    }

    if (engine->ConsoleVisible()) {
        return false;
    }

    int16_t **buf = (int16_t **)g_render.audioStream.tmpFrame->data;

    if (snd_surround_speakers.GetInt() != 2) {
        console->Print("Speaker configuration changed!\n");
        finishRender();
        return false;
    }

    int chans = g_render.audioStream.enc->channels;

    for (int i = 0; i < *g_snd_linear_count; i += chans) {
        for (int c = 0; c < chans; ++c) {
            buf[c][g_render.audioBufIdx] = clip16(((*g_snd_p)[i + c] * *g_snd_vol) >> 8);
        }

        ++g_render.audioBufIdx;
        if (g_render.audioBufIdx == g_render.audioBufSz) {
            if (!flushAudioBuf()) {
                return false;
            }
            g_render.audioBufIdx = 0;
        }
    }

    return false;
}

// }}}

// Video output {{{

void Renderer::Frame()
{
    // If performance is an issue, we might want to look into stopping
    // RecordMovieFrame being run. It's not doing anything bad, but it
    // *is* pulling the screen pixel data (and then immediately
    // discarding it) every frame, which isn't great

    if (!g_render.isRendering) return;
    if (engine->ConsoleVisible()) return;

    if (GetScreenWidth() != g_render.width) {
        console->Print("Screen resolution has changed!\n");
        finishRender();
        return;
    }

    if (GetScreenHeight() != g_render.height) {
        console->Print("Screen resolution has changed!\n");
        finishRender();
        return;
    }

    if (av_frame_make_writable(g_render.videoStream.frame) < 0) {
        console->Print("Failed to make video frame writable!\n");
        finishRender();
        return;
    }

    if (g_render.toBlend == 1) {
        // We're not blending, so just write directly to the buffer
        ReadScreenPixels(0, 0, g_render.width, g_render.height, g_render.videoStream.tmpFrame->data[0], IMAGE_FORMAT_BGR888);
    } else {
        // Write to our temporary buffer, and then add the values in to
        // the frame buffer
        ReadScreenPixels(0, 0, g_render.width, g_render.height, g_render.blendTmpBuf, IMAGE_FORMAT_BGR888);
        int size = g_render.width * g_render.height * 3;
        for (size_t i = 0; i < size; ++i) {
            g_render.blendSumBuf[i] += g_render.blendTmpBuf[i];
        }

        if (++g_render.nextBlendIdx != g_render.toBlend) return; // not done blending yet

        g_render.nextBlendIdx = 0;

        for (size_t i = 0; i < size; ++i) {
            g_render.videoStream.tmpFrame->data[0][i] = g_render.blendSumBuf[i] / g_render.toBlend;
            g_render.blendSumBuf[i] = 0;
        }
    }

    // convert to output format
    sws_scale(g_render.videoStream.swsCtx, (const uint8_t *const *)g_render.videoStream.tmpFrame->data, g_render.videoStream.tmpFrame->linesize, 0, g_render.height, g_render.videoStream.frame->data, g_render.videoStream.frame->linesize);

    g_render.videoStream.frame->pts = g_render.videoStream.nextPts;

    if (avcodec_send_frame(g_render.videoStream.enc, g_render.videoStream.frame) < 0) {
        console->Print("Failed to send video frame for encoding!\n");
        finishRender();
        return;
    }

    if (!flushStream(&g_render.videoStream)) {
        console->Print("Failed to flush video stream!\n");
        finishRender();
        return;
    }

    ++g_render.videoStream.nextPts;

    if (Renderer::segmentEndTick != -1 && engine->demoplayer->IsPlaying() && engine->demoplayer->GetTick() >= Renderer::segmentEndTick) {
        finishRender(false);
    }
}

// }}}


// Init {{{

void Renderer::Init(void **videomode)
{
    g_videomode = videomode;

    snd_surround_speakers = Variable("snd_surround_speakers");

#ifdef _WIN32
    uintptr_t SND_RecordBuffer = Memory::Scan(engine->Name(), "55 8B EC 80 3D ? ? ? ? 00 53 56 57 0F 84 15 01 00 00 E8 68 DE 08 00 84 C0 0F 85 08 01 00 00 A1 ? ? ? ? 3B 05");

    g_movieInfo = *(MovieInfo_t **)(SND_RecordBuffer + 5);

    static uint8_t trampoline[] = {
      0x55,                         // 00: push ebp
      0x89, 0xE5,                   // 01: mov ebp, esp
      0xE8, 0, 0, 0, 0,             // 03: call ? (to be filled with SND_RecordBuffer_Hook)
      0x85, 0xC0,                   // 08: test eax, eax
      0x74, 0x0C,                   // 0A: jz $+E (18)
      0x80, 0x3D, 0, 0, 0, 0, 0x00, // 0C: cmp ?, 0 (to be filled with ptr to movieinfo, replacing overwritten instruction)
      0xE9, 0, 0, 0, 0,             // 13: jmp ? (to be filled with original function addr)
      0x5D,                         // 18: pop ebp
      0xC3,                         // 19: ret
    };

    Memory::UnProtect((void*)SND_RecordBuffer, 10);
    Memory::UnProtect(trampoline, sizeof trampoline);
    *(uint32_t *)(trampoline + 0x04) = (uintptr_t)SND_RecordBuffer_Hook - ((uintptr_t)trampoline + 0x04 + 4);
    *(uint32_t *)(trampoline + 0x0E) = (uintptr_t)g_movieInfo - ((uintptr_t)trampoline + 0x0E + 4);
    *(uint32_t *)(trampoline + 0x14) = (uintptr_t)SND_RecordBuffer + 10 - ((uintptr_t)trampoline + 0x14 + 4);
    ((uint8_t *)SND_RecordBuffer)[0] = 0xE9; // jmp
    *(uint32_t *)(SND_RecordBuffer + 1) = (uintptr_t)trampoline - (SND_RecordBuffer + 1 + 4);
    // We follow our code with some NOPs - not necessary as we jump past
    // them, but stops in-memory disassemblers getting confused
    ((uint8_t *)SND_RecordBuffer)[5] = 0x90;
    ((uint8_t *)SND_RecordBuffer)[6] = 0x90;
    ((uint8_t *)SND_RecordBuffer)[7] = 0x90;
    ((uint8_t *)SND_RecordBuffer)[8] = 0x90;
    ((uint8_t *)SND_RecordBuffer)[9] = 0x90;
#else
    uintptr_t SND_RecordBuffer = Memory::Scan(engine->Name(), "55 89 E5 57 56 53 83 EC 3C 65 A1 ? ? ? ? 89 45 E4 31 C0 E8 ? ? ? ? 84 C0 75 1B");

    uintptr_t SND_IsRecording = Memory::Read(SND_RecordBuffer + 21);
    g_movieInfo = *(MovieInfo_t **)(SND_IsRecording + 2);

    static uint8_t trampoline[] = {
      0x55,                   // 00: push ebp
      0x89, 0xE5,             // 01: mov ebp, esp
      0xE8, 0, 0, 0, 0,       // 03: call ? (to be filled with SND_RecordBuffer_Hook)
      0x57,                   // 08: push edi
      0x56,                   // 09: push esi
      0x85, 0xC0,             // 0A: test eax, eax
      0x0F, 0x85, 0, 0, 0, 0, // 0C: jnz ? (to be filled with original function addr)
      0x5E,                   // 12: pop esi
      0x5F,                   // 13: pop edi
      0x5D,                   // 14: pop ebp
      0xC3,                   // 15: ret
    };

    Memory::UnProtect((void*)SND_RecordBuffer, 5);
    Memory::UnProtect(trampoline, sizeof trampoline);
    *(uint32_t *)(trampoline + 0x04) = (uintptr_t)SND_RecordBuffer_Hook - ((uintptr_t)trampoline + 0x04 + 4);
    *(uint32_t *)(trampoline + 0x0E) = (uintptr_t)SND_RecordBuffer + 5 - ((uintptr_t)trampoline + 0x0E + 4);
    ((uint8_t *)SND_RecordBuffer)[0] = 0xE9; // jmp
    *(uint32_t *)(SND_RecordBuffer + 1) = (uintptr_t)trampoline - (SND_RecordBuffer + 1 + 4);
#endif

    g_snd_linear_count = *(int **)(SND_RecordBuffer + Offsets::snd_linear_count);
    g_snd_p = *(int ***)(SND_RecordBuffer + Offsets::snd_p);
    g_snd_vol = *(int **)(SND_RecordBuffer + Offsets::snd_vol);

    Command::Hook("startmovie", &startmovie_cbk, startmovie_origCbk);
    Command::Hook("endmovie", &endmovie_cbk, endmovie_origCbk);
}

// }}}


// Commands {{{

CON_COMMAND(sar_render_start, "sar_render_start <file> - start rendering frames to the given video file\n")
{
    if (args.ArgC() != 2) {
        console->Print(sar_render_start.ThisPtr()->m_pszHelpString);
        return;
    }

    g_render.filename = std::string(args[1]);

    startRender();
}

CON_COMMAND(sar_render_finish, "sar_render_finish - stop rendering frames\n")
{
    if (args.ArgC() != 1) {
        console->Print(sar_render_finish.ThisPtr()->m_pszHelpString);
        return;
    }

    finishRender(false);
}

// }}}
