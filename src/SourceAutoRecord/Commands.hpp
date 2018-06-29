#pragma once
#include "Modules/Command.hpp"
#include "Modules/Variable.hpp"

using namespace Tier1;

namespace Commands {

Variable sar_autorecord;

// Rebinder
Command sar_bind_save;
Command sar_bind_reload;
Command sar_unbind_save;
Command sar_unbind_reload;
Variable sar_save_flag;

// Demo
Command sar_time_demo;
Command sar_time_demos;
Variable sar_time_demo_dev;

// Summary
Command sar_sum_here;
Command sar_sum_stop;
Command sar_sum_result;
Variable sar_sum_during_session;

// Timer
Command sar_timer_start;
Command sar_timer_stop;
Command sar_timer_result;
Variable sar_timer_always_running;

// Timer average
Command sar_avg_start;
Command sar_avg_stop;
Command sar_avg_result;

// Timer checkpoints
Command sar_cps_add;
Command sar_cps_clear;
Command sar_cps_result;

// HUD
Variable sar_hud_text;
Variable sar_hud_position;
Variable sar_hud_angles;
Variable sar_hud_velocity;
Variable sar_hud_session;
Variable sar_hud_last_session;
Variable sar_hud_sum;
Variable sar_hud_timer;
Variable sar_hud_avg;
Variable sar_hud_cps;
Variable sar_hud_demo;
Variable sar_hud_jumps;
Variable sar_hud_portals;
Variable sar_hud_steps;
Variable sar_hud_distance;
Variable sar_hud_trace;
Variable sar_hud_velocity_peak;

Variable sar_hud_default_spacing;
Variable sar_hud_default_padding_x;
Variable sar_hud_default_padding_y;
Variable sar_hud_default_font_index;
Variable sar_hud_default_font_color;

// Stats
Variable sar_stats_auto_reset;
Command sar_stats_reset_jumps;
Command sar_stats_reset_steps;
Command sar_stats_reset_jump_distance;

// Cheats
Variable sar_autojump;
Variable sar_jumpboost;
Variable sar_aircontrol;
Command sar_teleport;
Command sar_teleport_setpos;
Command sar_startbhop;
Command sar_endbhop;

// Config
Command sar_cvars_save;
Command sar_cvars_load;

// TAS
Command sar_tas_frame_at;
Command sar_tas_frame_after;
Command sar_tas_start;
Command sar_tas_reset;
Variable sar_tas_autostart;

// Others
Command sar_session;
Command sar_about;

// Routing
Command sar_trace_a;
Command sar_trace_b;
Command sar_trace_result;
Command sar_velocity_peak;
Command sar_velocity_peak_reset;
Variable sar_velocity_peak_xy;

// From the game
Variable cl_showpos;
Variable sv_cheats;
Variable sv_footsteps;

Variable sv_bonus_challenge;
Variable sv_accelerate;
Variable sv_airaccelerate;
Variable sv_friction;
Variable sv_maxspeed;
Variable sv_stopspeed;
Variable sv_maxvelocity;
Variable sv_transition_fade_time;
Variable sv_laser_cube_autoaim;
Variable ui_loadingscreen_transition_time;
Variable hide_gun_when_holding;
}