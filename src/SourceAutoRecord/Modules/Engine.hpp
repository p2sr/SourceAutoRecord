#pragma once
#include <string>

#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "DemoRecorder.hpp"
#include "DemoPlayer.hpp"
#include "Vars.hpp"

#include "Features/Session.hpp"
#include "Features/Stats.hpp"
#include "Features/Summary.hpp"
#include "Features/Timer.hpp"

#include "Interfaces.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

using namespace Commands;

namespace Engine
{
	using _ClientCmd = int(__cdecl*)(void* thisptr, const char* szCmdString);
	using _Disconnect = int(__cdecl*)(void* thisptr, int bShowMainMenu);

	std::unique_ptr<VMTHook> engine;
	std::unique_ptr<VMTHook> cl;

	_ClientCmd ClientCmd;

	void ExecuteCommand(const char* cmd)
	{
		ClientCmd(engine->GetThisPtr(), cmd);
	}
	int GetTick()
	{
		int result = Vars::gpGlobals->tickcount - Session::BaseTick;
		return (result >= 0) ? result : 0;
	}
	std::string GetDir()
	{
		return std::string(Vars::GetGameDirectory());
	}

	namespace Original
	{
		_Disconnect Disconnect;
	}

	namespace Detour
	{
		int __cdecl Disconnect(void* thisptr, bool bShowMainMenu)
		{
			Console::PrintActive("Disconnect!\n");
			if (!*Vars::LoadGame && !DemoPlayer::IsPlaying()) {
				int tick = GetTick();

				if (tick != 0) {
					Console::Print("Session: %i (%.3f)\n", tick, tick * Vars::gpGlobals->interval_per_tick);
					Session::LastSession = tick;
				}

				if (Summary::IsRunning) {
					Summary::Add(tick, tick * Vars::gpGlobals->interval_per_tick, *Vars::Mapname);
					Console::Print("Total: %i (%.3f)\n", Summary::TotalTicks, Summary::TotalTime);
				}

				if (Timer::IsRunning) {
					if (sar_timer_always_running.GetBool()) {
						Timer::Save(Vars::gpGlobals->interval_per_tick);
						Console::Print("Timer paused: %i (%.3f)!\n", Timer::TotalTicks, Timer::TotalTicks * Vars::gpGlobals->interval_per_tick);
					}
					else {
						Timer::Stop(Vars::gpGlobals->interval_per_tick);
						Console::Print("Timer stopped!\n");
					}
				}

				if (sar_stats_auto_reset.GetInt() >= 1) {
					Stats::Reset();
				}

				DemoRecorder::CurrentDemo = "";
			}
			return Original::Disconnect(thisptr, bShowMainMenu);
		}
	}

	void Hook()
	{
		if (Interfaces::IVEngineClient) {
			engine = std::make_unique<VMTHook>(Interfaces::IVEngineClient);
			ClientCmd = engine->GetOriginalFunction<_ClientCmd>(Offsets::ClientCmd);
			Vars::GetGameDirectory = engine->GetOriginalFunction<Vars::_GetGameDirectory>(Offsets::GetGameDirectory);
		}
		
		auto cstate = SAR::Find("ClientState");
		if (cstate.Found) {
			typedef void*(*_GetClientState)();
			auto GetClientState = reinterpret_cast<_GetClientState>(cstate.Address);
			cl = std::make_unique<VMTHook>(GetClientState());
			cl->HookFunction((void*)Detour::Disconnect, Offsets::Disconnect);
			Original::Disconnect = cl->GetOriginalFunction<_Disconnect>(Offsets::Disconnect);
		}
	}
}