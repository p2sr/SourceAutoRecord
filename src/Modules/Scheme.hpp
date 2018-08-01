#pragma once
#include "Game.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

// TODO: Custom fonts
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
    VMT g_pVGuiSchemeManager;
    COPY_VMT(Interfaces::ISchemeManager, g_pVGuiSchemeManager)
    {
        using _GetIScheme = void*(__func*)(void* thisptr, unsigned long scheme);
        auto GetIScheme = g_pVGuiSchemeManager->GetOriginal<_GetIScheme>(Offsets::GetIScheme);

        // Default scheme is 1
        CREATE_VMT(GetIScheme(g_pVGuiSchemeManager->GetThisPtr(), 1), g_pScheme)
        {
            GetFont = g_pScheme->GetOriginal<_GetFont>(Offsets::GetFont);
        }
    }
}
void Unhook()
{
    DELETE_VMT(g_pScheme);
}
}