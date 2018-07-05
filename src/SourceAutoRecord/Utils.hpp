#pragma once
#include "minhook/MinHook.h"
#include "vmthook/vmthook.h"

#include "Utils/Math.hpp"
#include "Utils/Memory.hpp"
#include "Utils/SDK.hpp"

typedef std::unique_ptr<VMTHook> VMT;

#define HOOK(vmt, name) \
    vmt->HookFunction((void*)Detour::##name, Offsets::##name); \
    Original::##name = vmt->GetOriginalFunction<_##name>(Offsets::##name);
#define HOOK_O(vmt, name, offset) \
    vmt->HookFunction((void*)Detour::##name, offset); \
    Original::##name = vmt->GetOriginalFunction<_##name>(offset);

#define UNHOOK(vmt, name) \
    if (vmt) vmt->UnhookFunction(Offsets::##name);

bool mhInitialized = false;
#define MH_HOOK(orig, detour) \
    if (!mhInitialized) { MH_Initialize(); mhInitialized = true; } \
    MH_CreateHook(reinterpret_cast<LPVOID>(orig), detour, nullptr); \
    MH_EnableHook(reinterpret_cast<LPVOID>(orig));

#define _GAME_PATH(x) #x

#ifdef _WIN32
#define MODULE_EXTENSION ".dll"
#define GAME_PATH(x) _GAME_PATH(Games/Windows/##x.hpp)
#define __func __thiscall

#define DETOUR(name, ...) \
    using _##name = int(__thiscall*)(void* thisptr, __VA_ARGS__); \
    namespace Original { _##name name; } \
    namespace Detour { int __fastcall name(void* thisptr, int edx, __VA_ARGS__); } \
    int __fastcall Detour::##name(void* thisptr, int edx, __VA_ARGS__)
#define DETOUR_T(type, name, ...) \
    using _##name = type(__thiscall*)(void* thisptr, __VA_ARGS__); \
    namespace Original { _##name name; } \
    namespace Detour { type __fastcall name(void* thisptr, int edx, __VA_ARGS__); } \
    type __fastcall Detour::##name(void* thisptr, int edx, __VA_ARGS__)
#define DETOUR_B(name, ...) \
    using _##name = int(__thiscall*)(void* thisptr, __VA_ARGS__); \
    namespace Original { _##name name;  _##name name##Base; } \
    namespace Detour { int __fastcall name(void* thisptr, int edx, __VA_ARGS__); } \
    int __fastcall Detour::##name(void* thisptr, int edx, __VA_ARGS__)
#else
#define MODULE_EXTENSION ".so"
#define GAME_PATH(x) _GAME_PATH(Games/Linux/##x.hpp)
#define __func __attribute__((__cdecl__))
#define __cdecl __attribute__((__cdecl__))
#define __stdcall __attribute__((__stdcall__))
#define __fastcall __attribute__((__fastcall__))

#define DETOUR(name, ...) \
    using _##name = int(__cdecl*)(void* thisptr, __VA_ARGS__); \
    namespace Original { _##name name; } \
    namespace Detour { int __cdecl name(void* thisptr, __VA_ARGS__); } \
    int __cdecl Detour::##name(void* thisptr, __VA_ARGS__)
#define DETOUR_T(type, name, ...) \
    using _##name = type(__cdecl*)(void* thisptr, __VA_ARGS__); \
    namespace Original { _##name name; } \
    namespace Detour { type __cdecl name(void* thisptr, __VA_ARGS__); } \
    type __cdecl Detour::##name(void* thisptr, __VA_ARGS__)
#define DETOUR_B(name, ...) \
    using _##name = int(__cdecl*)(void* thisptr, __VA_ARGS__); \
    namespace Original { _##name name; _##name name##Base; } \
    namespace Detour { int __cdecl name(void* thisptr, __VA_ARGS__); } \
    int __cdecl Detour::##name(void* thisptr, __VA_ARGS__)
#endif

#define MODULE(name) name MODULE_EXTENSION
#define GAME(x) GAME_PATH(x)