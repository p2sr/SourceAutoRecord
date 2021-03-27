#pragma once

#include <string>
#include <variant>
#include <optional>
#include <map>
#include "Utils/Math.hpp"

enum class RuleAction
{
    START, // Only start if not already running
    FORCE_START, // Restart timer if already running
    STOP,
    SPLIT,
    PAUSE,
    RESUME,
};

enum PortalColor
{
    BLUE,
    ORANGE,
};

class SpeedrunRule;

#define ENTRULE_TARGETNAME (1 << 0)
#define ENTRULE_CLASSNAME (1 << 1)
#define ENTRULE_PARAMETER (1 << 2)

struct EntityInputRule {
    int typeMask;
    std::string targetname;
    std::string classname;
    std::string inputname;
    std::string parameter;

    bool Test(std::string targetname, std::string classname, std::string inputname, std::string parameter);

    static std::optional<SpeedrunRule> Create(std::map<std::string, std::string> params);
};

struct ZoneTriggerRule {
    Vector center;
    Vector size;
    double rotation;

    bool Test(Vector pos);

    static std::optional<SpeedrunRule> Create(std::map<std::string, std::string> params);
};

struct PortalPlacementRule {
    Vector center;
    Vector size;
    double rotation;
    std::optional<PortalColor> portal;

    bool Test(Vector pos, PortalColor portal);

    static std::optional<SpeedrunRule> Create(std::map<std::string, std::string> params);
};

struct SpeedrunRule {
    RuleAction action;

    std::string map;
    std::string onlyAfter;
    std::optional<int> slot;
    std::variant<EntityInputRule, ZoneTriggerRule, PortalPlacementRule> rule;

    bool fired;

    std::string Describe();

    SpeedrunRule(RuleAction action, std::string map, std::variant<EntityInputRule, ZoneTriggerRule, PortalPlacementRule> rule)
        : action(action)
        , map(map)
        , onlyAfter("")
        , slot()
        , rule(rule)
        , fired(false)
    {
    }

    bool TestGeneral(std::optional<int> slot);
};
