#include "Portal2.hpp"

#include "Game.hpp"
#include "Offsets.hpp"

Portal2::Portal2() {
	this->version = SourceGame_Portal2;
	Game::mapNames = {
		"sp_a1_intro1",
		"sp_a1_intro2",
		"sp_a1_intro3",
		"sp_a1_intro4",
		"sp_a1_intro5",
		"sp_a1_intro6",
		"sp_a1_intro7",
		"sp_a1_wakeup",
		"sp_a2_intro",
		"sp_a2_laser_intro",
		"sp_a2_laser_stairs",
		"sp_a2_dual_lasers",
		"sp_a2_laser_over_goo",
		"sp_a2_catapult_intro",
		"sp_a2_trust_fling",
		"sp_a2_pit_flings",
		"sp_a2_fizzler_intro",
		"sp_a2_sphere_peek",
		"sp_a2_ricochet",
		"sp_a2_bridge_intro",
		"sp_a2_bridge_the_gap",
		"sp_a2_turret_intro",
		"sp_a2_laser_relays",
		"sp_a2_turret_blocker",
		"sp_a2_laser_vs_turret",
		"sp_a2_pull_the_rug",
		"sp_a2_column_blocker",
		"sp_a2_laser_chaining",
		"sp_a2_triple_laser",
		"sp_a2_bts1",
		"sp_a2_bts2",
		"sp_a2_bts3",
		"sp_a2_bts4",
		"sp_a2_bts5",
		"sp_a2_bts6",
		"sp_a2_core",
		"sp_a3_00",
		"sp_a3_01",
		"sp_a3_03",
		"sp_a3_jump_intro",
		"sp_a3_bomb_flings",
		"sp_a3_crazy_box",
		"sp_a3_transition01",
		"sp_a3_speed_ramp",
		"sp_a3_speed_flings",
		"sp_a3_portal_intro",
		"sp_a3_end",
		"sp_a4_intro",
		"sp_a4_tb_intro",
		"sp_a4_tb_trust_drop",
		"sp_a4_tb_wall_button",
		"sp_a4_tb_polarity",
		"sp_a4_tb_catch",
		"sp_a4_stop_the_box",
		"sp_a4_laser_catapult",
		"sp_a4_laser_platform",
		"sp_a4_speed_catch",
		"sp_a4_jump_polarity",
		"sp_a4_finale1",
		"sp_a4_finale2",
		"sp_a4_finale3",
		"sp_a4_finale4",
	};
}
void Portal2::LoadOffsets() {
	using namespace Offsets;

	// engine.so

	Dtor = 0;                              // ConVar
	InternalSetValue = 19;                 // ConVar
	InternalSetFloatValue = 20;            // ConVar
	InternalSetIntValue = 21;              // ConVar
	Create = 25;                           // ConVar
	GetScreenSize = 5;                     // CEngineClient
	ClientCmd = 7;                         // CEngineClient
	GetClientStateFunction = 16;           // CEngineClient::ClientCmd
	Cbuf_AddText = 45;                     // CEngineClient::ClientCmd
	IsPaused = 86;                         // CEngineClient
	Con_IsVisible = 11;                    // CEngineClient
	GetLevelNameShort = 53;                // CEngineClient
	s_CommandBuffer = 69;                  // Cbuf_AddText
	CCommandBufferSize = 9556;             // Cbuf_AddText
	m_bWaitEnabled = 8265;                 // CCommandBuffer::AddText
	GetLocalPlayer = 12;                   // CEngineClient
	GetViewAngles = 18;                    // CEngineClient
	SetViewAngles = 19;                    // CEngineClient
	GetLocalClient = 98;                   // CEngineClient::SetViewAngles
	viewangles = 19012;                    // CEngineClient::SetViewAngles
	GetMaxClients = 20;                    // CEngineClient
	GetGameDirectory = 35;                 // CEngineClient
	GetSaveDirName = 124;                  // CEngineClient
	ExecuteClientCmd = 104;                // CEngineClient
	GetActiveSplitScreenPlayerSlot = 127;  // CEngineClient
	GetSteamAPIContext = 178;              // CEngineClient
	StringToButtonCode = 31;               // CInputSystem
	SleepUntilInput = 33;                  // CInputSystem
	IsButtonDown = 14;                     //CInputSystem
	GetCursorPosition = 45;                //CInputSystem
	SetCursorPosition = 38;                //CInputSystem
	GetRecordingTick = 1;                  // CDemoRecorder
	net_time = 24;                         // CDemoRecorder::GetRecordingTick
	SetSignonState = 3;                    // CDemoRecorder
	StartRecording = 2;                    // CDemoRecorder
	StopRecording = 7;                     // CDemoRecorder
	RecordCustomData = 14;                 // CDemoRecorder
	RecordCommand = 8;                     // CDemoRecorder
	GetPlaybackTick = 4;                   // CDemoPlayer
	StartPlayback = 6;                     // CDemoPlayer
	StopPlayback = 17;                     // CDemoPlayer
	IsPlayingBack = 7;                     // CDemoPlayer
	IsPlaybackPaused = 8;                  // CDemoPlayer
	IsSkipping = 10;                       // CDemoPlayer
	SkipToTick = 14;                       // CDemoPlayer
	m_szFileName = 4;                      // CDemoPlayer::SkipToTick
	m_szDemoBaseName = 1344;               // CDemoRecorder::StartupDemoFile
	m_nDemoNumber = 1608;                  // CDemoRecorder::StartupDemoFile
	m_bRecording = 1606;                   // CDemoRecorder::SetSignonState
	IsGameUIVisible = 2;                   // IEngineGui
	Paint = 15;                            // CEngineVGui
	ProcessTick = 12;                      // CClientState
	tickcount = 74;                        // CClientState::ProcessTick
	interval_per_tick = 82;                // CClientState::ProcessTick
	HostState_OnClientConnected = 1503;    // CClientState::SetSignonState
	hoststate = 9;                         // HostState_OnClientConnected
	Disconnect = 37;                       //  CClientState
	demoplayer = 92;                       // CClientState::Disconnect
	demorecorder = 105;                    // CClientState::Disconnect
	GetCurrentMap = 26;                    // CEngineTool
	HostFrameTime = 40;                    //CEngineTool
	ClientTime = 48;                       //CEngineTool
	ClientTick = 50;                       // CEngineTool
	ServerTick = 46;                       // CEngineTool
	HostTick = 42;                         // CEngineTool
	m_szLevelName = 64;                    // CEngineTool::GetCurrentMap
	PrecacheModel = 61;                    // CEngineTool::PrecacheModel
	AddListener = 4;                       // CGameEventManager
	RemoveListener = 6;                    // CGameEventManager
	FireEventClientSide = 9;               // CGameEventManager
	FireEventIntern = 16;                  // CGameEventManager::FireEventClientSide
	ConPrintEvent = 320;                   // CGameEventManager::FireEventIntern
	AutoCompletionFunc = 46;               // listdemo_CompletionFunc
	Key_SetBinding = 73;                   // unbind
	IsRunningSimulation = 12;              // CEngineAPI
	Init = 3;                              // CEngineAPI
	eng = 7;                               // CEngineAPI::IsRunningSimulation
	Frame = 6;                             // CEngine
	m_bLoadGame = 440;                     // CGameClient::ActivatePlaye/CBaseServer::m_szLevelName
	ScreenPosition = 11;                   // CIVDebugOverlay
	AddBoxOverlay = 1;                     // CIVDebugOverlay
	AddSphereOverlay = 2;                  // CIVDebugOverlay
	AddTriangleOverlay = 3;                // CIVDebugOverlay
	AddLineOverlay = 4;                    // CIVDebugOverlay
	AddScreenTextOverlay = 7;              // CIVDebugOverlay
	ClearAllOverlays = 16;                 // CIVDebugOverlay
	MAX_SPLITSCREEN_PLAYERS = 2;           // maxplayers
	OnGameOverlayActivated = 152;          // CSteam3Client
	IsAsleep = 3;                          // IPhysicsObject
	IsCollisionEnabled = 7;                // IPhysicsObject
	IsGravityEnabled = 8;                  // IPhysicsObject
	IsDragEnabled = 9;                     // IPhysicsObject
	IsMotionEnabled = 10;                  // IPhysicsObject
	GetPosition = 49;                      // IPhysicsObject
	GetVelocity = 53;                      // IPhysicsObject
	SetPosition = 47;                      // IPhysicsObject
	SetVelocity = 51;                      // IPhysicsObject
	EnableGravity = 14;                    // IPhysicsObject
	VideoMode_Create = 120;                // CEngineAPI::Init
	videomode = 183;                       // VideoMode_Create
	GetModeWidth = 15;                     // IVideoMode
	GetModeHeight = 16;                    // IVideoMode
	ReadScreenPixels = 29;                 // IVideoMode
	snd_linear_count = 57;                 // SND_RecordBuffer
	snd_p = 65;                            // SND_RecordBuffer
	snd_vol = 71;                          // SND_RecordBuffer

	// libvstdlib.so

	RegisterConCommand = 9;            // CCVar
	UnregisterConCommand = 10;         // CCvar
	FindCommandBase = 13;              // CCvar
	InstallGlobalChangeCallback = 19;  // CCvar
	RemoveGlobalChangeCallback = 20;   // CCvar
	m_pConCommandList = 48;            // CCvar
	IsCommand = 2;                     // ConCommandBase

	// vgui2.so

	GetIScheme = 9;  // CSchemeManager
	GetFont = 4;     // CScheme

	// server.so

	ProcessMovement = 2;                 // CGameMovement
	PlayerMove = 16;                     // CPortalGameMovement
	AirAccelerate = 23;                  // CPortalGameMovement
	AirMove = 24;                        // CPortalGameMovement
	AirMove_Offset1 = 29;                // CPortalGameMovement::~CPortalGameMovement
	AirMove_Offset2 = 12;                // CGameMovement::~CGameMovement
	FinishGravity = 35;                  // CPortalGameMovement
	CheckJumpButton = 37;                // CPortalGameMovement
	FullTossMove = 38;                   // CGameMovement
	mv = 8;                              // CPortalGameMovement::CheckJumpButton
	GameFrame = 4;                       // CServerGameDLL
	GetAllServerClasses = 10;            // CServerGameDLL
	IsRestoring = 24;                    // CServerGameDLL
	Think = 31;                          // CServerGameDLL
	UTIL_PlayerByIndex = 70;             // CServerGameDLL::Think
	gpGlobals = 12;                      // UTIL_PlayerByIndex
	player = 4;                          // CPortalGameMovement::PlayerMove
	m_MoveType = 226;                    // CBasePlayer::UpdateStepSound
	m_iClassName = 104;                  // CBaseEntity
	S_m_vecAbsVelocity = 372;            // CBaseEntity
	S_m_vecAbsOrigin = 468;              // CBaseEntity
	S_m_angAbsRotation = 480;            // CBaseEntity
	m_iEFlags = 208;                     // CBaseEntity
	m_flGravity = 792;                   // CBaseEntity
	m_takedamage = 574;                  // CBaseEntity
	NUM_ENT_ENTRIES = 8192;              // CBaseEntityList::CBaseEntityList
	ENT_ENTRY_MASK = 65535;              //CBaseEntityList::CBaseEntityList
	INVALID_EHANDLE_INDEX = 0xFFFFFFFF;  //CBaseEntityList::CBaseEntityList
	NUM_SERIAL_NUM_SHIFT_BITS = 16;      //CBaseEntityList::CBaseEntityList
	GetIServerEntity = 2;                // CServerTools
	m_EntPtrArray = 48;                  // CServerTools::GetIServerEntity
	SetKeyValueChar = 12;                // CServerTools::SetKeyValue (const char *szValue)
	SetKeyValueFloat = 13;               // CServerTools::SetKeyValue (float flValue )
	SetKeyValueVector = 14;              // CServerTools::SetKeyValue (const Vector &vecValue )
	CreateEntityByName = 15;             // CServerTools::CreateEntityByName
	DispatchSpawn = 16;                  //CServerTool::DispatchSpawn
	ClientCommand = 39;                  // CVEngineServer
	IsServerPaused = 81;                 // CVEngineServer
	ServerPause = 121;                   // CVEngineServer
	TraceRay = 5;                        // IEngineTrace
	IsPlayer = 86;                       // CBasePlayer
	AcceptInput = 41;                    // CBasePlayer
	m_pSurfaceData = 4116;               // CGameMovement::CheckJumpButton
	jumpFactor = 72;                     // CGameMovement::CheckJumpButton
	m_pShadowStand = 3184;               // CBasePlayer
	m_pShadowCrouch = 3188;              // CBasePlayer
	m_Local = 5084;                      // CBasePlayer
	m_surfaceFriction = 4120;            // CBasePlayer
	m_nTractorBeamCount = 396;           // CPlayerLocalData
	m_hTractorBeam = 392;                // CPlayerLocalData
	GetPaintPower = 513;                 // CPortal_Player
	UseSpeedPower = 519;                 // CPortal_Player
	StartTouch = 103;

	// client.so

	GetAllClasses = 8;            // CHLClient
	LevelInitPreEntity = 5;       // CHLClient
	HudProcessInput = 10;         // CHLClient
	C_m_vecAbsOrigin = 136;       // C_BasePlayer::GetAbsOrigin
	C_m_angAbsRotation = 172;     // C_BasePlayer::GetAbsAngles
	GetClientEntity = 3;          // CClientEntityList
	GetClientMode = 21;           // CHLClient::HudProcessInput
	g_pClientMode = 25;           // GetClientMode
	CreateMove = 25;              // ClientModeShared
	GetName = 11;                 // CHud
	ShouldDraw = 12;              // CHUDQuickInfo
	GetHud = 146;                 // cc_leaderboard_enable
	FindElement = 161;            // cc_leaderboard_enable
	ChatPrintf = 25;              // CBaseHudChat
	MsgFunc_SayText2 = 32;        // CBaseHudChat
	DecodeUserCmdFromBuffer = 7;  // CInput
	PerUserInput_tSize = 352;     // CInput::DecodeUserCmdFromBuffer
	m_pCommands = 244;            // CInput::DecodeUserCmdFromBuffer
	CUserCmdSize = 96;            // CInput::DecodeUserCmdFromBuffer
	MULTIPLAYER_BACKUP = 150;     // CInput::DecodeUserCmdFromBuffer
	IN_ActivateMouse = 15;        // CHLClient
	IN_DeactivateMouse = 16;      // CHLClient
	g_Input = 1;                  // CHLClient::IN_ActivateMouse
	GetButtonBits = 2;            // CInput
	ActivateMouse = 27;           // CInput
	DeactivateMouse = 28;         // CInput
	SteamControllerMove = 58;     // CInput
	JoyStickApplyMovement = 64;   // CInput
	KeyDown = 208;                // CInput::JoyStickApplyMovement
	KeyUp = 284;                  // CInput::JoyStickApplyMovement
	OverrideView = 19;            // ClientModeShared

	// vguimatsurface.so

	DrawSetColor = 13;         // CMatSystemSurface
	DrawFilledRect = 15;       // CMatSystemSurface
	DrawLine = 18;             // CMatSystemSurface
	DrawColoredCircle = 159;   // CMatSystemSurface
	DrawSetTextFont = 22;      // CMatSystemSurface
	DrawSetTextColor = 24;     // CMatSystemSurface
	GetFontTall = 72;          // CMatSystemSurface
	PaintTraverseEx = 117;     // CMatSystemSurface
	StartDrawing = 559;        // CMatSystemSurface::PaintTraverseEx
	FinishDrawing = 430;       // CMatSystemSurface::PaintTraverseEx
	DrawColoredText = 160;     // CMatSystemSurface
	DrawTextLen = 163;         // CMatSystemSurface
	GetKernedCharWidth = 147;  // CMatSystemSurface
	GetFontName = 132;         // CMatSystemSurface

	DrawSetTextureFile = 35;  // CMatSystemSurface
	DrawSetTextureRGBA = 36;  // CMatSystemSurface
	DrawSetTexture = 37;      // CMatSystemSurface
	DrawGetTextureSize = 38;  // CMatSystemSurface
	DrawTexturedRect = 39;    // CMatSystemSurface
	IsTextureIDValid = 40;    // CMatSystemSurface
	CreateNewTextureID = 41;  // CMatSystemSurface
}
const char *Portal2::Version() {
	return "Portal 2 (7293)";
}
const float Portal2::Tickrate() {
	return 60;
}
const char *Portal2::ModDir() {
	return "portal2";
}
