#include "CommandQueuer.hpp"

#include <algorithm>
#include <string>

CommandQueuer::CommandQueuer()
    : frames()
    , isRunning(false)
    , baseIndex(0)
{
    this->hasLoaded = true;
}
void CommandQueuer::AddFrame(int framesLeft, std::string command, bool relative)
{
    if (relative) {
        framesLeft += this->baseIndex;
    } else {
        this->baseIndex = framesLeft;
    }

    this->frames.push_back(CommandFrame{
        framesLeft,
        command });
}
void CommandQueuer::AddFrames(int framesLeft, int interval, int lastFrame, std::string command, bool relative)
{
    if (relative) {
        framesLeft += baseIndex;
        lastFrame += baseIndex;
    } else {
        baseIndex = framesLeft;
    }

    for (; framesLeft <= lastFrame; framesLeft += interval) {
        this->frames.push_back(CommandFrame{
            framesLeft,
            command });
    }
}
void CommandQueuer::Stop()
{
    this->isRunning = false;
}
void CommandQueuer::Reset()
{
    this->Stop();
    this->baseIndex = 0;
    this->frames.clear();
}
void CommandQueuer::Start()
{
    if (this->frames.size() != 0) {
        std::sort(this->frames.begin(), this->frames.end(), [](const auto& a, const auto& b) {
            return a.FramesLeft < b.FramesLeft;
        });
        this->isRunning = true;
    }
}

CommandQueuer* tasQueuer;
