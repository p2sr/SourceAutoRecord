#include "Stats.hpp"

#include "Utils.hpp"

void JumpStats::StartTrace(Vector source)
{
    this->source = source;
    this->isTracing = true;
}
void JumpStats::EndTrace(Vector destination, bool xyOnly)
{
    auto x = destination.x - this->source.x;
    auto y = destination.y - this->source.y;

    if (xyOnly) {
        this->distance = std::sqrt(x * x + y * y);
        this->type = ResultType::VEC2;
    } else {
        auto z = destination.z - source.z;
        this->distance = std::sqrt(x * x + y * y + z * z);
        this->type = ResultType::VEC3;
    }

    if (this->distance > this->distancePeak)
        distancePeak = distance;

    this->isTracing = false;
}
void JumpStats::Reset()
{
    this->total = 0;
    this->distance = 0;
    this->distancePeak = 0;
    this->type = ResultType::UNKNOWN;
    this->isTracing = false;
}
void StepStats::Reset()
{
    this->total = 0;
}
void VelocityStats::Save(Vector velocity, bool xyOnly)
{
    float vel = 0;
    if (xyOnly) {
        vel = velocity.Length2D();
        this->type = ResultType::VEC2;
    } else {
        vel = velocity.Length();
        this->type = ResultType::VEC3;
    }

    if (vel > this->peak)
        this->peak = vel;
}
void VelocityStats::Reset()
{
    this->peak = 0;
    this->type = ResultType::UNKNOWN;
}

Stats::Stats()
{
    this->jumps = new JumpStats();
    this->steps = new StepStats();
    this->velocity = new VelocityStats();
}
Stats::~Stats()
{
    SAFE_DELETE(this->jumps);
    SAFE_DELETE(this->steps);
    SAFE_DELETE(this->velocity);
}
void Stats::ResetAll()
{
    this->jumps->Reset();
    this->steps->Reset();
    this->velocity->Reset();
}

Stats* stats;
