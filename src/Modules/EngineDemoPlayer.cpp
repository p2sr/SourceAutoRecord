#include "EngineDemoPlayer.hpp"

#include "Features/Demo/Demo.hpp"
#include "Features/Demo/DemoParser.hpp"
#include "Features/Camera.hpp"

#include "Console.hpp"
#include "Engine.hpp"

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

    return this->hasLoaded = this->s_ClientDemoPlayer;
}
void EngineDemoPlayer::Shutdown()
{
    Interface::Delete(this->s_ClientDemoPlayer);
}
