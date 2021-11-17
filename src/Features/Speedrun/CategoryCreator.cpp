#include "Categories.hpp"
#include "Command.hpp"
#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include <map>
#include <string>

#define MAX_TRACE 65535.0f

static bool g_creatorActive;
static std::string g_creatorCategory;
static std::map<std::string, std::string> g_creatorDefaults;

static int g_placementStage;
static std::string g_placementName;
static std::string g_placementType;
static std::map<std::string, std::string> g_placementOptions;
static Vector g_placementStart;

static std::optional<std::map<std::string, std::string>> parseParams(size_t argc, const char *const *argv) {
	std::map<std::string, std::string> params;
	for (size_t i = 0; i < argc; ++i) {
		const char *pair = argv[i];
		const char *mid = pair;
		while (*mid && *mid != '=') ++mid;
		if (!*mid) {
			console->Print("Invalid argument '%s'\n", argv[i]);
			return {};
		}
		std::string key = std::string(pair, mid - pair);
		std::string val = std::string(mid + 1);
		if (key == "ccafter") {
			key = "after";
			val = g_creatorCategory + " - " + val;
		}
		params[key] = val;
	}
	return params;
}

CON_COMMAND(sar_speedrun_cc_start, "sar_speedrun_cc_start <category name> [default options]... - start the category creator\n") {
	if (args.ArgC() < 2) {
		console->Print(sar_speedrun_cc_start.ThisPtr()->m_pszHelpString);
		return;
	}

	if (g_creatorActive) {
		console->Print("[cc] category creation already in progress\n");
		return;
	}

	if (!strcmp(args[1], "")) {
		console->Print("[cc] category name cannot be empty\n");
		return;
	}

	g_creatorCategory = args[1];

	auto params = parseParams(args.ArgC() - 2, args.m_ppArgv + 2);
	if (!params) {
		console->Print("[cc] failed to parse options\n");
		return;
	}

	g_creatorDefaults = *params;

	if (!SpeedrunTimer::CreateCategory(g_creatorCategory)) {
		console->Print("[cc] failed to create category\n");
		return;
	}

	g_creatorActive = true;

	g_placementStage = 0;
	g_placementName = "";
	g_placementType = "";
	g_placementOptions = {};
}

CON_COMMAND(sar_speedrun_cc_rule, "sar_speedrun_rule <rule name> <rule type> [options]... - add a rule to the category currently being created\n") {
	if (args.ArgC() < 3) {
		console->Print(sar_speedrun_cc_rule.ThisPtr()->m_pszHelpString);
		return;
	}

	if (!g_creatorActive) {
		console->Print("[cc] no category creation in progress\n");
		return;
	}

	auto params = parseParams(args.ArgC() - 3, args.m_ppArgv + 3);
	if (!params) {
		console->Print("[cc] failed to parse options\n");
		return;
	}

	// Copy, as merge mutates the map
	auto defaults = g_creatorDefaults;
	params->merge(defaults);

	std::string ruleName = g_creatorCategory + " - " + args[1];

	if (!SpeedrunTimer::CreateRule(ruleName, args[2], *params)) {
		console->Print("[cc] failed to create rule\n");
		return;
	}

	if (!SpeedrunTimer::AddRuleToCategory(g_creatorCategory, ruleName)) {
		console->Print("[cc] failed to add rule to category\n");
		return;
	}
}

CON_COMMAND(sar_speedrun_cc_place_start, "sar_speedrun_cc_place_start <rule name> <rule type> [options]... - start placing a trigger-ey rule in the world\n") {
	if (args.ArgC() < 3) {
		console->Print(sar_speedrun_cc_place_start.ThisPtr()->m_pszHelpString);
		return;
	}

	if (!g_creatorActive) {
		console->Print("[cc] no category creation in progress\n");
		return;
	}

	if (!sv_cheats.GetBool()) {
		console->Print("[cc] trigger placement cannot occur without sv_cheats\n");
		return;
	}

	if (g_placementStage != 0) {
		console->Print("[cc] trigger placement already in progress\n");
		return;
	}

	std::string ruleName = g_creatorCategory + " - " + args[1];
	std::string ruleType = args[2];

	if (ruleType != "portal" && ruleType != "zone") {
		console->Print("[cc] '%s' is not a valid trigger-ey rule type\n", ruleType.c_str());
		return;
	}

	auto params = parseParams(args.ArgC() - 3, args.m_ppArgv + 3);
	if (!params) {
		console->Print("[cc] failed to parse options\n");
		return;
	}

	g_placementStage = 1;
	g_placementName = ruleName;
	g_placementType = ruleType;
	g_placementOptions = *params;
}

CON_COMMAND(sar_speedrun_cc_place, "sar_speedrun_cc_place - place a trigger-ey rule in the world\n") {
	if (!g_creatorActive) {
		console->Print("[cc] no category creation in progress\n");
		return;
	}

	if (!sv_cheats.GetBool()) {
		console->Print("[cc] trigger placement cannot occur without sv_cheats\n");
		return;
	}

	switch (g_placementStage) {
	case 0: {
		console->Print("[cc] no trigger placement in progress\n");
		return;
	}
	case 1: {
		CGameTrace tr;
		engine->TraceFromCamera(MAX_TRACE, tr);

		g_placementStart = tr.endpos;

		g_placementStage = 2;

		break;
	}
	case 2: {
		CGameTrace tr;
		engine->TraceFromCamera(MAX_TRACE, tr);

		Vector start = g_placementStart;
		Vector end = tr.endpos;

		Vector center = (start + end) / 2;

		Vector size = start - end;
		size.x = abs(size.x);
		size.y = abs(size.y);
		size.z = abs(size.z);

		if (g_placementType == "portal") {
			// For portal rules, we expand the size by a unit in each
			// direction just to make sure that triggers on flat
			// surfaces work okay.
			size.x += 1;
			size.y += 1;
			size.z += 1;
		}

		float angle = 0;

		auto params = g_placementOptions;
		params.merge(std::map<std::string, std::string>{
			{"center", Utils::ssprintf("%f,%f,%f", center.x, center.y, center.z)},
			{"size", Utils::ssprintf("%f,%f,%f", size.x, size.y, size.z)},
			{"angle", Utils::ssprintf("%f", angle)},
		});
		// Copy, as merge mutates the map
		auto defaults = g_creatorDefaults;
		params.merge(defaults);

		if (!SpeedrunTimer::CreateRule(g_placementName, g_placementType, params)) {
			console->Print("[cc] failed to create rule\n");
			g_placementStage = 0;
			return;
		}

		if (!SpeedrunTimer::AddRuleToCategory(g_creatorCategory, g_placementName)) {
			console->Print("[cc] failed to add rule to category\n");
			g_placementStage = 0;
			return;
		}

		g_placementStage = 0;

		break;
	}
	}
}

CON_COMMAND(sar_speedrun_cc_finish, "sar_speedrun_cc_finish - finish the category creator\n") {
	if (!g_creatorActive) {
		console->Print("[cc] no category creation in progress\n");
		return;
	}

	if (g_placementStage != 0) {
		console->Print("[cc] a trigger placement is still in progress\n");
		return;
	}

	console->Print("[cc] created category '%s'\n", g_creatorCategory.c_str());

	g_creatorActive = false;
	g_creatorCategory = "";
	g_creatorDefaults = {};

	g_placementStage = 0;
	g_placementName = "";
	g_placementType = "";
	g_placementOptions = {};
}

ON_EVENT(PRE_TICK) {
	if (!sv_cheats.GetBool()) {
		return;
	}

	if (g_placementStage == 2) {
		CGameTrace tr;
		engine->TraceFromCamera(MAX_TRACE, tr);

		engine->AddBoxOverlay(
			nullptr,
			Vector{0, 0, 0},
			g_placementStart,
			tr.endpos,
			{0, 0, 0},
			255,
			0,
			0,
			0,
			2 * *engine->interval_per_tick);
	}
}
