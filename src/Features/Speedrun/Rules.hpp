#pragma once

#include "Features/Hud/Hud.hpp"
#include "Utils/Math.hpp"

#include <map>
#include <optional>
#include <string>
#include <variant>

enum class RuleAction {
	START,        // Only start if not already running
	FORCE_START,  // Restart timer if already running
	STOP,
	SPLIT,
	PAUSE,
	RESUME,
};

enum PortalColor {
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
	void DrawInWorld();
	void OverlayInfo(SpeedrunRule *rule);

	static std::optional<SpeedrunRule> Create(std::map<std::string, std::string> params);
};

struct PortalPlacementRule {
	Vector center;
	Vector size;
	double rotation;
	std::optional<PortalColor> portal;

	bool Test(Vector pos, PortalColor portal);
	void DrawInWorld();
	void OverlayInfo(SpeedrunRule *rule);

	static std::optional<SpeedrunRule> Create(std::map<std::string, std::string> params);
};

struct ChallengeFlagsRule {
	static std::optional<SpeedrunRule> Create(std::map<std::string, std::string> params);
	bool Test();
};

struct MapLoadRule {
	static std::optional<SpeedrunRule> Create(std::map<std::string, std::string> params);
	bool Test();
};

struct MapEndRule {
	static std::optional<SpeedrunRule> Create(std::map<std::string, std::string> params);
	bool Test();
};

struct CrouchFlyRule {
	static std::optional<SpeedrunRule> Create(std::map<std::string, std::string> params);
	bool Test();
};

struct SpeedrunRule {
	using _RuleTypes = std::variant<
		EntityInputRule,
		ZoneTriggerRule,
		PortalPlacementRule,
		ChallengeFlagsRule,
		MapLoadRule,
		MapEndRule,
		CrouchFlyRule>;

	RuleAction action;

	std::vector<std::string> maps;
	std::optional<std::string> onlyAfter;
	std::optional<int> slot;
	std::optional<std::pair<int, int>> cycle;
	_RuleTypes rule;

	bool fired;

	std::string Describe();

    SpeedrunRule(RuleAction action, std::string map, _RuleTypes rule)
		: action(action)
		, maps({map})
		, onlyAfter()
		, slot()
		, rule(rule)
		, fired(false) {
	}

	SpeedrunRule(RuleAction action, std::vector<std::string> maps, _RuleTypes rule)
		: action(action)
		, maps(maps)
		, onlyAfter()
		, slot()
		, rule(rule)
		, fired(false) {
	}

	bool TestGeneral(std::optional<int> slot);
};

namespace SpeedrunTimer {
	void TickRules();
};
