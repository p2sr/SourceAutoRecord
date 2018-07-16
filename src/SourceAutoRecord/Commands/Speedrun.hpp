#pragma once
#include "Modules/Engine.hpp"
#include "Modules/Console.hpp"

#include "Features/Speedrun.hpp"

#include "Command.hpp"

CON_COMMAND(sar_speedrun_start, "Starts speedrun timer manually.\n")
{
    Speedrun::timer->Start(*Engine::tickcount);
    Console::Print("Speedrun started!\n");
}

CON_COMMAND(sar_speedrun_stop, "Stops speedrun timer manually.\n")
{
    if (Speedrun::timer->IsRunning()) {
        Speedrun::timer->Stop();
        Console::Print("Speedrun finished!\n");
    } else {
        Console::Print("Speedrun not started!\n");
    }
}

CON_COMMAND(sar_speedrun_result, "Prints result of speedrun.\n")
{
    auto session = Speedrun::timer->GetSession();
    auto total = Speedrun::timer->GetTotal();
    auto ipt = Speedrun::timer->GetIntervalPerTick();

    if (Speedrun::timer->IsRunning()) {
        Console::PrintActive("Session: %s (%i)\n", Speedrun::TimerFormat(session * ipt).c_str(), session);
        Console::PrintActive("Total:   %s (%i)\n", Speedrun::TimerFormat(total * ipt).c_str(), total);
    } else {
        auto splits = (args.ArgC() == 2 && std::strcmp(args[1], "pb"))
            ? Speedrun::timer->GetPersonalBest()->splits
            : Speedrun::timer->GetResult()->splits;

        auto segments = 0;
        for (auto split: splits) {
            auto completedIn = split.GetTotal();
            Console::Print("%s in %s (%i)\n", split.map, Speedrun::TimerFormat(completedIn * ipt).c_str(), completedIn);
            for (auto seg : split.segments) {
                Console::Msg("-> %s (%i)\n", Speedrun::TimerFormat(seg.session * ipt).c_str(), seg.session);
                segments++;
            }
        }

        Console::Print("Splits: %i\n", segments);
        Console::Print("Total:    %s (%i)\n", Speedrun::TimerFormat(total * ipt).c_str(), total);
    }
}

#define SAR_SPEEDRUN_EXPORT_HEADER "Map,Ticks,Time,Map Ticks,Map Time,Total Ticks,Total Time,Segment"

CON_COMMAND(sar_speedrun_export, "Saves speedrun result to a csv file.\n")
{
    if (args.ArgC() != 2) {
        Console::Print("sar_speedrun_export [file_name] : Saves speedrun result to a csv file.\n");
        return;
    }

    auto filePath = std::string(Engine::GetGameDirectory()) + std::string("/") + std::string(args[1]);
    if (filePath.substr(filePath.length() - 4, 4) != ".csv")
        filePath += ".csv";

    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.good()) {
        Console::Print("File not found.\n");
        return;
    }

    auto result = Speedrun::timer->GetResult();
    if (result->splits.empty()) {
        Console::Print("Nothing to export!\n");
        return;
    }

    file << SAR_SPEEDRUN_EXPORT_HEADER << std::endl;

    auto segment = 0;
    auto ipt = Speedrun::timer->GetIntervalPerTick();

    for (auto split : result->splits) {
        auto total = split.entered;
        auto totalTime = total * ipt;
        auto ticks = split.GetTotal();
        auto time = ticks * ipt;

        for (auto seg : split.segments) {
            file << split.map << ","
                << seg.session << ","
                << (seg.session * ipt) << ","
                << ticks << ","
                << time << ","
                << total << ","
                << totalTime << ","
                << ++segment;
        }
    }

    file.close();
}

CON_COMMAND_AUTOCOMPLETEFILE(sar_speedrun_import, "", 0, 0, csv)
{
    if (args.ArgC() != 2) {
        Console::Print("sar_speedrun_import [file_name] : Imports speedrun data file.\n");
        return;
    }

    auto filePath = std::string(Engine::GetGameDirectory()) + std::string("/") + std::string(args[1]);
    if (filePath.substr(filePath.length() - 4, 4) != ".csv")
        filePath += ".csv";

    std::ifstream file(filePath, std::ios::in);
    if (!file.good()) {
        Console::Print("File not found.\n");
        return;
    }

    std::string buffer;
    std::getline(file, buffer);

    if (buffer == std::string(SAR_SPEEDRUN_EXPORT_HEADER)) {
        auto pb = Speedrun::TimerResult();
        std::string buffer;
        std::string lastMap;
        while (std::getline(file, buffer)) {
            std::stringstream line(buffer);
            std::string element;
            std::vector<std::string> elements;
            while (std::getline(line, element, ',')) {
                elements.push_back(element);
            }

            if (elements[0] != lastMap) {
                pb.AddSplit(atoi(elements[5].c_str()), (char*)elements[0].c_str());
            }
            pb.AddSegment(atoi(elements[1].c_str()));
            lastMap = elements[0];
        }

        Speedrun::timer->SetPersonalBest(pb);
        Console::Print("Imported personal best!\n");
    }
    else {
        Console::Print("Invalid file format!\n");
    }

    file.close();
}