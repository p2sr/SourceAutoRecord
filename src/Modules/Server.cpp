#include "Server.hpp"

#include <cstdint>
#include <cstring>

#include "Features/EntityList.hpp"
#include "Features/FovChanger.hpp"
#include "Features/Hud/Crosshair.hpp"
#include "Features/Hud/StrafeQuality.hpp"
#include "Features/Hud/ScrollSpeed.hpp"
#include "Features/OffsetFinder.hpp"
#include "Features/Routing/EntityInspector.hpp"
#include "Features/Routing/SeamshotFind.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Tas/AutoStrafer.hpp"
#include "Features/Tas/TasTools.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/TimescaleDetect.hpp"
#include "Features/SegmentedTools.hpp"
#include "Features/NetMessage.hpp"
#include "Features/GroundFramesCounter.hpp"

#include "Engine.hpp"
#include "Client.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Variable.hpp"
#include "Event.hpp"
#include "Hook.hpp"

#define RESET_COOP_PROGRESS_MESSAGE_TYPE "coop-reset"

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
Variable sv_gravity;

REDECL(Server::CheckJumpButton);
REDECL(Server::CheckJumpButtonBase);
REDECL(Server::PlayerMove);
REDECL(Server::FinishGravity);
REDECL(Server::AirMove);
REDECL(Server::AirMoveBase);
REDECL(Server::GameFrame);
REDECL(Server::ProcessMovement);
REDECL(Server::StartTouchChallengeNode);
#ifdef _WIN32
REDECL(Server::AirMove_Skip);
REDECL(Server::AirMove_Continue);
REDECL(Server::AirMove_Mid);
REDECL(Server::AirMove_Mid_Trampoline);
#endif

MDECL(Server::GetPortals, int, iNumPortalsPlaced);
MDECL(Server::GetAbsOrigin, Vector, S_m_vecAbsOrigin);
MDECL(Server::GetAbsAngles, QAngle, S_m_angAbsRotation);
MDECL(Server::GetLocalVelocity, Vector, S_m_vecVelocity);
MDECL(Server::GetFlags, int, m_fFlags);
MDECL(Server::GetEFlags, int, m_iEFlags);
MDECL(Server::GetMaxSpeed, float, m_flMaxspeed);
MDECL(Server::GetGravity, float, m_flGravity);
MDECL(Server::GetViewOffset, Vector, S_m_vecViewOffset);
MDECL(Server::GetEntityName, char*, m_iName);
MDECL(Server::GetEntityClassName, char*, m_iClassName);

void* Server::GetPlayer(int index)
{
    return this->UTIL_PlayerByIndex(index);
}
bool Server::IsPlayer(void* entity)
{
    return Memory::VMT<bool (*)(void*)>(entity, Offsets::IsPlayer)(entity);
}
bool Server::AllowsMovementChanges()
{
    return !sv_bonus_challenge.GetBool() || sv_cheats.GetBool();
}
int Server::GetSplitScreenPlayerSlot(void* entity)
{
    // Simplified version of CBasePlayer::GetSplitScreenPlayerSlot
    for (auto i = 0; i < Offsets::MAX_SPLITSCREEN_PLAYERS; ++i) {
        if (server->UTIL_PlayerByIndex(i + 1) == entity) {
            return i;
        }
    }

    return 0;
}
void Server::KillEntity(void* entity)
{
    variant_t val = {0};
    val.fieldType = FIELD_VOID;
    void* player = this->GetPlayer(1);
    server->AcceptInput(entity, "Kill", player, player, val, 0);
}

float Server::GetCMTimer()
{
    void *player = this->GetPlayer(1);
    if (!player) return 0.0f;
    return *(float *)((uintptr_t)player + Offsets::m_StatsThisLevel + 12);
}

// CGameMovement::CheckJumpButton
DETOUR_T(bool, Server::CheckJumpButton)
{
    auto jumped = false;

    if (server->AllowsMovementChanges()) {
        auto mv = *reinterpret_cast<CHLMoveData**>((uintptr_t)thisptr + Offsets::mv);

        if (sar_autojump.GetBool() && !server->jumpedLastTime) {
            mv->m_nOldButtons &= ~IN_JUMP;
        }

        server->jumpedLastTime = false;
        server->savedVerticalVelocity = mv->m_vecVelocity[2];

        server->callFromCheckJumpButton = true;
        jumped = (sar_duckjump.isRegistered && sar_duckjump.GetBool())
            ? Server::CheckJumpButtonBase(thisptr)
            : Server::CheckJumpButton(thisptr);
        server->callFromCheckJumpButton = false;

        if (jumped) {
            server->jumpedLastTime = true;
        }
    } else {
        jumped = Server::CheckJumpButton(thisptr);
    }

    if (jumped) {
        auto player = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::player);
        auto stat = stats->Get(server->GetSplitScreenPlayerSlot(player));
        ++stat->jumps->total;
        ++stat->steps->total;
        stat->jumps->StartTrace(server->GetAbsOrigin(player));
    }

    return jumped;
}

// CGameMovement::PlayerMove
DETOUR(Server::PlayerMove)
{
    auto player = *reinterpret_cast<void**>((uintptr_t)thisptr + Offsets::player);
    auto mv = *reinterpret_cast<const CHLMoveData**>((uintptr_t)thisptr + Offsets::mv);

    if (sar_crosshair_mode.GetBool() || sar_quickhud_mode.GetBool() || sar_crosshair_P1.GetBool()) {
        auto m_hActiveWeapon = *reinterpret_cast<CBaseHandle*>((uintptr_t)player + Offsets::m_hActiveWeapon);
        server->portalGun = entityList->LookupEntity(m_hActiveWeapon);
    }

    auto m_fFlags = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_fFlags);
    auto m_MoveType = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_MoveType);
    auto m_nWaterLevel = *reinterpret_cast<int*>((uintptr_t)player + Offsets::m_nWaterLevel);

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
        && ((m_fFlags & FL_ONGROUND && mv->m_vecVelocity.Length2D() > 0.0001f) || m_MoveType == MOVETYPE_LADDER)) {
        stepCounter->Increment(m_fFlags, m_MoveType, mv->m_vecVelocity, m_nWaterLevel);
        ++stat->steps->total;
    }

    stat->velocity->Save(server->GetLocalVelocity(player), sar_stats_velocity_peak_xy.GetBool());
    inspector->Record();

    return Server::PlayerMove(thisptr);
}

// CGameMovement::ProcessMovement
DETOUR(Server::ProcessMovement, void* player, CMoveData* move)
{
    if (sv_cheats.GetBool()) {
        autoStrafer->Strafe(player, move);
        tasTools->SetAngles(player);
    }

    unsigned int groundHandle = *(unsigned int *)((uintptr_t)player + Offsets::S_m_hGroundEntity);
    bool grounded = groundHandle != 0xFFFFFFFF;
    int slot = client->GetSplitScreenPlayerSlot(player);
    groundFramesCounter->HandleMovementFrame(slot, grounded);
    strafeQuality.OnMovement(slot, grounded);
    if (move->m_nButtons & IN_JUMP) scrollSpeedHud.OnJump(slot);

    return Server::ProcessMovement(thisptr, player, move);
}

extern Hook g_flagStartTouchHook;
DETOUR(Server::StartTouchChallengeNode, void* entity)
{
    if (server->IsPlayer(entity)) {
        int slot = server->GetSplitScreenPlayerSlot(entity);

        if (engine->demorecorder->isRecordingDemo) {
            char data[2] = { 0x06, slot };
            engine->demorecorder->RecordData(data, sizeof data);
        }

        SpeedrunTimer::TestFlagRules(slot);
    }

    g_flagStartTouchHook.Disable();
    auto ret = Server::StartTouchChallengeNode(thisptr, entity);
    g_flagStartTouchHook.Enable();

    return ret;
}
Hook g_flagStartTouchHook(&Server::StartTouchChallengeNode_Hook);

// CGameMovement::FinishGravity
DETOUR(Server::FinishGravity)
{
    if (server->callFromCheckJumpButton) {
        if (sar_duckjump.GetBool()) {
            auto player = *reinterpret_cast<uintptr_t*>((uintptr_t)thisptr + Offsets::player);
            auto mv = *reinterpret_cast<CHLMoveData**>((uintptr_t)thisptr + Offsets::mv);

            auto m_pSurfaceData = *reinterpret_cast<uintptr_t*>(player + Offsets::m_pSurfaceData);
            auto m_bDucked = *reinterpret_cast<bool*>(player + Offsets::m_bDucked);
            auto m_fFlags = *reinterpret_cast<int*>(player + Offsets::m_fFlags);

            auto flGroundFactor = (m_pSurfaceData) ? *reinterpret_cast<float*>(m_pSurfaceData + Offsets::jumpFactor) : 1.0f;
            auto flMul = std::sqrt(2 * sv_gravity.GetFloat() * GAMEMOVEMENT_JUMP_HEIGHT);

            if (m_bDucked || m_fFlags & FL_DUCKING) {
                mv->m_vecVelocity[2] = flGroundFactor * flMul;
            } else {
                mv->m_vecVelocity[2] = server->savedVerticalVelocity + flGroundFactor * flMul;
            }
        }

        if (sar_jumpboost.GetBool()) {
            auto player = *reinterpret_cast<uintptr_t*>((uintptr_t)thisptr + Offsets::player);
            auto mv = *reinterpret_cast<CHLMoveData**>((uintptr_t)thisptr + Offsets::mv);

            auto m_bDucked = *reinterpret_cast<bool*>(player + Offsets::m_bDucked);

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
    }

    return Server::FinishGravity(thisptr);
}

// CGameMovement::AirMove
DETOUR_B(Server::AirMove)
{
    if (sar_aircontrol.GetInt() >= 2 && server->AllowsMovementChanges()) {
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

    if (sar_aircontrol.GetBool() && server->AllowsMovementChanges())
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

extern Hook g_AcceptInputHook;

// TODO: the windows signature is a bit dumb. fastcall is like thiscall
// but for normal functions and takes an arg in edx, so we use it
// because msvc won't let us use thiscall on a non-member function
#ifdef _WIN32
static void __fastcall AcceptInput_Hook(void* thisptr, void* unused, const char* inputName, void* activator, void* caller, variant_t parameter, int outputID)
#else
static void __cdecl AcceptInput_Hook(void* thisptr, const char* inputName, void* activator, void* caller, variant_t &parameter, int outputID)
#endif
{
    const char *entName = server->GetEntityName(thisptr);
    const char *className = server->GetEntityClassName(thisptr);
    if (!entName) entName = "";
    if (!className) className = "";

    std::optional<int> activatorSlot;

    if (activator && server->IsPlayer(activator)) {
        activatorSlot = server->GetSplitScreenPlayerSlot(activator);
    }

    SpeedrunTimer::TestInputRules(entName, className, inputName, parameter.ToString(), activatorSlot);

    if (engine->demorecorder->isRecordingDemo) {
        size_t entNameLen = strlen(entName);
        size_t classNameLen = strlen(className);
        size_t inputNameLen = strlen(inputName);

        const char *paramStr = parameter.ToString();

        size_t len = entNameLen + classNameLen + inputNameLen + strlen(paramStr) + 5;
        if (activatorSlot) {
            len += 1;
        }
        char *data = (char *)malloc(len);
        char *data1 = data;
        if (!activatorSlot) {
            data[0] = 0x03;
        } else {
            data[0] = 0x04;
            data[1] = *activatorSlot;
            ++data1;
        }
        strcpy(data1 + 1, entName);
        strcpy(data1 + 2 + entNameLen, className);
        strcpy(data1 + 3 + entNameLen + classNameLen, inputName);
        strcpy(data1 + 4 + entNameLen + classNameLen + inputNameLen, paramStr);
        engine->demorecorder->RecordData(data, len);
        free(data);
    }
    //console->DevMsg("%.4d INPUT '%s' TARGETNAME '%s' CLASSNAME '%s' PARAM '%s'\n", session->GetTick(), inputName, server->GetEntityName(thisptr), server->GetEntityClassName(thisptr), parameter.ToString());

    g_AcceptInputHook.Disable();
    server->AcceptInput(thisptr, inputName, activator, caller, parameter, outputID);
    g_AcceptInputHook.Enable();
}

// This is kinda annoying - it's got to be in a separate function
// because we need a reference to an entity vtable to find the address
// of CBaseEntity::AcceptInput, but we generally can't do that until
// we've loaded into a level.
static bool IsAcceptInputTrampolineInitialized = false;
Hook g_AcceptInputHook(&AcceptInput_Hook);
static void InitAcceptInputTrampoline()
{
    void* ent = server->m_EntPtrArray[0].m_pEntity;
    if (ent == nullptr) return;
    IsAcceptInputTrampolineInitialized = true;
    server->AcceptInput = Memory::VMT<Server::_AcceptInput>(ent, Offsets::AcceptInput);

    g_AcceptInputHook.SetFunc(server->AcceptInput);
}

static bool g_IsCMFlagHookInitialized = false;
static void InitCMFlagHook()
{
    for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
        void* ent = server->m_EntPtrArray[i].m_pEntity;
        if (!ent) continue;

        auto classname = server->GetEntityClassName(ent);
        if (!classname || strcmp(classname, "challenge_mode_end_node")) continue;

        Server::StartTouchChallengeNode = Memory::VMT<Server::_StartTouchChallengeNode>(ent, Offsets::StartTouch);
        g_flagStartTouchHook.SetFunc(Server::StartTouchChallengeNode);
        g_IsCMFlagHookInitialized = true;

        break;
    }
}

// CServerGameDLL::GameFrame
#ifdef _WIN32
DETOUR_STD(void, Server::GameFrame, bool simulating)
#else
DETOUR(Server::GameFrame, bool simulating)
#endif
{
    if (!IsAcceptInputTrampolineInitialized) InitAcceptInputTrampoline();
    if (!g_IsCMFlagHookInitialized) InitCMFlagHook();

    if (sar_tick_debug.GetInt() >= 3 || (sar_tick_debug.GetInt() >= 2 && simulating)) {
        int host, server, client;
        engine->GetTicks(host, server, client);
        console->Print("CServerGameDLL::GameFrame %s (host=%d server=%d client=%d)\n", simulating ? "simulating" : "non-simulating", host, server, client);
    }

    int tick = session->GetTick();

    Event::Trigger<Event::PRE_TICK>({ simulating, tick });

#ifdef _WIN32
    Server::GameFrame(simulating);
#else
    auto result = Server::GameFrame(thisptr, simulating);
#endif

    Event::Trigger<Event::POST_TICK>({ simulating, tick });

    ++server->tickCount;

#ifndef _WIN32
    return result;
#endif
}

static int (*GlobalEntity_GetIndex)(const char *);
static void (*GlobalEntity_SetFlags)(int, int);

static void resetCoopProgress(void *data, size_t size)
{
    GlobalEntity_SetFlags(GlobalEntity_GetIndex("glados_spoken_flags0"), 0);
    GlobalEntity_SetFlags(GlobalEntity_GetIndex("glados_spoken_flags1"), 0);
    GlobalEntity_SetFlags(GlobalEntity_GetIndex("glados_spoken_flags2"), 0);
    GlobalEntity_SetFlags(GlobalEntity_GetIndex("glados_spoken_flags3"), 0);
    engine->ExecuteCommand("mp_mark_all_maps_incomplete", true);
    engine->ExecuteCommand("mp_lock_all_taunts", true);
}


bool Server::Init()
{
    this->g_GameMovement = Interface::Create(this->Name(), "GameMovement001");
    this->g_ServerGameDLL = Interface::Create(this->Name(), "ServerGameDLL005");

    if (this->g_GameMovement) {
        this->g_GameMovement->Hook(Server::CheckJumpButton_Hook, Server::CheckJumpButton, Offsets::CheckJumpButton);
        this->g_GameMovement->Hook(Server::PlayerMove_Hook, Server::PlayerMove, Offsets::PlayerMove);

        this->g_GameMovement->Hook(Server::ProcessMovement_Hook, Server::ProcessMovement, Offsets::ProcessMovement);
        this->g_GameMovement->Hook(Server::FinishGravity_Hook, Server::FinishGravity, Offsets::FinishGravity);
        this->g_GameMovement->Hook(Server::AirMove_Hook, Server::AirMove, Offsets::AirMove);

        auto ctor = this->g_GameMovement->Original(0);
        auto baseCtor = Memory::Read(ctor + Offsets::AirMove_Offset1);
        uintptr_t baseOffset;
#ifndef _WIN32
        if (sar.game->Is(SourceGame_EIPRelPIC)) {
            baseOffset = baseCtor + 5 + *(uint32_t *)(baseCtor + 6) + *(uint32_t *)(baseCtor + 19);
        } else
#endif
        baseOffset = Memory::Deref<uintptr_t>(baseCtor + Offsets::AirMove_Offset2);
        Memory::Deref<_AirMove>(baseOffset + Offsets::AirMove * sizeof(uintptr_t*), &Server::AirMoveBase);

        Memory::Deref<_CheckJumpButton>(baseOffset + Offsets::CheckJumpButton * sizeof(uintptr_t*), &Server::CheckJumpButtonBase);

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

    if (auto g_ServerTools = Interface::Create(this->Name(), "VSERVERTOOLS001")) {
        auto GetIServerEntity = g_ServerTools->Original(Offsets::GetIServerEntity);
#ifndef _WIN32
        if (sar.game->Is(SourceGame_EIPRelPIC)) {
            this->m_EntPtrArray = (CEntInfo *)(GetIServerEntity + 12 + *(uint32_t *)(GetIServerEntity + 14) + *(uint32_t *)(GetIServerEntity + 54) + 4);
        } else
#endif
        Memory::Deref(GetIServerEntity + Offsets::m_EntPtrArray, &this->m_EntPtrArray);

        this->CreateEntityByName = g_ServerTools->Original<_CreateEntityByName>(Offsets::CreateEntityByName);
        this->DispatchSpawn = g_ServerTools->Original<_DispatchSpawn>(Offsets::DispatchSpawn);
        this->SetKeyValueChar = g_ServerTools->Original<_SetKeyValueChar>(Offsets::SetKeyValueChar);
        this->SetKeyValueFloat = g_ServerTools->Original<_SetKeyValueFloat>(Offsets::SetKeyValueFloat);
        this->SetKeyValueVector = g_ServerTools->Original<_SetKeyValueVector>(Offsets::SetKeyValueVector);

        Interface::Delete(g_ServerTools);
    }

    if (this->g_ServerGameDLL) {
        auto Think = this->g_ServerGameDLL->Original(Offsets::Think);
        Memory::Read<_UTIL_PlayerByIndex>(Think + Offsets::UTIL_PlayerByIndex, &this->UTIL_PlayerByIndex);
#ifndef _WIN32
        if (sar.game->Is(SourceGame_EIPRelPIC)) {
            this->gpGlobals = *(CGlobalVars **)((uintptr_t)this->UTIL_PlayerByIndex + 5 + *(uint32_t *)((uintptr_t)UTIL_PlayerByIndex + 7) + *(uint32_t *)((uintptr_t)UTIL_PlayerByIndex + 21));
        } else
#endif
        Memory::DerefDeref<CGlobalVars*>((uintptr_t)this->UTIL_PlayerByIndex + Offsets::gpGlobals, &this->gpGlobals);

        this->GetAllServerClasses = this->g_ServerGameDLL->Original<_GetAllServerClasses>(Offsets::GetAllServerClasses);
        this->IsRestoring = this->g_ServerGameDLL->Original<_IsRestoring>(Offsets::IsRestoring);

        this->g_ServerGameDLL->Hook(Server::GameFrame_Hook, Server::GameFrame, Offsets::GameFrame);
    }

#ifdef _WIN32
    GlobalEntity_GetIndex = (int (*)(const char *))Memory::Scan(server->Name(), "55 8B EC 51 8B 45 08 50 8D 4D FC 51 B9 ? ? ? ? E8 ? ? ? ? 66 8B 55 FC B8 FF FF 00 00", 0);
    GlobalEntity_SetFlags = (void (*)(int, int))Memory::Scan(server->Name(), "55 8B EC 80 3D ? ? ? ? 00 75 1F 8B 45 08 85 C0 78 18 3B 05 ? ? ? ? 7D 10 8B 4D 0C 8B 15 ? ? ? ? 8D 04 40 89 4C 82 08", 0);
#else
    if (sar.game->Is(SourceGame_EIPRelPIC)) {
        GlobalEntity_GetIndex = (int (*)(const char *))Memory::Scan(server->Name(), "53 E8 ? ? ? ? 81 ? ? ? ? 00 83 EC 18 8D 44 24 0E 83 EC 04 FF 74 24 24 8D ? ? ? ? ? 52 50 E8 ? ? ? ? 0F B7 4C 24 1A", 0);
        GlobalEntity_SetFlags = (void (*)(int, int))Memory::Scan(server->Name(), "E8 ? ? ? ? 05 ? ? ? ? 8B 54 24 04 80 B8 ? ? ? ? 01 74 21 85 D2 78 1D 3B 90 ? ? ? ? 7D 15 8B 88 ? ? ? ? 8D 14 52 8D 14 91", 0);
    } else {
        GlobalEntity_GetIndex = (int (*)(const char *))Memory::Scan(server->Name(), "55 89 E5 53 8D 45 F6 83 EC 24 8B 55 08 C7 44 24 04 ? ? ? ? 89 04 24 89 54 24 08", 0);
        GlobalEntity_SetFlags = (void (*)(int, int))Memory::Scan(server->Name(), "80 3D ? ? ? ? 00 55 89 E5 8B 45 08 75 1E 85 C0 78 1A 3B 05 ? ? ? ? 7D 12 8B 15", 0);
    }
#endif

    // Remove the limit on how quickly you can use 'say'
    void *say_callback = Command("say").ThisPtr()->m_pCommandCallback;
#ifdef _WIN32
    uintptr_t insn_addr = (uintptr_t)say_callback + 52;
#else
    uintptr_t insn_addr = (uintptr_t)say_callback + (sar.game->Is(SourceGame_EIPRelPIC) ? 67 : 88);
#endif
    // This is the location of an ADDSD instruction which adds 0.66
    // to the current time. If we instead *subtract* 0.66, we'll
    // always be able to chat again! We can just do this by changing
    // the third byte from 0x58 to 0x5C, hence making the full
    // opcode start with F2 0F 5C.
    Memory::UnProtect((void *)(insn_addr + 2), 1);
    *(char *)(insn_addr + 2) = 0x5C;

    NetMessage::RegisterHandler(RESET_COOP_PROGRESS_MESSAGE_TYPE, &resetCoopProgress);

    offsetFinder->ServerSide("CBasePlayer", "m_nWaterLevel", &Offsets::m_nWaterLevel);
    offsetFinder->ServerSide("CBasePlayer", "m_iName", &Offsets::m_iName);
    offsetFinder->ServerSide("CBasePlayer", "m_vecVelocity[0]", &Offsets::S_m_vecVelocity);
    offsetFinder->ServerSide("CBasePlayer", "m_fFlags", &Offsets::m_fFlags);
    offsetFinder->ServerSide("CBasePlayer", "m_flMaxspeed", &Offsets::m_flMaxspeed);
    offsetFinder->ServerSide("CBasePlayer", "m_vecViewOffset[0]", &Offsets::S_m_vecViewOffset);
    offsetFinder->ServerSide("CBasePlayer", "m_hGroundEntity", &Offsets::S_m_hGroundEntity);
    offsetFinder->ServerSide("CBasePlayer", "m_bDucked", &Offsets::m_bDucked);
    offsetFinder->ServerSide("CBasePlayer", "m_flFriction", &Offsets::m_flFriction);
    offsetFinder->ServerSide("CPortal_Player", "m_StatsThisLevel", &Offsets::m_StatsThisLevel);

    offsetFinder->ServerSide("CPortal_Player", "iNumPortalsPlaced", &Offsets::iNumPortalsPlaced);
    offsetFinder->ServerSide("CPortal_Player", "m_hActiveWeapon", &Offsets::m_hActiveWeapon);
    offsetFinder->ServerSide("CProp_Portal", "m_bActivated", &Offsets::m_bActivated);
    offsetFinder->ServerSide("CProp_Portal", "m_bIsPortal2", &Offsets::m_bIsPortal2);
    offsetFinder->ServerSide("CWeaponPortalgun", "m_bCanFirePortal1", &Offsets::m_bCanFirePortal1);
    offsetFinder->ServerSide("CWeaponPortalgun", "m_bCanFirePortal2", &Offsets::m_bCanFirePortal2);
    offsetFinder->ServerSide("CWeaponPortalgun", "m_hPrimaryPortal", &Offsets::m_hPrimaryPortal);
    offsetFinder->ServerSide("CWeaponPortalgun", "m_hSecondaryPortal", &Offsets::m_hSecondaryPortal);
    offsetFinder->ServerSide("CWeaponPortalgun", "m_iPortalLinkageGroupID", &Offsets::m_iPortalLinkageGroupID);

    sv_cheats = Variable("sv_cheats");
    sv_footsteps = Variable("sv_footsteps");
    sv_alternateticks = Variable("sv_alternateticks");
    sv_bonus_challenge = Variable("sv_bonus_challenge");
    sv_accelerate = Variable("sv_accelerate");
    sv_airaccelerate = Variable("sv_airaccelerate");
    sv_friction = Variable("sv_friction");
    sv_maxspeed = Variable("sv_maxspeed");
    sv_stopspeed = Variable("sv_stopspeed");
    sv_maxvelocity = Variable("sv_maxvelocity");
    sv_gravity = Variable("sv_gravity");

    return this->hasLoaded = this->g_GameMovement && this->g_ServerGameDLL;
}
CON_COMMAND(sar_coop_reset_progress, "sar_coop_reset_progress - resets all coop progress\n")
{
    if (engine->IsCoop()) {
        NetMessage::SendMsg(RESET_COOP_PROGRESS_MESSAGE_TYPE, nullptr, 0);
        resetCoopProgress(nullptr, 0);
    }
}
void Server::Shutdown()
{
#ifdef _WIN32
    MH_UNHOOK(this->AirMove_Mid);
#endif
    Interface::Delete(this->g_GameMovement);
    Interface::Delete(this->g_ServerGameDLL);
}

Server* server;
