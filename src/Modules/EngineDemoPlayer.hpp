#pragma once
#include "Module.hpp"

#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

class EngineDemoPlayer : public Module {
public:
    Interface* s_ClientDemoPlayer;

    using _IsPlayingBack = bool(__func*)(void* thisptr);
    using _GetPlaybackTick = int(__func*)(void* thisptr);

    _IsPlayingBack IsPlayingBack;
    _GetPlaybackTick GetPlaybackTick;

    char* DemoName;

public:
    int GetTick();
    bool IsPlaying();

private:
    // CDemoRecorder::StartPlayback
    DECL_DETOUR(StartPlayback, const char* filename, bool bAsTimeDemo)

public:
    bool Init() override;
    void Shutdown() override;
};
