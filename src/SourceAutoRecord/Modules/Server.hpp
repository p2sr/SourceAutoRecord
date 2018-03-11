#pragma once
#include "vmthook/vmthook.h"

#include "Commands.hpp"
#include "Offsets.hpp"
#include "Stats.hpp"

#define IN_JUMP	(1 << 1)
#define IN_USE	(1 << 5)

using _CheckJumpButton = bool(__cdecl*)(void* thisptr);
using _PlayerUse = int(__cdecl*)(void* thisptr);

using namespace Commands;

namespace Server
{
	namespace Hooks
	{
		std::unique_ptr<VMTHook> CheckJumpButton;
		std::unique_ptr<VMTHook> PlayerUse;
	}

	namespace Original
	{
		void* AirMove;
		void* AirMoveSkip;
		void* PlayerRunCommand;
		void* PlayerRunCommandSkip;
	}

	namespace Detour
	{
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

			bool result = Hooks::CheckJumpButton
				->GetOriginalFunction<_CheckJumpButton>(Offsets::CheckJumpButton)(thisptr);

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
				Console::PrintActive("Hello World!\n");
			}
			return Hooks::PlayerUse->GetOriginalFunction<_PlayerUse>(Offsets::PlayerUse)(thisptr);
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