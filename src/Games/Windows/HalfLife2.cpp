#include "HalfLife2.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

HalfLife2::HalfLife2()
{
    this->version = SourceGame::HalfLife2;
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
    HostState_OnClientConnected = 735; // CClientState::SetSignonState (TODO)
    hoststate = 9; // HostState_OnClientConnected (TODO)
    Disconnect = 14; //  CClientState
    demoplayer = 110; // CClientState::Disconnect
    demorecorder = 121; // CClientState::Disconnect
    GetCurrentMap = 23; // CEngineTool
    m_szLevelName = 32; // CEngineTool::GetCurrentMap
    AutoCompletionFunc = 37; // listdemo_CompletionFunc (TODO)
    Key_SetBinding = 60; // unbind (TODO)
    IsRunningSimulation = 12; // CEngineAPI (TODO)
    eng = 7; // CEngineAPI::IsRunningSimulation (TODO)
    Frame = 5; // CEngine
    m_bLoadGame = 440; // CGameClient::ActivatePlayer/CBaseServer::m_szLevelName (TODO)

    // vstdlib.dll
    RegisterConCommand = 6; // CCVar
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
    player = 4; // CGameMovement::CheckJumpButton
    m_vecVelocity2 = 64; // CGameMovement::PlayerMove
    GameFrame = 4; // CServerGameDLL (TODO)
    g_InRestore = 32; // CServerGameDLL::GameFrame (TODO)
    gpGlobals = 84; // CServerGameDLL::GameFrame (TODO)
    ServiceEventQueue = 328; // CServerGameDLL::GameFrame (TODO)
    g_EventQueue = 24; // ServiceEventQueue (TODO)
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 38; // CServerGameDLL::Think
    iNumPortalsPlaced = 4816; // CPortal_Player::IncrementPortalsPlaced
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
    GetFontTall = 69; // CMatSystemSurface
    PaintTraverseEx = 117; // CMatSystemSurface (TODO)
    StartDrawing = 193; // CMatSystemSurface::PaintTraverseEx (TODO)
    FinishDrawing = 590; // CMatSystemSurface::PaintTraverseEx (TODO)
    DrawColoredText = 162; // CMatSystemSurface
    DrawTextLen = 165; // CMatSystemSurface
}
void HalfLife2::LoadRules()
{
}
const char* HalfLife2::GetVersion()
{
    return (this->version != SourceGame::Portal)
        ? "Half-Life 2 (2257546)"
        : "Portal (1910503)";
}