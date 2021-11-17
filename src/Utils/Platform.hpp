#pragma once
#ifdef _WIN32
#	include "minhook/MinHook.h"
#endif

#define _GAME_PATH(x) #x

#ifdef _WIN32
#	define MODULE_EXTENSION ".dll"
// clang-format off
#define GAME_PATH(x) _GAME_PATH(Games/Windows/##x.hpp)
// clang-format on
#	define __rescall __thiscall
#	define DLL_EXPORT extern "C" __declspec(dllexport)
#	define SEEK_DIR_CUR std::ios_base::_Seekdir::_Seekcur

#	define DECL_DETOUR(name, ...)                                   \
		using _##name = int(__rescall *)(void *thisptr, ##__VA_ARGS__); \
		static _##name name;                                            \
		static int __fastcall name##_Hook(void *thisptr, int edx, ##__VA_ARGS__)
#	define DECL_DETOUR_T(type, name, ...)                            \
		using _##name = type(__rescall *)(void *thisptr, ##__VA_ARGS__); \
		static _##name name;                                             \
		static type __fastcall name##_Hook(void *thisptr, int edx, ##__VA_ARGS__)
#	define DECL_DETOUR_B(name, ...)                                 \
		using _##name = int(__rescall *)(void *thisptr, ##__VA_ARGS__); \
		static _##name name;                                            \
		static _##name name##Base;                                      \
		static int __fastcall name##_Hook(void *thisptr, int edx, ##__VA_ARGS__)

#	define DETOUR(name, ...) \
		int __fastcall name##_Hook(void *thisptr, int edx, ##__VA_ARGS__)
#	define DETOUR_T(type, name, ...) \
		type __fastcall name##_Hook(void *thisptr, int edx, ##__VA_ARGS__)
#	define DETOUR_B(name, ...) \
		int __fastcall name##_Hook(void *thisptr, int edx, ##__VA_ARGS__)

namespace {
	bool mhInitialized = false;
}
#	define MH_HOOK(name, target)                                                                                  \
		if (!mhInitialized) {                                                                                         \
			MH_Initialize();                                                                                             \
			mhInitialized = true;                                                                                        \
		}                                                                                                             \
		name = reinterpret_cast<decltype(name)>(target);                                                              \
		MH_CreateHook(reinterpret_cast<LPVOID>(target), name##_Hook, reinterpret_cast<LPVOID *>(&name##_Trampoline)); \
		MH_EnableHook(reinterpret_cast<LPVOID>(target));
#	define MH_HOOK_MID(name, target)                                                                            \
		if (!mhInitialized) {                                                                                       \
			MH_Initialize();                                                                                           \
			mhInitialized = true;                                                                                      \
		}                                                                                                           \
		name = target;                                                                                              \
		MH_CreateHook(reinterpret_cast<LPVOID>(name), name##_Hook, reinterpret_cast<LPVOID *>(&name##_Trampoline)); \
		MH_EnableHook(reinterpret_cast<LPVOID>(name));
#	define MH_UNHOOK(name)                           \
		if (name) {                                      \
			MH_DisableHook(reinterpret_cast<LPVOID>(name)); \
			MH_RemoveHook(reinterpret_cast<LPVOID>(name));  \
		}
#	define DECL_DETOUR_MID_MH(name)     \
		static uintptr_t name;              \
		static uintptr_t name##_Trampoline; \
		static void name##_Hook()
#	define DECL_DETOUR_MH(name, ...)                               \
		using _##name = int(__thiscall *)(void *thisptr, __VA_ARGS__); \
		static _##name name;                                           \
		static _##name name##_Trampoline;                              \
		static int __fastcall name##_Hook(void *thisptr, int edx, __VA_ARGS__)

#	define DETOUR_MID_MH(name) \
		__declspec(naked) void name##_Hook()
#	define DETOUR_MH(name, ...) \
		int __fastcall name##_Hook(void *thisptr, int edx, __VA_ARGS__)
#else
#	define MODULE_EXTENSION ".so"
// clang-format off
#define GAME_PATH(x) _GAME_PATH(Games/Linux/x.hpp)
// clang-format on
#	define __rescall __attribute__((__cdecl__))
#	define __cdecl __attribute__((__cdecl__))
#	define __fastcall __attribute__((__fastcall__))
#	define DLL_EXPORT extern "C" __attribute__((visibility("default")))
#	define SEEK_DIR_CUR std::ios_base::seekdir::_S_cur

#	define DECL_DETOUR(name, ...)                                   \
		using _##name = int(__rescall *)(void *thisptr, ##__VA_ARGS__); \
		static _##name name;                                            \
		static int __rescall name##_Hook(void *thisptr, ##__VA_ARGS__)
#	define DECL_DETOUR_T(type, name, ...)                            \
		using _##name = type(__rescall *)(void *thisptr, ##__VA_ARGS__); \
		static _##name name;                                             \
		static type __rescall name##_Hook(void *thisptr, ##__VA_ARGS__)
#	define DECL_DETOUR_B(name, ...)                                 \
		using _##name = int(__rescall *)(void *thisptr, ##__VA_ARGS__); \
		static _##name name;                                            \
		static _##name name##Base;                                      \
		static int __rescall name##_Hook(void *thisptr, ##__VA_ARGS__)

#	define DETOUR(name, ...) \
		int __rescall name##_Hook(void *thisptr, ##__VA_ARGS__)
#	define DETOUR_T(type, name, ...) \
		type __rescall name##_Hook(void *thisptr, ##__VA_ARGS__)
#	define DETOUR_B(name, ...) \
		int __rescall name##_Hook(void *thisptr, ##__VA_ARGS__)
#endif
