#pragma once
#include "Offsets.hpp"
#include "Patterns.hpp"

#include "TheStanleyParable.hpp"

namespace TheBeginnersGuide {

using namespace Offsets;
using namespace Patterns;

void Patterns()
{
    TheStanleyParable::Patterns();

    // engine.so

    Inherit("ConCommandCtor", "The Beginners Guide Build 6172", "ConCommand::ConCommand");
    Inherit("ConVarCtor", "The Beginners Guide Build 6172", "ConVar::ConVar");
    Inherit("m_bLoadgame", "The Beginners Guide Build 6172", "CGameClient::ActivatePlayer");
    Inherit("Key_SetBinding", "The Beginners Guide Build 6172", "Key_SetBinding");
    Inherit("AutoCompletionFunc", "The Beginners Guide Build 6172", "CBaseAutoCompleteFileList::AutoCompletionFunc");

    // vguimatsurface.so

    Inherit("StartDrawing", "The Beginners Guide Build 6172", "CMatSystemSurface::StartDrawing");
    Inherit("FinishDrawing", "The Beginners Guide Build 6172", "CMatSystemSurface::FinishDrawing");
}
void Offsets()
{
    TheStanleyParable::Offsets();
}
}