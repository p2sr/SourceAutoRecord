#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Timer.hpp"
#include "Features/TimerAverage.hpp"
#include "Features/TimerCheckPoints.hpp"

#include "Cheats.hpp"
#include "Command.hpp"

CON_COMMAND(sar_timer_start, "Starts timer.\n")
{
    if (Timer::IsRunning)
        console->DevMsg("Restarting timer!\n");
    else
        console->DevMsg("Starting timer!\n");
    Timer::Start(*Engine::tickcount);

    if (sar_stats_auto_reset.GetInt() >= 2) {
        Stats::ResetAll();
    }
}

CON_COMMAND(sar_timer_stop, "Stops timer.\n")
{
    if (!Timer::IsRunning) {
        console->DevMsg("Timer isn't running!\n");
        return;
    }

    Timer::Stop(*Engine::tickcount);

    if (Timer::Average::IsEnabled) {
        int tick = Timer::GetTick(*Engine::tickcount);
        Timer::Average::Add(tick, Engine::ToTime(tick), Engine::m_szLevelName);
    }
}

CON_COMMAND(sar_timer_result, "Prints result of timer.\n")
{
    int tick = Timer::GetTick(*Engine::tickcount);
    float time = Engine::ToTime(tick);

    if (Timer::IsRunning) {
        console->PrintActive("Result: %i (%.3f)\n", tick, time);
    } else {
        console->Print("Result: %i (%.3f)\n", tick, time);
    }
}

CON_COMMAND(sar_avg_start, "Starts calculating the average when using timer.\n")
{
    Timer::Average::Start();
}

CON_COMMAND(sar_avg_stop, "Stops average calculation.\n")
{
    Timer::Average::IsEnabled = false;
}

CON_COMMAND(sar_avg_result, "Prints result of average.\n")
{
    int average = Timer::Average::Items.size();
    if (average > 0) {
        console->Print("Average of %i:\n", average);
    } else {
        console->Print("No result!\n");
        return;
    }

    for (int i = 0; i < average; i++) {
        console->Print("%s -> ", Timer::Average::Items[i].Map);
        console->Print("%i ticks", Timer::Average::Items[i].Ticks);
        console->Print("(%.3f)\n", Timer::Average::Items[i].Time);
    }

    if (Timer::IsRunning) {
        console->PrintActive("Result: %i (%.3f)\n", Timer::Average::AverageTicks, Timer::Average::AverageTime);
    } else {
        console->Print("Result: %i (%.3f)\n", Timer::Average::AverageTicks, Timer::Average::AverageTime);
    }
}

CON_COMMAND(sar_cps_add, "Saves current time of timer.\n")
{
    if (!Timer::IsRunning) {
        console->DevMsg("Timer isn't running!\n");
        return;
    }

    int tick = Timer::GetTick(Engine::GetSessionTick());
    Timer::CheckPoints::Add(tick, Engine::ToTime(tick), Engine::m_szLevelName);
}

CON_COMMAND(sar_cps_clear, "Resets saved times of timer.\n")
{
    Timer::CheckPoints::Reset();
}

CON_COMMAND(sar_cps_result, "Prints result of timer checkpoints.\n")
{
    int cps = Timer::CheckPoints::Items.size();
    if (cps > 0) {
        console->Print("Result of %i checkpoint%s:\n", cps, (cps == 1) ? "" : "s");
    } else {
        console->Print("No result!\n");
        return;
    }

    for (int i = 0; i < cps; i++) {
        if (i == cps - 1 && Timer::IsRunning) {
            console->PrintActive("%s -> ", Timer::CheckPoints::Items[i].Map);
            console->PrintActive("%i ticks", Timer::CheckPoints::Items[i].Ticks);
            console->PrintActive("(%.3f)\n", Timer::CheckPoints::Items[i].Time);
        } else {
            console->Print("%s -> ", Timer::CheckPoints::Items[i].Map);
            console->Print("%i ticks", Timer::CheckPoints::Items[i].Ticks);
            console->Print("(%.3f)\n", Timer::CheckPoints::Items[i].Time);
        }
    }

    if (!Timer::IsRunning) {
        int tick = Timer::GetTick(*Engine::tickcount);
        float time = Engine::ToTime(tick);
        console->Print("Result: %i (%.3f)\n", tick, time);
    }
}