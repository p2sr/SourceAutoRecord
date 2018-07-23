#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Summary.hpp"

#include "Cheats.hpp"
#include "Command.hpp"

CON_COMMAND(sar_sum_here, "Starts counting total ticks of sessions.\n")
{
    if (Summary::IsRunning) {
        console->Print("Summary has already started!\n");
        return;
    }
    Summary::Start();
}

CON_COMMAND(sar_sum_stop, "Stops summary counter.\n")
{
    if (!Summary::IsRunning) {
        console->Print("There's no summary to stop!\n");
        return;
    }

    if (Cheats::sar_sum_during_session.GetBool()) {
        int tick = Engine::GetSessionTick();
        Summary::Add(tick, Engine::ToTime(tick), *Engine::m_szLevelName);
    }
    Summary::IsRunning = false;
}

CON_COMMAND(sar_sum_result, "Prints result of summary.\n")
{
    int sessions = Summary::Items.size();
    if (Summary::IsRunning && sessions == 0) {
        console->Print("Summary of this session:\n");
    } else if (Summary::IsRunning && sessions > 0) {
        console->Print("Summary of %i sessions:\n", sessions + 1);
    } else if (sessions > 0) {
        console->Print("Summary of %i session%s:\n", sessions, (sessions == 1) ? "" : "s");
    } else {
        console->Print("There's no result of a summary!\n");
        return;
    }

    for (size_t i = 0; i < Summary::Items.size(); i++) {
        console->Print("%s -> ", Summary::Items[i].Map);
        console->Print("%i ticks", Summary::Items[i].Ticks);
        console->Print("(%.3f)\n", Summary::Items[i].Time);
    }

    float totalTime = Engine::ToTime(Summary::TotalTicks);
    if (Summary::IsRunning) {
        int tick = Engine::GetSessionTick();
        float time = Engine::ToTime(tick);
        console->PrintActive("%s -> ", *Engine::m_szLevelName);
        console->PrintActive("%i ticks ", tick);
        console->PrintActive("(%.3f)\n", time);
        console->Print("---------------\n");
        console->Print("Total Ticks: %i ", Summary::TotalTicks);
        console->PrintActive("(%i)\n", Summary::TotalTicks + tick);
        console->Print("Total Time: %.3f ", totalTime);
        console->PrintActive("(%.3f)\n", totalTime + time);
    } else {
        console->Print("---------------\n");
        console->Print("Total Ticks: %i\n", Summary::TotalTicks);
        console->Print("Total Time: %.3f\n", totalTime);
    }
}