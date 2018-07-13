#pragma once
#include "Offsets.hpp"
#include "Patterns.hpp"

namespace Portal2 {

using namespace Offsets;
using namespace Patterns;

void Patterns()
{
    Init();
    Create(MODULE("client"), "FindElement");

    // engine.so

    // \x55\xB9\x00\x00\x00\x00\x89\xE5\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x45\x00\x8B\x55\x00\xC6\x43\x00\x00\x89\x43\x00\x0F\xB6\x43\x00\xC7\x43\x00\x00\x00\x00\x00 xx????xxxxx?xx?xx?xx?xx??xx?xxx?xx?????
    Add("ConCommandCtor", "Portal 2 Build 7054",
        "ConCommand::ConCommand",
        "55 B9 ? ? ? ? 89 E5 53 83 EC ? 8B 5D ? 8B 45 ? 8B 55 ? C6 43 ? ? 89 43 ? 0F B6 43 ? C7 43 ? ? ? ? ? ");

    // \x55\x89\xE5\x53\x83\xEC\x00\x8B\x5D\x00\x8B\x45\x00\xC6\x43\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x03\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00\xC7\x43\x00\x00\x00\x00\x00 xxxxxx?xx?xx?xx??xx?????xx?????xx?????xx?????xx????xx?????xx?????
    Add("ConVarCtor", "Portal 2 Build 7054",
        "ConVar::ConVar",
        "55 89 E5 53 83 EC ? 8B 5D ? 8B 45 ? C6 43 ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? C7 03 ? ? ? ? C7 43 ? ? ? ? ? C7 43 ? ? ? ? ? ");

    // \x55\x89\xE5\x57\x56\x53\x83\xEC\x00\x8B\x5D\x00\x89\x1C\x24\xE8\x00\x00\x00\x00\xC7\x04\x24\x00\x00\x00\x00\xE8\x00\x00\x00\x00 xxxxxxxx?xx?xxxx????xxx????x????
    Add("m_bLoadgame", "Portal 2 Build 7054",
        "CGameClient::ActivatePlayer",
        "55 89 E5 57 56 53 83 EC ? 8B 5D ? 89 1C 24 E8 ? ? ? ? C7 04 24 ? ? ? ? E8 ? ? ? ? ",
        34);

    // \x55\x89\xE5\x83\xEC\x00\x8B\x45\x00\x89\x75\x00\x89\x5D\x00\x8B\x75\x00\x89\x7D\x00\x83\xF8\x00\x0F\x84\x00\x00\x00\x00 xxxxx?xx?xx?xx?xx?xx?xx?xx????
    Add("Key_SetBinding", "Portal 2 Build 7054",
        "Key_SetBinding",
        "55 89 E5 83 EC ? 8B 45 ? 89 75 ? 89 5D ? 8B 75 ? 89 7D ? 83 F8 ? 0F 84 ? ? ? ? ");

    // \x55\x89\xE5\x57\x56\x53\x81\xEC\x00\x00\x00\x00\x8B\x5D\x00\x8B\x75\x00\x8B\x03\x89\x34\x24\x89\x85\x00\x00\x00\x00\x89\x44\x24\x00\xE8\x00\x00\x00\x00\x85\xC0 xxxxxxxx????xx?xx?xxxxxxx????xxx?x????xx
    Add("AutoCompletionFunc", "Portal 2 Build 7054",
        "CBaseAutoCompleteFileList::AutoCompletionFunc",
        "55 89 E5 57 56 53 81 EC ? ? ? ? 8B 5D ? 8B 75 ? 8B 03 89 34 24 89 85 ? ? ? ? 89 44 24 ? E8 ? ? ? ? 85 C0");

    // vguimatsurface.so

    // \x55\x89\xE5\x53\x83\xEC\x00\x80\x3D\x00\x00\x00\x00\x00\x8B\x5D\x00\x0F\x84\x00\x00\x00\x00 xxxxxx?xx?????xx?xx????
    Add("StartDrawing", "Portal 2 Build 7054",
        "CMatSystemSurface::StartDrawing",
        "55 89 E5 53 83 EC ? 80 3D ? ? ? ? ? 8B 5D ? 0F 84 ? ? ? ? ");

    // \x55\x89\xE5\x53\x83\xEC\x00\xC7\x04\x24\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xA1\x00\x00\x00\x00 xxxxxx?xxx????x????x????
    Add("FinishDrawing", "Portal 2 Build 7054",
        "CMatSystemSurface::FinishDrawing",
        "55 89 E5 53 83 EC ? C7 04 24 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? ");

    // client.so

    // \x55\x89\xE5\x57\x56\x53\x83\xEC\x00\x8B\x75\x00\x8B\x7D\x00\x8B\x46\x00\x85\xC0\x7E\x00\x31\xDB\xEB\x00\x8D\xB6\x00\x00\x00\x00\x83\xC3\x00\x39\x5E\x00\x7E\x00\x8B\x46\x00 xxxxxxxx?xx?xx?xx?xxx?xxx?xxxxxxxx?xx?x?xx?
    Add("FindElement", "Portal 2 Build 7054",
        "CHud::FindElement",
        "55 89 E5 57 56 53 83 EC ? 8B 75 ? 8B 7D ? 8B 46 ? 85 C0 7E ? 31 DB EB ? 8D B6 00 00 00 00 83 C3 ? 39 5E ? 7E ? 8B 46 ? ");
}
void Offsets()
{
    // engine.so

    InternalSetValue = 19; // ConVar
    InternalSetFloatValue = 20; // ConVar
    InternalSetIntValue = 21; // ConVar
    ClientCmd = 7; // CEngineClient
    GetLocalPlayer = 12; // CEngineClient
    GetViewAngles = 18; // CEngineClient
    SetViewAngles = 19; // CEngineClient
    GetGameDirectory = 35; // CEngineClient
    StringToButtonCode = 31; // ReadCheatCommandsFromFile
    GetRecordingTick = 1; // CDemoRecorder
    SetSignonState = 3; // CDemoRecorder
    StopRecording = 7; // CDemoRecorder
    GetPlaybackTick = 3; // CDemoPlayer
    StartPlayback = 6; // CDemoPlayer
    IsPlayingBack = 7; // CDemoPlayer
    m_szFileName = 4; // CDemoPlayer::SkipToTick
    m_szDemoBaseName = 1344; // CDemoRecorder::StartupDemoFile
    m_nDemoNumber = 1608; // CDemoRecorder::StartupDemoFile
    m_bRecording = 1606; // CDemoRecorder::SetSignonState
    Paint = 15; // CEngineVGui
    ProcessTick = 12; // CClientState
    tickcount = 73; // CClientState::ProcessTick
    interval_per_tick = 81; // CClientState::ProcessTick
    Disconnect = 37; //  CClientState
    GetClientStateFunction = 11; // CEngineClient::ClientCmd
    GetCurrentMap = 26; // CEngineTool
    demoplayer = 93; // CClientState::Disconnect
    demorecorder = 106; // CClientState::Disconnect
    m_szLevelName = 72; // CEngineTool::GetCurrentMap

    // libvstdlib.so
    UnregisterConCommand = 10; // CCvar (TODO)
    FindCommandBase = 13; // CCvar (TODO)

    // vgui2.so

    GetIScheme = 9; // CSchemeManager
    GetFont = 4; // CScheme

    // server.so

    PlayerMove = 16; // CPortalGameMovement
    AirAccelerate = 23; // CPortalGameMovement
    AirMove = 24; // CPortalGameMovement
    AirMove_Offset1 = 14; // CPortalGameMovement::~CPortalGameMovement
    AirMove_Offset2 = 12; // CGameMovement::~CGameMovement
    FinishGravity = 35; // CPortalGameMovement
    CheckJumpButton = 37; // CPortalGameMovement
    FullTossMove = 38; // CGameMovement
    mv = 8; // CPortalGameMovement::CheckJumpButton
    m_nOldButtons = 40; // CPortalGameMovement::CheckJumpButton
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 61; // CServerGameDLL::Think
    iNumPortalsPlaced = 5724; // CPortal_Player::IncrementPortalsPlaced
    gpGlobals = 467; // CGameMovement::FullTossMove
    player = 4; // CPortalGameMovement::PlayerMove
    m_fFlags = 212; // CBasePlayer::UpdateStepSound
    m_MoveType = 226; // CBasePlayer::UpdateStepSound
    m_nWaterLevel = 347; // CBasePlayer::UpdateStepSound
    m_vecVelocity2 = 64; // CPortalGameMovement::PlayerMove
    frametime = 16; // CBasePlayer::UpdateStepSound
    m_bDucked = 2296; // CPortalGameMovement::FinishUnDuck

    // client.so

    HudProcessInput = 12; // CHLClient
    HudUpdate = 11; // CHLClient
    GetHud = 144; // CHLClient::HudUpdate
    m_vecAbsOrigin = 136; // C_BasePlayer::GetAbsOrigin
    m_angAbsRotation = 172; // C_BasePlayer::GetAbsAngles
    m_vecVelocity = 244; // CFPSPanel::Paint
    GetClientEntity = 3; // CClientEntityList
    GetFlags = 228; // C_BasePlayer::PhysicsSimulate
    GetClientMode = 4; // CHLClient::HudProcessInput (TODO)
    CreateMove = 25; // ClientModeShared
    GetName = 11; // CHud

    // vguimatsurface.so (TODO)

    DrawSetColor = 14; // CMatSystemSurface (TODO)
    DrawFilledRect = 15; // CMatSystemSurface (TODO)
    GetFontTall = 72; // CFPSPanel::ComputeSize
    DrawColoredText = 160; // CFPSPanel::Paint
    DrawTextLen = 163; // CNetGraphPanel::DrawTextFields (TODO)
}
}