#pragma once

#include <string>
#include <cstddef>

namespace NetMessage {
    void RegisterHandler(const char *type, void (*handler)(void *data, size_t size));
    void SendMessage(const char *type, void *data, size_t size);
    bool ChatData(std::string str);
};
