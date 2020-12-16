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
    std::string param;
};

struct TasPlayerInfo {
    int slot;
    int tick;
    Vector position;
    QAngle angles;
    Vector velocity;
    float surfaceFriction;
    float maxSpeed;
    bool ducked;
    bool grounded;
    int oldButtons;
};

class TasPlayer : public Feature {
private:
    bool active = false;
    bool ready = false;
    int startTick = 0; // used to store the absolute tick in which player started playing the script
    int currentTick = 0; // tick position of script player, relative to its starting point.
    int lastTick = 0; // last tick of script, relative to its starting point

    TasStartInfo startInfo;
    std::string tasFileName;

    std::vector<TasFramebulk> framebulkQueue;
    std::vector<TasFramebulk> processedFramebulks;
public:
    void Update();

    inline int GetTick() const { return currentTick; };
    inline int GetAbsoluteTick() const { return startTick+currentTick; };
    inline bool IsActive() const { return active; };

    void Activate();
    void Start();
    void PostStart();
    void Stop();

    TasFramebulk GetRawFramebulkAt(int tick);
    TasPlayerInfo GetPlayerInfo(void* player, CMoveData* pMove);
    void SetFrameBulkQueue(std::vector<TasFramebulk> fbQueue);
    void SetStartInfo(TasStartType type, std::string);
    inline void SetLoadedFileName(std::string name) { tasFileName = name;  };
    void SaveProcessedFramebulks();


    void FetchInputs(TasController* controller);
    void PostProcess(void* player, CMoveData* pMove);

    TasPlayer();
    ~TasPlayer();
};

extern Variable sar_tas_debug;
extern Variable sar_tas_tools_enabled;
extern Variable sar_tas_autosave_raw;

extern TasPlayer* tasPlayer;