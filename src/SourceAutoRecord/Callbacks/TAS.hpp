#pragma once
#include "Modules/Console.hpp"
#include "Modules/Tier1.hpp"

#include "Features/TAS.hpp"

namespace Callbacks {

void AddTasFrameAt(const CCommand& args)
{
    if (args.ArgC() != 3) {
        Console::Print("sar_tas_frame_at <frame> [command_to_execute] : Adds command frame to the queue.\n");
        return;
    }

    TAS::AddFrame(atoi(args[1]), std::string(args[2]));
}
void AddTasFramesAt(const CCommand& args)
{
    if (args.ArgC() != 5) {
        Console::Print("sar_tas_frames_at <frame> <interval> <last_frame> [command_to_execute] : Adds command frame multiple times to the queue.\n");
        return;
    }

    TAS::AddFrames(atoi(args[1]), atoi(args[2]), atoi(args[3]), std::string(args[4]));
}
void AddTasFrameAfter(const CCommand& args)
{
    if (args.ArgC() != 3) {
        Console::Print("sar_tas_frame_after <frames_to_wait> [command_to_execute] : Adds command frame to the queue.\n");
        return;
    }

    TAS::AddFrame(atoi(args[1]), std::string(args[2]), true);
}
void AddTasFramesAfter(const CCommand& args)
{
    if (args.ArgC() != 5) {
        Console::Print("sar_tas_frames_after <frames_to_wait> <interval> <length> [command_to_execute] : Adds command frame multiple times to the queue.\n");
        return;
    }

    TAS::AddFrames(atoi(args[1]), atoi(args[2]), atoi(args[3]), std::string(args[4]), true);
}
void StartTas()
{
    TAS::Start();
}
void ResetTas()
{
    TAS::Reset();
}
}