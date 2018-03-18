#pragma once
#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "Surface.hpp"
#include "Vars.hpp"

#include "Features/Session.hpp"
#include "Features/Stats.hpp"
#include "Features/Timer.hpp"
#include "Features/TimerAverage.hpp"
#include "Features/TimerCheckPoints.hpp"

#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

namespace VGui
{
	using _Paint = int(__cdecl*)(void* thisptr, int mode);
	
	std::unique_ptr<VMTHook> enginevgui;

	namespace Original
	{
		_Paint Paint;
	}

	namespace Detour
	{
		int __cdecl Paint(void* thisptr, int mode)
		{
			Surface::StartDrawing(Surface::matsurface->GetThisPtr());

			int elements = 0;
			int xPadding = sar_hud_default_padding_x.GetInt();
			int yPadding = sar_hud_default_padding_y.GetInt();
			int spacing = sar_hud_default_spacing.GetInt();

			unsigned long m_hFont = 16 + sar_hud_default_font_index.GetInt();
			int fontSize = sar_hud_default_font_size.GetInt();

			int r, g, b, a;
			sscanf(sar_hud_default_font_color.GetString(), "%i%i%i%i", &r, &g, &b, &a);
			Color textColor(r, g, b, a);

			if (cl_showpos.GetBool()) {
				yPadding += 65; // Ehem?
			}

			// cl_showpos replacement
			if (sar_hud_text.GetString()[0] != '\0') {
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, (char*)sar_hud_text.GetString());
				elements++;
			}
			if (sar_hud_position.GetBool()) {
				char position[64];
				
				if (sar_hud_position.GetInt() == 1) {
					snprintf(position, sizeof(position), "pos: %.3f %.3f %.3f", Client::MainViewOrigin->x, Client::MainViewOrigin->y, Client::MainViewOrigin->z);
				}
				else {
					auto pos = Client::GetAbsOrigin();
					snprintf(position, sizeof(position), "pos: %.3f %.3f %.3f", pos.x, pos.y, pos.z);
				}
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, position);
				elements++;
			}
			if (sar_hud_angles.GetBool()) {
				char angles[64];
				snprintf(angles, sizeof(angles), "ang: %.3f %.3f", Client::MainViewAngles->x, Client::MainViewAngles->y);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, angles);
				elements++;
			}
			if (sar_hud_velocity.GetBool()) {
				char velocity[64];
				snprintf(velocity, sizeof(velocity), "vel: %.3f", (sar_hud_velocity.GetInt() == 1) ? Client::GetLocalVelocity().Length() : Client::GetLocalVelocity().Length2D());
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, velocity);
				elements++;
			}
			// Session
			if (sar_hud_session.GetBool()) {
				int tick = (!*Vars::LoadGame) ? Engine::GetTick() : 0;
				float time = tick * Vars::gpGlobals->interval_per_tick;

				char session[64];
				snprintf(session, sizeof(session), "session: %i (%.3f)", tick, time);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, session);
				elements++;
			}
			if (sar_hud_last_session.GetBool()) {
				char session[64];
				snprintf(session, sizeof(session), "last session: %i (%.3f)", Session::LastSession, Session::LastSession * Vars::gpGlobals->interval_per_tick);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, session);
				elements++;
			}
			if (sar_hud_sum.GetBool()) {
				char sum[64];
				if (Summary::IsRunning && sar_sum_during_session.GetBool()) {
					int tick = (!*Vars::LoadGame) ? Engine::GetTick() : 0;
					float time = tick * Vars::gpGlobals->interval_per_tick;
					snprintf(sum, sizeof(sum), "sum: %i (%.3f)", Summary::TotalTicks + tick, Summary::TotalTime + time);
				}
				else {
					snprintf(sum, sizeof(sum), "sum: %i (%.3f)", Summary::TotalTicks, Summary::TotalTime);
				}
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, sum);
				elements++;
			}
			// Timer
			if (sar_hud_timer.GetBool()) {
				int tick = (!Timer::IsPaused) ? Timer::GetTick(Vars::gpGlobals->interval_per_tick) : Timer::TotalTicks;
				float time = tick * Vars::gpGlobals->interval_per_tick;

				char timer[64];
				snprintf(timer, sizeof(timer), "timer: %i (%.3f)", tick, time);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, timer);
				elements++;
			}
			if (sar_hud_avg.GetBool()) {
				char avg[64];
				snprintf(avg, sizeof(avg), "avg: %i (%.3f)", Timer::Average::AverageTicks, Timer::Average::AverageTime);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, avg);
				elements++;
			}
			if (sar_hud_cps.GetBool()) {
				char cps[64];
				snprintf(cps, sizeof(cps), "last cp: %i (%.3f)", Timer::CheckPoints::LatestTick, Timer::CheckPoints::LatestTime);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, cps);
				elements++;
			}
			// Demo
			if (sar_hud_demo.GetBool()) {
				char demo[64];
				if (!*Vars::LoadGame && *DemoRecorder::Recording && !DemoRecorder::CurrentDemo.empty()) {
					int tick = DemoRecorder::GetTick();
					float time = tick * Vars::gpGlobals->interval_per_tick;
					snprintf(demo, sizeof(demo), "demo: %s %i (%.3f)", DemoRecorder::CurrentDemo.c_str(), tick, time);
				}
				else if (!*Vars::LoadGame && DemoPlayer::IsPlaying()) {
					int tick = DemoPlayer::GetTick();
					// Demos should overwrite interval_per_tick anyway if I remember correctly
					float time = tick * Vars::gpGlobals->interval_per_tick;
					snprintf(demo, sizeof(demo), "demo: %s %i (%.3f)", DemoPlayer::DemoName, tick, time);
				}
				else {
					snprintf(demo, sizeof(demo), "demo: -");
				}
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, demo);
				elements++;
			}
			/* if (sar_hud_last_demo.GetBool()) {
				char demo[64];
				if (!DemoRecorder::LastDemo.empty()) {
					int tick = DemoRecorder::LastDemoTick;
					float time = tick * Vars::gpGlobals->interval_per_tick;
					snprintf(demo, sizeof(demo), "last demo: %s %i (%.3f)", DemoRecorder::LastDemo.c_str(), tick, time);
				}
				else {
					snprintf(demo, sizeof(demo), "last demo: -");
				}
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, demo);
				elements++;
			} */
			// Stats
			if (sar_hud_jumps.GetBool()) {
				char jumps[64];
				snprintf(jumps, sizeof(jumps), "jumps: %i", Stats::TotalJumps);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, jumps);
				elements++;
			}
			/* if (sar_hud_uses.GetBool()) {
				char uses[64];
				snprintf(uses, sizeof(uses), "uses: %i", Stats::TotalUses);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, uses);
				elements++;
			} */

			Surface::FinishDrawing();
			return Original::Paint(thisptr, mode);
		}
	}

	void Hook()
	{
		if (Interfaces::IEngineVGui) {
			enginevgui = std::make_unique<VMTHook>(Interfaces::IEngineVGui);
			enginevgui->HookFunction((void*)Detour::Paint, Offsets::Paint);
			Original::Paint = enginevgui->GetOriginalFunction<_Paint>(Offsets::Paint);
		}
	}
}