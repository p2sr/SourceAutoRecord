#pragma once
#include "Offsets.h"
#include "Utils.h"

#define FCVAR_NEVER_AS_STRING (1 << 12)

using _FindVar = void*(__fastcall*)(void* thisptr, void* edx, const char* name);

namespace Cvar
{
	void* Ptr;
	_FindVar FindVar;

	void Set(uintptr_t cvarPtr)
	{
		Ptr = **(void***)(cvarPtr);
		FindVar = (_FindVar)GetVirtualFunctionByIndex(Ptr, Offsets::FindVar);
	}
}