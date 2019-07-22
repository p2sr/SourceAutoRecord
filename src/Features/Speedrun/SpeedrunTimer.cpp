#include "SpeedrunTimer.hpp"

#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "TimerCategory.hpp"
#include "TimerInterface.hpp"
#include "TimerResult.hpp"
#include "TimerRule.hpp"
#include "TimerSplit.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Command.hpp"
#include "Game.hpp"
#include "Variable.hpp"

Variable sar_speedrun_autostart("sar_speedrun_autostart", "0",
    "Starts speedrun timer automatically on first frame after a load.\n");
Variable sar_speedrun_autostop("sar_speedrun_autostop", "0",
    "Stops speedrun timer automatically when going into the menu.\n");
Variable sar_speedrun_standard("sar_speedrun_standard", "1",
    "Timer automatically starts, splits and stops.\n");
Variable sar_speedrun_time_pauses("sar_speedrun_time_pauses", "1",
    "Timer automatically adds non-simulated ticks when server pauses.\n");
Variable sar_speedrun_smartsplit("sar_speedrun_smartsplit", "1",
    "Timer interface only splits once per level change.\n");

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
    , category(nullptr)
    , offset(0)
    , pause(0)
    , visitedMaps()
{
    this->pubInterface = std::make_unique<TimerInterface>();
    this->result = std::make_unique<TimerResult>();
    this->pb = std::make_unique<TimerResult>();

    this->hasLoaded = true;
}
bool SpeedrunTimer::IsActive()
{
    return this->state == TimerState::Running
        || this->state == TimerState::Paused;
}
void SpeedrunTimer::Start(const int engineTicks)
{
    this->StatusReport("Speedrun started!\n");
    this->base = engineTicks;

    if (this->IsActive()) {
        this->pubInterface.get()->SetAction(TimerAction::Restart);
    } else {
        this->pubInterface.get()->SetAction(TimerAction::Start);
    }

    this->total = this->prevTotal = this->offset;
    this->pause = 0;
    this->state = TimerState::Running;
    this->visitedMaps.clear();
    this->visitedMaps.push_back(this->map);

    this->result.get()->Reset();
    this->result.get()->NewSplit(this->total, this->GetCurrentMap());
}
void SpeedrunTimer::Pause()
{
    if (this->state == TimerState::Running) {
        this->StatusReport("Speedrun paused!\n");
        this->pubInterface.get()->SetAction(TimerAction::Pause);
        this->state = TimerState::Paused;
        this->prevTotal = this->total;
        this->result.get()->AddSegment(this->session + this->pause);
    }
}
void SpeedrunTimer::Resume(const int engineTicks)
{
    if (this->state == TimerState::Paused) {
        this->StatusReport("Speedrun resumed!\n");
        this->pubInterface.get()->SetAction(TimerAction::Resume);
        this->state = TimerState::Running;
        this->base = engineTicks;
        this->pause = 0;
    }
}
void SpeedrunTimer::PreUpdate(const int engineTicks, const char* engineMap)
{
    if (this->state != TimerState::Running) {
        if (std::strncmp(this->map, engineMap, sizeof(this->map))) {
            std::strncpy(this->map, engineMap, sizeof(this->map));

            auto visited = false;
            if (this->state == TimerState::Paused && sar_speedrun_smartsplit.GetBool() && std::strlen(this->map) != 0) {
                for (auto& map : this->visitedMaps) {
                    if (!map.compare(this->map)) {
                        visited = true;
                    }
                }

                if (!visited) {
                    this->visitedMaps.push_back(this->map);
                }
            }

            console->DevMsg("Speedrun map change: %s\n", this->GetCurrentMap());
            if (this->state == TimerState::Paused && !visited) {
                this->Split(visited);
            }
            this->InitRules();
        }
    }
}
void SpeedrunTimer::PostUpdate(const int engineTicks, const char* engineMap)
{
    if (this->state == TimerState::Running) {
        this->session = engineTicks - this->base;
        this->total = this->prevTotal + this->session + this->pause;
        this->pubInterface.get()->Update(this);
    }
}
void SpeedrunTimer::CheckRules(const int engineTicks)
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
        this->Split(false);
        source->madeAction = true;
        break;
    case TimerAction::Start:
        this->Start(engineTicks);
        source->madeAction = true;
        break;
    case TimerAction::End:
        if (this->IsActive()) {
            this->Stop();
            source->madeAction = true;
        }
    default:
        break;
    }
}
void SpeedrunTimer::Stop(bool addSegment)
{
    if (this->IsActive()) {
        this->StatusReport("Speedrun stopped!\n");
        this->pubInterface.get()->SetAction(TimerAction::End);
        this->state = TimerState::NotRunning;
        if (addSegment) {
            this->result.get()->AddSegment(this->session + this->pause);
        }
        this->result.get()->EndSplit(this->total);
        this->pause = 0;
    } else {
        console->Print("Ready for new speedun!\n");
        this->pubInterface.get()->SetAction(TimerAction::Reset);
        this->Reset();
    }
}
void SpeedrunTimer::Reset()
{
    this->total = this->offset;
    this->prevTotal = 0;
    this->base = 0;
    this->pause = 0;
    TimerCategory::ResetAll();
    this->InitRules();
}
void SpeedrunTimer::Split(bool visited)
{
    if (this->IsActive()) {
        this->StatusReport("Speedrun split!\n");
        this->result.get()->Split(this->total, this->GetCurrentMap());
        this->pb.get()->UpdateSplit(this->GetCurrentMap());
        if (!visited) {
            this->pubInterface.get()->SetAction(TimerAction::Split);
        }
    }
}
void SpeedrunTimer::IncrementPauseTime()
{
    ++this->pause;
}
int SpeedrunTimer::GetSession()
{
    return this->session;
}
int SpeedrunTimer::GetTotal()
{
    return this->total;
}
const char* SpeedrunTimer::GetCurrentMap()
{
    return (std::strlen(this->map) != 0) ? this->map : "menu";
}
void SpeedrunTimer::LoadRules(Game* game)
{
    auto filtered = TimerCategory::FilterByGame(game);
    if (filtered != 0) {
        this->category = TimerCategory::list[0];
        console->DevMsg("Loaded %i speedrun %s!\n", filtered, (filtered == 1) ? "category" : "categories");
    }
}
void SpeedrunTimer::InitRules()
{
    this->rules.clear();
    if (this->category) {
        for (const auto& rule : this->category->rules) {
            if (rule->IsEmpty() || !std::strcmp(this->map, rule->mapName)) {
                this->rules.push_back(rule);
            }
        }
    }
}
void SpeedrunTimer::ReloadRules()
{
    for (const auto& rule : this->rules) {
        if (!rule->Load()) {
            console->Warning("Failed to load rule: %s -> %s\n", rule->name, rule->mapName);
        } else {
            console->DevMsg("Loaded rule: %s -> %s\n", rule->name, rule->mapName);
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
    this->pubInterface->SetIntervalPerTick(ipt);
}
const float SpeedrunTimer::GetIntervalPerTick()
{
    return this->ipt;
}
void SpeedrunTimer::SetCategory(TimerCategory* category)
{
    this->category = category;
}
TimerCategory* SpeedrunTimer::GetCategory()
{
    return this->category;
}
void SpeedrunTimer::SetOffset(const int offset)
{
    this->offset = this->total = offset;
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
                 << SpeedrunTimer::Format(seg.session * this->ipt).c_str() << ","
                 << ticks << ","
                 << time.c_str() << ","
                 << total << ","
                 << SpeedrunTimer::Format(total * this->ipt).c_str() << ","
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
        auto pb = new TimerResult();
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
                pb->NewSplit(total - segment, map);
            } else if (elements[0] != lastMap) {
                pb->Split(total - segment, map);
            }

            pb->AddSegment(segment);
            lastMap = elements[0];

            totaltotal = total;
            ++row;
        }

        if (!pb->splits.empty()) {
            pb->EndSplit(totaltotal);
            pb->total = totaltotal;
        }

        this->pb.reset(pb);
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
void SpeedrunTimer::StatusReport(const char* message)
{
    console->Print("%s", message);
    console->DevMsg("%s\n", SpeedrunTimer::Format(this->total * this->ipt).c_str());
}
SpeedrunTimer::~SpeedrunTimer()
{
    this->pubInterface.reset();
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

// Completion Function

int sar_category_CompletionFunc(const char* partial,
    char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    const char* cmd = "sar_speedrun_category ";
    char* match = (char*)partial;
    if (std::strstr(partial, cmd) == partial) {
        match = match + std::strlen(cmd);
    }

    // Filter items
    static auto items = std::vector<std::string>();
    items.clear();
    for (auto& cat : TimerCategory::list) {
        if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
            break;
        }

        if (std::strlen(match) != std::strlen(cmd)) {
            if (std::strstr(cat->name, match)) {
                items.push_back(cat->name);
            }
        } else {
            items.push_back(cat->name);
        }
    }

    // Copy items into list buffer
    auto count = 0;
    for (auto& item : items) {
        std::strcpy(commands[count++], (std::string(cmd) + item).c_str());
    }

    return count;
}

// Commands

CON_COMMAND(sar_speedrun_start, "Starts speedrun timer manually.\n")
{
    speedrun->Start(engine->GetTick());
}
CON_COMMAND(sar_speedrun_stop, "Stops speedrun timer manually.\n")
{
    speedrun->Stop();
}
CON_COMMAND(sar_speedrun_split, "Splits speedrun timer manually.\n")
{
    speedrun->Split(false);
}
CON_COMMAND(sar_speedrun_pause, "Pauses speedrun timer manually.\n")
{
    speedrun->Pause();
}
CON_COMMAND(sar_speedrun_resume, "Resumes speedrun timer manually.\n")
{
    speedrun->Resume(engine->GetTick());
}
CON_COMMAND(sar_speedrun_reset, "Resets speedrun timer.\n")
{
    if (speedrun->IsActive()) {
        speedrun->Stop();
    }
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
CON_COMMAND(sar_speedrun_export, "Saves speedrun result to a csv file.\n"
                                 "Usage: sar_speedrun_export <file_name>\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_speedrun_export.ThisPtr()->m_pszHelpString);
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
CON_COMMAND(sar_speedrun_export_pb, "Saves speedrun personal best to a csv file.\n"
                                    "Usage: sar_speedrun_export_pb <file_name>\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_speedrun_export_pb.ThisPtr()->m_pszHelpString);
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
CON_COMMAND_AUTOCOMPLETEFILE(sar_speedrun_import, "Imports speedrun data file.\n"
                                                  "Usage: sar_speedrun_import <file_name>\n",
    0, 0, csv)
{
    if (args.ArgC() != 2) {
        return console->Print(sar_speedrun_import.ThisPtr()->m_pszHelpString);
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
CON_COMMAND_F_COMPLETION(sar_speedrun_category, "Sets the category for a speedrun.\n", 0, sar_category_CompletionFunc)
{
    if (!speedrun->GetCategory() || TimerCategory::list.empty()) {
        return console->Print("This game does not have any categories!\n");
    }

    auto PrintCategory = []() {
        auto category = speedrun->GetCategory();
        console->Print("Current category: %s\n", category->name);
        console->Msg("Rules:\n");
        for (auto const& rule : category->rules) {
            console->Msg("  -> %s (%s)\n", rule->name, rule->mapName);
        }
    };

    if (args.ArgC() != 2) {
        return PrintCategory();
    }

    for (auto const& category : TimerCategory::list) {
        if (!std::strcmp(category->name, args[1])) {
            speedrun->SetCategory(category);
            speedrun->InitRules();
        }
    }

    return PrintCategory();
}
CON_COMMAND(sar_speedrun_offset, "Sets offset in ticks at which the timer should start.\n")
{
    if (args.ArgC() == 2) {
        if (speedrun->IsActive()) {
            return console->Print("Cannot change offset during an active speedrun!\n");
        }

        auto offset = std::atoi(args[1]);
        if (offset < 0) {
            return console->Print("Offset cannot be negative!\n");
        }

        speedrun->SetOffset(offset);
    }

    console->Print("Timer will start at: %s\n", SpeedrunTimer::Format(speedrun->GetOffset() * speedrun->GetIntervalPerTick()).c_str());
}
