#include "Categories.hpp"
#include "SpeedrunTimer.hpp"
#include "CategoriesPreset.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Hud/Hud.hpp"
#include "Utils.hpp"

#include <memory>
#include <set>
#include <variant>
#include <cstdlib>

Variable sar_speedrun_draw_triggers("sar_speedrun_draw_triggers", "0", "Draw the triggers associated with speedrun rules in the world.\n");

template<typename V>
static inline V *lookupMap(std::map<std::string, V> &m, std::string k)
{
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

void SpeedrunTimer::InitCategories()
{
    InitSpeedrunCategoriesTo(&g_categories, &g_rules, &g_currentCategory);
}

// }}}

// Testing rules {{{

static void dispatchRule(std::string name, SpeedrunRule *rule)
{
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
}

template<typename RuleType, typename... Ts>
static void GeneralTestRules(std::optional<int> slot, Ts... args)
{
    if (engine->IsOrange()) return;
    for (std::string ruleName : g_categories[g_currentCategory].rules) {
        auto rule = SpeedrunTimer::GetRule(ruleName);
        if (!rule) continue;
        if (!std::holds_alternative<RuleType>(rule->rule)) continue;
        if (!SpeedrunTimer::IsRunning() && rule->action != RuleAction::START && rule->action != RuleAction::FORCE_START) continue;
        if (!rule->TestGeneral(slot)) continue;
        if (!std::get<RuleType>(rule->rule).Test(args...)) continue;

        dispatchRule(ruleName, rule);

        rule->fired = true;
        return; // We don't want to dispatch any more rules in this tick lest shit get fucked
    }
}

void SpeedrunTimer::TestInputRules(std::string targetname, std::string classname, std::string inputname, std::string parameter, std::optional<int> triggerSlot)
{
    GeneralTestRules<EntityInputRule>(triggerSlot, targetname, classname, inputname, parameter);
}

void SpeedrunTimer::TestZoneRules(Vector pos, int slot)
{
    GeneralTestRules<ZoneTriggerRule>(slot, pos);
}

void SpeedrunTimer::TestPortalRules(Vector pos, int slot, PortalColor portal)
{
    GeneralTestRules<PortalPlacementRule>(slot, pos, portal);
}

void SpeedrunTimer::TestFlagRules(int slot)
{
    GeneralTestRules<ChallengeFlagsRule>(slot);
}

void SpeedrunTimer::TestFlyRules(int slot)
{
    GeneralTestRules<CrouchFlyRule>(slot);
}

void SpeedrunTimer::TestLoadRules()
{
    GeneralTestRules<MapLoadRule>({});
}

// }}}

SpeedrunRule *SpeedrunTimer::GetRule(std::string name)
{
    auto search = g_rules.find(name);
    if (search == g_rules.end()) {
        return nullptr;
    }

    return &search->second;
}

void SpeedrunTimer::ResetCategory()
{
    for (std::string ruleName : g_categories[g_currentCategory].rules) {
        auto rule = SpeedrunTimer::GetRule(ruleName);
        if (!rule) continue;
        rule->fired = false;
    }
}

void SpeedrunTimer::DrawTriggers()
{
    const int drawDelta = 30;
    static int lastDrawTick = -1000;

    if (!sar_speedrun_draw_triggers.GetBool()) return;
    if (!sv_cheats.GetBool()) return;

    int tick = engine->GetTick();
    if (tick > lastDrawTick && tick < lastDrawTick + drawDelta) return;

    lastDrawTick = tick;

    for (std::string ruleName : g_categories[g_currentCategory].rules) {
        auto rule = SpeedrunTimer::GetRule(ruleName);
        if (!rule) continue;
        if (rule->map != engine->GetCurrentMapName()) continue;
        if (std::holds_alternative<ZoneTriggerRule>(rule->rule)) {
            std::get<ZoneTriggerRule>(rule->rule).DrawInWorld((drawDelta + 1) * *engine->interval_per_tick);
        } else if (std::holds_alternative<PortalPlacementRule>(rule->rule)) {
            std::get<PortalPlacementRule>(rule->rule).DrawInWorld((drawDelta + 1) * *engine->interval_per_tick);
        }
    }
}

HUD_ELEMENT2_NO_DISABLE(speedrun_triggers, HudType_InGame)
{
    if (sar_speedrun_draw_triggers.GetBool() && sv_cheats.GetBool()) {
        for (std::string ruleName : g_categories[g_currentCategory].rules) {
            auto rule = SpeedrunTimer::GetRule(ruleName);
            if (!rule) continue;
            if (rule->map != engine->GetCurrentMapName()) continue;
            if (std::holds_alternative<ZoneTriggerRule>(rule->rule)) {
                std::get<ZoneTriggerRule>(rule->rule).OverlayInfo(ctx, rule);
            } else if (std::holds_alternative<PortalPlacementRule>(rule->rule)) {
                std::get<PortalPlacementRule>(rule->rule).OverlayInfo(ctx, rule);
            }
        }
    }
}

static std::string maybeQuote(std::string arg)
{
    if (arg.find(' ') != std::string::npos) {
        return "\"" + arg + "\"";
    }

    return arg;
}

static int vectorCompletion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH], std::vector<std::string> possible)
{
    size_t i = 0;
    for (std::string cmd : possible) {
        const char *cmd_c = cmd.c_str();

        if (strstr(cmd_c, partial) != cmd_c) {
            continue;
        }

        strncpy(commands[i], cmd_c, COMMAND_COMPLETION_ITEM_LENGTH - 1);
        commands[i][COMMAND_COMPLETION_ITEM_LENGTH - 1] = 0;

        if (++i == COMMAND_COMPLETION_MAXITEMS) {
            break;
        }
    }

    return i;
}

// Setting/printing categories {{{

static int _sar_speedrun_category_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::vector<std::string> possible;

    possible.push_back("sar_speedrun_category");

    for (auto cat : g_categories) {
        possible.push_back("sar_speedrun_category " + maybeQuote(cat.first));
    }

    return vectorCompletion(partial, commands, possible);
}

CON_COMMAND_F_COMPLETION(sar_speedrun_category, "sar_speedrun_category [category] - get or set the speedrun category.\n", 0, &_sar_speedrun_category_completion)
{
    if (args.ArgC() > 1) {
        if (!lookupMap(g_categories, args[1])) {
            console->Print("Category %s does not exist!\n", args[1]);
        } else {
            g_currentCategory = args[1];
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

static int _sar_speedrun_rule_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::vector<std::string> possible;

    possible.push_back("sar_speedrun_rule");

    for (auto rule : g_rules) {
        possible.push_back("sar_speedrun_rule " + maybeQuote(rule.first));
    }

    return vectorCompletion(partial, commands, possible);
}


CON_COMMAND_F_COMPLETION(sar_speedrun_rule, "sar_speedrun_rule [rule] - show information about speedrun rules.\n", 0, &_sar_speedrun_rule_completion)
{
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

bool SpeedrunTimer::CreateCategory(std::string name)
{
    if (name == "") {
        console->Print("Category name cannot be empty\n");
        return false;
    }

    if (lookupMap(g_categories, name)) {
        console->Print("Category %s already exists\n", name.c_str());
        return false;
    }

    g_categories[name] = SpeedrunCategory{ { } };
    return true;
}

CON_COMMAND(sar_speedrun_category_create, "sar_speedrun_category_create <category> - create a new speedrun category with the given name.\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_speedrun_category_create.ThisPtr()->m_pszHelpString);
    }

    SpeedrunTimer::CreateCategory(args[1]);
}

static int _sar_speedrun_category_remove_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::vector<std::string> possible;

    for (auto cat : g_categories) {
        possible.push_back("sar_speedrun_category_remove " + maybeQuote(cat.first));
    }

    return vectorCompletion(partial, commands, possible);
}

CON_COMMAND_F_COMPLETION(sar_speedrun_category_remove, "sar_speedrun_category_remove <category> - delete the given speedrun category.\n", 0, &_sar_speedrun_category_remove_completion)
{
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

static int _sar_speedrun_category_add_rule_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::vector<std::string> possible;

    for (auto cat : g_categories) {
        for (auto rule : g_rules) {
            if (cat.second.rules.find(rule.first) == cat.second.rules.end()) {
                possible.push_back(Utils::ssprintf("sar_speedrun_category_add_rule %s %s", maybeQuote(cat.first).c_str(), maybeQuote(rule.first).c_str()));
            }
        }
    }

    return vectorCompletion(partial, commands, possible);
}

bool SpeedrunTimer::AddRuleToCategory(std::string category, std::string rule)
{
    auto cat = lookupMap(g_categories, category);
    if (!cat) {
        console->Print("Category %s does not exist\n", category.c_str());
        return false;
    }

    if (!lookupMap(g_rules, rule)) {
        console->Print("Rule %s does not exist\n", rule.c_str());
        return false;
    }

    cat->rules.insert(rule);
    return true;
}

CON_COMMAND_F_COMPLETION(sar_speedrun_category_add_rule, "sar_speedrun_category_add_rule <category> <rule> - add a rule to a speedrun category.\n", 0, &_sar_speedrun_category_add_rule_completion)
{
    if (args.ArgC() != 3) {
        return console->Print(sar_speedrun_category_add_rule.ThisPtr()->m_pszHelpString);
    }

    SpeedrunTimer::AddRuleToCategory(args[1], args[2]);
}

static int _sar_speedrun_category_remove_rule_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::vector<std::string> possible;

    for (auto cat : g_categories) {
        for (auto rule : cat.second.rules) {
            possible.push_back(Utils::ssprintf("sar_speedrun_category_remove_rule %s %s", maybeQuote(cat.first).c_str(), maybeQuote(rule).c_str()));
        }
    }

    return vectorCompletion(partial, commands, possible);
}

CON_COMMAND_F_COMPLETION(sar_speedrun_category_remove_rule, "sar_speedrun_category_remove_rule <category> <rule> - remove a rule from a speedrun category.\n", 0, &_sar_speedrun_category_remove_rule_completion)
{
    if (args.ArgC() != 3) {
        return console->Print(sar_speedrun_category_add_rule.ThisPtr()->m_pszHelpString);
    }

    auto cat = lookupMap(g_categories, args[1]);
    if (!cat) {
        return console->Print("Category %s does not exist\n", args[1]);
    }

    cat->rules.erase(args[2]);
}

// }}}

// Rule creation/deletion {{{

bool SpeedrunTimer::CreateRule(std::string name, std::string type, std::map<std::string, std::string> params)
{
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
        type == "entity" ? EntityInputRule::Create(params) :
        type == "zone" ? ZoneTriggerRule::Create(params) :
        type == "portal" ? PortalPlacementRule::Create(params) :
        type == "flags" ? ChallengeFlagsRule::Create(params) :
        type == "fly" ? CrouchFlyRule::Create(params) :
        type == "load" ? MapLoadRule::Create(params) :
        std::optional<SpeedrunRule>{};

    if (!rule) {
        console->Print("Failed to create rule\n");
        return false;
    }

    rule->action = action;
    rule->map = params["map"];

    auto after = lookupMap(params, "after");
    if (!after) {
        rule->onlyAfter = {};
    } else {
        rule->onlyAfter = *after;
    }

    auto slotStr = lookupMap(params, "player");
    if (slotStr && type != "load") {
        rule->slot = atoi(slotStr->c_str());
    }

    rule->fired = false;

    g_rules.insert({name, *rule});

    return true;
}

CON_COMMAND(sar_speedrun_rule_create, "sar_speedrun_rule_create <name> <type> [option=value]... - create a speedrun rule with the given name, type, and options.\n")
{
    if (args.ArgC() < 3) {
        return console->Print(sar_speedrun_rule_create.ThisPtr()->m_pszHelpString);
    }

    std::map<std::string, std::string> params;

    for (size_t i = 3; i < args.ArgC(); ++i) {
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

static int _sar_speedrun_rule_remove_completion(const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    std::vector<std::string> possible;

    for (auto rule : g_rules) {
        possible.push_back("sar_speedrun_rule_remove " + maybeQuote(rule.first));
    }

    return vectorCompletion(partial, commands, possible);
}


CON_COMMAND_F_COMPLETION(sar_speedrun_rule_remove, "sar_speedrun_rule_remove <rule> - delete the given speedrun rule.\n", 0, &_sar_speedrun_rule_remove_completion)
{
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
        if (cat.second.rules.find(ruleName) != cat.second.rules.end()) {
            console->Print("Cannot delete rule %s as it exists in the category %s\n", ruleName.c_str(), cat.first.c_str());
            canDelete = false;
        }
    }

    if (canDelete) {
        g_rules.erase(ruleName);
    }
}

// }}}

CON_COMMAND(sar_speedrun_reset_categories, "sar_speedrun_reset_categories - delete all custom categories and rules, reverting to the builtin ones.\n")
{
    if (args.ArgC() != 2 || std::string(args[1]) != "yes") {
        console->Print("WARNING: this will delete all custom categories! Run 'sar_speedrun_reset_categories yes' to confirm.\n");
        return;
    }

    SpeedrunTimer::InitCategories();
}
