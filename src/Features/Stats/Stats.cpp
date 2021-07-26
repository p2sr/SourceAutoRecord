#include "Stats.hpp"

#include "Command.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "JumpStats.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Offsets.hpp"
#include "StatsResultType.hpp"
#include "StepStats.hpp"
#include "Utils.hpp"
#include "Variable.hpp"
#include "VelocityStats.hpp"

Variable sar_stats_jumps_xy("sar_stats_jumps_xy", "0", "Saves jump distance as 2D vector.\n");
Variable sar_stats_velocity_peak_xy("sar_stats_velocity_peak_xy", "0", "Saves velocity peak as 2D vector.\n");
Variable sar_stats_auto_reset("sar_stats_auto_reset", "0", 0,
                              "Resets all stats automatically.\n"
                              "0 = Default,\n"
                              "1 = Restart or disconnect only,\n"
                              "2 = Any load & sar_timer_start.\n"
                              "Note: Portal counter is not part of the \"stats\" feature.\n");

Stats *stats;

Stats::Stats() {
	for (auto i = 0; i < Offsets::MAX_SPLITSCREEN_PLAYERS; ++i) {
		this->playerStats.push_back(new PlayerStats());
	}

	this->hasLoaded = true;
}
Stats::~Stats() {
	for (auto &stat : this->playerStats) {
		delete stat;
	}
	this->playerStats.clear();
}
PlayerStats *Stats::Get(int nSlot) {
	return this->playerStats[nSlot];
}
void Stats::ResetAll() {
	for (auto &stat : this->playerStats) {
		stat->Reset();
	}
}

// Commands

CON_COMMAND(sar_stats_jump, "sar_stats_jump - prints jump stats\n") {
	auto stat = stats->Get(GET_SLOT());

	auto type = std::string();
	if (stat->jumps->type == StatsResultType::VEC2) {
		type = std::string(" (vec2)");
	} else if (stat->jumps->type == StatsResultType::VEC3) {
		type = std::string(" (vec3)");
	}

	console->Print("Distance: %.3f\n", stat->jumps->distance);
	console->Print("Peak: %.3f %s\n", stat->jumps->distancePeak, type.c_str());
	console->Print("Jumps: %i\n", stat->jumps->total);
}
CON_COMMAND(sar_stats_steps, "sar_stats_steps - prints total amount of steps\n") {
	auto stat = stats->Get(GET_SLOT());
	console->Print("Steps: %i\n", stat->jumps->total);
}
CON_COMMAND(sar_stats_velocity, "sar_stats_velocity - prints velocity stats\n") {
	auto nSlot = GET_SLOT();
	auto stat = stats->Get(nSlot);

	std::string type;
	if (stat->velocity->type == StatsResultType::VEC2) {
		type = std::string(" (vec2)");
	} else if (stat->velocity->type == StatsResultType::VEC3) {
		type = std::string(" (vec3)");
	}

	auto player = server->GetPlayer(nSlot + 1);
	if (player) {
		auto curVel = server->GetLocalVelocity(player);
		console->Print("Current: %.3f/%.3f (vec2/vec3)\n", curVel.Length2D(), curVel.Length());
	}

	console->Print("Peak: %.3f %s\n", stat->velocity->peak, type.c_str());
}
CON_COMMAND(sar_stats_jumps_reset, "sar_stats_jumps_reset - resets total jump count and jump distance peak\n") {
	stats->Get(GET_SLOT())->jumps->Reset();
}
CON_COMMAND(sar_stats_steps_reset, "sar_stats_steps_reset - resets total step count\n") {
	stats->Get(GET_SLOT())->steps->Reset();
}
CON_COMMAND(sar_stats_velocity_reset, "sar_stats_velocity_reset - resets velocity peak\n") {
	stats->Get(GET_SLOT())->velocity->Reset();
}
CON_COMMAND(sar_stats_reset, "sar_stats_reset - resets all saved stats\n") {
	stats->Get(GET_SLOT())->Reset();
}

// HUD

HUD_ELEMENT2(jumps, "0", "Draws total jump count.\n", HudType_InGame | HudType_Paused) {
	auto stat = stats->Get(ctx->slot);
	ctx->DrawElement("jumps: %i", stat->jumps->total);
}
HUD_ELEMENT2(portals, "0", "Draws total portal count.\n", HudType_InGame | HudType_Paused) {
	auto player = server->GetPlayer(ctx->slot + 1);
	if (player) {
		ctx->DrawElement("portals: %i", server->GetPortals(player));
	} else {
		ctx->DrawElement("portals: -");
	}
}
HUD_ELEMENT2(steps, "0", "Draws total step count.\n", HudType_InGame | HudType_Paused) {
	auto stat = stats->Get(ctx->slot);
	ctx->DrawElement("steps: %i", stat->steps->total);
}
HUD_ELEMENT2(jump, "0", "Draws current jump distance.\n", HudType_InGame | HudType_Paused) {
	auto stat = stats->Get(ctx->slot);
	ctx->DrawElement("jump: %.3f", stat->jumps->distance);
}
HUD_ELEMENT2(jump_peak, "0", "Draws longest jump distance.\n", HudType_InGame | HudType_Paused) {
	auto stat = stats->Get(ctx->slot);
	ctx->DrawElement("jump peak: %.3f", stat->jumps->distancePeak);
}
HUD_ELEMENT2(velocity_peak, "0", "Draws last saved velocity peak.\n", HudType_InGame | HudType_Paused) {
	auto stat = stats->Get(ctx->slot);
	ctx->DrawElement("vel peak: %.3f", stat->velocity->peak);
}

CON_COMMAND(sar_export_stats, "sar_export_stats <filepath> -  export the stats to the specifed path in a .csv file\n") {
	auto nSlot = GET_SLOT();
	auto stat = stats->Get(nSlot);

	bool result = false;
	std::string path = args.ArgC() == 1 ? sar_statcounter_filePath.GetString() : args[1];

	if (args.ArgC() >= 1) {
		result = stat->statsCounter->ExportToFile(path);
	} else {
		return console->Print(sar_export_stats.ThisPtr()->m_pszHelpString);
	}

	if (!result) {
		return console->Print("Couldn't write to this path. Please verify you actually can write here or the folder exits: \"%s\".\n", path.c_str());
	}

	console->Print("Datas has been successfully exported.\n");
}

CON_COMMAND(sar_import_stats, "sar_import_stats <filePath> - import the stats from the specified .csv file\n") {
	auto nSlot = GET_SLOT();
	auto stat = stats->Get(nSlot);

	bool result = false;
	std::string path = args.ArgC() == 1 ? sar_statcounter_filePath.GetString() : args[1];

	if (args.ArgC() >= 1) {
		result = stat->statsCounter->LoadFromFile(path);
	} else {
		return console->Print(sar_import_stats.ThisPtr()->m_pszHelpString);
	}

	if (!result) {
		return console->Print("Couldn't open the file. Are you sure the file is here? : \"%s\".\n", path.c_str());
	}

	console->Print("Datas has been successfully loaded.\n");
}

CON_COMMAND(sar_print_stats, "sar_print_stats - prints your statistics if those are loaded\n") {
	auto current = 1;
	auto nSlot = GET_SLOT();
	stats->Get(nSlot)->statsCounter->Print();
}
