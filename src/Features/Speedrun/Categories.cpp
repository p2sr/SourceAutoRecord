#include "Categories.hpp"
#include "SpeedrunTimer.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Hud/Toasts.hpp"
#include "Utils.hpp"

#include <memory>
#include <set>
#include <variant>
#include <cstdlib>

Variable sar_speedrun_draw_triggers("sar_speedrun_draw_triggers", "0", "Draw the triggers associated with speedrun rules in the world.\n");
Variable sar_speedrun_notify_duration("sar_speedrun_notify_duration", "6", "Number of seconds to show the speedrun notification on-screen for.\n");

static int g_notifyR = 255;
static int g_notifyG = 255;
static int g_notifyB = 255;

CON_COMMAND(sar_speedrun_notify_set_color, "sar_speedrun_notify_set_color <hex code> - sets the speedrun notification color to the specified sRGB color code.\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_speedrun_notify_set_color.ThisPtr()->m_pszHelpString);
    }

    const char *color = args[1];
    if (color[0] == '#') {
        ++color;
    }

    int r, g, b;
    int end;
    if (sscanf(color, "%2x%2x%2x%n", &r, &g, &b, &end) != 3 || end != 6) {
        return console->Print("Invalid color code!\n");
    }

    g_notifyR = Utils::ConvertFromSrgb(r);
    g_notifyG = Utils::ConvertFromSrgb(g);
    g_notifyB = Utils::ConvertFromSrgb(b);
}

template<typename V>
static inline V *lookupMap(std::map<std::string, V> &m, std::string k)
{
    auto search = m.find(k);
    if (search == m.end()) {
        return nullptr;
    }
    return &search->second;
}

struct SpeedrunCategory
{
    std::set<std::string> rules;
};

// Pre-set rules and categories {{{

static std::string g_currentCategory = "Singleplayer";

static std::map<std::string, SpeedrunCategory> g_categories = {
    { "Singleplayer", { { "Container Ride Start", "Vault Start", "Moon Shot" } } },
    { "Coop", { { "Coop Start", "Coop Course 5 End" } } },
    { "Coop AC", { { "Coop Start", "Coop Course 6 End" } } },
};

// It's important that this map stores pointers rather than plain rules
// due to object slicing.
static std::map<std::string, SpeedrunRule> g_rules = {
    {
        "Container Ride Start",
        SpeedrunRule(
            RuleAction::START,
            "sp_a1_intro1",
            EntityInputRule{
                ENTRULE_TARGETNAME,
                "camera_intro",
                "",
                "TeleportToView",
                "",
            }
        ),
    },
    {
        "Vault Start",
        SpeedrunRule(
            RuleAction::START,
            "sp_a1_intro1",
            EntityInputRule{
                ENTRULE_TARGETNAME,
                "camera_1",
                "",
                "TeleportPlayerToProxy",
                "",
            }
        ),
    },
    {
        "Moon Shot",
        SpeedrunRule(
            RuleAction::STOP,
            "sp_a4_finale4",
            EntityInputRule{
                ENTRULE_TARGETNAME | ENTRULE_PARAMETER,
                "@glados",
                "",
                "RunScriptCode",
                "BBPortalPlaced()",
            }
        ),
    },
    {
        "Coop Start",
        SpeedrunRule(
            RuleAction::START,
            "mp_coop_start",
            EntityInputRule{
                ENTRULE_TARGETNAME,
                "teleport_start",
                "",
                "Enable",
                "",
            }
        ),
    },
    {
        "Coop Course 5 End",
        SpeedrunRule(
            RuleAction::STOP,
            "mp_coop_paint_longjump_intro",
            EntityInputRule{
                ENTRULE_TARGETNAME,
                "vault-movie_outro",
                "",
                "PlayMovieForAllPlayers",
                "",
            }
        ),
    },
    {
        "Coop Course 6 End",
        SpeedrunRule(
            RuleAction::STOP,
            "mp_coop_paint_crazy_box",
            EntityInputRule{
                ENTRULE_TARGETNAME,
                "movie_outro",
                "",
                "PlayMovieForAllPlayers",
                "",
            }
        ),
    },
};


// }}}

// Testing rules {{{

static void dispatchRule(std::string name, SpeedrunRule *rule)
{
    if (sar_speedrun_notify_duration.GetFloat() > 0 && rule->action == RuleAction::SPLIT) {
        float totalTime = SpeedrunTimer::GetTotalTicks() * *engine->interval_per_tick;
        float splitTime = SpeedrunTimer::GetSplitTicks() * *engine->interval_per_tick;
        std::string text = Utils::ssprintf("%s\n%s (%s)", name.c_str(), SpeedrunTimer::Format(totalTime).c_str(), SpeedrunTimer::Format(splitTime).c_str());
        toastHud.AddToast(text, { g_notifyR, g_notifyG, g_notifyB, 255 }, sar_speedrun_notify_duration.GetFloat());
    }

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
    GeneralTestRules<MapLoadRule>(0);
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

CON_COMMAND(sar_speedrun_category_create, "sar_speedrun_category_create <category> - create a new speedrun category with the given name.\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_speedrun_category_create.ThisPtr()->m_pszHelpString);
    }

    std::string catName = args[1];

    if (catName == "") {
        return console->Print("Category name cannot be empty\n");
    }

    if (lookupMap(g_categories, catName)) {
        return console->Print("Category %s already exists\n", args[1]);
    }

    g_categories[catName] = SpeedrunCategory{ { } };
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

CON_COMMAND_F_COMPLETION(sar_speedrun_category_add_rule, "sar_speedrun_category_add_rule <category> <rule> - add a rule to a speedrun category.\n", 0, &_sar_speedrun_category_add_rule_completion)
{
    if (args.ArgC() != 3) {
        return console->Print(sar_speedrun_category_add_rule.ThisPtr()->m_pszHelpString);
    }

    std::string ruleName = args[2];

    auto cat = lookupMap(g_categories, args[1]);
    if (!cat) {
        return console->Print("Category %s does not exist\n", args[1]);
    }

    if (!lookupMap(g_rules, ruleName)) {
        return console->Print("Rule %s does not exist\n", args[2]);
    }

    cat->rules.insert(ruleName);
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

CON_COMMAND(sar_speedrun_rule_create, "sar_speedrun_rule_create <name> <type> [option=value]... - create a speedrun rule with the given name, type, and options.\n")
{
    if (args.ArgC() < 3) {
        return console->Print(sar_speedrun_rule_create.ThisPtr()->m_pszHelpString);
    }

    std::string name = args[1];
    std::string type = args[2];

    if (name == "") {
        return console->Print("Rule name cannot be empty\n");
    }

    if (SpeedrunTimer::GetRule(name)) {
        return console->Print("Rule %s already exists\n", args[1]);
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

    RuleAction action;

    auto actionStr = lookupMap(params, "action");
    if (!actionStr) {
        return console->Print("action not specified\n");
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
        return console->Print("Unknown action %s\n", actionStr->c_str());
    }

    if (!lookupMap(params, "map")) {
        return console->Print("map not specified\n");
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
        return console->Print("Failed to create rule\n");
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
