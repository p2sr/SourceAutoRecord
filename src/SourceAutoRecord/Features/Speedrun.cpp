#include "Speedrun.hpp"

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

int TimerSplit::GetTotal()
{
    auto result = 0;
    for (const auto& seg : this->segments) {
        result += seg.session;
    }
    return result;
}

void TimerResult::AddSplit(int tick, char* map)
{
    this->prevSplit = this->curSplit;
    this->curSplit = TimerSplit { tick, map };
    this->splits.push_back(this->curSplit);
}
void TimerResult::AddSegment(int tick)
{
    if (!this->splits.empty()) {
        this->splits.back().segments.push_back(TimerSegment { tick });
    }
}
void TimerResult::UpdateSplit(char* map)
{
    for (const auto& split : this->splits) {
        if (!std::strcmp(map, split.map)) {
            this->prevSplit = this->curSplit;
            this->curSplit = split;
        }
    }
}
void TimerResult::Reset()
{
    this->total = 0;
    this->splits.clear();
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
bool Timer::IsRunning()
{
    return this->state == TimerState::Running
        || this->state == TimerState::Paused;
}
void Timer::Start(const int* engineTicks)
{
    this->base = *engineTicks;

    if (this->IsRunning()) {
        this->liveSplit.get()->SetAction(TimerAction::Restart);
    } else {
        this->liveSplit.get()->SetAction(TimerAction::Start);
    }

    this->state = TimerState::Running;
    this->result.get()->Reset();
}
void Timer::Unpause(const int* engineTicks)
{
    if (this->state == TimerState::Paused) {
        this->state = TimerState::Running;
        this->base = *engineTicks;
    }
}
void Timer::Update(const int* engineTicks, const bool* engineIsPaused, const char* engineMap)
{
    if (!engineTicks)
        return;
    if (!engineMap || std::strlen(engineMap) == 0)
        return;
    if (!engineIsPaused)
        return;

    auto mapChange = false;
    if (std::strncmp(this->map, engineMap, sizeof(this->map))) {
        std::strncpy(this->map, engineMap, sizeof(this->map));
        mapChange = true;
    }

    if (this->IsRunning()) {
        if (*engineIsPaused) {
            // Completed segment, save time from previous tick
            if (this->state == TimerState::Running) {
                this->state = TimerState::Paused;
                this->prevTotal = this->total;
                this->result.get()->AddSegment(this->session);
            }

            // Split on map change
            if (mapChange) {
                this->liveSplit.get()->SetAction(TimerAction::Split);
                this->result.get()->AddSplit(this->total, this->map);
                this->pb.get()->UpdateSplit(this->map);
            }
        } else {
            this->session = *engineTicks - this->base;
            this->total = this->prevTotal + this->session;
            this->liveSplit.get()->Update(this);
        }
    }
}
void Timer::Stop()
{
    if (this->IsRunning()) {
        this->liveSplit.get()->SetAction(TimerAction::End);
        this->state = TimerState::NotRunning;
        this->result.get()->total = this->total;
        this->result.get()->AddSegment(this->session);
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
    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.good()) {
        return false;
    }

    auto result = Speedrun::timer->GetResult();
    if (result->splits.empty()) {
        return false;
    }

    file << SAR_SPEEDRUN_EXPORT_HEADER << std::endl;

    auto segment = 0;
    auto ipt = Speedrun::timer->GetIntervalPerTick();

    for (auto& split : result->splits) {
        auto total = split.entered;
        auto totalTime = total * ipt;
        auto ticks = split.GetTotal();
        auto time = ticks * ipt;

        for (const auto& seg : split.segments) {
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

        *this->pb = pb;
        file.close();
        return true;
    }

    return false;
}
int Timer::GetSplitDelta()
{
    return this->result.get()->curSplit.entered - this->pb.get()->prevSplit.entered;
}
int Timer::GetCurrentDelta()
{
    return this->total - this->pb.get()->curSplit.entered;
}
Timer::~Timer()
{
    this->result.reset();
    this->pb.reset();
}
std::string Timer::Format(float raw)
{
    char format[16];

    auto sec = int(std::floor(raw));
    auto ms = int(std::ceil((raw - sec) * 1000));

    if (sec > 60) {
        auto min = sec / 60;
        sec = sec % 60;
        if (min > 60) {
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
