#pragma once
#include "Modules/ConCommand.hpp"
#include "Modules/ConVar.hpp"

using namespace Tier1;

namespace Commands
{
	ConVar sar_autorecord;

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
	ConVar sar_hud_text;
	ConVar sar_hud_position;
	ConVar sar_hud_angles;
	ConVar sar_hud_velocity;
	ConVar sar_hud_session;
	ConVar sar_hud_last_session;
	ConVar sar_hud_sum;
	ConVar sar_hud_timer;
	ConVar sar_hud_avg;
	ConVar sar_hud_cps;
	ConVar sar_hud_demo;
	ConVar sar_hud_jumps;
	ConVar sar_hud_portals;
	ConVar sar_hud_steps;

	ConVar sar_hud_default_spacing;
	ConVar sar_hud_default_padding_x;
	ConVar sar_hud_default_padding_y;
	ConVar sar_hud_default_font_index;
	ConVar sar_hud_default_font_color;

	// Stats
	ConVar sar_stats_auto_reset;
	ConCommand sar_stats_reset_jumps;
	ConCommand sar_stats_reset_steps;

	// Cheats
	ConVar sar_autojump;
	ConCommand sar_teleport;
	ConCommand sar_teleport_setpos;

	// Config
	ConCommand sar_cvars_save;
	ConCommand sar_cvars_load;

	// TAS
	ConCommand sar_tas_frame_at;
	ConCommand sar_tas_start;
	ConCommand sar_tas_reset;
	ConVar sar_tas_autostart;

	// Others
	ConCommand sar_session;
	ConCommand sar_about;

	// From the game
	ConVar cl_showpos;
	ConVar sv_cheats;
	ConVar sv_footsteps;

	ConVar sv_bonus_challenge;
	ConVar sv_accelerate;
	ConVar sv_airaccelerate;
	ConVar sv_friction;
	ConVar sv_maxspeed;
	ConVar sv_stopspeed;
	ConVar sv_maxvelocity;
	ConVar sv_transition_fade_time;
	ConVar sv_laser_cube_autoaim;
	ConVar ui_loadingscreen_transition_time;
	ConVar hide_gun_when_holding;
}