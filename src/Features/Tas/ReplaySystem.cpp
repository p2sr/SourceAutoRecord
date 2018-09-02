#include "ReplaySystem.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Command.hpp"
#include "Variable.hpp"

Variable sar_replay_autorecord("sar_replay_autorecord", "0", "Starts recording inputs on first frame after a load.\n");
Variable sar_replay_autoplay("sar_replay_autoplay", "1", "Starts playing back recorded inputs on first frame after a load.\n");

ReplaySystem* tasReplaySystem;

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
    if (filePath.substr(filePath.length() - 4, 4) != SAR_TAS_REPLAY_EXTENSION)
        filePath += SAR_TAS_REPLAY_EXTENSION;

    std::ofstream file(filePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!file.good()) {
        console->Print("File not found.\n");
        return;
    }

    if (tasReplaySystem->frames.size() == 0) {
        console->Print("Nothing to export!\n");
        return;
    }

    file << SAR_TAS_REPLAY_HEADER001 << std::endl;

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
    if (filePath.substr(filePath.length() - 4, 4) != SAR_TAS_REPLAY_EXTENSION)
        filePath += SAR_TAS_REPLAY_EXTENSION;

    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.good()) {
        console->Print("File not found.\n");
        return;
    }

    std::string buffer;
    std::getline(file, buffer);

    if (buffer == std::string(SAR_TAS_REPLAY_HEADER001)) {
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
