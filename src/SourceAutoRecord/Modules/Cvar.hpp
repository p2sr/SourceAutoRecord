#pragma once
#include "ConVar.hpp"

#include "Offsets.hpp"
#include "Utils.hpp"

using _FindVar = void*(__cdecl*)(void* thisptr, const char* name);

namespace Cvar
{
	void* Ptr;
	_FindVar FindVar;

	ConCommandBase* ConCommandList;

	void Set(uintptr_t cvarPtr)
	{
		Ptr = **(void***)(cvarPtr);
		FindVar = (_FindVar)GetVirtualFunctionByIndex(Ptr, Offsets::FindVar);
		ConCommandList = (ConCommandBase*)((uintptr_t)Ptr + Offsets::m_pConCommandList);
	}
	ConVar FindCvar(const char* ref)
	{
		return ConVar(Cvar::FindVar(Cvar::Ptr, ref));
	}
}