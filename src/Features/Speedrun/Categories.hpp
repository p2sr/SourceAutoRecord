#pragma once

#include <vector>
#include <string>
#include <map>

#include "Command.hpp"
#include "Rules.hpp"

namespace SpeedrunTimer {
    void TestInputRules(std::string targetname, std::string classname, std::string inputname, std::string parameter, std::optional<int> triggerSlot);
    void TestZoneRules(Vector pos, int slot);
    void TestPortalRules(Vector pos, int slot, PortalColor portal);
    void TestFlagRules(int slot);
    void ResetCategory();
    void DrawTriggers();
    SpeedrunRule *GetRule(std::string name);
};

extern Command sar_speedrun_category;
