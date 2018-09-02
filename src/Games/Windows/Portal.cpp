#include "Portal.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Portal::Portal()
{
    this->version = SourceGame_Portal;
}
void Portal::LoadOffsets()
{
    HalfLife2::LoadOffsets();

    using namespace Offsets;

    // engine.dll

    InternalSetValue = 10; // ConVar
    InternalSetFloatValue = 11; // ConVar
    InternalSetIntValue = 12; // ConVar
    GetScreenSize = 5; // CEngineClient
    ClientCmd = 7; // CEngineClient
    Cbuf_AddText = 60; // CEngineClient::ClientCmd
    s_CommandBuffer = 71; // Cbuf_AddText
    AddText = 76; // Cbuf_AddText
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
    tickcount = 98; // CClientState::ProcessTick
    interval_per_tick = 68; // CClientState::ProcessTick
    HostState_OnClientConnected = 570; // CClientState::SetSignonState
    hoststate = 1; // HostState_OnClientConnected
    Disconnect = 14; //  CClientState
    demoplayer = 115; // CClientState::Disconnect
    demorecorder = 128; // CClientState::Disconnect
    GetCurrentMap = 23; // CEngineTool
    m_szLevelName = 34; // CEngineTool::GetCurrentMap
    //AddListener = 3; // CGameEventManager
    //RemoveListener = 5; // CGameEventManager
    //FireEventClientSide = 8; // CGameEventManager
    //FireEventIntern = 12; // CGameEventManager::FireEventClientSide
    //ConPrintEvent = 262; // CGameEventManager::FireEventIntern
    AutoCompletionFunc = 66; // listdemo_CompletionFunc
    Key_SetBinding = 135; // unbind
    IsRunningSimulation = 9; // CEngineAPI
    eng = 2; // CEngineAPI::IsRunningSimulation
    Frame = 5; // CEngine
    m_bLoadGame = 335; // CGameClient::ActivatePlayer/CBaseServer::m_szLevelName
    ScreenPosition = 10; // CIVDebugOverlay

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
    gpGlobals = 29; // CServerGameDLL::GameFrame
    ServiceEventQueue = 152; // CServerGameDLL::GameFrame
    g_EventQueue = 1; // ServiceEventQueue
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 39; // CServerGameDLL::Think
    iNumPortalsPlaced = 4796; // CPortal_Player::IncrementPortalsPlaced
    m_fFlags = 260; // CBasePlayer::UpdateStepSound
    m_MoveType = 314; // CBasePlayer::UpdateStepSound
    m_nWaterLevel = 503; // CBasePlayer::UpdateStepSound

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
    StartDrawing = 129; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 650; // CMatSystemSurface::PaintTraverseEx
    DrawColoredText = 162; // CMatSystemSurface
    DrawTextLen = 165; // CMatSystemSurface
}
void Portal::LoadRules()
{
}
const char* Portal::Version()
{
    return "Portal (TODO)";
}
const char* Portal::Process()
{
    return "hl2.exe";
}
