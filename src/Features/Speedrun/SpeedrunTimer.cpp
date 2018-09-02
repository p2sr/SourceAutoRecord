#include "SpeedrunTimer.hpp"

#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "TimerInterface.hpp"
#include "TimerResult.hpp"
#include "TimerRule.hpp"
#include "TimerSplit.hpp"

#include "Command.hpp"
#include "Variable.hpp"

Variable sar_speedrun_autostart("sar_speedrun_autostart", "0", "Starts speedrun timer automatically on first frame after a load.\n");
Variable sar_speedrun_autostop("sar_speedrun_autostop", "0", "Stops speedrun timer automatically when going into the menu.\n");

SpeedrunTimer* speedrun;

SpeedrunTimer::SpeedrunTimer()
    : session(0)
    , base(0)
    , total(0)
    , prevTotal(0)
    , map("unknown")
    , ipt(0.0f)
    , state(TimerState::NotRunning)
    , rules()
{
    this->liveSplit = std::make_unique<TimerInterface>();
    this->result = std::make_unique<TimerResult>();
    this->pb = std::make_unique<TimerResult>();

    this->hasLoaded = true;
}
bool SpeedrunTimer::IsActive()
{
    return this->state == TimerState::Running
        || this->state == TimerState::Paused;
}
void SpeedrunTimer::Start(const int* engineTicks)
{
    console->Print("Speedrun started!\n");
    this->base = *engineTicks;

    if (this->IsActive()) {
        this->liveSplit.get()->SetAction(TimerAction::Restart);
    } else {
        this->liveSplit.get()->SetAction(TimerAction::Start);
    }

    this->total = 0;
    this->prevTotal = 0;
    this->state = TimerState::Running;

    this->result.get()->Reset();
    this->result.get()->NewSplit(this->total, this->map);
}
void SpeedrunTimer::Pause()
{
    if (this->state == TimerState::Running) {
        console->Print("Speedrun paused!\n");
        this->state = TimerState::Paused;
        this->prevTotal = this->total;
        this->result.get()->AddSegment(this->session);
    }
}
void SpeedrunTimer::Unpause(const int* engineTicks)
{
    if (this->state == TimerState::Paused) {
        console->Print("Speedrun unpaused!\n");
        this->state = TimerState::Running;
        this->base = *engineTicks;
    }
}
void SpeedrunTimer::Update(const int* engineTicks, const char* engineMap)
{
    if (!engineTicks || !engineMap || std::strlen(engineMap) == 0)
        return;

    auto mapChanged = false;
    if (this->state != TimerState::Running) {
        if (std::strncmp(this->map, engineMap, sizeof(this->map))) {
            std::strncpy(this->map, engineMap, sizeof(this->map));
            console->DevMsg("Speedrun map change: %s\n", this->map);
            mapChanged = true;
        }
    }

    if (this->state == TimerState::Paused) {
        if (mapChanged) {
            console->Print("Speedrun split!\n");
            this->liveSplit.get()->SetAction(TimerAction::Split);
            this->result.get()->Split(this->total, this->map);
            this->pb.get()->UpdateSplit(this->map);
        }
    } else if (this->state == TimerState::Running) {
        this->session = *engineTicks - this->base;
        this->total = this->prevTotal + this->session;
        this->liveSplit.get()->Update(this);
    }
}
void SpeedrunTimer::Stop(bool addSegment)
{
    if (this->IsActive()) {
        console->Print("Speedrun stopped!\n");
        this->liveSplit.get()->SetAction(TimerAction::End);
        this->state = TimerState::NotRunning;
        if (addSegment) {
            this->result.get()->AddSegment(this->session);
        }
        this->result.get()->EndSplit(this->total);
    }
}
void SpeedrunTimer::AddRule(TimerRule rule)
{
    this->rules.push_back(rule);
}
std::vector<TimerRule> SpeedrunTimer::GetRules()
{
    return this->rules;
}
void SpeedrunTimer::CheckRules(const EventQueuePrioritizedEvent_t* event, const int* engineTicks)
{
    if (this->state != TimerState::Paused) {
        for (auto& rule : this->rules) {
            rule.Check(event, engineTicks, this);
        }
    }
}
int SpeedrunTimer::GetSession()
{
    return this->session;
}
int SpeedrunTimer::GetTotal()
{
    return this->total;
}
char* SpeedrunTimer::GetCurrentMap()
{
    return this->map;
}
void SpeedrunTimer::SetIntervalPerTick(const float* ipt)
{
    this->ipt = *ipt;
    this->liveSplit->SetIntervalPerTick(ipt);
}
float SpeedrunTimer::GetIntervalPerTick()
{
    return this->ipt;
}
TimerResult* SpeedrunTimer::GetResult()
{
    return this->result.get();
}
TimerResult* SpeedrunTimer::GetPersonalBest()
{
    return this->pb.get();
}
bool SpeedrunTimer::ExportResult(std::string filePath, bool pb)
{
    auto result = (pb)
        ? this->GetPersonalBest()
        : this->GetResult();

    if (result->splits.empty()) {
        return false;
    }

    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.good()) {
        return false;
    }

    file << SAR_SPEEDRUN_EXPORT_HEADER << std::endl;

    auto segment = 1;

    for (auto& split : result->splits) {
        auto ticks = split->GetTotal();
        auto time = SpeedrunTimer::Format(ticks * this->ipt);

        for (const auto& seg : split->segments) {
            auto total = split->entered + seg.session;
            file << split->map << ","
                 << seg.session << ","
                 << SpeedrunTimer::Format(seg.session * this->ipt) << ","
                 << ticks << ","
                 << time << ","
                 << total << ","
                 << SpeedrunTimer::Format(total * this->ipt) << ","
                 << ++segment << std::endl;
        }
    }

    file.close();
    return true;
}
bool SpeedrunTimer::ExportPersonalBest(std::string filePath)
{
    return this->ExportResult(filePath, true);
}
bool SpeedrunTimer::ImportPersonalBest(std::string filePath)
{
    std::ifstream file(filePath, std::ios::in);
    if (!file.good()) {
        return false;
    }

    std::string buffer;
    std::getline(file, buffer);

    if (buffer == std::string(SAR_SPEEDRUN_EXPORT_HEADER)) {
        auto pb = TimerResult();
        std::string buffer;
        std::string lastMap;
        auto row = 0;
        auto totaltotal = 0;
        while (std::getline(file, buffer)) {
            std::stringstream line(buffer);
            std::string element;
            std::vector<std::string> elements;
            while (std::getline(line, element, ',')) {
                elements.push_back(element);
            }

            auto map = elements[0].c_str();
            auto segment = std::atoi(elements[1].c_str());
            auto total = std::atoi(elements[5].c_str());

            if (row == 0) {
                pb.NewSplit(total - segment, map);
            } else if (elements[0] != lastMap) {
                pb.Split(total - segment, map);
            }

            pb.AddSegment(segment);
            lastMap = elements[0];

            totaltotal = total;
            ++row;
        }

        if (!pb.splits.empty()) {
            pb.EndSplit(totaltotal);
            pb.total = totaltotal;
        }

        *this->pb = pb;
        file.close();
        return true;
    }

    return false;
}
int SpeedrunTimer::GetSplitDelta()
{
    return this->result.get()->curSplit->entered - this->pb.get()->prevSplit->entered;
}
int SpeedrunTimer::GetCurrentDelta()
{
    return this->total - this->pb.get()->curSplit->entered;
}
SpeedrunTimer::~SpeedrunTimer()
{
    this->liveSplit.reset();
    this->result.reset();
    this->pb.reset();
    this->rules.clear();
}
std::string SpeedrunTimer::Format(float raw)
{
    char format[16];

    auto sec = int(std::floor(raw));
    auto ms = int(std::ceil((raw - sec) * 1000));

    if (sec >= 60) {
        auto min = sec / 60;
        sec = sec % 60;
        if (min >= 60) {
            auto hrs = min / 60;
            min = min % 60;
            snprintf(format, sizeof(format), "%i:%02i:%02i.%03i", hrs, min, sec, ms);
        } else {
            snprintf(format, sizeof(format), "%i:%02i.%03i", min, sec, ms);
        }
    } else {
        snprintf(format, sizeof(format), "%i.%03i", sec, ms);
    }

    return std::string(format);
}

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
