#pragma once
#include "vmthook/vmthook.h"

#include "ConVar.hpp"

#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

namespace Cvar
{
	using _FindVar = void*(__cdecl*)(void* thisptr, const char* name);
	_FindVar FindVar;

	std::unique_ptr<VMTHook> g_pCVar;

	bool Loaded()
	{
		if (Interfaces::ICVar) {
			g_pCVar = std::make_unique<VMTHook>(Interfaces::ICVar);
			FindVar = g_pCVar->GetOriginalFunction<_FindVar>(Offsets::FindVar);
		}
		return Interfaces::ICVar != nullptr;
	}
	ConVar GetConVar(const char* var_name)
	{
		return ConVar(FindVar(g_pCVar->GetThisPtr(), var_name));
	}
}