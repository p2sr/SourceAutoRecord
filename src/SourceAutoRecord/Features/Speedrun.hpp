#pragma once
#include <cmath>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

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
    const char* activator;
    TimerAction action;

public:
    TimerRule(const char* map, const char* activator, TimerAction action);
    void Check(Timer* timer, const char* activator, const int* engineTicks);
};

typedef std::vector<TimerRule> TimerRules;

class TimerSegment {
public:
    int session;
};

class TimerSplit {
public:
    int entered;
    char* map;
    std::vector<TimerSegment> segments;

    int GetTotal();
};

class TimerResult {
public:
    int total;
    TimerSplit curSplit;
    TimerSplit prevSplit;
    std::vector<TimerSplit> splits;

    void AddSplit(int tick, char* map);
    void AddSegment(int tick);
    void UpdateSplit(char* map);
    void Reset();
};

class Timer;

class TimerInterface {
public:
    char start[16]; // 0-15
    int total; // 16
    float ipt; // 20
    TimerAction action; // 24
    char end[14]; // 28-41

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
    std::unique_ptr<TimerRules> rules;

public:
    Timer();
    bool IsRunning();
    void Start(const int* engineTicks);
    void Unpause(const int* engineTicks);
    void Update(const int* engineTicks, const bool* engineIsPaused, const char* engineMap);
    void Stop();
    void LoadRules(TimerRules rules);
    TimerRules* Timer::GetRules();
    void CheckRules(const char* activator, const int* engineTicks);
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
