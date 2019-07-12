#include "HalfLife2Unpack.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

HalfLife2Unpack::HalfLife2Unpack()
{
    this->version = SourceGame_HalfLife2;
}
void HalfLife2Unpack::LoadOffsets()
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
    Cbuf_AddText = 25; // CEngineClient::ClientCmd
    s_CommandBuffer = 69; // Cbuf_AddText
    AddText = 74; // Cbuf_AddText
    GetLocalPlayer = 12; // CEngineClient
    GetViewAngles = 19; // CEngineClient
    SetViewAngles = 20; // CEngineClient
    GetMaxClients = 21; // CEngineClient
    GetGameDirectory = 35; // CEngineClient
    ServerCmdKeyValues = 83; // CEngineClient (not real ServerCmdKeyValues)
    cl = 1; // CEngineClient::ServerCmdKeyValues
    StringToButtonCode = 29; // CInputSystem
    GetRecordingTick = 1; // CDemoRecorder
    SetSignonState = 3; // CDemoRecorder
    StopRecording = 7; // CDemoRecorder
    m_szDemoBaseName = 1344; // CDemoRecorder::StartupDemoFile
    m_nDemoNumber = 1608; // CDemoRecorder::StartupDemoFile
    m_bRecording = 1606; // CDemoRecorder::SetSignonState
    GetPlaybackTick = 3; // CDemoPlayer
    StartPlayback = 5; // CDemoPlayer
    IsPlayingBack = 6; // CDemoPlayer
    m_szFileName = 4; // CDemoPlayer::SkipToTick
    Paint = 13; // CEngineVGui
    ProcessTick = 1; // CClientState/IServerMessageHandler
    tickcount = 97; // CClientState::ProcessTick
    interval_per_tick = 63; // CClientState::ProcessTick
    HostState_OnClientConnected = 554; // CClientState::SetSignonState
    hoststate = 1; // HostState_OnClientConnected
    Disconnect = 14; //  CClientState
    demoplayer = 54; // CClientState::Disconnect
    demorecorder = 67; // CClientState::Disconnect
    GetCurrentMap = 23; // CEngineTool
    m_szLevelName = 34; // CEngineTool::GetCurrentMap
    AutoCompletionFunc = 65; // listdemo_CompletionFunc
    Key_SetBinding = 127; // unbind
    IsRunningSimulation = 9; // CEngineAPI
    eng = 2; // CEngineAPI::IsRunningSimulation
    Frame = 5; // CEngine
    m_bLoadGame = 320; // CGameClient::ActivatePlayer/CBaseServer::m_szLevelName
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

    PlayerMove = 12; // CGameMovement
    CheckJumpButton = 22; // CGameMovement
    mv = 8; // CGameMovement::CheckJumpButton
    player = 4; // CGameMovement::CheckJumpButton
    GameFrame = 4; // CServerGameDLL
    GetAllServerClasses = 10; // CServerGameDLL
    IsRestoring = 24; // CServerGameDLL
    Think = 30; // CServerGameDLL
    UTIL_PlayerByIndex = 36; // CServerGameDLL::Think
    gpGlobals = 13; // UTIL_PlayerByIndex
    m_MoveType = 306; // CBasePlayer::UpdateStepSound
    m_iClassName = 92; // CBaseEntity
    m_iName = 260; // CBaseEntity
    S_m_vecAbsOrigin = 580; // CBaseEntity
    S_m_angAbsRotation = 704; // CBaseEntity
    m_iEFlags = 252; // CBaseEntity
    m_flGravity = 552; // CBaseEntity
    NUM_ENT_ENTRIES = 4096; // CBaseEntityList::CBaseEntityList
    GetIServerEntity = 1; // CServerTools
    m_EntPtrArray = 48; // CServerTools::GetIServerEntity

    // client.dll

    GetAllClasses = 6; // CHLClient
    HudProcessInput = 8; // CHLClient
    GetClientMode = 2; // CHLClient::HudProcessInput
    HudUpdate = 9; // CHLClient
    IN_ActivateMouse = 13; // CHLClient
    g_Input = 2; // CHLClient::IN_ActivateMouse
    C_m_vecAbsOrigin = 504; // C_BasePlayer::GetAbsOrigin
    C_m_angAbsRotation = 516; // C_BasePlayer::GetAbsAngles
    GetClientEntity = 3; // CClientEntityList
    CreateMove = 21; // ClientModeShared
    DecodeUserCmdFromBuffer = 7; // CInput
    m_pCommands = 180; // CInput::DecodeUserCmdFromBuffer
    CUserCmdSize = 84; // CInput::DecodeUserCmdFromBuffer
    MULTIPLAYER_BACKUP = 90; // CInput::DecodeUserCmdFromBuffer

    // vguimatsurface.dll

    DrawSetColor = 11; // CMatSystemSurface
    DrawFilledRect = 12; // CMatSystemSurface
    DrawLine = 15; // CMatSystemSurface
    DrawSetTextFont = 17; // CMatSystemSurface
    DrawSetTextColor = 18; // CMatSystemSurface
    GetFontTall = 67; // CMatSystemSurface
    PaintTraverseEx = 111; // CMatSystemSurface
    StartDrawing = 91; // CMatSystemSurface::PaintTraverseEx
    FinishDrawing = 591; // CMatSystemSurface::PaintTraverseEx
    DrawColoredText = 138; // CMatSystemSurface
    DrawTextLen = 141; // CMatSystemSurface
}
const char* HalfLife2Unpack::Version()
{
    return "Half-Life 2 (5135)";
}
const float HalfLife2Unpack::Tickrate()
{
    return 1 / 0.015f;
}
const char* HalfLife2Unpack::ModDir()
{
    return "hl2";
}
