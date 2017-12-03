#pragma once

namespace Offsets
{
	int Variant;

	// VFT
	int FindVar;
	int ClientCommand;
	int SetValueString;
	int SetValueFloat;
	int SetValueInt;
	int DrawColoredText;
	int StringToButtonCode;
	int GetRecordingTick;
	int IsPlayingBack;

	// AutoJump
	int mv;
	int m_nOldButtons;

	// DemoRecorder
	int m_szDemoBaseName;
	int m_bIsDemoHeader;
	int m_bCloseDemoFile;
	int m_bRecording;
	int m_nDemoNumber;

	// DemoPlayer
	int m_szFileName;

	void Init(int variant) {
		Console::DevMsg("SAR: Offsets will be set correctly for variant: %i!\n", variant);
		switch (Variant = variant) {
		case 0:	// Portal 2 6879
			SetValueString = 12;
			SetValueFloat = 13;
			SetValueInt = 14;
			FindVar = 16;
			ClientCommand = 7;
			StringToButtonCode = 31;
			GetRecordingTick = 1;
			IsPlayingBack = 6;
			mv = 2;
			m_nOldButtons = 40;
			DrawColoredText = 160;
			m_szDemoBaseName = 1344;
			m_bIsDemoHeader = 1604;
			m_bCloseDemoFile = 1605;
			m_bRecording = 1606;
			m_nDemoNumber = 1608;
			m_szFileName = 4;
			break;
		case 1: // INFRA 6905
			SetValueString = 14;
			SetValueFloat = 15;
			SetValueInt = 16;
			FindVar = 16;
			ClientCommand = 7; // TODO
			StringToButtonCode = 31; // TODO
			GetRecordingTick = 1; // TODO
			IsPlayingBack = 6; // TODO
			mv = 2;
			m_nOldButtons = 40;
			DrawColoredText = 160; // TODO
			m_szDemoBaseName = 1344; // TODO
			m_bIsDemoHeader = 1604; // TODO
			m_bCloseDemoFile = 1605; // TODO
			m_bRecording = 1606; // TODO
			m_nDemoNumber = 1608; // TODO
			m_szFileName = 4; // TODO
			break;
		}
	}
	const char* GetVariant() {
		switch (Variant) {
		case 0:
			return "Portal 2 (6879)";
		case 1:
			return "INFRA (6905)";
		}
		return "Unknown";
	}
}