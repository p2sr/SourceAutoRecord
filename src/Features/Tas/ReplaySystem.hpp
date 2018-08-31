#pragma once
#include "Features/Feature.hpp"

#include "Utils.hpp"

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
