#include "Replay.hpp"

Replay::Replay()
    : views(1)
    , playIndex(0)
{
}
void Replay::Reset()
{
    this->playIndex = 0;
}
bool Replay::Ended()
{
    return this->playIndex + 1 >= (int)this->views[0].frames.size();
}
void Replay::Resize()
{
    for (auto& view : this->views) {
        view.frames.resize(this->playIndex);
    }
}
void Replay::Record(CUserCmd* cmd, int slot)
{
    this->views[slot].frames.push_back(ReplayFrame{
        cmd->viewangles,
        cmd->forwardmove,
        cmd->sidemove,
        cmd->upmove,
        cmd->buttons,
        cmd->impulse,
        cmd->mousedx,
        cmd->mousedy });
}
void Replay::Play(CUserCmd* cmd, int slot)
{
    if (this->Ended())
        return;

    auto frame = this->views[slot].frames[this->playIndex++];

    cmd->viewangles = frame.viewangles;
    cmd->forwardmove = frame.forwardmove;
    cmd->sidemove = frame.sidemove;
    cmd->upmove = frame.upmove;
    cmd->buttons = frame.buttons;
    cmd->impulse = frame.impulse;
    cmd->mousedx = frame.mousedx;
    cmd->mousedy = frame.mousedy;
}
