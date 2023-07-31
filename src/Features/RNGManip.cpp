#include "RNGManip.hpp"

#include "Event.hpp"
#include "Features/Tas/TasPlayer.hpp"
#include "Hook.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/FileSystem.hpp"
#include "Modules/Server.hpp"
#include "Offsets.hpp"
#include "PlayerTrace.hpp"
#include "Utils/json11.hpp"
#include "Utils/SDK.hpp"

#include <cstring>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>
#include <set>

static std::optional<json11::Json> g_session_state;
static std::optional<json11::Json> g_pending_load;

static std::deque<QAngle> g_queued_view_punches;
static std::vector<QAngle> g_recorded_view_punches;
static std::deque<int> g_queued_randomseeds;
static std::vector<int> g_recorded_randomseeds;

static json11::Json saveViewPunches() {
	std::vector<json11::Json> punches;

	for (QAngle punch : g_recorded_view_punches) {
		std::vector<json11::Json> ang{ {(double)punch.x, (double)punch.y, (double)punch.z} };
		punches.push_back(json11::Json(ang));
	}

	return json11::Json(punches);
}

static bool restoreViewPunches(const json11::Json &data) {
	if (!data.is_array()) return false;

	for (auto &val : data.array_items()) {
		float x = (float)val[0].number_value();
		float y = (float)val[1].number_value();
		float z = (float)val[2].number_value();
		g_queued_view_punches.push_back({x,y,z});
	}

	return true;
}

static json11::Json savePaintSprayers() {
	std::vector<json11::Json> vals;

	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;

		auto classname = server->GetEntityClassName(ent);
		if (!classname || strcmp(classname, "info_paint_sprayer")) continue;

		int seed = SE(ent)->field<int>("m_nBlobRandomSeed");
		vals.push_back({seed});
	}

	return vals;
}

static bool restorePaintSprayers(const json11::Json &data) {
	if (!data.is_array()) return false;

	size_t idx = 0;

	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;

		auto classname = server->GetEntityClassName(ent);
		if (!classname || strcmp(classname, "info_paint_sprayer")) continue;

		if (idx == data.array_items().size()) {
			// bad count
			return false;
		}

		SE(ent)->field<int>("m_nBlobRandomSeed") = data[idx].int_value();
		++idx;
	}
	return idx == data.array_items().size();
}

static json11::Json saveRandomSeeds() {
	std::vector<json11::Json> seeds;

	for (int seed : g_recorded_randomseeds) {
		seeds.push_back(json11::Json(seed));
	}

	return json11::Json(seeds);
}

static bool restoreRandomSeeds(const json11::Json &data) {
	if (!data.is_array()) return false;

	for (auto &val : data.array_items()) {
		g_queued_randomseeds.push_back(val.int_value());
	}

	return true;
}

// clear old rng data
ON_EVENT_P(SESSION_START, 999) {
	g_queued_view_punches.clear();
	g_recorded_view_punches.clear();
	g_queued_randomseeds.clear();
	g_recorded_randomseeds.clear();
}

// load pending rng data
ON_EVENT(SESSION_START) {
	if (!g_pending_load) return;

	json11::Json data = *g_pending_load;
	g_pending_load = std::optional<json11::Json>{};

	if (!engine->isRunning()) return;
	if (!sv_cheats.GetBool()) return;

	if (!data.is_object()) {
		console->Print("Invalid RNG data!\n");
		return;
	}

	if (data["map"].string_value() != engine->GetCurrentMapName()) {
		console->Print("Invalid map for RNG data!\n");
		return;
	}

	if (!restorePaintSprayers(data["paint"])) {
		console->Print("Failed to restore RNG paint sprayer data!\n");
	}

	if (!restoreViewPunches(data["view_punch"])) {
		console->Print("Failed to restore RNG view punch data!\n");
	}

	if (!restoreRandomSeeds(data["seeds"])) {
		console->Print("Failed to restore RNG random seed data!\n");
	}

	console->Print("RNG restore complete\n");
}

// save rng data (after loading)
ON_EVENT_P(SESSION_START, -999) {
	if (!engine->isRunning()) {
		g_session_state = std::optional<json11::Json>{};
		return;
	}

	g_session_state = json11::Json(json11::Json::object{
		{ "map", { engine->GetCurrentMapName() } },
		{ "paint", savePaintSprayers() },
	});
}

void RngManip::saveData(const char *filename) {
	if (!g_session_state) {
		console->Print("No RNG data to save!\n");
		return;
	}

	auto root = g_session_state->object_items();
	root["view_punch"] = saveViewPunches();
	root["seeds"] = saveRandomSeeds();

	auto filepath = fileSystem->FindFileSomewhere(filename).value_or(filename);
	FILE *f = fopen(filepath.c_str(), "w");
	if (!f) {
		console->Print("Failed to open file %s\n", filename);
		return;
	}

	fputs(json11::Json(root).dump().c_str(), f);
	fclose(f);

	console->Print("Wrote RNG data to %s\n", filename);
}

void RngManip::loadData(const char *filename) {
	auto filepath = fileSystem->FindFileSomewhere(filename).value_or(filename);
	std::ifstream st(filepath);
	if (!st.good()) {
		console->Print("Failed to open file %s\n", filename);
		return;
	}

	std::stringstream buf;
	buf << st.rdbuf();

	std::string err;
	auto json = json11::Json::parse(buf.str(), err);
	if (err != "") {
		console->Print("Failed to parse RNG file: %s\n", err.c_str());
		return;
	}

	g_pending_load = json;

	console->Print("Read RNG data from %s\n", filename);
}

void RngManip::viewPunch(QAngle *offset) {
	QAngle orig = *offset;
	if (g_queued_view_punches.size() > 0) {
		*offset = g_queued_view_punches.front();
		g_queued_view_punches.pop_front();
	}

	g_recorded_view_punches.push_back(*offset);
	playerTrace->EmitLog("ViewPunch(%.6f, %.6f, %.6f) -> (%.6f, %.6f, %.6f)", orig.x, orig.y, orig.z, offset->x, offset->y, offset->z);
}

void RngManip::randomSeed(int *seed) {
	int orig = *seed;
	if (g_queued_randomseeds.size() > 0) {
		*seed = g_queued_randomseeds.front();
		g_queued_randomseeds.pop_front();
	}

	g_recorded_randomseeds.push_back(*seed);
	playerTrace->EmitLog("RandomSeed(%d) -> %d", orig, *seed);
}

CON_COMMAND(sar_rng_save, "sar_rng_save [filename] - save RNG seed data to the specified file. If filename isn't given, use last TAS script path\n") {
	if (args.ArgC() < 1 || args.ArgC() > 2) {
		console->Print(sar_rng_save.ThisPtr()->m_pszHelpString);
		return;
	}

	std::string filename = "";
	if (args.ArgC() == 1) {
		if (tasPlayer->IsRunning()) {
			filename = tasPlayer->playbackInfo.GetMainScript().header.rngManipFile;
		} else if (tasPlayer->previousPlaybackInfo.HasActiveSlot()) {
			filename = tasPlayer->previousPlaybackInfo.GetMainScript().header.rngManipFile;
		} else {
			console->Print(sar_rng_save.ThisPtr()->m_pszHelpString);
			console->Print("No filename specified and no previous TAS script played\n");
			return;
		}
	} else {
		filename = std::string(args[1]);
	}

	size_t lastdot = filename.find_last_of(".");
	if (lastdot != std::string::npos) {
		filename = filename.substr(0, lastdot);
	}
	filename += "." RNG_MANIP_EXT;
	RngManip::saveData(filename.c_str());
}

CON_COMMAND(sar_rng_load, "sar_rng_load [filename] - load RNG seed data on next session start. If filename isn't given, use last TAS script path\n") {
	if (args.ArgC() < 1 || args.ArgC() > 2) {
		console->Print(sar_rng_load.ThisPtr()->m_pszHelpString);
		return;
	}

	std::string filename = "";
	if (args.ArgC() == 1) {
		if (tasPlayer->IsRunning()) {
			filename = tasPlayer->playbackInfo.GetMainScript().header.rngManipFile;
		} else if (tasPlayer->previousPlaybackInfo.HasActiveSlot()) {
			filename = tasPlayer->previousPlaybackInfo.GetMainScript().header.rngManipFile;
		} else {
			console->Print(sar_rng_load.ThisPtr()->m_pszHelpString);
			console->Print("No filename specified and no previous TAS script played\n");
			return;
		}
	} else {
		filename = std::string(args[1]);
	}
	size_t lastdot = filename.find_last_of(".");
	if (lastdot != std::string::npos) {
		filename = filename.substr(0, lastdot);
	}
	filename += "." RNG_MANIP_EXT;
	RngManip::loadData(filename.c_str());
}

extern Hook g_RandomSeed_Hook;
void (*RandomSeed)(int);
void RandomSeed_Hook(int seed) {
	RngManip::randomSeed(&seed);
	g_RandomSeed_Hook.Disable();
	RandomSeed(seed);
	g_RandomSeed_Hook.Enable();
}
Hook g_RandomSeed_Hook(RandomSeed_Hook);

static float *g_PhysicsHook_impactSoundTime;
static std::set<unsigned short> g_reset_sound_files;

struct SoundFile {
	unsigned short symbol;
	uint8_t gender;
	uint8_t available;
};

extern Hook g_EnsureAvailableSlotsForGender_Hook;
DECL_DETOUR_T(void, EnsureAvailableSlotsForGender, SoundFile *pSounds, int count, int gender) {
	for (int i = 0; i < count; ++i) {
		if (g_reset_sound_files.insert(pSounds[i].symbol).second) {
			playerTrace->EmitLog("Resetting sound file %d", pSounds[i].symbol);
			pSounds[i].available = 1;
		}
	}

	g_EnsureAvailableSlotsForGender_Hook.Disable();
	EnsureAvailableSlotsForGender(thisptr, pSounds, count, gender);
	g_EnsureAvailableSlotsForGender_Hook.Enable();
}
Hook g_EnsureAvailableSlotsForGender_Hook(EnsureAvailableSlotsForGender_Hook);

static void recordEntityOrgs() {
	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		ServerEnt *ent = (ServerEnt *)server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;
		auto classname = server->GetEntityClassName(ent);
		if (strcmp(classname, "prop_weighted_cube") && strcmp(classname, "player")) {
			continue;
		}

		playerTrace->EnterLogScope(Utils::ssprintf("entity %d", i).c_str());
		playerTrace->EmitLog(classname);
		auto org = ent->abs_origin();
		playerTrace->EmitLog("abs org:  (%.6f,%.6f,%.6f)", org.x, org.y, org.z);
		org = ent->collision().GetCollisionOrigin();
		playerTrace->EmitLog("coll org: (%.6f,%.6f,%.6f)", org.x, org.y, org.z);
		playerTrace->ExitLogScope();
	}
}

static void recordIvpEntityOrgs() {
	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		ServerEnt *ent = (ServerEnt *)server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;
		auto classname = server->GetEntityClassName(ent);
		if (strcmp(classname, "prop_weighted_cube") && strcmp(classname, "player")) {
			continue;
		}

		IVP_Real_Object *ivp_obj = ((CPhysicsObject *)ent->collision().GetVPhysicsObject())->real_obj;
		IVP_Core *core = ivp_obj->physical_core;
		auto speed = core->speed;
		auto pos = core->pos_world_f_core_last_psi;

		playerTrace->EnterLogScope(Utils::ssprintf("entity %d", i).c_str());
		playerTrace->EmitLog(classname);
		playerTrace->EmitLog("speed: (%.6f,%.6f,%.6f)", speed.x, speed.y, speed.z);
		playerTrace->EmitLog("pos:   (%.6f,%.6f,%.6f)", pos.x, pos.y, pos.z);
		playerTrace->ExitLogScope();
	}
}

static void recordIvpPlayerControllerShit(void *player_controller) {
#define GET(T, x, off) T x = *(T *)((char *)player_controller + (off))
	GET(IVP_U_Point, target_pos, 0x40);
	GET(IVP_U_Float_Point, ground_pos, 0x50);
	GET(IVP_U_Float_Point, max_speed, 0x60);
	GET(IVP_U_Float_Point, cur_speed, 0x70);
	GET(IVP_U_Float_Point, last_impulse, 0x80);
	GET(uint8_t, flags, 0x90);
#undef GET
#define EMIT_VEC(vec, name) playerTrace->EmitLog(name ": (%.6f,%.6f,%.6f)", vec.x, vec.y, vec.z)
#define EMIT_BOOL(val, name) playerTrace->EmitLog(name ": %s", (val) ? "true" : "false")
	EMIT_VEC(target_pos, "m_targetPosition");
	EMIT_VEC(ground_pos, "m_groundPosition");
	EMIT_VEC(max_speed, "m_maxSpeed");
	EMIT_VEC(cur_speed, "m_currentSpeed");
	EMIT_VEC(last_impulse, "m_lastImpulse");
	EMIT_BOOL(flags & 0x01, "m_enable");
	EMIT_BOOL(flags & 0x02, "m_onground");
	EMIT_BOOL(flags & 0x04, "m_forceTeleport");
	EMIT_BOOL(flags & 0x08, "m_updatedSinceLast");
#undef EMIT_VEC
#undef EMIT_BOOL
}

extern Hook g_FrameUpdatePostEntityThink_Hook;
DECL_DETOUR_T(void, FrameUpdatePostEntityThink) {
	playerTrace->EnterLogScope("HOOK CPhysicsHook::FrameUpdatePostEntityThink");

	recordEntityOrgs();

	playerTrace->EnterLogScope("CPhysicsHook::FrameUpdatePostEntityThink");
	g_FrameUpdatePostEntityThink_Hook.Disable();
	FrameUpdatePostEntityThink(thisptr);
	g_FrameUpdatePostEntityThink_Hook.Enable();
	playerTrace->ExitLogScope();

	recordEntityOrgs();

	playerTrace->ExitLogScope();
}
Hook g_FrameUpdatePostEntityThink_Hook(FrameUpdatePostEntityThink_Hook);

extern Hook g_simulate_psi_Hook;
DECL_DETOUR_T(void, simulate_psi, IVP_Time time) {
	playerTrace->EnterLogScope("IVP_Environment::simulate_psi");

	g_simulate_psi_Hook.Disable();
	simulate_psi(thisptr, time);
	g_simulate_psi_Hook.Enable();

	playerTrace->ExitLogScope();
}
Hook g_simulate_psi_Hook(simulate_psi_Hook);

extern Hook g_UpdateVPhysPos_Hook;
DECL_DETOUR_T(void, UpdateVPhysPos, const Vector &pos, const Vector &vel, float secs_to_arrival) {
	if (fabsf(vel.x) + fabsf(vel.y) + fabsf(vel.z) < 0.001) {
		playerTrace->EmitLog("CBasePlayer::UpdateVPhysicsPosition(vel=0)");
	}
	g_UpdateVPhysPos_Hook.Disable();
	UpdateVPhysPos(thisptr, pos, vel, secs_to_arrival);
	g_UpdateVPhysPos_Hook.Enable();
}
Hook g_UpdateVPhysPos_Hook(UpdateVPhysPos_Hook);

extern Hook g_do_sim_cont_Hook;
DECL_DETOUR_T(void, do_sim_cont, void *es, void *idc) {
	playerTrace->EnterLogScope("HOOK CPlayerController::do_simulation_controller");

	recordIvpEntityOrgs();
	recordIvpPlayerControllerShit(thisptr);

	playerTrace->EnterLogScope("CPlayerController::do_simulation_controller");

	g_do_sim_cont_Hook.Disable();
	do_sim_cont(thisptr, es, idc);
	g_do_sim_cont_Hook.Enable();

	playerTrace->ExitLogScope();

	recordIvpEntityOrgs();
	recordIvpPlayerControllerShit(thisptr);

	playerTrace->ExitLogScope();
}
Hook g_do_sim_cont_Hook(do_sim_cont_Hook);

extern Hook g_UpdatePusherPhysEOT_Hook;
DECL_DETOUR_T(void, UpdatePusherPhysEOT) {
	playerTrace->EnterLogScope("CPhysicsPushedEntities::UpdatePusherPhysicsEndOfTick");

	g_UpdatePusherPhysEOT_Hook.Disable();
	UpdatePusherPhysEOT(thisptr);
	g_UpdatePusherPhysEOT_Hook.Enable();

	playerTrace->ExitLogScope();
}
Hook g_UpdatePusherPhysEOT_Hook(UpdatePusherPhysEOT_Hook);

extern Hook g_FinishPush_Hook;
DECL_DETOUR_T(void, FinishPush, bool is_rot_push, void *rot_push_move) {
	playerTrace->EnterLogScope("CPhysicsPushedEntities::FinishPush");

	g_FinishPush_Hook.Disable();
	FinishPush(thisptr, is_rot_push, rot_push_move);
	g_FinishPush_Hook.Enable();

	playerTrace->ExitLogScope();
}
Hook g_FinishPush_Hook(FinishPush_Hook);

extern Hook g_PhysicsStep_Hook;
DECL_DETOUR_T(void, PhysicsStep) {
	playerTrace->EnterLogScope("CBaseEntity::PhysicsStep");

	g_PhysicsStep_Hook.Disable();
	PhysicsStep(thisptr);
	g_PhysicsStep_Hook.Enable();

	playerTrace->ExitLogScope();
}
Hook g_PhysicsStep_Hook(PhysicsStep_Hook);

extern Hook g_PhysRelinkChildren_Hook;
DECL_DETOUR_T(void, PhysRelinkChildren, float dt) {
	playerTrace->EnterLogScope("CBaseEntity::PhysicsRelinkChildren");

	g_PhysRelinkChildren_Hook.Disable();
	PhysRelinkChildren(thisptr, dt);
	g_PhysRelinkChildren_Hook.Enable();

	playerTrace->ExitLogScope();
}
Hook g_PhysRelinkChildren_Hook(PhysRelinkChildren_Hook);

extern Hook g_VPSU_Hook;
DECL_DETOUR_T(void, VPSU, IPhysicsObject *physics) {
	playerTrace->EnterLogScope("CBasePlayer::VPhysicsShadowUpdate");

	g_VPSU_Hook.Disable();
	VPSU(thisptr, physics);
	g_VPSU_Hook.Enable();

	playerTrace->ExitLogScope();
}
Hook g_VPSU_Hook(VPSU_Hook);

extern Hook g_VPhysUpdate_Hook;
DECL_DETOUR_T(void, VPhysUpdate, IPhysicsObject *physics) {
	playerTrace->EnterLogScope("CBaseEntity::VPhysicsUpdate");

	g_VPhysUpdate_Hook.Disable();
	VPhysUpdate(thisptr, physics);
	g_VPhysUpdate_Hook.Enable();

	playerTrace->ExitLogScope();
}
Hook g_VPhysUpdate_Hook(VPhysUpdate_Hook);

extern Hook g_SetParent_Hook;
DECL_DETOUR_T(void, SetParent, const char *newParent, ServerEnt *activator, int attachment) {
	playerTrace->EnterLogScope("CBaseEntity::SetParent");

	g_SetParent_Hook.Disable();
	SetParent(thisptr, newParent, activator, attachment);
	g_SetParent_Hook.Enable();

	playerTrace->ExitLogScope();
}
Hook g_SetParent_Hook(SetParent_Hook);

extern Hook g_PhysicsSimulate_Hook;
DECL_DETOUR_T(void, PhysicsSimulate) {
	playerTrace->EnterLogScope("CBasePlayer::PhysicsSimulate");

	g_PhysicsSimulate_Hook.Disable();
	PhysicsSimulate(thisptr);
	g_PhysicsSimulate_Hook.Enable();

	playerTrace->ExitLogScope();
}
Hook g_PhysicsSimulate_Hook(PhysicsSimulate_Hook);

extern Hook g_PostThinkVPhys_Hook;
DECL_DETOUR_T(void, PostThinkVPhys) {
	playerTrace->EnterLogScope("CBasePlayer::PostThinkVPhysics");

	auto *vel = &SE(thisptr)->field<Vector>("m_vNewVPhysicsVelocity");

	playerTrace->EmitLog("PRE  m_vNewVPhysicsVelocity=(%.6f,%.6f,%.6f)", vel->x, vel->y, vel->z);
	playerTrace->EmitLog("PRE  m_touchedPhysObject=%s", SE(thisptr)->fieldOff<bool>("m_flNextDecalTime", -4) ? "true" : "false");

	g_PostThinkVPhys_Hook.Disable();
	PostThinkVPhys(thisptr);
	g_PostThinkVPhys_Hook.Enable();

	playerTrace->EmitLog("POST m_vNewVPhysicsVelocity=(%.6f,%.6f,%.6f)", vel->x, vel->y, vel->z);
	playerTrace->EmitLog("POST m_touchedPhysObject=%s", SE(thisptr)->fieldOff<bool>("m_flNextDecalTime", -4) ? "true" : "false");

	playerTrace->ExitLogScope();
}
Hook g_PostThinkVPhys_Hook(PostThinkVPhys_Hook);

static void *g_gamemovement;
void RngManip::EnterProcessMovement(void *gamemovement, CMoveData *move) {
	g_gamemovement = gamemovement;
	playerTrace->EnterLogScope("CPortalGameMovement::ProcessMovement");
	auto v = move->m_outWishVel;
	playerTrace->EmitLog("PRE  m_outWishVel=(%.6f,%.6f,%.6f)", v.x, v.y, v.z);
}
void RngManip::ExitProcessMovement(CMoveData *move) {
	auto v = move->m_outWishVel;
	playerTrace->EmitLog("POST m_outWishVel=(%.6f,%.6f,%.6f)", v.x, v.y, v.z);
	playerTrace->ExitLogScope();
}

ON_EVENT(SESSION_START) {
	// Reset this between sessions so the stuck check can't depend on previous sessions
	if (!g_gamemovement) return;
	float *m_flStuckCheckTime = (float *)(((char *)g_gamemovement) + 36 + 33*3*16 + 8);
	memset(m_flStuckCheckTime, 0, 34 * 2 * sizeof (float));
}

extern Hook g_Friction_Hook;
DECL_DETOUR_T(void, Friction) {
	playerTrace->EnterLogScope("CPortalGameMovement::Friction");

	CMoveData *mv = *(CMoveData **)((char *)thisptr + 8);
	playerTrace->EmitLog("PRE  m_outWishVel=(%.6f,%.6f,%.6f)", mv->m_outWishVel.x, mv->m_outWishVel.y, mv->m_outWishVel.z);

	g_Friction_Hook.Disable();
	Friction(thisptr);
	g_Friction_Hook.Enable();

	playerTrace->EmitLog("PRE  m_outWishVel=(%.6f,%.6f,%.6f)", mv->m_outWishVel.x, mv->m_outWishVel.y, mv->m_outWishVel.z);

	playerTrace->ExitLogScope();
}
Hook g_Friction_Hook(Friction_Hook);

extern Hook g_WalkMove_Hook;
DECL_DETOUR_T(void, WalkMove) {
	playerTrace->EnterLogScope("CPortalGameMovement::WalkMove");

	CMoveData *mv = *(CMoveData **)((char *)thisptr + 8);
	playerTrace->EmitLog("PRE  m_outWishVel=(%.6f,%.6f,%.6f)", mv->m_outWishVel.x, mv->m_outWishVel.y, mv->m_outWishVel.z);

	g_WalkMove_Hook.Disable();
	WalkMove(thisptr);
	g_WalkMove_Hook.Enable();

	playerTrace->EmitLog("PRE  m_outWishVel=(%.6f,%.6f,%.6f)", mv->m_outWishVel.x, mv->m_outWishVel.y, mv->m_outWishVel.z);

	playerTrace->ExitLogScope();
}
Hook g_WalkMove_Hook(WalkMove_Hook);

extern Hook g_AirAccelerate_Hook;
DECL_DETOUR_T(void, AirAccelerate, Vector *wishdir, float wishspeed, float accel) {
	playerTrace->EnterLogScope("CPortalGameMovement::AirAccelerate");

	playerTrace->EmitLog("wishdir=(%.6f,%.6f,%.6f)", wishdir->x, wishdir->y, wishdir->z);
	playerTrace->EmitLog("wishspeed=%.6f", wishspeed);
	playerTrace->EmitLog("accel=%.6f", accel);

	CMoveData *mv = *(CMoveData **)((char *)thisptr + 8);
	playerTrace->EmitLog("PRE  m_outWishVel=(%.6f,%.6f,%.6f)", mv->m_outWishVel.x, mv->m_outWishVel.y, mv->m_outWishVel.z);

	g_AirAccelerate_Hook.Disable();
	AirAccelerate(thisptr, wishdir, wishspeed, accel);
	g_AirAccelerate_Hook.Enable();

	playerTrace->EmitLog("PRE  m_outWishVel=(%.6f,%.6f,%.6f)", mv->m_outWishVel.x, mv->m_outWishVel.y, mv->m_outWishVel.z);

	playerTrace->ExitLogScope();
}
Hook g_AirAccelerate_Hook(AirAccelerate_Hook);


ON_INIT {
	RandomSeed = Memory::GetSymbolAddress<decltype(RandomSeed)>(Memory::GetModuleHandleByName(tier1->Name()), "RandomSeed");
	g_RandomSeed_Hook.SetFunc(RandomSeed);
#ifdef _WIN32
	uintptr_t PhysFrame = Memory::Scan(server->Name(), "55 8B EC 8B 0D ? ? ? ? 83 EC 14 53 56 57 85 C9 0F 84 ? ? ? ? 80 3D ? ? ? ? 00 0F 85 ? ? ? ? F3 0F 10 4D 08 0F 2F 0D ? ? ? ? F3 0F 10 15 ? ? ? ? 0F 57 C0");
	uintptr_t m_bPaused = *(uint32_t *)(PhysFrame + 25);
	g_PhysicsHook_impactSoundTime = (float *)(m_bPaused - 4);

	EnsureAvailableSlotsForGender = (decltype(EnsureAvailableSlotsForGender))Memory::Scan(MODULE("soundemittersystem"), "55 8B EC 8B 4D 0C 33 C0 83 EC 20 3B C8 0F 8E ? ? ? ? 53 56 33 DB 33 F6 89 5D E0 89 45 E4 89 45 E8 89 75 EC 89 45 F0");
	g_EnsureAvailableSlotsForGender_Hook.SetFunc(EnsureAvailableSlotsForGender);
#else
	// TODO: mod support
	uintptr_t PhysFrame = Memory::Scan(server->Name(), "A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? 80 3D ? ? ? ? 00 0F 85 ? ? ? ? 55 89 E5 57 56 53 83 EC 3C 0F 2F 05 ? ? ? ?");
	uintptr_t m_bPaused = *(uint32_t *)(PhysFrame + 15);
	g_PhysicsHook_impactSoundTime = (float *)(m_bPaused - 4);

	EnsureAvailableSlotsForGender = (decltype(EnsureAvailableSlotsForGender))Memory::Scan(MODULE("soundemittersystem"), "55 57 56 53 83 EC 2C 8B 74 24 48 8B 5C 24 44 8B 7C 24 4C 85 F6 0F 8E ? ? ? ? C7 44 24 0C 00 00 00 00 31 D2 31 C9 31 C0");
	g_EnsureAvailableSlotsForGender_Hook.SetFunc(EnsureAvailableSlotsForGender);

	FrameUpdatePostEntityThink = (decltype(FrameUpdatePostEntityThink))Memory::Scan(server->Name(), "55 89 E5 57 56 53 83 EC 0C 8B 1D ? ? ? ? 8B 75 08 85 DB 0F 85 ? ? ? ? A1 ? ? ? ? 66 0F EF C0 66 0F 7E C7 F3 0F 10 48 10 0F 2F C8");
	g_FrameUpdatePostEntityThink_Hook.SetFunc(FrameUpdatePostEntityThink);

	simulate_psi = (decltype(simulate_psi))Memory::Scan(MODULE("vphysics"), "55 89 E5 57 56 53 81 EC 24 04 00 00 8B 5D 08 8B 43 28 8B 10 6A 01 50 FF 52 04 83 C4 10 66 83 BB DA 00 00 00 00");
	g_simulate_psi_Hook.SetFunc(simulate_psi);

	do_sim_cont = (decltype(do_sim_cont))Memory::Scan(MODULE("vphysics"), "55 57 56 53 81 EC EC 00 00 00 8B B4 24 ? ? ? ?");
	g_do_sim_cont_Hook.SetFunc(do_sim_cont);

	UpdateVPhysPos = (decltype(UpdateVPhysPos))Memory::Scan(server->Name(), "8B 44 24 ? 8B 54 24 ? 8B 4C 24 ? F3 0F 10 44 24 ? 83 B8 ? ? ? ? 00");
	g_UpdateVPhysPos_Hook.SetFunc(UpdateVPhysPos);

	UpdatePusherPhysEOT = (decltype(UpdatePusherPhysEOT))Memory::Scan(server->Name(), "57 56 53 8B 5C 24 ? 8B 73 ? 85 F6 0F 8E ? ? ? ?");
	g_UpdatePusherPhysEOT_Hook.SetFunc(UpdatePusherPhysEOT);

	FinishPush = (decltype(FinishPush))Memory::Scan(server->Name(), "55 57 56 53 83 EC 1C 8B 6C 24 ? 0F B6 44 24 ?");
	g_FinishPush_Hook.SetFunc(FinishPush);

	PhysicsStep = (decltype(PhysicsStep))Memory::Scan(server->Name(), "55 57 56 53 81 EC 8C 00 00 00 8B 9C 24 ? ? ? ? 80 7B ? 00");
	g_PhysicsStep_Hook.SetFunc(PhysicsStep);

	PhysRelinkChildren = (decltype(PhysRelinkChildren))Memory::Scan(server->Name(), "55 57 56 53 83 EC 0C 8B 0D ? ? ? ? 8B 44 24 ? 8B 80 ? ? ? ? 83 F8 FF 0F 84 ? ? ? ? 0F B7 D0 C1 E8 10");
	g_PhysRelinkChildren_Hook.SetFunc(PhysRelinkChildren);

	VPSU = (decltype(VPSU))Memory::Scan(server->Name(), "55 89 E5 57 56 53 81 EC 9C 01 00 00 A1 ? ? ? ?");
	g_VPSU_Hook.SetFunc(VPSU);

	VPhysUpdate = (decltype(VPhysUpdate))Memory::Scan(server->Name(), "55 57 56 53 83 EC 4C 8B 6C 24 ? 8B 5C 24 ?");
	g_VPhysUpdate_Hook.SetFunc(VPhysUpdate);

	SetParent = (decltype(SetParent))Memory::Scan(server->Name(), "55 57 56 53 83 EC 1C 8B 44 24 ? 8B 6C 24 ? 8B 74 24 ? 8B 4C 24 ?");
	g_SetParent_Hook.SetFunc(SetParent);

	PhysicsSimulate = (decltype(PhysicsSimulate))Memory::Scan(server->Name(), "55 89 E5 57 56 53 83 EC 4C A1 ? ? ? ? 8B 7D ? 89 45 B0");
	g_PhysicsSimulate_Hook.SetFunc(PhysicsSimulate);

	PostThinkVPhys = (decltype(PostThinkVPhys))Memory::Scan(server->Name(), "55 57 56 53 81 EC 9C 00 00 00 8B 9C 24 ? ? ? ? 8B 83 ? ? ? ?");
	g_PostThinkVPhys_Hook.SetFunc(PostThinkVPhys);

	Friction = (decltype(Friction))Memory::Scan(server->Name(), "55 66 0F EF F6 57 56 53 81 EC FC 00 00 00");
	g_Friction_Hook.SetFunc(Friction);

	WalkMove = (decltype(WalkMove))Memory::Scan(server->Name(), "55 57 56 53 81 EC 2C 01 00 00 8D 44 24 ?");
	g_WalkMove_Hook.SetFunc(WalkMove);

	AirAccelerate = (decltype(AirAccelerate))Memory::Scan(server->Name(), "56 53 83 EC 14 8B 5C 24 ? 8B 74 24 ? 8B 43 ? 80 B8 ? ? ? ? 00");
	g_AirAccelerate_Hook.SetFunc(AirAccelerate);
#endif
}

ON_EVENT(SESSION_END) {
	engine->ExecuteCommand("phys_timescale 1", true);
	//console->Print("physics rng state reset\n");
	*g_PhysicsHook_impactSoundTime = 0.0f;
	//console->Print("impact sound time reset\n");
	g_reset_sound_files.clear();
}
