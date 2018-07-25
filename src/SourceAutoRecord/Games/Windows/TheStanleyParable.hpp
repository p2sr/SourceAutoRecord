#pragma once
#include "Features/Speedrun.hpp"

#include "Offsets.hpp"
#include "Patterns.hpp"

#include "Portal2.hpp"

namespace TheStanleyParable {

using namespace Offsets;
using namespace Patterns;

void Patterns()
{
    Portal2::Patterns();

    // engine.dll

    Inherit("ConCommandCtor", "The Stanley Parable Build 5454", "ConCommand::ConCommand");
    Inherit("ConVarCtor", "The Stanley Parable Build 5454", "ConVar::ConVar");
    Inherit("m_bLoadgame", "The Stanley ParableBuild 5454", "CGameClient::ActivatePlayer");

    // \x55\x8B\xEC\x57\x8B\x7D\x08\x83\xFF\xFF xxxxxxxxxx
    Add("Key_SetBinding", "The Stanley Parable Build 5454",
        "Key_SetBinding",
        "55 8B EC 57 8B 7D 08 83 FF FF");

    Inherit("AutoCompletionFunc", "The Stanley Parable Build 5454", "CBaseAutoCompleteFileList::AutoCompletionFunc");

    // vguimatsurface.dll

    Inherit("StartDrawing", "The Stanley Parable Build 5454", "CMatSystemSurface::StartDrawing");
    Inherit("FinishDrawing", "The Stanley Parable Build 5454", "CMatSystemSurface::FinishDrawing");
}
void Offsets()
{
    Portal2::Offsets();

    // engine.dll

    tickcount = 103; // CClientState::ProcessTick
    interval_per_tick = 73; // CClientState::ProcessTick
    Disconnect = 16; //  CClientState
    m_szLevelName = 36; // CEngineTool::GetCurrentMap

    // client.dll

    DrawColoredText = 159; // CFPSPanel::Paint
    GetButtonBits = 2;
    IN_ActivateMouse = 15;
    g_Input = 2; // CHLClient::IN_ActivateMouse
    JoyStickApplyMovement = 60; // CInput
    in_jump = 420; // CInput::GetButtonBits
    KeyDown = 255; // CInput::JoyStickApplyMovement
    KeyUp = 234; // CInput::JoyStickApplyMovement
}
void Rules()
{
    /* Speedrun::TimerRules rules;

    rules.push_back(Speedrun::TimerRule("TODO", "TODO", Speedrun::TimerAction::Start));
    rules.push_back(Speedrun::TimerRule("TODO", "TODO", Speedrun::TimerAction::End));

    Speedrun::timer->LoadRules(rules); */
}
}