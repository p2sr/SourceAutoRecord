#include "EngineDemoPlayer.hpp"

#include "Features/Demo/Demo.hpp"
#include "Features/Demo/DemoParser.hpp"
#include "Features/Camera.hpp"

#include "Console.hpp"
#include "Engine.hpp"
#include "Server.hpp"

#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

#include <filesystem>

REDECL(EngineDemoPlayer::StartPlayback);
REDECL(EngineDemoPlayer::stopdemo_callback);

int EngineDemoPlayer::GetTick()
{
    return this->GetPlaybackTick(this->s_ClientDemoPlayer->ThisPtr());
}
bool EngineDemoPlayer::IsPlaying()
{
    return this->IsPlayingBack(this->s_ClientDemoPlayer->ThisPtr());
}

void EngineDemoPlayer::ClearDemoQueue()
{
    engine->demoplayer->demoQueue.clear();
    engine->demoplayer->demoQueueSize = 0;
    engine->demoplayer->currentDemoID = -1;
}

std::string EngineDemoPlayer::GetLevelName()
{
    return this->levelName;
}

DETOUR_COMMAND(EngineDemoPlayer::stopdemo)
{
    engine->demoplayer->ClearDemoQueue();
    EngineDemoPlayer::stopdemo_callback(args);
}

// CDemoRecorder::StartPlayback
DETOUR(EngineDemoPlayer::StartPlayback, const char* filename, bool bAsTimeDemo)
{
    auto result = EngineDemoPlayer::StartPlayback(thisptr, filename, bAsTimeDemo);

    if (result) {
        DemoParser parser;
        Demo demo;
        auto dir = std::string(engine->GetGameDirectory()) + std::string("/")
            + std::string(engine->demoplayer->DemoName);
        if (parser.Parse(dir, &demo)) {
            parser.Adjust(&demo);
            console->Print("Client:   %s\n", demo.clientName);
            console->Print("Map:      %s\n", demo.mapName);
            console->Print("Ticks:    %i\n", demo.playbackTicks);
            console->Print("Time:     %.3f\n", demo.playbackTime);
            console->Print("Tickrate: %.3f\n", demo.Tickrate());
            engine->demoplayer->levelName = demo.mapName;
        } else {
            console->Print("Could not parse \"%s\"!\n", engine->demoplayer->DemoName);
        }
    }

    camera->RequestTimeOffsetRefresh();

    return result;
}

bool EngineDemoPlayer::Init()
{
    auto disconnect = engine->cl->Original(Offsets::Disconnect);
    auto demoplayer = Memory::DerefDeref<void*>(disconnect + Offsets::demoplayer);
    if (this->s_ClientDemoPlayer = Interface::Create(demoplayer)) {
        this->s_ClientDemoPlayer->Hook(EngineDemoPlayer::StartPlayback_Hook, EngineDemoPlayer::StartPlayback, Offsets::StartPlayback);

        this->GetPlaybackTick = s_ClientDemoPlayer->Original<_GetPlaybackTick>(Offsets::GetPlaybackTick);
        this->IsPlayingBack = s_ClientDemoPlayer->Original<_IsPlayingBack>(Offsets::IsPlayingBack);
        this->DemoName = reinterpret_cast<char*>((uintptr_t)demoplayer + Offsets::m_szFileName);
    }

    Command::Hook("stopdemo", EngineDemoPlayer::stopdemo_callback_hook, EngineDemoPlayer::stopdemo_callback);

    this->currentDemoID = -1;
    this->demoQueueSize = 0;

    return this->hasLoaded = this->s_ClientDemoPlayer;
}
void EngineDemoPlayer::Shutdown()
{
    Interface::Delete(this->s_ClientDemoPlayer);
    Command::Unhook("stopdemo", EngineDemoPlayer::stopdemo_callback);
}

// Commands

CON_COMMAND_AUTOCOMPLETEFILE(sar_startdemos, "Improved version of startdemos. 'sar_startdemos <demoname>' Use 'stopdemo' to stop playing demos.\n",
    0, 0, dem)
{
    // Always print a useful message for the user if not used correctly
    if (args.ArgC() <= 1) {
        return console->Print(sar_startdemos.ThisPtr()->m_pszHelpString);
    }

    engine->demoplayer->demoQueue.clear();

    std::string name = args[1];

    if (name.length() > 4) {
        if (name.substr(name.length() - 4, 4) == ".dem")
            name.resize(name.length() - 4);
    }

    auto dir = engine->GetGameDirectory() + std::string("/");
    int counter = 2;

    Demo demo;
    DemoParser parser;
    bool ok = parser.Parse(dir + name, &demo);

    if (!ok) {
        return console->Print("Could not parse \"%s\"!\n", engine->GetGameDirectory() + std::string("/") + args[1]);
    }

    engine->demoplayer->demoQueue.push_back(name);

    while (ok) {
        auto tmp_dir = dir + name + "_" + std::to_string(counter);
        ok = parser.Parse(tmp_dir, &demo);
        if (ok) {
            engine->demoplayer->demoQueue.push_back(name + "_" + std::to_string(counter));
        }
        ++counter;
    }

    engine->demoplayer->demoQueueSize = engine->demoplayer->demoQueue.size();
    engine->demoplayer->currentDemoID = 0;

    EngineDemoPlayer::stopdemo_callback(args);

    //Demos are played in Engine::Frame
}
CON_COMMAND(sar_startdemosfolder, "sar_startdemosfolder <folder name>. Plays all the demos in the specified folder by alphabetic order.\n")
{
    if (args.ArgC() < 2) {
        return console->Print(sar_startdemosfolder.ThisPtr()->m_pszHelpString);
    }

    engine->demoplayer->demoQueue.clear();

    auto dir = engine->GetGameDirectory() + std::string("/");
    std::string filepath;
    Demo demo;
    DemoParser parser;

    for (const auto& file : std::filesystem::directory_iterator(dir + args[1])) {
        if (file.path().extension() != ".dem")
            continue;

        filepath = args[1] + std::string("/") + file.path().filename().string();
        console->Print("%s\n", filepath.c_str());
        if (parser.Parse(dir + filepath, &demo)) {
            engine->demoplayer->demoQueue.push_back(filepath);
        }
    }

    std::sort(engine->demoplayer->demoQueue.begin(), engine->demoplayer->demoQueue.end());

    engine->demoplayer->demoQueueSize = engine->demoplayer->demoQueue.size();
    engine->demoplayer->currentDemoID = 0;

    EngineDemoPlayer::stopdemo_callback(args);
}
CON_COMMAND_COMPLETION(sar_skiptodemo, "sar_skiptodemo <demoname>. Skip demos in demo queue to this demo.\n", ({ engine->demoplayer->demoQueue }))
{
    if (args.ArgC() < 2) {
        return console->Print(sar_skiptodemo.ThisPtr()->m_pszHelpString);
    }

    auto it = std::find(engine->demoplayer->demoQueue.begin(), engine->demoplayer->demoQueue.end(), args[1]);
    if (it == engine->demoplayer->demoQueue.end())
        return;

    engine->demoplayer->currentDemoID =  std::distance(engine->demoplayer->demoQueue.begin(), it);

    EngineDemoPlayer::stopdemo_callback(args);
}
CON_COMMAND(sar_nextdemo, "Plays the next demo in demo queue.\n")
{
    if (++engine->demoplayer->currentDemoID >= engine->demoplayer->demoQueueSize) {
        return engine->demoplayer->ClearDemoQueue();
    }

    EngineDemoPlayer::stopdemo_callback(args);
}
