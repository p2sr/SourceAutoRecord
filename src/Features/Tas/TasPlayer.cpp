#include "TasPlayer.hpp"

#include "Features/Session.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasTool.hpp"
#include "Features/Tas/TasServer.hpp"
#include "Features/Hud/Hud.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Variable.hpp"

#include <climits>
#include <filesystem>
#include <fstream>

#ifndef _WIN32
#include <dirent.h>
#endif

Variable sar_tas_debug("sar_tas_debug", "0", 0, 2, "Debug TAS informations. 0 - none, 1 - basic, 2 - all.\n");
Variable sar_tas_tools_enabled("sar_tas_tools_enabled", "1", "Enables tool processing for TAS script making.\n");
Variable sar_tas_tools_force("sar_tas_tools_force", "0", "Force tool playback for TAS scripts; primarily for debugging.\n");
Variable sar_tas_autosave_raw("sar_tas_autosave_raw", "0", "Enables automatic saving of raw, processed TAS scripts.\n");
Variable sar_tas_pauseat("sar_tas_pauseat", "0", 0, "Pauses the TAS playback on specified tick.\n");
Variable sar_tas_skipto("sar_tas_skipto", "0", 0, "Fast-forwards the TAS playback until given playback tick.\n");
Variable sar_tas_playback_rate("sar_tas_playback_rate", "1.0", 0.02, "The rate at which to play back TAS scripts.\n");
Variable sar_tas_restore_fps("sar_tas_restore_fps", "1", "Restore fps_max and host_framerate after TAS playback.\n");

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

	if (active && !was_active) {
		old_forceuser = in_forceuser.GetInt();
		old_hostframerate = host_framerate.GetInt();
		in_forceuser.SetValue(tasPlayer->coopControlSlot >= 0 ? tasPlayer->coopControlSlot : 100);
		host_framerate.SetValue(60);
	} else if (!active && was_active) {
		in_forceuser.SetValue(old_forceuser);
		if (sar_tas_restore_fps.GetBool()) {
			host_framerate.SetValue(old_hostframerate);
			if (saved_fps) {
				fps_max.SetValue(old_fpsmax);
				session->oldFpsMax = old_fpsmax; // In case we're restoring during an uncapped load
			}
		}
		saved_fps = false;
	}

	// Don't save fps_max in loads, or uncap might get in the way
	// Wait for the session to start instead
	if (session->isRunning && active && !saved_fps) {
		old_fpsmax = fps_max.GetInt();
		saved_fps = true;
	}

	if (saved_fps && active) {
		if (tasPlayer->GetTick() < sar_tas_skipto.GetInt()) {
			fps_max.SetValue(0);
		} else if (tasPlayer->GetTick() >= sar_tas_skipto.GetInt()) {
			fps_max.SetValue((int)(sar_tas_playback_rate.GetFloat() * 60.0f));
		}
	}

	was_active = active;
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
	if (std::ifstream(engine->GetGameDirectory() + name).good()) return true;
	if (std::ifstream("portal2_dlc1" + name).good()) return true;
	if (std::ifstream("sdk_content" + name).good()) return true;
	if (std::ifstream("portal2_dlc2" + name).good()) return true;
	return false;
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

	active = true;
	startTick = -1;
	currentTick = 0;

	pauseTick = sar_tas_pauseat.GetInt();

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

	int m_nOldButtons = *(int *)((uintptr_t)player + Offsets::S_m_nJumpTimeMsecs + 12); // This field isn't networked and I can't be bothered to add another explicit offset for it

	pi.tick = *reinterpret_cast<int *>((uintptr_t)player + Offsets::m_nTickBase);
	pi.slot = server->GetSplitScreenPlayerSlot(player);
	pi.surfaceFriction = *reinterpret_cast<float *>((uintptr_t)player + Offsets::m_surfaceFriction);
	pi.ducked = *reinterpret_cast<bool *>((uintptr_t)player + Offsets::S_m_bDucked);

	float *m_flMaxspeed = reinterpret_cast<float *>((uintptr_t)player + Offsets::m_flMaxspeed);
	float oldMaxSpeed = *m_flMaxspeed;
	// TODO: maxSpeed will be inaccurate without proper prediction. figure out these DAMN offsets to make it work.

	// maxSpeed is modified within ProcessMovement function. This cheesy hack allows us to "predict" its next value
	/*using _GetPaintPower = const PaintPowerInfo_t&(__rescall*)(void* thisptr, unsigned paintId);
    _GetPaintPower GetPaintPower = Memory::VMT<_GetPaintPower>(player, Offsets::GetPaintPower);
    const PaintPowerInfo_t& paintInfo = GetPaintPower(player, 2);*/

	/*using _UseSpeedPower = void(__rescall*)(void* thisptr, PaintPowerInfo_t& info);
    _UseSpeedPower UseSpeedPower = Memory::VMT<_UseSpeedPower>(player, Offsets::UseSpeedPower);
    UseSpeedPower(player, paintInfo);
    */

	pi.maxSpeed = *m_flMaxspeed;
	*m_flMaxspeed = oldMaxSpeed;

	unsigned int groundEntity = *reinterpret_cast<unsigned int *>((uintptr_t)player + Offsets::S_m_hGroundEntity);
	pi.grounded = groundEntity != 0xFFFFFFFF;

	// predict the grounded state after jump.
	if (pi.grounded && (cmd->buttons & IN_JUMP) && !(m_nOldButtons & IN_JUMP)) {
		pi.grounded = false;
	}

	pi.position = *reinterpret_cast<Vector *>((uintptr_t)player + Offsets::S_m_vecAbsOrigin);
	pi.angles = engine->GetAngles(pi.slot);
	pi.velocity = *reinterpret_cast<Vector *>((uintptr_t)player + Offsets::S_m_vecAbsVelocity);

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

void TasPlayer::SaveProcessedFramebulks() {
	if (processedFramebulks[0].size() > 0 && tasFileName[0].size() > 0) {
		if (tasFileName[0].find("_raw") == std::string::npos) {
			TasParser::SaveFramebulksToFile(tasFileName[0], startInfo, processedFramebulks[0]);
		}
	}

	if (processedFramebulks[1].size() > 0 && tasFileName[1].size() > 0) {
		if (tasFileName[1].find("_raw") == std::string::npos) {
			TasParser::SaveFramebulksToFile(tasFileName[1], startInfo, processedFramebulks[1]);
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

// special tools have to be parsed in input processing part.
// because of alternateticks, a pair of inputs are created and then executed at the same time,
// meaning that second tick in pair reads outdated info.
void TasPlayer::PostProcess(int slot, void *player, CUserCmd *cmd) {
	if (paused || engine->IsGamePaused()) return;
	if (slot == this->coopControlSlot) return;

	cmd->forwardmove = 0;
	cmd->sidemove = 0;
	cmd->upmove = 0;
	cmd->buttons = 0;

	auto playerInfo = GetPlayerInfo(player, cmd);
	// player tickbase seems to be an accurate way of getting current time in ProcessMovement
	// every other way of getting time is incorrect due to alternateticks
	int tasTick = playerInfo.tick - startTick;

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
	for (TasTool *tool : TasTool::GetList(slot)) {
		tool->Apply(fb, playerInfo);
	}

	// add processed framebulk to the cmd
	// using angles from playerInfo, as these seem to be the most accurate
	// cmd ones are created before tool parsing and GetAngles is wacky.
	// idk, as long as it produces the correct script file we should be fine lmfao
	cmd->viewangles.y = playerInfo.angles.y - fb.viewAnalog.x;  // positive values should rotate right.
	cmd->viewangles.x = playerInfo.angles.x - fb.viewAnalog.y;  // positive values should rotate up.
	cmd->viewangles.x = std::min(std::max(cmd->viewangles.x, -cl_pitchdown.GetFloat()), cl_pitchup.GetFloat());

	engine->SetAngles(playerInfo.slot, cmd->viewangles);

	if (fb.moveAnalog.y > 0.0) {
		cmd->forwardmove = cl_forwardspeed.GetFloat() * fb.moveAnalog.y;
	} else {
		cmd->forwardmove = cl_backspeed.GetFloat() * fb.moveAnalog.y;
	}
	cmd->sidemove = cl_sidespeed.GetFloat() * fb.moveAnalog.x;

	// making sure none of the move values are NaN
	if (std::isnan(cmd->forwardmove)) cmd->forwardmove = 0;
	if (std::isnan(cmd->sidemove)) cmd->sidemove = 0;

	cmd->buttons = 0;
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		if (g_TasControllerInGameButtons[i] && fb.buttonStates[i]) {
			cmd->buttons |= g_TasControllerInGameButtons[i];
		}
	}

	// put processed framebulk in the list
	if (fbTick != tasTick) {
		std::vector<std::string> empty;
		fb.commands = empty;
	}
	processedFramebulks[slot].push_back(fb);
}

void TasPlayer::Update() {
	if (active && !paused) {
		if (!ready) {
			if (startInfo.type == StartImmediately || !session->isRunning) {
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

			if (sar_tas_pauseat.GetInt() > pauseTick) {
				pauseTick = sar_tas_pauseat.GetInt();
			}

			if (currentTick == pauseTick && pauseTick > 0) {
				Pause();
			}
		}

		// make sure all ticks are processed by tools before stopping
		bool s0done = (!IsUsingTools(0) && currentTick > lastTick) || processedFramebulks[0].size() > lastTick;
		bool s1done = !this->isCoop || (!IsUsingTools(1) && currentTick > lastTick) || processedFramebulks[1].size() > lastTick;
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

	UpdateServer();
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
	if (g_replayTasSingleCoop) {
		int slot = g_replayTas[0].size() == 0 ? 1 : 0;
		engine->ExecuteCommand(Utils::ssprintf("sar_tas_play_single \"%s\" %d", g_replayTas[slot].c_str(), slot).c_str());
	} else if (g_replayTasCoop) {
		engine->ExecuteCommand(Utils::ssprintf("sar_tas_play \"%s\" \"%s\"", g_replayTas[0].c_str(), g_replayTas[1].c_str()).c_str());
	} else {
		engine->ExecuteCommand(Utils::ssprintf("sar_tas_play \"%s\"", g_replayTas[0].c_str()).c_str());
	}
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
