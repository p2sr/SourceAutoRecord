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

#include <cstring>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>

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
	playerTrace->EmitLog(Utils::ssprintf("ViewPunch(%.6f, %.6f, %.6f) -> (%.6f, %.6f, %.6f)", orig.x, orig.y, orig.z, offset->x, offset->y, offset->z).c_str());
}

void RngManip::randomSeed(int *seed) {
	int orig = *seed;
	if (g_queued_randomseeds.size() > 0) {
		*seed = g_queued_randomseeds.front();
		g_queued_randomseeds.pop_front();
	}

	g_recorded_randomseeds.push_back(*seed);
	playerTrace->EmitLog(Utils::ssprintf("RandomSeed(%d) -> %d", orig, *seed).c_str());
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

ON_INIT {
	RandomSeed = Memory::GetSymbolAddress<decltype(RandomSeed)>(Memory::GetModuleHandleByName(tier1->Name()), "RandomSeed");
	g_RandomSeed_Hook.SetFunc(RandomSeed);
}
