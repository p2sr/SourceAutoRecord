#pragma once

#include "Features/Feature.hpp"
#include "Features/Tas/TasController.hpp"
#include "Features/Tas/TasTool.hpp"

#include "Utils/SDK.hpp"

#include "Command.hpp"
#include "Variable.hpp"

class TasToolCommand;

struct TasFramebulk {
    int tick = 0;
    Vector moveAnalog = { 0, 0 };
    Vector viewAnalog = { 0, 0 };
    bool buttonStates[TAS_CONTROLLER_INPUT_COUNT] = { 0 };
    std::vector<std::string> commands;
    std::vector<TasToolCommand> toolCmds;

    std::string ToString();
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

class TasPlayer : public Feature {
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

    int GetTick();

    void Activate();
    void Start();
    void Stop();

    TasFramebulk GetCurrentRawFramebulk();
    void ProcessFramebulk(TasFramebulk& fb);
    void SetFrameBulkQueue(std::vector<TasFramebulk> fbQueue);

    void FetchInputs(TasController* controller);

    TasPlayer();
    ~TasPlayer();
};

extern Variable sar_tas2_debug;

extern TasPlayer* tasPlayer;