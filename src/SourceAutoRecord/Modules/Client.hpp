#pragma once
#include "Engine.hpp"
#include "Surface.hpp"

#include "Session.hpp"
#include "Stats.hpp"
#include "Timer.hpp"
#include "TimerAverage.hpp"
#include "TimerCheckPoints.hpp"
#include "Utils.hpp"

// Is this large enough for everyone?
#define FPS_PANEL_WIDTH 3333

struct CUserCmd {
	int unk;
	int command_number; // 4
	int tick_count; // 8
	QAngle viewangles; // 12, 16, 20
	float forwardmove; // 24
	float sidemove; // 28
	float upmove; // 32
	int buttons; // 36
	unsigned char impulse; // 40
	int weaponselect; // 44
	int weaponsubtype; // 48
	int random_seed; // 52
	short mousedx; // 56
	short mousedy; // 58
	bool hasbeenpredicted; // 60
};

using _Paint = int(__cdecl*)(void* thisptr);
using _ShouldDraw = bool(__cdecl*)(void* thisptr);
using _FindElement = int(__cdecl*)(void* thisptr, const char* pName);
using _SetSize = int(__cdecl*)(void* thisptr, int wide, int tall);
using _GetLocalPlayer = void*(__cdecl*)(int unk);

namespace Client
{
	_SetSize SetSize;
	_GetLocalPlayer GetLocalPlayer;

	Vector* MainViewOrigin;
	QAngle* MainViewAngles;

	void Set(uintptr_t setSizeAddress, uintptr_t getLocalPlayerAddress, uintptr_t getPosAddress)
	{
		SetSize = (_SetSize)setSizeAddress;
		GetLocalPlayer = (_GetLocalPlayer)getLocalPlayerAddress;
		MainViewOrigin = *(Vector**)((uintptr_t)getPosAddress + Offsets::MainViewOrigin);
		MainViewAngles = *(QAngle**)((uintptr_t)getPosAddress + Offsets::MainViewAngles);
	}
	Vector GetAbsOrigin()
	{
		auto player = Client::GetLocalPlayer(-1);
		return (player) ? *(Vector*)((uintptr_t)player + Offsets::GetAbsOrigin) : Vector();
	}
	QAngle GetAbsAngles()
	{
		auto player = Client::GetLocalPlayer(-1);
		return (player) ? *(QAngle*)((uintptr_t)player + Offsets::GetAbsAngles) : QAngle();
	}
	Vector GetLocalVelocity()
	{
		auto player = Client::GetLocalPlayer(-1);
		return (player) ? *(Vector*)((uintptr_t)player + Offsets::GetLocalVelocity) : Vector();
	}

	namespace Original
	{
		_Paint Paint;
		_ComputeSize ComputeSize;
		_ShouldDraw ShouldDraw;
		_FindElement FindElement;
		_CreateMove CreateMove;
		_ControllerMove ControllerMove;
	}

	namespace Detour
	{
		CUserCmd* cmd;

		int __fastcall Paint(void* thisptr, int edx)
		{
			int result = 0;

			int elements = 0;
			int xPadding = sar_hud_default_padding_x.GetInt();
			int yPadding = sar_hud_default_padding_y.GetInt();
			int spacing = sar_hud_default_spacing.GetInt();

			unsigned long m_hFont = *(unsigned long*)((uintptr_t)thisptr + Offsets::m_hFont) + sar_hud_default_font_index.GetInt();
			int fontSize = sar_hud_default_font_size.GetInt();

			int r, g, b, a;
			sscanf(sar_hud_default_font_color.GetString(), "%i%i%i%i", &r, &g, &b, &a);
			Color textColor(r, g, b, a);

			if (cl_showpos.GetBool()) {
				result = Original::Paint(thisptr);
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
					snprintf(position, sizeof(position), "pos: %.3f %.3f %.3f", MainViewOrigin->x, MainViewOrigin->y, MainViewOrigin->z);
				}
				else {
					auto pos = GetAbsOrigin();
					snprintf(position, sizeof(position), "pos: %.3f %.3f %.3f", pos.x, pos.y, pos.z);
				}
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, position);
				elements++;
			}
			if (sar_hud_angles.GetBool()) {
				char angles[64];
				snprintf(angles, sizeof(angles), "ang: %.3f %.3f", MainViewAngles->x, MainViewAngles->y);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, angles);
				elements++;
			}
			if (sar_hud_velocity.GetBool()) {
				char velocity[64];
				snprintf(velocity, sizeof(velocity), "vel: %.3f", (sar_hud_velocity.GetInt() == 1) ? GetLocalVelocity().Length() : GetLocalVelocity().Length2D());
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, velocity);
				elements++;
			}
			// Session
			if (sar_hud_session.GetBool()) {
				int tick = (!*Engine::LoadGame) ? Engine::GetTick() : 0;
				float time = tick * *Engine::IntervalPerTick;

				char session[64];
				snprintf(session, sizeof(session), "session: %i (%.3f)", tick, time);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, session);
				elements++;
			}
			if (sar_hud_last_session.GetBool()) {
				char session[64];
				snprintf(session, sizeof(session), "last session: %i (%.3f)", Session::LastSession, Session::LastSession * *Engine::IntervalPerTick);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, session);
				elements++;
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
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, sum);
				elements++;
			}
			// Timer
			if (sar_hud_timer.GetBool()) {
				int tick = (!Timer::IsPaused) ? Timer::GetTick(*Engine::TickCount) : Timer::TotalTicks;
				float time = tick * *Engine::IntervalPerTick;

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
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, demo);
				elements++;
			}
			if (sar_hud_last_demo.GetBool()) {
				char demo[64];
				if (!DemoRecorder::LastDemo.empty()) {
					int tick = DemoRecorder::LastDemoTick;
					float time = tick * *Engine::IntervalPerTick;
					snprintf(demo, sizeof(demo), "last demo: %s %i (%.3f)", DemoRecorder::LastDemo.c_str(), tick, time);
				}
				else {
					snprintf(demo, sizeof(demo), "last demo: -");
				}
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, demo);
				elements++;
			}
			// Stats
			if (sar_hud_jumps.GetBool()) {
				char jumps[64];
				snprintf(jumps, sizeof(jumps), "jumps: %i", Stats::TotalJumps);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, jumps);
				elements++;
			}
			if (sar_hud_uses.GetBool()) {
				char uses[64];
				snprintf(uses, sizeof(uses), "uses: %i", Stats::TotalUses);
				Surface::Draw(m_hFont, xPadding, yPadding + elements * (fontSize + spacing), textColor, uses);
				elements++;
			}

			// Original paint function might resize the panel
			// And this needs more space anyway
			SetSize(thisptr, FPS_PANEL_WIDTH, FPS_PANEL_WIDTH);
			return result;
		}
		bool __fastcall ShouldDraw(void* thisptr, int edx)
		{
			return Original::ShouldDraw(thisptr)
				|| sar_hud_text.GetString()[0] != '\0'
				|| sar_hud_position.GetBool()
				|| sar_hud_angles.GetBool()
				|| sar_hud_velocity.GetBool()
				|| sar_hud_session.GetBool()
				|| sar_hud_last_session.GetBool()
				|| sar_hud_sum.GetBool()
				|| sar_hud_timer.GetBool()
				|| sar_hud_avg.GetBool()
				|| sar_hud_cps.GetBool()
				|| sar_hud_demo.GetBool()
				|| sar_hud_last_demo.GetBool()
				|| sar_hud_jumps.GetBool()
				|| sar_hud_uses.GetBool();
		}
		int __fastcall FindElement(void* thisptr, int edx, const char* pName)
		{
			if (sar_never_open_cm_hud.GetBool() && strcmp(pName, "CHUDChallengeStats") == 0) {
				return 0;
			}
			return Original::FindElement(thisptr, pName);
		}
	}
}