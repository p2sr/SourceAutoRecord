#pragma once
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

VMT g_GameMovement;

using _UTIL_PlayerByIndex = void*(__CALL*)(int index);
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

namespace Detour {
    bool JumpedLastTime = false;
    bool CallFromCheckJumpButton = false;
}

// CGameMovement::CheckJumpButton
DETOUR_T(bool, CheckJumpButton)
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

// CGameMovement::PlayerMove
DETOUR(PlayerMove)
{
    auto player = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::player);
    auto mv = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::mv);

    auto m_fFlags = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_fFlags);
    auto m_MoveType = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_MoveType);
    auto m_nWaterLevel = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_nWaterLevel);
    auto m_pSurfaceData = *reinterpret_cast<void**>((uintptr_t)player + Offsets::m_pSurfaceData);

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
        && m_pSurfaceData
        && m_MoveType != MOVETYPE_NOCLIP
        && sv_footsteps.GetFloat()) {

        StepCounter::Increment(m_fFlags, m_vecVelocity, m_nWaterLevel);
    }

    Stats::Velocity::Save(Client::GetLocalVelocity(), sar_stats_velocity_peak_xy.GetBool());

    return Original::PlayerMove(thisptr);
}

// CGameMovement::FinishGravity
DETOUR(FinishGravity)
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

// CGameMovement::AirMove
DETOUR_B(AirMove)
{
    if (sar_aircontrol.GetBool()) {
        return Original::AirMoveBase(thisptr);
    }
    return Original::AirMove(thisptr);
}

// CGameMovement::AirAccelerate
//DETOUR_B(AirAccelerate, Vector& wishdir, float wishspeed, float accel)
//{
//    //return Original::AirAccelerate(thisptr, wishdir, wishspeed, accel);
//    return Original::AirAccelerateBase(thisptr, wishdir, wishspeed, accel);
//}

void Hook()
{
    if (SAR::NewVMT(Interfaces::IGameMovement, g_GameMovement)) {
        HOOK(g_GameMovement, CheckJumpButton);
        HOOK(g_GameMovement, PlayerMove);

        if (Game::IsPortal2Engine()) {
            HOOK(g_GameMovement, FinishGravity);
            HOOK(g_GameMovement, AirMove);

            // TODO
            auto destructor = g_GameMovement->GetOriginalFunction<uintptr_t>(0);
            auto baseDestructor = Memory::ReadAbsoluteAddress(destructor + Offsets::AirMove_Offset1);
            auto baseOffset = *reinterpret_cast<uintptr_t*>(baseDestructor + Offsets::AirMove_Offset2);
            auto airMoveAddr = *reinterpret_cast<uintptr_t*>(baseOffset + Offsets::AirMove * sizeof(uintptr_t*));
            Original::AirMoveBase = reinterpret_cast<_AirMove>(airMoveAddr);

            /*HOOK(g_GameMovement, AirAccelerate);
            auto airAccelerateAddr = *reinterpret_cast<uintptr_t*>(baseOffset + Offsets::AirAccelerate * sizeof(uintptr_t*));
            Original::AirAccelerateBase = reinterpret_cast<_AirAccelerate>(airAccelerateAddr);*/
        }

        auto FullTossMove = g_GameMovement->GetOriginalFunction<uintptr_t>(Offsets::FullTossMove);
        gpGlobals = **reinterpret_cast<void***>(FullTossMove + Offsets::gpGlobals);
    }

    if (Interfaces::IServerGameDLL) {
        auto g_ServerGameDLL = std::make_unique<VMTHook>(Interfaces::IServerGameDLL);
        auto Think = g_ServerGameDLL->GetOriginalFunction<uintptr_t>(Offsets::Think);
        auto addr = Memory::ReadAbsoluteAddress(Think + Offsets::UTIL_PlayerByIndex);
        UTIL_PlayerByIndex = reinterpret_cast<_UTIL_PlayerByIndex>(addr);
    }
}

void Unhook()
{
    UNHOOK(g_GameMovement, CheckJumpButton);
    UNHOOK(g_GameMovement, CheckJumpButton);
    UNHOOK(g_GameMovement, PlayerMove);
    UNHOOK(g_GameMovement, AirMove);
    //UNHOOK(g_GameMovement, AirAccelerate);
    SAR::DeleteVMT(g_GameMovement);
}
}