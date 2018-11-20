#pragma once
#include <vector>

#include "Utils/SDK.hpp"

#include "Variable.hpp"

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
    int ViewSize();
};
