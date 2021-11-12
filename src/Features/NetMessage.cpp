#include "NetMessage.hpp"

#include "Features/Session.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Utils.hpp"

#include <map>
#include <queue>
#include <stdexcept>

// if blue: whether orange is ready
// if orange: whether we've sent the ready packet
static bool g_orangeReady = false;

static std::map<std::string, void (*)(void *, size_t)> g_handlers;

void NetMessage::RegisterHandler(const char *type, void (*handler)(void *, size_t)) {
	g_handlers[std::string(type)] = handler;
}

static inline void handleMessage(const char *type, void *data, size_t size) {
	if (!strcmp(type, "__sync")) {
		if (size == 6 && !strcmp((char *)data, "ready")) {
			g_orangeReady = true;
		}
		return;
	}

	auto match = g_handlers.find(std::string(type));
	if (match != g_handlers.end()) {
		(*match->second)(data, size);
	}
}

static bool readyToSend() {
	if (!engine->IsCoop() || engine->IsOrange()) {
		return session->isRunning;
	} else {
		return g_orangeReady;
	}
}

void NetMessage::SessionEnded() {
	g_orangeReady = false;
}

static std::queue<std::string> g_queued;

void NetMessage::SendMsg(const char *type, const void *data, size_t size) {
	if (!engine->IsCoop()) {
		// It doesn't make sense to send messages in SP
		return;
	}

	char player = engine->IsOrange() ? 'o' : 'b';

	char *data_ = (char *)data;
	std::string cmd = std::string("say !SAR:") + player + ":" + type + ":";
	for (size_t i = 0; i < size; ++i) {
		char c = data_[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
			cmd += c;
		} else {
			char hex[4];
			snprintf(hex, sizeof hex, "#%02X", (int)(unsigned char)c);
			cmd += hex;
		}
	}

	if (cmd.size() > 255) {
		// TODO: AHHHHHHHHHH
	}

	if (readyToSend()) {
		engine->ExecuteCommand(cmd.c_str());
	} else {
		g_queued.push(cmd);
	}
}

void NetMessage::Update() {
	if (!session->isRunning) {
		g_orangeReady = false;
	}

	if (engine->IsOrange() && !g_orangeReady && readyToSend()) {
		NetMessage::SendMsg("__sync", (void *)"ready", 6);
		g_orangeReady = true;
	}

	if (readyToSend()) {
		while (!g_queued.empty()) {
			engine->ExecuteCommand(g_queued.front().c_str());
			g_queued.pop();
		}
	}
}

bool NetMessage::ChatData(std::string str) {
	if (str[str.size() - 1] != '\n') return false;

	size_t pos = str.find(": !SAR:");
	if (pos == std::string::npos) return false;

	// Strips header and trailing newline
	str = str.substr(pos + 7, str.size() - pos - 8);

	if (str[0] != 'o' && str[0] != 'b') return false;
	if (str[1] != ':') return false;

	char player = engine->IsOrange() ? 'o' : 'b';
	if (str[0] == player) return true; // Ignore messages we sent

	// Strip o: or b:
	str = str.substr(2, str.size() - 2);

	pos = str.find(":");
	if (pos == std::string::npos) return false;

	std::string type = str.substr(0, pos);
	str = str.substr(pos + 1);

	std::vector<uint8_t> buf;

	for (size_t i = 0; i < str.size(); ++i) {
		if (str[i] == '#') {
			try {
				int c = stoi(str.substr(i + 1, 2), 0, 16);
				buf.push_back((uint8_t)c);
			} catch (std::invalid_argument &e) {
				return false;
			}
			i += 2;
		} else {
			buf.push_back((uint8_t)str[i]);
		}
	}

	handleMessage(type.c_str(), buf.data(), buf.size());

	return true;
}
