#pragma once

namespace Offsets
{
	// CCvar
	int FindVar;

	// CEngineClient
	int ClientCmd;
	int GetLocalPlayer;
	int GetGameDirectory;
	int GetViewAngles;
	int SetViewAngles;

	// CHLClient
	int HudUpdate;

	// IClientMode
	int CreateMove;

	// ConVar
	int InternalSetValue;
	int InternalSetFloatValue;
	int InternalSetIntValue;

	// IMatSystemSurface
	int GetFontTall;
	int DrawColoredText;

	// CInputSystem
	int StringToButtonCode;

	// CGameMovement
	int PlayerMove;
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
	int psurface;

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

	// Others
	int tickcount;
	int interval_per_tick;
	int GetClientStateFunction;
	int ServerCmdKeyValues;
	int cl;
	int demoplayer;
	int demorecorder;
	int m_szLevelName;
	int UTIL_PlayerByIndex;
	int gpGlobals;
}