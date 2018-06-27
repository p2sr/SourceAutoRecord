#pragma once
#include "Modules/ConCommand.hpp"
#include "Modules/ConVar.hpp"
#include "Modules/Cvar.hpp"
#include "Modules/Tier1.hpp"

#include "Callbacks.hpp"
#include "Commands.hpp"
#include "Interfaces.hpp"

#include "Game.hpp"

using namespace Commands;
using namespace Cvar;

namespace Cheats
{
	void Create()
	{
		sar_autorecord = CreateBoolean(
			"sar_autorecord",
			"0",
			"Enables automatic demo recording.\n");

		// Rebinder
		sar_bind_save = CreateCommandArgs(
			"sar_bind_save", Callbacks::BindSaveRebinder,
			"Automatic save rebinding when server has loaded. File indexing will be synced when recording demos. Usage: sar_bind_save <key> [save_name]\n");
		sar_bind_reload = CreateCommandArgs(
			"sar_bind_reload", Callbacks::BindReloadRebinder,
			"Automatic save-reload rebinding when server has loaded. File indexing will be synced when recording demos. Usage: sar_bind_reload <key> [save_name]\n");
		sar_unbind_save = CreateCommand(
			"sar_unbind_save", Callbacks::UnbindSaveRebinder,
			"Unbinds current save rebinder.\n");
		sar_unbind_reload = CreateCommand(
			"sar_unbind_reload", Callbacks::UnbindReloadRebinder,
			"Unbinds current save-reload rebinder.\n");
		sar_save_flag = CreateString(
			"sar_save_flag", "#SAVE#",
			"Echo message when using sar_bind_save. Default is \"#SAVE#\", a SourceRuns standard. Keep this empty if no echo message should be binded.\n");

		// Demo parsing
		sar_time_demo = CreateCommandArgs(
			"sar_time_demo",
			Callbacks::PrintDemoInfo,
			"Parses a demo and prints some information about it.\n");
		sar_time_demos = CreateCommandArgs(
			"sar_time_demos",
			Callbacks::PrintDemoInfos,
			"Parses multiple demos and prints the total sum of them.\n");
		sar_time_demo_dev = CreateFloat(
			"sar_time_demo_dev",
			"0",
			0,
			"Printing mode when using sar_time_demo. 0 = default, 1 = console commands, 2 = console commands & packets.\n");

		// Summary
		sar_sum_here = CreateCommand(
			"sar_sum_here",
			Callbacks::StartSummary,
			"Starts counting total ticks of sessions.\n");
		sar_sum_stop = CreateCommand(
			"sar_sum_stop",
			Callbacks::StopSummary,
			"Stops summary counter.\n");
		sar_sum_result = CreateCommand(
			"sar_sum_result",
			Callbacks::PrintSummary,
			"Prints result of summary.\n");
		sar_sum_during_session = CreateBoolean(
			"sar_sum_during_session",
			"1",
			"Updates the summary counter automatically during a session.\n");

		// Timer
		sar_timer_start = CreateCommand(
			"sar_timer_start",
			Callbacks::StartTimer,
			"Starts timer.\n");
		sar_timer_stop = CreateCommand(
			"sar_timer_stop",
			Callbacks::StopTimer,
			"Stops timer.\n");
		sar_timer_result = CreateCommand(
			"sar_timer_result",
			Callbacks::PrintTimer,
			"Prints result of timer.\n");
		sar_timer_always_running = CreateBoolean(
			"sar_timer_always_running",
			"1",
			"Timer will save current value when disconnecting.\n");

		// Timer average
		sar_avg_start = CreateCommand(
			"sar_avg_start",
			Callbacks::StartAverage,
			"Starts calculating the average when using timer.\n");
		sar_avg_stop = CreateCommand(
			"sar_avg_stop",
			Callbacks::StopAverage,
			"Stops average calculation.\n");
		sar_avg_result = CreateCommand(
			"sar_avg_result",
			Callbacks::PrintAverage,
			"Prints result of average.\n");

		// Timer checkpoints
		sar_cps_add = CreateCommand(
			"sar_cps_add",
			Callbacks::AddCheckpoint,
			"Saves current time of timer.\n");
		sar_cps_clear = CreateCommand(
			"sar_cps_clear",
			Callbacks::ClearCheckpoints,
			"Resets saved times of timer.\n");
		sar_cps_result = CreateCommand(
			"sar_cps_result",
			Callbacks::PrintCheckpoints,
			"Prints result of timer checkpoints.\n");

		// HUD
		sar_hud_text = CreateString(
			"sar_hud_text",
			"",
			"Draws specified text when not empty.\n");
		sar_hud_position = CreateFloat(
			"sar_hud_position",
			"0",
			0,
			"Draws absolute position of the client.\n");
		sar_hud_angles = CreateBoolean(
			"sar_hud_angles",
			"0",
			"Draws absolute view angles of the client.\n");
		sar_hud_velocity = CreateFloat(
			"sar_hud_velocity",
			"0",
			0,
			"Draws velocity of the client. 0 = default, 1 = x/y/z , 2 = x/y\n");
		sar_hud_session = CreateBoolean(
			"sar_hud_session",
			"0",
			"Draws current session value.\n");
		sar_hud_last_session = CreateBoolean(
			"sar_hud_last_session",
			"0",
			"Draws value of latest completed session.\n");
		sar_hud_sum = CreateBoolean(
			"sar_hud_sum",
			"0",
			"Draws summary value of sessions.\n");
		sar_hud_timer = CreateBoolean(
			"sar_hud_timer",
			"0",
			"Draws current value of timer.\n");
		sar_hud_avg = CreateBoolean(
			"sar_hud_avg",
			"0",
			"Draws calculated average of timer.\n");
		sar_hud_cps = CreateBoolean(
			"sar_hud_cps",
			"0",
			"Draws latest checkpoint of timer.\n");
		sar_hud_demo = CreateBoolean(
			"sar_hud_demo",
			"0",
			"Draws name, tick and time of current demo.\n");
		sar_hud_jumps = CreateBoolean(
			"sar_hud_jumps",
			"0",
			"Draws total jump count.\n");
		sar_hud_portals = CreateBoolean(
			"sar_hud_portals",
			"0",
			"Draws total portal count.\n");
		sar_hud_steps = CreateBoolean(
			"sar_hud_steps",
			"0",
			"Draws total step count.\n");
		sar_hud_distance = CreateBoolean(
			"sar_hud_distance",
			"0",
			"Draws calculated jump distance.\n");
		sar_hud_default_spacing = CreateFloat(
			"sar_hud_default_spacing",
			"4",
			0,
			"Spacing between elements of HUD.\n");
		sar_hud_default_padding_x = CreateFloat(
			"sar_hud_default_padding_x",
			"2",
			0,
			"X padding of HUD.\n");
		sar_hud_default_padding_y = CreateFloat(
			"sar_hud_default_padding_y",
			"2",
			0,
			"Y padding of HUD.\n");
		sar_hud_default_font_index = CreateFloat(
			"sar_hud_default_font_index",
			"0",
			0,
			"Font index of HUD.\n");
		sar_hud_default_font_color = CreateString(
			"sar_hud_default_font_color",
			"255 255 255 255",
			"RGBA font color of HUD.\n");

		// Stats
		sar_stats_auto_reset = CreateFloat(
			"sar_stats_auto_reset",
			"0",
			0,
			"Resets all stats automatically. 0 = default, 1 = restart or disconnect only, 2 = any load & sar_timer_start. Note: Portal counter is not part of the \"stats\" feature.\n");
		sar_stats_reset_jumps = CreateCommand(
			"sar_stats_reset_jumps",
			Callbacks::ResetJumps,
			"Resets jump counter.\n");
		sar_stats_reset_steps = CreateCommand(
			"sar_stats_reset_steps",
			Callbacks::ResetSteps,
			"Resets step counter.\n");

		// Cheats
		sar_autojump = CreateBoolean(
			"sar_autojump",
			"0",
			"Enables automatic jumping on the server.\n");
		if (Game::Version == Game::Portal2) {
			sar_jumpboost = CreateFloat(
				"sar_jumpboost",
				"0",
				0,
				"Enables special game movement on the server. 0 = Default, 1 = Orange Box Engine, 2 = Pre-OBE\n");
			sar_aircontrol = CreateBoolean(
				"sar_aircontrol",
				"0",
				"Enables more air-control on the server.\n");
		}
		sar_teleport = CreateCommand(
			"sar_teleport",
			Callbacks::Teleport,
			"Teleports the player to the last saved location.\n");
		sar_teleport_setpos = CreateCommand(
			"sar_teleport_setpos",
			Callbacks::SetTeleport,
			"Saves current location for teleportation.\n");
        if (Game::Version == Game::TheStanleyParable) {
            sar_startbhop = CreateCommandArgs(
			    "+bhop",
			    Callbacks::IN_BhopDown,
                "Jump.");
            sar_endbhop = CreateCommandArgs(
			    "-bhop",
			    Callbacks::IN_BhopUp,
                "Jump.");
        }

		// TAS
		sar_tas_frame_at = CreateCommandArgs(
			"sar_tas_frame_at",
			Callbacks::AddFrameAtTas,
			"Adds a command frame to TAS (absolute).\n");
		sar_tas_frame_after = CreateCommandArgs(
			"sar_tas_frame_after",
			Callbacks::AddFrameAfterTas,
			"Adds a command frame to TAS (relative).\n");
		sar_tas_start = CreateCommand(
			"sar_tas_start",
			Callbacks::StartTas,
			"Starts TAS.\n");
		sar_tas_reset = CreateCommand(
			"sar_tas_reset",
			Callbacks::ResetTas,
			"Resets TAS.\n");
		sar_tas_autostart = CreateBoolean(
			"sar_tas_autostart",
			"0",
			"Starts TAS automatically on first frame after a load.\n");

		// Others
		sar_session = CreateCommand(
			"sar_session",
			Callbacks::PrintSession,
			"Prints the current tick of the server since it has loaded.\n");
		sar_about = CreateCommand(
			"sar_about",
			Callbacks::PrintAbout,
			"Prints info about this plugin.\n");
		sar_cvars_save = CreateCommand(
			"sar_cvars_save",
			Callbacks::SaveCvars,
			"Saves important SAR cvars.\n");
		sar_cvars_load = CreateCommand(
			"sar_cvars_load",
			Callbacks::LoadCvars,
			"Loads important SAR cvars.\n");
		sar_trace_a = CreateCommand(
			"sar_trace_a",
			Callbacks::SaveTracerA,
			"Saves location A for tracing.\n");
		sar_trace_b = CreateCommand(
			"sar_trace_b",
			Callbacks::SaveTracerB,
			"Saves location B for tracing.\n");
		sar_trace_result = CreateCommand(
			"sar_trace_result",
			Callbacks::PrintTracerResult,
			"Prints tracing result.\n");
		sar_max_vel = CreateCommand(
			"sar_max_vel",
			Callbacks::PrintMaxVel,
			"Prints latest maximum velocity.\n");
		sar_max_vel_xy = CreateBoolean(
			"sar_max_vel_xy",
			"0",
			"Saves 2D velocity instead.\n");
		sar_max_vel_reset = CreateCommand(
			"sar_max_vel_reset",
			Callbacks::ResetMaxVel,
			"Resets saved maximum velocity.\n");

		// From the game
		cl_showpos = GetConVar("cl_showpos");
		sv_cheats = GetConVar("sv_cheats");
		sv_footsteps = GetConVar("sv_footsteps");

		sv_bonus_challenge = GetConVar("sv_bonus_challenge");
		sv_accelerate = GetConVar("sv_accelerate");
		sv_airaccelerate = GetConVar("sv_airaccelerate");
		sv_friction = GetConVar("sv_friction");
		sv_maxspeed = GetConVar("sv_maxspeed");
		sv_stopspeed = GetConVar("sv_stopspeed");
		sv_maxvelocity = GetConVar("sv_maxvelocity");

		if (Game::Version == Game::Portal2) {
			sv_transition_fade_time = GetConVar("sv_transition_fade_time");
			sv_laser_cube_autoaim = GetConVar("sv_laser_cube_autoaim");
			ui_loadingscreen_transition_time = GetConVar("ui_loadingscreen_transition_time");
			hide_gun_when_holding = GetConVar("hide_gun_when_holding");
		}

		Console::DevMsg("SAR: Created %i ConVars and %i ConCommands!\n", Tier1::ConVarCount, Tier1::ConCommandCount);
	}
	void Unlock(ConVar& var, bool asCheat = true)
	{
		var.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		if (asCheat) var.AddFlag(FCVAR_CHEAT);
	}
	void UnlockAll()
	{
		Unlock(sv_accelerate);
		Unlock(sv_airaccelerate);
		Unlock(sv_friction);
		Unlock(sv_maxspeed);
		Unlock(sv_stopspeed);
		Unlock(sv_maxvelocity);
		Unlock(sv_footsteps);

		if (Game::Version == Game::Portal2) {
			// Challenge mode will reset every cheat automatically
			// Flagging this as a cheat would break cm. I think
			// it's impossible to abuse this anyway
			Unlock(sv_bonus_challenge, false);

			Unlock(sv_transition_fade_time);
			Unlock(sv_laser_cube_autoaim);
			Unlock(ui_loadingscreen_transition_time);

			// Not a real cheat, right?
			Unlock(hide_gun_when_holding, false);
		}
	}
}