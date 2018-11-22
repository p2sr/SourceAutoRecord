#include "Stats.hpp"

#include "JumpStats.hpp"
#include "StatsResultType.hpp"
#include "StepStats.hpp"
#include "VelocityStats.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"

#include "Command.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

Variable sar_stats_jumps_xy("sar_stats_jumps_xy", "0", "Saves jump distance as 2D vector.\n");
Variable sar_stats_velocity_peak_xy("sar_stats_velocity_peak_xy", "0", "Saves velocity peak as 2D vector.\n");
Variable sar_stats_auto_reset("sar_stats_auto_reset", "0", 0, "Resets all stats automatically.\n"
                                                              "0 = default,\n"
                                                              "1 = restart or disconnect only,\n"
                                                              "2 = any load & sar_timer_start.\n"
                                                              "Note: Portal counter is not part of the \"stats\" feature.\n");

Stats* stats;

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

// Commmands

CON_COMMAND(sar_stats_jump, "Prints jump stats.\n")
{
    std::string type;
    if (stats->jumps->type == StatsResultType::VEC2) {
        type = std::string(" (vec2)");
    } else if (stats->jumps->type == StatsResultType::VEC3) {
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
    auto current = client->GetLocalVelocity();

    std::string type;
    if (stats->velocity->type == StatsResultType::VEC2) {
        type = std::string(" (vec2)");
    } else if (stats->velocity->type == StatsResultType::VEC3) {
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
