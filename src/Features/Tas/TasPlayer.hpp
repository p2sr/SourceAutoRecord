#pragma once

#include "Features/Feature.hpp"
#include "Features/Tas/TasController.hpp"

#include "Utils/SDK.hpp"

#include "Command.hpp"
#include "Variable.hpp"

struct TasFramebulk {
    int tick = 0;
    Vector moveAnalog{ 0, 0 };
    Vector viewAnalog{ 0, 0 };
    bool buttonStates[TAS_CONTROLLER_INPUT_COUNT]{};
    std::vector<char*> commands;
    std::vector<char*> toolCmds;
};

enum TasStartType {
    UnknownStart,
    ChangeLevel,
    LoadQuicksave,
    StartImmediately,
    WaitForNewSession,
};

struct TasStartInfo {
    TasStartType type;
    char* param;
};

class TasPlayer : public Feature{
private:
    bool active = false;
    bool ready = false;
    int currentTick = 0;
    int lastTick = 0;

    TasStartInfo startInfo;

    std::vector<TasFramebulk> framebulkQueue;
    TasFramebulk currentFramebulkBuffer;
public:
    void Update();

    void Activate();
    void Start();
    void Stop();

    TasFramebulk GetCurrentRawFramebulk();
    TasFramebulk GetCurrentProcessedFramebulk();

    void FetchInputs(TasController* controller);

    TasPlayer();
    ~TasPlayer();
};

extern Variable sar_tas2_debug;

extern TasPlayer* tasPlayer;