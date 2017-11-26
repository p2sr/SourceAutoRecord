#pragma once

namespace Offsets
{
	int Variant;

	// VFT
	int FindVar;
	int SetValueString;
	int SetValueFloat;
	int SetValueInt;

	// CheckJumpButton
	int mv;
	int m_nOldButtons;

	// Paint
	int DrawColoredText;

	void Init(int variant)
	{
		switch (Variant = variant) {
		case 0:	// Portal 2 6879
			SetValueString = 12;
			SetValueFloat = 13;
			SetValueInt = 14;
			FindVar = 16;
			mv = 2;
			m_nOldButtons = 40;
			DrawColoredText = 160;//640;
			break;
		case 1: // INFRA 6905
			SetValueString = 14;
			SetValueFloat = 15;
			SetValueInt = 16;
			FindVar = 16;
			mv = 2;
			m_nOldButtons = 40;
			DrawColoredText = 160;
			break;
		}
	}
}