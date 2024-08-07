#pragma once

#include "Variable.hpp"

#include <cstddef>
#include <string>

extern bool g_orangeReady;
extern bool g_partnerHasSAR;

extern Variable sar_netmessage_debug;

#define SAR_MSG_HELLO "Hello! I have \x07SourceAutoRecord\x07, a plugin mainly used for speedrunning."
#define SAR_MSG_HELLO_ACK "&^@$Yes hello I also have \x07SourceAutoRecord\x07 thank you for checking before spamming chat."

namespace NetMessage {
	void RegisterHandler(const char *type, void (*handler)(const void *data, size_t size));
	void SendMsg(const char *type, const void *data, size_t size);
	bool ChatData(std::string str);
	void Update();
	void SessionStarted();
	void SessionEnded();
};  // namespace NetMessage
