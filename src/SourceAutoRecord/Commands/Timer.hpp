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
        Console::DevMsg("Restarting timer!\n");
    else
        Console::DevMsg("Starting timer!\n");
    Timer::Start(*Engine::tickcount);

    if (Cheats::sar_stats_auto_reset.GetInt() >= 2) {
        Stats::ResetAll();
    }
}

CON_COMMAND(sar_timer_stop, "Stops timer.\n")
{
    if (!Timer::IsRunning) {
        Console::DevMsg("Timer isn't running!\n");
        return;
    }

    Timer::Stop(*Engine::tickcount);

    if (Timer::Average::IsEnabled) {
        int tick = Timer::GetTick(*Engine::tickcount);
        Timer::Average::Add(tick, Engine::GetTime(tick), *Engine::m_szLevelName);
    }
}

CON_COMMAND(sar_timer_result, "Prints result of timer.\n")
{
    int tick = Timer::GetTick(*Engine::tickcount);
    float time = Engine::GetTime(tick);

    if (Timer::IsRunning) {
        Console::PrintActive("Result: %i (%.3f)\n", tick, time);
    } else {
        Console::Print("Result: %i (%.3f)\n", tick, time);
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
        Console::Print("Average of %i:\n", average);
    } else {
        Console::Print("No result!\n");
        return;
    }

    for (int i = 0; i < average; i++) {
        Console::Print("%s -> ", Timer::Average::Items[i].Map);
        Console::Print("%i ticks", Timer::Average::Items[i].Ticks);
        Console::Print("(%.3f)\n", Timer::Average::Items[i].Time);
    }

    if (Timer::IsRunning) {
        Console::PrintActive("Result: %i (%.3f)\n", Timer::Average::AverageTicks, Timer::Average::AverageTime);
    } else {
        Console::Print("Result: %i (%.3f)\n", Timer::Average::AverageTicks, Timer::Average::AverageTime);
    }
}

CON_COMMAND(sar_cps_add, "Saves current time of timer.\n")
{
    if (!Timer::IsRunning) {
        Console::DevMsg("Timer isn't running!\n");
        return;
    }

    int tick = Timer::GetTick(Engine::GetTick());
    Timer::CheckPoints::Add(tick, Engine::GetTime(tick), *Engine::m_szLevelName);
}

CON_COMMAND(sar_cps_clear, "Resets saved times of timer.\n")
{
    Timer::CheckPoints::Reset();
}

CON_COMMAND(sar_cps_result, "Prints result of timer checkpoints.\n")
{
    int cps = Timer::CheckPoints::Items.size();
    if (cps > 0) {
        Console::Print("Result of %i checkpoint%s:\n", cps, (cps == 1) ? "" : "s");
    } else {
        Console::Print("No result!\n");
        return;
    }

    for (int i = 0; i < cps; i++) {
        if (i == cps - 1 && Timer::IsRunning) {
            Console::PrintActive("%s -> ", Timer::CheckPoints::Items[i].Map);
            Console::PrintActive("%i ticks", Timer::CheckPoints::Items[i].Ticks);
            Console::PrintActive("(%.3f)\n", Timer::CheckPoints::Items[i].Time);
        } else {
            Console::Print("%s -> ", Timer::CheckPoints::Items[i].Map);
            Console::Print("%i ticks", Timer::CheckPoints::Items[i].Ticks);
            Console::Print("(%.3f)\n", Timer::CheckPoints::Items[i].Time);
        }
    }

    if (!Timer::IsRunning) {
        int tick = Timer::GetTick(*Engine::tickcount);
        float time = Engine::GetTime(tick);
        Console::Print("Result: %i (%.3f)\n", tick, time);
    }
}