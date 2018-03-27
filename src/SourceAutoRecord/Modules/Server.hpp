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
	using _UTIL_PlayerByIndex = void*(__cdecl*)(int index);
	
	std::unique_ptr<VMTHook> g_GameMovement;
	std::unique_ptr<VMTHook> g_ServerGameDLL;

	_UTIL_PlayerByIndex UTIL_PlayerByIndex;

	void* GetPlayer()
	{
		return UTIL_PlayerByIndex(1);
	}
	int GetPortals()
	{
		auto player = GetPlayer();
		return (player) ? *reinterpret_cast<int*>((uintptr_t)player + Offsets::iNumPortalsPlaced) : 0;
	}
	int GetSteps()
	{
		auto player = GetPlayer();
		return (player) ? *reinterpret_cast<int*>((uintptr_t)player + Offsets::iNumStepsTaken) : 0;
	}

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

		if (Interfaces::IServerGameDLL) {
			g_ServerGameDLL = std::make_unique<VMTHook>(Interfaces::IServerGameDLL);
			auto Think = g_ServerGameDLL->GetOriginalFunction<uintptr_t>(Offsets::Think);
			auto abs = GetAbsoluteAddress(Think + Offsets::UTIL_PlayerByIndex);
			UTIL_PlayerByIndex = reinterpret_cast<_UTIL_PlayerByIndex>(abs);
		}
	}
}