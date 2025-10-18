#pragma once

#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"

struct SavestateInfo {
	// All the player shit
	int slot;
	int tick;

	// The main guys
	Vector position;
	QAngle angles;
	Vector abs_velocity;
	Vector vec_velocity;
	Vector view_offset;
	float gravity;
	std::optional<CUserCmd> usrcmd;

	// Various movement stuff
	float surfaceFriction;
	float maxSpeed;
	int waterLevel;
	CBaseHandle ground_entity;
	bool onSpeedPaint;
	int oldButtons;
	char moveType;
	int flags;
	int e_flags;

	// ducking stuff
	bool ducked;
	bool ducking;
	bool in_duck_jump;
	int duck_time_msecs;
	int duckjump_time_msecs;

	// Other
	int player_condition;
	char takedamage; // betsrigher

	// Tas tools state
	int tasTick;
	std::list<std::shared_ptr<TasTool>> tools;

	// Trigger event queue
	// std::vector<EventQueuePrioritizedEvent_t> event_queue;

	// TODO: 
	// - event queue state (game/server/eventqueue.h)
	// - duckedInAir? (hud.cpp:830)
	// - funnels (hud.cpp, tbeam stuff)
	// - held object (m_hAttachedObject)
	
	// Investigate these: they have known issues
	// - blue gel bounce is not consistent (you don't always get the same height depending on which savestate is loaded)
	// - speed gel doesn't behave the same on replay than on prediction
};

class SavestateManager {
private:
	std::vector<SavestateInfo> savestates[2];

public:
	static SavestateInfo GetSavestate(int slot, CUserCmd* cmd = 0) {
		ServerEnt *pl = (ServerEnt *)server->GetPlayer(slot + 1);

		SavestateInfo pi {
			.slot = slot,
			.tick = pl->template field<int>("m_nTickBase"),

			.position = server->RealGetAbsOrigin(pl),
			.angles = engine->GetAngles(slot),
			.abs_velocity = pl->abs_velocity(),
			.vec_velocity = pl->local_velocity(),
			.view_offset = pl->view_offset(),
			.gravity = pl->gravity(),
			.usrcmd = cmd ? std::optional<CUserCmd>(*cmd) : std::nullopt,
			
			.surfaceFriction = *reinterpret_cast<float *>((uintptr_t)pl + Offsets::S_m_surfaceFriction),
			.maxSpeed = pl->template field<float>("m_flMaxspeed"),
			.waterLevel = pl->template field<char>("m_nWaterLevel"),
			.ground_entity = pl->ground_entity(),
			.oldButtons = pl->template field<int>("m_nOldButtons"),
			.moveType = pl->template field<char>("m_MoveType"),
			.flags = pl->template field<int>("m_fFlags"),
			.e_flags = pl->eflags(),
			
			.ducked = pl->ducked(),
			.ducking = pl->template field<bool>("m_bDucking"),
			.in_duck_jump = pl->template field<bool>("m_bInDuckJump"),
			.duck_time_msecs = pl->template field<int>("m_nDuckTimeMsecs"),
			.duckjump_time_msecs = pl->template field<int>("m_nDuckJumpTimeMsecs"),

			.player_condition = pl->template field<int>("m_nPlayerCond"),
			.takedamage = pl->field<char>("m_takedamage"),
		};

		// Store tools
		for (TasTool *tool : TasTool::GetList(pi.slot)) {
			pi.tools.push_back(tool->makeCopy());
		}

		return pi;
	};
	static void RestoreSavestate(int slot, SavestateInfo savestate, CUserCmd* cmd = 0) {
		ServerEnt *pl = (ServerEnt *)server->GetPlayer(slot + 1);

		pl->template field<int>("m_nTickBase") = savestate.tick;
		
		server->SetAbsOrigin(pl, &savestate.position);
		engine->SetAngles(savestate.slot, savestate.angles);
		pl->abs_velocity() = savestate.abs_velocity;
		pl->local_velocity() = savestate.vec_velocity;
		pl->view_offset() = savestate.view_offset;
		pl->gravity() = savestate.gravity;
		if (savestate.usrcmd.has_value() && cmd) {
			*cmd = savestate.usrcmd.value();
		}

		*reinterpret_cast<float *>((uintptr_t)pl + Offsets::S_m_surfaceFriction) = savestate.surfaceFriction;
		pl->template field<float>("m_flMaxspeed") = savestate.maxSpeed;
		pl->template field<char>("m_nWaterLevel") = savestate.waterLevel;
		pl->ground_entity() = savestate.ground_entity;
		pl->template field<int>("m_nOldButtons") = savestate.oldButtons;
		pl->template field<char>("m_MoveType") = savestate.moveType;
		pl->template field<int>("m_fFlags") = savestate.flags;
		pl->eflags() = savestate.e_flags;

		pl->ducked() = savestate.ducked;
		pl->template field<bool>("m_bDucking") = savestate.ducking;
		pl->template field<bool>("m_bInDuckJump") = savestate.in_duck_jump;
		pl->template field<int>("m_nDuckTimeMsecs") = savestate.duck_time_msecs;
		pl->template field<int>("m_nDuckJumpTimeMsecs") = savestate.duckjump_time_msecs;

		pl->template field<int>("m_nPlayerCond") = savestate.player_condition;
		pl->field<char>("m_takedamage") = savestate.takedamage;

		for (auto tool : savestate.tools) {
			TasTool::GetInstanceByName(savestate.slot, std::string(tool->GetName()))->fromCopy(tool);
		}
	};

	// Make a savestate for the given player slot at the current tick
	// Invalidates and deletes any savestates made after the current tick.
    void MakeSavestate(int slot, int tasTick, CUserCmd* cmd = 0) {
		auto new_savestate = GetSavestate(slot, cmd);
		new_savestate.tasTick = tasTick;
		int insert_idx = 0;
		// Find the right savestate
		// TODO: dichotomy or start at end (usage patterns in the tas tools will mean we will want the last ones more often)
		for(auto savestate: savestates[slot]) {
			if (savestate.tasTick >= tasTick) {
				break;
			}
			insert_idx++;
		}
		savestates[slot].resize(insert_idx + 1);
		savestates[slot][insert_idx] = new_savestate;
	};
	
	// Restore the savestate that is before the given tick
	// Returns the tick of the restored savestate, or -1 if no suitable tick was found
    int RestoreSavestateBefore(int slot, int tasTick, CUserCmd* cmd = 0) {
		for(auto savestate: savestates[slot] | std::views::reverse) {
			if (savestate.tasTick <= tasTick) {
				RestoreSavestate(slot, savestate, cmd);
				return savestate.tasTick;
			}
		}

		return -1;
	};

	// Clear all savestates
	void clear() {
		savestates[0].clear();
		savestates[1].clear();
	}
};
