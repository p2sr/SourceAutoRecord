#pragma once
#include "Modules/Client.hpp"

#include "Features/Stats.hpp"

namespace Callbacks {

void PrintJumpStats()
{
    std::string type;
    if (Stats::Jumps::Type == Stats::ResultType::VEC2) {
        type = std::string(" (vec2)");
    } else if (Stats::Jumps::Type == Stats::ResultType::VEC3) {
        type = std::string(" (vec2)");
    }

    Console::Print("Distance: %.3f\n", Stats::Jumps::Distance);
    Console::Print("Peak: %.3f %s\n", Stats::Jumps::DistancePeak, type.c_str());
    Console::Print("Jumps: %i\n", Stats::Jumps::Total);
}
void PrintStepStats()
{
    Console::Print("Steps: %i\n", Stats::Jumps::Total);
}
void PrintVelocityStats()
{
    auto current = Client::GetLocalVelocity();

    std::string type;
    if (Stats::Velocity::Type == Stats::ResultType::VEC2) {
        type = std::string(" (vec2)");
    } else if (Stats::Velocity::Type == Stats::ResultType::VEC3) {
        type = std::string(" (vec2)");
    }

    Console::Print("Current: %.3f/%.3f (vec2/vec3)", current.Length2D(), current.Length());
    Console::Print("Peak: %.3f %s\n", Stats::Velocity::Peak, type.c_str());
}
void ResetJumpStats()
{
    Stats::Jumps::Reset();
}
void ResetStepStats()
{
    Stats::Steps::Reset();
}
void ResetVelocityStats()
{
    Stats::Velocity::Reset();
}
void ResetAllStats()
{
    Stats::ResetAll();
}
}