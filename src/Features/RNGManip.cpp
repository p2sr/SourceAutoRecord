#include "RNGManip.hpp"
#include "Utils/json11.hpp"
#include "Event.hpp"
#include "Offsets.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include <cstring>
#include <fstream>
#include <string>
#include <sstream>
#include <deque>

static std::optional<json11::Json> g_session_state;
static std::optional<json11::Json> g_pending_load;

static std::deque<QAngle> g_queued_view_punches;
static std::vector<QAngle> g_recorded_view_punches;

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

// clear old rng data
ON_EVENT_P(SESSION_START, 999) {
	g_queued_view_punches.clear();
	g_recorded_view_punches.clear();
}

// load pending rng data
ON_EVENT(SESSION_START) {
	if (!g_pending_load) return;

	json11::Json data = *g_pending_load;
	g_pending_load = std::optional<json11::Json>{};

	if (!engine->isRunning()) return;
	if (!sv_cheats.GetBool()) return;

	if (!data.is_object()) {
		console->Print("Invalid p2rng data!\n");
		return;
	}

	if (data["map"].string_value() != engine->GetCurrentMapName()) {
		console->Print("Invalid map for p2rng data!\n");
		return;
	}

	if (!restorePaintSprayers(data["paint"])) {
		console->Print("Failed to restore p2rng paint sprayer data!\n");
	}

	if (!restoreViewPunches(data["view_punch"])) {
		console->Print("Failed to restore p2rng view punch data!\n");
	}

	console->Print("p2rng restore complete\n");
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

	FILE *f = fopen(filename, "w");
	if (!f) {
		console->Print("Failed to open file %s\n", filename);
		return;
	}

	fputs(json11::Json(root).dump().c_str(), f);
	fclose(f);

	console->Print("Wrote RNG data to %s\n", filename);
}

void RngManip::loadData(const char *filename) {
	std::ifstream st(filename);
	if (!st.good()) {
		console->Print("Failed to open file %s\n", filename);
		return;
	}

	std::stringstream buf;
	buf << st.rdbuf();

	std::string err;
	auto json = json11::Json::parse(buf.str(), err);
	if (err != "") {
		console->Print("Failed to parse p2rng file: %s\n", err.c_str());
		return;
	}

	g_pending_load = json;

	console->Print("Read RNG data from %s\n", filename);
}

void RngManip::viewPunch(QAngle *offset) {
	if (g_queued_view_punches.size() > 0) {
		*offset = g_queued_view_punches.front();
		g_queued_view_punches.pop_front();
	}

	g_recorded_view_punches.push_back(*offset);
}

CON_COMMAND(sar_rng_save, "sar_rng_save <filename> - save RNG seed data to the specified file\n") {
	if (args.ArgC() != 2) {
		console->Print(sar_rng_save.ThisPtr()->m_pszHelpString);
		return;
	}

	std::string filename = std::string(args[1]) + ".p2rng";
	RngManip::saveData(filename.c_str());
}

CON_COMMAND(sar_rng_load, "sar_rng_load <filename> - load RNG seed data on next session start\n") {
	if (args.ArgC() != 2) {
		console->Print(sar_rng_load.ThisPtr()->m_pszHelpString);
		return;
	}

	std::string filename = std::string(args[1]) + ".p2rng";
	RngManip::loadData(filename.c_str());
}
