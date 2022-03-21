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
};

struct ModelEntity {
	EntityType type;
	Vector pos;
	Vector facing;
	bool activated;

	CBaseHandle handle;
};

static std::vector<ModelEntity> g_ents;
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

void UpdateEntity(ModelEntity &model_ent) {
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
	}
}

ON_EVENT(SESSION_END) {
	g_ents.clear();
	g_pedestal_button_disable_ticks.clear();
}

ON_EVENT(SESSION_START) {
	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;

		const char *classname = server->GetEntityClassName(ent);

		std::optional<EntityType> type = {};

#define MATCH(name, enttype) if (!strcmp(classname, name)) type = EntityType::enttype;
		MATCH("prop_laser_catcher",      LASER_CATCHER)
		MATCH("prop_laser_relay",        LASER_RELAY)
		MATCH("prop_floor_button",       FLOOR_BUTTON)
		MATCH("prop_button",             PEDESTAL_BUTTON)
		MATCH("env_portal_laser",        LASER_EMITTER)
		MATCH("prop_tractor_beam",       FUNNEL)
		MATCH("prop_wall_projector",     LIGHT_BRIDGE)
		MATCH("trigger_portal_cleanser", FIZZLER)
		MATCH("prop_testchamber_door",   DOOR)
#undef MATCH

		if (!type) continue;

		// FIXME: this isn't necessarily correct for fizzlers! They're brush
		// entities so might have their origin anywhere really. Luckily they
		// mostly seem to have their origin set where you'd expect
		Vector pos = server->GetAbsOrigin(ent);

		Vector forward, right, up;
		Math::AngleVectors(server->GetAbsAngles(ent), &forward, &right, &up);

		Vector facing;
		switch (*type) {
		case EntityType::LASER_CATCHER:
		case EntityType::LASER_EMITTER:
		case EntityType::FUNNEL:
		case EntityType::LIGHT_BRIDGE:
		case EntityType::DOOR:
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
			// TODO
			facing = {0, 0, 1};
			break;
		}

		g_ents.push_back({
			*type,
			pos,
			facing,
			false,
			((IHandleEntity *)ent)->GetRefEHandle(),
		});

		UpdateEntity(g_ents[g_ents.size() - 1]);
	}
}

ON_EVENT(POST_TICK) {
	for (auto &ent : g_ents) {
		UpdateEntity(ent);
	}
}

ON_EVENT(RENDER) {
	for (auto &ent : g_ents) {
		OverlayRender::addBox(ent.pos, {-20,-20,-20}, {20,20,20}, {0,0,0}, ent.activated ? Color{0,255,0,100} : Color{255,0,0,100});
		OverlayRender::addLine(ent.pos, ent.pos + ent.facing*50.0, {0,0,255,255});
	}
}
