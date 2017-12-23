#pragma once

using _CommandCallbackVoid = void(*)();
using _CommandCallbackArgs = void(*)(const void* args);
using _ConCommand = void(__fastcall*)(void* thisptr, void* edx, const char* name, void* callback, const char* helpstr, int flags, void* compfunc);
using _ConVar = void(__fastcall*)(void* thisptr, void* edx, const char* name, const char* value, int flags, const char* helpstr, bool hasmin, float min, bool hasmax, float max);
using _SetValueString = void(__fastcall*)(void* thisptr, void* edx, const char* value);
using _SetValueFloat = void(__fastcall*)(void* thisptr, void* edx, float value);
using _SetValueInt = void(__fastcall*)(void* thisptr, void* edx, int value);

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