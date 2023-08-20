#pragma once

#include <string>
#include <cstdint>
#include <deque>

// defining socket here manually because
// for unknown reasons winsock cannot be included here
#ifdef _WIN32
#	define SOCKET unsigned int
#else
#	define SOCKET int
#endif


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
		RECV_MESSAGE = 8,
		RECV_PLAY_SCRIPT_PROTOCOL = 10,
		RECV_ENTITY_INFO = 100,
		RECV_SET_CONT_ENTITY_INFO = 101
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
		SEND_MESSAGE = 8,
		SEND_PROCESSED_SCRIPT = 10,
		SEND_ENTITY_INFO = 100,
		SEND_GAME_LOCATION = 255
	};

	struct Status {
		bool active;
		std::string tas_path[2];
		PlaybackState playback_state;
		float playback_rate;
		int playback_tick;
	};

	struct ConnectionData {
		SOCKET sock;
		std::deque<uint8_t> cmdbuf;
		std::string contInfoEntSelector;
	};

	void SetStatus(Status s);
	void SendProcessedScript(uint8_t slot, std::string scriptString);
	void SendEntityInfo(ConnectionData& conn, std::string entSelector);
	void SendTextMessage(std::string message);

} // namespace TasProtocol
