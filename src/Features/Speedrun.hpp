#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Utils/SDK.hpp"

#define SAR_SPEEDRUN_EXPORT_HEADER "Map,Ticks,Time,Map Ticks,Map Time,Total Ticks,Total Time,Segment"

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

class Timer;

class TimerRule {
public:
    const char* map;
    const char* target;
    const char* targetInput;
    TimerAction action;

public:
    TimerRule(const char* map, const char* target, const char* targetInput, TimerAction action);
    void Check(const EventQueuePrioritizedEvent_t* event, const int* engineTicks, Timer* timer);
};

class TimerSegment {
public:
    int session;
};

class TimerSplit {
public:
    int entered;
    int finished;
    char map[64];
    std::vector<TimerSegment> segments;

    TimerSplit(const int start, const char* map);
    int GetTotal();
};

class TimerResult {
public:
    int total;
    TimerSplit* curSplit;
    TimerSplit* prevSplit;
    std::vector<TimerSplit*> splits;

    TimerResult();
    void NewSplit(const int started, const char* map);
    void EndSplit(const int finished);
    void Split(const int ticks, const char* map);
    void AddSegment(int ticks);
    void UpdateSplit(char* map);
    void Reset();
    ~TimerResult();
};

class Timer;

class TimerInterface {
public:
    char start[16]; // 0
    int total; // 16
    float ipt; // 20
    TimerAction action; // 24
    char end[14]; // 28

public:
    TimerInterface();
    void SetIntervalPerTick(const float* ipt);
    void Update(Timer* timer);
    void SetAction(TimerAction action);
};

class Timer {
public:
    std::unique_ptr<TimerInterface> liveSplit;

private:
    int session;
    int base;
    int total;
    int prevTotal;
    char map[64];
    float ipt;
    TimerState state;
    std::unique_ptr<TimerResult> result;
    std::unique_ptr<TimerResult> pb;
    std::vector<TimerRule> rules;

public:
    Timer();
    bool IsActive();
    void Start(const int* engineTicks);
    void Pause();
    void Unpause(const int* engineTicks);
    void Update(const int* engineTicks, const bool* engineIsPaused, const char* engineMap);
    void Stop();
    void AddRule(TimerRule rule);
    std::vector<TimerRule> GetRules();
    void CheckRules(const EventQueuePrioritizedEvent_t* event, const int* engineTicks);
    int GetSession();
    int GetTotal();
    char* GetCurrentMap();
    void SetIntervalPerTick(const float* ipt);
    float GetIntervalPerTick();
    TimerResult* GetResult();
    TimerResult* GetPersonalBest();
    bool ExportResult(std::string filePath, bool pb = false);
    bool ExportPersonalBest(std::string filePath);
    bool ImportPersonalBest(std::string filePath);
    int GetSplitDelta();
    int GetCurrentDelta();
    ~Timer();
    static std::string Format(float raw);
};

extern Timer* timer;
}
