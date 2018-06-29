#pragma once
#include "Features/Stats.hpp"

namespace Callbacks {

void ResetJumps()
{
    Stats::TotalJumps = 0;
}
void ResetSteps()
{
    Stats::TotalSteps = 0;
}
void ResetJumpDistance()
{
    Stats::JumpDistance::Reset();
}
}