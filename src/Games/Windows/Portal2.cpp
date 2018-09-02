#include "Portal2.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Portal2::Portal2()
{
    this->version = SourceGame_Portal2;
}
void Portal2::LoadOffsets()
{
    using namespace Offsets;

    // engine.dll

    InternalSetValue = 12; // ConVar
    InternalSetFloatValue = 13; // ConVar
    InternalSetIntValue = 14; // ConVar
    GetScreenSize = 5; // CEngineClient
    ClientCmd = 7; // CEngineClient
    GetClientStateFunction = 4; // CEngineClient::ClientCmd
    Cbuf_AddText = 46; // CEngineClient::ClientCmd
    s_CommandBuffer = 82; // Cbuf_AddText
    m_bWaitEnabled = 8265; // CCommandBuffer::AddText
    GetLocalPlayer = 12; // CEngineClient
    GetViewAngles = 18; // CEngineClient
    SetViewAngles = 19; // CEngineClient
    GetMaxClients = 20; // CEngineClient
    GetGameDirectory = 35; // CEngineClient
    GetActiveSplitScreenPlayerSlot = 127; // CEngineClient
    StringToButtonCode = 31; // CInputSystem
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
    HostState_OnClientConnected = 684; // CClientState::SetSignonState
    hoststate = 1; // HostState_OnClientConnected
    Disconnect = 16; //  CClientState
    demoplayer = 74; // CClientState::Disconnect
    demorecorder = 87; // CClientState::Disconnect
    GetCurrentMap = 25; // CEngineTool
    m_szLevelName = 36; // CEngineTool::GetCurrentMap
    AddListener = 3; // CGameEventManager
    RemoveListener = 5; // CGameEventManager
    FireEventClientSide = 8; // CGameEventManager
    FireEventIntern = 12; // CGameEventManager::FireEventClientSide
    ConPrintEvent = 303; // CGameEventManager::FireEventIntern
    AutoCompletionFunc = 66; // listdemo_CompletionFunc
    Key_SetBinding = 135; // unbind
    IsRunningSimulation = 12; // CEngineAPI
    eng = 2; // CEngineAPI::IsRunningSimulation
    Frame = 5; // CEngine
    m_bLoadGame = 448; // CGameClient::ActivatePlayer/CBaseServer::m_szLevelName
    ScreenPosition = 12; // CIVDebugOverlay

    // vstdlib.dll

    RegisterConCommand = 9; // CCVar
    UnregisterConCommand = 10; // CCvar
    FindCommandBase = 13; // CCVar
    m_pConCommandList = 48; // CCvar
    IsCommand = 1; // ConCommandBase

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
    mv = 8; // CPortalGameMovement::CheckJumpButton
    m_nOldButtons = 40; // CPortalGameMovement::CheckJumpButton
    GameFrame = 4; // CServerGameDLL
    g_InRestore = 8; // CServerGameDLL::GameFrame
    gpGlobals = 92; // CServerGameDLL::GameFrame
    ServiceEventQueue = 249; // CServerGameDLL::GameFrame
    g_EventQueue = 1; // ServiceEventQueue
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 39; // CServerGameDLL::Think
    iNumPortalsPlaced = 5700; // CPortal_Player::IncrementPortalsPlaced
    player = 4; // CPortalGameMovement::PlayerMove
    m_fFlags = 204; // CBasePlayer::UpdateStepSound
    m_MoveType = 218; // CBasePlayer::UpdateStepSound
    m_nWaterLevel = 339; // CBasePlayer::UpdateStepSound
    m_vecVelocity2 = 64; // CPortalGameMovement::PlayerMove
    m_bDucked = 2272; // CPortalGameMovement::FinishUnDuck

    // client.dll

    HudProcessInput = 10; // CHLClient
    HudUpdate = 11; // CHLClient
    m_vecAbsOrigin = 156; // C_BasePlayer::GetAbsOrigin
    m_angAbsRotation = 192; // C_BasePlayer::GetAbsAngles
    m_vecVelocity = 264; // CFPSPanel::Paint
    GetClientEntity = 3; // CClientEntityList
    GetFlags = 248; // C_BasePlayer::PhysicsSimulate
    GetClientMode = 4; // CHLClient::HudProcessInput
    CreateMove = 24; // ClientModeShared
    GetName = 10; // CHud
    GetHud = 125; // cc_leaderboard_enable
    FindElement = 135; // cc_leaderboard_enable

    // vguimatsurface.dll

    DrawSetColor = 14; // CMatSystemSurface
    DrawFilledRect = 15; // CMatSystemSurface
    DrawLine = 18; // CMatSystemSurface
    DrawSetTextFont = 22; // CMatSystemSurface
    DrawSetTextColor = 23; // CMatSystemSurface
    GetFontTall = 72; // CMatSystemSurface
    PaintTraverseEx = 117; // CMatSystemSurface
    StartDrawing = 127; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 603; // CMatSystemSurface::PaintTraverseEx
    DrawColoredText = 160; // CMatSystemSurface
    DrawTextLen = 163; // CMatSystemSurface
}
void Portal2::LoadRules()
{
    speedrun->AddRule(TimerRule(
        "sp_a1_intro1",
        "camera_intro",
        "TeleportToView",
        TimerAction::Start));
}
const char* Portal2::Version()
{
    return "Portal 2 (7054)";
}
const char* Portal2::Process()
{
    return "portal2.exe";
}
