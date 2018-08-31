#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Timer/Timer.hpp"
#include "Features/Timer/TimerAverage.hpp"
#include "Features/Timer/TimerCheckPoints.hpp"

#include "Cheats.hpp"
#include "Command.hpp"

CON_COMMAND(sar_timer_start, "Starts timer.\n")
{
    if (timer->isRunning)
        console->DevMsg("Restarting timer!\n");
    else
        console->DevMsg("Starting timer!\n");
    timer->Start(*Engine::tickcount);

    if (sar_stats_auto_reset.GetInt() >= 2) {
        stats->ResetAll();
    }
}

CON_COMMAND(sar_timer_stop, "Stops timer.\n")
{
    if (!timer->isRunning) {
        console->DevMsg("Timer isn't running!\n");
        return;
    }

    timer->Stop(*Engine::tickcount);

    if (timer->avg->isEnabled) {
        int tick = timer->GetTick(*Engine::tickcount);
        timer->avg->Add(tick, Engine::ToTime(tick), Engine::m_szLevelName);
    }
}

CON_COMMAND(sar_timer_result, "Prints result of timer.\n")
{
    int tick = timer->GetTick(*Engine::tickcount);
    float time = Engine::ToTime(tick);

    if (timer->isRunning) {
        console->PrintActive("Result: %i (%.3f)\n", tick, time);
    } else {
        console->Print("Result: %i (%.3f)\n", tick, time);
    }
}

CON_COMMAND(sar_avg_start, "Starts calculating the average when using timer.\n")
{
    timer->avg->Start();
}

CON_COMMAND(sar_avg_stop, "Stops average calculation.\n")
{
    timer->avg->isEnabled = false;
}

CON_COMMAND(sar_avg_result, "Prints result of average.\n")
{
    int average = timer->avg->items.size();
    if (average > 0) {
        console->Print("Average of %i:\n", average);
    } else {
        console->Print("No result!\n");
        return;
    }

    for (int i = 0; i < average; i++) {
        console->Print("%s -> ", timer->avg->items[i].map);
        console->Print("%i ticks", timer->avg->items[i].ticks);
        console->Print("(%.3f)\n", timer->avg->items[i].time);
    }

    if (timer->isRunning) {
        console->PrintActive("Result: %i (%.3f)\n", timer->avg->averageTicks, timer->avg->averageTime);
    } else {
        console->Print("Result: %i (%.3f)\n", timer->avg->averageTicks, timer->avg->averageTime);
    }
}

CON_COMMAND(sar_cps_add, "Saves current time of timer.\n")
{
    if (!timer->isRunning) {
        console->DevMsg("Timer isn't running!\n");
        return;
    }

    int tick = timer->GetTick(Engine::GetSessionTick());
    timer->cps->Add(tick, Engine::ToTime(tick), Engine::m_szLevelName);
}

CON_COMMAND(sar_cps_clear, "Resets saved times of timer.\n")
{
    timer->cps->Reset();
}

CON_COMMAND(sar_cps_result, "Prints result of timer checkpoints.\n")
{
    int cps = timer->cps->items.size();
    if (cps > 0) {
        console->Print("Result of %i checkpoint%s:\n", cps, (cps == 1) ? "" : "s");
    } else {
        console->Print("No result!\n");
        return;
    }

    for (int i = 0; i < cps; i++) {
        if (i == cps - 1 && timer->isRunning) {
            console->PrintActive("%s -> ", timer->cps->items[i].map);
            console->PrintActive("%i ticks", timer->cps->items[i].ticks);
            console->PrintActive("(%.3f)\n", timer->cps->items[i].time);
        } else {
            console->Print("%s -> ", timer->cps->items[i].map);
            console->Print("%i ticks", timer->cps->items[i].ticks);
            console->Print("(%.3f)\n", timer->cps->items[i].time);
        }
    }

    if (!timer->isRunning) {
        int tick = timer->GetTick(*Engine::tickcount);
        float time = Engine::ToTime(tick);
        console->Print("Result: %i (%.3f)\n", tick, time);
    }
}
