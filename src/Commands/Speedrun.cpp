#include "Speedrun.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Speedrun/SpeedrunTimer.hpp"

#include "Command.hpp"

CON_COMMAND(sar_speedrun_result, "Prints result of speedrun.\n")
{
    auto pb = (args.ArgC() == 2 && !std::strcmp(args[1], "pb"));

    auto session = speedrun->GetSession();
    auto total = speedrun->GetTotal();
    auto ipt = speedrun->GetIntervalPerTick();

    auto result = (pb)
        ? speedrun->GetPersonalBest()
        : speedrun->GetResult();

    if (!pb && speedrun->IsActive()) {
        console->PrintActive("Session: %s (%i)\n", SpeedrunTimer::Format(session * ipt).c_str(), session);
    }

    auto segments = 0;
    for (auto& split : result->splits) {
        auto completedIn = split->GetTotal();
        console->Print("%s -> %s (%i)\n", split->map, SpeedrunTimer::Format(completedIn * ipt).c_str(), completedIn);
        for (const auto& seg : split->segments) {
            console->Msg("  -> %s (%i)\n", SpeedrunTimer::Format(seg.session * ipt).c_str(), seg.session);
            ++segments;
        }
    }

    if (!pb && speedrun->IsActive()) {
        console->PrintActive("Segments: %i\n", segments);
        console->PrintActive("Total:    %s (%i)\n", SpeedrunTimer::Format(total * ipt).c_str(), total);
    } else {
        console->Print("Segments: %i\n", segments);
        console->Print("Total:    %s (%i)\n", SpeedrunTimer::Format(result->total * ipt).c_str(), result->total);
    }
}

CON_COMMAND(sar_speedrun_export, "Saves speedrun result to a csv file.\n")
{
    if (args.ArgC() != 2) {
        console->Print("sar_speedrun_export [file_name] : Saves speedrun result to a csv file.\n");
        return;
    }

    auto filePath = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
    if (filePath.substr(filePath.length() - 4, 4) != ".csv")
        filePath += ".csv";

    if (speedrun->ExportResult(filePath)) {
        console->Print("Exported result!\n");
    } else {
        console->Warning("Failed to export result!\n");
    }
}

CON_COMMAND(sar_speedrun_export_pb, "Saves speedrun personal best to a csv file.\n")
{
    if (args.ArgC() != 2) {
        console->Print("sar_speedrun_export_pb [file_name] : Saves speedrun personal best to a csv file.\n");
        return;
    }

    auto filePath = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
    if (filePath.substr(filePath.length() - 4, 4) != ".csv")
        filePath += ".csv";

    if (speedrun->ExportPersonalBest(filePath)) {
        console->Print("Exported personal best!\n");
    } else {
        console->Warning("Failed to export personal best!\n");
    }
}

CON_COMMAND_AUTOCOMPLETEFILE(sar_speedrun_import, "Imports speedrun data file.", 0, 0, csv)
{
    if (args.ArgC() != 2) {
        console->Print("sar_speedrun_import [file_name] : Imports speedrun data file.\n");
        return;
    }

    auto filePath = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
    if (filePath.substr(filePath.length() - 4, 4) != ".csv")
        filePath += ".csv";

    if (speedrun->ImportPersonalBest(filePath)) {
        console->Print("Imported %s!\n", args[1]);
    } else {
        console->Warning("Failed to import file!\n");
    }
}

CON_COMMAND(sar_speedrun_rules, "Prints loaded rules which the timer will follow.\n")
{
    auto rules = speedrun->GetRules();
    if (rules.size() == 0) {
        console->Print("No rules loaded!\n");
        return;
    }

    for (const auto& rule : rules) {
        console->Print("%s\n", rule.map);
        console->Print("    -> Target: %s\n", rule.target);
        console->Print("    -> Input:  %s\n", rule.targetInput);
        console->Print("    -> Type:   %s\n", (rule.action == TimerAction::Start) ? "Start" : "Stop");
    }
}
