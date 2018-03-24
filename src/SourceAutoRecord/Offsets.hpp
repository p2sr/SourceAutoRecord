#pragma once

#include "Game.hpp"

namespace Offsets
{
	// CCvar
	int FindVar;
	int m_pConCommandList;

	// CEngineClient
	int ClientCmd;
	int GetGameDirectory;

	// CHLClient
	int HudUpdate;

	// ConVar
	int InternalSetValue;
	int InternalSetFloatValue;
	int InternalSetIntValue;

	// IMatSystemSurface
	int DrawColoredText;

	// CFPSPanel
	int m_hFont;

	// CInputSystem
	int StringToButtonCode;

	// CGameMovement
	int CheckJumpButton;
	int mv;
	int m_nOldButtons;

	// CDemoRecorder
	int GetRecordingTick;
	int SetSignonState;
	int StopRecording;
	int m_szDemoBaseName;
	int m_bIsDemoHeader;
	int m_bCloseDemoFile;
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

	// C_BasePlayer
	int GetAbsOrigin;
	int GetAbsAngles;
	int GetLocalVelocity;

	// IClientEntityList
	int GetClientEntity;

	// Others
	int HS_RUN;
	int HS_CHANGE_LEVEL_SP;
	int MainViewOrigin;
	int MainViewAngles;
	int tickcount;
	int interval_per_tick;

	// IEngineVGuiInternal
	int Paint;

	// IServerGameDLL
	int LevelInit;
	int LevelInit_gpGlobals;

	void Init()
	{
		if (Game::Version == Game::Portal2) {
			// engine.so
			InternalSetValue = 12; // tier1/convar.cpp
			InternalSetFloatValue = 13; // tier1/convar.cpp
			InternalSetIntValue = 14; // tier1/convar.cpp
			FindVar = 15; // CCvarUtilities::IsValidToggleCommand
			m_pConCommandList = 48; // vstdlib/cvar.cpp
			ClientCmd = 7; // CEngineClient
			GetGameDirectory = 35; // CEngineClient
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
			m_bIsDemoHeader = 1604; // CDemoRecorder::StartupDemoHeader
			m_bCloseDemoFile = 1605; // CDemoRecorder::SetSignonState
			m_bRecording = 1606; // CDemoRecorder::SetSignonState
			Paint = 15; // IEngineVGuiInternal
			LevelInit = 2; // IServerGameDLL
			LevelInit_gpGlobals = 83; // IServerGameDLL::LevelInit
			ProcessTick = 12; // CClientState
			tickcount = 73; // CClientState::ProcessTick
			interval_per_tick = 81; // CClientState::ProcessTick
			Disconnect = 37; //  CClientState
			//GetClientEntity = 3; // R_BuildCubemapSamples
			HS_RUN = 4; // host_state.cpp
			HS_CHANGE_LEVEL_SP = 2; // host_state.cpp

			// server.so
			CheckJumpButton = 37; // CPortalGameMovement
			mv = 2; // CPortalGameMovement::CheckJumpButton
			m_nOldButtons = 40; // CPortalGameMovement::CheckJumpButton

			// client.so
			HudUpdate = 11; // CHLClient
			DrawColoredText = 160; // CFPSPanel::Paint
			m_hFont = 344; // CFPSPanel::Paint
			GetAbsOrigin = 136; // C_BasePlayer::GetAbsOrigin
			GetAbsAngles = 172; // C_BasePlayer::GetAbsAngles
			GetLocalVelocity = 244; // CFPSPanel::Paint
			MainViewOrigin = 39; // GetPos view.cpp
			MainViewAngles = 65; // GetPos view.cpp
		}
	}
}