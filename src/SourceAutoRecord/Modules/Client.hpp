#pragma once
#include "Engine.hpp"
#include "Surface.hpp"

#include "Patterns.hpp"
#include "Timer.hpp"
#include "TimerAverage.hpp"
#include "TimerCheckpoints.hpp"
#include "Utils.hpp"

#define FPS_PANEL_WIDTH 300

using _Paint = int(__thiscall*)(void* thisptr);
using _ComputeSize = int(__thiscall*)(void* thisptr);
using _SetSize = int(__thiscall*)(void* thisptr, int wide, int tall);

// client.dll
namespace Client
{
	_SetSize SetSize;

	void Set(uintptr_t setSizeAddress)
	{	
		SetSize = (_SetSize)setSizeAddress;
	}

	namespace Original
	{
		_Paint Paint;
		_ComputeSize ComputeSize;
	}

	namespace Detour
	{
		int __fastcall Paint(void* thisptr, int edx)
		{
			int result = Original::Paint(thisptr);
			if (cl_showpos.GetBool()) {
				int m_hFont = *(int*)((uintptr_t)thisptr + Offsets::m_hFont);
				const int start = 65;
				const int factor = 10;
				int level = 0;

				// Session
				if (sar_draw_session.GetBool()) {
					int tick = (!*Engine::LoadGame) ? Engine::GetTick() : 0;
					float time = tick * *Engine::IntervalPerTick;

					char session[64];
					snprintf(session, sizeof(session), "session: %i (%.3f)", tick, time);
					Surface::Draw(m_hFont, 1, start + (factor * level), COL_WHITE, session);
					level++;
				}
				if (sar_draw_sum.GetBool()) {
					char sum[64];
					if (Summary::IsRunning && sar_sum_during_session.GetBool()) {
						int tick = (!*Engine::LoadGame) ? Engine::GetTick() : 0;
						float time = tick * *Engine::IntervalPerTick;
						snprintf(sum, sizeof(sum), "sum: %i (%.3f)", Summary::TotalTicks + tick, Summary::TotalTime + time);
					}
					else {
						snprintf(sum, sizeof(sum), "sum: %i (%.3f)", Summary::TotalTicks, Summary::TotalTime);
					}
					Surface::Draw(m_hFont, 1, start + (factor * level), COL_WHITE, sum);
					level++;
				}
				// Timer
				if (sar_draw_timer.GetBool()) {
					int tick = Timer::GetTick((Timer::IsRunning) ? Engine::GetTick() : -1);
					float time = tick * *Engine::IntervalPerTick;

					char timer[64];
					snprintf(timer, sizeof(timer), "timer: %i (%.3f)", tick, time);
					Surface::Draw(m_hFont, 1, start + (factor * level), COL_WHITE, timer);
					level++;
				}
				if (sar_draw_avg.GetBool()) {
					char avg[64];
					snprintf(avg, sizeof(avg), "avg: %i (%.3f)", Timer::Average::AverageTicks, Timer::Average::AverageTime);
					Surface::Draw(m_hFont, 1, start + (factor * level), COL_WHITE, avg);
					level++;
				}
				if (sar_draw_cps.GetBool()) {
					char cps[64];
					snprintf(cps, sizeof(cps), "last cp: %i (%.3f)", Timer::CheckPoints::LatestTick, Timer::CheckPoints::LatestTime);
					Surface::Draw(m_hFont, 1, start + (factor * level), COL_WHITE, cps);
					level++;
				}
				// Demo
				if (sar_draw_demo.GetBool()) {
					char demo[64];
					snprintf(demo, sizeof(demo), "demo: %s", (DemoRecorder::LastDemo.empty()) ? "-" : DemoRecorder::LastDemo.c_str());
					Surface::Draw(m_hFont, 1, start + (factor * level), COL_WHITE, demo);
					level++;
				}

				SetSize(thisptr, FPS_PANEL_WIDTH, FPS_PANEL_WIDTH);
			}
			return result;
		}
	}
}