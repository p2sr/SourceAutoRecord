#pragma once
#include <string>

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
	int m_afButtonPressed;
	int GetAbsOrigin;
	int GetAbsAngles;
	int GetLocalVelocity;

	// Others
	int HS_RUN;
	int HS_CHANGE_LEVEL_SP;
	int AirMoveSkip;
	int PlayerRunCommandSkip;
	int MainViewOrigin;
	int MainViewAngles;

	void Init()
	{
		if (Game::Version == Game::Portal2) {
			InternalSetValue = 12;
			InternalSetFloatValue = 13;
			InternalSetIntValue = 14;
			FindVar = 16;
			m_pConCommandList = 48;
			ClientCommand = 7;
			StringToButtonCode = 31;
			GetRecordingTick = 1;
			GetPlaybackTick = 3;
			IsPlayingBack = 6;
			tickcount = 12;
			interval_per_tick = 16;
			mv = 2;
			m_nOldButtons = 40;
			DrawColoredText = 160;
			m_hFont = 348;
			m_szDemoBaseName = 1344;
			m_bIsDemoHeader = 1604;
			m_bCloseDemoFile = 1605;
			m_bRecording = 1606;
			m_nDemoNumber = 1608;
			m_szFileName = 4;
			HS_RUN = 4;
			HS_CHANGE_LEVEL_SP = 2;
			m_afButtonPressed = 440;
			AirMoveSkip = 372;
			PlayerRunCommandSkip = 51;
			GetAbsOrigin = 156;
			GetAbsAngles = 192;
			GetLocalVelocity = 264;
			MainViewOrigin = 31;
			MainViewAngles = 59;
		}
	}
}