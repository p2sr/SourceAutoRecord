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

    // engine.dll

    InternalSetValue = 10; // ConVar
    InternalSetFloatValue = 11; // ConVar
    InternalSetIntValue = 12; // ConVar
    GetScreenSize = 5; // CEngineClient
    ClientCmd = 7; // CEngineClient
    Cbuf_AddText = 58; // CEngineClient::ClientCmd
    s_CommandBuffer = 64; // Cbuf_AddText
    AddText = 69; // Cbuf_AddText
    GetLocalPlayer = 12; // CEngineClient
    GetViewAngles = 19; // CEngineClient
    SetViewAngles = 20; // CEngineClient
    GetMaxClients = 21; // CEngineClient
    GetGameDirectory = 35; // CEngineClient
    ServerCmdKeyValues = 127; // CEngineClient
    cl = 4; // CEngineClient::ServerCmdKeyValues
    StringToButtonCode = 29; // CInputSystem
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
    HostState_OnClientConnected = 518; // CClientState::SetSignonState
    hoststate = 1; // HostState_OnClientConnected
    Disconnect = 14; //  CClientState
    demoplayer = 110; // CClientState::Disconnect
    demorecorder = 121; // CClientState::Disconnect
    GetCurrentMap = 23; // CEngineTool
    m_szLevelName = 32; // CEngineTool::GetCurrentMap
    //AddListener = 3; // CGameEventManager
    //RemoveListener = 5; // CGameEventManager
    //FireEventClientSide = 8; // CGameEventManager
    //FireEventIntern = 11; // CGameEventManager::FireEventClientSide
    //ConPrintEvent = 231; // CGameEventManager::FireEventIntern
    AutoCompletionFunc = 62; // listdemo_CompletionFunc
    Key_SetBinding = 110; // unbind
    IsRunningSimulation = 9; // CEngineAPI
    eng = 2; // CEngineAPI::IsRunningSimulation
    Frame = 5; // CEngine
    m_bLoadGame = 335; // CGameClient::ActivatePlayer/CBaseServer::m_szLevelName
    ScreenPosition = 10; // CIVDebugOverlay
    cmd_alias = 284; // alias (TODO)

    // vstdlib.dll

    RegisterConCommand = 6; // CCVar
    UnregisterConCommand = 7; // CCvar
    FindCommandBase = 10; // CCvar
    m_pConCommandList = 44; // CCvar
    IsCommand = 1; // ConCommandBase

    // vgui2.dll

    GetIScheme = 8; // CSchemeManager
    GetFont = 3; // CScheme

    // server.dll

    PlayerMove = 13; // CGameMovement
    CheckJumpButton = 29; // CGameMovement
    FullTossMove = 30; // CGameMovement
    mv = 8; // CGameMovement::CheckJumpButton
    m_nOldButtons = 40; // CGameMovement::CheckJumpButton
    player = 4; // CGameMovement::CheckJumpButton
    m_vecVelocity2 = 64; // CGameMovement::PlayerMove
    GameFrame = 5; // CServerGameDLL
    g_InRestore = 8; // CServerGameDLL::GameFrame
    ServiceEventQueue = 147; // CServerGameDLL::GameFrame
    g_EventQueue = 1; // ServiceEventQueue
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 38; // CServerGameDLL::Think
    gpGlobals = 29; // UTIL_PlayerByIndex (TODO)
    m_fFlags = 260; // CBasePlayer::UpdateStepSound
    m_MoveType = 314; // CBasePlayer::UpdateStepSound
    m_nWaterLevel = 503; // CBasePlayer::UpdateStepSound
    m_iClassName = 104; // CBaseEntity (TODO)
    m_iName = 216; // CBaseEntity (TODO)
    m_vecAbsOrigin2 = 468; // CBaseEntity (TODO)
    m_angAbsRotation2 = 480; // CBaseEntity (TODO)
    m_vecVelocity3 = 492; // CBaseEntity (TODO)
    m_iEFlags = 208; // CBaseEntity (TODO)
    m_flMaxspeed = 3728; // CBaseEntity (TODO)
    m_flGravity = 792; // CBaseEntity (TODO)
    m_vecViewOffset = 748; // CBaseEntity (TODO)
    NUM_ENT_ENTRIES = 8192; // CBaseEntityList::CBaseEntityList (TODO)
    GetIServerEntity = 2; // CServerTools (TODO)
    m_EntPtrArray = 48; // CServerTools::GetIServerEntity (TODO)

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
    DrawLine = 15; // CMatSystemSurface
    DrawSetTextFont = 17; // CMatSystemSurface
    DrawSetTextColor = 18; // CMatSystemSurface
    GetFontTall = 69; // CMatSystemSurface
    PaintTraverseEx = 114; // CMatSystemSurface
    StartDrawing = 124; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 606; // CMatSystemSurface::PaintTraverseEx
    DrawColoredText = 162; // CMatSystemSurface
    DrawTextLen = 165; // CMatSystemSurface
}
void HalfLife2::LoadRules()
{
}
const char* HalfLife2::Version()
{
    return "Half-Life 2 (2257546)";
}
const char* HalfLife2::Process()
{
    return "hl2.exe";
}
const float HalfLife2::Tickrate()
{
    return 1 / 0.015;
}
