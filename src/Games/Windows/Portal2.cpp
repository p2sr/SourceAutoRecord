#include "Portal2.hpp"

#include "Features/Speedrun.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Portal2::Portal2()
{
    this->version = SourceGame::Portal2;
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
    GetLocalPlayer = 12; // CEngineClient
    GetViewAngles = 18; // CEngineClient
    SetViewAngles = 19; // CEngineClient
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
    HostState_OnClientConnected = 123; // CClientState::SetSignonState (TODO)
    hoststate = 123; // HostState_OnClientConnected (TODO)
    Disconnect = 16; //  CClientState
    demoplayer = 74; // CClientState::Disconnect
    demorecorder = 87; // CClientState::Disconnect
    GetCurrentMap = 25; // CEngineTool
    m_szLevelName = 36; // CEngineTool::GetCurrentMap
    AddListener = 3; // CGameEventManager
    RemoveListener = 5; // CGameEventManager
    AutoCompletionFunc = 37; // listdemo_CompletionFunc (TODO)
    Key_SetBinding = 60; // unbind (TODO)
    IsRunningSimulation = 5; // CEngineAPI (TODO)
    eng = 7; // CEngineAPI::IsRunningSimulation (TODO)
    Frame = 5; // CEngine
    m_bLoadGame = 440; // CGameClient::ActivatePlayer/CBaseServer::m_szLevelName (TODO)

    // vstdlib.dll
    RegisterConCommand = 9; // CCVar
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
    mv = 8; // CPortalGameMovement::CheckJumpButton
    m_nOldButtons = 40; // CPortalGameMovement::CheckJumpButton
    GameFrame = 4; // CServerGameDLL
    g_InRestore = 8; // CServerGameDLL::GameFrame
    gpGlobals = 92; // CServerGameDLL::GameFrame
    ServiceEventQueue = 249; // CServerGameDLL::GameFrame
    g_EventQueue = 1; // ServiceEventQueue
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 37; // CServerGameDLL::Think
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
    GetHud = 221; // cc_leaderboard_enable (TODO)
    FindElement = 123; // cc_leaderboard_enable (TODO)

    // vguimatsurface.dll

    DrawSetColor = 14; // CMatSystemSurface
    DrawFilledRect = 15; // CMatSystemSurface
    GetFontTall = 72; // CMatSystemSurface
    PaintTraverseEx = 117; // CMatSystemSurface (TODO)
    StartDrawing = 193; // CMatSystemSurface::PaintTraverseEx (TODO)
    FinishDrawing = 590; // CMatSystemSurface::PaintTraverseEx (TODO)
    DrawColoredText = 160; // CMatSystemSurface
    DrawTextLen = 163; // CMatSystemSurface
}
void Portal2::LoadRules()
{
    Speedrun::timer->AddRule(Speedrun::TimerRule(
        "sp_a1_intro1",
        "camera_intro",
        "TeleportToView",
        Speedrun::TimerAction::Start));
    Speedrun::timer->AddRule(Speedrun::TimerRule(
        "sp_a4_finale4",
        "transition_portal2",
        "TODO",
        Speedrun::TimerAction::End));
}
const char* Portal2::GetVersion()
{
    return "Portal 2 (7054)";
}
