#include "Offsets.hpp"

namespace Offsets {

// CCvar
int RegisterConCommand;
int UnregisterConCommand;
int FindCommandBase;
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

// CHLClient
int HudProcessInput;
int HudUpdate;
int IN_ActivateMouse;
int JoyStickApplyMovement;

// ClientModeShared
int CreateMove;

// ConVar
int InternalSetValue;
int InternalSetFloatValue;
int InternalSetIntValue;

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

// CInput
int GetButtonBits;

// CGameMovement
int PlayerMove;
int AirAccelerate;
int AirMove;
int FinishGravity;
int CheckJumpButton;
int FullTossMove;
int mv;
int player;

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

// C_BaseEntity
int m_vecAbsOrigin;
int m_angAbsRotation;
int m_vecVelocity;
int GetFlags;

// CBaseEntity
int m_vecAbsOrigin2;
int m_angAbsRotation2;
int m_vecVelocity3;
int m_iEFlags;
int m_flMaxspeed;
int m_flGravity;
int m_vecViewOffset;

// CBasePlayer
int iNumPortalsPlaced;
int m_fFlags;
int m_MoveType;
int m_nWaterLevel;
int m_bDucked;

// IEngineVGuiInternal
int Paint;

// CEngineTool
int GetCurrentMap;

// CSchemeManager
int GetIScheme;

// CScheme
int GetFont;

// IClientEntityList
int GetClientEntity;

// CServerGameDLL
int GameFrame;
int Think;

// CMoveData
int m_nOldButtons;
int m_vecVelocity2;

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
int AddText;
int g_InRestore;
int ServiceEventQueue;
int g_EventQueue;
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
int cmd_alias;
int NUM_ENT_ENTRIES;
int m_iClassName;
int m_iName;
}
