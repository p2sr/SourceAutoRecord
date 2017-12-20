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
	ConVar sar_timer_always_running;

	// Timer average
	ConCommand sar_avg_start;
	ConCommand sar_avg_stop;
	ConCommand sar_avg_result;

	// Timer checkpoints
	ConCommand sar_cps_add;
	ConCommand sar_cps_clear;
	ConCommand sar_cps_result;

	// HUD
	ConVar sar_hud_session;
	ConVar sar_hud_last_session;
	ConVar sar_hud_sum;
	ConVar sar_hud_timer;
	ConVar sar_hud_avg;
	ConVar sar_hud_cps;
	ConVar sar_hud_demo;
	ConVar sar_hud_last_demo;
	ConVar sar_hud_jumps;
	ConVar sar_hud_uses;

	// Stats
	ConVar sar_stats_auto_reset;
	ConCommand sar_stats_reset_jumps;
	ConCommand sar_stats_reset_uses;

	// Cheats
	ConVar sar_autojump;
	ConVar sar_aircontrol;
	ConVar sar_never_open_cm_hud;
	ConVar sar_never_delay_start;

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