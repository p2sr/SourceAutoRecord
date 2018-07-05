#pragma once
#include "Offsets.hpp"
#include "Patterns.hpp"

#include "Games/Portal2.hpp"

namespace TheStanleyParable {

using namespace Offsets;
using namespace Patterns;

void Patterns()
{
    Portal2::Patterns();

    // engine.so

    Inherit("ConCommandCtor", "The Stanley Parable Build 6130", "ConCommand::ConCommand");

    // \x55\x89\xE5\x56\x53\x83\xEC\x00\x8B\x45\x00\x8B\x5D\x00\x8B\x55\x00\xF3\x0F\x10\x45\x00\x0F\xB6\xC0\xC6\x43\x00\x00 xxxxxxx?xx?xx?xx?xxxx?xxxxx??
    Add("ConVarCtor", "The Stanley Parable Build 6130",
        "ConVar::ConVar",
        "55 89 E5 56 53 83 EC ? 8B 45 ? 8B 5D ? 8B 55 ? F3 0F 10 45 ? 0F B6 C0 C6 43 ? ? ");

    // \x55\x89\xE5\x57\x56\x53\x83\xEC\x00\x8B\x5D\x00\x89\x1C\x24\xE8\x00\x00\x00\x00\xC7\x04\x24\x00\x00\x00\x00\xE8\x00\x00\x00\x00 xxxxxxxx?xx?xxxx????xxx????x????
    Add("m_bLoadgame", "The Stanley Parable Build 6130",
        "CGameClient::ActivatePlayer",
        "55 89 E5 57 56 53 83 EC ? 8B 5D ? 89 1C 24 E8 ? ? ? ? C7 04 24 ? ? ? ? E8 ? ? ? ? ",
        34);

    // \x55\x89\xE5\x57\x89\xD7\x56\x53\x83\xEC\x00\x89\x04\x24 xxxxxxxxxx?xxx
    Add("Key_SetBinding", "The Stanley Parable Build 6130",
        "Key_SetBinding",
        "55 89 E5 57 89 D7 56 53 83 EC ? 89 04 24");

    // \x55\x89\xE5\x57\x56\x53\x81\xEC\x00\x00\x00\x00\x8B\x45\x00\x8B\x5D\x00\x8B\x75\x00\x89\x85\x00\x00\x00\x00\x65\xA1\x00\x00\x00\x00\x89\x45\x00\x31\xC0\x8B\x03\x89\x34\x24 xxxxxxxx????xx?xx?xx?xx????xx????xx?xxxxxxx
    Add("AutoCompletionFunc", "The Stanley Parable Build 6130",
        "CBaseAutoCompleteFileList::AutoCompletionFunc",
        "55 89 E5 57 56 53 81 EC ? ? ? ? 8B 45 ? 8B 5D ? 8B 75 ? 89 85 ? ? ? ? 65 A1 ? ? ? ? 89 45 ? 31 C0 8B 03 89 34 24");

    // vguimatsurface.so

    Inherit("StartDrawing", "The Stanley Parable Build 6130", "CMatSystemSurface::StartDrawing");
    Inherit("FinishDrawing", "The Stanley Parable Build 6130", "CMatSystemSurface::FinishDrawing");
}
void Offsets()
{
    Portal2::Offsets();

    // engine.so
    tickcount = 74; // CClientState::ProcessTick
    interval_per_tick = 82; // CClientState::ProcessTick
    m_szLevelName = 56; // CEngineTool::GetCurrentMap
    demoplayer = 92; // CClientState::Disconnect
    demorecorder = 105; // CClientState::Disconnect

    // server.so

    iNumPortalsPlaced = 5728; // CPortal_Player::IncrementPortalsPlaced
    gpGlobals = 576; // CGameMovement::FullTossMove
    m_pSurfaceData = 4120; // CPortalGameMovement::PlayerMove

    // client.so

    IN_ActivateMouse = 15; // CHLClient
    g_Input = 1; // CHLClient::IN_ActivateMouse
    GetButtonBits = 2; // CInput
    JoyStickApplyMovement = 64; // CInput
    in_jump = 210; // CInput::GetButtonBits
    KeyDown = 337; // CInput::JoyStickApplyMovement
    KeyUp = 384; // CInput::JoyStickApplyMovement
}
}