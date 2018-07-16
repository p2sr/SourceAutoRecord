#pragma once
#include "Utils.hpp"

namespace Speedrun {

enum class TimerAction {
    Unknown,
    Start,
    Restart,
    Split,
    End
};

enum class TimerState {
    NotRunning,
    Running,
    Paused
};

class TimerSegment {
public:
    int session;
};

class TimerSplit {
public:
    int entered;
    char* map;
    std::vector<TimerSegment> segments;

    int GetTotal()
    {
        auto result = 0;
        for (auto seg: this->segments) {
            result += seg.session;
        }
        return result;
    }
};

class TimerResult {
public:
    int total;
    TimerSplit curSplit;
    TimerSplit prevSplit;
    std::vector<TimerSplit> splits;

    void AddSplit(int tick, char* map)
    {
        this->prevSplit = this->curSplit;
        this->curSplit = TimerSplit { tick, map };
        this->splits.push_back(this->curSplit);
    }
    void AddSegment(int tick)
    {
        if (!this->splits.empty()) {
            this->splits.back().segments.push_back(TimerSegment { tick });
        }
    }
    void UpdateSplit(char* map)
    {
        for (auto split : this->splits) {
            if (!std::strcmp(map, split.map)) {
                this->prevSplit = this->curSplit;
                this->curSplit = split;
            }
        }
    }
    void Reset()
    {
        this->total = 0;
        this->splits.clear();
    }
};

class Timer {
private:
    char start[16] = "SAR_TIMER_START"; // 0-15
    int session; // 16
    int base; // 20
    int total; // 24
    int prevTotal; // 28
    char* map; // 32
    float ipt; // 36
    TimerAction action; // 40
    TimerState state; // 44
    std::unique_ptr<TimerResult> result; // 48-55
    std::unique_ptr<TimerResult> pb; // 56-63
    char end[14] = "SAR_TIMER_END"; // 64-77

public:
    Timer(float ipt)
    {
        this->ipt = ipt;
        this->result = std::make_unique<TimerResult>();
        this->pb = std::make_unique<TimerResult>();
    }
    bool IsRunning()
    {
        return this->state != TimerState::NotRunning;
    }
    void Start(int engineTicks)
    {
        this->base = engineTicks;

        if (this->IsRunning()) {
            this->action = TimerAction::Restart;
        } else {
            this->action = TimerAction::Start;            
        }

        this->state = TimerState::Running;
        this->result.get()->Reset();
    }
    void Unpause(int engineTicks)
    {
        if (this->state == TimerState::Paused) {
            this->state = TimerState::Running;
            this->base = engineTicks;
        }
    }
    void Update(int engineTicks, char* engineMap, bool engineIsPaused)
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
                    this->action = TimerAction::Split;
                    this->map = map;
                    this->result.get()->AddSplit(this->total, this->map);
                    this->pb.get()->UpdateSplit(this->map);
                }
            } else {
                this->session = engineTicks - this->base;
                this->total = this->prevTotal + this->session;
            }
        }
    }
    void Stop()
    {
        if (this->IsRunning()) {
            this->action = TimerAction::End;
            this->state = TimerState::NotRunning;
            this->result.get()->total = this->total;
        }
    }
    int GetSession()
    {
        return this->session;
    }
    int GetTotal()
    {
        return this->total;
    }
    float GetIntervalPerTick()
    {
        return this->ipt;
    }
    TimerResult* GetResult()
    {
        return this->result.get();
    }
    TimerResult* GetPersonalBest()
    {
        return this->pb.get();
    }
    void SetPersonalBest(TimerResult pb)
    {
        *this->pb = pb;
    }
    int GetSplitDelta()
    {
        return this->result.get()->curSplit.entered - this->pb.get()->prevSplit.entered;
    }
    int GetCurrentDelta()
    {
        return this->total - this->pb.get()->curSplit.entered;
    }
    ~Timer()
    {
        this->result.reset();
        this->pb.reset();
    }
};

Timer* timer;

// Util
std::string TimerFormat(float raw)
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
}