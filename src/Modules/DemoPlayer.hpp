#pragma once
#include "Console.hpp"
#include "Engine.hpp"

#include "Features/Demo/Demo.hpp"
#include "Features/Demo/DemoParser.hpp"

#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

namespace Engine {

using _GetGameDirectory = char*(__cdecl*)();
_GetGameDirectory GetGameDirectory;

namespace DemoPlayer {

    Interface* s_ClientDemoPlayer;

    using _IsPlayingBack = bool(__func*)(void* thisptr);
    using _GetPlaybackTick = int(__func*)(void* thisptr);

    _IsPlayingBack IsPlayingBack;
    _GetPlaybackTick GetPlaybackTick;

    char* DemoName;

    int GetTick()
    {
        return GetPlaybackTick(s_ClientDemoPlayer->ThisPtr());
    }
    bool IsPlaying()
    {
        return IsPlayingBack(s_ClientDemoPlayer->ThisPtr());
    }

    // CDemoRecorder::StartPlayback
    DETOUR(StartPlayback, const char* filename, bool bAsTimeDemo)
    {
        auto result = Original::StartPlayback(thisptr, filename, bAsTimeDemo);

        if (result) {
            DemoParser parser;
            Demo demo;
            auto dir = std::string(GetGameDirectory()) + std::string("/") + std::string(DemoName);
            if (parser.Parse(dir, &demo)) {
                parser.Adjust(&demo);
                console->Print("Client:   %s\n", demo.clientName);
                console->Print("Map:      %s\n", demo.mapName);
                console->Print("Ticks:    %i\n", demo.playbackTicks);
                console->Print("Time:     %.3f\n", demo.playbackTime);
                console->Print("Tickrate: %.3f\n", demo.Tickrate());
            } else {
                console->Print("Could not parse \"%s\"!\n", DemoName);
            }
        }
        return result;
    }

    void Init(void* demoplayer)
    {
        if (s_ClientDemoPlayer = Interface::Create(demoplayer)) {
            s_ClientDemoPlayer->Hook(Detour::StartPlayback, Original::StartPlayback, Offsets::StartPlayback);

            GetPlaybackTick = s_ClientDemoPlayer->Original<_GetPlaybackTick>(Offsets::GetPlaybackTick);
            IsPlayingBack = s_ClientDemoPlayer->Original<_IsPlayingBack>(Offsets::IsPlayingBack);
            DemoName = reinterpret_cast<char*>((uintptr_t)demoplayer + Offsets::m_szFileName);
        }
    }
    void Shutdown()
    {
        Interface::Delete(s_ClientDemoPlayer);
    }
}
}
