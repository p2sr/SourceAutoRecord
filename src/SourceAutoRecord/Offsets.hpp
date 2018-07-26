#pragma once

namespace Offsets {

// CCvar
int UnregisterConCommand;
int FindCommandBase;

// CEngineClient
int GetScreenSize;
int ClientCmd;
int GetLocalPlayer;
int GetGameDirectory;
int GetViewAngles;
int SetViewAngles;
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
int GetFontTall;
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
int AirMove_Offset1;
int AirMove_Offset2;
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
int Think;

// CGlobalVarsBase
int frametime;

// CMoveData
int m_nOldButtons;
int m_vecVelocity2;

// CHud
int GetName;

// CGameEventManager
int AddListener;
int RemoveListener;

// CBaseEntity
int m_iClassname;
int m_iName;

// CBaseEntityOutput
int m_ActionList;

// CEngine
int Frame;

// Others
int tickcount;
int interval_per_tick;
int GetClientStateFunction;
int cl;
int demoplayer;
int demorecorder;
int m_szLevelName;
int UTIL_PlayerByIndex;
int gpGlobals;
int g_Input;
int in_jump;
int KeyDown;
int KeyUp;
int GetHud;
int GetClientMode;
int hoststate;
int FrameUpdate;
int State_Shutdown;
int eng;
int Cbuf_AddText;
int s_CommandBuffer;
int AddText;
}
