#include "TasController.hpp"

#include "Features/Tas/TasPlayer.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/VGui.hpp"

#include <chrono>

Variable cl_pitchdown;
Variable cl_pitchup;

const char *g_TasControllerDigitalActions[] = {
	"+jump", "+duck", "+use", "+zoom", "+attack", "+attack2",
};
const int g_TasControllerInGameButtons[] = {
	IN_JUMP, IN_DUCK, IN_USE, IN_ZOOM, IN_ATTACK, IN_ATTACK2,
};

Variable sar_tas_real_controller_debug("sar_tas_real_controller_debug", "0", 0, 4, "Debugs controller.\n");

TasController *tasControllers[2];

TasController::TasController() {
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		buttons[i].command = g_TasControllerDigitalActions[i];
	}
	this->hasLoaded = true;

	cl_pitchdown = Variable("cl_pitchdown");
	cl_pitchup = Variable("cl_pitchup");
}

TasController::~TasController() {
	Disable();
	commandQueue.clear();
}

Vector TasController::GetMoveAnalog() {
	return moveAnalog;
}

Vector TasController::GetViewAnalog() {
	return viewAnalog;
}

void TasController::SetMoveAnalog(float x, float y) {
	moveAnalog.x = x;
	moveAnalog.y = y;
}

void TasController::SetViewAnalog(float x, float y) {
	viewAnalog.x = x;
	viewAnalog.y = y;
}

bool TasController::isEnabled() {
	return enabled;
}

void TasController::Enable() {
	enabled = true;
}

void TasController::Disable() {
	enabled = false;
	SetViewAnalog(0, 0);
	SetMoveAnalog(0, 0);
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		buttons[i].active = true;
		buttons[i].state = false;
	}
	ResetDigitalInputs();
}


void TasController::ResetDigitalInputs() {
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		TasControllerButton *button = &buttons[i];
		if (button->command[0] == '+') {
			std::string cmd = button->command;
			cmd[0] = '-';
			cmd = (this == tasControllers[0] ? "cmd1 " : "cmd2 ") + cmd;
			engine->ExecuteCommand(cmd.c_str(), true);
		}
	}
}


void TasController::AddCommandToQueue(std::string c) {
	commandQueue.push_back(c);
}

bool TasController::GetButtonState(TasControllerInput i) {
	return buttons[i].state;
}

void TasController::SetButtonState(TasControllerInput i, bool state) {
	TasControllerButton *btn = &buttons[i];
	if (btn->state != state) {
		btn->active = true;
	}
	btn->state = state;
}

std::chrono::time_point<std::chrono::high_resolution_clock> g_lastControllerMove;

void TasController::ControllerMove(int nSlot, float flFrametime, CUserCmd *cmd) {
	// ControllerMove is executed several times for one tick, idk why,
	// but only once with tick_count bigger than 0. Working only
	// on these seems to work fine, so I assume these are correct.
	if (cmd->tick_count == 0) return;

	// doing some debugs to test the behaviour of the real controller
	if (sar_tas_real_controller_debug.GetBool()) {
		int debugType = sar_tas_real_controller_debug.GetInt();
		if (debugType == 1) {
			console->Print("forwardmove: %.5f, sidemove: %.5f\n", cmd->forwardmove, cmd->sidemove);
		}

		auto now = std::chrono::high_resolution_clock::now();
		if (debugType == 2) {
			auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - g_lastControllerMove).count();
			console->Print("Time since last valid ControllerMove: %dus\n", timePassed);
		}

		g_lastControllerMove = now;

		if (debugType == 4) {
			console->Print("ControllerMove tick count: %d\n", cmd->tick_count);
		}
	}

	// affect inputs only if the virtual controller is enabled
	if (!enabled) return;
	if (tasPlayer->playbackInfo.coopControlSlot == nSlot) return;

	//console->Print("TasController::ControllerMove (%d, ", cmd->tick_count);

	tasPlayer->FetchInputs(nSlot, this);

	//TAS is now controlling inputs. Reset everything we can.
	cmd->forwardmove = 0;
	cmd->sidemove = 0;
	cmd->upmove = 0;
	cmd->buttons = 0;

	// Handle digital inputs. 
	// It's a mess because it was copied from original SteamControllerMove.
	for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
		TasControllerButton *button = &buttons[i];
		if (button->active && button->command[0] == '+') {
			char cmdbuf[128];
			snprintf(cmdbuf, sizeof(cmdbuf), "%s", button->command);
			if (!button->state) {
				cmdbuf[0] = '-';
			}
			engine->ExecuteCommand(cmdbuf, true);

			//TODO: find if stuff below is needed
			/*
            IClientMode* clientMode = GetClientMode();
            if (clientMode != NULL) {
                clientMode->KeyInput(bState ? true : false, STEAMCONTROLLER_SELECT, cmdbuf);
            }
            */
		}
		button->active = false;

		if (button->state) {
			cmd->buttons |= g_TasControllerInGameButtons[i];
		}
	}

	// Handle all additional commands from the command queue
	if (commandQueue.size() > 0) {
		tasPlayer->inControllerCommands = true;
		for (std::string cmd : commandQueue) {
			engine->ExecuteCommand(cmd.c_str(), true);
		}
		commandQueue.clear();
		tasPlayer->inControllerCommands = false;
	}

	//block analog inputs if paused (probably to block changing the view angle while paused)
	if (engine->IsGamePaused())
		return;

	//movement analog
	if (moveAnalog.y > 0.0) {
		cmd->forwardmove += cl_forwardspeed.GetFloat() * moveAnalog.y;
	} else {
		cmd->forwardmove += cl_backspeed.GetFloat() * moveAnalog.y;
	}

	cmd->sidemove += cl_sidespeed.GetFloat() * moveAnalog.x;

	//viewangle analog

	// don't do this part if tools are enabled.
	// tools processing will do it instead
	if (!tasPlayer->IsUsingTools()) {
		QAngle viewangles;
		viewangles = engine->GetAngles(nSlot);

		viewangles.y -= viewAnalog.x;  // positive values should rotate right.
		viewangles.x -= viewAnalog.y;  // positive values should rotate up.
		viewangles.x = std::min(std::max(viewangles.x, -cl_pitchdown.GetFloat()), cl_pitchup.GetFloat());

		cmd->mousedx = (int)(-viewAnalog.x);
		cmd->mousedy = (int)(-viewAnalog.y);

		engine->SetAngles(nSlot, viewangles);
	}

	{
		// Just to be safe, we don't change the viewangles in the original
		// CUserCmd, as the normal controller movement code doesn't. But we
		// want to set it for dumping the usercmd, hence this temporary
		CUserCmd tmp = *cmd;
		tmp.viewangles = engine->GetAngles(nSlot);
		tasPlayer->DumpUsercmd(nSlot, &tmp, tasPlayer->GetTick() + 1, "client"); // off-by-one bullshit on tick count
	}
}
