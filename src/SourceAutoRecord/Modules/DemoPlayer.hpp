#pragma once
#include "vmthook/vmthook.h"

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

    using _IsPlayingBack = bool(__cdecl*)(void* thisptr);
    using _GetPlaybackTick = int(__cdecl*)(void* thisptr);
    using _StartPlayback = int(__cdecl*)(void* thisptr, const char* filename, bool bAsTimeDemo);

    std::unique_ptr<VMTHook> s_ClientDemoPlayer;

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

    namespace Original {
        _StartPlayback StartPlayback;
    }

    namespace Detour {
        int __cdecl StartPlayback(void* thisptr, const char* filename, bool bAsTimeDemo)
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
                    Console::Print("Tickrate: %i\n", demo.Tickrate());
                } else {
                    Console::Print("Could not parse \"%s\"!\n", DemoName);
                }
            }
            return result;
        }
    }

    void Hook(void* demoplayer)
    {
        if (demoplayer) {
            s_ClientDemoPlayer = std::make_unique<VMTHook>(demoplayer);
            s_ClientDemoPlayer->HookFunction((void*)Detour::StartPlayback, Offsets::StartPlayback);
            Original::StartPlayback = s_ClientDemoPlayer->GetOriginalFunction<_StartPlayback>(Offsets::StartPlayback);

            GetPlaybackTick = s_ClientDemoPlayer->GetOriginalFunction<_GetPlaybackTick>(Offsets::GetPlaybackTick);
            IsPlayingBack = s_ClientDemoPlayer->GetOriginalFunction<_IsPlayingBack>(Offsets::IsPlayingBack);
            DemoName = reinterpret_cast<char*>((uintptr_t)demoplayer + Offsets::m_szFileName);
        }
    }
}
}