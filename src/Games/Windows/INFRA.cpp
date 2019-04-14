#include "INFRA.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

INFRA::INFRA()
{
    this->version = SourceGame_INFRA;
}
void INFRA::LoadOffsets()
{
    Portal2::LoadOffsets();

    using namespace Offsets;

    // engine.dll

    InternalSetValue = 14; // ConVar
    InternalSetFloatValue = 15; // ConVar
    InternalSetIntValue = 16; // ConVar
    //GetScreenSize = 5; // CEngineClient
    //ClientCmd = 7; // CEngineClient
    //GetClientStateFunction = 4; // CEngineClient::ClientCmd
    //Cbuf_AddText = 46; // CEngineClient::ClientCmd
    //s_CommandBuffer = 82; // Cbuf_AddText
    //CCommandBufferSize = 9556; // Cbuf_AddText
    //m_bWaitEnabled = 8265; // CCommandBuffer::AddText
    //GetLocalPlayer = 12; // CEngineClient
    //GetViewAngles = 18; // CEngineClient
    //SetViewAngles = 19; // CEngineClient
    GetLocalClient = 138; // CEngineClient::SetViewAngles
    viewangles = 35424; // CEngineClient::SetViewAngles
    //GetMaxClients = 20; // CEngineClient
    //GetGameDirectory = 35; // CEngineClient
    GetActiveSplitScreenPlayerSlot = 129; // CEngineClient
    //StringToButtonCode = 31; // CInputSystem
    //GetRecordingTick = 1; // CDemoRecorder
    //SetSignonState = 3; // CDemoRecorder
    //StopRecording = 7; // CDemoRecorder
    //m_szDemoBaseName = 1344; // CDemoRecorder::StartupDemoFile
    //m_nDemoNumber = 1608; // CDemoRecorder::StartupDemoFile
    //m_bRecording = 1606; // CDemoRecorder::SetSignonState
    //GetPlaybackTick = 3; // CDemoPlayer
    //StartPlayback = 5; // CDemoPlayer
    //IsPlayingBack = 6; // CDemoPlayer
    //m_szFileName = 4; // CDemoPlayer::SkipToTick
    //Paint = 14; // CEngineVGui
    //ProcessTick = 1; // CClientState/IServerMessageHandler
    tickcount = 104; // CClientState::ProcessTick
    interval_per_tick = 74; // CClientState::ProcessTick
    HostState_OnClientConnected = 724; // CClientState::SetSignonState
    //hoststate = 1; // HostState_OnClientConnected
    //Disconnect = 16; //  CClientState
    //demoplayer = 74; // CClientState::Disconnect
    //demorecorder = 87; // CClientState::Disconnect
    //GetCurrentMap = 25; // CEngineTool
    //m_szLevelName = 36; // CEngineTool::GetCurrentMap
    //AddListener = 3; // CGameEventManager
    //RemoveListener = 5; // CGameEventManager
    //FireEventClientSide = 8; // CGameEventManager
    //FireEventIntern = 12; // CGameEventManager::FireEventClientSide
    ConPrintEvent = 351; // CGameEventManager::FireEventIntern
    //AutoCompletionFunc = 66; // listdemo_CompletionFunc
    //Key_SetBinding = 135; // unbind
    //IsRunningSimulation = 12; // CEngineAPI
    //eng = 2; // CEngineAPI::IsRunningSimulation
    //Frame = 5; // CEngine
    //m_bLoadGame = 448; // CGameClient::ActivatePlayer/CBaseServer::m_szLevelName
    //ScreenPosition = 12; // CIVDebugOverlay
    //cmd_alias = 37; // alias
    ClientCommand = 40; // CVEngineServer

    // vstdlib.dll

    //RegisterConCommand = 9; // CCVar
    //UnregisterConCommand = 10; // CCvar
    //FindCommandBase = 13; // CCVar
    //m_pConCommandList = 48; // CCvar
    //IsCommand = 1; // ConCommandBase

    // vgui2.dll

    //GetIScheme = 8; // CSchemeManager
    //GetFont = 3; // CScheme

    // server.dll

    //ProcessMovement = 1; // CGameMovement
    //PlayerMove = 17; // CPortalGameMovement
    //AirAccelerate = 24; // CPortalGameMovement
    //AirMove = 25; // CPortalGameMovement
    //AirMove_Offset1 = 7; // CPortalGameMovement::CPortalGameMovement
    //AirMove_Offset2 = 5; // CGameMovement::CGameMovement
    FinishGravity = 33; // CPortalGameMovement
    CheckJumpButton = 35; // CPortalGameMovement
    //mv = 8; // CPortalGameMovement::CheckJumpButton
    //m_nOldButtons = 40; // CPortalGameMovement::CheckJumpButton
    //player = 4; // CPortalGameMovement::PlayerMove
    //mv_m_vecVelocity = 64; // CPortalGameMovement::PlayerMove
    //GameFrame = 4; // CServerGameDLL
    //GetAllServerClasses = 10; // CServerGameDLL
    IsRestoring = 25; // CServerGameDLL
    Think = 32; // CServerGameDLL
    //UTIL_PlayerByIndex = 39; // CServerGameDLL::Think
    //gpGlobals = 14; // UTIL_PlayerByIndex
    //m_MoveType = 218; // CBasePlayer::UpdateStepSound
    //m_iClassName = 96; // CBaseEntity
    //S_m_vecAbsOrigin = 460; // CBaseEntity
    //S_m_angAbsRotation = 472; // CBaseEntity
    //m_iEFlags = 200; // CBaseEntity
    //m_flGravity = 772; // CBaseEntity
    NUM_ENT_ENTRIES = 16384; // CBaseEntityList::CBaseEntityList
    //GetIServerEntity = 1; // CServerTools
    //m_EntPtrArray = 61; // CServerTools::GetIServerEntity
    //IsPlayer = 85; // CBasePlayer

    // client.dll

    GetAllClasses = 9; // CHLClient
    HudProcessInput = 11; // CHLClient
    //GetClientMode = 4; // CHLClient::HudProcessInput
    HudUpdate = 12; // CHLClient
    IN_ActivateMouse = 16; // CHLClient
    //g_Input = 2; // CHLClient::IN_ActivateMouse
    //C_m_vecAbsOrigin = 156; // C_BasePlayer::GetAbsOrigin
    //C_m_angAbsRotation = 192; // C_BasePlayer::GetAbsAngles
    //GetClientEntity = 3; // CClientEntityList
    //g_pClientMode = 19; // GetClientMode
    //CreateMove = 24; // ClientModeShared
    //GetName = 10; // CHud?
    //GetHud = 125; // cc_leaderboard_enable?
    //FindElement = 135; // cc_leaderboard_enable?
    //DecodeUserCmdFromBuffer = 7; // CInput
    PerUserInput_tSize = 388; // CInput::DecodeUserCmdFromBuffer
    m_pCommands = 380; // CInput::DecodeUserCmdFromBuffer
    CUserCmdSize = 116; // CInput::DecodeUserCmdFromBuffer
    //MULTIPLAYER_BACKUP = 150; // CInput::DecodeUserCmdFromBuffer
    //GetButtonBits = 2; // CInput
    JoyStickApplyMovement = 61; // CInput
    KeyDown = 401; // CInput::JoyStickApplyMovement
    KeyUp = 380; // CInput::JoyStickApplyMovement

    // vguimatsurface.dll

    //DrawSetColor = 14; // CMatSystemSurface
    //DrawFilledRect = 15; // CMatSystemSurface
    //DrawLine = 18; // CMatSystemSurface
    //DrawSetTextFont = 22; // CMatSystemSurface
    //DrawSetTextColor = 23; // CMatSystemSurface
    //GetFontTall = 72; // CMatSystemSurface
    PaintTraverseEx = 118; // CMatSystemSurface
    //StartDrawing = 127; // CMatSystemSurface::PaintTraverseEx
    //FinishDrawing = 603; // CMatSystemSurface::PaintTraverseEx
    //DrawColoredText = 160; // CMatSystemSurface
    //DrawTextLen = 163; // CMatSystemSurface
}
const char* INFRA::Version()
{
    return "INFRA (6905)";
}
const char* INFRA::Process()
{
    return "infra.exe";
}
const float INFRA::Tickrate()
{
    return 120;
}
