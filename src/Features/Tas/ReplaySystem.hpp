#pragma once
#include "Features/Feature.hpp"

#include "Utils/SDK.hpp"
#include "Variable.hpp"

#define SAR_TAS_REPLAY_HEADER001 "sar-tas-replay v1.7"
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

class ReplaySystem : public Feature {
public:
    std::vector<ReplayFrame> frames;
    bool isRecording;
    bool isPlaying;
    int playIndex;

public:
    ReplaySystem();
    void StartRecording();
    void StartReRecording();
    void StartPlaying();
    void Stop();
    void Record(CUserCmd* cmd);
    void Play(CUserCmd* cmd);
};

extern ReplaySystem* tasReplaySystem;

extern Variable sar_replay_autorecord;
extern Variable sar_replay_autoplay;
