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

    // \x55\x8B\xEC\xF3\x0F\x10\x45\x00\x51\xB9\x00\x00\x00\x00\xF3\x0F\x11\x04\x24\xE8\x00\x00\x00\x00\x5D\xC3 xxxxxxx?xx????xxxxxx????xx
    Add("HostState_Frame", "The Stanley Parable Build 5454",
        "HostState_Frame",
        "55 8B EC F3 0F 10 45 ? 51 B9 ? ? ? ? F3 0F 11 04 24 E8 ? ? ? ? 5D C3");

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
    hoststate = 10; // HostState_Frame
    FrameUpdate = 20; // HostState_Frame
    eng = 284; // CHostState::FrameUpdate

    // client.dll

    IN_ActivateMouse = 15; // CHLClient
    g_Input = 2; // CHLClient::IN_ActivateMouse
    GetButtonBits = 2; //CInput
    JoyStickApplyMovement = 60; // CInput
    in_jump = 420; // CInput::GetButtonBits
    KeyDown = 255; // CInput::JoyStickApplyMovement
    KeyUp = 234; // CInput::JoyStickApplyMovement

    // vguimatsurface.dll
    DrawColoredText = 159; // CMatSystemSurface
    DrawTextLen = 162; // CMatSystemSurface
}
void Rules()
{
    /* Speedrun::TimerRules rules;

    rules.push_back(Speedrun::TimerRule("TODO", "TODO", Speedrun::TimerAction::Start));
    rules.push_back(Speedrun::TimerRule("TODO", "TODO", Speedrun::TimerAction::End));

    Speedrun::timer->LoadRules(rules); */
}
}
