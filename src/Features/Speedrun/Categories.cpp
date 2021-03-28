#include "Categories.hpp"
#include "SpeedrunTimer.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Hud/Hud.hpp"

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

struct SpeedrunCategory
{
    std::set<std::string> rules;
};

// Pre-set rules and categories {{{

static std::string g_currentCategory = "singleplayer";

static std::map<std::string, SpeedrunCategory> g_categories = {
    { "singleplayer", { { "container-ride-start", "container-vault-start", "moon-shot" } } },
    { "coop-any-amc", { { "coop-start", "coop-course5-end" } } },
    { "coop-ac", { { "coop-start", "coop-course6-end" } } },
};

// It's important that this map stores pointers rather than plain rules
// due to object slicing.
static std::map<std::string, SpeedrunRule> g_rules = {
    {
        "container-ride-start",
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
        "container-vault-start",
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
        "moon-shot",
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
        "coop-start",
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
        "coop-course5-end",
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
        "coop-course6-end",
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
        SpeedrunTimer::Stop();
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
    for (std::string ruleName : g_categories[g_currentCategory].rules) {
        auto rule = SpeedrunTimer::GetRule(ruleName);
        if (!rule) continue;
        if (!std::holds_alternative<RuleType>(rule->rule)) continue;
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
        if (rule->map != engine->m_szLevelName) continue;
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
            if (rule->map != engine->m_szLevelName) continue;
            if (std::holds_alternative<ZoneTriggerRule>(rule->rule)) {
                std::get<ZoneTriggerRule>(rule->rule).OverlayInfo(ctx, rule);
            } else if (std::holds_alternative<PortalPlacementRule>(rule->rule)) {
                std::get<PortalPlacementRule>(rule->rule).OverlayInfo(ctx, rule);
            }
        }
    }
}

// Setting/printing categories {{{

CON_COMMAND(sar_speedrun_category, "sar_speedrun_category [category] - get or set the speedrun category.\n")
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

CON_COMMAND(sar_speedrun_rule, "sar_speedrun_rule [rule] - show information about speedrun rules.\n")
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

CON_COMMAND(sar_speedrun_category_remove, "sar_speedrun_category_remove <category> - delete the given speedrun category.\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_speedrun_category_remove.ThisPtr()->m_pszHelpString);
    }

    std::string catName = args[1];

    if (lookupMap(g_categories, catName)) {
        return console->Print("Category %s does not exist\n", args[1]);
    }

    if (g_currentCategory == catName) {
        return console->Print("Cannot delete the current category\n");
    }

    g_categories.erase(catName);
}

// }}}

// Category rule management {{{

CON_COMMAND(sar_speedrun_category_add_rule, "sar_speedrun_category_add_rule <category> <rule> - add a rule to a speedrun category.\n")
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

CON_COMMAND(sar_speedrun_category_remove_rule, "sar_speedrun_category_remove_rule <category> <rule> - remove a rule from a speedrun category.\n")
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
    } else if (*actionStr == "force-start") {
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
    if (slotStr) {
        rule->slot = atoi(slotStr->c_str());
    }

    rule->fired = false;

    g_rules.insert({name, *rule});
}

CON_COMMAND(sar_speedrun_rule_remove, "sar_speedrun_rule_remove <rule> - delete the given speedrun rule.\n")
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
