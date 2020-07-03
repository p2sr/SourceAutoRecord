#include "TasPlayer.hpp"

#include "Variable.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Session.hpp"

Variable sar_tas2_debug("sar_tas2_debug", "0", 0, 2, "Debug TAS informations. 0 - none, 1 - basic, 2 - all.");

TasPlayer* tasPlayer;

TasPlayer::TasPlayer()
{
    framebulkQueue.push_back({ 61, { 0, 0 }, { 0, 0 }, { false,  true, false, false, false, false }});
    framebulkQueue.push_back({ 71, { 0, -1 }, { 0, 0 }, { false, true, false, false, false, false } });
    framebulkQueue.push_back({ 73, { 1, 0 }, { 0, 0 }, { false, true, false, false, false, false } });
    framebulkQueue.push_back({ 201, { 0, 0 }, { 0, 0 }, { false, false, false, false, false, false }, {"pause"} });
}

TasPlayer::~TasPlayer()
{
    framebulkQueue.clear();
}

void TasPlayer::Activate()
{
    if (sar_tas2_debug.GetInt() > 0) {
        console->Print("TAS script has been activated.\n");
    }

    active = true;
    currentTick = -1;

    for (TasFramebulk fb : framebulkQueue) {
        if (fb.tick > lastTick) {
            lastTick = fb.tick;
        }
    }

    //TODO: put level change and save loading here, depending on start info

    if (startInfo.type == StartImmediately) {
        ready = true;
    } else {
        ready = false;
    }
}

void TasPlayer::Start() {
    ready = true;
    currentTick = -1;
    tasController->Enable();
}

void TasPlayer::Stop()
{
    if (sar_tas2_debug.GetInt() > 0) {
        console->Print("TAS script has ended after %d ticks.\n", currentTick);
    }

    active = false;
    ready = false;
    currentTick = 0;
    tasController->Disable();   
}

// returns raw framebulk that should be used right now
TasFramebulk TasPlayer::GetCurrentRawFramebulk()
{
    int closestTime = INT_MAX;
    TasFramebulk closest;
    for (TasFramebulk framebulk : framebulkQueue) {
        int timeDist = currentTick - framebulk.tick;
        if (timeDist >= 0 && timeDist < closestTime) {
            closestTime = timeDist;
            closest = framebulk;
        }
    }
    return closest;
}

// returns framebulk processed using tools and game's info in current tick.
TasFramebulk TasPlayer::GetCurrentProcessedFramebulk()
{
    TasFramebulk rawFb = GetCurrentRawFramebulk();

    //TODO: all of the tools processing goes here

    return rawFb;
}


/*
    This function is called in ControllerMove, which is what completely
    avoids sv_alternateticks behaviour. Technically speaking, the game
    fetches controller inputs on each ControllerMove, so that should be
    perfectly legal action, BUT in case it is not, the ultimate test would
    be to debug real controller values - if you're able to get 60 different
    values in one second with sv_alternateticks on, a theory is proven.
*/

void TasPlayer::FetchInputs(TasController* controller)
{
    TasFramebulk fb = GetCurrentProcessedFramebulk();

    if (sar_tas2_debug.GetInt() > 1 && fb.tick == currentTick) {
        console->Print("[%d]\n", fb.tick);
    }

    controller->SetViewAnalog(fb.viewAnalog.x, fb.viewAnalog.y);
    controller->SetMoveAnalog(fb.moveAnalog.x, fb.moveAnalog.y);
    for (int i = 0; i < TAS_CONTROLLER_INPUT_COUNT; i++) {
        controller->SetButtonState((TasControllerInput)i, fb.buttonStates[i]);
    }
    for (char* cmd : fb.commands) {
        controller->AddCommandToQueue(cmd);
    }
}



void TasPlayer::Update()
{
    if (active) {
        if (!ready && !session->isRunning) {
            Start();
        }

        if (ready && session->isRunning) {
            currentTick++;
            //console->Print("TasPlayer::Update (%d)\n", currentTick);
        }

        if (currentTick > lastTick) {
            Stop();
        }
    }
}




CON_COMMAND(sar_tas2_test,
    "Activates test TAS.")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 1) {
        return console->Print(sar_tas2_test.ThisPtr()->m_pszHelpString);
    }

    tasPlayer->Activate();
}