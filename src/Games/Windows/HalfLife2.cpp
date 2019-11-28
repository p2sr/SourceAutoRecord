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

    Dtor = 0; // ConVar
    InternalSetValue = 10; // ConVar
    InternalSetFloatValue = 11; // ConVar
    InternalSetIntValue = 12; // ConVar
    Create = 15; // ConVar
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
    net_time = 19; // CDemoRecorder::GetRecordingTick (TODO)
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
    MAX_SPLITSCREEN_PLAYERS = 1; // maxplayers

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
    player = 4; // CGameMovement::CheckJumpButton
    GameFrame = 5; // CServerGameDLL
    GetAllServerClasses = 11; // CServerGameDLL
    IsRestoring = 25; // CServerGameDLL
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 43; // CServerGameDLL::Think
    gpGlobals = 11; // UTIL_PlayerByIndex
    m_MoveType = 314; // CBasePlayer::UpdateStepSound
    m_iClassName = 92; // CBaseEntity
    m_iName = 264; // CBaseEntity
    S_m_vecAbsOrigin = 636; // CBaseEntity
    S_m_angAbsRotation = 760; // CBaseEntity
    m_iEFlags = 256; // CBaseEntity
    m_flGravity = 608; // CBaseEntity
    NUM_ENT_ENTRIES = 4096; // CBaseEntityList::CBaseEntityList
    GetIServerEntity = 1; // CServerTools
    m_EntPtrArray = 51; // CServerTools::GetIServerEntity

    // client.dll

    GetAllClasses = 8; // CHLClient
    HudProcessInput = 10; // CHLClient
    HudUpdate = 11; // CHLClient
    C_m_vecAbsOrigin = 604; // C_BasePlayer::GetAbsOrigin
    C_m_angAbsRotation = 616; // C_BasePlayer::GetAbsAngles
    GetClientEntity = 3; // CClientEntityList
    GetClientMode = 5; // CHLClient::HudProcessInput
    CreateMove = 21; // ClientModeShared
    DecodeUserCmdFromBuffer = 7; // CInput
    m_pCommands = 196; // CInput::DecodeUserCmdFromBuffer
    CUserCmdSize = 84; // CInput::DecodeUserCmdFromBuffer
    MULTIPLAYER_BACKUP = 90; // CInput::DecodeUserCmdFromBuffer
    IN_ActivateMouse = 15; // CHLClient
    g_Input = 2; // CHLClient::IN_ActivateMouse

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
const char* HalfLife2::Version()
{
    return "Half-Life 2 (5377866)";
}
const float HalfLife2::Tickrate()
{
    return 1 / 0.015f;
}
const char* HalfLife2::ModDir()
{
    return "hl2";
}
