#pragma once

#include "Game.hpp"

namespace Offsets
{
	// CCvar
	int FindVar;
	int m_pConCommandList;

	// CEngineClient
	int ClientCmd;
	int GetLocalPlayer;
	int GetGameDirectory;
	int GetViewAngles;
	int SetViewAngles;

	// CHLClient
	int HudUpdate;

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
	int CheckJumpButton;
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
	int GetClientState;
	int demoplayer;
	int demorecorder;
	int m_szLevelName;
	int UTIL_PlayerByIndex;
	int gpGlobals;

	void Init()
	{
		if (Game::Version == Game::Portal2) {
			// engine.so
			InternalSetValue = 19; // ConVar
			InternalSetFloatValue = 20; // ConVar
			InternalSetIntValue = 21; // ConVar
			FindVar = 15; // CCvarUtilities::IsValidToggleCommand
			ClientCmd = 7; // CEngineClient
			GetLocalPlayer = 12; // CEngineClient
			GetGameDirectory = 35; // CEngineClient
			GetViewAngles = 18; // CEngineClient
			SetViewAngles = 19; // CEngineClient
			StringToButtonCode = 31; // ReadCheatCommandsFromFile
			GetRecordingTick = 1; // CDemoRecorder
			SetSignonState = 3; // CDemoRecorder
			StopRecording = 7; // CDemoRecorder
			GetPlaybackTick = 3; // CDemoPlayer
			StartPlayback = 6; // CDemoPlayer
			IsPlayingBack = 7; // CDemoPlayer
			m_szFileName = 4; // CDemoPlayer::SkipToTick
			m_szDemoBaseName = 1344; // CDemoRecorder::StartupDemoFile
			m_nDemoNumber = 1608; // CDemoRecorder::StartupDemoFile
			m_bRecording = 1606; // CDemoRecorder::SetSignonState
			Paint = 15; // IEngineVGuiInternal
			ProcessTick = 12; // CClientState
			tickcount = 73; // CClientState::ProcessTick
			interval_per_tick = 81; // CClientState::ProcessTick
			Disconnect = 37; //  CClientState
			GetClientState = 11; // CEngineClient::ClientCmd
			GetCurrentMap = 26; // CEngineTool
			demoplayer = 93; // CClientState::Disconnect
			demorecorder = 106; // CClientState::Disconnect
			m_szLevelName = 72; // CEngineTool::GetCurrentMap
			GetIScheme = 9; // CSchemeManager
			GetFont = 4; // CScheme
			// server.so
			PlayerMove = 16; // CPortalGameMovement
			CheckJumpButton = 37; // CPortalGameMovement
			mv = 8; // CPortalGameMovement::CheckJumpButton
			m_nOldButtons = 40; // CPortalGameMovement::CheckJumpButton
			Think = 31; // CServerGameDLL
			UTIL_PlayerByIndex = 61; // CServerGameDLL::Think
			iNumPortalsPlaced = 5724; // CPortal_Player::IncrementPortalsPlaced
			gpGlobals = 467; // CGameMovement::FullTossMove
			player = 4; // CPortalGameMovement::PlayerMove
			m_fFlags = 212; // CBasePlayer::UpdateStepSound
			m_MoveType = 226; // CBasePlayer::UpdateStepSound
			m_nWaterLevel = 347; // CBasePlayer::UpdateStepSound
			psurface = 4116; // CPortalGameMovement::PlayerMove
			m_vecVelocity2 = 64; // CPortalGameMovement::PlayerMove
			frametime = 16; // CBasePlayer::UpdateStepSound
			// client.so
			HudUpdate = 11; // CHLClient
			GetFontTall = 72; // CFPSPanel::ComputeSize
			DrawColoredText = 160; // CFPSPanel::Paint
			m_vecAbsOrigin = 136; // C_BasePlayer::GetAbsOrigin
			m_angAbsRotation = 172; // C_BasePlayer::GetAbsAngles
			m_vecVelocity = 244; // CFPSPanel::Paint
			GetClientEntity = 3; // IClientEntityList
			// libvstdlib.so
			m_pConCommandList = 48; // CCvar::RegisterConCommand
		}
	}
}