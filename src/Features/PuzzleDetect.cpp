#include "Event.hpp"
#include "Offsets.hpp"
#include "Modules/Server.hpp"
#include "Features/Session.hpp"
#include "Features/EntityList.hpp"
#include "Features/OverlayRender.hpp"

#include <vector>
#include <map>

enum class EntityType {
	LASER_CATCHER,
	LASER_RELAY,
	FLOOR_BUTTON,
	PEDESTAL_BUTTON,
	LASER_EMITTER,
	FUNNEL,
	LIGHT_BRIDGE,
	FIZZLER,
	DOOR,
	FAITH_PLATE,
	CUBE,
	TURRET,
	PORTAL,
};

struct ModelEntity {
	EntityType type;
	Vector pos;
	Vector facing;
	bool activated;
	Vector mins;
	Vector maxs;

	CBaseHandle handle;
};

static std::vector<ModelEntity> g_static_ents;
static std::vector<ModelEntity> g_dynamic_ents;
static std::map<int, int> g_pedestal_button_disable_ticks;

static bool checkEntityClass(uintptr_t ent, const char *expect) {
	const char *actual = server->GetEntityClassName((void *)ent);
	return actual && !strcmp(expect, actual);
}

static bool checkPedestalButton(uintptr_t ent, int ent_idx) {
	int tick = session->GetTick();

	// First, check if the entity is currently doing stuff
	float goal = *(float *)(ent + Offsets::m_flGoalTime);
	if (goal > server->gpGlobals->curtime) {
		// The button is currently partway through being pressed
		g_pedestal_button_disable_ticks[ent_idx] = tick + 6; // Report all buttons as being pressed for at least 0.1s
		return true;
	}

	auto res = g_pedestal_button_disable_ticks.find(ent_idx);
	if (res == g_pedestal_button_disable_ticks.end()) return false;

	if (res->second < tick) return true;

	g_pedestal_button_disable_ticks.erase(ent_idx);
	return false;
}

static void UpdateActivation(ModelEntity &model_ent) {
	IHandleEntity *handle_ent = entityList->LookupEntity(model_ent.handle);
	if (!handle_ent) {
		// entity was removed?
		model_ent.activated = false;
		return;
	}

	uintptr_t ent = (uintptr_t)handle_ent;

	switch (model_ent.type) {
	case EntityType::LASER_CATCHER:
		model_ent.activated =
			checkEntityClass(ent, "prop_laser_catcher") &&
			*(int *)(ent + Offsets::m_iPowerState) > 1;
		break;
	case EntityType::LASER_RELAY:
		model_ent.activated =
			checkEntityClass(ent, "prop_laser_relay") &&
			*(int *)(ent + Offsets::m_iPowerState) > 1;
		break;
	case EntityType::FLOOR_BUTTON:
		model_ent.activated =
			checkEntityClass(ent, "prop_floor_button") &&
			*(bool *)(ent + Offsets::m_bButtonState);
		break;
	case EntityType::PEDESTAL_BUTTON:
		model_ent.activated =
			checkEntityClass(ent, "prop_button") &&
			checkPedestalButton(ent, model_ent.handle.GetEntryIndex());
		break;
	case EntityType::LASER_EMITTER:
		model_ent.activated =
			checkEntityClass(ent, "env_portal_laser") &&
			*(bool *)(ent + Offsets::m_bLaserOn);
		break;
	case EntityType::FUNNEL:
		model_ent.activated =
			checkEntityClass(ent, "prop_tractor_beam") &&
			*(bool *)(ent + Offsets::m_bEnabled);
		break;
	case EntityType::LIGHT_BRIDGE:
		model_ent.activated =
			checkEntityClass(ent, "prop_wall_projector") &&
			*(bool *)(ent + Offsets::m_bEnabled);
		break;
	case EntityType::FIZZLER:
		model_ent.activated =
			checkEntityClass(ent, "trigger_portal_cleanser") &&
			!*(bool *)(ent + Offsets::m_bDisabled);
		break;
	case EntityType::DOOR:
		model_ent.activated =
			checkEntityClass(ent, "prop_testchamber_door") &&
			*(bool *)(ent + Offsets::m_bIsOpen);
		break;
	case EntityType::FAITH_PLATE:
		if (checkEntityClass(ent, "trigger_catapult")) {
			float *delay = (float *)(ent + Offsets::m_flRefireDelay);
			float curtime = server->gpGlobals->curtime;
			// 0 = entities, 1/2 = player slots
			model_ent.activated = (delay[0] > curtime || delay[1] > curtime || delay[2] > curtime);
		} else {
			model_ent.activated = false;
		}
		break;
	case EntityType::CUBE:
		if (checkEntityClass(ent, "prop_weighted_cube")) {
			void *reflected = entityList->LookupEntity(*(CBaseHandle *)(ent + Offsets::m_hLaser));
			bool reflect = reflected && *(bool *)((uintptr_t)reflected + Offsets::m_bLaserOn);
			model_ent.activated = reflect || *(bool *)(ent + Offsets::Cube_m_bActivated);
		} else {
			model_ent.activated = false;
		}
		break;
	case EntityType::TURRET:
		model_ent.activated =
			checkEntityClass(ent, "npc_portal_turret_floor") &&
			*(bool *)(ent + Offsets::m_bIsFiring);
		break;
	case EntityType::PORTAL:
		if (checkEntityClass(ent, "prop_portal")) {
			void *linked = entityList->LookupEntity(*(CBaseHandle *)(ent + Offsets::m_hLinkedPortal));
			model_ent.activated = linked && *(bool *)(linked + Offsets::m_bActivated);
		} else {
			model_ent.activated = false;
		}
		break;
	}
}

static void UpdatePosition(ModelEntity &model_ent) {
	void *ent = entityList->LookupEntity(model_ent.handle);
	ICollideable *coll = (ICollideable *)((uintptr_t)ent + Offsets::S_m_Collision);

	Vector mins, maxs;
	coll->WorldSpaceSurroundingBounds(&mins, &maxs);

	Vector pos = server->GetAbsOrigin(ent);
	if (model_ent.type == EntityType::FAITH_PLATE || model_ent.type == EntityType::FIZZLER) {
		// Brush entities! Use the center of their absbox
		pos = (mins + maxs) / 2;
	}

	Vector forward, right, up;
	Math::AngleVectors(server->GetAbsAngles(ent), &forward, &right, &up);

	Vector facing;
	switch (model_ent.type) {
	case EntityType::LASER_CATCHER:
	case EntityType::LASER_EMITTER:
	case EntityType::FUNNEL:
	case EntityType::LIGHT_BRIDGE:
	case EntityType::DOOR:
	case EntityType::CUBE:
	case EntityType::TURRET:
	case EntityType::PORTAL:
		// facing forward
		facing = forward;
		break;

	case EntityType::FLOOR_BUTTON:
	case EntityType::PEDESTAL_BUTTON:
	case EntityType::LASER_RELAY:
		// facing up
		facing = up;
		break;

	case EntityType::FIZZLER:
		// facing the largest wall of the thing i guess?
		{
			Vector size = maxs - mins;
			if (size.x < size.y && size.x < size.z) {
				facing = {1, 0, 0};
			} else if (size.y < size.x && size.y < size.z) {
				facing = {0, 1, 0};
			} else {
				facing = {0, 0, 1};
			}
		}
		break;

	case EntityType::FAITH_PLATE:
		Math::AngleVectors(*(QAngle *)((uintptr_t)ent + Offsets::m_vecLaunchAngles), &facing);
		// TODO: handle entities with a target!
		if (facing.SquaredLength() < 0.1f) facing = Vector{0, 0, 1};
		break;
	}

	model_ent.pos = pos;
	model_ent.facing = facing;
	model_ent.mins = mins - pos;
	model_ent.maxs = maxs - pos;
}

static void FindEnts(bool dynamic) {
	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;

		const char *classname = server->GetEntityClassName(ent);

		std::optional<EntityType> type = {};

#define MATCH(name, enttype) if (!strcmp(classname, name)) type = EntityType::enttype;
		if (dynamic) {
			MATCH("prop_weighted_cube",      CUBE)
			MATCH("npc_portal_turret_floor", TURRET)
			MATCH("prop_portal",             PORTAL)
		} else {
			MATCH("prop_laser_catcher",      LASER_CATCHER)
			MATCH("prop_laser_relay",        LASER_RELAY)
			MATCH("prop_floor_button",       FLOOR_BUTTON)
			MATCH("prop_button",             PEDESTAL_BUTTON)
			MATCH("env_portal_laser",        LASER_EMITTER)
			MATCH("prop_tractor_beam",       FUNNEL)
			MATCH("prop_wall_projector",     LIGHT_BRIDGE)
			MATCH("trigger_portal_cleanser", FIZZLER)
			MATCH("prop_testchamber_door",   DOOR)
			MATCH("trigger_catapult",        FAITH_PLATE)
		}
#undef MATCH

		if (!type) continue;

		if (*type == EntityType::FAITH_PLATE) {
			// Ignore scripted momentum
			bool scripted = *(bool *)((uintptr_t)ent + Offsets::m_bUseThresholdCheck);
			if (scripted) {
				// Allow ones that trigger for just walking I guess?
				float min = *(float *)((uintptr_t)ent + Offsets::m_flLowerThreshold);
				float max = *(float *)((uintptr_t)ent + Offsets::m_flLowerThreshold);
				if (min > 1.0f || max < 200.0f) continue;
			}
		}

		if (*type == EntityType::PORTAL) {
			// Ignore non-activated portals
			if (!*(bool *)((uintptr_t)ent + Offsets::m_bActivated)) continue;
		}

		ModelEntity e{
			*type, {0,0,0}, {0,0,0},
			false, {0,0,0}, {0,0,0}, 
			((IHandleEntity *)ent)->GetRefEHandle(),
		};

		if (dynamic) {
			g_dynamic_ents.push_back(e);
			UpdatePosition(g_dynamic_ents[g_dynamic_ents.size() - 1]);
			UpdateActivation(g_dynamic_ents[g_dynamic_ents.size() - 1]);
		} else {
			g_static_ents.push_back(e);
			UpdatePosition(g_static_ents[g_static_ents.size() - 1]);
			UpdateActivation(g_static_ents[g_static_ents.size() - 1]);
		}
	}
}

ON_EVENT(SESSION_END) {
	g_static_ents.clear();
	g_dynamic_ents.clear();
	g_pedestal_button_disable_ticks.clear();
}

ON_EVENT(SESSION_START) {
	g_static_ents.clear();
	FindEnts(false);
}

ON_EVENT(POST_TICK) {
	for (auto &ent : g_static_ents) {
		UpdateActivation(ent);
	}

	g_dynamic_ents.clear();
	FindEnts(true);
}

ON_EVENT(RENDER) {
	for (auto &ent : g_static_ents) {
		OverlayRender::addBox(ent.pos, ent.mins, ent.maxs, {0,0,0}, ent.activated ? Color{0,255,0,100} : Color{255,0,0,100});
		OverlayRender::addLine(ent.pos, ent.pos + ent.facing*100.0, {100,100,255,255});
	}

	for (auto &ent : g_dynamic_ents) {
		OverlayRender::addBox(ent.pos, ent.mins, ent.maxs, {0,0,0}, ent.activated ? Color{0,255,0,100} : Color{255,0,0,100});
		OverlayRender::addLine(ent.pos, ent.pos + ent.facing*100.0, {100,100,255,255});
	}
}
