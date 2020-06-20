#include "EngineDemoPlayer.hpp"

#include "Features/Demo/Demo.hpp"
#include "Features/Demo/DemoParser.hpp"

#include "Console.hpp"
#include "Engine.hpp"
#include "Server.hpp"

#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

REDECL(EngineDemoPlayer::StartPlayback);

int EngineDemoPlayer::GetTick()
{
    return this->GetPlaybackTick(this->s_ClientDemoPlayer->ThisPtr());
}
bool EngineDemoPlayer::IsPlaying()
{
    return this->IsPlayingBack(this->s_ClientDemoPlayer->ThisPtr());
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
        } else {
            console->Print("Could not parse \"%s\"!\n", engine->demoplayer->DemoName);
        }
    }

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

    return this->hasLoaded = this->s_ClientDemoPlayer;
}
void EngineDemoPlayer::Shutdown()
{
    Interface::Delete(this->s_ClientDemoPlayer);
}

// Commands

CON_COMMAND_AUTOCOMPLETEFILE(sar_startdemos, "Improved version of startdemos. 'sar_startdemos <demoname>' Use 'sar_stopdemos' to stop playing demos.\n",
    0, 0, dem)
{
    // Always print a useful message for the user if not used correctly
    if (args.ArgC() <= 1) {
        return console->Print(sar_startdemos.ThisPtr()->m_pszHelpString);
    }

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

    engine->demoplayer->demoQueue.push(name);
    engine->demoplayer->demoQueueSize = 0;

    while (ok) {
        auto tmp_dir = dir + name + "_" + std::to_string(counter);
        ok = parser.Parse(tmp_dir, &demo);
        if (ok) {
            engine->demoplayer->demoQueue.push(name + "_" + std::to_string(counter));
            ++engine->demoplayer->demoQueueSize;
        }
        ++counter;
    }

    if (engine->demoplayer->demoQueueSize > 0) {
        engine->ExecuteCommand(std::string("playdemo " + engine->demoplayer->demoQueue.front()).c_str());
        engine->demoplayer->demoQueue.pop();
        --engine->demoplayer->demoQueueSize;
    }
}
CON_COMMAND(sar_stopdemos, "Stop demo queue.\n")
{
    engine->demoplayer->demoQueue = {};
    engine->demoplayer->demoQueueSize = 0;
    engine->ExecuteCommand("stopdemo");
}