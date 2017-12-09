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
					if (sar_avg_enabled.GetBool()) {
						if (Timer::IsRunning) {
							snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f) | average: %i (%.3f)", Timer::GetTick() + tick, Timer::GetTime() + time, Timer::Average::AverageTicks, Timer::Average::AverageTime);
						}
						else {
							snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f) | average: %i (%.3f)", Timer::GetTick(), Timer::GetTime(), Timer::Average::AverageTicks, Timer::Average::AverageTime);
						}
					}
					else if (sar_cps_enabled.GetBool()) {
						if (Timer::IsRunning) {
							snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f) | last cp: %i (%.3f)", Timer::GetTick() + tick, Timer::GetTime() + time, Timer::CheckPoints::LatestTick, Timer::CheckPoints::LatestTime);
						}
						else {
							snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f) | last cp: %i (%.3f)", Timer::GetTick(), Timer::GetTime(), Timer::CheckPoints::LatestTick, Timer::CheckPoints::LatestTime);
						}
					}
					else {
						if (Timer::IsRunning) {
							snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f)", Timer::GetTick() + tick, Timer::GetTime() + time);
						}
						else {
							snprintf(ticks, sizeof(ticks), "ticks: %i (%.3f)", Timer::GetTick(), Timer::GetTime());
						}
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