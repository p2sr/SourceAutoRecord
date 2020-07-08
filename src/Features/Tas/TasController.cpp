#include "TasController.hpp"

#include "Features/Tas/TasPlayer.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/VGui.hpp"

Variable cl_pitchdown;
Variable cl_pitchup;

Variable sar_tas2_real_controller_debug("sar_tas2_real_controller_debug", "0", "debugs controller.");


Variable sensitivity;
void LockMouse()
{
    sensitivity.ThisPtr()->m_fValue = 0;
}
void UnlockMouse()
{
    sensitivity.SetValue(sensitivity.ThisPtr()->m_pszString);
}


TasController* tasController;

TasController::TasController()
{
    for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
        buttons[i].command = g_TasControllerDigitalActions[i];
    }
    this->hasLoaded = true;

    cl_pitchdown = Variable("cl_pitchdown");
    cl_pitchup = Variable("cl_pitchup");
    sensitivity = Variable("sensitivity");
}

TasController::~TasController()
{
    Disable();
    commandQueue.clear();
}


Vector TasController::GetMoveAnalog()
{
    return moveAnalog;
}

Vector TasController::GetViewAnalog()
{
    return viewAnalog;
}

void TasController::SetMoveAnalog(float x, float y) 
{
    moveAnalog.x = x;
    moveAnalog.y = y;
}

void TasController::SetViewAnalog(float x, float y)
{
    viewAnalog.x = x;
    viewAnalog.y = y;
}


bool TasController::isEnabled()
{
    return enabled;
}

void TasController::Enable()
{
    enabled = true;
    LockMouse();
}

void TasController::Disable()
{
    enabled = false;
    SetViewAnalog(0, 0);
    SetMoveAnalog(0, 0);
    for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
        buttons[i].active = true;
        buttons[i].state = false;
    }
    UnlockMouse();
}

void TasController::AddCommandToQueue(std::string c)
{
    commandQueue.push_back(c);
}

bool TasController::GetButtonState(TasControllerInput i)
{
    return buttons[i].state;
}

void TasController::SetButtonState(TasControllerInput i, bool state)
{
    TasControllerButton* btn = &buttons[i];
    if (btn->state != state) {
        btn->active = true;
    }
    btn->state = state;
}


void TasController::ControllerMove(int nSlot, float flFrametime, CUserCmd* cmd)
{

    if (cmd->tick_count != 0 && sar_tas2_real_controller_debug.GetBool()) {
        console->Print("%.5f %.5f\n", cmd->forwardmove, cmd->sidemove);
    }

    if (!enabled || cmd->tick_count==0) return;
    //console->Print("TasController::ControllerMove (%d, ", cmd->tick_count);

    tasPlayer->FetchInputs(this);

    //TAS is now controlling inputs. Reset everything we can.
    cmd->forwardmove = 0;
    cmd->sidemove = 0;
    cmd->upmove = 0;
    cmd->buttons = 0;

    LockMouse();

    //in terms of functionality, this whole stuff below is mostly a literal copy of SteamControllerMove.

    //handle digital inputs
    for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
        TasControllerButton* button = &buttons[i];
        if (button->active && button->command[0] == '+') {
            char cmdbuf[128];
            snprintf(cmdbuf, sizeof(cmdbuf), "%s", button->command);
            if (!button->state) {
                cmdbuf[0] = '-';
            }
            engine->ExecuteCommand(cmdbuf);

            //TODO: find if stuff below is needed
            /*
            IClientMode* clientMode = GetClientMode();
            if (clientMode != NULL) {
                clientMode->KeyInput(bState ? true : false, STEAMCONTROLLER_SELECT, cmdbuf);
            }
            */
        }
        button->active = false;
    }

    // handle all additional commands from the command queue (not in the original, but um why not?)
    if (commandQueue.size() > 0) {
        for (std::string cmd : commandQueue) {
            char cmdbuf[128];
            snprintf(cmdbuf, sizeof(cmdbuf), "%s", cmd.c_str());
            engine->ExecuteCommand(cmdbuf);
        }
        commandQueue.clear();
    }

    //block analog inputs if paused (probably to block changing the view angle while paused)
    if (vgui->IsUIVisible())
        return;

    //movement analog
    if (moveAnalog.y > 0.0) {
        cmd->forwardmove += cl_forwardspeed.GetFloat() * moveAnalog.y;
    } else {
        cmd->forwardmove += cl_backspeed.GetFloat() * moveAnalog.y;
    }

    cmd->sidemove += cl_sidespeed.GetFloat() * moveAnalog.x;

    //viewangle analog
    QAngle viewangles;
    viewangles = engine->GetAngles(GET_SLOT());

    //TODO: check if this stuff below is important.
    //view->StopPitchDrift();
    //::g_pInputSystem->SetCurrentInputDevice(INPUT_DEVICE_STEAM_CONTROLLER);
    //if (!GetPerUser().m_fCameraInterceptingMouse && g_pInputStackSystem->IsTopmostEnabledContext(m_hInputContext))

    viewangles.y += viewAnalog.x;

    viewangles.x += viewAnalog.y;
    viewangles.x = std::min(std::max(viewangles.x, -cl_pitchdown.GetFloat()), cl_pitchup.GetFloat());

    cmd->mousedx = (int)(-viewAnalog.x);
    cmd->mousedy = (int)(-viewAnalog.y);

    engine->SetAngles(GET_SLOT(), viewangles);
}
