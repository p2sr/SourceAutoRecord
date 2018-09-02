#include "Scheme.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

unsigned long Scheme::GetDefaultFont()
{
    return this->GetFont(this->g_pScheme->ThisPtr(), "DefaultFixedOutline", 0);
}
bool Scheme::Init()
{
    auto g_pVGuiSchemeManager = Interface::Create(this->Name(), "VGUI_Scheme0", false);
    if (g_pVGuiSchemeManager) {
        using _GetIScheme = void*(__func*)(void* thisptr, unsigned long scheme);
        auto GetIScheme = g_pVGuiSchemeManager->Original<_GetIScheme>(Offsets::GetIScheme);

        // Default scheme is 1
        this->g_pScheme = Interface::Create(GetIScheme(g_pVGuiSchemeManager->ThisPtr(), 1));
        if (this->g_pScheme) {
            this->GetFont = this->g_pScheme->Original<_GetFont>(Offsets::GetFont);
        }
    }

    return this->hasLoaded = this->g_pScheme;
}
void Scheme::Shutdown()
{
    Interface::Delete(this->g_pScheme);
}

Scheme* scheme;
