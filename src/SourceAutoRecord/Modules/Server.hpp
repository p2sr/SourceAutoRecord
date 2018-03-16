#pragma once
#include "vmthook/vmthook.h"

#include "Commands.hpp"
#include "Offsets.hpp"
#include "Stats.hpp"

#define IN_JUMP	(1 << 1)

using _CheckJumpButton = bool(__cdecl*)(void* thisptr);

using namespace Commands;

namespace Server
{
	std::unique_ptr<VMTHook> g_GameMovement;

	bool CantJumpNextTime = false;

	bool __cdecl CheckJumpButton(void* thisptr)
	{
		int* m_nOldButtons = NULL;
		int original = 0;

		if ((!sv_bonus_challenge.GetBool() || sv_cheats.GetBool()) && sar_autojump.GetBool()) {
			m_nOldButtons = (int*)(*((uintptr_t*)thisptr + Offsets::mv) + Offsets::m_nOldButtons);
			original = *m_nOldButtons;

			if (!CantJumpNextTime)
				*m_nOldButtons &= ~IN_JUMP;
		}
		CantJumpNextTime = false;

		bool result = g_GameMovement->GetOriginalFunction<_CheckJumpButton>(Offsets::CheckJumpButton)(thisptr);

		if ((!sv_bonus_challenge.GetBool() || sv_cheats.GetBool()) && sar_autojump.GetBool()) {
			if (!(*m_nOldButtons & IN_JUMP))
				*m_nOldButtons = original;
		}

		if (result) {
			CantJumpNextTime = true;
			Stats::TotalJumps++;
		}

		return result;
	}
}