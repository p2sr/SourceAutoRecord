#include "CommandQueuer.hpp"

#include <algorithm>
#include <string>

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Command.hpp"
#include "Variable.hpp"

Variable sar_tas_autostart("sar_tas_autostart", "1", "Starts queued commands automatically on first frame after a load.\n");

CommandQueuer* tasQueuer;

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

CON_COMMAND(sar_tas_frame_at,
    "Adds command frame to the queue at specified frame.\n"
    "Usage: sar_tas_frame_at <frame> [command_to_execute]\n")
{
    if (args.ArgC() != 3) {
        console->Print("sar_tas_frame_at <frame> [command_to_execute] : Adds command frame to the queue.\n");
        return;
    }

    tasQueuer->AddFrame(atoi(args[1]), std::string(args[2]));
}
CON_COMMAND(sar_tas_frames_at,
    "Adds command frame multiple times to the queue at specified frame.\n"
    "Usage: sar_tas_frames_at <frame> <interval> <last_frame> [command_to_execute]\n")
{
    if (args.ArgC() != 5) {
        console->Print("sar_tas_frames_at <frame> <interval> <last_frame> [command_to_execute] : Adds command frame multiple times to the queue.\n");
        return;
    }

    tasQueuer->AddFrames(atoi(args[1]), atoi(args[2]), atoi(args[3]), std::string(args[4]));
}
CON_COMMAND(sar_tas_frame_after,
    "Adds command frame to the queue after waiting for specified amount of frames.\n"
    "Usage: sar_tas_frame_after <frames_to_wait> [command_to_execute]\n")
{
    if (args.ArgC() != 3) {
        console->Print("sar_tas_frame_after <frames_to_wait> [command_to_execute] : Adds command frame to the queue.\n");
        return;
    }

    tasQueuer->AddFrame(atoi(args[1]), std::string(args[2]), true);
}
CON_COMMAND(sar_tas_frames_after,
    "Adds command frame multiple times to the queue after waiting for specified amount of frames.\n"
    "Usage: sar_tas_frames_after <frames_to_wait> <interval> <length> [command_to_execute]\n")
{
    if (args.ArgC() != 5) {
        console->Print("sar_tas_frames_after <frames_to_wait> <interval> <length> [command_to_execute] : Adds command frame multiple times to the queue.\n");
        return;
    }

    tasQueuer->AddFrames(atoi(args[1]), atoi(args[2]), atoi(args[3]), std::string(args[4]), true);
}
CON_COMMAND(sar_tas_start, "Starts executing queued commands.\n")
{
    tasQueuer->Start();
}
CON_COMMAND(sar_tas_reset, "Stops executing commands and clears them from the queue.\n")
{
    tasQueuer->Reset();
}
