#include "Categories.hpp"

#include "CategoriesPreset.hpp"
#include "Event.hpp"
#include "Features/Demo/DemoGhostPlayer.hpp"
#include "Features/Hud/Hud.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "SpeedrunTimer.hpp"
#include "Utils.hpp"

#include <cctype>
#include <cstdlib>
#include <memory>
#include <optional>
#include <variant>

Variable sar_speedrun_draw_triggers("sar_speedrun_draw_triggers", "0", "Draw the triggers associated with speedrun rules in the world.\n");
Variable sar_speedrun_triggers_info("sar_speedrun_triggers_info", "0", "Print player velocity (and position) upon mtrigger activation.\n1 - position and velocity\n2 - only horizontal velocity\n");

static std::optional<std::vector<std::string>> extractPartialArgs(const char *str, const char *cmd) {
	while (*cmd) {
		if (*str != *cmd) {
			return {};
		}
		++str, ++cmd;
	}

	if (!isspace(*str) && *str != '"' && *str) {
		return {};
	}

	std::vector<std::string> args;

	while (*str) {
		while (isspace(*str)) ++str;

		if (*str == '"') {
			++str;
			size_t i = 0;
			while (str[i] && str[i] != '"') ++i;
			args.push_back({str, i});
			str += i + 1;
		} else {
			size_t i = 0;
			while (!isspace(str[i]) && str[i]) ++i;
			args.push_back({str, i});
			str += i;
		}
	}

	return args;
}

template <typename V>
static inline V *lookupMap(std::map<std::string, V> &m, std::string k) {
	auto search = m.find(k);
	if (search == m.end()) {
		return nullptr;
	}
	return &search->second;
}

// Pre-set rules and categories {{{

static std::string g_currentCategory = "Singleplayer";
static std::map<std::string, SpeedrunCategory> g_categories;
static std::map<std::string, SpeedrunRule> g_rules;

// Ad-hoc trigger state
static std::vector<std::string> g_adhocTriggers;
static int g_adhocCounter = 0;
static std::string g_previousCategory = "";
static bool g_practiceActive = false;

void SpeedrunTimer::InitCategories() {
	InitSpeedrunCategoriesTo(&g_categories, &g_rules, &g_currentCategory);
}

// }}}

// Testing rules {{{

struct ScheduledRule {
	SpeedrunRule *rule;
	std::string name;
	int tick;
};

static std::vector<ScheduledRule> g_scheduledRules;

static void dispatchRule(std::string name, SpeedrunRule *rule) {
	switch (rule->action) {
	case RuleAction::START:
		if (!SpeedrunTimer::IsRunning()) {
			SpeedrunTimer::Start();
		}
		break;

	case RuleAction::FORCE_START:
		SpeedrunTimer::Start();
		break;

	case RuleAction::STOP:
		SpeedrunTimer::Stop(name);
		break;

	case RuleAction::SPLIT:
		SpeedrunTimer::Split(true, name);
		break;

	case RuleAction::PAUSE:
		SpeedrunTimer::Pause();
		break;

	case RuleAction::RESUME:
		SpeedrunTimer::Resume();
		break;
	}

	rule->fired = true;

	// Handle `sar_speedrun_triggers_info`
	int info = sar_speedrun_triggers_info.GetInt();
	if (info == 0) return;

	void *player = client->GetPlayer(GET_SLOT() + 1);
	if (!player) return;

	Vector pos = client->GetAbsOrigin(player);
	Vector vel = client->GetLocalVelocity(player);

	if (info == 1) {
		// Info type 1 prints everything
		console->Print("Player triggered rule '%s':\n", name.c_str());
		console->Print("  Position: %.2f %.2f %.2f\n", pos.x, pos.y, pos.z);
		console->Print("  Velocity: %.2f %.2f %.2f\n", vel.x, vel.y, vel.z);
	} else if (info == 2) {
		// Info type 2 prints just the horizontal velocity
		console->Print("Player velocity on last rule: %.2f\n", vel.Length2D());
	}
}

ON_EVENT(PRE_TICK) {
	int tick = SpeedrunTimer::GetTotalTicks();
	for (size_t i = 0; i < g_scheduledRules.size(); ++i) {
		if (tick >= g_scheduledRules[i].tick) {
			dispatchRule(g_scheduledRules[i].name, g_scheduledRules[i].rule);
			g_scheduledRules.erase(g_scheduledRules.begin() + i);
			--i;
		}
	}
}

static int getNextCycleTick(std::pair<int, int> cycle) {
	int cur = SpeedrunTimer::GetTotalTicks();
	int cur1 = cur - cycle.first;
	if (cur1 % cycle.second == cycle.second - 1) return cur;
	return cur + cycle.second - (cur1 % cycle.second) - 1;
}

static void dispatchRuleCycled(std::string name, SpeedrunRule *rule) {
	if (!rule->cycle) {
		dispatchRule(name, rule);
		return;
	}

	int tick = getNextCycleTick(*rule->cycle);
	if (tick == SpeedrunTimer::GetTotalTicks()) {
		dispatchRule(name, rule);
		return;
	}

	g_scheduledRules.push_back({rule, name, tick});
}

template <typename RuleType, typename... Ts>
static bool GeneralTestRules(std::optional<int> slot, Ts... args) {
	if (engine->IsOrange()) return false;
	for (std::string ruleName : g_categories[g_currentCategory].rules) {
		auto rule = SpeedrunTimer::GetRule(ruleName);
		if (!rule) continue;
		if (!std::holds_alternative<RuleType>(rule->rule)) continue;
		if (!SpeedrunTimer::IsRunning() && rule->action != RuleAction::START && rule->action != RuleAction::FORCE_START) continue;
		if (!rule->TestGeneral(slot)) continue;
		if (!std::get<RuleType>(rule->rule).Test(args...)) continue;

		dispatchRuleCycled(ruleName, rule);

		rule->fired = true;
		return true;  // We don't want to dispatch any more rules in this tick lest shit get fucked
	}
	return false;
}

bool SpeedrunTimer::TestInputRules(std::string targetname, std::string classname, std::string inputname, std::string parameter, std::optional<int> triggerSlot) {
	bool result = GeneralTestRules<EntityInputRule>(triggerSlot, targetname, classname, inputname, parameter);
	if (result)
		demoGhostPlayer.TestInputRule(targetname.c_str(), classname.c_str(), inputname.c_str(), parameter.c_str(), triggerSlot);

	return result;
}

void SpeedrunTimer::TestZoneRules(Vector pos, int slot) {
	GeneralTestRules<ZoneTriggerRule>(slot, pos);
}

void SpeedrunTimer::TestJumpRules(Vector pos, int slot) {
	GeneralTestRules<JumpTriggerRule>(slot, pos);
}

bool SpeedrunTimer::TestPortalRules(Vector pos, int slot, PortalColor portal) {
	bool result = GeneralTestRules<PortalPlacementRule>(slot, pos, portal);
	if(result)
		demoGhostPlayer.TestInputRule( pos, slot, portal);

	return result;
}

bool SpeedrunTimer::TestFlagRules(int slot) {
	bool result = GeneralTestRules<ChallengeFlagsRule>(slot);
	if (result)
		demoGhostPlayer.TestInputRule(slot);

	return result;
}

bool SpeedrunTimer::TestFlyRules(int slot) {
	bool result = GeneralTestRules<CrouchFlyRule>(slot);
	if (result)
		demoGhostPlayer.TestInputRule(slot);

	return result;
}

void SpeedrunTimer::TestLoadRules() {
	GeneralTestRules<MapLoadRule>({});
}

ON_EVENT(SESSION_END) {
	if (event.transition) GeneralTestRules<MapEndRule>({});
}

ON_EVENT(CM_FLAGS) {
	if (!engine->IsOrange()) {
		SpeedrunTimer::TestFlagRules(event.slot);
		if (event.end) GeneralTestRules<MapEndRule>({});
	}
}

// }}}

SpeedrunRule *SpeedrunTimer::GetRule(std::string name) {
	auto search = g_rules.find(name);
	if (search == g_rules.end()) {
		return nullptr;
	}

	return &search->second;
}

std::vector<std::string> SpeedrunTimer::GetCategoryRules() {
	return g_categories[g_currentCategory].rules;
}

std::string SpeedrunTimer::GetCategoryName() {
	return g_currentCategory;
}

void SpeedrunTimer::ResetCategory() {
	for (std::string ruleName : g_categories[g_currentCategory].rules) {
		auto rule = SpeedrunTimer::GetRule(ruleName);
		if (!rule) continue;
		rule->fired = false;
	}
	g_scheduledRules.clear();
}

ON_EVENT(RENDER) {
	if (!sar_speedrun_draw_triggers.GetBool()) return;
	if (!sv_cheats.GetBool()) return;

	for (std::string ruleName : g_categories[g_currentCategory].rules) {
		auto rule = SpeedrunTimer::GetRule(ruleName);
		if (!rule) continue;
		if (std::find_if(rule->maps.begin(), rule->maps.end(), [](std::string map) {
				return map == "*" || !strcasecmp(map.c_str(), engine->GetCurrentMapName().c_str());
			}) == rule->maps.end()) continue;
		if (std::holds_alternative<ZoneTriggerRule>(rule->rule)) {
			std::get<ZoneTriggerRule>(rule->rule).DrawInWorld();
			std::get<ZoneTriggerRule>(rule->rule).OverlayInfo(rule);
		} else if (std::holds_alternative<JumpTriggerRule>(rule->rule)) {
			std::get<JumpTriggerRule>(rule->rule).DrawInWorld();
			std::get<JumpTriggerRule>(rule->rule).OverlayInfo(rule);
		} else if (std::holds_alternative<PortalPlacementRule>(rule->rule)) {
			std::get<PortalPlacementRule>(rule->rule).DrawInWorld();
			std::get<PortalPlacementRule>(rule->rule).OverlayInfo(rule);
		}
	}
}

static std::string maybeQuote(std::string arg) {
	if (arg.find(' ') != std::string::npos) {
		return "\"" + arg + "\"";
	}

	return arg;
}

static int vectorCompletion(const char *partial, const char *cmd, char completionsOut[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH], std::vector<std::vector<std::string>> options) {
	auto existing = extractPartialArgs(partial, cmd);
	if (!existing || existing->size() == 0) {
		return {};
	}

	size_t argIdx = existing->size() - 1;
	if (argIdx >= options.size()) {
		return {};
	}

	auto argOpts = options[argIdx];

	std::string argPart = (*existing)[argIdx];

	int completionIdx = 0;

	for (size_t i = 0; i < argOpts.size(); ++i) {
		if (argOpts[i].substr(0, argPart.size()) == argPart) {
			auto compArgs = *existing;
			compArgs[compArgs.size() - 1] = argOpts[i];
			std::string comp = cmd;
			for (std::string arg : compArgs) {
				comp += " " + maybeQuote(arg);
			}
			strncpy(completionsOut[completionIdx], comp.c_str(), COMMAND_COMPLETION_ITEM_LENGTH - 1);
			completionsOut[completionIdx][COMMAND_COMPLETION_ITEM_LENGTH - 1] = 0;
			if (++completionIdx == COMMAND_COMPLETION_MAXITEMS) {
				break;
			}
		}
	}

	return completionIdx;
}

template <typename K, typename V>
static std::vector<K> mapKeys(std::map<K, V> m) {
	std::vector<K> keys;
	keys.reserve(m.size());
	for (const auto &x : m) {
		keys.push_back(x.first);
	}
	return keys;
}

// Setting/printing categories {{{

static int _sar_speedrun_category_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	return vectorCompletion(partial, "sar_speedrun_category", commands, {mapKeys(g_categories)});
}

CON_COMMAND_F_COMPLETION(sar_speedrun_category, "sar_speedrun_category [category] - get or set the speedrun category\n", 0, &_sar_speedrun_category_completion) {
	if (args.ArgC() > 1) {
		if (!lookupMap(g_categories, args[1])) {
			console->Print("Category %s does not exist!\n", args[1]);
		} else {
			// Don't allow changing category if we're in practice mode
			// (unless we're explicitly setting it to Practice, which happens in ensurePracticeCategory)
			if (g_practiceActive && std::string(args[1]) != "Practice") {
				console->Print("Cannot change category to '%s' while ad-hoc triggers are active.\n", args[1]);
				console->Print("Use 'sar_speedrun_adhoc_clear' to exit practice mode.\n");
				return;
			}

			bool same = g_currentCategory == args[1];
			if (!same) {
				g_currentCategory = args[1];
				SpeedrunTimer::CategoryChanged();
				g_scheduledRules.clear();
			}
			console->Print("Using category '%s'\n", g_currentCategory.c_str());
			return;
		}
	}

	console->Print("Using '%s' with %d rules:\n", g_currentCategory.c_str(), g_categories[g_currentCategory].rules.size());
	for (std::string ruleName : g_categories[g_currentCategory].rules) {
		console->Print("    %s\n", ruleName.c_str());
	}
	console->Print("\n");
	console->Print("Available categories:\n");
	for (auto cat : g_categories) {
		console->Print("    %s (%d rules)\n", cat.first.c_str(), cat.second.rules.size());
	}
}

// }}}

// Showing rules {{{

static int _sar_speedrun_rule_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	return vectorCompletion(partial, "sar_speedrun_rule", commands, {mapKeys(g_rules)});
}

CON_COMMAND_F_COMPLETION(sar_speedrun_rule, "sar_speedrun_rule [rule] - show information about speedrun rules\n", 0, &_sar_speedrun_rule_completion) {
	if (args.ArgC() == 1) {
		for (auto rule : g_rules) {
			console->Print("%s %s\n", rule.first.c_str(), rule.second.Describe().c_str());
		}
	} else if (args.ArgC() == 2) {
		auto rule = SpeedrunTimer::GetRule(args[1]);
		if (!rule) {
			console->Print("Rule %s does not exist!\n", args[1]);
		} else {
			console->Print("%s %s\n", args[1], rule->Describe().c_str());
		}
	}
}

// }}}

// Category creation/deletion {{{

bool SpeedrunTimer::CreateCategory(std::string name) {
	if (name == "") {
		console->Print("Category name cannot be empty\n");
		return false;
	}

	if (lookupMap(g_categories, name)) {
		console->Print("Category %s already exists\n", name.c_str());
		return false;
	}

	g_categories[name] = SpeedrunCategory{{}};
	return true;
}

CON_COMMAND(sar_speedrun_category_create, "sar_speedrun_category_create <category> - create a new speedrun category with the given name\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_speedrun_category_create.ThisPtr()->m_pszHelpString);
	}

	SpeedrunTimer::CreateCategory(args[1]);
}

static int _sar_speedrun_category_remove_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	return vectorCompletion(partial, "sar_speedrun_category_remove", commands, {mapKeys(g_categories)});
}

CON_COMMAND_F_COMPLETION(sar_speedrun_category_remove, "sar_speedrun_category_remove <category> - delete the given speedrun category\n", 0, &_sar_speedrun_category_remove_completion) {
	if (args.ArgC() != 2) {
		return console->Print(sar_speedrun_category_remove.ThisPtr()->m_pszHelpString);
	}

	std::string catName = args[1];

	if (!lookupMap(g_categories, catName)) {
		return console->Print("Category %s does not exist\n", args[1]);
	}

	if (g_currentCategory == catName) {
		return console->Print("Cannot delete the current category\n");
	}

	g_categories.erase(catName);
}

// }}}

// Category rule management {{{

static int _sar_speedrun_category_add_rule_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	return vectorCompletion(partial, "sar_speedrun_category_add_rule", commands, {mapKeys(g_categories), mapKeys(g_rules)});
}

bool SpeedrunTimer::AddRuleToCategory(std::string category, std::string rule) {
	auto cat = lookupMap(g_categories, category);
	if (!cat) {
		console->Print("Category %s does not exist\n", category.c_str());
		return false;
	}

	if (!lookupMap(g_rules, rule)) {
		console->Print("Rule %s does not exist\n", rule.c_str());
		return false;
	}

	if (std::find(cat->rules.begin(), cat->rules.end(), rule) != cat->rules.end()) {
		console->Print("Rule %s already in category %s\n", rule.c_str(), category.c_str());
		return false;
	}

	cat->rules.push_back(rule);
	return true;
}

CON_COMMAND_F_COMPLETION(sar_speedrun_category_add_rule, "sar_speedrun_category_add_rule <category> <rule> - add a rule to a speedrun category\n", 0, &_sar_speedrun_category_add_rule_completion) {
	if (args.ArgC() != 3) {
		return console->Print(sar_speedrun_category_add_rule.ThisPtr()->m_pszHelpString);
	}

	SpeedrunTimer::AddRuleToCategory(args[1], args[2]);
}

static int _sar_speedrun_category_remove_rule_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	return vectorCompletion(partial, "sar_speedrun_category_remove_rule", commands, {mapKeys(g_categories), mapKeys(g_rules)});
}

CON_COMMAND_F_COMPLETION(sar_speedrun_category_remove_rule, "sar_speedrun_category_remove_rule <category> <rule> - remove a rule from a speedrun category\n", 0, &_sar_speedrun_category_remove_rule_completion) {
	if (args.ArgC() != 3) {
		return console->Print(sar_speedrun_category_remove_rule.ThisPtr()->m_pszHelpString);
	}

	auto cat = lookupMap(g_categories, args[1]);
	if (!cat) {
		return console->Print("Category %s does not exist\n", args[1]);
	}

	auto it = std::find(cat->rules.begin(), cat->rules.end(), std::string(args[2]));
	if (it == cat->rules.end()) {
		return console->Print("Rule %s is not in category %s\n", args[2], args[1]);
	}

	cat->rules.erase(it);
}

// }}}

// Rule creation/deletion {{{

bool SpeedrunTimer::CreateRule(std::string name, std::string type, std::map<std::string, std::string> params) {
	if (name == "") {
		console->Print("Rule name cannot be empty\n");
		return false;
	}

	if (SpeedrunTimer::GetRule(name)) {
		console->Print("Rule %s already exists\n", name.c_str());
		return false;
	}

	RuleAction action;

	auto actionStr = lookupMap(params, "action");
	if (!actionStr) {
		console->Print("action not specified\n");
		return false;
	}

	if (*actionStr == "start") {
		action = RuleAction::START;
	} else if (*actionStr == "force_start") {
		action = RuleAction::FORCE_START;
	} else if (*actionStr == "stop") {
		action = RuleAction::STOP;
	} else if (*actionStr == "split") {
		action = RuleAction::SPLIT;
	} else if (*actionStr == "pause") {
		action = RuleAction::PAUSE;
	} else if (*actionStr == "resume") {
		action = RuleAction::RESUME;
	} else {
		console->Print("Unknown action %s\n", actionStr->c_str());
		return false;
	}

	if (!lookupMap(params, "map")) {
		console->Print("map not specified\n");
		return false;
	}

	std::optional<SpeedrunRule> rule =
		type == "entity"   ? EntityInputRule::Create(params)
		: type == "zone"   ? ZoneTriggerRule::Create(params)
		: type == "portal" ? PortalPlacementRule::Create(params)
		: type == "flags"  ? ChallengeFlagsRule::Create(params)
		: type == "fly"    ? CrouchFlyRule::Create(params)
		: type == "load"   ? MapLoadRule::Create(params)
		: type == "end"    ? MapEndRule::Create(params)
		: type == "jump"   ? JumpTriggerRule::Create(params)
		: std::optional<SpeedrunRule>{};

	if (!rule) {
		console->Print("Failed to create rule\n");
		return false;
	}

	rule->action = action;

	std::string mapToken, delimiter = "|";
	if (params["map"].find(delimiter) != std::string::npos) {
		size_t pos = 0;
		while ((pos = params["map"].find(delimiter)) != std::string::npos) {
			mapToken = params["map"].substr(0, pos);
			rule->maps.push_back(mapToken);
			params["map"].erase(0, pos + delimiter.length());
		}
	}
	if (params["map"].length() > 0) rule->maps.push_back(params["map"]);

	auto after = lookupMap(params, "after");
	if (!after) {
		rule->onlyAfter = {};
	} else {
		rule->onlyAfter = *after;
	}

	auto cycle = lookupMap(params, "cycle");
	rule->cycle = {};
	if (cycle) {
		char *end;
		int start = strtol(cycle->c_str(), &end, 10);
		if (*end == ',') {
			int freq = atoi(end+1);
			rule->cycle = {start, freq};
		}
	}

	auto slotStr = lookupMap(params, "player");
	if (slotStr && type != "load") {
		rule->slot = atoi(slotStr->c_str());
	}

	rule->fired = false;

	g_rules.insert({name, *rule});

	return true;
}

CON_COMMAND(sar_speedrun_rule_create, "sar_speedrun_rule_create <name> <type> [option=value]... - create a speedrun rule with the given name, type, and options\n") {
	if (args.ArgC() < 3) {
		return console->Print(sar_speedrun_rule_create.ThisPtr()->m_pszHelpString);
	}

	std::map<std::string, std::string> params;

	for (int i = 3; i < args.ArgC(); ++i) {
		const char *pair = args[i];
		const char *mid = pair;
		while (*mid && *mid != '=') ++mid;
		if (!*mid) {
			return console->Print("Invalid argument '%s'\n", args[i]);
		}
		std::string key = std::string(pair, mid - pair);
		std::string val = std::string(mid + 1);
		params[key] = val;
	}

	SpeedrunTimer::CreateRule(args[1], args[2], params);
}

static int _sar_speedrun_rule_remove_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH]) {
	return vectorCompletion(partial, "sar_speedrun_rule_remove", commands, {mapKeys(g_rules)});
}


CON_COMMAND_F_COMPLETION(sar_speedrun_rule_remove, "sar_speedrun_rule_remove <rule> - delete the given speedrun rule\n", 0, &_sar_speedrun_rule_remove_completion) {
	if (args.ArgC() != 2) {
		return console->Print(sar_speedrun_rule_remove.ThisPtr()->m_pszHelpString);
	}

	std::string ruleName = args[1];

	auto rule = SpeedrunTimer::GetRule(ruleName);
	if (!rule) {
		return console->Print("Rule %s does not exist\n", args[1]);
	}

	bool canDelete = true;
	for (auto cat : g_categories) {
		if (std::find(cat.second.rules.begin(), cat.second.rules.end(), ruleName) != cat.second.rules.end()) {
			console->Print("Cannot delete rule %s as it exists in the category %s\n", ruleName.c_str(), cat.first.c_str());
			canDelete = false;
		}
	}

	if (canDelete) {
		g_rules.erase(ruleName);
	}
}

// }}}

CON_COMMAND(sar_speedrun_reset_categories, "sar_speedrun_reset_categories - delete all custom categories and rules, reverting to the builtin ones\n") {
	if (args.ArgC() != 2 || std::string(args[1]) != "yes") {
		console->Print("WARNING: this will delete all custom categories! Run 'sar_speedrun_reset_categories yes' to confirm.\n");
		return;
	}

	SpeedrunTimer::InitCategories();
	SpeedrunTimer::CategoryChanged();
	g_scheduledRules.clear();
}

// ============================================
// Ad-hoc triggers
// ============================================

static void ensurePracticeCategory() {
	// Check if we're ACTUALLY in Practice category (not just if flag is set)
	// This handles cases where category was reset but g_practiceActive wasn't
	if (g_practiceActive && g_currentCategory == "Practice") {
		return;
	}

	// Save current category (only if not already in practice mode)
	if (!g_practiceActive) {
		g_previousCategory = SpeedrunTimer::GetCategoryName();
	}

	// Create Practice category if it doesn't exist
	if (!lookupMap(g_categories, std::string("Practice"))) {
		g_categories["Practice"] = SpeedrunCategory{{}};
	}

	// Switch to Practice category (same as sar_speedrun_category command)
	g_currentCategory = "Practice";
	SpeedrunTimer::CategoryChanged();
	g_scheduledRules.clear();

	// Set offset to 0 and start_on_load to 2 (always restart on load)
	sar_speedrun_offset.SetValue(0);
	sar_speedrun_start_on_load.SetValue(2);

	g_practiceActive = true;
	console->Print("Switched to Practice category (offset=0, start_on_load=2)\n");
}

CON_COMMAND(sar_speedrun_adhoc_trigger, "sar_speedrun_adhoc_trigger - create a trigger at current position that splits the timer when touched\n") {
	// Ensure we're in Practice category with proper settings
	ensurePracticeCategory();

	// Auto-enable cheats and visualization
	Variable sv_cheats("sv_cheats");
	sv_cheats.SetValue(1);
	sar_speedrun_draw_triggers.SetValue(1);

	// Get player position
	void *player = server->GetPlayer(GET_SLOT() + 1);
	if (!player) {
		console->Print("No player found\n");
		return;
	}
	Vector pos = server->GetAbsOrigin(player);

	// Generate unique rule name
	std::string ruleName = Utils::ssprintf("adhoc_%d", g_adhocCounter++);

	// Create rule parameters
	std::map<std::string, std::string> params = {
		{"center", Utils::ssprintf("%f,%f,%f", pos.x, pos.y, pos.z)},
		{"size", "64,64,128"},
		{"angle", "0"},
		{"action", "split"},
		{"map", "*"},
	};

	// Create the rule
	if (!SpeedrunTimer::CreateRule(ruleName, "zone", params)) {
		console->Print("Failed to create adhoc trigger\n");
		return;
	}
	console->Print("[adhoc] Created rule '%s' in g_rules\n", ruleName.c_str());

	// Add to Practice category
	if (!SpeedrunTimer::AddRuleToCategory("Practice", ruleName)) {
		console->Print("Failed to add adhoc trigger to category\n");
		return;
	}

	g_adhocTriggers.push_back(ruleName);

	// Debug: show category state
	console->Print("[adhoc] Current category: %s with %d rules\n", 
	               g_currentCategory.c_str(), 
	               (int)g_categories[g_currentCategory].rules.size());

	console->Print("Created adhoc trigger '%s' at %.0f, %.0f, %.0f (total: %d)\n", 
	               ruleName.c_str(), pos.x, pos.y, pos.z, (int)g_adhocTriggers.size());
	// console->Print("Use 'sar_speedrun_draw_triggers 1' to visualize\n");
}

CON_COMMAND(sar_speedrun_adhoc_clear, "sar_speedrun_adhoc_clear - remove all adhoc triggers and restore previous category\n") {
	if (g_adhocTriggers.empty() && !g_practiceActive) {
		console->Print("No adhoc triggers to clear\n");
		return;
	}

	int count = 0;

	// Remove all adhoc triggers from Practice category
	auto cat = lookupMap(g_categories, std::string("Practice"));
	for (const auto& name : g_adhocTriggers) {
		if (cat) {
			auto it = std::find(cat->rules.begin(), cat->rules.end(), name);
			if (it != cat->rules.end()) {
				cat->rules.erase(it);
			}
		}
		g_rules.erase(name);
		count++;
	}
	g_adhocTriggers.clear();

	// Restore previous category
	if (g_practiceActive && !g_previousCategory.empty()) {
		g_currentCategory = g_previousCategory;
		SpeedrunTimer::CategoryChanged();
		console->Print("Restored category: %s\n", g_previousCategory.c_str());
	}

	g_practiceActive = false;
	g_previousCategory = "";
	g_adhocCounter = 0;

	console->Print("Cleared %d adhoc triggers\n", count);
}
