#pragma once
#include "Module.hpp"

#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

class EngineDemoPlayer : public Module {
public:
    Interface* s_ClientDemoPlayer = nullptr;

    using _IsPlayingBack = bool(__func*)(void* thisptr);
    using _GetPlaybackTick = int(__func*)(void* thisptr);

    _IsPlayingBack IsPlayingBack = nullptr;
    _GetPlaybackTick GetPlaybackTick = nullptr;

    char* DemoName = nullptr;

public:
    int GetTick();
    bool IsPlaying();

    // CDemoRecorder::StartPlayback
    DECL_DETOUR(StartPlayback, const char* filename, bool bAsTimeDemo);

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("engine"); }
};
