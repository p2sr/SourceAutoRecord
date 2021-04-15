#pragma once

#include <string>
#include <cstddef>

namespace NetMessage {
    void RegisterHandler(const char *type, void (*handler)(void *data, size_t size));
    void SendMsg(const char *type, void *data, size_t size);
    bool ChatData(std::string str);
    void Update();
};
