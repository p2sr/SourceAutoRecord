#pragma once
#include "../Commands.h"
#include "Offsets.h"

#define IN_JUMP (1 << 1)

using _CheckJumpButton = int(__thiscall*)(void* thisptr);

// server.dll
namespace Server
{
	_CheckJumpButton Original_CheckJumpButton;
	bool cantJumpNextTime = false;

	int __fastcall Detour_CheckJumpButton(void* thisptr, int edx)
	{
		int *pM_nOldButtons = NULL;
		int origM_nOldButtons = 0;

		if (Commands::SvCheats.GetBool() && Commands::AutoJump.GetBool()) {
			pM_nOldButtons = (int*)(*((uintptr_t*)thisptr + Offsets::mv) + Offsets::m_nOldButtons);
			origM_nOldButtons = *pM_nOldButtons;

			if (!cantJumpNextTime)
				*pM_nOldButtons &= ~IN_JUMP;
		}
		cantJumpNextTime = false;

		bool result = Original_CheckJumpButton(thisptr);

		if (Commands::SvCheats.GetBool() && Commands::AutoJump.GetBool()) {
			if (!(*pM_nOldButtons & IN_JUMP))
				*pM_nOldButtons = origM_nOldButtons;
		}

		if (result) cantJumpNextTime = true;
		return result;
	}
}