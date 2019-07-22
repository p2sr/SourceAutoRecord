#include "Portal2.hpp"

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

    Dtor = 9; // ConVar
    InternalSetValue = 12; // ConVar
    InternalSetFloatValue = 13; // ConVar
    InternalSetIntValue = 14; // ConVar
    Create = 27; // ConVar
    GetScreenSize = 5; // CEngineClient
    ClientCmd = 7; // CEngineClient
    GetClientStateFunction = 4; // CEngineClient::ClientCmd
    Cbuf_AddText = 46; // CEngineClient::ClientCmd
    s_CommandBuffer = 82; // Cbuf_AddText
    CCommandBufferSize = 9556; // Cbuf_AddText
    m_bWaitEnabled = 8265; // CCommandBuffer::AddText
    GetLocalPlayer = 12; // CEngineClient
    GetViewAngles = 18; // CEngineClient
    SetViewAngles = 19; // CEngineClient
    GetLocalClient = 128; // CEngineClient::SetViewAngles
    viewangles = 19040; // CEngineClient::SetViewAngles
    GetMaxClients = 20; // CEngineClient
    GetGameDirectory = 35; // CEngineClient
    GetActiveSplitScreenPlayerSlot = 127; // CEngineClient
    GetSteamAPIContext = 177; // CEngineClient
    StringToButtonCode = 31; // CInputSystem
    SleepUntilInput = 33; // CInputSystem
    GetRecordingTick = 1; // CDemoRecorder
    net_time = 19; // CDemoRecorder::GetRecordingTick
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
    MAX_SPLITSCREEN_PLAYERS = 2; // maxplayers
    OnGameOverlayActivated = 144; // CSteam3Client

    // vstdlib.dll

    RegisterConCommand = 9; // CCVar
    UnregisterConCommand = 10; // CCvar
    FindCommandBase = 13; // CCVar
    InstallGlobalChangeCallback = 19; // CCvar
    RemoveGlobalChangeCallback = 20; // CCvar
    m_pConCommandList = 48; // CCvar
    IsCommand = 1; // ConCommandBase

    // vgui2.dll

    GetIScheme = 8; // CSchemeManager
    GetFont = 3; // CScheme

    // server.dll

    ProcessMovement = 1; // CGameMovement
    PlayerMove = 17; // CPortalGameMovement
    AirAccelerate = 24; // CPortalGameMovement
    AirMove = 25; // CPortalGameMovement
    AirMove_Offset1 = 7; // CPortalGameMovement::CPortalGameMovement
    AirMove_Offset2 = 5; // CGameMovement::CGameMovement
    FinishGravity = 34; // CPortalGameMovement
    CheckJumpButton = 36; // CPortalGameMovement
    mv = 8; // CPortalGameMovement::CheckJumpButton
    GameFrame = 4; // CServerGameDLL
    GetAllServerClasses = 10; // CServerGameDLL
    IsRestoring = 24; // CServerGameDLL
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 39; // CServerGameDLL::Think
    gpGlobals = 14; // UTIL_PlayerByIndex
    player = 4; // CPortalGameMovement::PlayerMove
    m_MoveType = 218; // CBasePlayer::UpdateStepSound
    m_iClassName = 96; // CBaseEntity
    S_m_vecAbsOrigin = 460; // CBaseEntity
    S_m_angAbsRotation = 472; // CBaseEntity
    m_iEFlags = 200; // CBaseEntity
    m_flGravity = 772; // CBaseEntity
    NUM_ENT_ENTRIES = 8192; // CBaseEntityList::CBaseEntityList
    GetIServerEntity = 1; // CServerTools
    m_EntPtrArray = 61; // CServerTools::GetIServerEntity
    ClientCommand = 39; // CVEngineServer
    IsPlayer = 85; // CBasePlayer
    m_pSurfaceData = 3868; // CGameMovement::CheckJumpButton
    jumpFactor = 68; // CGameMovement::CheckJumpButton

    // client.dll

    GetAllClasses = 8; // CHLClient
    HudProcessInput = 10; // CHLClient
    HudUpdate = 11; // CHLClient
    C_m_vecAbsOrigin = 156; // C_BasePlayer::GetAbsOrigin
    C_m_angAbsRotation = 192; // C_BasePlayer::GetAbsAngles
    GetClientEntity = 3; // CClientEntityList
    GetClientMode = 4; // CHLClient::HudProcessInput
    g_pClientMode = 19; // GetClientMode
    CreateMove = 24; // ClientModeShared
    GetName = 10; // CHud
    GetHud = 125; // cc_leaderboard_enable
    FindElement = 135; // cc_leaderboard_enable
    DecodeUserCmdFromBuffer = 7; // CInput
    PerUserInput_tSize = 368; // CInput::DecodeUserCmdFromBuffer
    m_pCommands = 228; // CInput::DecodeUserCmdFromBuffer
    CUserCmdSize = 96; // CInput::DecodeUserCmdFromBuffer
    MULTIPLAYER_BACKUP = 150; // CInput::DecodeUserCmdFromBuffer
    IN_ActivateMouse = 15; // CHLClient
    g_Input = 2; // CHLClient::IN_ActivateMouse
    GetButtonBits = 2; // CInput
    JoyStickApplyMovement = 64; // CInput
    KeyDown = 398; // CInput::JoyStickApplyMovement
    KeyUp = 377; // CInput::JoyStickApplyMovement

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
const char* Portal2::Version()
{
    return "Portal 2 (7054)";
}
const float Portal2::Tickrate()
{
    return 60;
}
const char* Portal2::ModDir()
{
    return "portal2";
}
