#include "TasPlayer.hpp"

#include "Variable.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Features/Session.hpp"
#include "Features/Tas/TasParser.hpp"
#include "Features/Tas/TasTool.hpp"

#include <fstream>

Variable sar_tas_debug("sar_tas_debug", "1", 0, 2, "Debug TAS informations. 0 - none, 1 - basic, 2 - all.");
Variable sar_tas_tools_enabled("sar_tas_tools_enabled", "1", 0, 1, "Enables tool processing for TAS script making.");
Variable sar_tas_autosave_raw("sar_tas_autosave_raw", "1", 0, 1, "Enables automatic saving of raw, processed TAS scripts.");

TasPlayer* tasPlayer;

std::string TasFramebulk::ToString()
{
    std::string output = "[" + std::to_string(tick) + "] mov: ("
        + std::to_string(moveAnalog.x) + " " + std::to_string(moveAnalog.y) + "), ang:"
        + std::to_string(viewAnalog.x) + " " + std::to_string(viewAnalog.y) + "), btns:";
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
    : startInfo({ TasStartType::UnknownStart, "" })
{
}

TasPlayer::~TasPlayer()
{
    framebulkQueue.clear();
}

void TasPlayer::Activate()
{
    //reset the controller before using it
    tasController->Disable();

    for (TasTool* tool : TasTool::GetList()) {
        tool->Reset();
    }

    active = true;
    startTick = -1;
    currentTick = 0;

    lastTick = 0;
    for (TasFramebulk fb : framebulkQueue) {
        if (fb.tick > lastTick) {
            lastTick = fb.tick;
        }
    }

    ready = false;
    if (startInfo.type == ChangeLevel) {
        //check if map exists
        std::string mapPath = std::string(engine->GetGameDirectory()) + "/maps/" + startInfo.param + ".bsp";
        std::ifstream mapf(mapPath);
        if (mapf.good()) {
            std::string cmd = "map ";
            if (session->isRunning) {
                std::string cmd = "changelevel ";
            }
            cmd += startInfo.param;
            char cmdbuf[128];
            snprintf(cmdbuf, sizeof(cmdbuf), "%s", cmd.c_str());
            engine->ExecuteCommand(cmdbuf);
        } else {
            console->ColorMsg(Color(255, 100, 100), "Cannot activate TAS file - unknown map '%s.'\n", startInfo.param);
            Stop();
            return;
        }
        mapf.close();
    } else if (startInfo.type == LoadQuicksave) {
        //check if save file exists
        std::string savePath = std::string(engine->GetGameDirectory()) + "/" + engine->GetSaveDirName() + startInfo.param + ".sav";
        std::ifstream savef(savePath);
        if (savef.good()) {
            std::string cmd = "load ";
            cmd += startInfo.param;
            char cmdbuf[128];
            snprintf(cmdbuf, sizeof(cmdbuf), "%s", cmd.c_str());
            engine->ExecuteCommand(cmdbuf);
        } else {
            console->ColorMsg(Color(255, 100, 100), "Cannot activate TAS file - unknown save file '%s'.\n", startInfo.param);
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

void TasPlayer::Start()
{
    processedFramebulks.clear();

    ready = true;
    currentTick = 0;
    startTick = -1;
}

void TasPlayer::PostStart()
{
    startTick = server->gpGlobals->tickcount;
    tasController->Enable();
    engine->ExecuteCommand("phys_timescale 1", true);
}

void TasPlayer::Stop()
{
    if (active && ready) {
        console->Print("TAS script has ended after %d ticks.\n", currentTick);

        if (sar_tas_autosave_raw.GetBool()) {
            SaveProcessedFramebulks();
        }
    }

    active = false;
    ready = false;
    currentTick = 0;
    tasController->Disable();
}

// returns raw framebulk that should be used for given tick
TasFramebulk TasPlayer::GetRawFramebulkAt(int tick)
{
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

TasPlayerInfo TasPlayer::GetPlayerInfo(void* player, CMoveData* pMove)
{
    TasPlayerInfo pi;

    pi.tick = *reinterpret_cast<int*>((uintptr_t)player + 3792);
    pi.slot = server->GetSplitScreenPlayerSlot(player);
    pi.surfaceFriction = *reinterpret_cast<float*>((uintptr_t)player + 4096);
    pi.ducked = *reinterpret_cast<bool*>((uintptr_t)player + Offsets::m_bDucked);
    pi.maxSpeed = *reinterpret_cast<float*>((uintptr_t)player + Offsets::m_flMaxspeed);

    unsigned int groundEntity = *reinterpret_cast<unsigned int*>((uintptr_t)player + 344); // m_hGroundEntity
    pi.grounded = groundEntity != 0xFFFFFFFF;

    pi.position = *reinterpret_cast<Vector*>((uintptr_t)player + 460);
    pi.angles = engine->GetAngles(pi.slot);
    pi.velocity = *reinterpret_cast<Vector*>((uintptr_t)player + 364);

    pi.oldButtons = pMove->m_nOldButtons;

    pi.ticktime = 1.0f / 60.0f; // TODO: find actual tickrate variable and put it there

    return pi;
}

void TasPlayer::SetFrameBulkQueue(std::vector<TasFramebulk> fbQueue)
{
    this->framebulkQueue = fbQueue;
}

void TasPlayer::SetStartInfo(TasStartType type, std::string param)
{
    this->startInfo = TasStartInfo{ type, param };
}

void TasPlayer::SaveProcessedFramebulks()
{
    if (processedFramebulks.size() > 0 && tasFileName.size() > 0) {
        if (tasFileName.find("_raw") != std::string::npos) {
            return;
        }
        TasParser::SaveFramebulksToFile(tasFileName, startInfo, processedFramebulks);
    }
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
    //add commands only for tick when framebulk is placed. Don't preserve it to other ticks.
    if (currentTick == fbTick) {
        for (std::string cmd : fb.commands) {
            controller->AddCommandToQueue(cmd);
        }
    }
}

// special tools have to be parsed in input processing part.
// because of alternateticks, a pair of inputs are created and then executed at the same time,
// meaning that second tick in pair reads outdated info.
void TasPlayer::PostProcess(void* player, CMoveData* pMove)
{

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

    

    for (TasTool* tool : TasTool::GetList()) {
        tool->Apply(fb, playerInfo);
    }

    // add processed framebulk to the pMove
    // using angles from playerInfo, as these seem to be the most accurate
    // pMove ones are created before tool parsing and GetAngles is wacky.
    // idk, as long as it produces the correct script file we should be fine lmfao
    pMove->m_vecAngles.y = playerInfo.angles.y + fb.viewAnalog.x;
    pMove->m_vecAngles.x = playerInfo.angles.x + fb.viewAnalog.y;
    pMove->m_vecAngles.x = std::min(std::max(pMove->m_vecAngles.x, -cl_pitchdown.GetFloat()), cl_pitchup.GetFloat());

    pMove->m_vecViewAngles = pMove->m_vecAbsViewAngles = pMove->m_vecAngles;
    engine->SetAngles(playerInfo.slot, pMove->m_vecAngles);

    if (fb.moveAnalog.y > 0.0) {
        pMove->m_flForwardMove = cl_forwardspeed.GetFloat() * fb.moveAnalog.y;
    } else {
        pMove->m_flForwardMove = cl_backspeed.GetFloat() * fb.moveAnalog.y;
    }
    pMove->m_flSideMove = cl_sidespeed.GetFloat() * fb.moveAnalog.x;

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

void TasPlayer::Update()
{
    if (active) {
        if (!ready) {
            if (startInfo.type == StartImmediately || !session->isRunning) {
                Start();
            }
        }
        if (ready && session->isRunning) {
            if (startTick == -1) {
                PostStart();
            } else {
                currentTick++;
            }
        }

        // make sure all ticks are processed by tools before stopping
        if ((!sar_tas_tools_enabled.GetBool() && currentTick > lastTick) || processedFramebulks.size() > lastTick) {
            Stop();
        }
        // also do not allow inputs after TAS has ended
        if (currentTick > lastTick) {
            tasController->Disable();
        }
    }
}

CON_COMMAND(sar_tas_playfile,
    "Plays a TAS script.")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 2) {
        return console->Print(sar_tas_playfile.ThisPtr()->m_pszHelpString);
    }

    std::string fileName(args[1]);
    std::string filePath("tas/" + fileName + ".p2tas");
    try {
        std::vector<TasFramebulk> fb = TasParser::ParseFile(filePath);

        if (fb.size() > 0) {
            tasPlayer->SetFrameBulkQueue(fb);
            tasPlayer->Activate();
        }
    } catch (TasParserException& e) {
        return console->ColorMsg(Color(255, 100, 100), "Error while opening TAS file: %s\n", e.what());
    }
}

CON_COMMAND(sar_tas_stop, "Stop TAS playing\n")
{
    tasPlayer->Stop();
}

CON_COMMAND(sar_tas_save_raw,
    "Saves a processed version of just processed script.")
{
    IGNORE_DEMO_PLAYER();

    if (args.ArgC() != 1) {
        return console->Print(sar_tas_save_raw.ThisPtr()->m_pszHelpString);
    }

    tasPlayer->SaveProcessedFramebulks();
}
