#pragma once
#include "Modules/Client.hpp"

#include "Features/Stats.hpp"

#include "Command.hpp"

CON_COMMAND(sar_stats_jump, "Prints jump stats.\n")
{
    std::string type;
    if (stats->jumps->type == ResultType::VEC2) {
        type = std::string(" (vec2)");
    } else if (stats->jumps->type == ResultType::VEC3) {
        type = std::string(" (vec2)");
    }

    console->Print("Distance: %.3f\n", stats->jumps->distance);
    console->Print("Peak: %.3f %s\n", stats->jumps->distancePeak, type.c_str());
    console->Print("Jumps: %i\n", stats->jumps->total);
}

CON_COMMAND(sar_stats_steps, "Prints total amount of steps.\n")
{
    console->Print("Steps: %i\n", stats->jumps->total);
}

CON_COMMAND(sar_stats_velocity, "Prints velocity stats.\n")
{
    auto current = Client::GetLocalVelocity();

    std::string type;
    if (stats->velocity->type == ResultType::VEC2) {
        type = std::string(" (vec2)");
    } else if (stats->velocity->type == ResultType::VEC3) {
        type = std::string(" (vec2)");
    }

    console->Print("Current: %.3f/%.3f (vec2/vec3)", current.Length2D(), current.Length());
    console->Print("Peak: %.3f %s\n", stats->velocity->peak, type.c_str());
}

CON_COMMAND(sar_stats_jumps_reset, "Resets total jump count and jump distance peak.\n")
{
    stats->jumps->Reset();
}

CON_COMMAND(sar_stats_steps_reset, "Resets total step count.\n")
{
    stats->steps->Reset();
}

CON_COMMAND(sar_stats_velocity_reset, "Resets velocity peak.\n")
{
    stats->velocity->Reset();
}

CON_COMMAND(sar_stats_reset, "Resets all saved stats.\n")
{
    stats->ResetAll();
}
