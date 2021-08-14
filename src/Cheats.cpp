#include "Cheats.hpp"

#include "Features/Cvars.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Hud/InspectionHud.hpp"
#include "Features/Hud/SpeedrunHud.hpp"
#include "Features/Imitator.hpp"
#include "Features/Listener.hpp"
#include "Features/ReloadedFix.hpp"
#include "Features/ReplaySystem/ReplayPlayer.hpp"
#include "Features/ReplaySystem/ReplayProvider.hpp"
#include "Features/Routing/EntityInspector.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/WorkshopList.hpp"
#include "Game.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Offsets.hpp"

#include <cstring>

Variable sar_autorecord("sar_autorecord", "0", -1, 1, "Enables or disables automatic demo recording.\n");
Variable sar_autojump("sar_autojump", "0", "Enables automatic jumping on the server.\n");
Variable sar_jumpboost("sar_jumpboost", "0", 0,
                       "Enables special game movement on the server.\n"
                       "0 = Default,\n"
                       "1 = Orange Box Engine,\n"
                       "2 = Pre-OBE.\n");
Variable sar_aircontrol("sar_aircontrol", "0", 0, 2, "Enables more air-control on the server.\n");
Variable sar_duckjump("sar_duckjump", "0", "Allows duck-jumping even when fully crouched, similar to prevent_crouch_jump.\n");
Variable sar_disable_challenge_stats_hud("sar_disable_challenge_stats_hud", "0", "Disables opening the challenge mode stats HUD.\n");
Variable sar_disable_steam_pause("sar_disable_steam_pause", "0", "Prevents pauses from steam overlay.\n");
Variable sar_disable_no_focus_sleep("sar_disable_no_focus_sleep", "0", "Does not yield the CPU when game is not focused.\n");
Variable sar_disable_progress_bar_update("sar_disable_progress_bar_update", "0", 0, 2, "Disables excessive usage of progress bar.\n");
Variable sar_prevent_mat_snapshot_recompute("sar_prevent_mat_snapshot_recompute", "0", "Shortens loading times by preventing state snapshot recomputation.\n");
Variable sar_challenge_autostop("sar_challenge_autostop", "0", 0, 2, "Automatically stops recording demos when the leaderboard opens after a CM run. If 2, automatically appends the run time to the demo name.\n");
Variable sar_show_entinp("sar_show_entinp", "0", "Print all entity inputs to console.\n");

Variable sv_laser_cube_autoaim;
Variable ui_loadingscreen_transition_time;
Variable ui_loadingscreen_fadein_time;
Variable ui_loadingscreen_mintransition_time;
Variable hide_gun_when_holding;

// P2 only
CON_COMMAND(sar_togglewait, "sar_togglewait - enables or disables \"wait\" for the command buffer\n") {
	auto state = !*engine->m_bWaitEnabled;
	*engine->m_bWaitEnabled = *engine->m_bWaitEnabled2 = state;
	console->Print("%s wait!\n", (state) ? "Enabled" : "Disabled");
}

// P2, INFRA and HL2 only
#ifdef _WIN32
#	define TRACE_SHUTDOWN_PATTERN "6A 00 68 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? "
#	define TRACE_SHUTDOWN_OFFSET1 3
#	define TRACE_SHUTDOWN_OFFSET2 10
#else
#	define TRACE_SHUTDOWN_PATTERN "C7 44 24 04 00 00 00 00 C7 04 24 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? C7"
#	define TRACE_SHUTDOWN_OFFSET1 11
#	define TRACE_SHUTDOWN_OFFSET2 10
#endif
CON_COMMAND(sar_delete_alias_cmds, "sar_delete_alias_cmds - deletes all alias commands\n") {
	using _Cmd_Shutdown = int (*)();
	static _Cmd_Shutdown Cmd_Shutdown = nullptr;

	if (!Cmd_Shutdown) {
		auto result = Memory::MultiScan(engine->Name(), TRACE_SHUTDOWN_PATTERN, TRACE_SHUTDOWN_OFFSET1);
		if (!result.empty()) {
			for (auto const &addr : result) {
				if (!std::strcmp(*reinterpret_cast<char **>(addr), "Cmd_Shutdown()")) {
					Cmd_Shutdown = Memory::Read<_Cmd_Shutdown>(addr + TRACE_SHUTDOWN_OFFSET2);
					break;
				}
			}
		}
	}

	if (Cmd_Shutdown) {
		Cmd_Shutdown();
	} else {
		console->Print("Unable to find Cmd_Shutdown() function!\n");
	}
}

CON_COMMAND_COMPLETION(sar_fast_load_preset, "set_fast_load_preset <preset> - sets all loading fixes to preset values\n", ({"none", "sla", "normal", "full"})) {
	if (args.ArgC() != 2) {
		console->Print(sar_fast_load_preset.ThisPtr()->m_pszHelpString);
		return;
	}

	const char *preset = args.Arg(1);

#define CMD(x) engine->ExecuteCommand(x)
	if (!strcmp(preset, "none")) {
		CMD("ui_loadingscreen_transition_time 1.0");
		CMD("ui_loadingscreen_fadein_time 1.0");
		CMD("ui_loadingscreen_mintransition_time 0.5");
		CMD("sar_disable_progress_bar_update 0");
		CMD("sar_prevent_mat_snapshot_recompute 0");
		CMD("sar_loads_uncap 0");
		CMD("sar_loads_norender 0");
	} else if (!strcmp(preset, "sla")) {
		CMD("ui_loadingscreen_transition_time 0.0");
		CMD("ui_loadingscreen_fadein_time 0.0");
		CMD("ui_loadingscreen_mintransition_time 0.0");
		CMD("sar_disable_progress_bar_update 1");
		CMD("sar_prevent_mat_snapshot_recompute 1");
		CMD("sar_loads_uncap 0");
		CMD("sar_loads_norender 0");
	} else if (!strcmp(preset, "normal")) {
		CMD("ui_loadingscreen_transition_time 0.0");
		CMD("ui_loadingscreen_fadein_time 0.0");
		CMD("ui_loadingscreen_mintransition_time 0.0");
		CMD("sar_disable_progress_bar_update 1");
		CMD("sar_prevent_mat_snapshot_recompute 1");
		CMD("sar_loads_uncap 1");
		CMD("sar_loads_norender 0");
	} else if (!strcmp(preset, "full")) {
		CMD("ui_loadingscreen_transition_time 0.0");
		CMD("ui_loadingscreen_fadein_time 0.0");
		CMD("ui_loadingscreen_mintransition_time 0.0");
		CMD("sar_disable_progress_bar_update 2");
		CMD("sar_prevent_mat_snapshot_recompute 1");
		CMD("sar_loads_uncap 1");
		CMD("sar_loads_norender 1");
	} else {
		console->Print("Unknown preset %s!\n", preset);
		console->Print(sar_fast_load_preset.ThisPtr()->m_pszHelpString);
	}
#undef CMD
}

CON_COMMAND(sar_clear_lines, "sar_clear_lines - clears all active drawline overlays\n") {
	// So, hooking this would be really annoying, however Valve's code
	// is dumb and bad and only allows 20 lines (after which it'll start
	// overwriting old ones), so let's just draw 20 zero-length lines!
	for (int i = 0; i < 20; ++i) {
		engine->ExecuteCommand("drawline 0 0 0 0 0 0", true);
	}
}

void Cheats::Init() {
	sv_laser_cube_autoaim = Variable("sv_laser_cube_autoaim");
	ui_loadingscreen_transition_time = Variable("ui_loadingscreen_transition_time");
	ui_loadingscreen_fadein_time = Variable("ui_loadingscreen_fadein_time");
	ui_loadingscreen_mintransition_time = Variable("ui_loadingscreen_mintransition_time");
	hide_gun_when_holding = Variable("hide_gun_when_holding");

	sar_disable_challenge_stats_hud.UniqueFor(SourceGame_Portal2);

	sar_workshop.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
	sar_workshop_update.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
	sar_workshop_list.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);

	sar_fix_reloaded_cheats.UniqueFor(SourceGame_PortalReloaded);

	cvars->Unlock();

	Variable::RegisterAll();
	Command::RegisterAll();

	// putting this here is really dumb but i dont even care any
	// more
	sar_hud_text.AddCallBack(sar_hud_text_callback);
}
void Cheats::Shutdown() {
	cvars->Lock();

	Variable::UnregisterAll();
	Command::UnregisterAll();
}
