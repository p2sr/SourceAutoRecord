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

	// AutoJump
	int mv;
	int m_nOldButtons;

	// DemoRecorder
	int m_szDemoBaseName;
	int m_bIsDemoHeader;
	int m_bCloseDemoFile;
	int m_bRecording;
	int m_nDemoNumber;

	void Init(int variant) {
		Console::DevMsg("SAR: Setting variant value to %i!\n", variant);
		switch (Variant = variant) {
		case 0:	// Portal 2 6879
			SetValueString = 12;
			SetValueFloat = 13;
			SetValueInt = 14;
			FindVar = 16;
			ClientCommand = 7;
			mv = 2;
			m_nOldButtons = 40;
			DrawColoredText = 160;
			m_szDemoBaseName = 1344;
			m_bIsDemoHeader = 1604;
			m_bCloseDemoFile = 1605;
			m_bRecording = 1606;
			m_nDemoNumber = 1608;
			break;
		case 1: // INFRA 6905
			SetValueString = 14;
			SetValueFloat = 15;
			SetValueInt = 16;
			FindVar = 16;
			ClientCommand = 7; // TODO
			mv = 2;
			m_nOldButtons = 40;
			DrawColoredText = 160; // TODO
			m_szDemoBaseName = 1344; // TODO
			m_bIsDemoHeader = 1604; // TODO
			m_bCloseDemoFile = 1605; // TODO
			m_bRecording = 1606; // TODO
			m_nDemoNumber = 1608; // TODO
			break;
		}
	}
}