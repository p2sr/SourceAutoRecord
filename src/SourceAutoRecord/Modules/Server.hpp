#pragma once

#include "Commands.hpp"
#include "Offsets.hpp"
#include "Stats.hpp"

#define IN_JUMP	(1 << 1)
#define IN_USE	(1 << 5)

using _CheckJumpButton = bool(__thiscall*)(void* thisptr);
using _PlayerUse = int(__thiscall*)(void* thisptr);

using namespace Commands;

namespace Server
{
	namespace Original
	{
		_CheckJumpButton CheckJumpButton;
		_PlayerUse PlayerUse;
		void* AirMove;
		void* AirMoveSkip;
		void* PlayerRunCommand;
		void* PlayerRunCommandSkip;
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
		// TODO
		__attribute__((naked)) void AirMove()
		{
			__asm("pushal");
			__asm("pushfl");

			if ((!sv_bonus_challenge.GetBool() || sv_cheats.GetBool()) && sar_aircontrol.GetBool()) {
				__asm("popfl");
				__asm("popal");
				__asm("jmp *%0" : : "r"(Original::AirMoveSkip));
			}

			__asm("popfl");
			__asm("popal");
			// ucomiss xmm2, ds:dword_B8A1F0 (43160000h = 1125515264)
			__asm("ucomiss %ds:0x43160000, %xmm2");
			__asm("jmp *%0" : : "r"(Original::AirMove));
		}
		__attribute__((naked)) void PlayerRunCommand()
		{
			__asm("pushal");
			__asm("pushfl");

			if (sar_never_delay_start.GetBool()) {
				__asm("popfl");
				__asm("popal");
				__asm("jmp *%0" : : "r"(Original::PlayerRunCommandSkip));
			}

			__asm("popfl");
			__asm("popal");
			// mov dword ptr [esi+18h], 0
			__asm("movl $0, 0x18(%esi)");
			__asm("jmp *%0" : : "r"(Original::PlayerRunCommand));
		}
	}
}