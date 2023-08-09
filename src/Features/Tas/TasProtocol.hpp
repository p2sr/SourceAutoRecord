#pragma once

#include <string>
#include <cstdint>
#include <deque>

namespace TasProtocol {
	enum class PlaybackState {
		PLAYING,
		PAUSED,
		SKIPPING,
	};

	enum RecvMsg : uint8_t {
		RECV_PLAY_SCRIPT = 0,
		RECV_STOP = 1,
		RECV_PLAYBACK_RATE = 2,
		RECV_RESUME = 3,
		RECV_PAUSE = 4,
		RECV_FAST_FORWARD = 5,
		RECV_SET_PAUSE_TICK = 6,
		RECV_ADVANCE_TICK = 7,
		RECV_PLAY_SCRIPT_PROTOCOL = 10,
		RECV_ENTITY_INFO = 100
	};

	enum SendMsg : uint8_t {
		SEND_ACTIVE = 0,
		SEND_INACTIVE = 1,
		SEND_PLAYBACK_RATE = 2,
		SEND_PLAYING = 3,
		SEND_PAUSED = 4,
		SEND_SKIPPING = 5,
		SEND_CURRENT_TICK = 6,
		SEND_DEBUG_TICK = 7,
		SEND_PROCESSED_SCRIPT = 10,
		SEND_ENTITY_INFO = 100,
		SEND_GAME_LOCATION = 255
	};

	struct ConnectionData {
		SOCKET sock;
		std::deque<uint8_t> cmdbuf;
	};

	struct Status {
		bool active;
		std::string tas_path[2];
		PlaybackState playback_state;
		float playback_rate;
		int playback_tick;
	};

	void SetStatus(Status s);
	void SendProcessedScript(uint8_t slot, std::string scriptString);

} // namespace TasProtocol
