#include "Cheats.hpp"

#include "Event.hpp"
#include "Features/Cvars.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Hud/InspectionHud.hpp"
#include "Features/Hud/SpeedrunHud.hpp"
#include "Features/Listener.hpp"
#include "Features/OverlayRender.hpp"
#include "Features/ReloadedFix.hpp"
#include "Features/Routing/EntityInspector.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasTools/AutoJumpTool.hpp"
#include "Features/Tas/TasTools/StrafeTool.hpp"
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
Variable sar_autostrafe("sar_autostrafe", "0", "Automatically strafes in your current wishdir.\n");
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
Variable sar_challenge_autostop("sar_challenge_autostop", "0", 0, 3, "Automatically stops recording demos when the leaderboard opens after a CM run. If 2, automatically appends the run time to the demo name.\n");
Variable sar_show_entinp("sar_show_entinp", "0", "Print all entity inputs to console.\n");
Variable sar_force_qc("sar_force_qc", "0", 0, 1, "When ducking, forces view offset to always be at standing height. Requires sv_cheats to work.\n");
Variable sar_patch_bhop("sar_patch_bhop", "0", 0, 1, "Patches bhop by limiting wish direction if your velocity is too high.\n");
Variable sar_patch_cfg("sar_patch_cfg", "0", 0, 1, "Patches Crouch Flying Glitch.\n");
Variable sar_prevent_ehm("sar_prevent_ehm", "0", 0, 1, "Prevents Entity Handle Misinterpretation (EHM) from happening.\n");
Variable sar_disable_weapon_sway("sar_disable_weapon_sway", "0", 0, 1, "Disables the viewmodel lagging behind.\n");

Variable sv_laser_cube_autoaim;
Variable ui_loadingscreen_transition_time;
Variable ui_loadingscreen_fadein_time;
Variable ui_loadingscreen_mintransition_time;
Variable ui_transition_effect;
Variable ui_transition_time;
Variable hide_gun_when_holding;
Variable cl_viewmodelfov;
Variable r_flashlightbrightness;

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

DECL_AUTO_COMMAND_COMPLETION(sar_fast_load_preset, ({"none", "sla", "normal", "full"}))
CON_COMMAND_F_COMPLETION(sar_fast_load_preset, "sar_fast_load_preset <preset> - sets all loading fixes to preset values\n", FCVAR_DONTRECORD, AUTOCOMPLETION_FUNCTION(sar_fast_load_preset)) {
	if (args.ArgC() != 2) {
		console->Print(sar_fast_load_preset.ThisPtr()->m_pszHelpString);
		return;
	}

	const char *preset = args.Arg(1);

#define CMD(x) engine->ExecuteCommand(x)
	if (!strcmp(preset, "none")) {
		if (!Game::IsSpeedrunMod()) {
			CMD("ui_loadingscreen_transition_time 1.0");
			CMD("ui_loadingscreen_fadein_time 1.0");
			CMD("ui_loadingscreen_mintransition_time 0.5");
		}
		CMD("sar_disable_progress_bar_update 0");
		CMD("sar_prevent_mat_snapshot_recompute 0");
		CMD("sar_loads_uncap 0");
		CMD("sar_loads_norender 0");
	} else if (!strcmp(preset, "sla")) {
		if (!Game::IsSpeedrunMod()) {
			CMD("ui_loadingscreen_transition_time 0.0");
			CMD("ui_loadingscreen_fadein_time 0.0");
			CMD("ui_loadingscreen_mintransition_time 0.0");
		}
		CMD("sar_disable_progress_bar_update 1");
		CMD("sar_prevent_mat_snapshot_recompute 1");
		CMD("sar_loads_uncap 0");
		CMD("sar_loads_norender 0");
	} else if (!strcmp(preset, "normal")) {
		if (!Game::IsSpeedrunMod()) {
			CMD("ui_loadingscreen_transition_time 0.0");
			CMD("ui_loadingscreen_fadein_time 0.0");
			CMD("ui_loadingscreen_mintransition_time 0.0");
		}
		CMD("sar_disable_progress_bar_update 1");
		CMD("sar_prevent_mat_snapshot_recompute 1");
		CMD("sar_loads_uncap 1");
		CMD("sar_loads_norender 0");
	} else if (!strcmp(preset, "full")) {
		if (!Game::IsSpeedrunMod()) {
			CMD("ui_loadingscreen_transition_time 0.0");
			CMD("ui_loadingscreen_fadein_time 0.0");
			CMD("ui_loadingscreen_mintransition_time 0.0");
		}
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

struct DrawLineInfo {
	Vector start, end;
	Color col;
};
static std::vector<DrawLineInfo> g_drawlines;

CON_COMMAND(sar_drawline, "sar_drawline <x> <y> <z> <x> <y> <z> [r] [g] [b] - overlay a line in the world\n") {
	if (args.ArgC() != 7 && args.ArgC() != 10) {
		return console->Print(sar_drawline.ThisPtr()->m_pszHelpString);
	}

	float x0 = atof(args[1]);
	float y0 = atof(args[2]);
	float z0 = atof(args[3]);
	float x1 = atof(args[4]);
	float y1 = atof(args[5]);
	float z1 = atof(args[6]);

	uint8_t r = 255;
	uint8_t g = 255;
	uint8_t b = 255;

	if (args.ArgC() == 10) {
		r = (uint8_t)atoi(args[7]);
		g = (uint8_t)atoi(args[8]);
		b = (uint8_t)atoi(args[9]);
	}

	g_drawlines.push_back({
		{x0, y0, z0},
		{x1, y1, z1},
		{r, g, b},
	});
}


CON_COMMAND(sar_drawline_clear, "sar_drawline_clear - clear all active sar_drawlines\n") {
	g_drawlines.clear();
}

ON_EVENT(RENDER) {
	if (!sv_cheats.GetBool()) return;

	std::map<int, MeshId> meshes;

	for (auto l : g_drawlines) {
		int col_int = *(int *)&l.col;
		if (meshes.count(col_int) == 0) {
			meshes[col_int] = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant(l.col, true));
		}
		OverlayRender::addLine(meshes[col_int], l.start, l.end);
	}
}

CON_COMMAND(sar_getpos, "sar_getpos [slot] [server|client] - get the absolute origin and angles of a particular player from either the server or client. Defaults to slot 0 and server.\n") {
	if (args.ArgC() > 3) {
		return console->Print(sar_getpos.ThisPtr()->m_pszHelpString);
	}

	bool use_serv = true;

	if (args.ArgC() == 3) {
		if (!strcmp(args[2], "client")) {
			use_serv = false;
		} else if (strcmp(args[2], "server")) {
			return console->Print(sar_getpos.ThisPtr()->m_pszHelpString);
		}
	}

	int slot = args.ArgC() >= 2 ? atoi(args[1]) : 0;

	if (slot >= engine->GetMaxClients()) return console->Print("Could not get player at slot %d\n", slot);

	Vector origin;
	QAngle angles;

	if (use_serv) {
		void *player = server->GetPlayer(slot + 1);
		if (!player) return console->Print("Could not get player at slot %d\n", slot);
		origin = server->GetAbsOrigin(player);
		angles = server->GetAbsAngles(player);
	} else {
		void *player = client->GetPlayer(slot + 1);
		if (!player) return console->Print("Could not get player at slot %d\n", slot);
		origin = client->GetAbsOrigin(player);
		angles = client->GetAbsAngles(player);
	}

	console->Print("origin: %.6f %.6f %.6f\n", origin.x, origin.y, origin.z);
	console->Print("angles: %.6f %.6f %.6f\n", angles.x, angles.y, angles.z);
}

CON_COMMAND(sar_geteyepos, "sar_geteyepos [slot] - get the view position (portal shooting origin) and view angles of a certain player.\n") {
	if (args.ArgC() > 2) {
		return console->Print(sar_geteyepos.ThisPtr()->m_pszHelpString);
	}

	int slot = args.ArgC() >= 2 ? atoi(args[1]) : 0;

	if (slot >= engine->GetMaxClients()) return console->Print("Could not get player at slot %d\n", slot);

	Vector eye;
	QAngle angles;

	void *player = server->GetPlayer(slot + 1);
	if (!player) return console->Print("Could not get player at slot %d\n", slot);
	eye = server->GetAbsOrigin(player) + server->GetViewOffset(player) + server->GetPortalLocal(player).m_vEyeOffset;
	angles = engine->GetAngles(slot);

	console->Print("eye: %.6f %.6f %.6f\n", eye.x, eye.y, eye.z);
	console->Print("angles: %.6f %.6f %.6f\n", angles.x, angles.y, angles.z);
}

void Cheats::Init() {
	sv_laser_cube_autoaim = Variable("sv_laser_cube_autoaim");
	ui_loadingscreen_transition_time = Variable("ui_loadingscreen_transition_time");
	ui_loadingscreen_fadein_time = Variable("ui_loadingscreen_fadein_time");
	ui_loadingscreen_mintransition_time = Variable("ui_loadingscreen_mintransition_time");
	ui_transition_effect = Variable("ui_transition_effect");
	ui_transition_time = Variable("ui_transition_time");
	hide_gun_when_holding = Variable("hide_gun_when_holding");
	cl_viewmodelfov = Variable("cl_viewmodelfov");
	r_flashlightbrightness = Variable("r_flashlightbrightness");

	sar_disable_challenge_stats_hud.UniqueFor(SourceGame_Portal2);

	sar_disable_weapon_sway.UniqueFor(SourceGame_Portal2);

	sar_workshop.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
	sar_workshop_update.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);
	sar_workshop_list.UniqueFor(SourceGame_Portal2 | SourceGame_ApertureTag);

	sar_fix_reloaded_cheats.UniqueFor(SourceGame_PortalReloaded);

	cvars->Unlock();

	Variable::RegisterAll();
	Command::RegisterAll();
}
void Cheats::Shutdown() {
	cvars->Lock();

	Variable::UnregisterAll();
	Command::UnregisterAll();
}


// FUN PATCHES :))))))

void Cheats::PatchBhop(int slot, void *player, CUserCmd *cmd) {
	if (!server->AllowsMovementChanges() || !sar_patch_bhop.GetBool()) return;

	TasPlayerInfo info = tasPlayer->GetPlayerInfo(slot, player, cmd);

	float currVel = info.velocity.Length2D();
	float predictVel = autoStrafeTool[info.slot].GetVelocityAfterMove(info, cmd->forwardmove, cmd->sidemove).Length2D();

	float maxSpeed = autoStrafeTool[info.slot].GetMaxSpeed(info, Vector(0, 1));
	if (maxSpeed == 0) return;

	// player surpassed max speed through its own movement - limit wishdir
	if (predictVel > maxSpeed && predictVel > currVel) {
		float mult = (maxSpeed - currVel) / (predictVel - currVel);
		mult = fminf(fmaxf(mult, 0.0f), 1.0f);
		cmd->forwardmove *= mult;
		cmd->sidemove *= mult;
	}
}

ON_EVENT(PROCESS_MOVEMENT) {
	if (!server->AllowsMovementChanges() || !sar_patch_cfg.GetBool()) return;

	auto player = server->GetPlayer(event.slot + 1);
	if (player == nullptr) return;

	auto portalLocal = server->GetPortalLocal(player);

	void *tbeamHandle = reinterpret_cast<void *>(portalLocal.m_hTractorBeam);

	if (!tbeamHandle || (uint32_t)tbeamHandle == (unsigned)Offsets::INVALID_EHANDLE_INDEX) return;

	for (int i = 0; i < 2; i++) {
		int hitboxOffset = i == 0 ? Offsets::m_pShadowCrouch : Offsets::m_pShadowStand;
		auto shadow = *reinterpret_cast<void **>((uintptr_t)player + hitboxOffset);

		// WAKE UP YOU MORON YOU'RE RUINING MY FUNNELS ARGGHHH
		Memory::VMT<void(__rescall *)(void *)>(shadow, Offsets::Wake)(shadow);
	}
}

void Cheats::AutoStrafe(int slot, void *player, CUserCmd *cmd) {
	if (!server->AllowsMovementChanges() || !sar_autostrafe.GetBool()) return;

	if (cmd->forwardmove == 0 && cmd->sidemove == 0) return;

	auto m_MoveType = SE(player)->field<char>("m_MoveType");

	if (m_MoveType == MOVETYPE_NOCLIP) return;

	TasPlayerInfo info = tasPlayer->GetPlayerInfo(slot, player, cmd);

	float angle = Math::AngleNormalize(RAD2DEG(DEG2RAD(info.angles.y) + atan2(-cmd->sidemove, cmd->forwardmove)));

	TasFramebulk fb;
	tasPlayer->playbackInfo.slots[slot].header.version = MAX_SCRIPT_VERSION;
	autoJumpTool[info.slot].SetParams(autoJumpTool[info.slot].ParseParams(std::vector<std::string>{sar_autojump.GetBool() && (cmd->buttons & IN_JUMP) ? "on" : "off"}));
	autoStrafeTool[info.slot].SetParams(autoStrafeTool[info.slot].ParseParams(std::vector<std::string> {"vec", "max", std::to_string(angle) + "deg"}));
	autoStrafeTool[info.slot].Apply(fb, info);

	if (fb.moveAnalog.y > 0.0) {
		cmd->forwardmove = cl_forwardspeed.GetFloat() * fb.moveAnalog.y;
	} else {
		cmd->forwardmove = cl_backspeed.GetFloat() * fb.moveAnalog.y;
	}
	cmd->sidemove = cl_sidespeed.GetFloat() * fb.moveAnalog.x;


}
