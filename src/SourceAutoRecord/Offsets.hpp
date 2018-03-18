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

	// IEngineVGuiInternal
	int Paint;

	// IServerGameDLL
	int LevelInit;
	int LevelInit_gpGlobals;

	void Init()
	{
		if (Game::Version == Game::Portal2) {
			// engine.so
			InternalSetValue = 12;
			InternalSetFloatValue = 13;
			InternalSetIntValue = 14;
			FindVar = 15; // CCvarUtilities::IsValidToggleCommand
			m_pConCommandList = 48;
			ClientCmd = 7; // CEngineClient
			GetGameDirectory = 35; // CEngineClient
			StringToButtonCode = 31;
			GetRecordingTick = 1; // CDemoRecorder
			SetSignonState = 3; // CDemoRecorder
			StopRecording = 7; // CDemoRecorder
			GetPlaybackTick = 3; // CDemoPlayer
			StartPlayback = 6; // CDemoPlayer
			IsPlayingBack = 7; // CDemoPlayer
			m_szFileName = 4; // CDemoPlayer::SkipToTick
			m_szDemoBaseName = 1344;
			m_bIsDemoHeader = 1604;
			m_bCloseDemoFile = 1605;
			m_bRecording = 1606;
			m_nDemoNumber = 1608;
			Paint = 15; // IEngineVGuiInternal
			LevelInit = 2; // IServerGameDLL
			LevelInit_gpGlobals = 83; // IServerGameDLL::LevelInit
			Disconnect = 37; //  CClientState
			//GetClientEntity = 3; // R_BuildCubemapSamples
			HS_RUN = 4;
			HS_CHANGE_LEVEL_SP = 2;

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