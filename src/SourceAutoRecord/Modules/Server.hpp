#pragma once
#include "Commands.hpp"
#include "Offsets.hpp"
#include "Stats.hpp"

#define IN_JUMP	(1 << 1)
#define IN_USE	(1 << 5)

using _CheckJumpButton = bool(__thiscall*)(void* thisptr);
using _PlayerUse = int(__thiscall*)(void* thisptr);

using namespace Commands;

// server.dll
namespace Server
{
	namespace Original
	{
		_CheckJumpButton CheckJumpButton;
		_PlayerUse PlayerUse;
	}

	namespace Detour
	{
		bool CantJumpNextTime = false;

		bool __fastcall CheckJumpButton(void* thisptr, int edx)
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

			bool result = Original::CheckJumpButton(thisptr);

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
		int __fastcall PlayerUse(void* thisptr, int edx)
		{
			int* m_afButtonPressed = (int*)((uintptr_t)thisptr + Offsets::m_afButtonPressed);
			if (*m_afButtonPressed & IN_USE) {
				Stats::TotalUses++;
			}
			return Original::PlayerUse(thisptr);
		}
	}
}