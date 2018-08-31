#pragma once
#include <algorithm>

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Tier1.hpp"

#include "Utils.hpp"

namespace TAS {

struct TasFrame {
    int FramesLeft;
    std::string Command;
};

std::vector<TasFrame> Frames;
bool IsRunning = false;
int BaseIndex = 0;

void AddFrame(int framesLeft, std::string command, bool relative = false)
{
    if (relative) {
        framesLeft += BaseIndex;
    } else {
        BaseIndex = framesLeft;
    }

    Frames.push_back(TasFrame{
        framesLeft,
        command });
}
void AddFrames(int framesLeft, int interval, int lastFrame, std::string command, bool relative = false)
{
    if (relative) {
        framesLeft += BaseIndex;
        lastFrame += BaseIndex;
    } else {
        BaseIndex = framesLeft;
    }

    for (; framesLeft <= lastFrame; framesLeft += interval) {
        Frames.push_back(TasFrame{
            framesLeft,
            command });
    }
}
void Stop()
{
    IsRunning = false;
}
void Reset()
{
    Stop();
    BaseIndex = 0;
    Frames.clear();
}
void Start()
{
    if (Frames.size() != 0) {
        std::sort(Frames.begin(), Frames.end(), [](const auto& a, const auto& b) {
            return a.FramesLeft < b.FramesLeft;
        });
        IsRunning = true;
    }
}
}

namespace TAS2 {

struct TasFrame {
    QAngle viewangles;
    float forwardmove;
    float sidemove;
    float upmove;
    int buttons;
    unsigned char impulse;
    short mousedx;
    short mousedy;
};

std::vector<TasFrame> Frames;
bool IsRecording = false;
bool IsPlaying = false;
int PlayIndex;

void StartRecording()
{
    IsRecording = true;
    Frames.clear();
}
void StartReRecording()
{
    IsRecording = true;
    IsPlaying = false;
    Frames.resize(PlayIndex);
}
void StartPlaying()
{
    if (Frames.size() != 0) {
        IsPlaying = true;
        PlayIndex = 0;
    }
}
void Stop()
{
    IsRecording = false;
    IsPlaying = false;
}
void Record(CUserCmd* cmd)
{
    Frames.push_back(TasFrame{
        cmd->viewangles,
        cmd->forwardmove,
        cmd->sidemove,
        cmd->upmove,
        cmd->buttons,
        cmd->impulse,
        cmd->mousedx,
        cmd->mousedy });
}
void Play(CUserCmd* cmd)
{
    auto frame = Frames[PlayIndex];

    cmd->viewangles = frame.viewangles;
    cmd->forwardmove = frame.forwardmove;
    cmd->sidemove = frame.sidemove;
    cmd->upmove = frame.upmove;
    cmd->buttons = frame.buttons;
    cmd->impulse = frame.impulse;
    cmd->mousedx = frame.mousedx;
    cmd->mousedy = frame.mousedy;

    if (++PlayIndex >= (int)Frames.size()) {
        IsPlaying = false;
    }
}
}
