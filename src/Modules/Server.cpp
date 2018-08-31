#include "Server.hpp"

#include "Client.hpp"
#include "Engine.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"

#include "Cheats.hpp"
#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

REDECL(Server::CheckJumpButton)
REDECL(Server::PlayerMove)
REDECL(Server::FinishGravity)
REDECL(Server::AirMove)
REDECL(Server::AirMoveBase)
REDECL(Server::GameFrame)

void* Server::GetPlayer()
{
    return this->UTIL_PlayerByIndex(1);
}
int Server::GetPortals()
{
    auto player = this->GetPlayer();
    return (player) ? *reinterpret_cast<int*>((uintptr_t)player + Offsets::iNumPortalsPlaced) : -1;
}

// CGameMovement::CheckJumpButton
DETOUR_T(bool, Server::CheckJumpButton)
{
    auto mv = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::mv);
    auto m_nOldButtons = reinterpret_cast<int*>((uintptr_t)mv + Offsets::m_nOldButtons);

    auto enabled = (!sv_bonus_challenge.GetBool() || sv_cheats.GetBool())
        && sar_autojump.GetBool();

    auto original = 0;
    if (enabled) {
        original = *m_nOldButtons;

        if (!server->jumpedLastTime)
            *m_nOldButtons &= ~IN_JUMP;
    }

    server->jumpedLastTime = false;

    server->callFromCheckJumpButton = true;
    auto result = Server::CheckJumpButton(thisptr);
    server->callFromCheckJumpButton = false;

    if (enabled) {
        if (!(*m_nOldButtons & IN_JUMP))
            *m_nOldButtons = original;
    }

    if (result) {
        server->jumpedLastTime = true;
        ++stats->jumps->total;
        ++stats->steps->total;
        stats->jumps->StartTrace(client->GetAbsOrigin());
    }

    return result;
}

// CGameMovement::PlayerMove
DETOUR(Server::PlayerMove)
{
    auto player = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::player);
    auto mv = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::mv);

    auto m_fFlags = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_fFlags);
    auto m_MoveType = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_MoveType);
    auto m_nWaterLevel = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_nWaterLevel);

    auto m_vecVelocity = *reinterpret_cast<Vector*>((uintptr_t)mv + Offsets::m_vecVelocity2);

    // Landed after a jump
    if (stats->jumps->isTracing
        && m_fFlags & FL_ONGROUND
        && m_MoveType != MOVETYPE_NOCLIP) {
        stats->jumps->EndTrace(client->GetAbsOrigin(), sar_stats_jumps_xy.GetBool());
    }

    stepCounter->ReduceTimer(server->gpGlobals->frametime);

    // Player is on ground and moving etc.
    if (stepCounter->stepSoundTime <= 0
        && m_MoveType != MOVETYPE_NOCLIP
        && sv_footsteps.GetFloat()
        && !(m_fFlags & (FL_FROZEN | FL_ATCONTROLS))
        && ((m_fFlags & FL_ONGROUND && m_vecVelocity.Length2D() > 0.0001f)
               || m_MoveType == MOVETYPE_LADDER)) {
        stepCounter->Increment(m_fFlags, m_MoveType, m_vecVelocity, m_nWaterLevel);
    }

    stats->velocity->Save(client->GetLocalVelocity(), sar_stats_velocity_peak_xy.GetBool());

    return Server::PlayerMove(thisptr);
}

// CGameMovement::FinishGravity
DETOUR(Server::FinishGravity)
{
    if (server->callFromCheckJumpButton && sar_jumpboost.GetBool()) {
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
    return Server::FinishGravity(thisptr);
}

// CGameMovement::AirMove
DETOUR_B(Server::AirMove)
{
#ifdef _WIN32
    if (sar_aircontrol.GetInt() >= 2) {
#else
    if (sar_aircontrol.GetBool()) {
#endif
        return Server::AirMoveBase(thisptr);
    }
    return Server::AirMove(thisptr);
}

#ifdef _WIN32
// CGameMovement::AirMove
DETOUR_MID_MH(Server::AirMove_Mid)
{
    __asm {
        pushad
        pushfd
    }

    if ((!sv_bonus_challenge.GetBool() || sv_cheats.GetBool()) && sar_aircontrol.GetBool())
    {
        __asm {
            popfd
            popad
            jmp Server::AirMove_Skip
        }
    }

    __asm {
        popfd
        popad
        movss xmm2, dword ptr[eax + 0x40]
        jmp Server::AirMove_Continue
    }
}
#endif

// CServerGameDLL::GameFrame
#ifdef _WIN32
DETOUR_STD(Server::GameFrame, bool simulating)
#else
DETOUR(Server::GameFrame, bool simulating)
#endif
{
    if (!*server->g_InRestore) {
        auto pe = server->g_EventQueue->m_Events.m_pNext;
        while (pe && pe->m_flFireTime <= server->gpGlobals->curtime) {
            if (sar_debug_event_queue.GetBool()) {
                console->Print("[%i] Event fired!\n", engine->GetSessionTick());
                console->Msg("    - m_flFireTime   %f\n", pe->m_flFireTime);
                console->Msg("    - m_iTarget      %s\n", pe->m_iTarget);
                console->Msg("    - m_pEntTarget   %p\n", pe->m_pEntTarget);
                console->Msg("    - m_iTargetInput %s\n", pe->m_iTargetInput);
                console->Msg("    - m_pActivator   %p\n", pe->m_pActivator);
                console->Msg("    - m_pCaller      %p\n", pe->m_pCaller);
                console->Msg("    - m_iOutputID    %i\n", pe->m_iOutputID);
            }

            speedrun->CheckRules(pe, engine->tickcount);

            pe = pe->m_pNext;
        }
    }
#ifdef _WIN32
    Server::GameFrame(simulating);
#else
    return Server::GameFrame(thisptr, simulating);
#endif
}

bool Server::Init()
{
    this->g_GameMovement = Interface::Create(MODULE("server"), "GameMovement0");
    this->g_ServerGameDLL = Interface::Create(MODULE("server"), "ServerGameDLL0");

    if (this->g_GameMovement) {
        this->g_GameMovement->Hook(this->CheckJumpButton_Hook, this->CheckJumpButton, Offsets::CheckJumpButton);
        this->g_GameMovement->Hook(this->PlayerMove_Hook, this->PlayerMove, Offsets::PlayerMove);

        if (sar.game->IsPortal2Engine()) {
            this->g_GameMovement->Hook(this->FinishGravity_Hook, this->FinishGravity, Offsets::FinishGravity);
            this->g_GameMovement->Hook(this->AirMove_Hook, this->AirMove, Offsets::AirMove);

            auto ctor = this->g_GameMovement->Original(0);
            auto baseCtor = Memory::Read(ctor + Offsets::AirMove_Offset1);
            auto baseOffset = Memory::Deref<uintptr_t>(baseCtor + Offsets::AirMove_Offset2);
            Memory::Deref<_AirMove>(baseOffset + Offsets::AirMove * sizeof(uintptr_t*), &this->AirMoveBase);

#ifdef _WIN32
            auto airMoveMid = this->g_GameMovement->Original(Offsets::AirMove) + AirMove_Mid_Offset;
            if (Memory::FindAddress(airMoveMid, airMoveMid + 5, AirMove_Signature) == airMoveMid) {
                MH_HOOK_MID(this->AirMove_Mid, airMoveMid);
                this->AirMove_Continue = airMoveMid + AirMove_Continue_Offset;
                this->AirMove_Skip = airMoveMid + AirMove_Skip_Offset;
                console->DevMsg("SAR: Verified sar_aircontrol 1!\n");
            } else {
                console->Warning("SAR: Failed to enable sar_aircontrol 1 style!\n");
            }
#endif
        }
    }

    if (this->g_ServerGameDLL) {
        auto Think = this->g_ServerGameDLL->Original(Offsets::Think);
        Memory::Read<_UTIL_PlayerByIndex>(Think + Offsets::UTIL_PlayerByIndex, &this->UTIL_PlayerByIndex);

        auto GameFrame = this->g_ServerGameDLL->Original(Offsets::GameFrame);
        Memory::DerefDeref<CGlobalVars*>(GameFrame + Offsets::gpGlobals, &this->gpGlobals);
        Memory::Deref<bool*>(GameFrame + Offsets::g_InRestore, &this->g_InRestore);

        auto ServiceEventQueue = Memory::Read(GameFrame + Offsets::ServiceEventQueue);
        Memory::Deref<CEventQueue*>(ServiceEventQueue + Offsets::g_EventQueue, &this->g_EventQueue);

        this->g_ServerGameDLL->Hook(this->GameFrame_Hook, this->GameFrame, Offsets::GameFrame);
    }

    return this->hasLoaded = this->g_GameMovement && this->g_ServerGameDLL;
}
void Server::Shutdown()
{
#if _WIN32
    MH_UNHOOK(this->AirMove_Mid);
#endif
    Interface::Delete(this->g_GameMovement);
    Interface::Delete(this->g_ServerGameDLL);
}

Server* server;
