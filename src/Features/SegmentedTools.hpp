#pragma once

#include "Feature.hpp"
#include "Command.hpp"
#include "Variable.hpp"

class SegmentedTools : public Feature {

public:
    int waitTick;
    std::string pendingCommands;

public:
    SegmentedTools();

};

extern SegmentedTools* segmentedTools;

extern Command waitCmd;
extern Variable wait_mode;
extern Variable wait_persist_across_loads;
