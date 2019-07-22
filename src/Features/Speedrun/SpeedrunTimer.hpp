#pragma once
#include <memory>
#include <string>
#include <vector>

#include "TimerAction.hpp"
#include "TimerCategory.hpp"
#include "TimerInterface.hpp"
#include "TimerResult.hpp"
#include "TimerRule.hpp"
#include "TimerState.hpp"

#include "Features/Feature.hpp"

#include "Utils/SDK.hpp"

#include "Command.hpp"
#include "Game.hpp"
#include "Variable.hpp"

#define SAR_SPEEDRUN_EXPORT_HEADER "Map,Ticks,Time,Map Ticks,Map Time,Total Ticks,Total Time,Segment"

class SpeedrunTimer : public Feature {
public:
    std::unique_ptr<TimerInterface> pubInterface;

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
    std::vector<TimerRule*> rules;
    TimerCategory* category;
    int offset;
    int pause;
public:
    std::vector<std::string> visitedMaps;

public:
    SpeedrunTimer();

    bool IsActive();

    void Start(const int engineTicks);
    void Pause();
    void Resume(const int engineTicks);
    void PreUpdate(const int engineTicks, const char* engineMap);
    void PostUpdate(const int engineTicks, const char* engineMap);
    void CheckRules(const int engineTicks);
    void Stop(bool addSegment = true);
    void Reset();
    void Split(bool visited);

    void IncrementPauseTime();

    int GetSession();
    int GetTotal();
    const char* GetCurrentMap();

    void SetIntervalPerTick(const float* ipt);
    const float GetIntervalPerTick();

    void SetCategory(TimerCategory* category);
    TimerCategory* GetCategory();

    void SetOffset(const int offset);
    const int GetOffset();

    void LoadRules(Game* game);
    void InitRules();
    void ReloadRules();
    void UnloadRules();
    const std::vector<TimerRule*>& GetRules();

    TimerResult* GetResult();
    TimerResult* GetPersonalBest();

    bool ExportResult(std::string filePath, bool pb = false);
    bool ExportPersonalBest(std::string filePath);
    bool ImportPersonalBest(std::string filePath);

    int GetSplitDelta();
    int GetCurrentDelta();

    void StatusReport(const char* message);

    ~SpeedrunTimer();

    static std::string Format(float raw);
};

extern SpeedrunTimer* speedrun;

extern Command sar_speedrun_start;
extern Command sar_speedrun_stop;
extern Command sar_speedrun_split;
extern Command sar_speedrun_pause;
extern Command sar_speedrun_resume;
extern Command sar_speedrun_reset;
extern Command sar_speedrun_result;
extern Command sar_speedrun_export;
extern Command sar_speedrun_export_pb;
extern Command sar_speedrun_import;
extern Command sar_speedrun_category;
extern Command sar_speedrun_offset;

extern Variable sar_speedrun_autostart;
extern Variable sar_speedrun_autostop;
extern Variable sar_speedrun_standard;
extern Variable sar_speedrun_time_pauses;
extern Variable sar_speedrun_smartsplit;
