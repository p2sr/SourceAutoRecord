#include "TasPlayer.hpp"

#include "Features/Session.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasTool.hpp"
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

Variable sar_tas_debug("sar_tas_debug", "0", 0, 2, "Debug TAS informations. 0 - none, 1 - basic, 2 - all.");
Variable sar_tas_tools_enabled("sar_tas_tools_enabled", "1", 0, 1, "Enables tool processing for TAS script making.");
Variable sar_tas_autosave_raw("sar_tas_autosave_raw", "0", 0, 1, "Enables automatic saving of raw, processed TAS scripts.");

Variable sar_tas_skipto("sar_tas_skipto", "0", 0, "Fast-forwards the TAS playback until given playback tick.");
Variable sar_tas_pauseat("sar_tas_pauseat", "0", 0, "Pauses the TAS playback on specified tick.");

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

TasPlayer::TasPlayer()
	: startInfo({TasStartType::StartImmediately, ""}) {
}

TasPlayer::~TasPlayer() {
	framebulkQueue.clear();
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

void TasPlayer::Activate() {
	//reset the controller before using it
	Stop(true);

	for (TasTool *tool : TasTool::GetList()) {
		tool->Reset();
	}
	processedFramebulks.clear();

	active = true;
	startTick = -1;
	currentTick = 0;

	pauseTick = sar_tas_pauseat.GetInt();

	lastTick = 0;
	for (TasFramebulk fb : framebulkQueue) {
		if (fb.tick > lastTick) {
			lastTick = fb.tick;
		}
	}

	ready = false;
	if (startInfo.type == ChangeLevel || startInfo.type == ChangeLevelCM) {
		//check if map exists
		std::string mapPath = std::string(engine->GetGameDirectory()) + "/maps/" + startInfo.param + ".bsp";
		std::ifstream mapf(mapPath);
		if (mapf.good()) {
			std::string cmd = "map ";
			if (session->isRunning && engine->GetCurrentMapName().size() > 0) {
				cmd = "changelevel ";
			}
			cmd += startInfo.param;
			engine->ExecuteCommand(cmd.c_str());
		} else {
			console->ColorMsg(Color(255, 100, 100), "Cannot activate TAS file - unknown map '%s.'\n", startInfo.param.c_str());
			Stop();
			return;
		}
		mapf.close();
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
	processedFramebulks.clear();

	if (startInfo.type == ChangeLevelCM) {
		sv_bonus_challenge.SetValue(1);
	}

	ready = true;
	currentTick = 0;
	startTick = -1;
}

void TasPlayer::PostStart() {
	startTick = server->gpGlobals->tickcount;
	if (sar_tas_debug.GetInt() > 1) {
		console->Print("Start tick: %d\n", startTick);
	}
	tasController->Enable();
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
	tasController->Disable();
}

void TasPlayer::Pause() {
	paused = true;
}

void TasPlayer::Resume() {
	paused = false;
}

void TasPlayer::AdvanceFrame() {
	Resume();
	pauseTick = currentTick + 2;
}

// returns raw framebulk that should be used for given tick
TasFramebulk TasPlayer::GetRawFramebulkAt(int tick) {
	int closestTime = INT_MAX;
	TasFramebulk closest;
	for (TasFramebulk framebulk : framebulkQueue) {
		int timeDist = tick - framebulk.tick;
		if (timeDist >= 0 && timeDist < closestTime) {
			closestTime = timeDist;
			closest = framebulk;
		}
	}
	return closest;
}

TasPlayerInfo TasPlayer::GetPlayerInfo(void *player, CMoveData *pMove) {
	TasPlayerInfo pi;

	pi.tick = *reinterpret_cast<int *>((uintptr_t)player + Offsets::m_nTickBase);
	pi.slot = server->GetSplitScreenPlayerSlot(player);
	pi.surfaceFriction = *reinterpret_cast<float *>((uintptr_t)player + Offsets::m_surfaceFriction);
	pi.ducked = *reinterpret_cast<bool *>((uintptr_t)player + Offsets::m_bDucked);

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
	if (pi.grounded && (pMove->m_nButtons & IN_JUMP) && !(pMove->m_nOldButtons & IN_JUMP)) {
		pi.grounded = false;
	}

	pi.position = *reinterpret_cast<Vector *>((uintptr_t)player + Offsets::S_m_vecAbsOrigin);
	pi.angles = engine->GetAngles(pi.slot);
	pi.velocity = *reinterpret_cast<Vector *>((uintptr_t)player + Offsets::S_m_vecAbsVelocity);

	pi.oldButtons = pMove->m_nOldButtons;

	pi.ticktime = 1.0f / 60.0f;  // TODO: find actual tickrate variable and put it there

	return pi;
}

void TasPlayer::SetFrameBulkQueue(std::vector<TasFramebulk> fbQueue) {
	this->framebulkQueue = fbQueue;
}

void TasPlayer::SetStartInfo(TasStartType type, std::string param) {
	this->startInfo = TasStartInfo{type, param};
}

void TasPlayer::SaveProcessedFramebulks() {
	if (processedFramebulks.size() > 0 && tasFileName.size() > 0) {
		if (tasFileName.find("_raw") != std::string::npos) {
			return;
		}
		TasParser::SaveFramebulksToFile(tasFileName, startInfo, processedFramebulks);
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
void TasPlayer::FetchInputs(TasController *controller) {
	TasFramebulk fb = GetRawFramebulkAt(currentTick);

	int fbTick = fb.tick;

	if (sar_tas_debug.GetInt() > 0 && fbTick == currentTick) {
		console->Print("%s\n", fb.ToString().c_str());
	}

	controller->SetViewAnalog(fb.viewAnalog.x, fb.viewAnalog.y);
	controller->SetMoveAnalog(fb.moveAnalog.x, fb.moveAnalog.y);
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		controller->SetButtonState((TasControllerInput)i, fb.buttonStates[i]);
	}
	// add commands only for tick when framebulk is placed. Don't preserve it to other ticks.
	if (currentTick == fbTick) {
		for (std::string cmd : fb.commands) {
			controller->AddCommandToQueue(cmd);
		}
	}
}

// special tools have to be parsed in input processing part.
// because of alternateticks, a pair of inputs are created and then executed at the same time,
// meaning that second tick in pair reads outdated info.
void TasPlayer::PostProcess(void *player, CMoveData *pMove) {
	if (paused || engine->IsGamePaused()) return;

	pMove->m_flForwardMove = 0;
	pMove->m_flSideMove = 0;
	pMove->m_nButtons = 0;

	auto playerInfo = GetPlayerInfo(player, pMove);
	// player tickbase seems to be an accurate way of getting current time in ProcessMovement
	// every other way of getting time is incorrect due to alternateticks
	int tasTick = playerInfo.tick - startTick;

	// do not allow inputs after TAS has ended
	if (tasTick > lastTick) {
		return;
	}

	TasFramebulk fb = GetRawFramebulkAt(tasTick);

	// update all tools that needs to be updated
	auto fbTick = fb.tick;
	fb.tick = tasTick;
	if (fbTick == tasTick) {
		for (TasToolCommand cmd : fb.toolCmds) {
			cmd.tool->SetParams(cmd.params);
		}
	}

	// applying tools
	for (TasTool *tool : TasTool::GetList()) {
		tool->Apply(fb, playerInfo);
	}


	// all behaviour that is prevented before ProcessMovement (We have to manually recreate them)

	// ducking midair
	if (prevent_crouch_jump.GetBool()) {
		int m_InAirState = *reinterpret_cast<int *>((uintptr_t)player + Offsets::m_InAirState);

		if (m_InAirState == 1) {  //in air jumped
			fb.buttonStates[Crouch] = false;
		}
	}

	// TODO: implement all other pre-ProcessMovement stuff (i believe coop taunt is one of them? but we don't have coop support yet)


	// add processed framebulk to the pMove
	// using angles from playerInfo, as these seem to be the most accurate
	// pMove ones are created before tool parsing and GetAngles is wacky.
	// idk, as long as it produces the correct script file we should be fine lmfao
	pMove->m_vecAngles.y = playerInfo.angles.y - fb.viewAnalog.x;  // positive values should rotate right.
	pMove->m_vecAngles.x = playerInfo.angles.x - fb.viewAnalog.y;  // positive values should rotate up.
	pMove->m_vecAngles.x = std::min(std::max(pMove->m_vecAngles.x, -cl_pitchdown.GetFloat()), cl_pitchup.GetFloat());

	pMove->m_vecViewAngles = pMove->m_vecAbsViewAngles = pMove->m_vecAngles;
	engine->SetAngles(playerInfo.slot, pMove->m_vecAngles);

	if (fb.moveAnalog.y > 0.0) {
		pMove->m_flForwardMove = cl_forwardspeed.GetFloat() * fb.moveAnalog.y;
	} else {
		pMove->m_flForwardMove = cl_backspeed.GetFloat() * fb.moveAnalog.y;
	}
	pMove->m_flSideMove = cl_sidespeed.GetFloat() * fb.moveAnalog.x;

	// making sure none of the move values are NaN
	if (std::isnan(pMove->m_flForwardMove)) pMove->m_flForwardMove = 0;
	if (std::isnan(pMove->m_flSideMove)) pMove->m_flForwardMove = 0;

	pMove->m_nButtons = 0;
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		if (g_TasControllerInGameButtons[i] && fb.buttonStates[i]) {
			pMove->m_nButtons |= g_TasControllerInGameButtons[i];
		}
	}

	// put processed framebulk in the list
	if (fbTick != tasTick) {
		std::vector<std::string> empty;
		fb.commands = empty;
	}
	processedFramebulks.push_back(fb);
}

void TasPlayer::Update() {
	if (active && !paused) {
		if (!ready) {
			if (startInfo.type == StartImmediately || !session->isRunning) {
				Start();
			}
		}
		if (ready && session->isRunning) {
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

			// fast-forward feature. We're just changing host_framerate lol.
			if (currentTick < sar_tas_skipto.GetInt()) {
				host_framerate.ThisPtr()->m_fValue = 1;
			} else if (currentTick == sar_tas_skipto.GetInt() || currentTick > lastTick) {
				host_framerate.SetValue(host_framerate.GetString());
			}
		}

		// make sure all ticks are processed by tools before stopping
		if ((!sar_tas_tools_enabled.GetBool() && currentTick > lastTick) || processedFramebulks.size() > lastTick || (!session->isRunning && startTick != -1)) {
			Stop();
		}
		// also do not allow inputs after TAS has ended
		if (currentTick > lastTick) {
			tasController->Disable();
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

DECL_COMMAND_COMPLETION(sar_tas_play) {
	try {
		for (
			auto i = std::filesystem::recursive_directory_iterator(TAS_SCRIPTS_DIR);
			i != std::filesystem::recursive_directory_iterator();
			++i
		) {
			if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
				break;
			}

			auto file = *i;

			auto scriptExt = file.path().extension().string();
			if (Utils::EndsWith(scriptExt, TAS_SCRIPT_EXT)) {
				auto scriptPath = file.path();
				auto scriptName = scriptPath.stem().string();
				for (int d = 0; d < i.depth(); d++) {
					scriptPath = scriptPath.parent_path();
					scriptName = scriptPath.stem().string() + "/" + scriptName;
				}

				if (std::strstr(scriptName.c_str(), match)) {
					items.push_back(scriptName);
				}
			}
		}
	} catch (std::filesystem::filesystem_error &e) {
	}

	FINISH_COMMAND_COMPLETION();
}

CON_COMMAND_F_COMPLETION(
	sar_tas_play,
	"sar_tas_play <filename> - plays a TAS script with given name\n",
	0,
	AUTOCOMPLETION_FUNCTION(sar_tas_play)) {
	IGNORE_DEMO_PLAYER();

	if (args.ArgC() != 2) {
		return console->Print(sar_tas_play.ThisPtr()->m_pszHelpString);
	}

	std::string fileName(args[1]);
	std::string filePath(std::string(TAS_SCRIPTS_DIR) + "/" + fileName + "." + TAS_SCRIPT_EXT);
	try {
		std::vector<TasFramebulk> fb = TasParser::ParseFile(filePath);

		if (fb.size() > 0) {
			tasPlayer->SetFrameBulkQueue(fb);
			tasPlayer->Activate();
		}
	} catch (TasParserException &e) {
		return console->ColorMsg(Color(255, 100, 100), "Error while opening TAS file: %s\n", e.what());
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
