#include "Offsets.hpp"

namespace Offsets {

	// CCvar
	int RegisterConCommand;
	int UnregisterConCommand;
	int FindCommandBase;
	int InstallGlobalChangeCallback;
	int RemoveGlobalChangeCallback;
	int m_pConCommandList;

	// ConCommandBase
	int IsCommand;

	// CEngineClient
	int GetScreenSize;
	int ClientCmd;
	int GetLocalPlayer;
	int GetGameDirectory;
	int GetViewAngles;
	int SetViewAngles;
	int GetMaxClients;
	int ServerCmdKeyValues;
	int GetSaveDirName;
	int ExecuteClientCmd;
	int GetActiveSplitScreenPlayerSlot;
	int GetSteamAPIContext;
	int IsPaused;
	int Con_IsVisible;
	int GetLevelNameShort;

	// CHLClient
	int GetAllClasses;
	int HudProcessInput;
	int IN_ActivateMouse;
	int IN_DeactivateMouse;
	int SteamControllerMove;
	int JoyStickApplyMovement;
	int LevelInitPreEntity;

	// ClientModeShared
	int CreateMove;
	int OverrideView;

	// ConVar
	int Dtor;
	int InternalSetValue;
	int InternalSetFloatValue;
	int InternalSetIntValue;
	int Create;

	// CMatSystemSurface
	int DrawSetColor;
	int DrawFilledRect;
	int DrawColoredCircle;
	int DrawLine;
	int DrawSetTextFont;
	int DrawSetTextColor;
	int GetFontTall;
	int PaintTraverseEx;
	int DrawColoredText;
	int DrawTextLen;
	int GetKernedCharWidth;
	int GetFontName;

	int DrawSetTextureFile;
	int DrawSetTextureRGBA;
	int DrawSetTexture;
	int DrawGetTextureSize;
	int DrawTexturedRect;
	int IsTextureIDValid;
	int CreateNewTextureID;

	// CInputSystem
	int StringToButtonCode;
	int SleepUntilInput;
	int IsButtonDown;
	int GetCursorPosition;
	int SetCursorPosition;

	// CInput
	int GetButtonBits;
	int ActivateMouse;
	int DeactivateMouse;
	int DecodeUserCmdFromBuffer;

	// CGameMovement
	int PlayerMove;
	int AirAccelerate;
	int AirMove;
	int FinishGravity;
	int CheckJumpButton;
	int FullTossMove;
	int mv;
	int player;
	int ProcessMovement;

	// CDemoRecorder
	int GetRecordingTick;
	int SetSignonState;
	int StartRecording;
	int StopRecording;
	int RecordCustomData;
	int RecordCommand;
	int m_szDemoBaseName;
	int m_bRecording;
	int m_nDemoNumber;

	// CDemoPlayer
	int GetPlaybackTick;
	int StartPlayback;
	int StopPlayback;
	int IsPlayingBack;
	int IsPlaybackPaused;
	int IsSkipping;
	int SkipToTick;
	int m_szFileName;

	// CClientState
	int ProcessTick;
	int Disconnect;
	int viewangles;

	// C_BaseEntity
	int C_m_vecAbsOrigin;
	int C_m_angAbsRotation;
	int C_m_vecVelocity;
	int C_m_vecViewOffset;

	// CBaseEntity
	int S_m_vecAbsVelocity;
	int S_m_vecAbsOrigin;
	int S_m_angAbsRotation;
	int S_m_vecVelocity;
	int m_iEFlags;
	int m_flMaxspeed;
	int m_flGravity;
	int m_takedamage;
	int S_m_vecViewOffset;
	int IsPlayer;
	int AcceptInput;

	// CBasePlayer
	int m_fFlags;
	int m_MoveType;
	int m_nWaterLevel;
	int m_bDucked;
	int m_flFriction;
	int m_pSurfaceData;
	int m_pShadowStand;
	int m_pShadowCrouch;
	int m_Local;
	int S_m_hGroundEntity;
	int C_m_hGroundEntity;
	int m_iBonusChallenge;
	int m_surfaceFriction;
	int m_nTickBase;
	int m_InAirState;

	// CPlayerLocalData
	int m_nTractorBeamCount;
	int m_hTractorBeam;

	// CPortal_Player
	int iNumPortalsPlaced;
	int GetPaintPower;
	int UseSpeedPower;
	int S_m_StatsThisLevel;
	int C_m_StatsThisLevel;

	//CWeaponPortalgun
	int m_bCanFirePortal1;
	int m_bCanFirePortal2;
	int m_hPrimaryPortal;
	int m_hSecondaryPortal;
	int m_iPortalLinkageGroupID;

	// CProp_Portal
	int m_bActivated;
	int m_bIsPortal2;
	int m_hActiveWeapon;

	// IEngineVGuiInternal
	int IsGameUIVisible;
	int Paint;

	// IEngineTrace
	int TraceRay;

	// CEngineTool
	int GetCurrentMap;
	int HostFrameTime;
	int ClientTime;
	int PrecacheModel;
	int ClientTick;
	int ServerTick;
	int HostTick;

	// CSchemeManager
	int GetIScheme;

	// CScheme
	int GetFont;

	// IClientEntityList
	int GetClientEntity;

	// CServerGameDLL
	int GameFrame;
	int Think;
	int GetAllServerClasses;
	int IsRestoring;

	// CHud
	int GetName;

	//CHUDQuickInfo
	int ShouldDraw;

	// CGameEventManager
	int AddListener;
	int RemoveListener;
	int FireEventClientSide;
	int FireEventIntern;
	int ConPrintEvent;

	// CEngine
	int Frame;

	// CEngineAPI
	int IsRunningSimulation;
	int Init;

	// CIVDebugOverlay
	int ScreenPosition;
	int AddBoxOverlay;
	int AddSphereOverlay;
	int AddTriangleOverlay;
	int AddLineOverlay;
	int AddScreenTextOverlay;
	int ClearAllOverlays;

	// CCommandBuffer
	int m_bWaitEnabled;

	// CServerTools
	int GetIServerEntity;
	int CreateEntityByName;
	int DispatchSpawn;
	int SetKeyValueChar;
	int SetKeyValueFloat;
	int SetKeyValueVector;

	// CVEngineServer
	int ClientCommand;
	int IsServerPaused;
	int ServerPause;

	// CBaseHudChat
	int ChatPrintf;
	int MsgFunc_SayText2;

	// CSteam3Client
	int OnGameOverlayActivated;

	// surfacedata_t
	int jumpFactor;

	// IPhysicsObject
	int IsAsleep;
	int IsCollisionEnabled;
	int IsGravityEnabled;
	int IsDragEnabled;
	int IsMotionEnabled;
	int GetPosition;
	int GetVelocity;
	int SetPosition;
	int SetVelocity;
	int EnableGravity;

	// IVideoMode
	int GetModeWidth;
	int GetModeHeight;
	int ReadScreenPixels;

	// Others
	int tickcount;
	int interval_per_tick;
	int GetClientStateFunction;
	int cl;
	int demoplayer;
	int demorecorder;
	int m_szLevelName;
	int AirMove_Offset1;
	int AirMove_Offset2;
	int UTIL_PlayerByIndex;
	int gpGlobals;
	int g_Input;
	int in_jump;
	int KeyDown;
	int KeyUp;
	int GetClientMode;
	int State_Shutdown;
	int Cbuf_AddText;
	int s_CommandBuffer;
	int CCommandBufferSize;
	int AddText;
	int AutoCompletionFunc;
	int StartDrawing;
	int FinishDrawing;
	int GetHud;
	int FindElement;
	int Key_SetBinding;
	int eng;
	int HostState_OnClientConnected;
	int hoststate;
	int m_bLoadGame;
	int NUM_ENT_ENTRIES;
	int ENT_ENTRY_MASK;
	int INVALID_EHANDLE_INDEX;
	int NUM_SERIAL_NUM_SHIFT_BITS;
	int m_iClassName;
	int m_iName;
	int m_EntPtrArray;
	int g_pClientMode;
	int m_pCommands;
	int CUserCmdSize;
	int MULTIPLAYER_BACKUP;
	int PerUserInput_tSize;
	int GetLocalClient;
	int MAX_SPLITSCREEN_PLAYERS;
	int net_time;
	int VideoMode_Create;
	int videomode;
	int snd_linear_count;
	int snd_p;
	int snd_vol;
	int StartTouch;
}  // namespace Offsets
