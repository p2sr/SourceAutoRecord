#include "ReplaySystem.hpp"

#include <fstream>

#include "Replay.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Command.hpp"
#include "Variable.hpp"

Variable sar_replay_autorecord("sar_replay_autorecord", "0", "Starts recording inputs on first frame after a load.\n");
Variable sar_replay_autoplay("sar_replay_autoplay", "0", "Starts playing back recorded inputs on first frame after a load.\n");

ReplaySystem* tasReplaySystem;

ReplaySystem::ReplaySystem()
    : replays()
    , isRecording(false)
    , isPlaying(false)
{
    this->hasLoaded = true;
}
void ReplaySystem::Record(bool rerecord)
{
    this->isRecording = true;
    if (rerecord && this->AnyReplaysLoaded()) {
        this->isPlaying = false;
        auto replay = this->GetCurrentReplay();
        replay->Resize();
    } else {
        this->DeleteAll();
        this->NewReplay();
    }
}
void ReplaySystem::Play()
{
    if (this->AnyReplaysLoaded()) {
        this->isPlaying = true;
        this->GetCurrentReplay()->Reset();
    }
}
void ReplaySystem::Stop()
{
    if (this->AnyReplaysLoaded()) {
        this->isRecording = false;
        this->isPlaying = false;
        this->GetCurrentReplay()->Reset();
    }
}
bool ReplaySystem::IsRecording()
{
    return this->isRecording;
}
bool ReplaySystem::IsPlaying()
{
    return this->isPlaying;
}
Replay* ReplaySystem::GetCurrentReplay()
{
    return this->replays.back();
}
bool ReplaySystem::AnyReplaysLoaded()
{
    return !this->replays.empty();
}
void ReplaySystem::DeleteAll()
{
    for (const auto& replay : this->replays) {
        delete replay;
    }
    this->replays.clear();
}
void ReplaySystem::MergeAll()
{
    if (this->replays.size() < 2) {
        return console->Print("Need at least two replays for a merge.\n");
    }

    auto viewSize = this->GetCurrentReplay()->views.size();
    auto baseReplay = new Replay();

    for (const auto& replay : this->replays) {
        if (viewSize != replay->views.size()) {
            console->Warning("Ignored different view size between replays!\n");
        }

        for (int viewIndex = 0; viewIndex < (int)viewSize; ++viewIndex) {
            baseReplay->views[viewIndex].frames.insert(
                baseReplay->views[viewIndex].frames.end(),
                replay->views[viewIndex].frames.begin(),
                replay->views[viewIndex].frames.end());
        }
    }

    this->DeleteAll();
    this->replays.push_back(baseReplay);
}
void ReplaySystem::NewReplay()
{
    this->replays.push_back(new Replay());
}
void ReplaySystem::DeleteReplay()
{
    auto last = this->GetCurrentReplay();
    delete last;
    this->replays.pop_back();
}
void ReplaySystem::Export(std::string filePath, int index)
{
    if (filePath.substr(filePath.length() - 4, 4) != SAR_TAS_REPLAY_EXTENSION)
        filePath += SAR_TAS_REPLAY_EXTENSION;

    std::ofstream file(filePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!file.good()) {
        console->Print("File not found.\n");
        return file.close();
    }

    if (!this->AnyReplaysLoaded()) {
        console->Print("Nothing to export!\n");
        return file.close();
    }

    if (index >= (int)this->replays.size()) {
        console->Print("Invalid replay index!\n");
        return file.close();
    }

    file << SAR_TAS_REPLAY_HEADER002 << std::endl;

    auto replay = this->replays[index];
    auto viewSize = (int)replay->views.size();
    file.write((char*)&viewSize, sizeof(viewSize));

    auto frameIndex = 0;
    auto frameSize = (int)replay->views[0].frames.size();
    while (frameIndex < frameSize) {
        for (int viewIndex = 0; viewIndex < viewSize; ++viewIndex) {
            auto frame = replay->views[viewIndex].frames[frameIndex];
            file.write((char*)&frame.buttons, sizeof(frame.buttons));
            file.write((char*)&frame.forwardmove, sizeof(frame.forwardmove));
            file.write((char*)&frame.impulse, sizeof(frame.impulse));
            file.write((char*)&frame.mousedx, sizeof(frame.mousedx));
            file.write((char*)&frame.mousedy, sizeof(frame.mousedy));
            file.write((char*)&frame.sidemove, sizeof(frame.sidemove));
            file.write((char*)&frame.upmove, sizeof(frame.upmove));
            file.write((char*)&frame.viewangles, sizeof(frame.viewangles));
        }
        ++frameIndex;
    }

    console->Print("Exported TAS replay!\n");

    file.close();
}
void ReplaySystem::Import(std::string filePath)
{
    if (filePath.substr(filePath.length() - 4, 4) != SAR_TAS_REPLAY_EXTENSION)
        filePath += SAR_TAS_REPLAY_EXTENSION;

    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.good()) {
        console->Print("File not found.\n");
        return file.close();
    }

    std::string buffer;
    std::getline(file, buffer);

    if (buffer == std::string(SAR_TAS_REPLAY_HEADER001)) {
        auto replay = new Replay();
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
            replay->views[0].frames.push_back(frame);
        }
        this->replays.push_back(replay);
    } else if (buffer == std::string(SAR_TAS_REPLAY_HEADER002)) {
        auto viewSize = 0;
        file.read((char*)&viewSize, sizeof(viewSize));

        if (viewSize != 1) {
            console->Print("Invalid view size.\n");
            return file.close();
        }

        auto replay = new Replay();
        while (!file.eof() && !file.bad()) {
            for (int viewIndex = 0; viewIndex < viewSize; ++viewIndex) {
                auto frame = ReplayFrame();
                file.read((char*)&frame.buttons, sizeof(frame.buttons));
                file.read((char*)&frame.forwardmove, sizeof(frame.forwardmove));
                file.read((char*)&frame.impulse, sizeof(frame.impulse));
                file.read((char*)&frame.mousedx, sizeof(frame.mousedx));
                file.read((char*)&frame.mousedy, sizeof(frame.mousedy));
                file.read((char*)&frame.sidemove, sizeof(frame.sidemove));
                file.read((char*)&frame.upmove, sizeof(frame.upmove));
                file.read((char*)&frame.viewangles, sizeof(frame.viewangles));
                replay->views[viewIndex].frames.push_back(frame);
            }
        }
        this->replays.push_back(replay);

    } else {
        console->Print("Invalid file format!\n");
        return file.close();
    }

    console->Print("Imported TAS replay!\n");
    file.close();
}

CON_COMMAND(sar_replay_record, "Starts recording user inputs.\n")
{
    if (tasReplaySystem->IsPlaying()) {
        console->Print("Playback active!\n");
    } else {
        tasReplaySystem->Record();
        console->Print("Recording!\n");
    }
}
CON_COMMAND(sar_replay_record_again, "Starts recording in play back mode.\n")
{
    if (!tasReplaySystem->IsPlaying()) {
        console->Print("Playback not active!\n");
    } else if (!tasReplaySystem->AnyReplaysLoaded()) {
        console->Print("Nothing to record again!\n");
    } else {
        tasReplaySystem->Record(true);
        console->Print("Re-Recording!\n");
    }
}
CON_COMMAND(sar_replay_play, "Plays back recorded user inputs.\n")
{
    if (tasReplaySystem->IsRecording()) {
        console->Print("Recording active!\n");
    } else if (!tasReplaySystem->AnyReplaysLoaded()) {
        console->Print("Nothing to play!\n");
    } else {
        tasReplaySystem->Play();
        console->Print("Playing!\n");
    }
}
CON_COMMAND(sar_replay_stop, "Stops recording or playing user inputs.\n")
{
    tasReplaySystem->Stop();
}
CON_COMMAND(sar_replay_merge, "Merges all replays into one.\n")
{
    tasReplaySystem->MergeAll();
}
CON_COMMAND(sar_replay_export, "Exports replay to a file.\n")
{
    if (args.ArgC() != 2) {
        console->Print("sar_replay_export <file> : Exports replay to a file.\n");
        return;
    }

    auto replay = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
    tasReplaySystem->Export(replay);
}
CON_COMMAND(sar_replay_export_at, "Exports specific replay to a file.\n")
{
    if (args.ArgC() != 3) {
        console->Print("sar_replay_export_at <index> <file> : Exports specific replay to a file.\n");
        return;
    }

    auto replay = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[2]);
    tasReplaySystem->Export(replay, std::atoi(args[1]));
}
CON_COMMAND_AUTOCOMPLETEFILE(sar_replay_import, "Imports replay file.", 0, 0, str)
{
    if (args.ArgC() != 2) {
        console->Print("sar_replay_import <file> : Imports replay file.\n");
        return;
    }

    auto replay = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
    tasReplaySystem->DeleteAll();
    tasReplaySystem->Import(replay);
}
CON_COMMAND_AUTOCOMPLETEFILE(sar_replay_import2, "Imports replay file but doesn't delete already added replays.", 0, 0, str)
{
    if (args.ArgC() != 2) {
        console->Print("sar_replay_import_append <file> : Imports replay file but doesn't delete already added replays.\n");
        return;
    }

    auto replay = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
    tasReplaySystem->Import(replay);
}
