#pragma once
#include <fstream>
#include <memory>
#include <string>
#include <sstream>
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
private:
    char* map;
    char* activator;
    TimerAction action;

public:
    TimerRule(char* map, char* activator, TimerAction action);
    void Check(char* map, char* activator, int engineTicks, Timer* timer);
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
    char start[16] = "SAR_TIMER_START"; // 0-15
    int total; // 16
    float ipt; // 20
    TimerAction action; // 24
    char end[14] = "SAR_TIMER_END"; // 28-41

public:
    TimerInterface(float ipt);
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
    char* map;
    float ipt;
    TimerState state;
    std::unique_ptr<TimerResult> result;
    std::unique_ptr<TimerResult> pb;
    std::unique_ptr<TimerRules> rules;

public:
    Timer();
    bool IsRunning();
    void Start(int engineTicks);
    void Unpause(int engineTicks);
    void Update(int engineTicks, char* engineMap, bool engineIsPaused);
    void Stop();
    void LoadRules(TimerRules rules);
    void CheckRules(char* map, char* activator, int engineTicks);
    int GetSession();
    int GetTotal();
    void SetIntervalPerTick(float ipt);
    float GetIntervalPerTick();
    TimerResult* GetResult();
    TimerResult* GetPersonalBest();
    bool ExportResult(std::string filePath, bool pb = false);
    bool ExportPersonalBest(std::string filePath);
    bool ImportPersonalBest(std::string filePath);
    int GetSplitDelta();
    int GetCurrentDelta();
    ~Timer();
    static std::string Timer::Format(float raw);
};

extern Timer* timer;
}