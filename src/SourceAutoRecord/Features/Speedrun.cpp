#include "Speedrun.hpp"

#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Modules/Console.hpp"

namespace Speedrun {

class Timer;

TimerRule::TimerRule(const char* map, const char* activator, TimerAction action)
{
    this->map = map;
    this->activator = activator;
    this->action = action;
}
void TimerRule::Check(Timer* timer, const char* activator, const int* engineTicks)
{
    if (std::strcmp(this->map, timer->GetCurrentMap()))
        return;
    if (!activator || std::strcmp(this->activator, activator))
        return;
    if (this->action == TimerAction::Start)
        timer->Start(engineTicks);
    else if (this->action == TimerAction::End)
        timer->Stop();
}

TimerSplit::TimerSplit(const int ticks, const char* map)
{
    this->entered = ticks;
    std::strcpy(this->map, map);
    this->segments = std::vector<TimerSegment>();
}
int TimerSplit::GetTotal()
{
    return this->finished - this->entered;
}

TimerResult::TimerResult()
    : total(0)
    , curSplit(nullptr)
    , prevSplit(nullptr)
    , splits()
{
}
void TimerResult::NewSplit(const int started, const char* map)
{
    this->prevSplit = this->curSplit;
    this->curSplit = new TimerSplit(started, map);
}
void TimerResult::EndSplit(const int finished)
{
    this->curSplit->finished = finished;
    this->splits.push_back(this->curSplit);
}
void TimerResult::Split(const int ticks, const char* map)
{
    this->EndSplit(ticks);
    this->NewSplit(ticks, map);
}
void TimerResult::AddSegment(int ticks)
{
    this->curSplit->segments.push_back(TimerSegment { ticks });
}
void TimerResult::UpdateSplit(char* map)
{
    for (const auto& split : this->splits) {
        if (!std::strcmp(map, split->map)) {
            this->prevSplit = this->curSplit;
            this->curSplit = split;
        }
    }
}
void TimerResult::Reset()
{
    this->total = 0;
    this->prevSplit = nullptr;

    if (this->curSplit) {
        delete this->curSplit;
        this->curSplit = nullptr;
    }

    for (auto it = this->splits.begin(); it != this->splits.end();) {
        delete *it;
        it = this->splits.erase(it);
    }
}
TimerResult::~TimerResult()
{
    this->Reset();
}

TimerInterface::TimerInterface()
    : start("SAR_TIMER_START")
    , total(0)
    , ipt(0)
    , action(TimerAction::Unknown)
    , end("SAR_TIMER_END")
{
}
void TimerInterface::SetIntervalPerTick(const float* ipt)
{
    this->ipt = *ipt;
}
void TimerInterface::Update(Timer* timer)
{
    this->total = timer->GetTotal();
}
void TimerInterface::SetAction(TimerAction action)
{
    this->action = action;
}

Timer::Timer()
    : session(0)
    , base(0)
    , total(0)
    , prevTotal(0)
    , map("unknown")
    , ipt(0.0f)
    , state(TimerState::NotRunning)
{
    this->liveSplit = std::make_unique<TimerInterface>();
    this->result = std::make_unique<TimerResult>();
    this->pb = std::make_unique<TimerResult>();
    this->rules = std::make_unique<TimerRules>();
}
bool Timer::IsActive()
{
    return this->state == TimerState::Running
        || this->state == TimerState::Paused;
}
void Timer::Start(const int* engineTicks)
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
void Timer::Pause()
{
    if (this->state == TimerState::Running) {
        console->Print("Speedrun paused!\n");
        this->state = TimerState::Paused;
        this->prevTotal = this->total;
        this->result.get()->AddSegment(this->session);
    }
}
void Timer::Unpause(const int* engineTicks)
{
    if (this->state == TimerState::Paused) {
        console->Print("Speedrun unpaused!\n");
        this->state = TimerState::Running;
        this->base = *engineTicks;
    }
}
void Timer::Update(const int* engineTicks, const bool* engineIsLoading, const char* engineMap)
{
    if (!engineTicks)
        return;
    if (!engineMap || std::strlen(engineMap) == 0)
        return;
    if (!engineIsLoading)
        return;

    auto mapChange = false;
    if (std::strncmp(this->map, engineMap, sizeof(this->map))) {
        std::strncpy(this->map, engineMap, sizeof(this->map));
        mapChange = true;
    }

    if (this->IsActive()) {
        if (mapChange) {
            console->Print("Speedrun split!\n");
            this->liveSplit.get()->SetAction(TimerAction::Split);
            this->result.get()->Split(this->total, this->map);
            this->pb.get()->UpdateSplit(this->map);
        }

        if (!*engineIsLoading) {
            this->session = *engineTicks - this->base;
            this->total = this->prevTotal + this->session;
            this->liveSplit.get()->Update(this);
        }
    }
}
void Timer::Stop()
{
    if (this->IsActive()) {
        this->liveSplit.get()->SetAction(TimerAction::End);
        this->state = TimerState::NotRunning;
        this->result.get()->AddSegment(this->session);
        this->result.get()->EndSplit(this->total);
    }
}
void Timer::LoadRules(TimerRules rules)
{
    *this->rules = rules;
}
TimerRules* Timer::GetRules()
{
    return this->rules.get();
}
void Timer::CheckRules(const char* activator, const int* engineTicks)
{
    for (auto& rule : *this->rules) {
        rule.Check(this, activator, engineTicks);
    }
}
int Timer::GetSession()
{
    return this->session;
}
int Timer::GetTotal()
{
    return this->total;
}
char* Timer::GetCurrentMap()
{
    return this->map;
}
void Timer::SetIntervalPerTick(const float* ipt)
{
    this->ipt = *ipt;
    this->liveSplit->SetIntervalPerTick(ipt);
}
float Timer::GetIntervalPerTick()
{
    return this->ipt;
}
TimerResult* Timer::GetResult()
{
    return this->result.get();
}
TimerResult* Timer::GetPersonalBest()
{
    return this->pb.get();
}
bool Timer::ExportResult(std::string filePath, bool pb)
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
        auto time = Timer::Format(ticks * this->ipt);

        for (const auto& seg : split->segments) {
            auto total = split->entered + seg.session;
            file << split->map << ","
                 << seg.session << ","
                 << Timer::Format(seg.session * this->ipt) << ","
                 << ticks << ","
                 << time << ","
                 << total << ","
                 << Timer::Format(total * this->ipt) << ","
                 << ++segment << std::endl;
        }
    }

    file.close();
    return true;
}
bool Timer::ExportPersonalBest(std::string filePath)
{
    return this->ExportResult(filePath, true);
}
bool Timer::ImportPersonalBest(std::string filePath)
{
    std::ifstream file(filePath, std::ios::in);
    if (!file.good()) {
        return false;
    }

    std::string buffer;
    std::getline(file, buffer);

    if (buffer == std::string(SAR_SPEEDRUN_EXPORT_HEADER)) {
        auto pb = Speedrun::TimerResult();
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
int Timer::GetSplitDelta()
{
    return this->result.get()->curSplit->entered - this->pb.get()->prevSplit->entered;
}
int Timer::GetCurrentDelta()
{
    return this->total - this->pb.get()->curSplit->entered;
}
Timer::~Timer()
{
    this->liveSplit.reset();
    this->result.reset();
    this->pb.reset();
    this->rules.reset();
}
std::string Timer::Format(float raw)
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

Timer* timer;
}
