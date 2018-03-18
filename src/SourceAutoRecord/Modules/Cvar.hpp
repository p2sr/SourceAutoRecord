#pragma once
#include "ConVar.hpp"

#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

namespace Cvar
{
	using _FindVar = void*(__cdecl*)(void* thisptr, const char* name);

	void* Ptr;
	_FindVar FindVar;

	ConCommandBase* ConCommandList;

	bool Loaded()
	{
		auto cvr = SAR::Find("CvarPtr");
		if (cvr.Found) {
			Ptr = **(void***)(cvr.Address);
			FindVar = (_FindVar)GetVirtualFunctionByIndex(Ptr, Offsets::FindVar);
			ConCommandList = (ConCommandBase*)((uintptr_t)Ptr + Offsets::m_pConCommandList);
		}
		return cvr.Found;
	}
	ConVar FindCvar(const char* ref)
	{
		return ConVar(Cvar::FindVar(Cvar::Ptr, ref));
	}
}