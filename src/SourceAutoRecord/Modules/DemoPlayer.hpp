#pragma once
#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "Vars.hpp"

#include "Features/Demo.hpp"

#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

using _IsPlayingBack = bool(__cdecl*)(void* thisptr);
using _GetPlaybackTick = int(__cdecl*)(void* thisptr);

using _StartPlayback = int(__cdecl*)(void* thisptr, const char* filename, bool bAsTimeDemo);

namespace DemoPlayer
{
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

	namespace Original
	{
		_StartPlayback StartPlayback;
	}

	namespace Detour
	{
		int __cdecl StartPlayback(void* thisptr, const char *filename, bool bAsTimeDemo)
		{
			Console::PrintActive("StartPlayback!\n");
			int result = Original::StartPlayback(thisptr, filename, bAsTimeDemo);

			if (result) {
				Demo demo;
				if (demo.Parse(Vars::GetGameDirectory() + std::string("\\") + std::string(DemoPlayer::DemoName))) {
					demo.Fix();
					Console::Print("Client: %s\n", demo.clientName);
					Console::Print("Map: %s\n", demo.mapName);
					Console::Print("Ticks: %i\n", demo.playbackTicks);
					Console::Print("Time: %.3f\n", demo.playbackTime);
					Console::Print("IpT: %.6f\n", demo.IntervalPerTick());
				}
				else {
					Console::Print("Could not parse \"%s\"!\n", DemoPlayer::DemoName);
				}
			}
			return result;
		}
	}

	void Hook()
	{
		auto player = SAR::Find("demoplayer");
		if (player.Found) {
			auto ptr = **(void***)player.Address;
			s_ClientDemoPlayer = std::make_unique<VMTHook>(ptr);
			s_ClientDemoPlayer->HookFunction((void*)Detour::StartPlayback, Offsets::StartPlayback);
			Original::StartPlayback = s_ClientDemoPlayer->GetOriginalFunction<_StartPlayback>(Offsets::StartPlayback);

			DemoPlayer::GetPlaybackTick = s_ClientDemoPlayer->GetOriginalFunction<_GetPlaybackTick>(Offsets::GetPlaybackTick);
			DemoPlayer::IsPlayingBack = s_ClientDemoPlayer->GetOriginalFunction<_IsPlayingBack>(Offsets::IsPlayingBack);
			DemoPlayer::DemoName = reinterpret_cast<char*>((uintptr_t)ptr + Offsets::m_szFileName);
		}
	}
}