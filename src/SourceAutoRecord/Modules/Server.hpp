#pragma once
#include "vmthook/vmthook.h"

#include "Client.hpp"

#include "Features/Routing.hpp"
#include "Features/StepCounter.hpp"

#include "Commands.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

#define IN_JUMP (1 << 1)

#define FL_ONGROUND (1 << 0)
#define FL_FROZEN (1 << 5)
#define FL_ATCONTROLS (1 << 6)

#define MOVETYPE_NOCLIP 8

using namespace Commands;

namespace Server {

using _CheckJumpButton = bool(__cdecl*)(void* thisptr);
using _PlayerMove = int(__cdecl*)(void* thisptr);
using _UTIL_PlayerByIndex = void*(__cdecl*)(int index);
using _FinishGravity = int(__cdecl*)(void* thisptr);
using _AirMove = int(__cdecl*)(void* thisptr);
using _AirAccelerate = int(__cdecl*)(void* thisptr, Vector& wishdir, float wishspeed, float accel);

std::unique_ptr<VMTHook> g_GameMovement;
std::unique_ptr<VMTHook> g_ServerGameDLL;

_UTIL_PlayerByIndex UTIL_PlayerByIndex;

void* gpGlobals;

void* GetPlayer()
{
    return UTIL_PlayerByIndex(1);
}
int GetPortals()
{
    auto player = GetPlayer();
    return (player) ? *reinterpret_cast<int*>((uintptr_t)player + Offsets::iNumPortalsPlaced) : 0;
}

namespace Original {
    _CheckJumpButton CheckJumpButton;
    _PlayerMove PlayerMove;
    _FinishGravity FinishGravity;
    _AirMove AirMove;
    _AirMove AirMoveBase;
    _AirAccelerate AirAccelerate;
    _AirAccelerate AirAccelerateBase;
}

namespace Detour {
    bool JumpedLastTime = false;
    bool CallFromCheckJumpButton = false;

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

        CallFromCheckJumpButton = true;
        auto result = Original::CheckJumpButton(thisptr);
        CallFromCheckJumpButton = false;

        if (enabled) {
            if (!(*m_nOldButtons & IN_JUMP))
                *m_nOldButtons = stored;
        }

        if (result) {
            JumpedLastTime = true;
            Stats::Jumps::Total++;
            Stats::Steps::Total++;
            Stats::Jumps::StartTrace(Client::GetAbsOrigin());
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

        // Landed after a jump
        if (Stats::Jumps::IsTracing
            && m_fFlags & FL_ONGROUND
            && m_MoveType != MOVETYPE_NOCLIP) {

            Stats::Jumps::EndTrace(Client::GetAbsOrigin(), sar_stats_jumps_xy.GetBool());
        }

        StepCounter::ReduceTimer(frametime);

        // Player is on ground and moving etc.
        if (StepCounter::StepSoundTime <= 0
            && m_fFlags & FL_ONGROUND && !(m_fFlags & (FL_FROZEN | FL_ATCONTROLS))
            && m_vecVelocity.Length2D() > 0.0001f
            && psurface
            && m_MoveType != MOVETYPE_NOCLIP
            && sv_footsteps.GetFloat()) {

            StepCounter::Increment(m_fFlags, m_vecVelocity, m_nWaterLevel);
        }

        Stats::Velocity::Save(Client::GetLocalVelocity(), sar_stats_velocity_peak_xy.GetBool());

        return Original::PlayerMove(thisptr);
    }
    int __cdecl FinishGravity(void* thisptr)
    {
        if (CallFromCheckJumpButton && sar_jumpboost.GetBool()) {
            auto player = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::player);
            auto mv = *reinterpret_cast<CHLMoveData**>((uintptr_t)thisptr + Offsets::mv);

            auto m_bDucked = *reinterpret_cast<bool*>((uintptr_t)player + Offsets::m_bDucked);

            Vector vecForward;
            Math::AngleVectors(mv->m_vecViewAngles, &vecForward);
            vecForward.z = 0;
            Math::VectorNormalize(vecForward);

            float flSpeedBoostPerc = (!mv->m_bIsSprinting && !m_bDucked) ? 0.5f : 0.1f;
            float flSpeedAddition = fabs(mv->m_flForwardMove * flSpeedBoostPerc);
            float flMaxSpeed = mv->m_flMaxSpeed + (mv->m_flMaxSpeed * flSpeedBoostPerc);
            float flNewSpeed = (flSpeedAddition + mv->m_vecVelocity.Length2D());

            if (sar_jumpboost.GetInt() == 1) {
                if (flNewSpeed > flMaxSpeed) {
                    flSpeedAddition -= flNewSpeed - flMaxSpeed;
                }

                if (mv->m_flForwardMove < 0.0f) {
                    flSpeedAddition *= -1.0f;
                }
            }

            Math::VectorAdd((vecForward * flSpeedAddition), mv->m_vecVelocity, mv->m_vecVelocity);
        }
        return Original::FinishGravity(thisptr);
    }
    int __cdecl AirMove(void* thisptr)
    {
        if (sar_aircontrol.GetBool()) {
            return Original::AirMoveBase(thisptr);
        }
        return Original::AirMove(thisptr);
    }
    int __cdecl AirAccelerate(void* thisptr, Vector& wishdir, float wishspeed, float accel)
    {
        //return Original::AirAccelerate(thisptr, wishdir, wishspeed, accel);
        return Original::AirAccelerateBase(thisptr, wishdir, wishspeed, accel);
    }
}

void Hook()
{
    if (Interfaces::IGameMovement) {
        g_GameMovement = std::make_unique<VMTHook>(Interfaces::IGameMovement);
        g_GameMovement->HookFunction((void*)Detour::CheckJumpButton, Offsets::CheckJumpButton);
        g_GameMovement->HookFunction((void*)Detour::PlayerMove, Offsets::PlayerMove);

        Original::CheckJumpButton = g_GameMovement->GetOriginalFunction<_CheckJumpButton>(Offsets::CheckJumpButton);
        Original::PlayerMove = g_GameMovement->GetOriginalFunction<_PlayerMove>(Offsets::PlayerMove);

        if (Game::IsPortal2Engine()) {
            g_GameMovement->HookFunction((void*)Detour::FinishGravity, Offsets::FinishGravity);
            g_GameMovement->HookFunction((void*)Detour::AirMove, Offsets::AirMove);

            Original::FinishGravity = g_GameMovement->GetOriginalFunction<_FinishGravity>(Offsets::FinishGravity);

            auto destructor = g_GameMovement->GetOriginalFunction<uintptr_t>(0);
            auto baseDestructor = Memory::ReadAbsoluteAddress(destructor + Offsets::AirMove_Offset1);
            auto baseOffset = *reinterpret_cast<uintptr_t*>(baseDestructor + Offsets::AirMove_Offset2);
            auto airMoveAddr = *reinterpret_cast<uintptr_t*>(baseOffset + Offsets::AirMove * sizeof(uintptr_t*));

            Original::AirMove = g_GameMovement->GetOriginalFunction<_AirMove>(Offsets::AirMove);
            Original::AirMoveBase = reinterpret_cast<_AirMove>(airMoveAddr);

            //g_GameMovement->HookFunction((void*)Detour::AirAccelerate, Offsets::AirAccelerate);
            //Original::AirAccelerate = g_GameMovement->GetOriginalFunction<_AirAccelerate>(Offsets::AirAccelerate);
            //auto airAccelerateAddr = *reinterpret_cast<uintptr_t*>(baseOffset + Offsets::AirAccelerate * sizeof(uintptr_t*));
            //Original::AirAccelerateBase = reinterpret_cast<_AirAccelerate>(airAccelerateAddr);
        }

        auto FullTossMove = g_GameMovement->GetOriginalFunction<uintptr_t>(Offsets::FullTossMove);
        gpGlobals = **reinterpret_cast<void***>(FullTossMove + Offsets::gpGlobals);
    }

    if (Interfaces::IServerGameDLL) {
        g_ServerGameDLL = std::make_unique<VMTHook>(Interfaces::IServerGameDLL);
        auto Think = g_ServerGameDLL->GetOriginalFunction<uintptr_t>(Offsets::Think);
        auto addr = Memory::ReadAbsoluteAddress(Think + Offsets::UTIL_PlayerByIndex);
        UTIL_PlayerByIndex = reinterpret_cast<_UTIL_PlayerByIndex>(addr);
    }
}
}