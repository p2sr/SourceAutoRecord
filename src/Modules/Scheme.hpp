#pragma once
#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

// TODO: Custom fonts
namespace Scheme {

Interface* g_pScheme;

using _GetFont = unsigned long(__func*)(void* thisptr, const char* fontName, bool proportional);
_GetFont GetFont;

unsigned long GetDefaultFont()
{
    return GetFont(g_pScheme->ThisPtr(), "DefaultFixedOutline", 0);
}

void Init()
{
    auto g_pVGuiSchemeManager = Interface::Create(MODULE("vgui2"), "VGUI_Scheme0", false);
    if (g_pVGuiSchemeManager) {
        using _GetIScheme = void*(__func*)(void* thisptr, unsigned long scheme);
        auto GetIScheme = g_pVGuiSchemeManager->Original<_GetIScheme>(Offsets::GetIScheme);

        // Default scheme is 1
        g_pScheme = Interface::Create(GetIScheme(g_pVGuiSchemeManager->ThisPtr(), 1));
        if (g_pScheme) {
            GetFont = g_pScheme->Original<_GetFont>(Offsets::GetFont);
        }
    }
}
void Shutdown()
{
    Interface::Delete(g_pScheme);
}
}
