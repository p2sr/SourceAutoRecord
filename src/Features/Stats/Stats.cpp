#include "Stats.hpp"

#include "JumpStats.hpp"
#include "StatsResultType.hpp"
#include "StepStats.hpp"
#include "VelocityStats.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Command.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

Variable sar_stats_jumps_xy("sar_stats_jumps_xy", "0", "Saves jump distance as 2D vector.\n");
Variable sar_stats_velocity_peak_xy("sar_stats_velocity_peak_xy", "0", "Saves velocity peak as 2D vector.\n");
Variable sar_stats_auto_reset("sar_stats_auto_reset", "0", 0, "Resets all stats automatically.\n"
                                                              "0 = Default,\n"
                                                              "1 = Restart or disconnect only,\n"
                                                              "2 = Any load & sar_timer_start.\n"
                                                              "Note: Portal counter is not part of the \"stats\" feature.\n");

Stats* stats;

Stats::Stats()
{
    for (auto i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i) {
        this->playerStats.push_back(new PlayerStats());
    }

    this->hasLoaded = true;
}
Stats::~Stats()
{
    for (auto& stat : this->playerStats) {
        delete stat;
    }
    this->playerStats.clear();
}
PlayerStats* Stats::Get(int nSlot)
{
    return this->playerStats[nSlot];
}
void Stats::ResetAll()
{
    for (auto& stat : this->playerStats) {
        stat->Reset();
    }
}

// Commands

CON_COMMAND(sar_stats_jump, "Prints jump stats.\n")
{
    auto stat = stats->Get(GET_SLOT());

    auto type = std::string();
    if (stat->jumps->type == StatsResultType::VEC2) {
        type = std::string(" (vec2)");
    } else if (stat->jumps->type == StatsResultType::VEC3) {
        type = std::string(" (vec3)");
    }

    console->Print("Distance: %.3f\n", stat->jumps->distance);
    console->Print("Peak: %.3f %s\n", stat->jumps->distancePeak, type.c_str());
    console->Print("Jumps: %i\n", stat->jumps->total);
}
CON_COMMAND(sar_stats_steps, "Prints total amount of steps.\n")
{
    auto stat = stats->Get(GET_SLOT());
    console->Print("Steps: %i\n", stat->jumps->total);
}
CON_COMMAND(sar_stats_velocity, "Prints velocity stats.\n")
{
    auto stat = stats->Get(GET_SLOT());

    std::string type;
    if (stat->velocity->type == StatsResultType::VEC2) {
        type = std::string(" (vec2)");
    } else if (stat->velocity->type == StatsResultType::VEC3) {
        type = std::string(" (vec3)");
    }

    auto player = server->GetPlayer();
    if (player) {
        auto curVel = server->GetLocalVelocity(player);
        console->Print("Current: %.3f/%.3f (vec2/vec3)\n", curVel.Length2D(), curVel.Length());
    }

    console->Print("Peak: %.3f %s\n", stat->velocity->peak, type.c_str());
}
CON_COMMAND(sar_stats_jumps_reset, "Resets total jump count and jump distance peak.\n")
{
    stats->Get(GET_SLOT())->jumps->Reset();
}
CON_COMMAND(sar_stats_steps_reset, "Resets total step count.\n")
{
    stats->Get(GET_SLOT())->steps->Reset();
}
CON_COMMAND(sar_stats_velocity_reset, "Resets velocity peak.\n")
{
    stats->Get(GET_SLOT())->velocity->Reset();
}
CON_COMMAND(sar_stats_reset, "Resets all saved stats.\n")
{
    stats->Get(GET_SLOT())->Reset();
}
