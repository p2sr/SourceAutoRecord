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

    // engine.so

    InternalSetValue = 19; // ConVar
    InternalSetFloatValue = 20; // ConVar
    InternalSetIntValue = 21; // ConVar
    GetScreenSize = 5; // CEngineClient
    ClientCmd = 7; // CEngineClient
    GetClientStateFunction = 11; // CEngineClient::ClientCmd
    Cbuf_AddText = 45; // CEngineClient::ClientCmd
    s_CommandBuffer = 69; // Cbuf_AddText
    CCommandBufferSize = 9556; // Cbuf_AddText
    m_bWaitEnabled = 8265; // CCommandBuffer::AddText
    GetLocalPlayer = 12; // CEngineClient
    GetViewAngles = 18; // CEngineClient
    SetViewAngles = 19; // CEngineClient
    GetLocalClient = 85; // CEngineClient::SetViewAngles
    viewangles = 19012; // CEngineClient::SetViewAngles
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
    cmd_alias = 236; // alias

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

    ProcessMovement = 2; // CGameMovement
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
    GetAllServerClasses = 10; // CServerGameDLL
    IsRestoring = 24; // CServerGameDLL
    Think = 31; // CServerGameDLL
    UTIL_PlayerByIndex = 61; // CServerGameDLL::Think
    gpGlobals = 12; // UTIL_PlayerByIndex
    player = 4; // CPortalGameMovement::PlayerMove
    m_MoveType = 226; // CBasePlayer::UpdateStepSound
    mv_m_vecVelocity = 64; // CPortalGameMovement::PlayerMove
    m_iClassName = 104; // CBaseEntity
    S_m_vecAbsOrigin = 468; // CBaseEntity
    S_m_angAbsRotation = 480; // CBaseEntity
    m_iEFlags = 208; // CBaseEntity
    m_flGravity = 792; // CBaseEntity
    NUM_ENT_ENTRIES = 8192; // CBaseEntityList::CBaseEntityList
    GetIServerEntity = 2; // CServerTools
    m_EntPtrArray = 48; // CServerTools::GetIServerEntity
    ClientCommand = 39; // CVEngineServer
    IsPlayer = 86; // CBasePlayer

    // client.so

    GetAllClasses = 8; // CHLClient
    HudProcessInput = 10; // CHLClient
    HudUpdate = 11; // CHLClient
    C_m_vecAbsOrigin = 136; // C_BasePlayer::GetAbsOrigin
    C_m_angAbsRotation = 172; // C_BasePlayer::GetAbsAngles
    GetClientEntity = 3; // CClientEntityList
    GetClientMode = 12; // CHLClient::HudProcessInput
    g_pClientMode = 25; // GetClientMode
    CreateMove = 25; // ClientModeShared
    GetName = 11; // CHud
    GetHud = 104; // cc_leaderboard_enable
    FindElement = 120; // cc_leaderboard_enable
    DecodeUserCmdFromBuffer = 7; // CInput
    m_pCommands = 172; // CInput::DecodeUserCmdFromBuffer
    CUserCmdSize = 96; // CInput::DecodeUserCmdFromBuffer
    MULTIPLAYER_BACKUP = 150; // CInput::DecodeUserCmdFromBuffer
    GetPerUser = 153; // CInput::DecodeUserCmdFromBuffer
    IN_ActivateMouse = 15; // CHLClient
    g_Input = 1; // CHLClient::IN_ActivateMouse
    GetButtonBits = 2; // CInput
    JoyStickApplyMovement = 64; // CInput
    KeyDown = 295; // CInput::JoyStickApplyMovement
    KeyUp = 341; // CInput::JoyStickApplyMovement

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
const char* Portal2::Version()
{
    return "Portal 2 (7054)";
}
const char* Portal2::Process()
{
    return "portal2_linux";
}
const float Portal2::Tickrate()
{
    return 60;
}
