#pragma once

#include "Command.hpp"
#include "Rules.hpp"

#include <map>
#include <string>
#include <vector>

struct SpeedrunCategory {
	std::vector<std::string> rules;
};

namespace SpeedrunTimer {
	bool TestInputRules(std::string targetname, std::string classname, std::string inputname, std::string parameter, std::optional<int> triggerSlot);
	void TestZoneRules(Vector pos, int slot);
	bool TestPortalRules(Vector pos, int slot, PortalColor portal);
	bool TestFlagRules(int slot);
	bool TestFlyRules(int slot);
	void TestLoadRules();
	void ResetCategory();
	void InitCategories();
	SpeedrunRule *GetRule(std::string name);

	bool CreateCategory(std::string name);
	bool AddRuleToCategory(std::string category, std::string rule);
	bool CreateRule(std::string name, std::string type, std::map<std::string, std::string> params);
	std::vector<std::string> GetCategoryRules();
	std::string GetCategoryName();
};  // namespace SpeedrunTimer

extern Command sar_speedrun_category;
