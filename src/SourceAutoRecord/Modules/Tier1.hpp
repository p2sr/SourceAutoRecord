#pragma once
#define __cdecl __attribute__((__cdecl__))

#define FCVAR_DEVELOPMENTONLY	(1<<1)
#define FCVAR_HIDDEN			(1<<4)
#define FCVAR_NEVER_AS_STRING	(1<<12)
#define FCVAR_CHEAT				(1<<14)

using _CommandCallbackVoid = void(*)();
using _CommandCallbackArgs = void(*)(const void* args);
using _ConCommand = void(__cdecl*)(void* thisptr, const char* name, void* callback, const char* helpstr, int flags, void* compfunc);
using _ConVar = void(__cdecl*)(void* thisptr, const char* name, const char* value, int flags, const char* helpstr, bool hasmin, float min, bool hasmax, float max);
using _SetValueString = void(__cdecl*)(void* thisptr, const char* value);
using _SetValueFloat = void(__cdecl*)(void* thisptr, float value);
using _SetValueInt = void(__cdecl*)(void* thisptr, int value);

namespace Tier1
{
	int ConCommandCount = 0;
	int ConVarCount = 0;

	struct ConCommandBase {
		void* VTable_ConCommandBase;
		ConCommandBase* Next;
		bool Registered;
		const char* Name;
		const char* HelpString;
		int Flags;
	};

	struct ConCommandArgsData {
		enum {
			COMMAND_MAX_ARGC = 64,
			COMMAND_MAX_LENGTH = 512,
		};
		int ArgC;
		int ArgV0Size;
		char ArgSBuffer[COMMAND_MAX_LENGTH];
		char ArgVBuffer[COMMAND_MAX_LENGTH];
		const char* ArgV[COMMAND_MAX_ARGC];
	};
}