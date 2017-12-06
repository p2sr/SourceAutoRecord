#pragma once
#include "Engine.hpp"
#include "Surface.hpp"
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
			char ticks[64];
			int tick = !*Engine::LoadGame ? Engine::GetTick() : 0;
			float time = tick * *Engine::IntervalPerTick;

			if (Summary::IsRunning) {
				if (sar_sum_during_session.GetBool()) {
					snprintf(ticks, sizeof(ticks), "ticks: %i (%.3fs) | total: %i (%.3f)", tick, time, Summary::TotalTicks + tick, Summary::TotalTime + time);
				}
				else {
					snprintf(ticks, sizeof(ticks), "ticks: %i (%.3fs) | total: %i (%.3f)", tick, time, Summary::TotalTicks, Summary::TotalTime);
				}
			}
			else {
				snprintf(ticks, sizeof(ticks), "ticks: %i (%.3fs)", tick, time);
			}

			Surface::Draw(ShowPosFont, 1, 65, COL_WHITE, ticks);
			return Original::Paint(thisptr);
		}
	}
}