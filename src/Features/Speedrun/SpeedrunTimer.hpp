#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Utils/SDK.hpp"

#include "Features/Feature.hpp"

#include "TimerAction.hpp"
#include "TimerInterface.hpp"
#include "TimerResult.hpp"
#include "TimerRule.hpp"
#include "TimerState.hpp"

#include "Command.hpp"
#include "Variable.hpp"

#define SAR_SPEEDRUN_EXPORT_HEADER "Map,Ticks,Time,Map Ticks,Map Time,Total Ticks,Total Time,Segment"

class SpeedrunTimer : public Feature {
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
    SpeedrunTimer();

    bool IsActive();

    void Start(const int* engineTicks);
    void Pause();
    void Unpause(const int* engineTicks);
    void Update(const int* engineTicks, const char* engineMap);
    void Stop(bool addSegment = true);

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

    ~SpeedrunTimer();

    static std::string Format(float raw);
};

extern SpeedrunTimer* speedrun;

extern Command sar_speedrun_start;
extern Command sar_speedrun_stop;
extern Command sar_speedrun_result;
extern Command sar_speedrun_export;
extern Command sar_speedrun_export_pb;
extern Command sar_speedrun_import;
extern Command sar_speedrun_rules;

extern Variable sar_speedrun_autostart;
extern Variable sar_speedrun_autostop;
