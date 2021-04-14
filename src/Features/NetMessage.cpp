#include "NetMessage.hpp"
#include "Modules/Engine.hpp"
#include "Utils.hpp"
#include <map>
#include <stdexcept>

static std::map<const char *, void (*)(void *, size_t)> g_handlers;

void NetMessage::RegisterHandler(const char *type, void (*handler)(void *, size_t))
{
    g_handlers[type] = handler;
}

static inline void handleMessage(const char *type, void *data, size_t size)
{
    auto match = g_handlers.find(type);
    if (match != g_handlers.end()) {
        (*match->second)(data, size);
    }
}

void NetMessage::SendMsg(const char *type, void *data, size_t size)
{
    char *data_ = (char *)data;
    std::string cmd = std::string("say !SAR:") + type + ":";
    for (size_t i = 0; i < size; ++i) {
        char c = data_[i];
        if ((c >= 'a' && c <= 'z')
            || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9'))
        {
            cmd += c;
        } else {
            char hex[4];
            snprintf(hex, sizeof hex, "#%02X", (int)c);
            cmd += hex;
        }
    }

    if (cmd.size() > 255) {
        // TODO: AHHHHHHHHHH
    }

    engine->ExecuteCommand(cmd.c_str());
}

bool NetMessage::ChatData(std::string str)
{
    if (str[str.size() - 1] != '\n') return false;

    size_t pos = str.find(": !SAR:");
    if (pos == std::string::npos) return false;

    // Strips header and trailing newline
    str = str.substr(pos + 7, str.size() - pos - 8);

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
