#pragma once
#include "Utils/Math.hpp"
#include "Utils/Memory.hpp"
#include "Utils/Platform.hpp"
#include "Utils/SDK.hpp"

#define MODULE(name) name MODULE_EXTENSION
#define GAME(x) GAME_PATH(x)

static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && !str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

#define DETOUR_STD(name, ...)                      \
    using _##name = void(__stdcall*)(__VA_ARGS__); \
    namespace Original {                           \
        _##name name;                              \
    }                                              \
    namespace Detour {                             \
        void __stdcall name(__VA_ARGS__);          \
    }                                              \
    void __stdcall Detour::name(__VA_ARGS__)

#define SAFE_DELETE(ptr) \
    if (ptr) {           \
        delete ptr;      \
        ptr = nullptr;   \
    }
