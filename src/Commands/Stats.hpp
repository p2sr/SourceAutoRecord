#pragma once
#include "Modules/Client.hpp"

#include "Features/Stats.hpp"

#include "Command.hpp"

CON_COMMAND(sar_stats_jump, "Prints jump stats.\n")
{
    std::string type;
    if (Stats::Jumps::Type == Stats::ResultType::VEC2) {
        type = std::string(" (vec2)");
    } else if (Stats::Jumps::Type == Stats::ResultType::VEC3) {
        type = std::string(" (vec2)");
    }

    console->Print("Distance: %.3f\n", Stats::Jumps::Distance);
    console->Print("Peak: %.3f %s\n", Stats::Jumps::DistancePeak, type.c_str());
    console->Print("Jumps: %i\n", Stats::Jumps::Total);
}

CON_COMMAND(sar_stats_steps, "Prints total amount of steps.\n")
{
    console->Print("Steps: %i\n", Stats::Jumps::Total);
}

CON_COMMAND(sar_stats_velocity, "Prints velocity stats.\n")
{
    auto current = Client::GetLocalVelocity();

    std::string type;
    if (Stats::Velocity::Type == Stats::ResultType::VEC2) {
        type = std::string(" (vec2)");
    } else if (Stats::Velocity::Type == Stats::ResultType::VEC3) {
        type = std::string(" (vec2)");
    }

    console->Print("Current: %.3f/%.3f (vec2/vec3)", current.Length2D(), current.Length());
    console->Print("Peak: %.3f %s\n", Stats::Velocity::Peak, type.c_str());
}

CON_COMMAND(sar_stats_jumps_reset, "Resets total jump count and jump distance peak.\n")
{
    Stats::Jumps::Reset();
}

CON_COMMAND(sar_stats_steps_reset, "Resets total step count.\n")
{
    Stats::Steps::Reset();
}

CON_COMMAND(sar_stats_velocity_reset, "Resets velocity peak.\n")
{
    Stats::Velocity::Reset();
}

CON_COMMAND(sar_stats_reset, "Resets all saved stats.\n")
{
    Stats::ResetAll();
}
