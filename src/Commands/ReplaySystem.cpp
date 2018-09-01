#include "ReplaySystem.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Features/Tas/ReplaySystem.hpp"

#include "Command.hpp"

CON_COMMAND(sar_replay_record, "Starts recording user inputs.\n")
{
    if (tasReplaySystem->isPlaying) {
        console->Print("Playback active!\n");
    } else {
        tasReplaySystem->StartRecording();
        console->Print("Recording!\n");
    }
}

CON_COMMAND(sar_replay_record_again, "Starts recording in play back mode.\n")
{
    if (!tasReplaySystem->isPlaying) {
        console->Print("Playback not active!\n");
    } else if (tasReplaySystem->frames.size() == 0) {
        console->Print("Nothing to record again!\n");
    } else {
        tasReplaySystem->StartReRecording();
        console->Print("Re-Recording!\n");
    }
}

CON_COMMAND(sar_replay_play, "Plays back recorded user inputs.\n")
{
    if (tasReplaySystem->isRecording) {
        console->Print("Recording active!\n");
    } else if (tasReplaySystem->frames.size() == 0) {
        console->Print("Nothing to play!\n");
    } else {
        tasReplaySystem->StartPlaying();
        console->Print("Recording!\n");
    }
}

CON_COMMAND(sar_replay_stop, "Stops recording or playing user inputs.\n")
{
    tasReplaySystem->Stop();
}

CON_COMMAND(sar_replay_export, "Export TAS replay to a file.\n")
{
    if (args.ArgC() != 2) {
        console->Print("sar_replay_export <file>: Export TAS replay to a file.\n");
        return;
    }

    auto filePath = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
    if (filePath.substr(filePath.length() - 4, 4) != SAR_TAS_EXTENSION)
        filePath += SAR_TAS_EXTENSION;

    std::ofstream file(filePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!file.good()) {
        console->Print("File not found.\n");
        return;
    }

    if (tasReplaySystem->frames.size() == 0) {
        console->Print("Nothing to export!\n");
        return;
    }

    file << SAR_TAS_HEADER001 << std::endl;

    for (const auto& frame : tasReplaySystem->frames) {
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

CON_COMMAND_AUTOCOMPLETEFILE(sar_replay_import, "Import TAS replay file.", 0, 0, str)
{
    if (args.ArgC() != 2) {
        console->Print("sar_replay_import <file>: Import TAS replay file.\n");
        return;
    }

    auto filePath = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
    if (filePath.substr(filePath.length() - 4, 4) != SAR_TAS_EXTENSION)
        filePath += SAR_TAS_EXTENSION;

    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.good()) {
        console->Print("File not found.\n");
        return;
    }

    std::string buffer;
    std::getline(file, buffer);

    if (buffer == std::string(SAR_TAS_HEADER001)) {
        tasReplaySystem->frames.clear();
        while (!file.eof() && !file.bad()) {
            auto frame = ReplayFrame();
            file.read((char*)&frame.buttons, sizeof(frame.buttons));
            file.read((char*)&frame.forwardmove, sizeof(frame.forwardmove));
            file.read((char*)&frame.impulse, sizeof(frame.impulse));
            file.read((char*)&frame.mousedx, sizeof(frame.mousedx));
            file.read((char*)&frame.mousedy, sizeof(frame.mousedy));
            file.read((char*)&frame.sidemove, sizeof(frame.sidemove));
            file.read((char*)&frame.upmove, sizeof(frame.upmove));
            file.read((char*)&frame.viewangles, sizeof(frame.viewangles));
            tasReplaySystem->frames.push_back(frame);
        }

        console->Print("Imported TAS replay!\n");
    } else {
        console->Print("Invalid file format!\n");
    }

    file.close();
}
