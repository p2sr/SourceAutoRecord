#pragma once
#include "Modules/ConCommand.hpp"
#include "Modules/ConVar.hpp"

using namespace Tier1;

namespace Commands
{
	// Rebinder
	ConCommand sar_bind_save;
	ConCommand sar_bind_reload;
	ConCommand sar_unbind_save;
	ConCommand sar_unbind_reload;
	ConVar sar_save_flag;

	// Demo
	ConCommand sar_time_demo;
	ConCommand sar_time_demos;
	ConVar sar_time_demo_dev;

	// Summary
	ConCommand sar_sum_here;
	ConCommand sar_sum_stop;
	ConCommand sar_sum_result;
	ConVar sar_sum_during_session;

	// Timer
	ConCommand sar_timer_start;
	ConCommand sar_timer_stop;
	ConCommand sar_timer_result;

	// Timer average
	ConCommand sar_avg_start;
	ConCommand sar_avg_stop;
	ConCommand sar_avg_result;

	// Timer checkpoints
	ConCommand sar_cps_add;
	ConCommand sar_cps_clear;
	ConCommand sar_cps_result;

	// Drawing
	ConVar sar_draw_session;
	ConVar sar_draw_last_session;
	ConVar sar_draw_sum;
	ConVar sar_draw_timer;
	ConVar sar_draw_avg;
	ConVar sar_draw_cps;
	ConVar sar_draw_demo;

	// Cheats
	ConVar sar_autojump;

	// Others
	ConCommand sar_session;
	ConCommand sar_about;

	// From the game
	ConVar cl_showpos;
	ConVar sv_cheats;
	ConVar sv_bonus_challenge;
	ConVar sv_accelerate;
	ConVar sv_airaccelerate;
	ConVar sv_friction;
	ConVar sv_maxspeed;
	ConVar sv_stopspeed;
}