#pragma once
#include "Module.hpp"

#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

#include <queue>

class EngineDemoPlayer : public Module {
public:
    Interface* s_ClientDemoPlayer = nullptr;

    using _IsPlayingBack = bool(__rescall*)(void* thisptr);
    using _GetPlaybackTick = int(__rescall*)(void* thisptr);

    _IsPlayingBack IsPlayingBack = nullptr;
    _GetPlaybackTick GetPlaybackTick = nullptr;

    char* DemoName = nullptr;
    int demoQueueSize = false;
    std::queue<std::string> demoQueue;

public:
    int GetTick();
    bool IsPlaying();

    // CDemoRecorder::StartPlayback
    DECL_DETOUR(StartPlayback, const char* filename, bool bAsTimeDemo);

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("engine"); }
};

extern Command sar_startdemos;
extern Command sar_stopdemos;