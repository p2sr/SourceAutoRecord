#pragma once
#include "Offsets.hpp"
#include "Patterns.hpp"

#include "Portal.hpp"

namespace HalfLife2 {

using namespace Offsets;
using namespace Patterns;

void Patterns()
{
    Portal::Patterns();

    // engine.so

    Inherit("ConCommand_Ctor1", "Half-Life 2 Build 2257546", "ConCommand::ConCommand");
    Inherit("ConCommand_Ctor2", "Half-Life 2 Build 2257546", "ConCommand::ConCommand");
    Inherit("ConVar_Ctor3", "Half-Life 2 Build 2257546", "ConVar::ConVar");
    Inherit("m_bLoadgame", "Half-Life 2 Build 2257546", "CGameClient::ActivatePlayer");
    Inherit("Key_SetBinding", "Half-Life 2 Build 2257546", "Key_SetBinding");
    Inherit("AutoCompletionFunc", "Half-Life 2 Build 2257546", "CBaseAutoCompleteFileList::AutoCompletionFunc");

    // vguimatsurface.so

    Inherit("StartDrawing", "Half-Life 2 Build 2257546", "CMatSystemSurface::StartDrawing");
    Inherit("FinishDrawing", "Half-Life 2 Build 2257546", "CMatSystemSurface::FinishDrawing");
}
void Offsets()
{
    Portal::Offsets();
}
}