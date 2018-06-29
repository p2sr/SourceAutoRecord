#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Timer.hpp"
#include "Features/TimerAverage.hpp"
#include "Features/TimerCheckPoints.hpp"

#include "Utils.hpp"

namespace Callbacks {

void StartTimer()
{
    if (Timer::IsRunning)
        Console::DevMsg("Restarting timer!\n");
    else
        Console::DevMsg("Starting timer!\n");
    Timer::Start(*Engine::tickcount);

    if (sar_stats_auto_reset.GetInt() >= 2) {
        Stats::Reset();
    }
}
void StopTimer()
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
void PrintTimer()
{
    int tick = Timer::GetTick(*Engine::tickcount);
    float time = Engine::GetTime(tick);

    if (Timer::IsRunning) {
        Console::PrintActive("Result: %i (%.3f)\n", tick, time);
    } else {
        Console::Print("Result: %i (%.3f)\n", tick, time);
    }
}
void StartAverage()
{
    Timer::Average::Start();
}
void StopAverage()
{
    Timer::Average::IsEnabled = false;
}
void PrintAverage()
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
void AddCheckpoint()
{
    if (!Timer::IsRunning) {
        Console::DevMsg("Timer isn't running!\n");
        return;
    }

    int tick = Timer::GetTick(Engine::GetTick());
    Timer::CheckPoints::Add(tick, Engine::GetTime(tick), *Engine::m_szLevelName);
}
void ClearCheckpoints()
{
    Timer::CheckPoints::Reset();
}
void PrintCheckpoints()
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
}