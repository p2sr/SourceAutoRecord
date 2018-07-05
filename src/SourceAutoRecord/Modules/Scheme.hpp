#pragma once
#include "Game.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace Scheme {

VMT g_pScheme;

using _GetFont = unsigned long(__func*)(void* thisptr, const char* fontName, bool proportional);
_GetFont GetFont;

unsigned long GetDefaultFont()
{
    return GetFont(g_pScheme->GetThisPtr(), "DefaultFixedOutline", 0);
}

void Hook()
{
    if (Interfaces::ISchemeManager) {
        auto g_pVGuiSchemeManager = std::make_unique<VMTHook>(Interfaces::ISchemeManager);
        using _GetIScheme = void*(__func*)(void* thisptr, unsigned long scheme);
        auto GetIScheme = g_pVGuiSchemeManager->GetOriginalFunction<_GetIScheme>(Offsets::GetIScheme);

        // Default scheme is 1
        if (SAR::NewVMT(GetIScheme(g_pVGuiSchemeManager->GetThisPtr(), 1), g_pScheme)) {
            GetFont = g_pScheme->GetOriginalFunction<_GetFont>(Offsets::GetFont);
        }
    }
}
void Unhook()
{
    SAR::DeleteVMT(g_pScheme);
}
}