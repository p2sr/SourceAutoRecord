#pragma once
#include "Offsets.hpp"
#include "Patterns.hpp"

namespace Portal2 {

using namespace Offsets;
using namespace Patterns;

void Patterns()
{
    Init();
    Create("client.dll", "FindElement");

    // engine.dll

    // \x55\x8B\xEC\x8B\x45\x0C\x53\x33\xDB\x56\x8B\xF1\x8B\x4D\x18\x80\x4E\x20\x02 xxxxxxxxxxxxxxxxxxx
    Add("ConCommandCtor", "Portal 2 Build 7054",
        "ConCommand::ConCommand",
        "55 8B EC 8B 45 0C 53 33 DB 56 8B F1 8B 4D 18 80 4E 20 02");

    // \x55\x8B\xEC\xF3\x0F\x10\x45\x00\x8B\x55\x14 xxxxxxx?xxx
    Add("ConVarCtor", "Portal 2 Build 7054",
        "ConVar::ConVar",
        "55 8B EC F3 0F 10 45 ? 8B 55 14");

    // \x55\x8B\xEC\x51\x53\x56\x57\x8B\xF1\xE8\x00\x00\x00\x00\x8B\x1D\x00\x00\x00\x00 xxxxxxxxxx????xx????
    Add("m_bLoadgame", "Portal 2 Build 7054",
        "CGameClient::ActivatePlayer",
        "55 8B EC 51 53 56 57 8B F1 E8 ? ? ? ? 8B 1D ? ? ? ? ",
        32);

    // \x55\x8B\xEC\x56\x8B\x75\x08\x83\xFE\xFF xxxxxxxxxx
    Add("Key_SetBinding", "Portal 2 Build 7054",
        "Key_SetBinding",
        "55 8B EC 56 8B 75 08 83 FE FF");

    // \x55\x8B\xEC\x81\xEC\x00\x00\x00\x00\x53\x8B\x5D\x08\x56\x57\x8B\xF9\x8B\x37\x56\x53\x89\x75\xE4\x89\x5D\xFC\xE8\x00\x00\x00\x00\x83\xC4\x08\x85\xC0\x74\x19\x8B\xC6\x8D\x50\x01\x8D\x64\x24\x00\x8A\x08\x40\x84\xC9\x75\xF9\x2B\xC2\x8D\x44\x18\x01\x89\x45\xFC xxxxx????xxxxxxxxxxxxxxxxxxx????xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    Add("AutoCompletionFunc", "Portal 2 Build 7054",
        "CBaseAutoCompleteFileList::AutoCompletionFunc",
        "55 8B EC 81 EC ? ? ? ? 53 8B 5D 08 56 57 8B F9 8B 37 56 53 89 75 E4 89 5D FC E8 ? ? ? ? 83 C4 08 85 C0 74 19 8B C6 8D 50 01 8D 64 24 00 8A 08 40 84 C9 75 F9 2B C2 8D 44 18 01 89 45 FC");

    // vguimatsurface.dll

    // \x55\x8B\xEC\x83\xEC\x10\x80\x3D\x00\x00\x00\x00\x00 xxxxxxxx?????
    Add("StartDrawing", "Portal 2 Build 7054",
        "CMatSystemSurface::StartDrawing",
        "55 8B EC 83 EC 10 80 3D ? ? ? ? ? ");

    // \x56\x6A\x00\xE8\x00\x00\x00\x00 xxxx????
    Add("FinishDrawing", "Portal 2 Build 7054",
        "CMatSystemSurface::FinishDrawing",
        "56 6A 00 E8 ? ? ? ? ");

    // client.dll

    // \x55\x8B\xEC\x53\x8B\x5D\x08\x56\x57\x8B\xF1\x33\xFF\x39\x7E\x28 xxxxxxxxxxxxxxxx
    Add("FindElement", "Portal 2 Build 7054",
        "CHud::FindElement",
        "55 8B EC 53 8B 5D 08 56 57 8B F1 33 FF 39 7E 28");
}
void Offsets()
{
    // engine.dll

    InternalSetValue = 12; // ConVar
    InternalSetFloatValue = 13; // ConVar
    InternalSetIntValue = 14; // ConVar
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
    StartPlayback = 5; // CDemoPlayer
    IsPlayingBack = 6; // CDemoPlayer
    m_szFileName = 4; // CDemoPlayer::SkipToTick
    m_szDemoBaseName = 1344; // CDemoRecorder::StartupDemoFile
    m_nDemoNumber = 1608; // CDemoRecorder::StartupDemoFile
    m_bRecording = 1606; // CDemoRecorder::SetSignonState
    Paint = 14; // CEngineVGui
    ProcessTick = 1; // CClientState/IServerMessageHandler
    tickcount = 95; // CClientState::ProcessTick
    interval_per_tick = 65; // CClientState::ProcessTick
    Disconnect = 16; //  CClientState
    GetClientStateFunction = 4; // CEngineClient::ClientCmd
    demoplayer = 74; // CClientState::Disconnect
    demorecorder = 87; // CClientState::Disconnect
    GetCurrentMap = 25; // CEngineTool
    m_szLevelName = 36; // CEngineTool::GetCurrentMap

    // vstdlib.dll
    UnregisterConCommand = 10; // CCvar 
    FindCommandBase = 13; // CCVar

    // vgui2.dll

    GetIScheme = 8; // CSchemeManager
    GetFont = 3; // CScheme

    // server.dll

    PlayerMove = 17; // CPortalGameMovement
    AirAccelerate = 24; // CPortalGameMovement
    AirMove = 25; // CPortalGameMovement
    AirMove_Offset1 = 7; // CPortalGameMovement::CPortalGameMovement
    AirMove_Offset2 = 5; // CGameMovement::CGameMovement
    FinishGravity = 34; // CPortalGameMovement
    CheckJumpButton = 36; // CPortalGameMovement
    FullTossMove = 37; // CGameMovement
    mv = 8; // CPortalGameMovement::CheckJumpButton
    m_nOldButtons = 40; // CPortalGameMovement::CheckJumpButton
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 37; // CServerGameDLL::Think
    iNumPortalsPlaced = 5700; // CPortal_Player::IncrementPortalsPlaced
    gpGlobals = 569; // CGameMovement::FullTossMove
    player = 4; // CPortalGameMovement::PlayerMove
    m_fFlags = 204; // CBasePlayer::UpdateStepSound
    m_MoveType = 218; // CBasePlayer::UpdateStepSound
    m_nWaterLevel = 339; // CBasePlayer::UpdateStepSound
    m_vecVelocity2 = 64; // CPortalGameMovement::PlayerMove
    frametime = 16; // CBasePlayer::UpdateStepSound
    m_bDucked = 2272; // CPortalGameMovement::FinishUnDuck

    // client.dll

    HudProcessInput = 10; // CHLClient
    HudUpdate = 11; // CHLClient
    GetHud = 221; // CHLClient::HudUpdate
    m_vecAbsOrigin = 156; // C_BasePlayer::GetAbsOrigin
    m_angAbsRotation = 192; // C_BasePlayer::GetAbsAngles
    m_vecVelocity = 264; // CFPSPanel::Paint
    GetClientEntity = 3; // CClientEntityList
    GetFlags = 248; // C_BasePlayer::PhysicsSimulate
    GetClientMode = 4; // CHLClient::HudProcessInput
    CreateMove = 24; // ClientModeShared
    GetName = 10; // CHud

    // vguimatsurface.dll

    DrawSetColor = 14; // CMatSystemSurface
    DrawFilledRect = 15; // CMatSystemSurface
    GetFontTall = 72; // CFPSPanel::ComputeSize
    DrawColoredText = 160; // CFPSPanel::Paint
    DrawTextLen = 163; // CNetGraphPanel::DrawTextFields
}
}