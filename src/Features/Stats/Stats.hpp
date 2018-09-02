#pragma once
#include "Features/Feature.hpp"

#include "JumpStats.hpp"
#include "StepStats.hpp"
#include "VelocityStats.hpp"

#include "Variable.hpp"

class Stats : public Feature {
public:
    JumpStats* jumps;
    StepStats* steps;
    VelocityStats* velocity;

public:
    Stats();
    ~Stats();
    void ResetAll();
};

extern Stats* stats;

extern Variable sar_stats_jumps_xy;
extern Variable sar_stats_velocity_peak_xy;
extern Variable sar_stats_auto_reset;
