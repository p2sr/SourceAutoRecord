#pragma once

#include <cstddef>
#include <string>

namespace NetMessage {
	void RegisterHandler(const char *type, void (*handler)(void *data, size_t size));
	void SendMsg(const char *type, const void *data, size_t size);
	bool ChatData(std::string str);
	void Update();
	void SessionEnded();
};  // namespace NetMessage
