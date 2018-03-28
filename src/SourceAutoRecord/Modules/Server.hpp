#pragma once
#include "vmthook/vmthook.h"

#include "Commands.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Stats.hpp"
#include "Utils.hpp"

#define IN_JUMP	(1 << 1)

#define	FL_ONGROUND (1 << 0)
#define FL_DUCKING (1 << 1)
#define FL_FROZEN (1 << 5)
#define FL_ATCONTROLS (1 << 6)

#define MOVETYPE_NOCLIP 8

using namespace Commands;

namespace Server
{
	using _CheckJumpButton = bool(__cdecl*)(void* thisptr);
	using _PlayerMove = int(__cdecl*)(void* thisptr);
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

	namespace Original
	{
		_CheckJumpButton CheckJumpButton;
		_PlayerMove PlayerMove;
	}

	void* gpGlobals;

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
				Stats::TotalSteps++;
			}
			return result;
		}
		int __cdecl PlayerMove(void* thisptr)
		{
			auto player = *reinterpret_cast<void**>((uintptr_t)thisptr + 4);
			auto mv = *reinterpret_cast<void**>((uintptr_t)thisptr + 8);
			auto m_fFlags = *reinterpret_cast<int*>((uintptr_t)player + 212);
			auto m_MoveType = *reinterpret_cast<int*>((uintptr_t)player + 226);
			auto m_vecVelocity = *reinterpret_cast<Vector*>((uintptr_t)mv + 64);
			auto psurface = *reinterpret_cast<void**>((uintptr_t)player + 4116);

			auto m_flStepSoundTime = reinterpret_cast<float*>((uintptr_t)player + 3720);
			auto m_flStepSoundTime_Stored = *m_flStepSoundTime;
			
			auto stepped = false;

			// Player is on ground and moving
			if (m_fFlags & FL_ONGROUND && !(m_fFlags & (FL_FROZEN | FL_ATCONTROLS))
				&& m_vecVelocity.Length2D() > 0.0001f
				&& psurface
				&& m_MoveType != MOVETYPE_NOCLIP) {
				// Calculate when to play next step sound
				if (m_flStepSoundTime_Stored > 0) {
					auto frametime = *reinterpret_cast<float*>((uintptr_t)gpGlobals + 16);
					m_flStepSoundTime_Stored -= 1000.0f * frametime;
					if (m_flStepSoundTime_Stored < 0) {
						m_flStepSoundTime_Stored = 0;
					}
				}
				if (m_flStepSoundTime_Stored <= 0) {
					Stats::TotalSteps++;
					stepped = true;
				}
			}

			auto result = Original::PlayerMove(thisptr);

			// Original function should have updated m_flStepSoundTime but it didn't
			if (stepped && *m_flStepSoundTime == m_flStepSoundTime_Stored) {
				auto velrun = (m_fFlags & FL_DUCKING) ? 80 : 220;
				auto bWalking = m_vecVelocity.Length() < velrun;
				*m_flStepSoundTime = (bWalking) ? 400 : 300;
				if (m_fFlags & FL_DUCKING) {
					*m_flStepSoundTime += 100;
				}
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
			g_GameMovement->HookFunction((void*)Detour::PlayerMove, Offsets::PlayerMove);
			Original::PlayerMove = g_GameMovement->GetOriginalFunction<_PlayerMove>(Offsets::PlayerMove);

			// After CheckJumpButton in VMT
			auto FullTossMove = g_GameMovement->GetOriginalFunction<uintptr_t>(Offsets::CheckJumpButton + 1);
			gpGlobals = **reinterpret_cast<void***>(FullTossMove + Offsets::gpGlobals);
		}

		if (Interfaces::IServerGameDLL) {
			g_ServerGameDLL = std::make_unique<VMTHook>(Interfaces::IServerGameDLL);
			auto Think = g_ServerGameDLL->GetOriginalFunction<uintptr_t>(Offsets::Think);
			auto abs = GetAbsoluteAddress(Think + Offsets::UTIL_PlayerByIndex);
			UTIL_PlayerByIndex = reinterpret_cast<_UTIL_PlayerByIndex>(abs);
		}
	}
}