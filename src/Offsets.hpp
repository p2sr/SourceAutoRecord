#pragma once

namespace Offsets {

// CCvar
static int RegisterConCommand;
static int UnregisterConCommand;
static int FindCommandBase;

// CEngineClient
static int GetScreenSize;
static int ClientCmd;
static int GetLocalPlayer;
static int GetGameDirectory;
static int GetViewAngles;
static int SetViewAngles;
static int ServerCmdKeyValues;
static int GetActiveSplitScreenPlayerSlot;

// CHLClient
static int HudProcessInput;
static int HudUpdate;
static int IN_ActivateMouse;
static int JoyStickApplyMovement;

// ClientModeShared
static int CreateMove;

// ConVar
static int InternalSetValue;
static int InternalSetFloatValue;
static int InternalSetIntValue;

// CMatSystemSurface
static int DrawSetColor;
static int DrawFilledRect;
static int GetFontTall;
static int PaintTraverseEx;
static int DrawColoredText;
static int DrawTextLen;

// CInputSystem
static int StringToButtonCode;

// CInput
static int GetButtonBits;

// CGameMovement
static int PlayerMove;
static int AirAccelerate;
static int AirMove;
static int FinishGravity;
static int CheckJumpButton;
static int FullTossMove;
static int mv;
static int player;

// CDemoRecorder
static int GetRecordingTick;
static int SetSignonState;
static int StopRecording;
static int m_szDemoBaseName;
static int m_bRecording;
static int m_nDemoNumber;

// CDemoPlayer
static int GetPlaybackTick;
static int StartPlayback;
static int IsPlayingBack;
static int m_szFileName;

// CClientState
static int ProcessTick;
static int Disconnect;

// C_BaseEntity
static int m_vecAbsOrigin;
static int m_angAbsRotation;
static int m_vecVelocity;
static int GetFlags;

// CBasePlayer
static int iNumPortalsPlaced;
static int m_fFlags;
static int m_MoveType;
static int m_nWaterLevel;
static int m_bDucked;

// IEngineVGuiInternal
static int Paint;

// CEngineTool
static int GetCurrentMap;

// CSchemeManager
static int GetIScheme;

// CScheme
static int GetFont;

// IClientEntityList
static int GetClientEntity;

// CServerGameDLL
static int GameFrame;
static int Think;

// CMoveData
static int m_nOldButtons;
static int m_vecVelocity2;

// CHud
static int GetName;

// CGameEventManager
static int AddListener;
static int RemoveListener;

// CEngine
static int Frame;

// CEngineAPI
static int IsRunningSimulation;

// Others
static int tickcount;
static int interval_per_tick;
static int GetClientStateFunction;
static int cl;
static int demoplayer;
static int demorecorder;
static int m_szLevelName;
static int AirMove_Offset1;
static int AirMove_Offset2;
static int UTIL_PlayerByIndex;
static int gpGlobals;
static int g_Input;
static int in_jump;
static int KeyDown;
static int KeyUp;
static int GetClientMode;
static int State_Shutdown;
static int Cbuf_AddText;
static int s_CommandBuffer;
static int AddText;
static int g_InRestore;
static int ServiceEventQueue;
static int g_EventQueue;
static int AutoCompletionFunc;
static int StartDrawing;
static int FinishDrawing;
static int GetHud;
static int FindElement;
static int Key_SetBinding;
static int eng;
static int HostState_OnClientConnected;
static int hoststate;
static int m_bLoadGame;
}
