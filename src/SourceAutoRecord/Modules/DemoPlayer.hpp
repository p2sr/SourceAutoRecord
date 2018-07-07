#pragma once
#include "Console.hpp"
#include "Engine.hpp"

#include "Features/Demo.hpp"

#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

namespace Engine {

using _GetGameDirectory = char*(__cdecl*)();
_GetGameDirectory GetGameDirectory;

namespace DemoPlayer {

    VMT s_ClientDemoPlayer;

    using _IsPlayingBack = bool(__func*)(void* thisptr);
    using _GetPlaybackTick = int(__func*)(void* thisptr);

    _IsPlayingBack IsPlayingBack;
    _GetPlaybackTick GetPlaybackTick;

    char* DemoName;

    int GetTick()
    {
        return GetPlaybackTick(s_ClientDemoPlayer->GetThisPtr());
    }
    bool IsPlaying()
    {
        return IsPlayingBack(s_ClientDemoPlayer->GetThisPtr());
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
                Console::Print("Client: %s\n", demo.clientName);
                Console::Print("Map: %s\n", demo.mapName);
                Console::Print("Ticks: %i\n", demo.playbackTicks);
                Console::Print("Time: %.3f\n", demo.playbackTime);
                Console::Print("Tickrate: %.3f\n", demo.Tickrate());
            } else {
                Console::Print("Could not parse \"%s\"!\n", DemoName);
            }
        }
        return result;
    }

    void Hook(void* demoplayer)
    {
        CREATE_VMT(demoplayer, s_ClientDemoPlayer) {
            HOOK(s_ClientDemoPlayer, StartPlayback);

            GetPlaybackTick = s_ClientDemoPlayer->GetOriginalFunction<_GetPlaybackTick>(Offsets::GetPlaybackTick);
            IsPlayingBack = s_ClientDemoPlayer->GetOriginalFunction<_IsPlayingBack>(Offsets::IsPlayingBack);
            DemoName = reinterpret_cast<char*>((uintptr_t)demoplayer + Offsets::m_szFileName);
        }
    }
    void Unhook()
    {
        UNHOOK(s_ClientDemoPlayer, StartPlayback);
        DELETE_VMT(s_ClientDemoPlayer);
    }
}
}