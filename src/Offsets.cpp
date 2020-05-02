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
int GetActiveSplitScreenPlayerSlot;
int GetSteamAPIContext;

// CHLClient
int GetAllClasses;
int HudProcessInput;
int HudUpdate;
int IN_ActivateMouse;
int JoyStickApplyMovement;

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
int DrawLine;
int DrawSetTextFont;
int DrawSetTextColor;
int GetFontTall;
int PaintTraverseEx;
int DrawColoredText;
int DrawTextLen;

// CInputSystem
int StringToButtonCode;
int SleepUntilInput;
int IsButtonDown;
int GetCursorPosition;
int SetCursorPosition;

// CInput
int GetButtonBits;
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
int StopRecording;
int m_szDemoBaseName;
int m_bRecording;
int m_nDemoNumber;

// CDemoPlayer
int GetPlaybackTick;
int StartPlayback;
int IsPlayingBack;
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
int S_m_vecAbsOrigin;
int S_m_angAbsRotation;
int S_m_vecVelocity;
int m_iEFlags;
int m_flMaxspeed;
int m_flGravity;
int S_m_vecViewOffset;
int IsPlayer;

// CBasePlayer
int m_fFlags;
int m_MoveType;
int m_nWaterLevel;
int m_bDucked;
int m_flFriction;
int m_pSurfaceData;

// CPortal_Player
int iNumPortalsPlaced;

// IEngineVGuiInternal
int IsGameUIVisible;
int Paint;

// CEngineTool
int GetCurrentMap;
int HostFrameTime;
int ClientTime;

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

// CIVDebugOverlay
int ScreenPosition;

// CCommandBuffer
int m_bWaitEnabled;

// CServerTools
int GetIServerEntity;

// CVEngineServer
int ClientCommand;

// CSteam3Client
int OnGameOverlayActivated;

// surfacedata_t
int jumpFactor;

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
}
