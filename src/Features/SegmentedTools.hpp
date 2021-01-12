#pragma once

#include "Feature.hpp"
#include "Command.hpp"

class SegmentedTools : public Feature {

public:
    int waitTick;
    std::string pendingCommands;

public:
    SegmentedTools();

};

extern SegmentedTools* segmentedTools;
