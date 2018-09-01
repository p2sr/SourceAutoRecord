#include "Stats.hpp"

#include "JumpStats.hpp"
#include "StepStats.hpp"
#include "VelocityStats.hpp"

#include "Utils.hpp"

Stats::Stats()
{
    this->jumps = new JumpStats();
    this->steps = new StepStats();
    this->velocity = new VelocityStats();

    this->hasLoaded = this->jumps && this->steps && this->velocity;
}
Stats::~Stats()
{
    SAFE_DELETE(this->jumps)
    SAFE_DELETE(this->steps)
    SAFE_DELETE(this->velocity)
}
void Stats::ResetAll()
{
    this->jumps->Reset();
    this->steps->Reset();
    this->velocity->Reset();
}

Stats* stats;
