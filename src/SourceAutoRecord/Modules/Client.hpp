#pragma once
#include "Engine.hpp"
#include "Surface.hpp"

#include "../Timer.hpp"
#include "../TimerAverage.hpp"
#include "../TimerCheckpoints.hpp"
#include "../Utils.hpp"

using _Paint = int(__thiscall*)(void* thisptr);

// client.dll
namespace Client
{
	namespace Original
	{
		_Paint Paint;
	}

	namespace Detour
	{
		int ShowPosFont = 5;

		int __fastcall Paint(void* thisptr, int edx)
		{
			if (sar_showticks.GetBool()) {
				char ticks[64];
				int tick = !*Engine::LoadGame ? Engine::GetTick() : 0;
				float time = tick * *Engine::IntervalPerTick;

				if (sar_timer_enabled.GetBool()) {
					if (Timer::IsRunning) {
						tick = Timer::GetTick(tick), time;
						time = tick * *Engine::IntervalPerTick;
					}
					else {
						tick = Timer::GetTick(), time;
						time = tick * *Engine::IntervalPerTick;
					}

					if (sar_avg_enabled.GetBool()) {
						snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f) | average: %i (%.3f)", tick, time, Timer::Average::AverageTicks, Timer::Average::AverageTime);
					}
					else if (sar_cps_enabled.GetBool()) {
						snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f) | last cp: %i (%.3f)", tick, time, Timer::CheckPoints::LatestTick, Timer::CheckPoints::LatestTime);
					}
					else {
						snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f)", tick, time);
					}
				}
				else if (Summary::IsRunning) {
					if (sar_sum_during_session.GetBool()) {
						snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f) | total: %i (%.3f)", tick, time, Summary::TotalTicks + tick, Summary::TotalTime + time);
					}
					else {
						snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f) | total: %i (%.3f)", tick, time, Summary::TotalTicks, Summary::TotalTime);
					}
				}
				else {
					snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f)", tick, time);
				}

				Surface::Draw(ShowPosFont, 1, 65, COL_WHITE, ticks);
			}
			return Original::Paint(thisptr);
		}
	}
}