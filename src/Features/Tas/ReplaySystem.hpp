#pragma once
#include "Features/Feature.hpp"

#include "Utils/SDK.hpp"
#include "Variable.hpp"

#define SAR_TAS_REPLAY_HEADER001 "sar-tas-replay v1.7"
#define SAR_TAS_REPLAY_HEADER002 "sar-tas-replay v1.8"
#define SAR_TAS_REPLAY_EXTENSION ".str"

struct ReplayFrame {
    QAngle viewangles;
    float forwardmove;
    float sidemove;
    float upmove;
    int buttons;
    unsigned char impulse;
    short mousedx;
    short mousedy;
};

struct ReplayView {
    std::vector<ReplayFrame> frames;
};

class Replay {
public:
    std::vector<ReplayView> views;

private:
    int playIndex;

public:
    Replay();
    bool Ended();
    void Reset();
    void Resize();
    void Record(CUserCmd* cmd, int slot);
    void Play(CUserCmd* cmd, int slot);
};

class ReplaySystem : public Feature {
private:
    std::vector<Replay*> replays;
    int replayIndex;
    bool isRecording;
    bool isPlaying;

public:
    ReplaySystem();
    void Record(bool rerecord = false);
    void Play();
    void Stop();
    bool IsRecording();
    bool IsPlaying();
    Replay* GetCurrentReplay();
    bool AnyReplaysLoaded();
    void DeleteAll();
    void MergeAll();

private:
    void NewReplay();
    void DeleteReplay();

public:
    void Export(std::string filePath, int index = 0);
    void Import(std::string filePath);
};

extern ReplaySystem* tasReplaySystem;

extern Variable sar_replay_autorecord;
extern Variable sar_replay_autoplay;
