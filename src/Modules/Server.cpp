#include "Server.hpp"

#include <cstdint>
#include <cstring>

#include "Features/OffsetFinder.hpp"
#include "Features/Routing/EntityInspector.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Tas/AutoStrafer.hpp"
#include "Features/Tas/TasTools.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Features/Timer/Timer.hpp"

#include "Engine.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

Variable sv_cheats;
Variable sv_footsteps;
Variable sv_alternateticks;
Variable sv_bonus_challenge;
Variable sv_accelerate;
Variable sv_airaccelerate;
Variable sv_friction;
Variable sv_maxspeed;
Variable sv_stopspeed;
Variable sv_maxvelocity;
Variable sv_transition_fade_time;
Variable sv_laser_cube_autoaim;
Variable sv_edgefriction;
Variable cl_sidespeed;
Variable cl_forwardspeed;
Variable host_framerate;

REDECL(Server::CheckJumpButton);
REDECL(Server::CheckJumpButtonBase);
REDECL(Server::PlayerMove);
REDECL(Server::FinishGravity);
REDECL(Server::AirMove);
REDECL(Server::AirMoveBase);
REDECL(Server::GameFrame);
REDECL(Server::ProcessMovement);
#ifdef _WIN32
REDECL(Server::AirMove_Skip);
REDECL(Server::AirMove_Continue);
REDECL(Server::AirMove_Mid);
REDECL(Server::AirMove_Mid_Trampoline);
#endif

void* Server::GetPlayer(int index)
{
    index = (index == -1) ? engine->GetLocalPlayerIndex() : index;
    return this->UTIL_PlayerByIndex(index);
}
int Server::GetPortals(void* entity)
{
    return *reinterpret_cast<int*>((uintptr_t)entity + Offsets::iNumPortalsPlaced);
}
Vector Server::GetAbsOrigin(void* entity)
{
    return *reinterpret_cast<Vector*>((uintptr_t)entity + Offsets::S_m_vecAbsOrigin);
}
QAngle Server::GetAbsAngles(void* entity)
{
    return *reinterpret_cast<QAngle*>((uintptr_t)entity + Offsets::S_m_angAbsRotation);
}
Vector Server::GetLocalVelocity(void* entity)
{
    return *reinterpret_cast<Vector*>((uintptr_t)entity + Offsets::S_m_vecVelocity);
}
int Server::GetFlags(void* entity)
{
    return *reinterpret_cast<int*>((uintptr_t)entity + Offsets::m_fFlags);
}
int Server::GetEFlags(void* entity)
{
    return *reinterpret_cast<int*>((uintptr_t)entity + Offsets::m_iEFlags);
}
float Server::GetMaxSpeed(void* entity)
{
    return *reinterpret_cast<float*>((uintptr_t)entity + Offsets::m_flMaxspeed);
}
float Server::GetGravity(void* entity)
{
    return *reinterpret_cast<float*>((uintptr_t)entity + Offsets::m_flGravity);
}
Vector Server::GetViewOffset(void* entity)
{
    return *reinterpret_cast<Vector*>((uintptr_t)entity + Offsets::S_m_vecViewOffset);
}
char* Server::GetEntityName(void* entity)
{
    return *reinterpret_cast<char**>((uintptr_t)entity + Offsets::m_iName);
}
char* Server::GetEntityClassName(void* entity)
{
    return *reinterpret_cast<char**>((uintptr_t)entity + Offsets::m_iClassName);
}
bool Server::IsPlayer(void* entity)
{
    return Memory::VMT<bool (*)(void*)>(entity, Offsets::IsPlayer)(entity);
}
int Server::GetSplitScreenPlayerSlot(void* entity)
{
    // Simplified version of CBasePlayer::GetSplitScreenPlayerSlot
    for (auto i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i) {
        if (server->UTIL_PlayerByIndex(i + 1) == entity) {
            return i;
        }
    }

    return 0;
}

// CGameMovement::CheckJumpButton
DETOUR_T(bool, Server::CheckJumpButton)
{
    auto player = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::player);

    auto mv = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::mv);
    auto m_nOldButtons = reinterpret_cast<int*>((uintptr_t)mv + Offsets::m_nOldButtons);

    auto cheating = !sv_bonus_challenge.GetBool() || sv_cheats.GetBool();
    auto autoJump = cheating && sar_autojump.GetBool();
    auto duckJump = cheating && sar_duckjump.isRegistered && sar_duckjump.GetBool();

    auto original = 0;
    if (autoJump) {
        original = *m_nOldButtons;

        if (!server->jumpedLastTime)
            *m_nOldButtons &= ~IN_JUMP;
    }

    server->jumpedLastTime = false;

    server->callFromCheckJumpButton = true;
    auto result = (duckJump && Server::CheckJumpButtonBase)
        ? Server::CheckJumpButtonBase(thisptr)
        : Server::CheckJumpButton(thisptr);
    server->callFromCheckJumpButton = false;

    if (autoJump) {
        if (!(*m_nOldButtons & IN_JUMP))
            *m_nOldButtons = original;
    }

    if (result) {
        server->jumpedLastTime = true;

        auto stat = stats->Get(server->GetSplitScreenPlayerSlot(player));        
        ++stat->jumps->total;
        ++stat->steps->total;
        stat->jumps->StartTrace(server->GetAbsOrigin(player));
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

    auto m_vecVelocity = *reinterpret_cast<Vector*>((uintptr_t)mv + Offsets::mv_m_vecVelocity);

    auto stat = stats->Get(server->GetSplitScreenPlayerSlot(player));

    // Landed after a jump
    if (stat->jumps->isTracing
        && m_fFlags & FL_ONGROUND
        && m_MoveType != MOVETYPE_NOCLIP) {
        stat->jumps->EndTrace(server->GetAbsOrigin(player), sar_stats_jumps_xy.GetBool());
    }

    stepCounter->ReduceTimer(server->gpGlobals->frametime);

    // Player is on ground and moving etc.
    if (stepCounter->stepSoundTime <= 0
        && m_MoveType != MOVETYPE_NOCLIP
        && sv_footsteps.GetFloat()
        && !(m_fFlags & (FL_FROZEN | FL_ATCONTROLS))
        && ((m_fFlags & FL_ONGROUND && m_vecVelocity.Length2D() > 0.0001f) || m_MoveType == MOVETYPE_LADDER)) {
        stepCounter->Increment(m_fFlags, m_MoveType, m_vecVelocity, m_nWaterLevel);
        ++stat->steps->total;
    }

    stat->velocity->Save(server->GetLocalVelocity(player), sar_stats_velocity_peak_xy.GetBool());
    inspector->Record();

    return Server::PlayerMove(thisptr);
}

// CGameMovement::ProcessMovement
DETOUR(Server::ProcessMovement, void* pPlayer, CMoveData* pMove)
{
    if (sv_cheats.GetBool()) {
        autoStrafer->Strafe(pPlayer, pMove);
    }

    return Server::ProcessMovement(thisptr, pPlayer, pMove);
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
        float flSpeedAddition = std::fabs(mv->m_flForwardMove * flSpeedBoostPerc);
        float flMaxSpeed = mv->m_flMaxSpeed + (mv->m_flMaxSpeed * flSpeedBoostPerc);
        float flNewSpeed = flSpeedAddition + mv->m_vecVelocity.Length2D();

        if (sar_jumpboost.GetInt() == 1) {
            if (flNewSpeed > flMaxSpeed) {
                flSpeedAddition -= flNewSpeed - flMaxSpeed;
            }

            if (mv->m_flForwardMove < 0.0f) {
                flSpeedAddition *= -1.0f;
            }
        }

        Math::VectorAdd(vecForward * flSpeedAddition, mv->m_vecVelocity, mv->m_vecVelocity);
    }
    return Server::FinishGravity(thisptr);
}

// CGameMovement::AirMove
DETOUR_B(Server::AirMove)
{
    if (sar_aircontrol.GetInt() >= 2) {
        return Server::AirMoveBase(thisptr);
    }

    return Server::AirMove(thisptr);
}
#ifdef _WIN32
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
DETOUR_STD(void, Server::GameFrame, bool simulating)
#else
DETOUR(Server::GameFrame, bool simulating)
#endif
{
    if (!server->IsRestoring()) {
        if (!simulating && !pauseTimer->IsActive()) {
            pauseTimer->Start();
        } else if (simulating && pauseTimer->IsActive()) {
            pauseTimer->Stop();
            console->DevMsg("Paused for %i non-simulated ticks.\n", pauseTimer->GetTotal());
        }
    }

#ifdef _WIN32
    Server::GameFrame(simulating);
#else
    auto result = Server::GameFrame(thisptr, simulating);
#endif

    if (session->isRunning && pauseTimer->IsActive()) {
        pauseTimer->Increment();

        if (speedrun->IsActive() && sar_speedrun_time_pauses.GetBool()) {
             speedrun->IncrementPauseTime();
        }

        if (timer->isRunning && sar_timer_time_pauses.GetBool()) {
            ++timer->totalTicks;
        }
    }

    if (session->isRunning && sar_speedrun_standard.GetBool()) {
        speedrun->CheckRules(engine->tickcount);
    }

#ifndef _WIN32
    return result;
#endif
}

bool Server::Init()
{
    this->g_GameMovement = Interface::Create(this->Name(), "GameMovement0");
    this->g_ServerGameDLL = Interface::Create(this->Name(), "ServerGameDLL0");

    if (this->g_GameMovement) {
        this->g_GameMovement->Hook(Server::CheckJumpButton_Hook, Server::CheckJumpButton, Offsets::CheckJumpButton);
        this->g_GameMovement->Hook(Server::PlayerMove_Hook, Server::PlayerMove, Offsets::PlayerMove);

        if (sar.game->version & SourceGame_Portal2Engine) {
            this->g_GameMovement->Hook(Server::ProcessMovement_Hook, Server::ProcessMovement, Offsets::ProcessMovement);
            this->g_GameMovement->Hook(Server::FinishGravity_Hook, Server::FinishGravity, Offsets::FinishGravity);
            this->g_GameMovement->Hook(Server::AirMove_Hook, Server::AirMove, Offsets::AirMove);

            auto ctor = this->g_GameMovement->Original(0);
            auto baseCtor = Memory::Read(ctor + Offsets::AirMove_Offset1);
            auto baseOffset = Memory::Deref<uintptr_t>(baseCtor + Offsets::AirMove_Offset2);
            Memory::Deref<_AirMove>(baseOffset + Offsets::AirMove * sizeof(uintptr_t*), &this->AirMoveBase);

            Memory::Deref<_CheckJumpButton>(baseOffset + Offsets::CheckJumpButton * sizeof(uintptr_t*), &this->CheckJumpButtonBase);

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

    if (auto g_ServerTools = Interface::Create(this->Name(), "VSERVERTOOLS0")) {
        auto GetIServerEntity = g_ServerTools->Original(Offsets::GetIServerEntity);
        Memory::Deref(GetIServerEntity + Offsets::m_EntPtrArray, &this->m_EntPtrArray);
    }

    if (this->g_ServerGameDLL) {
        auto Think = this->g_ServerGameDLL->Original(Offsets::Think);
        Memory::Read<_UTIL_PlayerByIndex>(Think + Offsets::UTIL_PlayerByIndex, &this->UTIL_PlayerByIndex);
        Memory::DerefDeref<CGlobalVars*>((uintptr_t)this->UTIL_PlayerByIndex + Offsets::gpGlobals, &this->gpGlobals);

        this->GetAllServerClasses = this->g_ServerGameDLL->Original<_GetAllServerClasses>(Offsets::GetAllServerClasses);
        this->IsRestoring = this->g_ServerGameDLL->Original<_IsRestoring>(Offsets::IsRestoring);

        if (sar.game->version & (SourceGame_Portal2Game | SourceGame_Portal)) {
            this->g_ServerGameDLL->Hook(Server::GameFrame_Hook, Server::GameFrame, Offsets::GameFrame);
        }
    }

    offsetFinder->ServerSide("CBasePlayer", "m_nWaterLevel", &Offsets::m_nWaterLevel);
    offsetFinder->ServerSide("CBasePlayer", "m_iName", &Offsets::m_iName);
    offsetFinder->ServerSide("CBasePlayer", "m_vecVelocity[0]", &Offsets::S_m_vecVelocity);
    offsetFinder->ServerSide("CBasePlayer", "m_fFlags", &Offsets::m_fFlags);
    offsetFinder->ServerSide("CBasePlayer", "m_flMaxspeed", &Offsets::m_flMaxspeed);
    offsetFinder->ServerSide("CBasePlayer", "m_vecViewOffset[0]", &Offsets::S_m_vecViewOffset);

    if (sar.game->version & SourceGame_Portal2Engine) {
        offsetFinder->ServerSide("CBasePlayer", "m_bDucked", &Offsets::m_bDucked);
        offsetFinder->ServerSide("CBasePlayer", "m_flFriction", &Offsets::m_flFriction);
    }
    if (sar.game->version & (SourceGame_Portal | SourceGame_Portal2Game)) {
        offsetFinder->ServerSide("CPortal_Player", "iNumPortalsPlaced", &Offsets::iNumPortalsPlaced);
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
