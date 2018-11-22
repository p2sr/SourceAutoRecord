#include "Server.hpp"

#include <cstdint>
#include <cstring>

#include "Features/Routing/EntityInspector.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Stats/StatsExport.hpp"
#include "Features/Tas/TasTools.hpp"

#include "Client.hpp"
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
#ifdef _WIN32
REDECL(Server::AirMove_Skip);
REDECL(Server::AirMove_Continue);
REDECL(Server::AirMove_Mid);
REDECL(Server::AirMove_Mid_Trampoline);
#endif

void* Server::GetPlayer()
{
    return this->UTIL_PlayerByIndex(1);
}
int Server::GetPortals()
{
    auto player = this->GetPlayer();
    return (player) ? *reinterpret_cast<int*>((uintptr_t)player + Offsets::iNumPortalsPlaced) : -1;
}
CEntInfo* Server::GetEntityInfoByIndex(int index)
{
    auto size = (sar.game->version & SourceGame_Portal2Engine)
        ? sizeof(CEntInfo2)
        : sizeof(CEntInfo);
    return reinterpret_cast<CEntInfo*>((uintptr_t)server->m_EntPtrArray + size * index);
}
CEntInfo* Server::GetEntityInfoByName(const char* name)
{
    for (int index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
        auto info = server->GetEntityInfoByIndex(index);
        if (info->m_pEntity == nullptr) {
            continue;
        }

        auto match = server->GetEntityName(info->m_pEntity);
        if (!match || std::strcmp(match, name) != 0) {
            continue;
        }

        return info;
    }

    return nullptr;
}
CEntInfo* Server::GetEntityInfoByClassName(const char* name)
{
    for (int index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
        auto info = server->GetEntityInfoByIndex(index);
        if (info->m_pEntity == nullptr) {
            continue;
        }

        auto match = server->GetEntityClassName(info->m_pEntity);
        if (!match || std::strcmp(match, name) != 0) {
            continue;
        }

        return info;
    }

    return nullptr;
}
char* Server::GetEntityName(void* entity)
{
    return *reinterpret_cast<char**>((uintptr_t)entity + Offsets::m_iName);
}
char* Server::GetEntityClassName(void* entity)
{
    return *reinterpret_cast<char**>((uintptr_t)entity + Offsets::m_iClassName);
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
void Server::GetOffset(const char* className, const char* propName, int& offset)
{
    if (this->GetAllServerClasses) {
        for (auto curClass = this->GetAllServerClasses(); curClass; curClass = curClass->m_pNext) {
            if (!std::strcmp(curClass->m_pNetworkName, className)) {
                auto result = FindOffset(curClass->m_pTable, propName);
                if (result != 0) {
                    console->DevMsg("Found %s::%s at %i (server-side)\n", className, propName, result);
                    offset = result;
                }
                break;
            }
        }
    }

    if (offset == 0) {
        console->DevWarning("Failed to find offset for: %s::%s (server-side)\n", className, propName);
    }
}
int16_t Server::FindOffset(SendTable* table, const char* propName)
{
    auto size = sar.game->version & SourceGame_Portal2Engine ? sizeof(SendProp2) : sizeof(SendProp);

    for (int i = 0; i < table->m_nProps; ++i) {
        auto prop = *reinterpret_cast<SendProp*>((uintptr_t)table->m_pProps + size * i);

        auto name = prop.m_pVarName;
        auto offset = prop.m_Offset;
        auto type = prop.m_Type;
        auto nextTable = prop.m_pDataTable;

        if (sar.game->version & SourceGame_Portal2Engine) {
            auto temp = *reinterpret_cast<SendProp2*>(&prop);
            name = temp.m_pVarName;
            offset = temp.m_Offset;
            type = temp.m_Type;
            nextTable = temp.m_pDataTable;
        }

        if (!std::strcmp(name, propName)) {
            return offset;
        }

        if (type != SendPropType::DPT_DataTable) {
            continue;
        }

        if (auto nextOffset = FindOffset(nextTable, propName)) {
            return offset + nextOffset;
        }
    }

    return 0;
}

// CGameMovement::CheckJumpButton
DETOUR_T(bool, Server::CheckJumpButton)
{
    auto mv = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::mv);
    auto m_nOldButtons = reinterpret_cast<int*>((uintptr_t)mv + Offsets::m_nOldButtons);

    auto cheating = !sv_bonus_challenge.GetBool() || sv_cheats.GetBool();
    auto autoJump = cheating && sar_autojump.GetBool();
    auto duckJump = cheating && sar_duckjump.GetBool();

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

    auto m_vecVelocity = *reinterpret_cast<Vector*>((uintptr_t)mv + Offsets::mv_m_vecVelocity);

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

#ifndef _WIN32
    inspector->Record();
#endif

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
DETOUR_STD(void, Server::GameFrame, bool simulating)
#else
DETOUR(Server::GameFrame, bool simulating)
#endif
{
    /* if (!*server->g_InRestore) {
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

            pe = pe->m_pNext;
        }
    } */

#ifdef _WIN32
    Server::GameFrame(simulating);
#else
    if (!*server->g_InRestore) {
        speedrun->CheckRules(engine->tickcount);
    }
    auto result = Server::GameFrame(thisptr, simulating);
#endif

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

    // TODO: windows
#ifndef _WIN32
    if (auto g_ServerTools = Interface::Create(this->Name(), "VSERVERTOOLS0")) {
        auto GetIServerEntity = g_ServerTools->Original(Offsets::GetIServerEntity);
        Memory::Deref(GetIServerEntity + Offsets::m_EntPtrArray, &this->m_EntPtrArray);
    }
#endif

    if (this->g_ServerGameDLL) {
        auto Think = this->g_ServerGameDLL->Original(Offsets::Think);
        Memory::Read<_UTIL_PlayerByIndex>(Think + Offsets::UTIL_PlayerByIndex, &this->UTIL_PlayerByIndex);
        Memory::DerefDeref<CGlobalVars*>((uintptr_t)this->UTIL_PlayerByIndex + Offsets::gpGlobals, &this->gpGlobals);

        this->GetAllServerClasses = this->g_ServerGameDLL->Original<_GetAllServerClasses>(Offsets::GetAllServerClasses);

        auto GameFrame = this->g_ServerGameDLL->Original(Offsets::GameFrame);
        auto ServiceEventQueue = Memory::Read(GameFrame + Offsets::ServiceEventQueue);

        Memory::Deref<bool*>(GameFrame + Offsets::g_InRestore, &this->g_InRestore);
        Memory::Deref<CEventQueue*>(ServiceEventQueue + Offsets::g_EventQueue, &this->g_EventQueue);

        this->g_ServerGameDLL->Hook(Server::GameFrame_Hook, Server::GameFrame, Offsets::GameFrame);
    }

    this->GetOffset("CBasePlayer", "m_nWaterLevel", Offsets::m_nWaterLevel);
    this->GetOffset("CBasePlayer", "m_iName", Offsets::m_iName);
    this->GetOffset("CBasePlayer", "m_vecVelocity[0]", Offsets::S_m_vecVelocity);
    this->GetOffset("CBasePlayer", "m_fFlags", Offsets::m_fFlags);
    this->GetOffset("CBasePlayer", "m_flMaxspeed", Offsets::m_flMaxspeed);
    this->GetOffset("CBasePlayer", "m_vecViewOffset[0]", Offsets::S_m_vecViewOffset);

    if (sar.game->version & SourceGame_Portal2Engine) {
        this->GetOffset("CBasePlayer", "m_bDucked", Offsets::m_bDucked);
    }
    if (sar.game->version & (SourceGame_Portal | SourceGame_Portal2)) {
        this->GetOffset("CPortal_Player", "iNumPortalsPlaced", Offsets::iNumPortalsPlaced);
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
