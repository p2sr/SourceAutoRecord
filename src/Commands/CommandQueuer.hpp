#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Tier1.hpp"

#include "Features/Tas/CommandQueuer.hpp"

#include "Command.hpp"

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
