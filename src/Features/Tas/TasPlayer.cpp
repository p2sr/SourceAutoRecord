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
#include <fstream>

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

	if (active && !was_active) {
		old_forceuser = in_forceuser.GetInt();
		old_hostframerate = host_framerate.GetInt();
		old_interpolate = cl_interpolate.GetBool();
		old_motionblur = mat_motion_blur_enabled.GetBool();
		in_forceuser.SetValue(tasPlayer->playbackInfo.coopControlSlot >= 0 ? tasPlayer->playbackInfo.coopControlSlot : 100);
		host_framerate.SetValue(60);
		if (!sar_tas_interpolate.GetBool() && tasPlayer->IsUsingTools()) {
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
	bool tools = tasPlayer->IsUsingTools();
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
{
}

TasPlayer::~TasPlayer() {
	//framebulkQueue[0].clear();
	//framebulkQueue[1].clear();
}

void TasPlayer::Activate(TasPlaybackInfo info) {

	if (!info.HasActiveSlot()) return;

	//reset the controller before using it
	Stop(true);

	for (int slot = 0; slot < 2; ++slot) {
		for (TasTool *tool : TasTool::GetList(slot)) {
			tool->Reset();
		}
	}
	
	playbackInfo = info;

	for (int slot = 0; slot < 2; ++slot) {
		playbackInfo.slots[slot].ClearGeneratedContent();
	}

	active = true;
	ready = false;
	startTick = -1;
	currentTick = 0;

	lastTick = 0;
	for (int slot = 0; slot < (info.IsCoop() ? 2 : 1); ++slot) {
		for (TasFramebulk fb : info.slots[slot].framebulks) {
			if (fb.tick > lastTick) {
				lastTick = fb.tick;
			}
		}
	}

	auto startInfo = info.GetMainHeader().startInfo;
	numSessionsBeforeStart = 0;
	if (startInfo.isNext) numSessionsBeforeStart += 1;
	if (startInfo.type != TasScriptStartType::StartImmediately) numSessionsBeforeStart += 1;
	
	if (startInfo.type == ChangeLevel || startInfo.type == ChangeLevelCM) {
		//check if map exists
		if (fileSystem->MapExists(startInfo.param)) {
			if (session->isRunning && engine->GetCurrentMapName() == startInfo.param) {
				engine->ExecuteCommand("restart_level");
			} else {
				const char *start;
				if (info.IsCoop()) {
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
		std::string savePath = std::string(engine->GetGameDirectory()) + "/" + fileSystem->GetSaveDirectory() + startInfo.param + ".sav";
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
	if (playbackInfo.IsCoop()) tasControllers[1]->Enable();
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
	} else if (active && !ready) {
		console->Print("TAS script has been prevented from playback.\n");
	}

	active = false;
	ready = false;
	currentTick = 0;
	tasControllers[0]->Disable();
	tasControllers[1]->Disable();

	SetPlaybackVars(false);

	engine->SetAdvancing(false);

	if (playbackInfo.HasActiveSlot()) {
		previousPlaybackInfo = playbackInfo;
	}
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
	for (TasFramebulk framebulk : playbackInfo.slots[slot].framebulks) {
		int timeDist = tick - framebulk.tick;
		if (timeDist >= 0 && timeDist < closestTime) {
			closestTime = timeDist;
			closest = framebulk;
		}
	}
	return closest;
}

TasPlayerInfo TasPlayer::GetPlayerInfo(int slot, void *player, CUserCmd *cmd, bool clientside) {
	TasPlayerInfo pi;

	int m_nOldButtons = 0;

	if (!clientside) {
		ServerEnt *pl = (ServerEnt *)player;

		m_nOldButtons = pl->template field<int>("m_nOldButtons");

		pi.tick = pl->template field<int>("m_nTickBase");
		pi.slot = server->GetSplitScreenPlayerSlot(player);
		pi.surfaceFriction = *reinterpret_cast<float *>((uintptr_t)player + Offsets::S_m_surfaceFriction);
		pi.ducked = pl->ducked();

		float *m_flMaxspeed = &pl->template field<float>("m_flMaxspeed");
		pi.maxSpeed = *m_flMaxspeed;

		pi.grounded = pl->ground_entity();
		pi.position = pl->abs_origin();
		pi.velocity = pl->abs_velocity();

#ifdef _WIN32
		// windows being weird. ask mlugg for explanation because idfk.
		void *paintPowerUser = (void *)((uint32_t)player + 0x1250);
#else
		void *paintPowerUser = player;
#endif
		using _GetPaintPower = const PaintPowerInfo_t &(__rescall *)(void *thisptr, unsigned paintId);
		_GetPaintPower GetPaintPower = Memory::VMT<_GetPaintPower>(paintPowerUser, Offsets::GetPaintPower);
		PaintPowerInfo_t speedPaintInfo = GetPaintPower(paintPowerUser, 2);
		pi.onSpeedPaint = speedPaintInfo.m_State == 1;  // ACTIVE_PAINT_POWER

		if (pi.onSpeedPaint) {
			// maxSpeed is modified within ProcessMovement. This hack allows us to "predict" its next value
			// Cache off old max speed to restore later
			float oldMaxSpeed = *m_flMaxspeed;
			// Use the speed paint to modify the max speed
			using _UseSpeedPower = void(__rescall *)(void *thisptr, PaintPowerInfo_t &info);
			_UseSpeedPower UseSpeedPower = Memory::VMT<_UseSpeedPower>(player, Offsets::UseSpeedPower);
			UseSpeedPower(player, speedPaintInfo);
			// Get the new ("predicted") max speed and restore the old one on the player
			pi.maxSpeed = *m_flMaxspeed;
			*m_flMaxspeed = oldMaxSpeed;
		}
	} else {
		ClientEnt *pl = (ClientEnt *)player;

		m_nOldButtons = pl->template field<int>("m_nOldButtons");

		pi.tick = pl->template field<int>("m_nTickBase");
		pi.slot = server->GetSplitScreenPlayerSlot(player);
		pi.surfaceFriction = *reinterpret_cast<float *>((uintptr_t)player + Offsets::C_m_surfaceFriction);
		pi.ducked = pl->ducked();

		float *m_flMaxspeed = &pl->template field<float>("m_flMaxspeed");
		pi.maxSpeed = *m_flMaxspeed;

		pi.grounded = pl->ground_entity();
		pi.position = pl->abs_origin();
		pi.velocity = pl->abs_velocity();
	}

	// this check was originally broken, so bypass it in v1
	if (tasPlayer->GetScriptVersion(slot) >= 2) {
		// predict the grounded state after jump.
		if (pi.grounded && (cmd->buttons & IN_JUMP) && !(m_nOldButtons & IN_JUMP)) {
			pi.grounded = false;
		}
	}

	pi.angles = engine->GetAngles(pi.slot);
	
	pi.oldButtons = m_nOldButtons;

	if (fabsf(*engine->interval_per_tick - 1.0f / 60.0f) < 0.00001f) {
		// Back compat - this used to be hardcoded, and maybe the engine's interval
		// could be slightly different to the value we used, leading to desyncs on
		// old scripts.
		pi.ticktime = 1.0f / 60.0f;
	} else {
		pi.ticktime = *engine->interval_per_tick;
	}

	return pi;
}

void TasPlayer::SaveUsercmdDebugs(int slot) {
	std::string filename = playbackInfo.slots[slot].name;
	std::vector<std::string> &lines = playbackInfo.slots[slot].userCmdDebugs;

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
	std::string filename = playbackInfo.slots[slot].name;
	std::vector<std::string> &lines = playbackInfo.slots[slot].userCmdDebugs;

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
	for (int slot = 0; slot < 2; ++slot) {
		auto script = playbackInfo.slots[slot];

		if (!script.IsActive()) continue;

		if (tasPlayer->inControllerCommands) {
			// Okay so this is an annoying situation. We've just started playing
			// a new TAS with a 'sar_tas_play' command *within* a framebulk of
			// another TAS, which has executed the command and called 'Stop'
			// which has called this. We actually want to save the framebulk
			// currently being run, since its commands are important! But
			// there's not any processed framebulk to hand; so we just add the
			// raw framebulks to the processed lists.
			if (script.name.size() > 0) {
				script.processedFramebulks.push_back(GetRawFramebulkAt(slot, currentTick + 1));
			}
		}
	
		bool slotProcessed = script.processedFramebulks.size() > 0 && script.name.size() > 0;

		if (script.loadedFromFile) {
			if (slotProcessed && !script.IsRaw()) {
				TasParser::SaveRawScriptToFile(script);
			}
		} else {
			std::string slotScript = slotProcessed ? TasParser::SaveRawScriptToString(script) : "";
			TasServer::SendProcessedScript((uint8_t)slot, slotScript);
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
	if (slot == playbackInfo.coopControlSlot) return;

	auto playerInfo = GetPlayerInfo(slot, player, cmd);
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
			auto tool = TasTool::GetInstanceByName(slot, cmd.tool->GetName());
			if (tool == nullptr) continue;
			tool->SetParams(cmd.params);
		}
	}

	// applying tools
	if (playbackInfo.slots[slot].header.version >= 3) {
		// use priority list for newer versions. technically all tools should be in the list
		for (std::string toolName : TasTool::priorityList) {
			auto tool = TasTool::GetInstanceByName(slot, toolName);
			if (tool == nullptr) continue;
			tool->Apply(fb, playerInfo);
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
	playbackInfo.slots[slot].processedFramebulks.push_back(fb);

	tasPlayer->DumpUsercmd(slot, cmd, tasTick, "processed");
}

void TasPlayer::DumpUsercmd(int slot, const CUserCmd *cmd, int tick, const char *source) {
	if (!sar_tas_dump_usercmd.GetBool()) return;
	std::string str = Utils::ssprintf("%s,%d,%.6f,%.6f,%08X,%.6f,%.6f,%.6f", source, tick, cmd->forwardmove, cmd->sidemove, cmd->buttons, cmd->viewangles.x, cmd->viewangles.y, cmd->viewangles.z);
	playbackInfo.slots[slot].userCmdDebugs.push_back(str);
}

void TasPlayer::DumpPlayerInfo(int slot, int tick, Vector pos, Vector eye_pos, QAngle ang) {
	if (!sar_tas_dump_player_info.GetBool()) return;
	std::string str = Utils::ssprintf("%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f", tick, pos.x, pos.y, pos.z, eye_pos.x, eye_pos.y, eye_pos.z, ang.x, ang.y, ang.z);
	playbackInfo.slots[slot].playerInfoDebugs.push_back(str);
}

ON_EVENT_P(SESSION_START, 999) { // before rng manip hook
	if (tasPlayer->IsActive() && !tasPlayer->IsReady() && tasPlayer->numSessionsBeforeStart > 0) {
		--tasPlayer->numSessionsBeforeStart;
		auto rngManipFile = tasPlayer->playbackInfo.GetMainHeader().rngManipFile;
		if (tasPlayer->numSessionsBeforeStart == 0 && rngManipFile != "") {
			RngManip::loadData((rngManipFile + ".p2rng").c_str());
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
		bool s0done = (playbackInfo.slots[0].IsRaw() && currentTick > lastTick) || (int)playbackInfo.slots[0].processedFramebulks.size() > lastTick;
		bool s1done = !playbackInfo.IsCoop() || (playbackInfo.slots[1].IsRaw() && currentTick > lastTick) || (int)playbackInfo.slots[1].processedFramebulks.size() > lastTick;
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
	status.tas_path[0] = playbackInfo.slots[0].name;
	status.tas_path[1] = playbackInfo.IsCoop() ? playbackInfo.slots[1].name : "";
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


void TasPlayer::PlayFile(std::string slot0scriptPath, std::string slot1scriptPath) {
	bool coop = slot1scriptPath.size() > 0;
	
	Stop(true);

	try {
		TasPlaybackInfo newInfo;

		std::string filePath(std::string(TAS_SCRIPTS_DIR) + "/" + slot0scriptPath + "." + TAS_SCRIPT_EXT);
		newInfo.slots[0] = TasParser::ParseFile(filePath);
		newInfo.slots[0].name = slot0scriptPath;

		if (coop) {
			std::string filePath2(std::string(TAS_SCRIPTS_DIR) + "/" + slot1scriptPath + "." + TAS_SCRIPT_EXT);
			newInfo.slots[1] = TasParser::ParseFile(filePath2);
			newInfo.slots[1].name = slot1scriptPath;
		}
		Activate(newInfo);
		
	} catch (TasParserException &e) {
		return console->ColorMsg(Color(255, 100, 100), "Error while opening TAS file: %s\n", e.what());
	}
}

void TasPlayer::PlayScript(std::string slot0name, std::string slot0script, std::string slot1name, std::string slot1script) {
	bool coop = slot1script.size() > 0;

	Stop(true);

	try {
		TasPlaybackInfo newInfo;

		newInfo.slots[0] = TasParser::ParseScript(slot0name, slot0script);
		newInfo.slots[0].name = slot0name;
		newInfo.slots[0].path = "protocol";

		if (coop) {
			newInfo.slots[1] = TasParser::ParseScript(slot1name, slot1script);
			newInfo.slots[1].name = slot1name;
			newInfo.slots[1].path = "protocol";
		}

		Activate(newInfo);
		
	} catch (TasParserException &e) {
		return console->ColorMsg(Color(255, 100, 100), "Error while loading TAS script input: %s\n", e.what());
	}
}

void TasPlayer::PlaySingleCoop(std::string file, int slot) {
	if (slot < 0 || slot > 1) {
		return console->Print("Invalid slot %d\n", slot);
	}

	Stop(true);

	try {
		TasPlaybackInfo newInfo;
		newInfo.coopControlSlot = slot;

		std::string filePath(std::string(TAS_SCRIPTS_DIR) + "/" + file + "." + TAS_SCRIPT_EXT);
		newInfo.slots[1-slot] = TasParser::ParseFile(filePath);

		Activate(newInfo);
		
	} catch (TasParserException &e) {
		return console->ColorMsg(Color(255, 100, 100), "Error while opening TAS file: %s\n", e.what());
	}
}

void TasPlayer::Replay(bool automatic) {

	Stop(true);

	if (!previousPlaybackInfo.HasActiveSlot()) return;

	int currentAutoReplays = previousPlaybackInfo.autoReplayCount + (automatic ? 1 : 0);
	int currentReplays = previousPlaybackInfo.replayCount + 1;

	if (previousPlaybackInfo.GetMainScript().loadedFromFile) {
		if (previousPlaybackInfo.coopControlSlot >= 0) {
			PlaySingleCoop(previousPlaybackInfo.slots[1 - previousPlaybackInfo.coopControlSlot].name, previousPlaybackInfo.coopControlSlot);
		} else {
			PlayFile(previousPlaybackInfo.slots[0].name, previousPlaybackInfo.slots[1].name);
		}
	} else {
		Activate(previousPlaybackInfo);
	}

	playbackInfo.autoReplayCount = currentAutoReplays;
	playbackInfo.replayCount = currentReplays;
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
	if (!tasPlayer->previousPlaybackInfo.HasActiveSlot() && !tasPlayer->IsRunning()) {
		return console->Print("No TAS to replay\n");
	}
	if (tasPlayer->IsRunning()) {
		console->Print("Replaying currently playing TAS script\n");
	} else {
		console->Print("Replaying previously played TAS script\n");
	}
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
	if (!tasPlayer->IsActive()) {
		console->Print("TAS player is not active.\n");
	} else {
		tasPlayer->Stop(true);
	}
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
