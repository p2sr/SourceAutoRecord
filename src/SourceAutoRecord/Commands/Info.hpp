#pragma once
#include "Modules/Console.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"

#include "Command.hpp"
#include "Listener.hpp"

#include "SAR.hpp"

CON_COMMAND(sar_session, "Prints the current tick of the server since it has loaded.\n")
{
    int tick = Engine::GetSessionTick();
    console->Print("Session Tick: %i (%.3f)\n", tick, Engine::ToTime(tick));
    if (*Engine::DemoRecorder::m_bRecording) {
        tick = Engine::DemoRecorder::GetTick();
        console->Print("Demo Recorder Tick: %i (%.3f)\n", tick, Engine::ToTime(tick));
    }
    if (Engine::DemoPlayer::IsPlaying()) {
        tick = Engine::DemoPlayer::GetTick();
        console->Print("Demo Player Tick: %i (%.3f)\n", tick, Engine::ToTime(tick));
    }
}

CON_COMMAND(sar_about, "Prints info about this tool.\n")
{
    console->Print("SourceAutoRecord tells the engine to keep recording when loading a save.\n");
    console->Print("More information at: https://nekzor.github.io/SourceAutoRecord\n");
    console->Print("Game: %s\n", Game::GetVersion());
    console->Print("Version: %s\n", SAR_VERSION);
    console->Print("Build: %s\n", SAR_BUILD);
}