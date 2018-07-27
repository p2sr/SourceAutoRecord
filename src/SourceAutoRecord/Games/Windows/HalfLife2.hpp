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

    // engine.dll

    // \x55\x8B\xEC\x8B\x45\x0C\x8B\x55\x18\x85\xD2\x56\x8B\xF1\x0F\x95\xC1\x89\x46\x18\xB8\x00\x00\x00\x00\x0F\x45\xC2\xC7\x46\x00\x00\x00\x00\x00\x89\x46\x1C\x80\xE1\x01\x8A\x46\x20\x24\xFA xxxxxxxxxxxxxxxxxxxxx????xxxxx?????xxxxxxxxxxx
    Add("ConCommandCtor", "Half-Life 2 Build 2257546",
        "ConCommand::ConCommand",
        "55 8B EC 8B 45 0C 8B 55 18 85 D2 56 8B F1 0F 95 C1 89 46 18 B8 ? ? ? ? 0F 45 C2 C7 46 ? ? ? ? ? 89 46 1C 80 E1 01 8A 46 20 24 FA");

    // \x55\x8B\xEC\xD9\x45\x24\x56\x6A\x00 xxxxxxxxx
    Add("ConVarCtor", "Half-Life 2 Build 2257546",
        "ConVar::ConVar",
        "55 8B EC D9 45 24 56 6A 00");

    // \x55\x89\xE5\x57\x56\x53\x83\xEC\x00\x8B\x5D\x00\x89\x1C\x24\xE8\x00\x00\x00\x00\xC7\x04\x24\x00\x00\x00\x00 xxxxxxxx?xx?xxxx????xxx????
    Add("m_bLoadgame", "Half-Life 2 Build 2257546",
        "CGameClient::ActivatePlayer",
        "55 8B EC 51 53 57 8B D9",
        31);

    // \x55\x8B\xEC\x53\x8B\x5D\x08\x83\xFB\xFF\x0F\x84\x00\x00\x00\x00 xxxxxxxxxxxx????
    Add("Key_SetBinding", "Half-Life 2 Build 2257546",
        "Key_SetBinding",
        "55 8B EC 53 8B 5D 08 83 FB FF 0F 84 ? ? ? ? ");

    // \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x56\x8B\xF1\x57\x8B\x7D\x08 xxxxx????xxxxxxxx
    Add("AutoCompletionFunc", "Half-Life 2 Build 2257546",
        "CBaseAutoCompleteFileList::AutoCompletionFunc",
        "55 8B EC 81 EC ? ? ? ? 53 56 8B F1 57 8B 7D 08");

    // \x55\x8B\xEC\xD9\x45\x08\x51\xB9\x00\x00\x00\x00\xD9\x1C\x24\xE8\x00\x00\x00\x00\x5D\xC3 xxxxxxxx????xxxx????xx
    Add("HostState_Frame", "Half-Life 2 Build 2257546",
        "HostState_Frame",
        "55 8B EC D9 45 08 51 B9 ? ? ? ? D9 1C 24 E8 ? ? ? ? 5D C3");

    // vguimatsurface.dll

    // \x55\x8B\xEC\x64\xA1\x00\x00\x00\x00\x6A\xFF\x68\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00\x83\xEC\x14 xxxxx????xxx????xxxx????xxx
    Add("StartDrawing", "Half-Life 2 Build 2257546",
        "CMatSystemSurface::StartDrawing",
        "55 8B EC 64 A1 ? ? ? ? 6A FF 68 ? ? ? ? 50 64 89 25 ? ? ? ? 83 EC 14");

    // \x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00\x51\x56\x6A\x00 xxxxxx????xx????xxxx????xxxx
    Add("FinishDrawing", "Half-Life 2 Build 2257546",
        "CMatSystemSurface::FinishDrawing",
        "55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 64 89 25 ? ? ? ? 51 56 6A 00");

    // server.dll

    // \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x8B\xC1 xxxxx????xxx
    Add("FireOutput", "Half-Life 2 Build 2257546",
        "CBaseEntityOutput::FireOutput",
        "55 8B EC 81 EC ? ? ? ? 53 8B C1");
}
void Offsets()
{
    // engine.dll

    InternalSetValue = 10; // ConVar
    InternalSetFloatValue = 11; // ConVar
    InternalSetIntValue = 12; // ConVar
    GetScreenSize = 5; // CEngineClient
    ClientCmd = 7; // CEngineClient
    GetLocalPlayer = 12; // CEngineClient
    GetViewAngles = 19; // CEngineClient
    SetViewAngles = 20; // CEngineClient
    GetGameDirectory = 35; // CEngineClient
    ServerCmdKeyValues = 127; // CEngineClient
    cl = 4; // CEngineClient::ServerCmdKeyValues
    StringToButtonCode = 29; // ReadCheatCommandsFromFile
    GetRecordingTick = 1; // CDemoRecorder
    SetSignonState = 3; // CDemoRecorder
    StopRecording = 7; // CDemoRecorder
    m_szDemoBaseName = 1348; // CDemoRecorder::StartupDemoFile
    m_nDemoNumber = 1612; // CDemoRecorder::StartupDemoFile
    m_bRecording = 1610; // CDemoRecorder::SetSignonState
    GetPlaybackTick = 3; // CDemoPlayer
    StartPlayback = 5; // CDemoPlayer
    IsPlayingBack = 6; // CDemoPlayer
    m_szFileName = 4; // CDemoPlayer::SkipToTick
    Paint = 13; // CEngineVGui
    ProcessTick = 1; // CClientState/IServerMessageHandler
    tickcount = 87; // CClientState::ProcessTick
    interval_per_tick = 65; // CClientState::ProcessTick
    Disconnect = 14; //  CClientState
    demoplayer = 110; // CClientState::Disconnect
    demorecorder = 121; // CClientState::Disconnect
    GetCurrentMap = 23; // CEngineTool
    m_szLevelName = 32; // CEngineTool::GetCurrentMap
    hoststate = 8; // HostState_Frame
    FrameUpdate = 16; // HostState_Frame
    eng = 281; // CHostState::FrameUpdate
    Frame = 5; // CEngine
    Cbuf_AddText = 58; // CEngine::ClientCmd
    s_CommandBuffer = 64; // Cbuf_AddText
    AddText = 69; // Cbuf_AddText

    // vstdlib.dll
    UnregisterConCommand = 7; // CCvar
    FindCommandBase = 10; // CCvar

    // vgui2.dll

    GetIScheme = 8; // CSchemeManager
    GetFont = 3; // CScheme

    // server.dll

    PlayerMove = 13; // CGameMovement
    CheckJumpButton = 29; // CGameMovement
    FullTossMove = 30; // CGameMovement
    mv = 8; // CGameMovement::CheckJumpButton
    m_nOldButtons = 40; // CGameMovement::CheckJumpButton
    gpGlobals = 888; // CGameMovement::FullTossMove
    player = 4; // CGameMovement::CheckJumpButton
    m_vecVelocity2 = 64; // CGameMovement::PlayerMove
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 38; // CServerGameDLL::Think
    iNumPortalsPlaced = 4816; // CPortal_Player::IncrementPortalsPlaced
    m_fFlags = 260; // CBasePlayer::UpdateStepSound
    m_MoveType = 314; // CBasePlayer::UpdateStepSound
    m_nWaterLevel = 503; // CBasePlayer::UpdateStepSound
    frametime = 16; // CBasePlayer::UpdateStepSound
    m_iClassname = 92; // CBaseEntityOutput::FireOutput
    m_iName = 264; // CBaseEntityOutput::FireOutput
    m_ActionList = 20; // CBaseEntityOutput::FireOutput

    // client.dll

    HudProcessInput = 10; // CHLClient
    HudUpdate = 11; // CHLClient
    m_vecVelocity = 240; // CFPSPanel::Paint
    m_vecAbsOrigin = 604; // C_BasePlayer::GetAbsOrigin
    m_angAbsRotation = 616; // C_BasePlayer::GetAbsAngles
    GetClientEntity = 3; // CClientEntityList
    GetClientMode = 5; // CHLClient::HudProcessInput
    CreateMove = 21; // ClientModeShared

    // vguimatsurface.dll

    DrawSetColor = 11; // CMatSystemSurface
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
