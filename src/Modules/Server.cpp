#include "Server.hpp"

#include "Client.hpp"
#include "Engine.hpp"
#include "Event.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/EntityList.hpp"
#include "Features/FovChanger.hpp"
#include "Features/GroundFramesCounter.hpp"
#include "Features/Hud/Crosshair.hpp"
#include "Features/Hud/ScrollSpeed.hpp"
#include "Features/Hud/StrafeQuality.hpp"
#include "Features/Hud/InputHud.hpp"
#include "Features/NetMessage.hpp"
#include "Features/OffsetFinder.hpp"
#include "Features/PlayerTrace.hpp"
#include "Features/ReloadedFix.hpp"
#include "Features/Routing/EntityInspector.hpp"
#include "Features/Routing/SeamshotFind.hpp"
#include "Features/SegmentedTools.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Tas/TasController.hpp"
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTools/StrafeTool.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/TimescaleDetect.hpp"
#include "Game.hpp"
#include "Hook.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

#include "Features/OverlayRender.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <cfloat>

#define RESET_COOP_PROGRESS_MESSAGE_TYPE "coop-reset"
#define CM_FLAGS_MESSAGE_TYPE "cm-flags"

Variable sv_cheats;
Variable sv_footsteps;
Variable sv_alternateticks;
Variable sv_bonus_challenge;
Variable sv_accelerate;
Variable sv_airaccelerate;
Variable sv_paintairacceleration;
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
REDECL(Server::PlayerRunCommand);
REDECL(Server::ProcessMovement);
REDECL(Server::StartTouchChallengeNode);
REDECL(Server::say_callback);

MDECL(Server::GetPortals, int, iNumPortalsPlaced);
MDECL(Server::GetAbsOrigin, Vector, S_m_vecAbsOrigin);
MDECL(Server::GetAbsAngles, QAngle, S_m_angAbsRotation);
MDECL(Server::GetLocalVelocity, Vector, S_m_vecVelocity);
MDECL(Server::GetFlags, int, m_fFlags);
MDECL(Server::GetEFlags, int, m_iEFlags);
MDECL(Server::GetMaxSpeed, float, m_flMaxspeed);
MDECL(Server::GetGravity, float, m_flGravity);
MDECL(Server::GetViewOffset, Vector, S_m_vecViewOffset);
MDECL(Server::GetPortalLocal, CPortalPlayerLocalData, S_m_PortalLocal);
MDECL(Server::GetEntityName, char *, m_iName);
MDECL(Server::GetEntityClassName, char *, m_iClassName);

void *Server::GetPlayer(int index) {
	return this->UTIL_PlayerByIndex(index);
}
bool Server::IsPlayer(void *entity) {
	return Memory::VMT<bool (*)(void *)>(entity, Offsets::IsPlayer)(entity);
}
bool Server::AllowsMovementChanges() {
	return sv_cheats.GetBool();
}
int Server::GetSplitScreenPlayerSlot(void *entity) {
	// Simplified version of CBasePlayer::GetSplitScreenPlayerSlot
	for (auto i = 0; i < Offsets::MAX_SPLITSCREEN_PLAYERS; ++i) {
		if (server->UTIL_PlayerByIndex(i + 1) == entity) {
			return i;
		}
	}

	return 0;
}
void Server::KillEntity(void *entity) {
	variant_t val = {0};
	val.fieldType = FIELD_VOID;
	void *player = this->GetPlayer(1);
	server->AcceptInput(entity, "Kill", player, player, val, 0);
}

float Server::GetCMTimer() {
	void *player = this->GetPlayer(1);
	if (!player) {
		void *clPlayer = client->GetPlayer(1);
		if (!clPlayer) return 0.0f;
		return *(float *)((uintptr_t)clPlayer + Offsets::C_m_StatsThisLevel + 12);
	}
	return *(float *)((uintptr_t)player + Offsets::S_m_StatsThisLevel + 12);
}

// CGameMovement::CheckJumpButton
DETOUR_T(bool, Server::CheckJumpButton) {
	auto jumped = false;

	if (server->AllowsMovementChanges()) {
		auto mv = *reinterpret_cast<CHLMoveData **>((uintptr_t)thisptr + Offsets::mv);

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
		auto player = *reinterpret_cast<void **>((uintptr_t)thisptr + Offsets::player);
		auto stat = stats->Get(server->GetSplitScreenPlayerSlot(player));
		++stat->jumps->total;
		++stat->steps->total;
		stat->jumps->StartTrace(server->GetAbsOrigin(player));
	}

	return jumped;
}

// CGameMovement::PlayerMove
DETOUR(Server::PlayerMove) {
	auto player = *reinterpret_cast<void **>((uintptr_t)thisptr + Offsets::player);
	auto mv = *reinterpret_cast<const CHLMoveData **>((uintptr_t)thisptr + Offsets::mv);

	if (sar_crosshair_mode.GetBool() || sar_quickhud_mode.GetBool() || sar_crosshair_P1.GetBool()) {
		auto m_hActiveWeapon = *reinterpret_cast<CBaseHandle *>((uintptr_t)player + Offsets::m_hActiveWeapon);
		server->portalGun = entityList->LookupEntity(m_hActiveWeapon);
	}

	auto m_fFlags = *reinterpret_cast<int *>((uintptr_t)player + Offsets::m_fFlags);
	auto m_MoveType = *reinterpret_cast<int *>((uintptr_t)player + Offsets::m_MoveType);
	auto m_nWaterLevel = *reinterpret_cast<int *>((uintptr_t)player + Offsets::m_nWaterLevel);

	auto stat = stats->Get(server->GetSplitScreenPlayerSlot(player));

	// Landed after a jump
	if (stat->jumps->isTracing && m_fFlags & FL_ONGROUND && m_MoveType != MOVETYPE_NOCLIP) {
		stat->jumps->EndTrace(server->GetAbsOrigin(player), sar_stats_jumps_xy.GetBool());
	}

	stepCounter->ReduceTimer(server->gpGlobals->frametime);

	// Player is on ground and moving etc.
	if (stepCounter->stepSoundTime <= 0 && m_MoveType != MOVETYPE_NOCLIP && sv_footsteps.GetFloat() && !(m_fFlags & (FL_FROZEN | FL_ATCONTROLS)) && ((m_fFlags & FL_ONGROUND && mv->m_vecVelocity.Length2D() > 0.0001f) || m_MoveType == MOVETYPE_LADDER)) {
		stepCounter->Increment(m_fFlags, m_MoveType, mv->m_vecVelocity, m_nWaterLevel);
		++stat->steps->total;
	}

	stat->velocity->Save(server->GetLocalVelocity(player), sar_stats_velocity_peak_xy.GetBool());
	inspector->Record();

	return Server::PlayerMove(thisptr);
}

extern Hook g_playerRunCommandHook;
// CPortal_Player::PlayerRunCommand
DETOUR(Server::PlayerRunCommand, CUserCmd *cmd, void *moveHelper) {
	if (!engine->IsGamePaused()) {
		if (sar_tas_real_controller_debug.GetInt() == 3) {
			auto playerInfo = tasPlayer->GetPlayerInfo(thisptr, cmd);
			console->Print("Jump input state at tick %d: %s\n", playerInfo.tick, (cmd->buttons & IN_JUMP) ? "true" : "false");
		}
	}

	int slot = server->GetSplitScreenPlayerSlot(thisptr);

	if (tasPlayer->IsActive()) {
		int tasTick = tasPlayer->GetPlayerInfo(thisptr, cmd).tick - tasPlayer->GetStartTick();
		tasPlayer->DumpUsercmd(slot, cmd, tasTick, "server");

		Vector pos = server->GetAbsOrigin(thisptr);
		Vector eye_pos = pos + server->GetViewOffset(thisptr) + server->GetPortalLocal(thisptr).m_vEyeOffset;
		tasPlayer->DumpPlayerInfo(slot, tasTick, pos, eye_pos, cmd->viewangles);
	}

	if (tasPlayer->IsActive() && tasPlayer->IsUsingTools(slot)) {
		tasPlayer->PostProcess(slot, thisptr, cmd);
	}

	inputHud.SetInputInfo(slot, cmd->buttons, {cmd->sidemove, cmd->forwardmove, cmd->upmove});

	g_playerRunCommandHook.Disable();
	auto ret = Server::PlayerRunCommand(thisptr, cmd, moveHelper);
	g_playerRunCommandHook.Enable();

	return ret;
}
Hook g_playerRunCommandHook(&Server::PlayerRunCommand_Hook);

// CGameMovement::ProcessMovement
DETOUR(Server::ProcessMovement, void *player, CMoveData *move) {
	int slot = server->GetSplitScreenPlayerSlot(player);
	unsigned int groundHandle = *(unsigned int *)((uintptr_t)player + Offsets::S_m_hGroundEntity);
	bool grounded = groundHandle != 0xFFFFFFFF;
	groundFramesCounter->HandleMovementFrame(slot, grounded);
	strafeQuality.OnMovement(slot, grounded);
	if (move->m_nButtons & IN_JUMP) scrollSpeedHud.OnJump(slot);
	Event::Trigger<Event::PROCESS_MOVEMENT>({ slot, true });

	auto res = Server::ProcessMovement(thisptr, player, move);

	// We edit pos after process movement to get accurate teleportation
	// This is for sar_trace_teleport_at
	if (g_playerTraceNeedsTeleport && slot == g_playerTraceTeleportSlot) {
		move->m_vecAbsOrigin = g_playerTraceTeleportLocation;
		g_playerTraceNeedsTeleport = false;
	}

	return res;
}

ON_INIT {
	NetMessage::RegisterHandler(
		CM_FLAGS_MESSAGE_TYPE, +[](const void *data, size_t size) {
			if (size == 6 && engine->IsOrange()) {
				char slot = *(char *)data;
				float time = *(float *)((uintptr_t)data + 1);
				bool end = !!*(char *)((uintptr_t)data + 5);
				Event::Trigger<Event::CM_FLAGS>({slot, time, end});
			}
		});
}

static inline bool hasSlotCompleted(void *thisptr, int slot) {
#ifdef _WIN32
	return *(uint8_t *)((uintptr_t)thisptr + 0x4B0 + slot);
#else
	return *(uint8_t *)((uintptr_t)thisptr + 0x4C8 + slot);
#endif
}

static inline bool isFakeFlag(void *thisptr) {
	return hasSlotCompleted(thisptr, 2);
}

static void TriggerCMFlag(int slot, float time, bool end) {
	Event::Trigger<Event::CM_FLAGS>({slot, time, end});
	if (engine->IsCoop()) {
		char data[6];
		data[0] = (char)slot;
		*(float *)(data + 1) = time;
		data[5] = (char)end;
		NetMessage::SendMsg(CM_FLAGS_MESSAGE_TYPE, data, sizeof data);
	}
}

extern Hook g_flagStartTouchHook;
DETOUR(Server::StartTouchChallengeNode, void *entity) {
	if (server->IsPlayer(entity) && !isFakeFlag(thisptr) && client->GetChallengeStatus() == CMStatus::CHALLENGE) {
		int slot = server->GetSplitScreenPlayerSlot(entity);
		if (!hasSlotCompleted(thisptr, slot)) {
			float time = server->GetCMTimer();
			bool end = !engine->IsCoop() || hasSlotCompleted(thisptr, slot == 1 ? 0 : 1);
			TriggerCMFlag(slot, time, end);
		}
	}

	g_flagStartTouchHook.Disable();
	auto ret = Server::StartTouchChallengeNode(thisptr, entity);
	g_flagStartTouchHook.Enable();

	return ret;
}
Hook g_flagStartTouchHook(&Server::StartTouchChallengeNode_Hook);

// CGameMovement::FinishGravity
DETOUR(Server::FinishGravity) {
	if (server->callFromCheckJumpButton) {
		if (sar_duckjump.GetBool()) {
			auto player = *reinterpret_cast<uintptr_t *>((uintptr_t)thisptr + Offsets::player);
			auto mv = *reinterpret_cast<CHLMoveData **>((uintptr_t)thisptr + Offsets::mv);

			auto m_pSurfaceData = *reinterpret_cast<uintptr_t *>(player + Offsets::m_pSurfaceData);
			auto m_bDucked = *reinterpret_cast<bool *>(player + Offsets::S_m_bDucked);
			auto m_fFlags = *reinterpret_cast<int *>(player + Offsets::m_fFlags);

			auto flGroundFactor = (m_pSurfaceData) ? *reinterpret_cast<float *>(m_pSurfaceData + Offsets::jumpFactor) : 1.0f;
			auto flMul = std::sqrt(2 * sv_gravity.GetFloat() * GAMEMOVEMENT_JUMP_HEIGHT);

			if (m_bDucked || m_fFlags & FL_DUCKING) {
				mv->m_vecVelocity[2] = flGroundFactor * flMul;
			} else {
				mv->m_vecVelocity[2] = server->savedVerticalVelocity + flGroundFactor * flMul;
			}
		}

		if (sar_jumpboost.GetBool()) {
			auto player = *reinterpret_cast<uintptr_t *>((uintptr_t)thisptr + Offsets::player);
			auto mv = *reinterpret_cast<CHLMoveData **>((uintptr_t)thisptr + Offsets::mv);

			auto m_bDucked = *reinterpret_cast<bool *>(player + Offsets::S_m_bDucked);

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
DETOUR_B(Server::AirMove) {
	if (sar_aircontrol.GetInt() >= 2 && server->AllowsMovementChanges()) {
		return Server::AirMoveBase(thisptr);
	}

	return Server::AirMove(thisptr);
}
static void setAircontrol(int val) {
	switch (val) {
	case 0:
		*server->aircontrol_fling_speed_addr = 300.0f * 300.0f;
		break;
	default:
		*server->aircontrol_fling_speed_addr = INFINITY;
		break;
	}
}
// cvar callbacks dont want to fucking work so we'll just do this bs
ON_EVENT(PRE_TICK) {
	setAircontrol(server->AllowsMovementChanges() ? sar_aircontrol.GetInt() : 0);
}

extern Hook g_AcceptInputHook;

// TODO: the windows signature is a bit dumb. fastcall is like thiscall
// but for normal functions and takes an arg in edx, so we use it
// because msvc won't let us use thiscall on a non-member function
#ifdef _WIN32
static void __fastcall AcceptInput_Hook(void *thisptr, void *unused, const char *inputName, void *activator, void *caller, variant_t parameter, int outputID)
#else
static void __cdecl AcceptInput_Hook(void *thisptr, const char *inputName, void *activator, void *caller, variant_t &parameter, int outputID)
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

	if (sar_show_entinp.GetBool() && sv_cheats.GetBool()) {
		console->Print("%.4d %s.%s(%s)\n", session->GetTick(), server->GetEntityName(thisptr), inputName, parameter.ToString());
	}

	// HACKHACK
	// Deals with maps where you hit a normal transition trigger
	if (!strcmp(className, "portal_stats_controller") && !strcmp(inputName, "OnLevelEnd") && client->GetChallengeStatus() == CMStatus::CHALLENGE) {
		float time = server->GetCMTimer();
		if (engine->IsCoop()) {
			TriggerCMFlag(0, time, false);
			TriggerCMFlag(1, time, true);
		} else {
			TriggerCMFlag(0, time, true);
		}
	}

	// allow reloaded fix to override some commands from point_servercommand
	reloadedFix->OverrideInput(className, inputName, &parameter);

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
static void InitAcceptInputTrampoline() {
	void *ent = server->m_EntPtrArray[0].m_pEntity;
	if (ent == nullptr) return;
	IsAcceptInputTrampolineInitialized = true;
	server->AcceptInput = Memory::VMT<Server::_AcceptInput>(ent, Offsets::AcceptInput);

	g_AcceptInputHook.SetFunc(server->AcceptInput);
}

static bool g_IsCMFlagHookInitialized = false;
static void InitCMFlagHook() {
	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;

		auto classname = server->GetEntityClassName(ent);
		if (!classname || strcmp(classname, "challenge_mode_end_node")) continue;

		Server::StartTouchChallengeNode = Memory::VMT<Server::_StartTouchChallengeNode>(ent, Offsets::StartTouch);
		g_flagStartTouchHook.SetFunc(Server::StartTouchChallengeNode);
		g_IsCMFlagHookInitialized = true;

		break;
	}
}

static bool g_IsPlayerRunCommandHookInitialized = false;
static void InitPlayerRunCommandHook() {
	void *player = server->GetPlayer(1);
	if (!player) return;
	Server::PlayerRunCommand = Memory::VMT<Server::_PlayerRunCommand>(player, Offsets::PlayerRunCommand);
	g_playerRunCommandHook.SetFunc(Server::PlayerRunCommand);
	g_IsPlayerRunCommandHookInitialized = true;
}

// CServerGameDLL::GameFrame
DETOUR(Server::GameFrame, bool simulating)
{
	if (!IsAcceptInputTrampolineInitialized) InitAcceptInputTrampoline();
	if (!g_IsCMFlagHookInitialized) InitCMFlagHook();
	if (!g_IsPlayerRunCommandHookInitialized) InitPlayerRunCommandHook();

	if (sar_tick_debug.GetInt() >= 3 || (sar_tick_debug.GetInt() >= 2 && simulating)) {
		int host, server, client;
		engine->GetTicks(host, server, client);
		console->Print("CServerGameDLL::GameFrame %s (host=%d server=%d client=%d)\n", simulating ? "simulating" : "non-simulating", host, server, client);
	}

	tasPlayer->Update();

	int tick = session->GetTick();

	Event::Trigger<Event::PRE_TICK>({simulating, tick});

	auto result = Server::GameFrame(thisptr, simulating);

	Event::Trigger<Event::POST_TICK>({simulating, tick});

	++server->tickCount;

	return result;
}

static int (*GlobalEntity_GetIndex)(const char *);
static void (*GlobalEntity_SetFlags)(int, int);

static void resetCoopProgress() {
	GlobalEntity_SetFlags(GlobalEntity_GetIndex("glados_spoken_flags0"), 0);
	GlobalEntity_SetFlags(GlobalEntity_GetIndex("glados_spoken_flags1"), 0);
	GlobalEntity_SetFlags(GlobalEntity_GetIndex("glados_spoken_flags2"), 0);
	GlobalEntity_SetFlags(GlobalEntity_GetIndex("glados_spoken_flags3"), 0);
	GlobalEntity_SetFlags(GlobalEntity_GetIndex("have_seen_dlc_tubes_reveal"), 0);
	engine->ExecuteCommand("mp_mark_all_maps_incomplete", true);
	engine->ExecuteCommand("mp_lock_all_taunts", true);
}

static int g_sendResetDoneAt = -1;

ON_EVENT(SESSION_START) {
	g_sendResetDoneAt = -1;
}

ON_EVENT(POST_TICK) {
	if (g_sendResetDoneAt != -1 && session->GetTick() >= g_sendResetDoneAt) {
		g_sendResetDoneAt = -1;
		NetMessage::SendMsg(RESET_COOP_PROGRESS_MESSAGE_TYPE, (void *)"done", 4);
	}
}

static void netResetCoopProgress(const void *data, size_t size) {
	if (size == 4 && !memcmp(data, "done", 4)) {
		Event::Trigger<Event::COOP_RESET_DONE>({});
	} else {
		resetCoopProgress();
		Event::Trigger<Event::COOP_RESET_REMOTE>({});
		g_sendResetDoneAt = session->GetTick() + 10; // send done message in 10 ticks, to be safe
	}
}

float hostTimeWrap() {
	return engine->GetHostTime();
}

static char g_orig_check_stuck_code[6];
static void *g_check_stuck_code;

bool Server::Init() {
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
		Memory::Deref<_AirMove>(baseOffset + Offsets::AirMove * sizeof(uintptr_t *), &Server::AirMoveBase);

		Memory::Deref<_CheckJumpButton>(baseOffset + Offsets::CheckJumpButton * sizeof(uintptr_t *), &Server::CheckJumpButtonBase);

		uintptr_t airMove = (uintptr_t)AirMove;
#ifdef _WIN32
		if (sar.game->Is(SourceGame_Portal2)) {
			this->aircontrol_fling_speed_addr = *(float **)(airMove + 791);
		} else {
			this->aircontrol_fling_speed_addr = *(float **)(airMove + 662);
		}
#else
		if (sar.game->Is(SourceGame_EIPRelPIC)) {
			this->aircontrol_fling_speed_addr = (float *)(airMove + 8 + *(uint32_t *)(airMove + 10) + *(uint32_t *)(airMove + 677));
		} else {
			this->aircontrol_fling_speed_addr = *(float **)(airMove + 524);
		}
#endif
		Memory::UnProtect(this->aircontrol_fling_speed_addr, 4);
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
			Memory::DerefDeref<CGlobalVars *>((uintptr_t)this->UTIL_PlayerByIndex + Offsets::gpGlobals, &this->gpGlobals);

		this->GetAllServerClasses = this->g_ServerGameDLL->Original<_GetAllServerClasses>(Offsets::GetAllServerClasses);
		this->IsRestoring = this->g_ServerGameDLL->Original<_IsRestoring>(Offsets::IsRestoring);

		this->g_ServerGameDLL->Hook(Server::GameFrame_Hook, Server::GameFrame, Offsets::GameFrame);
	}

#ifdef _WIN32
	GlobalEntity_GetIndex = (int (*)(const char *))Memory::Scan(server->Name(), "55 8B EC 51 8B 45 08 50 8D 4D FC 51 B9 ? ? ? ? E8 ? ? ? ? 66 8B 55 FC B8 FF FF 00 00", 0);
	GlobalEntity_SetFlags = (void (*)(int, int))Memory::Scan(server->Name(), "55 8B EC 80 3D ? ? ? ? 00 75 1F 8B 45 08 85 C0 78 18 3B 05 ? ? ? ? 7D 10 8B 4D 0C 8B 15 ? ? ? ? 8D 04 40 89 4C 82 08", 0);
#else
	if (sar.game->Is(SourceGame_EIPRelPIC)) {
		GlobalEntity_GetIndex = (int (*)(const char *))Memory::Scan(server->Name(), "53 E8 ? ? ? ? 81 C3 ? ? ? ? 83 EC 18 8D 44 24 0E 83 EC 04 FF 74 24 24 8D 93 ? ? ? ? 52 50 E8 ? ? ? ? 0F B7 4C 24 1A", 0);
		GlobalEntity_SetFlags = (void (*)(int, int))Memory::Scan(server->Name(), "E8 ? ? ? ? 05 ? ? ? ? 8B 54 24 04 80 B8 ? ? ? ? 01 74 21 85 D2 78 1D 3B 90 ? ? ? ? 7D 15 8B 88 ? ? ? ? 8D 14 52 8D 14 91", 0);
	} else {
		GlobalEntity_GetIndex = (int (*)(const char *))Memory::Scan(server->Name(), "55 89 E5 53 8D 45 F6 83 EC 24 8B 55 08 C7 44 24 04 ? ? ? ? 89 04 24 89 54 24 08", 0);
		GlobalEntity_SetFlags = (void (*)(int, int))Memory::Scan(server->Name(), "80 3D ? ? ? ? 00 55 89 E5 8B 45 08 75 1E 85 C0 78 1A 3B 05 ? ? ? ? 7D 12 8B 15", 0);
	}
#endif

	// Remove the limit on how quickly you can use 'say', and also hook it
	Command::Hook("say", Server::say_callback_hook, Server::say_callback);
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

	// find the TraceFirePortal function
#ifdef _WIN32
	TraceFirePortal = (_TraceFirePortal)Memory::Scan(server->Name(), "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC 38 07 00 00 56 57 8B F1", 0);
	FindPortal = (_FindPortal)Memory::Scan(server->Name(), "55 8B EC 0F B6 45 08 8D 0C 80 03 C9 53 8B 9C 09 ? ? ? ? 03 C9 56 57 85 DB 74 3C 8B B9 ? ? ? ? 33 C0 33 F6 EB 08", 0);
#else
	TraceFirePortal = (_TraceFirePortal)Memory::Scan(server->Name(), "55 89 E5 57 56 8D BD F4 F8 FF FF 53 E8 ? ? ? ? 81 C3 ? ? ? ? 81 EC 40 07 00 00 8B 45 14 6A 00 8B 75 0C", 0);
	FindPortal = (_FindPortal)Memory::Scan(server->Name(), "55 57 56 E8 ? ? ? ? 81 C6 ? ? ? ? 53 83 EC 2C 8B 44 24 40 8B 54 24 44 8B 7C 24 48 89 44 24 18 0F B6 C0", 0);
#endif

	{
		// a call to Plat_FloatTime in CGameMovement::CheckStuck
#ifdef _WIN32
		uintptr_t code = Memory::Scan(this->Name(), "FF ? ? ? ? ? D9 5D F8 8B 56 04 8B 42 1C 8B ? ? ? ? ? 3B C3 75 04 33 C9 EB 08 8B C8 2B 4A 58 C1 F9 04 F3 0F 10 84 CE 70", 0);
#else
		uintptr_t code;
		if (sar.game->Is(SourceGame_EIPRelPIC)) {
			code = Memory::Scan(this->Name(), "E8 ? ? ? ? 8B 46 04 66 0F EF C0 DD 5C 24 08 F2 0F 5A 44 24 08 8B 40 24 85 C0", 0);
		} else {
			code = Memory::Scan(this->Name(), "E8 ? ? ? ? 8B 43 04 DD 9D ? ? ? ? F2 0F 10 B5 ? ? ? ? 8B 50 24 66 0F 14 F6 66 0F 5A CE 85 D2", 0);
		}
#endif
		Memory::UnProtect((void *)code, sizeof g_orig_check_stuck_code);
		memcpy(g_orig_check_stuck_code, (void *)code, sizeof g_orig_check_stuck_code);

		*(uint8_t *)code = 0xE8;
		*(uint32_t *)(code + 1) = (uint32_t)&hostTimeWrap - (code + 5);
#ifdef _WIN32
		*(uint8_t *)(code + 5) = 0x90; // nop
#endif

		g_check_stuck_code = (void *)code;
	}

	NetMessage::RegisterHandler(RESET_COOP_PROGRESS_MESSAGE_TYPE, &netResetCoopProgress);

	offsetFinder->ServerSide("CBasePlayer", "m_nWaterLevel", &Offsets::m_nWaterLevel);
	offsetFinder->ServerSide("CBasePlayer", "m_iName", &Offsets::m_iName);
	offsetFinder->ServerSide("CBasePlayer", "m_vecVelocity[0]", &Offsets::S_m_vecVelocity);
	offsetFinder->ServerSide("CBasePlayer", "m_fFlags", &Offsets::m_fFlags);
	offsetFinder->ServerSide("CBasePlayer", "m_flMaxspeed", &Offsets::m_flMaxspeed);
	offsetFinder->ServerSide("CBasePlayer", "m_vecViewOffset[0]", &Offsets::S_m_vecViewOffset);
	offsetFinder->ServerSide("CBasePlayer", "m_hGroundEntity", &Offsets::S_m_hGroundEntity);
	offsetFinder->ServerSide("CBasePlayer", "m_bDucked", &Offsets::S_m_bDucked);
	offsetFinder->ServerSide("CBasePlayer", "m_flFriction", &Offsets::m_flFriction);
	offsetFinder->ServerSide("CBasePlayer", "m_nTickBase", &Offsets::m_nTickBase);
	offsetFinder->ServerSide("CBasePlayer", "m_nJumpTimeMsecs", &Offsets::S_m_nJumpTimeMsecs);
	offsetFinder->ServerSide("CBasePlayer", "m_Collision", &Offsets::S_m_Collision);
	offsetFinder->ServerSide("CPortal_Player", "m_lifeState", &Offsets::m_lifeState);
	offsetFinder->ServerSide("CPortal_Player", "m_InAirState", &Offsets::m_InAirState);
	offsetFinder->ServerSide("CPortal_Player", "m_StatsThisLevel", &Offsets::S_m_StatsThisLevel);
	offsetFinder->ServerSide("CPortal_Player", "m_PortalLocal", &Offsets::S_m_PortalLocal);
	offsetFinder->ServerSide("CPortal_Player", "m_nPlayerCond", &Offsets::S_m_nPlayerCond);

	offsetFinder->ServerSide("CPortal_Player", "iNumPortalsPlaced", &Offsets::iNumPortalsPlaced);
	offsetFinder->ServerSide("CPortal_Player", "m_hActiveWeapon", &Offsets::m_hActiveWeapon);
	offsetFinder->ServerSide("CProp_Portal", "m_bActivated", &Offsets::m_bActivated);
	offsetFinder->ServerSide("CProp_Portal", "m_bIsPortal2", &Offsets::m_bIsPortal2);
	offsetFinder->ServerSide("CWeaponPortalgun", "m_bCanFirePortal1", &Offsets::m_bCanFirePortal1);
	offsetFinder->ServerSide("CWeaponPortalgun", "m_bCanFirePortal2", &Offsets::m_bCanFirePortal2);
	offsetFinder->ServerSide("CWeaponPortalgun", "m_hPrimaryPortal", &Offsets::m_hPrimaryPortal);
	offsetFinder->ServerSide("CWeaponPortalgun", "m_hSecondaryPortal", &Offsets::m_hSecondaryPortal);
	offsetFinder->ServerSide("CPortal_Player", "m_hPortalEnvironment", &Offsets::S_m_hPortalEnvironment);
	offsetFinder->ServerSide("CPortal_Base2D", "m_ptOrigin", &Offsets::S_m_ptOrigin);
	Offsets::S_m_vForward = Offsets::S_m_ptOrigin + 12;

	int m_hViewModel;
	offsetFinder->ServerSide("CBasePlayer", "m_hViewModel", &m_hViewModel);
	Offsets::S_m_LastCmd = m_hViewModel + 8;

	sv_cheats = Variable("sv_cheats");
	sv_footsteps = Variable("sv_footsteps");
	sv_alternateticks = Variable("sv_alternateticks");
	sv_bonus_challenge = Variable("sv_bonus_challenge");
	sv_accelerate = Variable("sv_accelerate");
	sv_airaccelerate = Variable("sv_airaccelerate");
	sv_paintairacceleration = Variable("sv_paintairacceleration");
	sv_friction = Variable("sv_friction");
	sv_maxspeed = Variable("sv_maxspeed");
	sv_stopspeed = Variable("sv_stopspeed");
	sv_maxvelocity = Variable("sv_maxvelocity");
	sv_gravity = Variable("sv_gravity");

	return this->hasLoaded = this->g_GameMovement && this->g_ServerGameDLL;
}
CON_COMMAND(sar_coop_reset_progress, "sar_coop_reset_progress - resets all coop progress\n") {
	if (engine->IsCoop()) {
		NetMessage::SendMsg(RESET_COOP_PROGRESS_MESSAGE_TYPE, nullptr, 0);
		resetCoopProgress();
	}
}
CON_COMMAND(sar_give_fly, "sar_give_fly [n] - gives the player in slot n (0 by default) preserved crouchfly.\n") {
	if (args.ArgC() > 2) return console->Print(sar_give_fly.ThisPtr()->m_pszHelpString);
	if (!sv_cheats.GetBool()) return console->Print("sar_give_fly requires sv_cheats.\n");
	int slot = args.ArgC() == 2 ? atoi(args[1]) : 0;
	void *player = server->GetPlayer(slot + 1);
	if (player) {
		*(float *)((uintptr_t)player + Offsets::m_flGravity) = FLT_MIN;
		console->Print("Gave fly to player %d\n", slot);
	}
}
CON_COMMAND(sar_give_betsrighter, "sar_give_betsrighter [n] - gives the player in slot n (0 by default) betsrighter.\n") {
	if (args.ArgC() > 2) return console->Print(sar_give_fly.ThisPtr()->m_pszHelpString);
	if (!sv_cheats.GetBool()) return console->Print("sar_give_betsrighter requires sv_cheats.\n");
	int slot = args.ArgC() == 2 ? atoi(args[1]) : 0;
	void *player = server->GetPlayer(slot + 1);
	if (player) {
		*(char *)((uintptr_t)player + Offsets::m_takedamage) = 0;
		console->Print("Gave betsrighter to player %d\n", slot);
	}
}
DETOUR_COMMAND(Server::say) {
	if (args.ArgC() != 2 || Utils::StartsWith(args[1], "!SAR:") || !networkManager.HandleGhostSay(args[1])) {
		Server::say_callback(args);
	}
}
void Server::Shutdown() {
	Command::Unhook("say", Server::say_callback);
	setAircontrol(0);
	if (g_check_stuck_code) memcpy(g_check_stuck_code, g_orig_check_stuck_code, sizeof g_orig_check_stuck_code);
	Interface::Delete(this->g_GameMovement);
	Interface::Delete(this->g_ServerGameDLL);
}

Server *server;
