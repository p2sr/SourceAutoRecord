#pragma once
#include <string>

namespace Offsets
{
	int Game;

	// CCvar
	int FindVar;

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

	// Others
	int HS_RUN;
	int HS_CHANGE_LEVEL_SP;
	int AirMoveSkip;
	int PlayerRunCommandSkip;

	bool Init()
	{
		TCHAR temp[MAX_PATH];
		GetModuleFileName(NULL, temp, _countof(temp));
		std::string exe = std::string(temp);
		int index = exe.find_last_of("\\/");
		exe = exe.substr(index + 1, exe.length() - index);

		// Portal 2 6879
		if (exe == "portal2.exe") {
			Game = 0;
			InternalSetValue = 12;
			InternalSetFloatValue = 13;
			InternalSetIntValue = 14;
			FindVar = 16;
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
			m_afButtonPressed = 2860;
			HS_RUN = 4;
			HS_CHANGE_LEVEL_SP = 2;
			AirMoveSkip = 142;
			PlayerRunCommandSkip = 51;
		}
		// INFRA 6905
		else if (exe == "infra.exe") {
			Game = 1;
			InternalSetValue = 14;
			InternalSetFloatValue = 15;
			InternalSetIntValue = 16;
			FindVar = 16;
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
			m_afButtonPressed = 2900;
			HS_RUN = 5;
			HS_CHANGE_LEVEL_SP = 3;
			AirMoveSkip = 162;
			//PlayerRunCommandSkip = 132;
		}
		else {
			return false;
		}
		return true;
	}
	const char* GetGame() {
		switch (Game) {
		case 0:
			return "Portal 2 (6879)";
		case 1:
			return "INFRA (6905)";
		}
		return "Unknown";
	}
}