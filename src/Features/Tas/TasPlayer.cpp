#include "TasPlayer.hpp"

#include "Variable.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Features/Session.hpp"
#include "Features/Tas/TasTool.hpp"
#include "Features/Tas/TasTools/TestTool.hpp"

Variable sar_tas2_debug("sar_tas2_debug", "0", 0, 2, "Debug TAS informations. 0 - none, 1 - basic, 2 - all.");

TasPlayer* tasPlayer;

TasPlayer::TasPlayer()
{
    framebulkQueue.push_back({ 61, { 0, 0 }, { 0, 0 }, { false, true, false, false, false, false } });
    framebulkQueue.push_back({ 71, { 0, -1 }, { 0, 0 }, { false, true, false, false, false, false } });
    framebulkQueue.push_back({ 73, { 1, 0 }, { 0, 0 }, { false, true, false, false, false, false } });
    framebulkQueue.push_back({ 201, { 0, 0 }, { 0, 0 }, { false, false, false, false, false, false }, { "pause" } });
}

TasPlayer::~TasPlayer()
{
    framebulkQueue.clear();
}

int TasPlayer::GetTick() 
{
    return currentTick;
}

void TasPlayer::Activate()
{
    for (TasTool* tool : TasTool::GetList()) {
        tool->Reset();
    }

    active = true;
    currentTick = 0;

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

    if (sar_tas2_debug.GetInt() > 0) {
        console->Print("TAS script has been activated.\n");
    }
}

void TasPlayer::Start() {
    ready = true;
    currentTick = 0;
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

    rawFb.tick = currentTick;

    for (TasTool* tool : TasTool::GetList()) {
        tool->Apply(rawFb);
    }

    return rawFb;
}


/*
    This function is called in ControllerMove, which is what completely
    avoids sv_alternateticks behaviour. Technically speaking, the game
    fetches controller inputs on each ControllerMove, so that should be
    perfectly legal action. This has been proven by printing out Steam
    controller input after every ControllerMove call and getting three
    different results in three consecutive ticks (shoutouts to Amtyi
    for testing it out for us on his setup)
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
    for (std::string cmd : fb.commands) {
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

            //someone told me that would make physics deterministic >:(
            if (currentTick == 0) {
                //engine->ExecuteCommand("phys_timescale 1");
            }

            // update all tools that needs to be updated
            TasFramebulk fb = GetCurrentRawFramebulk();
            if (fb.tick == currentTick) {
                for (TasToolCommand cmd : fb.toolCmds) {
                    cmd.tool->SetParams(cmd.params);
                }
            }
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