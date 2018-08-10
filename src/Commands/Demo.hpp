#pragma once
#include "Modules/Console.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Tier1.hpp"

#include "Features/Demo/Demo.hpp"
#include "Features/Demo/DemoParser.hpp"

#include "Cheats.hpp"
#include "Command.hpp"

CON_COMMAND_AUTOCOMPLETEFILE(sar_time_demo, "Parses a demo and prints some information about it.", 0, 0, dem)
{
    if (args.ArgC() != 2) {
        console->Print("sar_time_demo [demo_name] : Parses a demo and prints some information about it.\n");
        return;
    }

    std::string name;
    if (args[1][0] == '\0') {
        if (Engine::DemoPlayer::DemoName[0] != '\0') {
            name = std::string(Engine::DemoPlayer::DemoName);
        } else {
            console->Print("No demo was recorded or played back!\n");
            return;
        }
    } else {
        name = std::string(args[1]);
    }

    DemoParser parser;
    parser.outputMode = sar_time_demo_dev.GetInt();

    Demo demo;
    auto dir = std::string(Engine::GetGameDirectory()) + std::string("/") + name;
    if (parser.Parse(dir, &demo)) {
        parser.Adjust(&demo);
        console->Print("Demo:     %s\n", name.c_str());
        console->Print("Client:   %s\n", demo.clientName);
        console->Print("Map:      %s\n", demo.mapName);
        console->Print("Ticks:    %i\n", demo.playbackTicks);
        console->Print("Time:     %.3f\n", demo.playbackTime);
        console->Print("Tickrate: %.3f\n", demo.Tickrate());
    } else {
        console->Print("Could not parse \"%s\"!\n", name.c_str());
    }
}

CON_COMMAND_AUTOCOMPLETEFILE(sar_time_demos, "Parses multiple demos and prints the total sum of them.", 0, 0, dem)
{
    if (args.ArgC() <= 1) {
        console->Print("sar_time_demos [demo_name] [demo_name2] [etc.] : Parses multiple demos and prints the total sum of them.\n");
        return;
    }

    int totalTicks = 0;
    float totalTime = 0;
    bool printTotal = false;

    DemoParser parser;
    parser.outputMode = sar_time_demo_dev.GetInt();

    std::string name;
    std::string dir = std::string(Engine::GetGameDirectory()) + std::string("/");
    for (int i = 1; i < args.ArgC(); i++) {
        name = std::string(args[i]);

        Demo demo;
        if (parser.Parse(dir + name, &demo)) {
            parser.Adjust(&demo);
            console->Print("Demo:     %s\n", name.c_str());
            console->Print("Client:   %s\n", demo.clientName);
            console->Print("Map:      %s\n", demo.mapName);
            console->Print("Ticks:    %i\n", demo.playbackTicks);
            console->Print("Time:     %.3f\n", demo.playbackTime);
            console->Print("Tickrate: %.3f\n", demo.Tickrate());
            console->Print("---------------\n");
            totalTicks += demo.playbackTicks;
            totalTime += demo.playbackTime;
            printTotal = true;
        } else {
            console->Print("Could not parse \"%s\"!\n", name.c_str());
        }
    }
    if (printTotal) {
        console->Print("Total Ticks: %i\n", totalTicks);
        console->Print("Total Time: %.3f\n", totalTime);
    }
}
