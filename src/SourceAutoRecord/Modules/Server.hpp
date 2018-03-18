#pragma once
#include "vmthook/vmthook.h"

#include "Commands.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Stats.hpp"
#include "Utils.hpp"

#define IN_JUMP	(1 << 1)

using namespace Commands;

namespace Server
{
	using _CheckJumpButton = bool(__cdecl*)(void* thisptr);
	
	std::unique_ptr<VMTHook> g_GameMovement;

	namespace Original
	{
		_CheckJumpButton CheckJumpButton;
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
	}

	void Hook()
	{
		if (Interfaces::IGameMovement) {
			g_GameMovement = std::make_unique<VMTHook>(Interfaces::IGameMovement);
			g_GameMovement->HookFunction((void*)Detour::CheckJumpButton, Offsets::CheckJumpButton);
			Original::CheckJumpButton = g_GameMovement->GetOriginalFunction<_CheckJumpButton>(Offsets::CheckJumpButton);
		}
	}
}