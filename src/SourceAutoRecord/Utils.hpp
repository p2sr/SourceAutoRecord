#pragma once
#ifdef _WIN32
#include "minhook/MinHook.h"
#endif
#include "vmthook/vmthook.h"

#include "Utils/Math.hpp"
#include "Utils/Memory.hpp"
#include "Utils/SDK.hpp"

typedef std::unique_ptr<VMTHook> VMT;

#define HOOK(vmt, name)                                        \
    vmt->HookFunction((void*)Detour::name, Offsets::name); \
    Original::name = vmt->GetOriginalFunction<_##name>(Offsets::name);
#define HOOK_O(vmt, name, offset)                     \
    vmt->HookFunction((void*)Detour::name, offset); \
    Original::name = vmt->GetOriginalFunction<_##name>(offset);

#define UNHOOK(vmt, name) \
    if (vmt)              \
        vmt->UnhookFunction(Offsets::name);
#define UNHOOK_O(vmt, offset) \
    if (vmt)                  \
        vmt->UnhookFunction(offset);

#define _GAME_PATH(x) #x

#ifdef _WIN32
#define MODULE_EXTENSION ".dll"
// clang-format off
#define GAME_PATH(x) _GAME_PATH(Games/Windows/##x.hpp)
// clang-format on
#define __func __thiscall
#define DLL_EXPORT extern "C" __declspec(dllexport)

#define DETOUR(name, ...)                                         \
    using _##name = int(__thiscall*)(void* thisptr, __VA_ARGS__); \
    namespace Original {                                          \
        _##name name;                                             \
    }                                                             \
    namespace Detour {                                            \
        int __fastcall name(void* thisptr, int edx, __VA_ARGS__); \
    }                                                             \
    int __fastcall Detour::##name(void* thisptr, int edx, __VA_ARGS__)
#define DETOUR_T(type, name, ...)                                  \
    using _##name = type(__thiscall*)(void* thisptr, __VA_ARGS__); \
    namespace Original {                                           \
        _##name name;                                              \
    }                                                              \
    namespace Detour {                                             \
        type __fastcall name(void* thisptr, int edx, __VA_ARGS__); \
    }                                                              \
    type __fastcall Detour::##name(void* thisptr, int edx, __VA_ARGS__)
#define DETOUR_B(name, ...)                                       \
    using _##name = int(__thiscall*)(void* thisptr, __VA_ARGS__); \
    namespace Original {                                          \
        _##name name;                                             \
        _##name name##Base;                                       \
    }                                                             \
    namespace Detour {                                            \
        int __fastcall name(void* thisptr, int edx, __VA_ARGS__); \
    }                                                             \
    int __fastcall Detour::##name(void* thisptr, int edx, __VA_ARGS__)

namespace {
bool mhInitialized = false;
}
#define MH_HOOK(name, target)                                                                                        \
    if (!mhInitialized) {                                                                                            \
        MH_Initialize();                                                                                             \
        mhInitialized = true;                                                                                        \
    }                                                                                                                \
    Original::##name = reinterpret_cast<_##name>(target);                                                            \
    MH_CreateHook(reinterpret_cast<LPVOID>(target), Detour::##name, reinterpret_cast<LPVOID*>(&Trampoline::##name)); \
    MH_EnableHook(reinterpret_cast<LPVOID>(target));
#define MH_HOOK_MID(name, target)                                                                                              \
    if (!mhInitialized) {                                                                                                      \
        MH_Initialize();                                                                                                       \
        mhInitialized = true;                                                                                                  \
    }                                                                                                                          \
    Original::##name = target;                                                                                                 \
    MH_CreateHook(reinterpret_cast<LPVOID>(Original::##name), Detour::##name, reinterpret_cast<LPVOID*>(&Trampoline::##name)); \
    MH_EnableHook(reinterpret_cast<LPVOID>(Original::##name));
#define MH_UNHOOK(name)                                             \
    if (Original::##name) {                                         \
        MH_DisableHook(reinterpret_cast<LPVOID>(Original::##name)); \
        MH_RemoveHook(reinterpret_cast<LPVOID>(Original::##name));  \
    }
#define DETOUR_MID_MH(name) \
    namespace Original {    \
        uintptr_t name;     \
    }                       \
    namespace Trampoline {  \
        uintptr_t name;     \
    }                       \
    namespace Detour {      \
        void name();        \
    }                       \
    __declspec(naked) void Detour::##name()
#define DETOUR_MH(name, ...)                                      \
    using _##name = int(__thiscall*)(void* thisptr, __VA_ARGS__); \
    namespace Original {                                          \
        _##name name;                                             \
    }                                                             \
    namespace Trampoline {                                        \
        _##name name;                                             \
    }                                                             \
    namespace Detour {                                            \
        int __fastcall name(void* thisptr, int edx, __VA_ARGS__); \
    }                                                             \
    int __fastcall Detour::##name(void* thisptr, int edx, __VA_ARGS__)
#else
#define MODULE_EXTENSION ".so"
// clang-format off
#define GAME_PATH(x) _GAME_PATH(Games/Linux/x.hpp)
// clang-format on
#define __func __attribute__((__cdecl__))
#define __cdecl __attribute__((__cdecl__))
#define __stdcall __attribute__((__stdcall__))
#define __fastcall __attribute__((__fastcall__))
#define DLL_EXPORT extern "C" __attribute__((visibility("default")))

#define DETOUR(name, ...)                                        \
    using _##name = int(__cdecl*)(void* thisptr, ##__VA_ARGS__); \
    namespace Original {                                         \
        _##name name;                                            \
    }                                                            \
    namespace Detour {                                           \
        int __cdecl name(void* thisptr, ##__VA_ARGS__);          \
    }                                                            \
    int __cdecl Detour::name(void* thisptr, ##__VA_ARGS__)
#define DETOUR_T(type, name, ...)                                 \
    using _##name = type(__cdecl*)(void* thisptr, ##__VA_ARGS__); \
    namespace Original {                                          \
        _##name name;                                             \
    }                                                             \
    namespace Detour {                                            \
        type __cdecl name(void* thisptr, ##__VA_ARGS__);          \
    }                                                             \
    type __cdecl Detour::name(void* thisptr, ##__VA_ARGS__)
#define DETOUR_B(name, ...)                                      \
    using _##name = int(__cdecl*)(void* thisptr, ##__VA_ARGS__); \
    namespace Original {                                         \
        _##name name;                                            \
        _##name name##Base;                                      \
    }                                                            \
    namespace Detour {                                           \
        int __cdecl name(void* thisptr, ##__VA_ARGS__);          \
    }                                                            \
    int __cdecl Detour::name(void* thisptr, ##__VA_ARGS__)
#endif

#define MODULE(name) name MODULE_EXTENSION
#define GAME(x) GAME_PATH(x)
