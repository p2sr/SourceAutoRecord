#pragma once

namespace Offsets {

// CCvar
extern int RegisterConCommand;
extern int UnregisterConCommand;
extern int FindCommandBase;
extern int InstallGlobalChangeCallback;
extern int RemoveGlobalChangeCallback;
extern int m_pConCommandList;

// ConCommandBase
extern int IsCommand;

// CEngineClient
extern int GetScreenSize;
extern int ClientCmd;
extern int GetLocalPlayer;
extern int GetGameDirectory;
extern int GetViewAngles;
extern int SetViewAngles;
extern int GetMaxClients;
extern int ServerCmdKeyValues;
extern int GetActiveSplitScreenPlayerSlot;
extern int GetSteamAPIContext;

// CHLClient
extern int GetAllClasses;
extern int HudProcessInput;
extern int HudUpdate;
extern int IN_ActivateMouse;
extern int JoyStickApplyMovement;

// ClientModeShared
extern int CreateMove;

// ConVar
extern int Dtor;
extern int InternalSetValue;
extern int InternalSetFloatValue;
extern int InternalSetIntValue;
extern int Create;

// CMatSystemSurface
extern int DrawSetColor;
extern int DrawFilledRect;
extern int DrawLine;
extern int DrawSetTextFont;
extern int DrawSetTextColor;
extern int GetFontTall;
extern int PaintTraverseEx;
extern int DrawColoredText;
extern int DrawTextLen;

// CInputSystem
extern int StringToButtonCode;
extern int SleepUntilInput;

// CInput
extern int GetButtonBits;
extern int DecodeUserCmdFromBuffer;

// CGameMovement
extern int PlayerMove;
extern int AirAccelerate;
extern int AirMove;
extern int FinishGravity;
extern int CheckJumpButton;
extern int FullTossMove;
extern int mv;
extern int player;
extern int ProcessMovement;

// CDemoRecorder
extern int GetRecordingTick;
extern int SetSignonState;
extern int StopRecording;
extern int m_szDemoBaseName;
extern int m_bRecording;
extern int m_nDemoNumber;

// CDemoPlayer
extern int GetPlaybackTick;
extern int StartPlayback;
extern int IsPlayingBack;
extern int m_szFileName;

// CClientState
extern int ProcessTick;
extern int Disconnect;
extern int viewangles;

// C_BaseEntity
extern int C_m_vecAbsOrigin;
extern int C_m_angAbsRotation;
extern int C_m_vecVelocity;
extern int C_m_vecViewOffset;

// CBaseEntity
extern int S_m_vecAbsOrigin;
extern int S_m_angAbsRotation;
extern int S_m_vecVelocity;
extern int m_iEFlags;
extern int m_flMaxspeed;
extern int m_flGravity;
extern int S_m_vecViewOffset;
extern int IsPlayer;

// CBasePlayer
extern int m_fFlags;
extern int m_MoveType;
extern int m_nWaterLevel;
extern int m_bDucked;
extern int m_flFriction;
extern int m_pSurfaceData;

// CPortal_Player
extern int iNumPortalsPlaced;

// IEngineVGuiInternal
extern int Paint;

// CEngineTool
extern int GetCurrentMap;

// CSchemeManager
extern int GetIScheme;

// CScheme
extern int GetFont;

// IClientEntityList
extern int GetClientEntity;

// CServerGameDLL
extern int GameFrame;
extern int Think;
extern int GetAllServerClasses;
extern int IsRestoring;

// CHud
extern int GetName;

// CGameEventManager
extern int AddListener;
extern int RemoveListener;
extern int FireEventClientSide;
extern int FireEventIntern;
extern int ConPrintEvent;

// CEngine
extern int Frame;

// CEngineAPI
extern int IsRunningSimulation;

// CIVDebugOverlay
extern int ScreenPosition;

// CCommandBuffer
extern int m_bWaitEnabled;

// CServerTools
extern int GetIServerEntity;

// CVEngineServer
extern int ClientCommand;

// CSteam3Client
extern int OnGameOverlayActivated;

// surfacedata_t
extern int jumpFactor;

// Others
extern int tickcount;
extern int interval_per_tick;
extern int GetClientStateFunction;
extern int cl;
extern int demoplayer;
extern int demorecorder;
extern int m_szLevelName;
extern int AirMove_Offset1;
extern int AirMove_Offset2;
extern int UTIL_PlayerByIndex;
extern int gpGlobals;
extern int g_Input;
extern int in_jump;
extern int KeyDown;
extern int KeyUp;
extern int GetClientMode;
extern int State_Shutdown;
extern int Cbuf_AddText;
extern int s_CommandBuffer;
extern int CCommandBufferSize;
extern int AddText;
extern int AutoCompletionFunc;
extern int StartDrawing;
extern int FinishDrawing;
extern int GetHud;
extern int FindElement;
extern int Key_SetBinding;
extern int eng;
extern int HostState_OnClientConnected;
extern int hoststate;
extern int m_bLoadGame;
extern int NUM_ENT_ENTRIES;
extern int m_iClassName;
extern int m_iName;
extern int m_EntPtrArray;
extern int g_pClientMode;
extern int m_pCommands;
extern int CUserCmdSize;
extern int MULTIPLAYER_BACKUP;
extern int PerUserInput_tSize;
extern int GetLocalClient;
extern int MAX_SPLITSCREEN_PLAYERS;
extern int net_time;
}
