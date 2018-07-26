#pragma once
#include "Features/Speedrun.hpp"

#include "Offsets.hpp"
#include "Patterns.hpp"

namespace HalfLife2 {

using namespace Offsets;
using namespace Patterns;

void Patterns()
{
    Init();

    // engine.so

    // \x55\xB9\x00\x00\x00\x00\x89\xE5\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x45\x00\x8B\x55\x00\xC6\x43\x00\x00\x89\x43\x00\x0F\xB6\x43\x00\xC7\x43\x00\x00\x00\x00\x00 xx????xxxxx?xx?xx?xx?xx??xx?xxx?xx?????
    Add("ConCommandCtor", "Half-Life 2 Build 2257546",
        "ConCommand::ConCommand",
        "55 B9 ? ? ? ? 89 E5 53 83 EC ? 8B 5D ? 8B 45 ? 8B 55 ? C6 43 ? ? 89 43 ? 0F B6 43 ? C7 43 ? ? ? ? ? ");

    // \x55\x89\xE5\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x45\x00\xC6\x43\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x03\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\x89\x44\x24\x00\x0F\xB6\x45\x00 xxxxxx?xx?xx?xx??xx?????xx?????xx?????xx?????xx????xx?????xxx?xxx?
    Add("ConVarCtor", "Half-Life 2 Build 2257546",
        "ConVar::ConVar",
        "55 89 E5 53 83 EC ? 8B 5D ? 8B 45 ? C6 43 ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? C7 03 ? ? ? ? C7 43 ? ? ? ? ? 89 44 24 ? 0F B6 45 ? ");

    // \x55\x89\xE5\x57\x56\x53\x83\xEC\x00\x8B\x5D\x00\x89\x1C\x24\xE8\x00\x00\x00\x00\xC7\x04\x24\x00\x00\x00\x00 xxxxxxxx?xx?xxxx????xxx????
    Add("m_bLoadgame", "Half-Life 2 Build 2257546",
        "CGameClient::ActivatePlayer",
        "55 89 E5 57 56 53 83 EC ? 8B 5D ? 89 1C 24 E8 ? ? ? ? C7 04 24 ? ? ? ? ",
        34);

    // \x55\x89\xE5\x83\xEC\x00\x89\x5D\x00\x8B\x5D\x00\x89\x75\x00\x8B\x75\x00\x89\x7D\x00\x83\xFB\x00\x74\x00 xxxxx?xx?xx?xx?xx?xx?xx?x?
    Add("Key_SetBinding", "Half-Life 2 Build 2257546",
        "Key_SetBinding",
        "55 89 E5 83 EC ? 89 5D ? 8B 5D ? 89 75 ? 8B 75 ? 89 7D ? 83 FB ? 74 ? ");

    // \x55\x89\xE5\x57\x56\x53\x81\xEC\x00\x00\x00\x00\x8B\x5D\x00\x8B\x75\x00\x8B\x03\x89\x34\x24 xxxxxxxx????xx?xx?xxxxx
    Add("AutoCompletionFunc", "Half-Life 2 Build 2257546",
        "CBaseAutoCompleteFileList::AutoCompletionFunc",
        "55 89 E5 57 56 53 81 EC ? ? ? ? 8B 5D ? 8B 75 ? 8B 03 89 34 24");

    // \x55\x89\xE5\x83\xEC\x00\x8B\x45\x00\xC7\x04\x24\x00\x00\x00\x00\x89\x44\x24\x00\xE8\x00\x00\x00\x00\xC9\xC3\x00\x00\x00\x00\x00\x55\xB8\x00\x00\x00\x00\x89\xE5\x8B\x55\x00\x83\x3A\x00\x74\x00\x83\x7A\x00\x00\x0F\x94\xC0 xxxxx?xx?xxx????xxx?x????xx?????xx????xxxx?xx?x?xx??xxx
    Add("HostState_Frame", "Half-Life 2 Build 2257546",
        "HostState_Frame",
        "55 89 E5 83 EC ? 8B 45 ? C7 04 24 ? ? ? ? 89 44 24 ? E8 ? ? ? ? C9 C3 ? ? ? ? ? 55 B8 ? ? ? ? 89 E5 8B 55 ? 83 3A ? 74 ? 83 7A ? ? 0F 94 C0");

    // vguimatsurface.so

    // \x55\x89\xE5\x53\x83\xEC\x00\x80\x3D\x00\x00\x00\x00\x00\x8B\x5D\x00\x0F\x84\x00\x00\x00\x00 xxxxxx?xx?????xx?xx????
    Add("StartDrawing", "Half-Life 2 Build 2257546",
        "CMatSystemSurface::StartDrawing",
        "55 89 E5 53 83 EC ? 80 3D ? ? ? ? ? 8B 5D ? 0F 84 ? ? ? ? ");

    // \x55\x89\xE5\x53\x83\xEC\x00\xC7\x04\x24\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xA1\x00\x00\x00\x00 xxxxxx?xxx????x????x????
    Add("FinishDrawing", "Half-Life 2 Build 2257546",
        "CMatSystemSurface::FinishDrawing",
        "55 89 E5 53 83 EC ? C7 04 24 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? ");
}
void Offsets()
{
    // engine.so

    InternalSetValue = 14; // ConVar
    InternalSetFloatValue = 15; // ConVar
    InternalSetIntValue = 16; // ConVar
    GetScreenSize = 5; // CEngineClient
    ClientCmd = 7; // CEngineClient
    GetLocalPlayer = 12; // CEngineClient
    GetViewAngles = 19; // CEngineClient
    SetViewAngles = 20; // CEngineClient
    GetGameDirectory = 35; // CEngineClient
    ServerCmdKeyValues = 128; // CEngineClient
    cl = 6; // CEngineClient::ServerCmdKeyValues
    StringToButtonCode = 29; // ReadCheatCommandsFromFile
    GetRecordingTick = 1; // CDemoRecorder
    SetSignonState = 3; // CDemoRecorder
    StopRecording = 7; // CDemoRecorder
    m_szDemoBaseName = 1348; // CDemoRecorder::StartupDemoFile
    m_nDemoNumber = 1612; // CDemoRecorder::StartupDemoFile
    m_bRecording = 1610; // CDemoRecorder::SetSignonState
    GetPlaybackTick = 3; // CDemoPlayer
    StartPlayback = 6; // CDemoPlayer
    IsPlayingBack = 7; // CDemoPlayer
    m_szFileName = 4; // CDemoPlayer::SkipToTick
    Paint = 14; // CEngineVGui
    ProcessTick = 12; // CClientState
    tickcount = 76; // CClientState::ProcessTick
    interval_per_tick = 84; // CClientState::ProcessTick
    Disconnect = 34; //  CClientState
    demoplayer = 151; // CClientState::Disconnect
    demorecorder = 164; // CClientState::Disconnect
    GetCurrentMap = 24; // CEngineTool
    m_szLevelName = 25; // CEngineTool::GetCurrentMap
    hoststate = 12; // HostState_Frame
    FrameUpdate = 21; // HostState_Frame
    State_Shutdown = 111; // CHostState::FrameUpdate
    eng = 17; // CHostState::FrameUpdate
    Frame = 6; // CEngine
    Cbuf_AddText = 58; // CEngine::ClientCmd (TODO)
    s_CommandBuffer = 64; // Cbuf_AddText (TODO)
    AddText = 69; // Cbuf_AddText (TODO)

    // libvstdlib.so
    UnregisterConCommand = 7; // CCvar
    FindCommandBase = 10; // CCvar

    // vgui2.so

    GetIScheme = 9; // CSchemeManager
    GetFont = 4; // CScheme

    // server.so

    PlayerMove = 12; // CGameMovement
    CheckJumpButton = 30; // CGameMovement
    FullTossMove = 31; // CGameMovement
    mv = 8; // CPortalGameMovement::CheckJumpButton
    m_nOldButtons = 40; // CPortalGameMovement::CheckJumpButton
    gpGlobals = 705; // CGameMovement::FullTossMove
    player = 4; // CPortalGameMovement::PlayerMove
    m_vecVelocity2 = 64; // CPortalGameMovement::PlayerMove
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 61; // CServerGameDLL::Think
    iNumPortalsPlaced = 4816; // CPortal_Player::IncrementPortalsPlaced
    m_fFlags = 280; // CBasePlayer::UpdateStepSound
    m_MoveType = 334; // CBasePlayer::UpdateStepSound
    m_nWaterLevel = 523; // CBasePlayer::UpdateStepSound
    frametime = 16; // CBasePlayer::UpdateStepSound

    // client.so

    HudProcessInput = 10; // CHLClient
    HudUpdate = 11; // CHLClient
    m_vecVelocity = 224; // CFPSPanel::Paint
    m_vecAbsOrigin = 588; // C_BasePlayer::GetAbsOrigin
    m_angAbsRotation = 600; // C_BasePlayer::GetAbsAngles
    GetClientEntity = 3; // CClientEntityList
    GetClientMode = 1; // CHLClient::HudProcessInput (TODO)
    CreateMove = 22; // ClientModeShared

    // vguimatsurface.so

    DrawSetColor = 10; // CMatSystemSurface
    DrawFilledRect = 12; // CMatSystemSurface
    GetFontTall = 69; // CMatSystemSurface
    DrawColoredText = 162; // CMatSystemSurface
    DrawTextLen = 165; // CMatSystemSurface
}
void Rules()
{
    /* Speedrun::TimerRules rules;

    rules.push_back(Speedrun::TimerRule("TODO", "TODO", Speedrun::TimerAction::Start));
    rules.push_back(Speedrun::TimerRule("TODO", "TODO", Speedrun::TimerAction::End));

    Speedrun::timer->LoadRules(rules); */
}
}
