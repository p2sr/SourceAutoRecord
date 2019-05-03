#include "HalfLife2.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

HalfLife2::HalfLife2()
{
    this->version = SourceGame_HalfLife2;
}
void HalfLife2::LoadOffsets()
{
    using namespace Offsets;

    // engine.so

    Dtor = 0; // ConVar
    InternalSetValue = 14; // ConVar
    InternalSetFloatValue = 15; // ConVar
    InternalSetIntValue = 16; // ConVar
    Create = 19; // ConVar
    GetScreenSize = 5; // CEngineClient
    ClientCmd = 7; // CEngineClient
    Cbuf_AddText = 28; // CEngineClient::ClientCmd
    s_CommandBuffer = 78; // Cbuf_AddText
    AddText = 87; // Cbuf_AddText
    GetLocalPlayer = 12; // CEngineClient
    GetViewAngles = 19; // CEngineClient
    SetViewAngles = 20; // CEngineClient
    GetMaxClients = 21; // CEngineClient
    GetGameDirectory = 35; // CEngineClient
    ServerCmdKeyValues = 128; // CEngineClient
    cl = 6; // CEngineClient::ServerCmdKeyValues
    StringToButtonCode = 29; // CInputSystem
    GetRecordingTick = 1; // CDemoRecorder
    SetSignonState = 3; // CDemoRecorder
    StopRecording = 7; // CDemoRecorder
    m_szDemoBaseName = 1348; // CDemoRecorder::StartupDemoFile
    m_nDemoNumber = 1612; // CDemoRecorder::StartupDemoFile
    m_bRecording = 1610; // CDemoRecorder::SetSignonState
    GetPlaybackTick = 4; // CDemoPlayer
    StartPlayback = 6; // CDemoPlayer
    IsPlayingBack = 7; // CDemoPlayer
    m_szFileName = 4; // CDemoPlayer::SkipToTick
    Paint = 14; // CEngineVGui
    ProcessTick = 12; // CClientState
    tickcount = 76; // CClientState::ProcessTick
    interval_per_tick = 84; // CClientState::ProcessTick
    HostState_OnClientConnected = 411; // CClientState::SetSignonState
    hoststate = 9; // HostState_OnClientConnected
    Disconnect = 34; //  CClientState
    demoplayer = 151; // CClientState::Disconnect
    demorecorder = 164; // CClientState::Disconnect
    GetCurrentMap = 24; // CEngineTool
    m_szLevelName = 25; // CEngineTool::GetCurrentMap
    //AddListener = 4; // CGameEventManager
    //RemoveListener = 6; // CGameEventManager
    //FireEventClientSide = 9; // CGameEventManager
    //FireEventIntern = 36; // CGameEventManager::FireEventClientSide
    //ConPrintEvent = 450; // CGameEventManager::FireEventIntern
    AutoCompletionFunc = 37; // listdemo_CompletionFunc
    Key_SetBinding = 60; // unbind
    IsRunningSimulation = 9; // CEngineAPI
    eng = 7; // CEngineAPI::IsRunningSimulation
    Frame = 6; // CEngine
    m_bLoadGame = 328; // CGameClient::ActivatePlayer/CBaseServer::m_szLevelName
    ScreenPosition = 9; // CIVDebugOverlay
    m_pConCommandList = 44; // CCvar
    IsCommand = 2; // ConCommandBase
    MAX_SPLITSCREEN_PLAYERS = 1; // maxplayers

    // libvstdlib.so

    RegisterConCommand = 6; // CCVar
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
    player = 4; // CPortalGameMovement::PlayerMove
    mv_m_vecVelocity = 64; // CPortalGameMovement::PlayerMove
    GameFrame = 5; // CServerGameDLL
    GetAllServerClasses = 11; // CServerGameDLL
    IsRestoring = 25; // CServerGameDLL
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 61; // CServerGameDLL::Think
    gpGlobals = 15; // UTIL_PlayerByIndex
    m_MoveType = 334; // CBasePlayer::UpdateStepSound
    m_iClassName = 100; // CBaseEntity
    m_iName = 284; // CBaseEntity
    S_m_vecAbsOrigin = 656; // CBaseEntity
    S_m_angAbsRotation = 780; // CBaseEntity
    m_iEFlags = 276; // CBaseEntity
    m_flGravity = 628; // CBaseEntity
    NUM_ENT_ENTRIES = 4096; // CBaseEntityList::CBaseEntityList
    GetIServerEntity = 2; // CServerTools
    m_EntPtrArray = 48; // CServerTools::GetIServerEntity

    // client.so

    GetAllClasses = 8; // CHLClient
    HudProcessInput = 10; // CHLClient
    HudUpdate = 11; // CHLClient
    C_m_vecAbsOrigin = 588; // C_BasePlayer::GetAbsOrigin
    C_m_angAbsRotation = 600; // C_BasePlayer::GetAbsAngles
    GetClientEntity = 3; // CClientEntityList
    GetClientMode = 1; // CHLClient::HudProcessInput
    CreateMove = 22; // ClientModeShared
    DecodeUserCmdFromBuffer = 7; // CInput
    m_pCommands = 196; // CInput::DecodeUserCmdFromBuffer
    CUserCmdSize = 84; // CInput::DecodeUserCmdFromBuffer
    MULTIPLAYER_BACKUP = 90; // CInput::DecodeUserCmdFromBuffer
    IN_ActivateMouse = 15; // CHLClient
    g_Input = 1; // CHLClient::IN_ActivateMouse

    // vguimatsurface.so

    DrawSetColor = 10; // CMatSystemSurface
    DrawFilledRect = 12; // CMatSystemSurface
    DrawLine = 15; // CMatSystemSurface
    DrawSetTextFont = 17; // CMatSystemSurface
    DrawSetTextColor = 19; // CMatSystemSurface
    GetFontTall = 69; // CMatSystemSurface
    PaintTraverseEx = 114; // CMatSystemSurface
    StartDrawing = 294; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 1681; // CMatSystemSurface::PaintTraverseEx
    DrawColoredText = 162; // CMatSystemSurface
    DrawTextLen = 165; // CMatSystemSurface
}
const char* HalfLife2::Version()
{
    return (this->version != SourceGame_Portal)
        ? "Half-Life 2 (2257546)"
        : "Portal (1910503)";
}
const float HalfLife2::Tickrate()
{
    return 1 / 0.015;
}
const char* HalfLife2::ModDir()
{
    return "hl2";
}
