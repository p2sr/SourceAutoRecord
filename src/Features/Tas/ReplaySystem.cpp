#include "ReplaySystem.hpp"

ReplaySystem::ReplaySystem()
    : frames()
    , isRecording(false)
    , isPlaying(false)
    , playIndex(0)
{
    this->hasLoaded = true;
}
void ReplaySystem::StartRecording()
{
    this->isRecording = true;
    this->frames.clear();
}
void ReplaySystem::StartReRecording()
{
    this->isRecording = true;
    this->isPlaying = false;
    this->frames.resize(this->playIndex);
}
void ReplaySystem::StartPlaying()
{
    if (this->frames.size() != 0) {
        this->isPlaying = true;
        this->playIndex = 0;
    }
}
void ReplaySystem::Stop()
{
    this->isRecording = false;
    this->isPlaying = false;
}
void ReplaySystem::Record(CUserCmd* cmd)
{
    this->frames.push_back(ReplayFrame{
        cmd->viewangles,
        cmd->forwardmove,
        cmd->sidemove,
        cmd->upmove,
        cmd->buttons,
        cmd->impulse,
        cmd->mousedx,
        cmd->mousedy });
}
void ReplaySystem::Play(CUserCmd* cmd)
{
    if (this->playIndex + 1 >= (int)this->frames.size()) {
        this->isPlaying = false;
    } else {
        auto frame = this->frames[this->playIndex++];

        cmd->viewangles = frame.viewangles;
        cmd->forwardmove = frame.forwardmove;
        cmd->sidemove = frame.sidemove;
        cmd->upmove = frame.upmove;
        cmd->buttons = frame.buttons;
        cmd->impulse = frame.impulse;
        cmd->mousedx = frame.mousedx;
        cmd->mousedy = frame.mousedy;
    }
}

ReplaySystem* tasReplaySystem;
