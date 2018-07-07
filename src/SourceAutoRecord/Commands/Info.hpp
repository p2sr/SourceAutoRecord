#pragma once
#include "Modules/Console.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"

#include "Command.hpp"
#include "SourceAutoRecord.hpp"

CON_COMMAND(sar_session, "Prints the current tick of the server since it has loaded.\n")
{
    int tick = Engine::GetTick();
    Console::Print("Session Tick: %i (%.3f)\n", tick, Engine::GetTime(tick));
    if (*Engine::DemoRecorder::m_bRecording) {
        tick = Engine::DemoRecorder::GetTick();
        Console::Print("Demo Recorder Tick: %i (%.3f)\n", tick, Engine::GetTime(tick));
    }
    if (Engine::DemoPlayer::IsPlaying()) {
        tick = Engine::DemoPlayer::GetTick();
        Console::Print("Demo Player Tick: %i (%.3f)\n", tick, Engine::GetTime(tick));
    }
}

CON_COMMAND(sar_about, "Prints info about this tool.\n")
{
    Console::Print("SourceAutoRecord tells the engine to keep recording when loading a save.\n");
    Console::Print("More information at: https://nekzor.github.io/SourceAutoRecord\n");
    Console::Print("Game: %s\n", Game::GetVersion());
    Console::Print("Version: %s\n", SAR_VERSION);
    Console::Print("Build: %s\n", SAR_BUILD);
}

//CON_COMMAND_AUTOCOMPLETEFILE(sar_map, "maps", 0, 0, bsp)
//{
//    if (args.ArgC() != 2) {
//        Console::Print("sar_map [map_name] : Starts server with specified map.\n");
//        return;
//    }
//}