#include "NetMessage.hpp"

#include "Features/Session.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Utils.hpp"
#include "Event.hpp"

#include <map>
#include <queue>
#include <stdexcept>

// 4-byte sequences crafted to be unlikely to appear in normal chat messages. These are sent at the
// start of messages to indicate SAR data. It's important these remain small. We have separate
// prefixes for continuations to prevent shit hitting the fan if the connection is lost mid-message.
#define SAR_MSG_INIT_B "&^!$"
#define SAR_MSG_INIT_O "&^!%"
#define SAR_MSG_CONT_B "&^?$"
#define SAR_MSG_CONT_O "&^?%"

// if blue: whether orange is ready
// if orange: whether we've sent the ready packet
bool g_orangeReady = false;

static size_t g_expected_len = 0;
static std::string g_partial;

static std::map<std::string, void (*)(const void *, size_t)> g_handlers;

void NetMessage::RegisterHandler(const char *type, void (*handler)(const void *, size_t)) {
	g_handlers[std::string(type)] = handler;
}

static inline void handleMessage(const char *type, const void *data, size_t size) {
	if (!strcmp(type, "__sync")) {
		if (size == 6 && !strcmp((const char *)data, "ready")) {
			g_orangeReady = true;
			Event::Trigger<Event::ORANGE_READY>({});
		}
		return;
	}

	auto match = g_handlers.find(std::string(type));
	if (match != g_handlers.end()) {
		(*match->second)(data, size);
	}
}

static bool readyToSend() {
	if (!engine->IsCoop()) {
		return false;
	} else if (engine->IsOrange()) {
		return session->isRunning;
	} else {
		return g_orangeReady;
	}
}

void NetMessage::SessionEnded() {
	g_orangeReady = false;
}

struct QueuedMsg {
	std::string type;
	std::vector<uint8_t> data;
};

static std::queue<QueuedMsg> g_queued;

///// START BASE92 /////

// This isn't really base92. Instead, we encode 4-byte input chunks into 5 base92 characters. If the
// final chunk is not 4 bytes, each byte of it is sent as 2 base92 characters; the receiver infers
// this from the buffer length and decodes accordingly. This system is almost as space-efficient as
// is possible.

static char base92_chars[93] = // 93 because null terminator
	"abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"0123456789"
	"!$%^&*-_=+()[]{}<>'@#~;:/?,.|\\";

// doing this at runtime is a little silly but shh
static const char *base92_reverse() {
	static char map[256];
	static bool initd = false;
	if (!initd) {
		initd = true;
		for (size_t i = 0; i < 92; ++i) {
			char c = base92_chars[i];
			map[(uint8_t)c] = i;
		}
	}
	return map;
}

static std::string base92_encode(const uint8_t *raw, size_t len) {
	std::string out;
	while (len >= 4) {
		uint32_t val = *(uint32_t *)raw;

		out += base92_chars[val % 92];
		val /= 92;
		out += base92_chars[val % 92];
		val /= 92;
		out += base92_chars[val % 92];
		val /= 92;
		out += base92_chars[val % 92];
		val /= 92;
		out += base92_chars[val];

		raw += 4;
		len -= 4;
	}
	while (len > 0) {
		uint8_t val = *raw;
		out += base92_chars[val % 92];
		out += base92_chars[val / 92];
		raw += 1;
		len -= 1;
	}
	return out;
}

static std::vector<uint8_t> base92_decode(const char *encoded, size_t len) {
	const char *base92_rev = base92_reverse();

	std::vector<uint8_t> out;
	while (len > 6 || len == 5) {
		uint32_t val = base92_rev[(uint8_t)encoded[4]];
		val = (val * 92) + base92_rev[(uint8_t)encoded[3]];
		val = (val * 92) + base92_rev[(uint8_t)encoded[2]];
		val = (val * 92) + base92_rev[(uint8_t)encoded[1]];
		val = (val * 92) + base92_rev[(uint8_t)encoded[0]];

		uint8_t *raw = (uint8_t *)&val;
		out.push_back(raw[0]);
		out.push_back(raw[1]);
		out.push_back(raw[2]);
		out.push_back(raw[3]);

		encoded += 5;
		len -= 5;
	}
	while (len > 0) {
		uint8_t val = base92_rev[(uint8_t)encoded[0]] + (base92_rev[(uint8_t)encoded[1]] * 92);
		out.push_back(val);
		encoded += 2;
		len -= 2;
	}
	return out;
}

///// END BASE92 /////

void NetMessage::SendMsg(const char *type, const void *data, size_t size) {
	if (!engine->IsCoop() || engine->IsSplitscreen()) {
		// It doesn't make sense to send messages in SP
		return;
	}

	if (!readyToSend()) {
		g_queued.push({
			std::string(type),
			std::vector<uint8_t>((const uint8_t *)data, (const uint8_t *)data + size),
		});
		return;
	}

	const char *init_prefix = engine->IsOrange() ? SAR_MSG_INIT_O : SAR_MSG_INIT_B;
	const char *cont_prefix = engine->IsOrange() ? SAR_MSG_CONT_O : SAR_MSG_CONT_B;

	size_t type_len = strlen(type);
	size_t raw_len = type_len + size + 1;
	uint8_t *raw = new uint8_t[raw_len];
	memcpy(raw, type, type_len);
	raw[type_len] = 0;
	memcpy(raw + type_len + 1, data, size);
	std::string encoded = base92_encode(raw, raw_len);
	delete[] raw;

	// TODO: first do compression on the raw data? probably only if it's long

	// We can only send messages up to 255 characters, which means sometimes a single NetMessage has
	// to correspond to multiple chat messages. We'll handle this by prefixing the length, and sending
	// a sequence of messages.
	uint32_t encoded_len = encoded.size();
	std::string encoded_len_encoded = base92_encode((uint8_t *)&encoded_len, 4);
	// note that encoded_len_encoded is exactly 5 characters

	size_t i = 0;
	std::string full_encoded = encoded_len_encoded + encoded;

	while (i < full_encoded.size()) {
		std::string chat = i == 0 ? init_prefix : cont_prefix;
		size_t n = 255 - chat.size();
		n = i + n > full_encoded.size() ? full_encoded.size() - i : n;
		chat.append(full_encoded, i, n);
		i += n;

		std::string cmd = Utils::ssprintf("say \"%s\"", chat.c_str());
		engine->ExecuteCommand(cmd.c_str(), true);
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
			auto &msg = g_queued.front();
			NetMessage::SendMsg(msg.type.c_str(), msg.data.data(), msg.data.size());
			g_queued.pop();
		}
	}
}

bool NetMessage::ChatData(std::string str) {
	if (str.size() < 4) return false;

	std::string prefix = str.substr(0, 4);
	bool has_prefix = true;
	bool cont, orange;
	if (prefix == SAR_MSG_INIT_B) {
		cont = false; orange = false;
	} else if (prefix == SAR_MSG_INIT_O) {
		cont = false; orange = true;
	} else if (prefix == SAR_MSG_CONT_B) {
		cont = true; orange = false;
	} else if (prefix == SAR_MSG_CONT_O) {
		cont = true; orange = true;
	} else {
		has_prefix = false;
	}

	if (!has_prefix) return false;
	if (orange == engine->IsOrange()) return true; // Ignore messages we sent

	if (cont) {
		if (!g_expected_len) return true;
		g_partial += str.substr(4);
	} else {
		const char *s = str.c_str();
		auto raw_len = base92_decode(s + 4, 5);
		g_expected_len = *(const uint32_t *)raw_len.data();
		g_partial = str.substr(9);
	}

	if (g_partial.size() < g_expected_len) return true;

	if (g_partial.size() == g_expected_len) {
		// valid message
		auto decoded = base92_decode(g_partial.data(), g_expected_len);
		const char *type = (const char *)decoded.data(); // starts with null-terminated type
		size_t type_len = strlen(type);
		const uint8_t *data = decoded.data() + type_len + 1;
		handleMessage(type, data, decoded.size() - type_len - 1);
	}

	g_expected_len = 0;
	g_partial.clear();

	return true;
}
