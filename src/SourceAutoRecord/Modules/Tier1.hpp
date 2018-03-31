#pragma once
#define __cdecl __attribute__((__cdecl__))

#define FCVAR_DEVELOPMENTONLY	(1<<1)
#define FCVAR_HIDDEN			(1<<4)
#define FCVAR_NEVER_AS_STRING	(1<<12)
#define FCVAR_CHEAT				(1<<14)

namespace Tier1
{
	using _CommandCallbackVoid = void(*)();
	using _CommandCallbackArgs = void(*)(const void* args);
	using _ConCommand = void(__cdecl*)(void* thisptr, const char* name, void* callback, const char* helpstr, int flags, void* compfunc);
	using _ConVar = void(__cdecl*)(void* thisptr, const char* name, const char* value, int flags, const char* helpstr, bool hasmin, float min, bool hasmax, float max);
	using _InternalSetValue = void(__cdecl*)(void* thisptr, const char* value);
	using _InternalSetFloatValue = void(__cdecl*)(void* thisptr, float value);
	using _InternalSetIntValue = void(__cdecl*)(void* thisptr, int value);

	int ConCommandCount = 0;
	int ConVarCount = 0;

	struct ConCommandBase {
		void* VMT;
		ConCommandBase* m_pNext;
		bool m_bRegistered;
		const char* m_pszName;
		const char* m_pszHelpString;
		int flags;
	};

	struct CCommand {
		enum {
			COMMAND_MAX_ARGC = 64,
			COMMAND_MAX_LENGTH = 512,
		};
		int m_nArgc;
		int m_nArgv0Size;
		char m_pArgSBuffer[COMMAND_MAX_LENGTH];
		char m_pArgvBuffer[COMMAND_MAX_LENGTH];
		const char* m_ppArgv[COMMAND_MAX_ARGC];
	};
}