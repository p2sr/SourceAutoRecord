#pragma once
#include "minhook/MinHook.h"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/DemoPlayer.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"
#include "Modules/Server.hpp"

#include "Modules/ConCommand.hpp"
#include "Modules/ConVar.hpp"

#include "Callbacks.hpp"
#include "Commands.hpp"
#include "Hooks.hpp"
#include "Offsets.hpp"
#include "Patterns.hpp"
#include "Utils.hpp"

using namespace Commands;

namespace SAR
{
	bool LoadEngine()
	{
		auto enc = Scan(Patterns::EngineClientPtr);
		auto ggd = Scan(Patterns::GetGameDir);
		auto crt = Scan(Patterns::CurtimePtr);
		auto ldg = Scan(Patterns::LoadgamePtr);
		auto mpn = Scan(Patterns::MapnamePtr);
		auto cns = Scan(Patterns::CurrentStatePtr);
		auto drc = Scan(Patterns::DemoRecorderPtr);
		auto ins = Scan(Patterns::InputSystemPtr);
		auto ksb = Scan(Patterns::Key_SetBinding);
		auto dpl = Scan(Patterns::DemoPlayerPtr);

		if (!enc.Found || !ggd.Found || !crt.Found || !ldg.Found || !mpn.Found
			|| !cns.Found || !drc.Found || !ins.Found || !ksb.Found || !dpl.Found)
			return false;

		Engine::Set(enc.Address, ggd.Address, crt.Address, ldg.Address, mpn.Address, cns.Address);
		DemoRecorder::Set(drc.Address);
		InputSystem::Set(ins.Address, ksb.Address);
		DemoPlayer::Set(dpl.Address);
		return true;
	}
	bool LoadTier1()
	{
		auto cvr = Scan(Patterns::CvarPtr);
		auto cnv = Scan(Patterns::ConVar_Ctor3);
		auto cnc = Scan(Patterns::ConCommand_Ctor1);
		auto cnc2 = Scan(Patterns::ConCommand_Ctor2);

		if (!cvr.Found || !cnv.Found || !cnc.Found || !cnc2.Found)
			return false;

		Cvar::Set(cvr.Address);
		Tier1::SetConVar(cnv.Address);
		Tier1::SetConCommand(cnc.Address, cnc2.Address);
		return true;
	}
	bool LoadClient()
	{
		auto sts = Scan(Patterns::SetSize);
		auto mss = Scan(Patterns::MatSystemSurfacePtr);

		if (!sts.Found || !mss.Found)
			return false;

		Client::Set(sts.Address);
		Surface::Set(mss.Address);
		return true;
	}
	void RegisterCommands()
	{
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
		sar_hud_last_demo = CreateBoolean(
			"sar_hud_last_demo",
			"0",
			"Draws value of latest completed demo.\n");
		sar_hud_jumps = CreateBoolean(
			"sar_hud_jumps",
			"0",
			"Draws total jump count.\n");
		sar_hud_uses = CreateBoolean(
			"sar_hud_uses",
			"0",
			"Draws total use count.\n");

		// Stats
		sar_stats_auto_reset = CreateFloat(
			"sar_stats_auto_reset",
			"0",
			0,
			"Resets all stats automatically. 0 = default, 1 = disconnect, 2 = disconnect & sar_timer_start.\n");
		sar_stats_reset_jumps = CreateCommand(
			"sar_stats_reset_jumps",
			Callbacks::ResetJumps,
			"Resets jump counter.\n");
		sar_stats_reset_uses = CreateCommand(
			"sar_stats_reset_uses",
			Callbacks::ResetUses,
			"Resets use counter.\n");

		// Cheats
		sar_autojump = CreateBoolean(
			"sar_autojump",
			"0",
			"Enables automatic jumping on the server.\n");
		sar_aircontrol = CreateBoolean(
			"sar_aircontrol",
			"0",
			"Enables \"more air-control\" on the server.\n");

		// Others
		sar_session = CreateCommand(
			"sar_session",
			Callbacks::PrintSession,
			"Prints the current tick of the server since it has loaded.\n");
		sar_about = CreateCommand(
			"sar_about",
			Callbacks::PrintAbout,
			"Prints info about this plugin.\n");

		// From the game
		cl_showpos = ConVar("cl_showpos");
		sv_cheats = ConVar("sv_cheats");
		sv_bonus_challenge = ConVar("sv_bonus_challenge");
		sv_accelerate = ConVar("sv_accelerate");
		sv_airaccelerate = ConVar("sv_airaccelerate");
		sv_friction = ConVar("sv_friction");
		sv_maxspeed = ConVar("sv_maxspeed");
		sv_stopspeed = ConVar("sv_stopspeed");
	}
	void EnableGameCheats()
	{
		sv_bonus_challenge.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_accelerate.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_airaccelerate.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_friction.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_maxspeed.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);
		sv_stopspeed.RemoveFlag(FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN);

		// Nobody likes cheaters
		sv_bonus_challenge.AddFlag(FCVAR_CHEAT);
		sv_accelerate.AddFlag(FCVAR_CHEAT);
		sv_airaccelerate.AddFlag(FCVAR_CHEAT);
		sv_friction.AddFlag(FCVAR_CHEAT);
		sv_maxspeed.AddFlag(FCVAR_CHEAT);
		sv_stopspeed.AddFlag(FCVAR_CHEAT);
	}
	void LoadPatches()
	{
		switch (Offsets::Variant) {
		// Portal 2 6879
		case 0:

			// Fix limited help string output in ConVar_PrintDescription
			// Changing 80 to 1024
			const char* thanksForNothingValve = "%-80s - %.1024s\n";

			auto prd = Scan(Patterns::ConVar_PrintDescription);
			if (prd.Found && WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(prd.Address), &thanksForNothingValve, 4, 0)) {
				Console::DevMsg("SAR: Patched ConVar_PrintDescription!\n");
			}

			// Fix executing commands in demo playback
			// Removing Cbuf_AddText and Cbuf_Execute
			BYTE ignoreEvilCommandsInDemos[22] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

			auto rdp = Scan(Patterns::ReadPacket);
			if (rdp.Found && WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(rdp.Address), ignoreEvilCommandsInDemos, 22, 0)) {
				Console::DevMsg("SAR: Patched CDemoPlayer::ReadPacket at 0x%p!\n", rdp.Address);
			}

			// Remove the default ConMsg when demo file gets closed
			BYTE weAlreadyPrintABetterMessage[29] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

			auto cdf = Scan("engine.dll", "D9 86 ? ? ? ? 8B 8E ? ? ? ? 51");
			if (cdf.Found && WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(cdf.Address), weAlreadyPrintABetterMessage, 29, 0)) {
				Console::DevMsg("SAR: Patched CDemoPlayer::CloseDemoFile at 0x%p!\n", cdf.Address);
			}

			break;
		}
	}
}