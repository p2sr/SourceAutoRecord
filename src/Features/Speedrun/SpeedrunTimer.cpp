#include "SpeedrunTimer.hpp"

#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "TimerInterface.hpp"
#include "TimerResult.hpp"
#include "TimerRule.hpp"
#include "TimerSplit.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Command.hpp"
#include "Game.hpp"
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
    , category("any")
    , offset(0)
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

    this->total = this->offset;
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
            this->InitRules();
        }
    }
    if (this->state == TimerState::Paused) {
        if (mapChanged) {
            console->Print("Speedrun split!\n");
            this->result.get()->Split(this->total, this->map);
            this->pb.get()->UpdateSplit(this->map);
        }
    } else if (this->state == TimerState::Running) {
        this->session = *engineTicks - this->base;
        this->total = this->prevTotal + this->session;
        this->liveSplit.get()->Update(this);
    }
}
void SpeedrunTimer::CheckRules(const int* engineTicks)
{
    auto action = TimerAction::DoNothing;
    TimerRule* source = nullptr;

    for (auto& rule : this->rules) {
        if (!rule->madeAction) {
            action = rule->Dispatch();
            if (action != TimerAction::DoNothing) {
                source = rule;
                break; // Only allow one action
            }
        }
    }

    switch (action) {
    case TimerAction::Split:
        console->Print("Speedrun split!\n");
        this->result.get()->Split(this->total, this->map);
        this->pb.get()->UpdateSplit(this->map);
        source->madeAction = true;
        break;
    case TimerAction::Start:
        this->Start(engineTicks);
        source->madeAction = true;
        break;
    case TimerAction::End:
        if (this->IsActive()) {
            this->Stop(engineTicks);
            source->madeAction = true;
        }
    default:
        break;
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
    } else {
        console->Print("Ready for new speedun!\n");
        this->Reset();
    }
}
void SpeedrunTimer::Reset()
{
    this->total = 0;
    this->prevTotal = 0;
    TimerRule::ResetAll();
    this->InitRules();
}
int SpeedrunTimer::GetSession()
{
    return this->session;
}
int SpeedrunTimer::GetTotal()
{
    return this->total + this->offset;
}
char* SpeedrunTimer::GetCurrentMap()
{
    return this->map;
}
void SpeedrunTimer::LoadRules(Game* game)
{
    auto filtered = TimerRule::FilterByGame(game);
    if (filtered != 0)
        console->DevMsg("Loaded %i speedrun rules!\n", filtered);
}
void SpeedrunTimer::InitRules()
{
    this->rules.clear();
    for (const auto& rule : TimerRule::list) {
        if (!std::strcmp(this->category, rule->categoryName) && !std::strcmp(this->map, rule->mapName)) {
            this->rules.push_back(rule);
        }
    }
}
void SpeedrunTimer::ReloadRules()
{
    for (const auto& rule : this->rules) {
        if (!rule->Load()) {
            console->Warning("Failed to load rule: %s -> %s\n", rule->categoryName, rule->mapName);
        }
    }
}
void SpeedrunTimer::UnloadRules()
{
    for (const auto& rule : this->rules) {
        rule->Unload();
    }
}
const std::vector<TimerRule*>& SpeedrunTimer::GetRules()
{
    return this->rules;
}
void SpeedrunTimer::SetIntervalPerTick(const float* ipt)
{
    this->ipt = *ipt;
    this->liveSplit->SetIntervalPerTick(ipt);
}
const float SpeedrunTimer::GetIntervalPerTick()
{
    return this->ipt;
}
void SpeedrunTimer::SetCategory(const char* category)
{
    this->category = category;
}
const char* SpeedrunTimer::GetCategory()
{
    return this->category;
}
void SpeedrunTimer::SetOffset(const int offset)
{
    this->offset = offset;
}
const int SpeedrunTimer::GetOffset()
{
    return this->offset;
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

    auto segment = 0;

    for (auto& split : result->splits) {
        auto ticks = split->GetTotal();
        auto time = SpeedrunTimer::Format(ticks * this->ipt);

        for (const auto& seg : split->segments) {
            auto total = split->GetTotal();
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

// Commands

CON_COMMAND(sar_speedrun_start, "Prints result of speedrun.\n")
{
    speedrun->Start(engine->tickcount);
}
CON_COMMAND(sar_speedrun_stop, "Prints result of speedrun.\n")
{
    speedrun->Stop();
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
        console->Print("sar_speedrun_export <file_name> : Saves speedrun result to a csv file.\n");
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
        console->Print("sar_speedrun_import <file_name> : Imports speedrun data file.\n");
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
CON_COMMAND(sar_speedrun_rules, "Prints currently loaded rules which the timer will follow.\n")
{
    auto rules = speedrun->GetRules();
    if (rules.empty()) {
        return console->Print("No rules loaded!\n");
    }

    for (const auto& rule : rules) {
        console->Print("%s -> %s\n", rule->categoryName, rule->mapName);
    }
}
CON_COMMAND(sar_speedrun_all_rules, "Prints all rules which the timer might follow.\n")
{
    auto rules = TimerRule::list;
    if (rules.empty()) {
        return console->Print("No rules loaded!\n");
    }

    for (const auto& rule : rules) {
        console->Print("%s -> %s\n", rule->categoryName, rule->mapName);
    }
}
CON_COMMAND(sar_speedrun_category, "Sets the category for a speedrun.\n")
{
    if (args.ArgC() == 2) {
        speedrun->SetCategory(args[1]);
        speedrun->InitRules();
    }

    console->Print("Current category: %s\n", speedrun->GetCategory());
}
CON_COMMAND(sar_speedrun_offset, "Sets offset in ticks at which the timer should start.\n")
{
    if (args.ArgC() == 2) {
        if (speedrun->IsActive()) {
            return console->Print("Cannot change offset during an active speedrun.\n");
        }

        speedrun->SetOffset(std::atoi(args[1]));
    }

    console->Print("Current offset: %i\n", speedrun->GetOffset());
}
