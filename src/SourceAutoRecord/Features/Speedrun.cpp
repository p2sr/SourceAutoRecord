#include "Speedrun.hpp"

namespace Speedrun {

class Timer;

TimerRule::TimerRule(char* map, char* activator, TimerAction action)
{
    this->map = map;
    this->activator = activator;
    this->action = action;
}
void TimerRule::Check(char* map, char* activator, int engineTicks, Timer* timer)
{
    if (!map || !activator)
        return;
    if (std::strcmp(this->activator, activator))
        return;
    if (std::strcmp(this->map, map))
        return;

    if (action == TimerAction::Start)
        timer->Start(engineTicks);
    else if (action == TimerAction::End)
        timer->Stop();
}

int TimerSplit::GetTotal()
{
    auto result = 0;
    for (auto seg: this->segments) {
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
    for (auto split : this->splits) {
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

TimerInterface::TimerInterface(float ipt)
{
    this->ipt = ipt;
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
{
    this->liveSplit = std::make_unique<TimerInterface>(ipt);
    this->ipt = ipt;
    this->result = std::make_unique<TimerResult>();
    this->pb = std::make_unique<TimerResult>();
    this->rules = std::make_unique<TimerRules>();
}
bool Timer::IsRunning()
{
    return this->state != TimerState::NotRunning;
}
void Timer::Start(int engineTicks)
{
    this->base = engineTicks;

    if (this->IsRunning()) {
        this->liveSplit.get()->SetAction(TimerAction::Restart);
    } else {
        this->liveSplit.get()->SetAction(TimerAction::Start);
    }

    this->state = TimerState::Running;
    this->result.get()->Reset();
}
void Timer::Unpause(int engineTicks)
{
    if (this->state == TimerState::Paused) {
        this->state = TimerState::Running;
        this->base = engineTicks;
    }
}
void Timer::Update(int engineTicks, char* engineMap, bool engineIsPaused)
{
    if (this->IsRunning()) {
        if (engineIsPaused) {
            // Save time from previous tick
            if (this->state == TimerState::Running) {
                this->state = TimerState::Paused;
                this->prevTotal = this->total;
                this->result.get()->AddSegment(this->session);
            }

            // Detect map change
            if (!std::strcmp(this->map, engineMap)) {
                this->liveSplit.get()->SetAction(TimerAction::Split);
                this->map = map;
                this->result.get()->AddSplit(this->total, this->map);
                this->pb.get()->UpdateSplit(this->map);
            }
        } else {
            this->session = engineTicks - this->base;
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
void Timer::CheckRules(char* map, char* activator, int engineTicks)
{
    for (auto rule : *this->rules) {
        rule.Check(map, activator, engineTicks, this);
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
void Timer::SetIntervalPerTick(float ipt)
{
    this->ipt = ipt;
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
        }
        else {
            snprintf(format, sizeof(format), "%i:%02i.%03i", min, sec, ms);
        }
    }
    else {
        snprintf(format, sizeof(format), "%i.%03i", sec, ms);
    }

    return std::string(format);
}

Timer* timer = new Timer();
}