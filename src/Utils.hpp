#pragma once
#include "Utils/Math.hpp"
#include "Utils/Memory.hpp"
#include "Utils/Platform.hpp"
#include "Utils/SDK.hpp"

#ifndef _WIN32
#include <unistd.h>
#endif

#define MODULE(name) name MODULE_EXTENSION
#define GAME(x) GAME_PATH(x)

static bool ends_with(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && !str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

#define REDECL(name) \
    decltype(name) name

#define SAFE_DELETE(ptr) \
    if (ptr) {           \
        delete ptr;      \
        ptr = nullptr;   \
    }

#if _WIN32
#define GO_THE_FUCK_TO_SLEEP(ms) Sleep(ms)
#else
#define GO_THE_FUCK_TO_SLEEP(ms) usleep(ms * 1000)
#endif
