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
		void* AirMove;
		void* AirMoveSkip;
		void* PlayerRunCommand;
		void* PlayerRunCommandSkip;
	}

	void SetAirMove(uintptr_t airMoveAddr)
	{
		// Old Portal 2 bunnymod converted the else-if condition into an if
		// which checks if sv_player_funnel_into_portals is on, let's just
		// ignore that and leave it as an else-if
		Original::AirMove = (void*)(airMoveAddr + 5);
		Original::AirMoveSkip = (void*)(airMoveAddr + Offsets::AirMoveSkip);
	}
	void SetRunCommand(uintptr_t playerRunCommandAddr)
	{
		Original::PlayerRunCommand = (void*)(playerRunCommandAddr + 8);
		Original::PlayerRunCommandSkip = (void*)(playerRunCommandAddr + Offsets::PlayerRunCommandSkip);
		DoNothingAt(playerRunCommandAddr + 5, 3);
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
		__declspec(naked) void AirMove()
		{
			__asm {
				pushad
				pushfd
			}

			if ((!sv_bonus_challenge.GetBool() || sv_cheats.GetBool()) && sar_aircontrol.GetBool()) {
				__asm {
					popfd
					popad
					jmp Original::AirMoveSkip
				}
			}

			__asm {
				popfd
				popad
				movss xmm2, dword ptr[eax + 0x40]
				jmp Original::AirMove
			}
		}
		__declspec(naked) void PlayerRunCommand()
		{
			__asm {
				pushad
				pushfd
			}

			if (sar_never_delay_start.GetBool()) {
				__asm {
					popfd
					popad
					jmp Original::PlayerRunCommandSkip
				}
			}

			__asm {
				popfd
				popad
				xorps xmm0, xmm0
				movss dword ptr[esi + 0x18], xmm0
				jmp Original::PlayerRunCommand
			}
		}
	}
}