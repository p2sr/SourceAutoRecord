#pragma once
#include "Module.hpp"

#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

#include <map>

class EngineDemoPlayer : public Module {
public:
    Interface* s_ClientDemoPlayer = nullptr;

    using _IsPlayingBack = bool(__rescall*)(void* thisptr);
    using _GetPlaybackTick = int(__rescall*)(void* thisptr);

    _IsPlayingBack IsPlayingBack = nullptr;
    _GetPlaybackTick GetPlaybackTick = nullptr;

    char* DemoName = nullptr;
    int demoQueueSize = false;
    int currentDemoID = false;
    std::vector<std::string> demoQueue;
    std::string levelName;

public:
    int GetTick();
    bool IsPlaying();
    void ClearDemoQueue();
    std::string GetLevelName();
    void CustomDemoData(char* data, size_t length);

    // CDemoRecorder::StartPlayback
    DECL_DETOUR(StartPlayback, const char* filename, bool bAsTimeDemo);
    // CDemoRecorder::StopPlayback
    DECL_DETOUR(StopPlayback);
    DECL_DETOUR_COMMAND(stopdemo);

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("engine"); }

    bool ShouldBlacklistCommand(const char *cmd);
};

extern Command sar_startdemos;
extern Command sar_startdemosfolder;
extern Command sar_skiptodemo;
extern Command sar_nextdemo;
