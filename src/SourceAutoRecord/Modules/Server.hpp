#pragma once
#include "vmthook/vmthook.h"

#include "Commands.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Stats.hpp"
#include "Utils.hpp"

#define IN_JUMP	(1 << 1)

#define FL_ONGROUND (1 << 0)
#define FL_DUCKING (1 << 1)
#define FL_FROZEN (1 << 5)
#define FL_ATCONTROLS (1 << 6)

#define MOVETYPE_NOCLIP 8

#define WL_Waist 2

using namespace Commands;

namespace Server
{
	using _CheckJumpButton = bool(__cdecl*)(void* thisptr);
	using _PlayerMove = int(__cdecl*)(void* thisptr);
	using _UTIL_PlayerByIndex = void*(__cdecl*)(int index);

	std::unique_ptr<VMTHook> g_GameMovement;
	std::unique_ptr<VMTHook> g_ServerGameDLL;

	_UTIL_PlayerByIndex UTIL_PlayerByIndex;

	void* gpGlobals;

	// Static version of CBasePlayer::m_flStepSoundTime to keep track
	// of a step globally and to not mess up sounds etc.
	float StepSoundTime;

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

	namespace Detour
	{
		bool JumpedLastTime = false;

		bool __cdecl CheckJumpButton(void* thisptr)
		{
			auto mv = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::mv);
			auto m_nOldButtons = reinterpret_cast<int*>((uintptr_t)mv + Offsets::m_nOldButtons);

			auto enabled = (!sv_bonus_challenge.GetBool() || sv_cheats.GetBool()) && sar_autojump.GetBool();
			auto stored = 0;

			if (enabled) {
				stored = *m_nOldButtons;

				if (!JumpedLastTime)
					*m_nOldButtons &= ~IN_JUMP;
			}

			JumpedLastTime = false;

			auto result = Original::CheckJumpButton(thisptr);

			if (enabled) {
				if (!(*m_nOldButtons & IN_JUMP))
					*m_nOldButtons = stored;
			}

			if (result) {
				JumpedLastTime = true;
				Stats::TotalJumps++;
				Stats::TotalSteps++;
			}

			return result;
		}
		int __cdecl PlayerMove(void* thisptr)
		{
			auto player = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::player);
			auto mv = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::mv);

			auto m_fFlags = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_fFlags);
			auto m_MoveType = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_MoveType);
			auto m_nWaterLevel = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_nWaterLevel);
			auto psurface = *reinterpret_cast<void**>((uintptr_t)player + Offsets::psurface);

			auto frametime = *reinterpret_cast<float*>((uintptr_t)gpGlobals + Offsets::frametime);
			auto m_vecVelocity = *reinterpret_cast<Vector*>((uintptr_t)mv + Offsets::m_vecVelocity2);

			// Calculate when to play next step sound
			if (StepSoundTime > 0) {
				StepSoundTime -= 1000.0f * frametime;
				if (StepSoundTime < 0) {
					StepSoundTime = 0;
				}
			}

			// Player is on ground and moving etc.
			if (StepSoundTime <= 0
				&& m_fFlags & FL_ONGROUND && !(m_fFlags & (FL_FROZEN | FL_ATCONTROLS))
				&& m_vecVelocity.Length2D() > 0.0001f
				&& psurface
				&& m_MoveType != MOVETYPE_NOCLIP
				&& sv_footsteps.GetFloat()) {

				// Adjust next step
				auto velrun = (m_fFlags & FL_DUCKING) ? 80 : 220;
				auto bWalking = m_vecVelocity.Length() < velrun;

				if (m_nWaterLevel == WL_Waist) {
					StepSoundTime = 600;
				}
				else {
					StepSoundTime = (bWalking) ? 400 : 300;
				}

				if (m_fFlags & FL_DUCKING) {
					StepSoundTime += 100;
				}

				Stats::TotalSteps++;
			}

			return Original::PlayerMove(thisptr);
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