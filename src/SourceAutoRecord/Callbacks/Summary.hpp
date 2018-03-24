#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Summary.hpp"

namespace Callbacks
{
	void StartSummary()
	{
		if (Summary::IsRunning) {
			Console::Print("Summary has already started!\n");
			return;
		}
		Summary::Start();
	}
	void StopSummary()
	{
		if (!Summary::IsRunning) {
			Console::Print("There's no summary to stop!\n");
			return;
		}

		if (sar_sum_during_session.GetBool()) {
			int tick = Engine::GetTick();
			Summary::Add(tick, tick * *Vars::interval_per_tick, *Vars::m_szLevelName);
		}
		Summary::IsRunning = false;
	}
	void PrintSummary()
	{
		int sessions = Summary::Items.size();
		if (Summary::IsRunning && sessions == 0) {
			Console::Print("Summary of this session:\n");
		}
		else if (Summary::IsRunning && sessions > 0) {
			Console::Print("Summary of %i sessions:\n", sessions + 1);
		}
		else if (sessions > 0) {
			Console::Print("Summary of %i session%s:\n", sessions, (sessions == 1) ? "" : "s");
		}
		else {
			Console::Print("There's no result of a summary!\n");
			return;
		}

		for (size_t i = 0; i < Summary::Items.size(); i++) {
			Console::Print("%s -> ", Summary::Items[i].Map);
			Console::Print("%i ticks", Summary::Items[i].Ticks);
			Console::Print("(%.3f)\n", Summary::Items[i].Time);
		}

		if (Summary::IsRunning) {
			int tick = Engine::GetTick();
			float time = tick * *Vars::interval_per_tick;
			Console::PrintActive("%s -> ", *Vars::m_szLevelName);
			Console::PrintActive("%i ticks ", tick);
			Console::PrintActive("(%.3f)\n", time);
			Console::Print("---------------\n");
			Console::Print("Total Ticks: %i ", Summary::TotalTicks);
			Console::PrintActive("(%i)\n", Summary::TotalTicks + tick);
			Console::Print("Total Time: %.3f ", Summary::TotalTime);
			Console::PrintActive("(%.3f)\n", Summary::TotalTime + time);
		}
		else {
			Console::Print("---------------\n");
			Console::Print("Total Ticks: %i\n", Summary::TotalTicks);
			Console::Print("Total Time: %.3f\n", Summary::TotalTime);
		}
	}
}