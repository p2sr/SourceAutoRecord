#pragma once
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Tier1.hpp"

#include "Features/Tas.hpp"

#include "Command.hpp"

CON_COMMAND(sar_tas_frame_at,
    "Adds command frame to the queue at specified frame. "
    "Usage: sar_tas_frame_at <frame> [command_to_execute]\n")
{
    if (args.ArgC() != 3) {
        console->Print("sar_tas_frame_at <frame> [command_to_execute] : Adds command frame to the queue.\n");
        return;
    }

    TAS::AddFrame(atoi(args[1]), std::string(args[2]));
}

CON_COMMAND(sar_tas_frames_at,
    "Adds command frame multiple times to the queue at specified frame. "
    "Usage: sar_tas_frames_at <frame> <interval> <last_frame> [command_to_execute]\n")
{
    if (args.ArgC() != 5) {
        console->Print("sar_tas_frames_at <frame> <interval> <last_frame> [command_to_execute] : Adds command frame multiple times to the queue.\n");
        return;
    }

    TAS::AddFrames(atoi(args[1]), atoi(args[2]), atoi(args[3]), std::string(args[4]));
}

CON_COMMAND(sar_tas_frame_after,
    "Adds command frame to the queue after waiting for specified amount of frames. "
    "Usage: sar_tas_frame_after <frames_to_wait> [command_to_execute]\n")
{
    if (args.ArgC() != 3) {
        console->Print("sar_tas_frame_after <frames_to_wait> [command_to_execute] : Adds command frame to the queue.\n");
        return;
    }

    TAS::AddFrame(atoi(args[1]), std::string(args[2]), true);
}

CON_COMMAND(sar_tas_frames_after,
    "Adds command frame multiple times to the queue after waiting for specified amount of frames. "
    "Usage: sar_tas_frames_after <frames_to_wait> <interval> <length> [command_to_execute]\n")
{
    if (args.ArgC() != 5) {
        console->Print("sar_tas_frames_after <frames_to_wait> <interval> <length> [command_to_execute] : Adds command frame multiple times to the queue.\n");
        return;
    }

    TAS::AddFrames(atoi(args[1]), atoi(args[2]), atoi(args[3]), std::string(args[4]), true);
}

CON_COMMAND(sar_tas_start, "Starts executing queued commands.\n")
{
    TAS::Start();
}

CON_COMMAND(sar_tas_reset, "Stops executing commands and clears them from the queue.\n")
{
    TAS::Reset();
}

CON_COMMAND(sar_tas_record, "Starts recording user inputs.\n")
{
    if (TAS2::IsPlaying) {
        console->Print("Playback active!\n");
    } else {
        TAS2::StartRecording();
        console->Print("Recording!\n");
    }
}

CON_COMMAND(sar_tas_record_again, "Starts recording in play back mode.\n")
{
    if (!TAS2::IsPlaying) {
        console->Print("Playback not active!\n");
    } else if (TAS2::Frames.size() == 0) {
        console->Print("Nothing to record again!\n");
    } else {
        TAS2::StartReRecording();
        console->Print("Re-Recording!\n");
    }
}

CON_COMMAND(sar_tas_play, "Plays back recorded user inputs.\n")
{
    if (TAS2::IsRecording) {
        console->Print("Recording active!\n");
    } else if (TAS2::Frames.size() == 0) {
        console->Print("Nothing to play!\n");
    } else {
        TAS2::StartPlaying();
        console->Print("Recording!\n");
    }
}

CON_COMMAND(sar_tas_stop, "Stops recording or playing user inputs.\n")
{
    TAS2::Stop();
}

#define SAR_TAS_HEADER "sar-tas-replay v1.7"
#define SAR_TAS_EXTENSION ".str"

CON_COMMAND(sar_tas_export, "Export TAS replay to a file.\n")
{
    if (args.ArgC() != 2) {
        console->Print("sar_tas_export <file>: Export TAS replay to a file.\n");
        return;
    }

    auto filePath = std::string(Engine::GetGameDirectory()) + std::string("/") + std::string(args[1]);
    if (filePath.substr(filePath.length() - 4, 4) != SAR_TAS_EXTENSION)
        filePath += SAR_TAS_EXTENSION;

    std::ofstream file(filePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!file.good()) {
        console->Print("File not found.\n");
        return;
    }

    if (TAS2::Frames.size() == 0) {
        console->Print("Nothing to export!\n");
        return;
    }

    file << SAR_TAS_HEADER << std::endl;

    for (const auto& frame : TAS2::Frames) {
        file.write((char*)&frame.buttons, sizeof(frame.buttons));
        file.write((char*)&frame.forwardmove, sizeof(frame.forwardmove));
        file.write((char*)&frame.impulse, sizeof(frame.impulse));
        file.write((char*)&frame.mousedx, sizeof(frame.mousedx));
        file.write((char*)&frame.mousedy, sizeof(frame.mousedy));
        file.write((char*)&frame.sidemove, sizeof(frame.sidemove));
        file.write((char*)&frame.upmove, sizeof(frame.upmove));
        file.write((char*)&frame.viewangles, sizeof(frame.viewangles));
    }

    console->Print("Exported TAS replay!\n");

    file.close();
}

CON_COMMAND_AUTOCOMPLETEFILE(sar_tas_import, "Import TAS replay file.", 0, 0, str)
{
    if (args.ArgC() != 2) {
        console->Print("sar_tas_export <file>: Import TAS replay file.\n");
        return;
    }

    auto filePath = std::string(Engine::GetGameDirectory()) + std::string("/") + std::string(args[1]);
    if (filePath.substr(filePath.length() - 4, 4) != SAR_TAS_EXTENSION)
        filePath += SAR_TAS_EXTENSION;

    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.good()) {
        console->Print("File not found.\n");
        return;
    }

    std::string buffer;
    std::getline(file, buffer);

    if (buffer == std::string(SAR_TAS_HEADER)) {
        TAS2::Frames.clear();
        while (!file.eof() && !file.bad()) {
            auto frame = TAS2::TasFrame();
            file.read((char*)&frame.buttons, sizeof(frame.buttons));
            file.read((char*)&frame.forwardmove, sizeof(frame.forwardmove));
            file.read((char*)&frame.impulse, sizeof(frame.impulse));
            file.read((char*)&frame.mousedx, sizeof(frame.mousedx));
            file.read((char*)&frame.mousedy, sizeof(frame.mousedy));
            file.read((char*)&frame.sidemove, sizeof(frame.sidemove));
            file.read((char*)&frame.upmove, sizeof(frame.upmove));
            file.read((char*)&frame.viewangles, sizeof(frame.viewangles));
            TAS2::Frames.push_back(frame);
        }

        console->Print("Imported TAS replay!\n");
    } else {
        console->Print("Invalid file format!\n");
    }

    file.close();
}
