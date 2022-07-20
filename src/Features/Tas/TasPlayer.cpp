#include "TasPlayer.hpp"

#include "Features/Session.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasTool.hpp"
#include "Features/Tas/TasServer.hpp"
#include "Features/Tas/TasTools/CheckTool.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/RNGManip.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/FileSystem.hpp"
#include "Event.hpp"
#include "Variable.hpp"

#include <climits>
#include <filesystem>
#include <fstream>

#ifndef _WIN32
#include <dirent.h>
#endif

Variable sar_tas_debug("sar_tas_debug", "0", 0, 2, "Debug TAS informations. 0 - none, 1 - basic, 2 - all.\n");
Variable sar_tas_dump_usercmd("sar_tas_dump_usercmd", "0", "Dump TAS-generated usercmds to a file.\n");
Variable sar_tas_dump_player_info("sar_tas_dump_player_info", "0", "Dump player info for each tick of TAS playback to a file.\n");
Variable sar_tas_tools_enabled("sar_tas_tools_enabled", "1", "Enables tool processing for TAS script making.\n");
Variable sar_tas_tools_force("sar_tas_tools_force", "0", "Force tool playback for TAS scripts; primarily for debugging.\n");
Variable sar_tas_autosave_raw("sar_tas_autosave_raw", "1", "Enables automatic saving of raw, processed TAS scripts.\n");
Variable sar_tas_pauseat("sar_tas_pauseat", "0", 0, "Pauses the TAS playback on specified tick.\n");
Variable sar_tas_skipto("sar_tas_skipto", "0", 0, "Fast-forwards the TAS playback until given playback tick.\n");
Variable sar_tas_playback_rate("sar_tas_playback_rate", "1.0", 0.02, "The rate at which to play back TAS scripts.\n");
Variable sar_tas_restore_fps("sar_tas_restore_fps", "1", "Restore fps_max and host_framerate after TAS playback.\n");
Variable sar_tas_interpolate("sar_tas_interpolate", "0", "Preserve client interpolation in TAS playback.\n");

TasPlayer *tasPlayer;

std::string TasFramebulk::ToString() {
	std::string output = "[" + std::to_string(tick) + "] mov: (" + std::to_string(moveAnalog.x) + " " + std::to_string(moveAnalog.y) + "), ang:" + std::to_string(viewAnalog.x) + " " + std::to_string(viewAnalog.y) + "), btns:";
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		output += (buttonStates[i]) ? "1" : "0";
	}
	output += ", cmds: ";
	for (std::string command : commands) {
		output += command + ";";
	}
	output += ", tools:";
	for (TasToolCommand toolCmd : toolCmds) {
		output += " {" + std::string(toolCmd.tool->GetName()) + "}";
	}
	return output;
}

void SetPlaybackVars(bool active) {
	static bool was_active;
	static bool saved_fps;
	static int old_forceuser;
	static int old_fpsmax;
	static int old_hostframerate;
	static bool old_interpolate;
	static bool old_motionblur;

	static Variable cl_interpolate("cl_interpolate");
	static Variable mat_motion_blur_enabled("mat_motion_blur_enabled");

	bool tools = tasPlayer->IsUsingTools(0) || tasPlayer->IsUsingTools(1);

	if (active && !was_active) {
		old_forceuser = in_forceuser.GetInt();
		old_hostframerate = host_framerate.GetInt();
		old_interpolate = cl_interpolate.GetBool();
		old_motionblur = mat_motion_blur_enabled.GetBool();
		in_forceuser.SetValue(tasPlayer->coopControlSlot >= 0 ? tasPlayer->coopControlSlot : 100);
		host_framerate.SetValue(60);
		if (!sar_tas_interpolate.GetBool() && tools) {
			cl_interpolate.SetValue(false);
			mat_motion_blur_enabled.SetValue(false);
		}
	} else if (!active && was_active) {
		in_forceuser.SetValue(old_forceuser);
		if (sar_tas_restore_fps.GetBool()) {
			engine->SetSkipping(false);
			host_framerate.SetValue(old_hostframerate);
			if (saved_fps) {
				fps_max.SetValue(old_fpsmax);
				session->oldFpsMax = old_fpsmax; // In case we're restoring during an uncapped load
			}
		}
		cl_interpolate.SetValue(old_interpolate);
		mat_motion_blur_enabled.SetValue(old_motionblur);
		saved_fps = false;
	}

	if (active) {
		// This likes to re-enable itself just after map load. Make sure it
		// always stays off
		mat_motion_blur_enabled.SetValue(false);
	}

	// Don't save fps_max in loads, or uncap might get in the way
	// Wait for the session to start instead
	if (session->isRunning && active && !saved_fps) {
		old_fpsmax = fps_max.GetInt();
		saved_fps = true;
	}

	if (saved_fps && active && tasPlayer->IsReady()) {
		if (tasPlayer->GetTick() < sar_tas_skipto.GetInt()) {
			engine->SetSkipping(true);
			fps_max.SetValue(0);
		} else if (tasPlayer->GetTick() >= sar_tas_skipto.GetInt()) {
			engine->SetSkipping(false);
			fps_max.SetValue((int)(sar_tas_playback_rate.GetFloat() * 60.0f));
		}
	}

	was_active = active;
}

ON_EVENT(FRAME) {
	bool tools = tasPlayer->IsUsingTools(0) || tasPlayer->IsUsingTools(1);
	if (tasPlayer->IsRunning() && !sar_tas_interpolate.GetBool() && tools) {
		for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
			// check for prop_portal on the server cuz i can't figure out how
			// to do it client-side lol
			void *ent = server->m_EntPtrArray[i].m_pEntity;
			if (!ent) continue;
			const char *classname = server->GetEntityClassName(ent);
			if (!classname || strcmp(classname, "prop_portal")) continue;

			// it's a portal, so get the corresponding client entity and
			// insta-open it
			void *cl_ent = client->GetPlayer(i);
			if (!cl_ent) continue;
			// yes these are hardcoded offsets no i dont care anymore
#ifdef _WIN32
			*(float *)((char *)cl_ent + 13584) = 0.0f; // m_fStaticAmount
			*(float *)((char *)cl_ent + 13588) = 0.0f; // m_fSecondaryStaticAmount
			*(float *)((char *)cl_ent + 13592) = 1.0f; // m_fOpenAmount
#else
			*(float *)((char *)cl_ent + 13552) = 0.0f; // m_fStaticAmount
			*(float *)((char *)cl_ent + 13556) = 0.0f; // m_fSecondaryStaticAmount
			*(float *)((char *)cl_ent + 13560) = 1.0f; // m_fOpenAmount
#endif
		}
	}
}

TasPlayer::TasPlayer()
	: startInfo({TasStartType::StartImmediately, ""}) {
}

TasPlayer::~TasPlayer() {
	framebulkQueue[0].clear();
	framebulkQueue[1].clear();
}

#ifdef _WIN32
#define getSaveDir engine->GetSaveDirName
#else
static std::string findSubCapitalization(const char *base, const char *sub) {
	DIR *d = opendir(base);
	if (!d) return std::string(sub);

	struct dirent *ent;
	while ((ent = readdir(d))) {
		if (!strcasecmp(ent->d_name, sub)) {
			closedir(d);
			return std::string(ent->d_name);
		}
	}

	closedir(d);
	return std::string(sub);
}
// Apparently, GetSaveDirName has the wrong capitalization sometimes
// kill me
static std::string getSaveDir() {
	std::string path = std::string(engine->GetGameDirectory()) + "/";
	std::string dir = findSubCapitalization(path.c_str(), "save");
	dir += (engine->GetSaveDirName() + 4);
	return dir;
}
#endif

static bool mapExists(std::string name) {
	name = "/maps/" + name + ".bsp";
	return fileSystem->FileExistsSomewhere(name);
}

void TasPlayer::Activate() {
	//reset the controller before using it
	Stop(true);

	for (TasTool *tool : TasTool::GetList(0)) {
		tool->Reset();
	}
	for (TasTool *tool : TasTool::GetList(1)) {
		tool->Reset();
	}
	processedFramebulks[0].clear();
	processedFramebulks[1].clear();
	usercmdDebugs[0].clear();
	usercmdDebugs[1].clear();
	playerInfoDebugs[0].clear();
	playerInfoDebugs[1].clear();

	g_tas_check_replays = 0;

	active = true;
	startTick = -1;
	currentTick = 0;

	lastTick = 0;
	for (int slot = 0; slot < (this->isCoop ? 2 : 1); ++slot) {
		for (TasFramebulk fb : framebulkQueue[slot]) {
			if (fb.tick > lastTick) {
				lastTick = fb.tick;
			}
		}
	}

	ready = false;
	if (startInfo.type == ChangeLevel || startInfo.type == ChangeLevelCM) {
		//check if map exists
		if (mapExists(startInfo.param)) {
			if (session->isRunning && engine->GetCurrentMapName() == startInfo.param) {
				engine->ExecuteCommand("restart_level");
			} else {
				const char *start;
				if (this->isCoop) {
					if (session->isRunning && engine->IsCoop() && !engine->IsOrange() && engine->GetCurrentMapName() != "") {
						start = "changelevel";
					} else {
						if (session->isRunning) engine->ExecuteCommand("disconnect");
						start = "ss_map";
					}
				} else {
					if (session->isRunning) engine->ExecuteCommand("disconnect");
					start = "map";
				}
				std::string cmd = Utils::ssprintf("%s %s", start, startInfo.param.c_str());
				engine->ExecuteCommand(cmd.c_str());
			}
		} else {
			console->ColorMsg(Color(255, 100, 100), "Cannot activate TAS file - unknown map '%s.'\n", startInfo.param.c_str());
			Stop();
			return;
		}
	} else if (startInfo.type == LoadQuicksave) {
		//check if save file exists
		std::string savePath = std::string(engine->GetGameDirectory()) + "/" + getSaveDir() + startInfo.param + ".sav";
		std::ifstream savef(savePath);
		if (savef.good()) {
			std::string cmd = "load ";
			cmd += startInfo.param;
			engine->ExecuteCommand(cmd.c_str());
		} else {
			console->ColorMsg(Color(255, 100, 100), "Cannot activate TAS file - unknown save file '%s'.\n", startInfo.param.c_str());
			Stop();
			return;
		}
		savef.close();
	}

	console->Print("TAS script has been activated.\n");
	if (sar_tas_debug.GetInt() > 0) {
		console->Print("Length: %d ticks\n", lastTick + 1);
	}
}

void TasPlayer::Start() {
	console->Print("TAS script has been started.\n");
	if (sar_tas_debug.GetInt() > 0) {
		console->Print("Length: %d ticks\n", lastTick + 1);
	}
	processedFramebulks[0].clear();
	processedFramebulks[1].clear();
	usercmdDebugs[0].clear();
	usercmdDebugs[1].clear();
	playerInfoDebugs[0].clear();
	playerInfoDebugs[1].clear();

	if (startInfo.type == ChangeLevelCM) {
		sv_bonus_challenge.SetValue(1);
	}

	ready = true;
	currentTick = 0;
	startTick = -1;

	SetPlaybackVars(true);
}

void TasPlayer::PostStart() {
	startTick = server->gpGlobals->tickcount;
	if (sar_tas_debug.GetInt() > 1) {
		console->Print("Start tick: %d\n", startTick);
	}
	tasControllers[0]->Enable();
	if (this->isCoop) tasControllers[1]->Enable();
	engine->ExecuteCommand("phys_timescale 1", true);  // technically it was supposed to fix the consistency issue

	Event::Trigger<Event::TAS_START>({});
}

void TasPlayer::Stop(bool interrupted) {
	if (active && ready) {
		console->Print(
			"TAS script has %s after %d ticks.\n", 
			interrupted ? "been interrupted" : "ended", 
			currentTick
		);

		if (sar_tas_autosave_raw.GetBool()) {
			SaveProcessedFramebulks();
		}

		SaveUsercmdDebugs(0);
		SaveUsercmdDebugs(1);

		SavePlayerInfoDebugs(0);
		SavePlayerInfoDebugs(1);

		Event::Trigger<Event::TAS_END>({});
	} else {
		console->Print("TAS player is no longer active.\n");
	}

	active = false;
	ready = false;
	currentTick = 0;
	tasControllers[0]->Disable();
	tasControllers[1]->Disable();

	SetPlaybackVars(false);

	engine->SetAdvancing(false);
}

void TasPlayer::Pause() {
	if (active && ready) engine->SetAdvancing(true);
}

void TasPlayer::Resume() {
	if (active && ready) engine->SetAdvancing(false);
}

void TasPlayer::AdvanceFrame() {
	if (active && ready) engine->AdvanceTick();
}

// returns raw framebulk that should be used for given tick
TasFramebulk TasPlayer::GetRawFramebulkAt(int slot, int tick) {
	int closestTime = INT_MAX;
	TasFramebulk closest;
	for (TasFramebulk framebulk : framebulkQueue[slot]) {
		int timeDist = tick - framebulk.tick;
		if (timeDist >= 0 && timeDist < closestTime) {
			closestTime = timeDist;
			closest = framebulk;
		}
	}
	return closest;
}

TasPlayerInfo TasPlayer::GetPlayerInfo(void *player, CUserCmd *cmd) {
	TasPlayerInfo pi;

	int m_nOldButtons = SE(player)->field<int>("m_nOldButtons");

	pi.tick = SE(player)->field<int>("m_nTickBase");
	pi.slot = server->GetSplitScreenPlayerSlot(player);
	pi.surfaceFriction = *reinterpret_cast<float *>((uintptr_t)player + Offsets::m_surfaceFriction);
	pi.ducked = SE(player)->ducked();

	float *m_flMaxspeed = &SE(player)->field<float>("m_flMaxspeed");
	pi.maxSpeed = *m_flMaxspeed;

#ifdef _WIN32
	// windows being weird. ask mlugg for explanation because idfk.
	void *paintPowerUser = (void*)((uint32_t)player + 0x1250);
#else
	void *paintPowerUser = player;
#endif
	using _GetPaintPower = const PaintPowerInfo_t&(__rescall*)(void* thisptr, unsigned paintId);
	_GetPaintPower GetPaintPower = Memory::VMT<_GetPaintPower>(paintPowerUser, Offsets::GetPaintPower);
	PaintPowerInfo_t speedPaintInfo = GetPaintPower(paintPowerUser, 2);
	pi.onSpeedPaint = speedPaintInfo.m_State == 1; // ACTIVE_PAINT_POWER

	if (pi.onSpeedPaint) {
		// maxSpeed is modified within ProcessMovement. This hack allows us to "predict" its next value
		// Cache off old max speed to restore later
		float oldMaxSpeed = *m_flMaxspeed;
		// Use the speed paint to modify the max speed
		using _UseSpeedPower = void(__rescall*)(void* thisptr, PaintPowerInfo_t& info);
		_UseSpeedPower UseSpeedPower = Memory::VMT<_UseSpeedPower>(player, Offsets::UseSpeedPower);
		UseSpeedPower(player, speedPaintInfo);
		// Get the new ("predicted") max speed and restore the old one on the player
		pi.maxSpeed = *m_flMaxspeed;
		*m_flMaxspeed = oldMaxSpeed;
	}

	pi.grounded = SE(player)->ground_entity();

	// this check was originally broken, so bypass it in v1
	if (tasPlayer->scriptVersion >= 2) {
		// predict the grounded state after jump.
		if (pi.grounded && (cmd->buttons & IN_JUMP) && !(m_nOldButtons & IN_JUMP)) {
			pi.grounded = false;
		}
	}

	pi.position = SE(player)->abs_origin();
	pi.angles = engine->GetAngles(pi.slot);
	pi.velocity = SE(player)->abs_velocity();

	pi.oldButtons = m_nOldButtons;

	pi.ticktime = 1.0f / 60.0f;  // TODO: find actual tickrate variable and put it there

	return pi;
}

void TasPlayer::SetFrameBulkQueue(int slot, std::vector<TasFramebulk> fbQueue) {
	this->framebulkQueue[slot] = fbQueue;
}

void TasPlayer::SetStartInfo(TasStartType type, std::string param) {
	// TODO: ensure start infos are the same in coop
	this->startInfo = TasStartInfo{type, param};
}

void TasPlayer::SaveUsercmdDebugs(int slot) {
	std::string filename = tasFileName[slot];
	std::vector<std::string> &lines = usercmdDebugs[slot];

	if (filename.size() == 0) return;
	if (lines.empty()) return;

	std::string fixedName = filename;
	size_t lastdot = filename.find_last_of(".");
	if (lastdot != std::string::npos) {
		fixedName = filename.substr(0, lastdot);
	}

	std::ofstream file(fixedName + "_usercmd.csv");
	file << "source,tick,forwardmove,sidemove,buttons,pitch,yaw,roll\n";
	for (auto &l : lines) file << l << "\n";
	file.close();
}

void TasPlayer::SavePlayerInfoDebugs(int slot) {
	std::string filename = tasFileName[slot];
	std::vector<std::string> &lines = playerInfoDebugs[slot];

	if (filename.size() == 0) return;
	if (lines.empty()) return;

	std::string fixedName = filename;
	size_t lastdot = filename.find_last_of(".");
	if (lastdot != std::string::npos) {
		fixedName = filename.substr(0, lastdot);
	}

	std::ofstream file(fixedName + "_player_info.csv");
	file << "tick,x,y,z,eye x,eye y,eye z,pitch,yaw,roll\n";
	for (auto &l : lines) file << l << "\n";
	file.close();
}

void TasPlayer::SaveProcessedFramebulks() {
	if (tasPlayer->inControllerCommands) {
		// Okay so this is an annoying situation. We've just started playing
		// a new TAS with a 'sar_tas_play' command *within* a framebulk of
		// another TAS, which has executed the command and called 'Stop'
		// which has called this. We actually want to save the framebulk
		// currently being run, since its commands are important! But
		// there's not any processed framebulk to hand; so we just add the
		// raw framebulks to the processed lists.
		if (tasFileName[0].size() > 0) {
			processedFramebulks[0].push_back(GetRawFramebulkAt(0, currentTick + 1));
		}
		if (tasFileName[1].size() > 0) {
			processedFramebulks[1].push_back(GetRawFramebulkAt(1, currentTick + 1));
		}
	}

	if (processedFramebulks[0].size() > 0 && tasFileName[0].size() > 0) {
		if (tasFileName[0].find("_raw") == std::string::npos) {
			TasParser::SaveFramebulksToFile(tasFileName[0], startInfo, wasStartNext, scriptVersion, processedFramebulks[0]);
		}
	}

	if (processedFramebulks[1].size() > 0 && tasFileName[1].size() > 0) {
		if (tasFileName[1].find("_raw") == std::string::npos) {
			TasParser::SaveFramebulksToFile(tasFileName[1], startInfo, wasStartNext, scriptVersion, processedFramebulks[1]);
		}
	}
}

/*
    This function is called by TAS controller's ControllerMove function.
    Even with alternateticks, the shortest interval between two ticks
    parsing inputs is about 0.5ms, and because it's alternateticks, it
    will oscillate between short and long (1/30th of a second) interval. 
    Steam Controller's response time is 1ms, so it's perfectly fine to
    assume it would be able to deliver different inputs for each
    subsequent tick, which has been confirmed by testing. Because of that,
    we assume the response time for our "virtual controller" to be
    non-existing and just let it parse inputs corresponding to given tick.
*/
void TasPlayer::FetchInputs(int slot, TasController *controller) {
	// Slight hack! Input fetching (including SteamControllerMove) is
	// called through _Host_RunFrame_Input, which is called *before*
	// GameFrame (that being called via _Host_RunFrame_Server). Therefore,
	// our tick count here is 1 lower than it should be. It'd be better to
	// actually call Update at the start of the tick, but this is easier
	// said than done since the input fetching code is only run when the
	// client is connected, so to match the behaviour we'd probably need
	// to actually hook at _Host_RunFrame_Input or CL_Move.
	int tick = currentTick + 1;

	TasFramebulk fb = GetRawFramebulkAt(slot, tick);

	int fbTick = fb.tick;

	if (sar_tas_debug.GetInt() > 0 && fbTick == tick) {
		console->Print("%s\n", fb.ToString().c_str());
	}

	controller->SetViewAnalog(fb.viewAnalog.x, fb.viewAnalog.y);
	controller->SetMoveAnalog(fb.moveAnalog.x, fb.moveAnalog.y);
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		controller->SetButtonState((TasControllerInput)i, fb.buttonStates[i]);
	}

	if (tick == 1) {
		// on tick 1, we'll run the commands from the bulk at tick 0 because
		// of the annoying off-by-one thing explained above
		TasFramebulk fb0 = GetRawFramebulkAt(slot, 0);
		for (std::string cmd : fb0.commands) {
			controller->AddCommandToQueue(cmd);
		}
	}

	// add commands only for tick when framebulk is placed. Don't preserve it to other ticks.
	if (tick == fbTick) {
		for (std::string cmd : fb.commands) {
			controller->AddCommandToQueue(cmd);
		}
	}
}

static bool IsTaunting(ClientEnt *player) {
	int cond = player->field<int>("m_nPlayerCond");
	if (cond & (1 << PORTAL_COND_TAUNTING)) return true;
	if (cond & (1 << PORTAL_COND_DROWNING)) return true;
	if (cond & (1 << PORTAL_COND_DEATH_CRUSH)) return true;
	if (cond & (1 << PORTAL_COND_DEATH_GIB)) return true;
	return false;
}

// special tools have to be parsed in input processing part.
// because of alternateticks, a pair of inputs are created and then executed at the same time,
// meaning that second tick in pair reads outdated info.
void TasPlayer::PostProcess(int slot, void *player, CUserCmd *cmd) {
	if (paused || engine->IsGamePaused()) return;
	if (!ready) return;
	if (slot == this->coopControlSlot) return;

	auto playerInfo = GetPlayerInfo(player, cmd);
	// player tickbase seems to be an accurate way of getting current time in ProcessMovement
	// every other way of getting time is incorrect due to alternateticks
	int tasTick = playerInfo.tick - startTick;

	float orig_forward = cmd->forwardmove;
	float orig_side = cmd->sidemove;
	float orig_up = cmd->upmove;

	cmd->forwardmove = 0;
	cmd->sidemove = 0;
	cmd->upmove = 0;
	cmd->buttons = 0;

	auto orig_angles = cmd->viewangles;

	// do not allow inputs after TAS has ended
	if (tasTick > lastTick) {
		return;
	}

	TasFramebulk fb = GetRawFramebulkAt(slot, tasTick);

	// update all tools that needs to be updated
	auto fbTick = fb.tick;
	fb.tick = tasTick;
	if (fbTick == tasTick) {
		for (TasToolCommand cmd : fb.toolCmds) {
			cmd.tool->SetParams(cmd.params);
		}
	}

	// applying tools
	if (scriptVersion >= 3) {
		// use priority list for newer versions. technically all tools should be in the list
		for (std::string toolName : TasTool::priorityList) {
			for (TasTool *tool : TasTool::GetList(slot)) {
				std::string tn(tool->GetName());
				if(toolName == tn) tool->Apply(fb, playerInfo);
			}
		}
	} else {
		// use old "earliest first" ordering system (partially also present in TasTool::SetParams)
		for (TasTool *tool : TasTool::GetList(slot)) {
			tool->Apply(fb, playerInfo);
		}
	}
	

	if (fb.moveAnalog.Length2D() > 1)
		fb.moveAnalog = fb.moveAnalog.Normalize();

	// make sure none of the framebulk is NaN
	if (std::isnan(fb.moveAnalog.x)) fb.moveAnalog.x = 0;
	if (std::isnan(fb.moveAnalog.y)) fb.moveAnalog.y = 0;
	if (std::isnan(fb.viewAnalog.x)) fb.viewAnalog.x = 0;
	if (std::isnan(fb.viewAnalog.y)) fb.viewAnalog.y = 0;

	// add processed framebulk to the cmd
	// using angles from playerInfo, as these seem to be the most accurate
	// cmd ones are created before tool parsing and GetAngles is wacky.
	// idk, as long as it produces the correct script file we should be fine lmfao
	cmd->viewangles.y = playerInfo.angles.y - fb.viewAnalog.x;  // positive values should rotate right.
	cmd->viewangles.x = playerInfo.angles.x - fb.viewAnalog.y;  // positive values should rotate up.
	cmd->viewangles.x = std::min(std::max(cmd->viewangles.x, -cl_pitchdown.GetFloat()), cl_pitchup.GetFloat());

	if (fb.moveAnalog.y > 0.0) {
		cmd->forwardmove = cl_forwardspeed.GetFloat() * fb.moveAnalog.y;
	} else {
		cmd->forwardmove = cl_backspeed.GetFloat() * fb.moveAnalog.y;
	}
	cmd->sidemove = cl_sidespeed.GetFloat() * fb.moveAnalog.x;

	cmd->buttons = 0;
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		if (g_TasControllerInGameButtons[i] && fb.buttonStates[i]) {
			cmd->buttons |= g_TasControllerInGameButtons[i];
		}
	}

	ClientEnt *clPlayer = client->GetPlayer(slot + 1);
	if (IsTaunting(clPlayer) || tasTick == 0) {
		// Don't actually do stuff on tick 0! We do this for consistency with
		// non-tools playback; see TasPlayer::FetchInputs for an
		// explanation. Also don't do stuff if we're taunting
		cmd->forwardmove = 0;
		cmd->sidemove = 0;
		cmd->upmove = 0;
		cmd->buttons = 0;
		cmd->weaponselect = 0;
		cmd->viewangles = orig_angles;
		// Zero out the framebulk too in this case; not strictly necessary,
		// but stops the generated raw having "fake" inputs (ones that never
		// actually happen)
		fb.moveAnalog = {0, 0};
		fb.viewAnalog = {0, 0};
		for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; ++i) {
			fb.buttonStates[i] = 0;
		}
	}

	engine->SetAngles(playerInfo.slot, cmd->viewangles);

	// CBasePlayer::PhysicsSimulate sets m_LastCmd a bit before calling
	// PlayerRunCommand (where we hook for post-processing), meaning it's
	// been set prior to tools processing. Luckily, this field is mostly
	// unused, but funnel movement relies on it to see if you're moving
	// "hard" enough to resist the funnel, so we'll set it here to allow
	// funnel movement. XXX: THERE IS AN ISSUE WITH THIS SOLUTION! In
	// PhysicsSimulate, m_LastCmd is set in a command context loop
	// *before* the main movement processing. This means that under
	// altticks (or any other context simulating multiple movement ticks
	// in one server tick), if you try and move in the funnel on an even
	// tick, the *following* usercmd (i.e. the last one in the context) is
	// used to check whether we can move. There's literally no way to
	// accurately recreate this with tools, since it'd require predicting
	// future tools output, so we'll have to hope this works for now.
	// UPDATE: OKAY, actually, only do this if tools changed our movement
	// analog, so we can at least try and counteract the bullshit described above
	if (fabsf(cmd->forwardmove - orig_forward) + fabsf(cmd->sidemove - orig_side) + fabsf(cmd->upmove - orig_up) > 0.01) {
		SE(player)->fieldOff<CUserCmd>("m_hViewModel", 8) /* m_LastCmd */ = *cmd;
	}

	// put processed framebulk in the list
	if (fbTick != tasTick) {
		std::vector<std::string> empty;
		fb.commands = empty;
	}
	processedFramebulks[slot].push_back(fb);

	tasPlayer->DumpUsercmd(slot, cmd, tasTick, "processed");
}

void TasPlayer::DumpUsercmd(int slot, const CUserCmd *cmd, int tick, const char *source) {
	if (!sar_tas_dump_usercmd.GetBool()) return;
	std::string str = Utils::ssprintf("%s,%d,%.6f,%.6f,%08X,%.6f,%.6f,%.6f", source, tick, cmd->forwardmove, cmd->sidemove, cmd->buttons, cmd->viewangles.x, cmd->viewangles.y, cmd->viewangles.z);
	usercmdDebugs[slot].push_back(str);
}

void TasPlayer::DumpPlayerInfo(int slot, int tick, Vector pos, Vector eye_pos, QAngle ang) {
	if (!sar_tas_dump_player_info.GetBool()) return;
	std::string str = Utils::ssprintf("%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f", tick, pos.x, pos.y, pos.z, eye_pos.x, eye_pos.y, eye_pos.z, ang.x, ang.y, ang.z);
	playerInfoDebugs[slot].push_back(str);
}

ON_EVENT_P(SESSION_START, 999) { // before rng manip hook
	if (tasPlayer->IsActive() && !tasPlayer->IsReady() && tasPlayer->numSessionsBeforeStart > 0) {
		--tasPlayer->numSessionsBeforeStart;
		if (tasPlayer->numSessionsBeforeStart == 0 && tasPlayer->rngManipFile != "") {
			RngManip::loadData((tasPlayer->rngManipFile + ".p2rng").c_str());
		}
	}
}

void TasPlayer::Update() {
	if (active && !paused) {
		if (!ready) {
			SetPlaybackVars(true); // Set host_framerate and friends asap!
			if (numSessionsBeforeStart == 0) {
				Start();
			}
		}
		if (ready && session->isRunning) {
			SetPlaybackVars(true);
			if (!IsRunning()) {
				PostStart();  // script has started its playback. Adjust the tick counter and offset
			} else {
				if (engine->IsGamePaused() && !wasEnginePaused) {
					wasEnginePaused = true;
					currentTick--;
				}
				if (!engine->IsGamePaused()) {
					wasEnginePaused = false;
					currentTick++;
				}
			}

			int pauseTick = sar_tas_pauseat.GetInt();
			if (currentTick == pauseTick && pauseTick > 0) {
				Pause();
			}
		}

		// make sure all ticks are processed by tools before stopping
		bool s0done = (!IsUsingTools(0) && currentTick > lastTick) || (int)processedFramebulks[0].size() > lastTick;
		bool s1done = !this->isCoop || (!IsUsingTools(1) && currentTick > lastTick) || (int)processedFramebulks[1].size() > lastTick;
		if ((s0done && s1done) || (!session->isRunning && startTick != -1)) {
			Stop();
		}
		// also do not allow inputs after TAS has ended
		if (currentTick > lastTick) {
			tasControllers[0]->Disable();
			tasControllers[1]->Disable();
		}

		// prevent usage of TAS tools in CM without cheats
		if (!sv_cheats.GetBool()) {
			sv_cheats.SetValue(true);
		}
	}

	if (startTick != -1 && active && ready) {
		if (paused) {
			//engine->ExecuteCommand("setpause", true);
		} else {
			//engine->ExecuteCommand("unpause", true);
		}
	}
}

ON_EVENT(FRAME) {
	if (tasPlayer) tasPlayer->UpdateServer();
}

void TasPlayer::UpdateServer() {
	TasStatus status;

	status.active = active;
	status.tas_path[0] = this->tasFileName[0];
	status.tas_path[1] = this->isCoop ? this->tasFileName[1] : "";
	status.playback_state =
		engine->IsAdvancing()
		? PlaybackState::PAUSED
		: this->GetTick() < sar_tas_skipto.GetInt()
		? PlaybackState::SKIPPING
		: PlaybackState::PLAYING;
	status.playback_rate = sar_tas_playback_rate.GetFloat();
	status.playback_tick = this->GetTick();

	TasServer::SetStatus(status);
}

DECL_COMMAND_FILE_COMPLETION(sar_tas_play, TAS_SCRIPT_EXT, TAS_SCRIPTS_DIR, 2)
DECL_COMMAND_FILE_COMPLETION(sar_tas_play_single, TAS_SCRIPT_EXT, TAS_SCRIPTS_DIR, 1)

static std::string g_replayTas[2];
static bool g_replayTasCoop;
static bool g_replayTasSingleCoop;

void TasPlayer::PlayFile(std::string slot0, std::string slot1) {
	bool coop = slot1.size() > 0;

	g_replayTas[0] = slot0;
	if (coop) g_replayTas[1] = slot1;
	g_replayTasCoop = coop;
	g_replayTasSingleCoop = false;

	tasPlayer->Stop(true);

	try {
		std::string filePath(std::string(TAS_SCRIPTS_DIR) + "/" + slot0 + "." + TAS_SCRIPT_EXT);
		std::vector<TasFramebulk> fb = TasParser::ParseFile(0, filePath);
		std::vector<TasFramebulk> fb2;

		if (coop) {
			std::string filePath2(std::string(TAS_SCRIPTS_DIR) + "/" + slot1 + "." + TAS_SCRIPT_EXT);
			fb2 = TasParser::ParseFile(1, filePath2);
		}

		if (fb.size() > 0 || fb2.size() > 0) {
			tasPlayer->isCoop = coop;
			tasPlayer->coopControlSlot = -1;
			tasPlayer->SetFrameBulkQueue(0, fb);
			tasPlayer->SetFrameBulkQueue(1, fb2);
			tasPlayer->Activate();
		}
	} catch (TasParserException &e) {
		return console->ColorMsg(Color(255, 100, 100), "Error while opening TAS file: %s\n", e.what());
	}
}

void TasPlayer::PlaySingleCoop(std::string file, int slot) {
	if (slot < 0 || slot > 1) {
		return console->Print("Invalid slot %d\n", slot);
	}

	g_replayTas[slot] = file;
	g_replayTas[1-slot] = "";
	g_replayTasCoop = true;
	g_replayTasSingleCoop = true;

	tasPlayer->Stop(true);

	try {
		std::string filePath(std::string(TAS_SCRIPTS_DIR) + "/" + file + "." + TAS_SCRIPT_EXT);
		std::vector<TasFramebulk> fb = TasParser::ParseFile(0, filePath);

		if (fb.size() > 0) {
			tasPlayer->isCoop = true;
			tasPlayer->coopControlSlot = 1-slot;
			tasPlayer->SetFrameBulkQueue(slot, fb);
			tasPlayer->SetFrameBulkQueue(1-slot, {});
			tasPlayer->Activate();
		}
	} catch (TasParserException &e) {
		return console->ColorMsg(Color(255, 100, 100), "Error while opening TAS file: %s\n", e.what());
	}
}

void TasPlayer::Replay() {
	if (g_replayTas[0].size() == 0 && g_replayTas[1].size() == 0) return;

	if (g_replayTasSingleCoop) {
		int slot = g_replayTas[0].size() == 0 ? 1 : 0;
		tasPlayer->PlaySingleCoop(g_replayTas[slot], slot);
	} else if (g_replayTasCoop) {
		tasPlayer->PlayFile(g_replayTas[0], g_replayTas[1]);
	} else {
		tasPlayer->PlayFile(g_replayTas[0], "");
	}
}

CON_COMMAND_F_COMPLETION(
	sar_tas_play,
	"sar_tas_play <filename> [filename2] - plays a TAS script with given name. If two script names are given, play coop\n",
	0,
	AUTOCOMPLETION_FUNCTION(sar_tas_play)) {
	IGNORE_DEMO_PLAYER();

	if (args.ArgC() != 2 && args.ArgC() != 3) {
		return console->Print(sar_tas_play.ThisPtr()->m_pszHelpString);
	}

	tasPlayer->PlayFile(args[1], args.ArgC() == 3 ? args[2] : "");
}

CON_COMMAND_F_COMPLETION(
	sar_tas_play_single,
	"sar_tas_play_single <filename> [slot] - plays a single coop TAS script, giving the player control of the other slot.\n",
	0,
	AUTOCOMPLETION_FUNCTION(sar_tas_play_single)) {
	IGNORE_DEMO_PLAYER();

	if (args.ArgC() != 2 && args.ArgC() != 3) {
		return console->Print(sar_tas_play_single.ThisPtr()->m_pszHelpString);
	}

	tasPlayer->PlaySingleCoop(args[1], args.ArgC() == 3 ? atoi(args[2]) : 0);
}

CON_COMMAND(sar_tas_replay, "sar_tas_replay - replays the last played TAS\n") {
	if (g_replayTas[0].size() == 0 && g_replayTas[1].size() == 0) {
		return console->Print("No TAS to replay\n");
	}

	console->Print("Replaying TAS\n");
	tasPlayer->Replay();
}

CON_COMMAND(sar_tas_pause, "sar_tas_pause - pauses TAS playback\n") {
	tasPlayer->Pause();
}

CON_COMMAND(sar_tas_resume, "sar_tas_resume - resumes TAS playback\n") {
	tasPlayer->Resume();
}

CON_COMMAND(sar_tas_advance, "sar_tas_advance - advances TAS playback by one tick\n") {
	tasPlayer->AdvanceFrame();
}

CON_COMMAND(sar_tas_stop, "sar_tas_stop - stop TAS playing\n") {
	tasPlayer->Stop(true);
}

CON_COMMAND(sar_tas_save_raw, "sar_tas_save_raw - saves a processed version of just processed script\n") {
	IGNORE_DEMO_PLAYER();

	if (args.ArgC() != 1) {
		return console->Print(sar_tas_save_raw.ThisPtr()->m_pszHelpString);
	}

	tasPlayer->SaveProcessedFramebulks();
}


HUD_ELEMENT2(tastick, "0", "Draws current TAS playback tick.\n", HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen) {
	if (!tasPlayer->IsActive()) {
		ctx->DrawElement("tastick: -");
	} else {
		int tick = tasPlayer->GetTick();
		ctx->DrawElement("tastick: %i (%.3f)", tick, engine->ToTime(tick));
	}
}
