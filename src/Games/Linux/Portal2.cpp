#include "Portal2.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Portal2::Portal2()
{
    this->version = SourceGame::Portal2;
}
void Portal2::LoadOffsets()
{
    using namespace Offsets;

    // engine.so

    InternalSetValue = 19; // ConVar
    InternalSetFloatValue = 20; // ConVar
    InternalSetIntValue = 21; // ConVar
    GetScreenSize = 5; // CEngineClient
    ClientCmd = 7; // CEngineClient
    GetClientStateFunction = 11; // CEngineClient::ClientCmd
    Cbuf_AddText = 45; // CEngineClient::ClientCmd
    s_CommandBuffer = 69; // Cbuf_AddText
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
    GetPlaybackTick = 4; // CDemoPlayer
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
    HostState_OnClientConnected = 735; // CClientState::SetSignonState
    hoststate = 9; // HostState_OnClientConnected
    Disconnect = 37; //  CClientState
    demoplayer = 93; // CClientState::Disconnect
    demorecorder = 106; // CClientState::Disconnect
    GetCurrentMap = 26; // CEngineTool
    m_szLevelName = 72; // CEngineTool::GetCurrentMap
    AddListener = 4; // CGameEventManager
    RemoveListener = 6; // CGameEventManager
    FireEventClientSide = 9; // CGameEventManager
    FireEventIntern = 36; // CGameEventManager::FireEventClientSide
    ConPrintEvent = 254; // CGameEventManager::FireEventIntern
    AutoCompletionFunc = 37; // listdemo_CompletionFunc
    Key_SetBinding = 60; // unbind
    IsRunningSimulation = 12; // CEngineAPI
    eng = 7; // CEngineAPI::IsRunningSimulation
    Frame = 6; // CEngine
    m_bLoadGame = 440; // CGameClient::ActivatePlaye/CBaseServer::m_szLevelName
    ScreenPosition = 11; // CIVDebugOverlay

    // libvstdlib.so
    RegisterConCommand = 9; // CCVar
    UnregisterConCommand = 10; // CCvar
    FindCommandBase = 13; // CCvar
    m_pConCommandList = 48; // CCvar
    IsCommand = 2; // ConCommandBase

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
    GameFrame = 4; // CServerGameDLL
    g_InRestore = 51; // CServerGameDLL::GameFrame
    gpGlobals = 104; // CServerGameDLL::GameFrame
    ServiceEventQueue = 484; // CServerGameDLL::GameFrame
    g_EventQueue = 43; // ServiceEventQueue
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 61; // CServerGameDLL::Think
    iNumPortalsPlaced = 5724; // CPortal_Player::IncrementPortalsPlaced
    player = 4; // CPortalGameMovement::PlayerMove
    m_fFlags = 212; // CBasePlayer::UpdateStepSound
    m_MoveType = 226; // CBasePlayer::UpdateStepSound
    m_nWaterLevel = 347; // CBasePlayer::UpdateStepSound
    m_vecVelocity2 = 64; // CPortalGameMovement::PlayerMove
    m_bDucked = 2296; // CPortalGameMovement::FinishUnDuck

    // client.so

    HudProcessInput = 10; // CHLClient
    HudUpdate = 11; // CHLClient
    m_vecAbsOrigin = 136; // C_BasePlayer::GetAbsOrigin
    m_angAbsRotation = 172; // C_BasePlayer::GetAbsAngles
    m_vecVelocity = 244; // CFPSPanel::Paint
    GetClientEntity = 3; // CClientEntityList
    GetFlags = 228; // C_BasePlayer::PhysicsSimulate
    GetClientMode = 12; // CHLClient::HudProcessInput
    CreateMove = 25; // ClientModeShared
    GetName = 11; // CHud
    GetHud = 104; // cc_leaderboard_enable
    FindElement = 120; // cc_leaderboard_enable

    // vguimatsurface.so

    DrawSetColor = 13; // CMatSystemSurface
    DrawFilledRect = 15; // CMatSystemSurface
    DrawLine = 18; // CMatSystemSurface
    DrawSetTextFont = 22; // CMatSystemSurface
    DrawSetTextColor = 24; // CMatSystemSurface
    GetFontTall = 72; // CMatSystemSurface
    PaintTraverseEx = 117; // CMatSystemSurface
    StartDrawing = 193; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 590; // CMatSystemSurface::PaintTraverseEx
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
    speedrun->AddRule(TimerRule(
        "sp_a4_finale4",
        "transition_portal2",
        "TODO",
        TimerAction::End));
}
const char* Portal2::Version()
{
    return "Portal 2 (7054)";
}
