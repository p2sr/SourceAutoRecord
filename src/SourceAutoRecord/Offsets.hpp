#pragma once

#include "Game.hpp"

namespace Offsets
{
	// CCvar
	int FindVar;
	int m_pConCommandList;

	// CEngineClient
	int ClientCommand;

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

	// CGlobalVarsBase
	int tickcount;
	int interval_per_tick;

	// CGameMovement
	int CheckJumpButton;
	int mv;
	int m_nOldButtons;

	// CDemoRecorder
	int GetRecordingTick;
	int m_szDemoBaseName;
	int m_bIsDemoHeader;
	int m_bCloseDemoFile;
	int m_bRecording;
	int m_nDemoNumber;

	// CDemoPlayer
	int GetPlaybackTick;
	int IsPlayingBack;
	int m_szFileName;

	// C_BasePlayer
	int PlayerUse;
	int m_afButtonPressed;
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

	void Init()
	{
		if (Game::Version == Game::Portal2) {
			// engine.so
			InternalSetValue = 12;
			InternalSetFloatValue = 13;
			InternalSetIntValue = 14;
			FindVar = 15; // CCvarUtilities::IsValidToggleCommand
			m_pConCommandList = 48;
			ClientCommand = 7;
			StringToButtonCode = 31;
			GetRecordingTick = 1;
			GetPlaybackTick = 3;
			IsPlayingBack = 6;
			tickcount = 12;
			interval_per_tick = 16;
			m_szDemoBaseName = 1344;
			m_bIsDemoHeader = 1604;
			m_bCloseDemoFile = 1605;
			m_bRecording = 1606;
			m_nDemoNumber = 1608;
			m_szFileName = 4;
			GetClientEntity = 3; // R_BuildCubemapSamples
			HS_RUN = 4;
			HS_CHANGE_LEVEL_SP = 2;
			// server.so
			PlayerUse = 439; // CPortal_Player
			CheckJumpButton = 37; // CPortalGameMovement
			mv = 2; // CPortalGameMovement::CheckJumpButton
			m_nOldButtons = 40; // CPortalGameMovement::CheckJumpButton
			m_afButtonPressed = 2884; // CPortal_Player::PlayerUse
			// client.so
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