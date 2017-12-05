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
		int __fastcall Paint(void* thisptr, int edx)
		{
			//int font = (int)(*((uintptr_t*)thisptr + 348));
			int font = 5;

			char ticks[64];
			int tick = !*Engine::LoadGame ? Engine::GetCurrentTick() : 0;
			float time = tick * *Engine::IntervalPerTick;
			if (Summary::HasStarted)
				snprintf(ticks, sizeof(ticks), "ticks: %i (%.3fs) | total: %i (%.3f)", tick, time, Summary::TotalTicks, Summary::TotalTime);
			else
				snprintf(ticks, sizeof(ticks), "ticks: %i (%.3fs)", tick, time);
			Surface::Draw(font, 1, 65, COL_WHITE, ticks);

			return Original::Paint(thisptr);
		}
	}
}