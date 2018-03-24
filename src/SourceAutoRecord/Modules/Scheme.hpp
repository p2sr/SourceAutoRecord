#pragma once
#include "vmthook/vmthook.h"

#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace Scheme
{
	using _GetIScheme = void*(__cdecl*)(void* thisptr, unsigned long scheme);
	using _GetFont = unsigned long(__cdecl*)(void* thisptr, const char* scheme);

	std::unique_ptr<VMTHook> g_pVGuiSchemeManager;
	_GetIScheme GetIScheme;
	
	std::unique_ptr<VMTHook> g_pScheme;
	_GetFont GetFont;

	unsigned long GetDefaultFont()
	{
		return GetFont(g_pScheme->GetThisPtr(), "DefaultFixedOutline");
	}

	void Hook()
	{
		if (Interfaces::ISchemeManager) {
			g_pVGuiSchemeManager = std::make_unique<VMTHook>(Interfaces::ISchemeManager);
			GetIScheme = g_pVGuiSchemeManager->GetOriginalFunction<_GetIScheme>(Offsets::GetIScheme);
			
			// Default scheme is 1
			g_pScheme = std::make_unique<VMTHook>(GetIScheme(g_pVGuiSchemeManager->GetThisPtr(), 1));
			GetFont = g_pScheme->GetOriginalFunction<_GetFont>(Offsets::GetFont);
		}
	}
}