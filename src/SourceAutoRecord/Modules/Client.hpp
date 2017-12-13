#pragma once
#include "Engine.hpp"
#include "Surface.hpp"

#include "Patterns.hpp"
#include "Stats.hpp"
#include "Timer.hpp"
#include "TimerAverage.hpp"
#include "TimerCheckpoints.hpp"
#include "Utils.hpp"

#define FPS_PANEL_WIDTH 300

using _Paint = int(__thiscall*)(void* thisptr);
using _ComputeSize = int(__thiscall*)(void* thisptr);
using _SetSize = int(__thiscall*)(void* thisptr, int wide, int tall);
using _ShouldDraw = bool(__thiscall*)(void* thisptr);

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
		_ShouldDraw ShouldDraw;
	}

	namespace Detour
	{
		int __fastcall Paint(void* thisptr, int edx)
		{
			int result = 0;

			int m_hFont = *(int*)((uintptr_t)thisptr + Offsets::m_hFont);

			int level = 0;
			int offset = 2;
			const int size = 10;
			const int spacing = 4;

			if (cl_showpos.GetBool()) {
				result = Original::Paint(thisptr);
				offset = 67;
			}

			// Session
			if (sar_hud_session.GetBool()) {
				int tick = (!*Engine::LoadGame) ? Engine::GetTick() : 0;
				float time = tick * *Engine::IntervalPerTick;

				char session[64];
				snprintf(session, sizeof(session), "session: %i (%.3f)", tick, time);
				Surface::Draw(m_hFont, 1, offset + level * (size + spacing), COL_WHITE, session);
				level++;
			}
			if (sar_hud_last_session.GetBool()) {
				char session[64];
				snprintf(session, sizeof(session), "last session: %i (%.3f)", Engine::LastSavedSession, Engine::LastSavedSession * *Engine::IntervalPerTick);
				Surface::Draw(m_hFont, 1, offset + level * (size + spacing), COL_WHITE, session);
				level++;
			}
			if (sar_hud_sum.GetBool()) {
				char sum[64];
				if (Summary::IsRunning && sar_sum_during_session.GetBool()) {
					int tick = (!*Engine::LoadGame) ? Engine::GetTick() : 0;
					float time = tick * *Engine::IntervalPerTick;
					snprintf(sum, sizeof(sum), "sum: %i (%.3f)", Summary::TotalTicks + tick, Summary::TotalTime + time);
				}
				else {
					snprintf(sum, sizeof(sum), "sum: %i (%.3f)", Summary::TotalTicks, Summary::TotalTime);
				}
				Surface::Draw(m_hFont, 1, offset + level * (size + spacing), COL_WHITE, sum);
				level++;
			}
			// Timer
			if (sar_hud_timer.GetBool()) {
				int tick = Timer::GetTick((Timer::IsRunning) ? Engine::GetTick() : -1);
				float time = tick * *Engine::IntervalPerTick;

				char timer[64];
				snprintf(timer, sizeof(timer), "timer: %i (%.3f)", tick, time);
				Surface::Draw(m_hFont, 1, offset + level * (size + spacing), COL_WHITE, timer);
				level++;
			}
			if (sar_hud_avg.GetBool()) {
				char avg[64];
				snprintf(avg, sizeof(avg), "avg: %i (%.3f)", Timer::Average::AverageTicks, Timer::Average::AverageTime);
				Surface::Draw(m_hFont, 1, offset + level * (size + spacing), COL_WHITE, avg);
				level++;
			}
			if (sar_hud_cps.GetBool()) {
				char cps[64];
				snprintf(cps, sizeof(cps), "last cp: %i (%.3f)", Timer::CheckPoints::LatestTick, Timer::CheckPoints::LatestTime);
				Surface::Draw(m_hFont, 1, offset + level * (size + spacing), COL_WHITE, cps);
				level++;
			}
			// Demo
			if (sar_hud_demo.GetBool()) {
				char demo[64];
				if (!*Engine::LoadGame && *DemoRecorder::Recording && !DemoRecorder::CurrentDemo.empty()) {
					int tick = DemoRecorder::GetTick();
					float time = tick * *Engine::IntervalPerTick;
					snprintf(demo, sizeof(demo), "demo: %s %i (%.3f)", DemoRecorder::CurrentDemo.c_str(), tick, time);
				}
				else if (!*Engine::LoadGame && DemoPlayer::IsPlaying()) {
					int tick = DemoPlayer::GetTick();
					// Demos should overwrite interval_per_tick anyway if I remember correctly
					float time = tick * *Engine::IntervalPerTick;
					snprintf(demo, sizeof(demo), "demo: %s %i (%.3f)", DemoPlayer::DemoName, tick, time);
				}
				else {
					snprintf(demo, sizeof(demo), "demo: -");
				}
				Surface::Draw(m_hFont, 1, offset + level * (size + spacing), COL_WHITE, demo);
				level++;
			}
			// Stats
			if (sar_hud_jumps.GetBool()) {
				char jumps[64];
				snprintf(jumps, sizeof(jumps), "jumps: %i", Stats::TotalJumps);
				Surface::Draw(m_hFont, 1, offset + level * (size + spacing), COL_WHITE, jumps);
				level++;
			}
			if (sar_hud_uses.GetBool()) {
				char uses[64];
				snprintf(uses, sizeof(uses), "uses: %i", Stats::TotalUses);
				Surface::Draw(m_hFont, 1, offset + level * (size + spacing), COL_WHITE, uses);
				level++;
			}

			// Original paint function might resize the panel
			// And this needs more space anyway
			SetSize(thisptr, FPS_PANEL_WIDTH, FPS_PANEL_WIDTH);
			return result;
		}
		bool __fastcall ShouldDraw(void* thisptr, int edx)
		{
			return Original::ShouldDraw(thisptr)
				|| sar_hud_session.GetBool()
				|| sar_hud_last_session.GetBool()
				|| sar_hud_sum.GetBool()
				|| sar_hud_timer.GetBool()
				|| sar_hud_avg.GetBool()
				|| sar_hud_cps.GetBool()
				|| sar_hud_demo.GetBool()
				|| sar_hud_jumps.GetBool()
				|| sar_hud_uses.GetBool();
		}
	}
}