#pragma once

#include <vector>
#include <string>

#include "Command.hpp"

enum class OutputType {
    ORANGE,
    BLUE,
    CHELL,
    OTHER,
};

namespace SpeedrunTimer {
    void TestInputRules(std::string targetname, std::string classname, std::string inputname, std::string parameter, OutputType caller);
    void TestTickRules();
    void ResetCategory();
};

extern Command sar_speedrun_category;
