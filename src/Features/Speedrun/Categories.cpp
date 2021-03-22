#include "Categories.hpp"
#include "SpeedrunTimer.hpp"

#include "Modules/Engine.hpp"

#include <memory>
#include <set>

template<typename V>
static inline V *lookupMap(std::map<std::string, V> &m, std::string k)
{
    auto search = m.find(k);
    if (search == m.end()) {
        return nullptr;
    }
    return &search->second;
}

enum class RuleAction
{
    START, // Only start if not already running
    FORCE_START, // Restart timer if already running
    STOP,
    SPLIT,
    PAUSE,
    RESUME,
};

static const char *printRuleAction(RuleAction action)
{
    switch (action) {
    case RuleAction::START:
        return "start";
    case RuleAction::FORCE_START:
        return "force-start";
    case RuleAction::STOP:
        return "stop";
    case RuleAction::SPLIT:
        return "split";
    case RuleAction::PAUSE:
        return "pause";
    case RuleAction::RESUME:
        return "resume";
    }
    return "(unknown)";
}

// Rule types {{{

// Base rule type {{{

struct SpeedrunRule
{
    RuleAction action;
    std::string map;
    std::string onlyAfter;
    
    bool fired;

    virtual bool TestTick()
    {
        return false;
    }

    virtual bool TestInput(std::string targetname, std::string classname, std::string inputname, std::string parameter, OutputType caller)
    {
        return false;
    }

    virtual std::string Describe()
    {
        std::string s = std::string("action='") + printRuleAction(this->action) + "'";
        s += " map='" + this->map + "'";
        if (this->onlyAfter != "") {
            s += " after='" + this->onlyAfter + "'";
        }
        return s;
    }

    SpeedrunRule(RuleAction action, std::string map, std::string onlyAfter)
        : action(action)
        , map(map)
        , onlyAfter(onlyAfter)
        , fired(false)
    {
    }

    SpeedrunRule()
        : fired(false)
    {
    }
};

// }}}

// EntityInputRule {{{

#define ENTRULE_TARGETNAME (1 << 0)
#define ENTRULE_CLASSNAME (1 << 1)
#define ENTRULE_PARAMETER (1 << 2)

struct EntityInputRule : public SpeedrunRule
{
    int typeMask;
    std::string targetname;
    std::string classname;
    std::string inputname;
    std::string parameter;

    bool TestInput(std::string targetname, std::string classname, std::string inputname, std::string parameter, OutputType caller) override
    {
        if ((this->typeMask & ENTRULE_TARGETNAME) && targetname != this->targetname) return false;
        if ((this->typeMask & ENTRULE_CLASSNAME) && classname != this->classname) return false;
        if (inputname != this->inputname) return false;
        if ((this->typeMask & ENTRULE_PARAMETER) && parameter != this->parameter) return false;
        return true;
    }

    std::string Describe() override
    {
        std::string s = "[entity] " + SpeedrunRule::Describe();
        if (this->typeMask & ENTRULE_TARGETNAME) {
            s += " targetname='" + this->targetname + "'";
        }
        if (this->typeMask & ENTRULE_CLASSNAME) {
            s += " classname='" + this->classname + "'";
        }
        s += " inputname='" + this->inputname + "'";
        if (this->typeMask & ENTRULE_PARAMETER) {
            s += " parameter='" + this->parameter + "'";
        }
        return s;
    }

    static std::shared_ptr<EntityInputRule> Create(std::map<std::string, std::string> params)
    {
        std::string *targetname = lookupMap(params, "targetname");
        std::string *classname = lookupMap(params, "classname");
        std::string *inputname = lookupMap(params, "inputname");
        std::string *parameter = lookupMap(params, "parameter");

        auto rule = std::make_shared<EntityInputRule>();

        if (targetname) {
            rule->targetname = *targetname;
            rule->typeMask |= ENTRULE_TARGETNAME;
        }

        if (classname) {
            rule->classname = *classname;
            rule->typeMask |= ENTRULE_CLASSNAME;
        }

        if (!inputname) {
            console->Print("inputname not specified\n");
            return nullptr;
        }
        rule->inputname = *inputname;

        if (parameter) {
            rule->parameter = *parameter;
            rule->typeMask |= ENTRULE_PARAMETER;
        }

        return rule;
    }

    EntityInputRule()
        : SpeedrunRule()
        , typeMask(0)
    {
    }

    EntityInputRule(RuleAction action, std::string map, std::string onlyAfter, int typeMask, std::string targetname, std::string classname, std::string inputname, std::string parameter)
        : SpeedrunRule(action, map, onlyAfter)
        , typeMask(typeMask)
        , targetname(targetname)
        , classname(classname)
        , inputname(inputname)
        , parameter(parameter)
    {
    }

    // The most common cases

    EntityInputRule(RuleAction action, std::string map, std::string targetname, std::string inputname)
        : EntityInputRule(action, map, "", ENTRULE_TARGETNAME, targetname, "", inputname, "")
    {
    }

    EntityInputRule(RuleAction action, std::string map, std::string targetname, std::string inputname, std::string parameter)
        : EntityInputRule(action, map, "", ENTRULE_TARGETNAME | ENTRULE_PARAMETER, targetname, "", inputname, parameter)
    {
    }
};

// }}}

// ZoneTriggerRule {{{

struct ZoneTriggerRule : public SpeedrunRule
{
    Vector center;
    Vector size;
    float rotation;

    bool TestTick()
    {
        // TODO
        return false;
    }

    static std::shared_ptr<ZoneTriggerRule> Create(std::map<std::string, std::string> params)
    {
        // TODO
        return nullptr;
    }
};

// }}}

// PortalPlacementRule TODO {{{

struct PortalPlacementRule : public SpeedrunRule
{
    static std::shared_ptr<PortalPlacementRule> Create(std::map<std::string, std::string> params)
    {
        // TODO
        return nullptr;
    }
};

// }}}

// }}}

struct SpeedrunCategory
{
    std::set<std::string> rules;
};

static std::string g_currentCategory = "singleplayer";

// Pre-set rules and categories {{{

static std::map<std::string, SpeedrunCategory> g_categories = {
    { "singleplayer", { { "container-ride-start", "container-vault-start", "moon-shot" } } },
    { "coop-any-amc", { { "coop-start", "coop-course5-end" } } },
    { "coop-ac", { { "coop-start", "coop-course6-end" } } },
};

// It's important that this map stores pointers rather than plain rules
// due to object slicing.
static std::map<std::string, std::shared_ptr<SpeedrunRule>> g_rules = {
    {
        "container-ride-start",
        std::make_shared<EntityInputRule>(
            RuleAction::START,
            "sp_a1_intro1",
            "camera_intro",
            "TeleportToView"
        ),
    },
    {
        "container-vault-start",
        std::make_shared<EntityInputRule>(
            RuleAction::START,
            "sp_a1_intro1",
            "camera_1",
            "TeleportPlayerToProxy"
        ),
    },
    {
        "moon-shot",
        std::make_shared<EntityInputRule>(
            RuleAction::STOP,
            "sp_a4_finale4",
            "@glados",
            "RunScriptCode",
            "BBPortalPlaced()"
        ),
    },
    {
        "coop-start",
        std::make_shared<EntityInputRule>(
            RuleAction::START,
            "mp_coop_start",
            "teleport_start",
            "Enable"
        ),
    },
    {
        "coop-course5-end",
        std::make_shared<EntityInputRule>(
            RuleAction::STOP,
            "mp_coop_paint_longjump_intro",
            "vault-movie_outro",
            "PlayMovieForAllPlayers"
        ),
    },
    {
        "coop-course6-end",
        std::make_shared<EntityInputRule>(
            RuleAction::STOP,
            "mp_coop_paint_crazy_box",
            "movie_outro",
            "PlayMovieForAllPlayers"
        ),
    },
};

// }}}

// Helper functions {{{

static void dispatchRule(std::string name, std::shared_ptr<SpeedrunRule> rule)
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

static std::shared_ptr<SpeedrunRule> getRule(std::string name)
{
    auto search = g_rules.find(name);
    if (search == g_rules.end()) {
        return nullptr;
    }

    return search->second;
}

// }}}

// Testing rules {{{

void SpeedrunTimer::TestInputRules(std::string targetname, std::string classname, std::string inputname, std::string parameter, OutputType caller)
{
    for (std::string ruleName : g_categories[g_currentCategory].rules) {
        auto rule = getRule(ruleName);
        if (!rule) continue;

        if (rule->fired) continue;

        if (rule->onlyAfter != "") {
            auto prereq = getRule(rule->onlyAfter);
            if (!prereq) continue;
            if (!prereq->fired) continue;
        }

        if (strcmp(engine->m_szLevelName, rule->map.c_str())) continue;
        if (!rule->TestInput(targetname, classname, inputname, parameter, caller)) continue;

        dispatchRule(ruleName, rule);

        rule->fired = true;
    }
}

void SpeedrunTimer::TestTickRules()
{
    for (std::string ruleName : g_categories[g_currentCategory].rules) {
        auto rule = getRule(ruleName);
        if (!rule) continue;

        if (rule->fired) continue;

        if (rule->onlyAfter != "") {
            auto prereq = getRule(rule->onlyAfter);
            if (!prereq) continue;
            if (!prereq->fired) continue;
        }

        if (strcmp(engine->m_szLevelName, rule->map.c_str())) continue;
        if (!rule->TestTick()) continue;

        dispatchRule(ruleName, rule);

        rule->fired = true;
    }
}

// }}}

void SpeedrunTimer::ResetCategory()
{
    for (std::string ruleName : g_categories[g_currentCategory].rules) {
        auto rule = getRule(ruleName);
        if (!rule) continue;
        rule->fired = false;
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
            console->Print("%s %s\n", rule.first.c_str(), rule.second->Describe().c_str());
        }
    } else if (args.ArgC() == 2) {
        auto rule = getRule(args[1]);
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

    if (getRule(name)) {
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

    std::shared_ptr<SpeedrunRule> rule = nullptr;

    if (type == "entity") {
        rule = EntityInputRule::Create(params);
    } else if (type == "zone") {
        rule = ZoneTriggerRule::Create(params);
    } else if (type == "portal") {
        rule = PortalPlacementRule::Create(params);
    } else {
        return console->Print("Unknown rule type %s\n", args[2]);
    }

    if (!rule) {
        return console->Print("Failed to create rule\n");
    }

    rule->action = action;
    rule->map = params["map"];
    
    auto after = lookupMap(params, "after");
    if (!after) {
        rule->onlyAfter = "";
    } else {
        rule->onlyAfter = *after;
    }

    rule->fired = false;

    g_rules[name] = rule;
}

CON_COMMAND(sar_speedrun_rule_remove, "sar_speedrun_rule_remove <rule> - delete the given speedrun rule.\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_speedrun_rule_remove.ThisPtr()->m_pszHelpString);
    }

    std::string ruleName = args[1];

    auto rule = getRule(ruleName);
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
