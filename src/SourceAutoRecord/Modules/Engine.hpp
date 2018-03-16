#pragma once
#include <string>

#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "DemoRecorder.hpp"
#include "DemoPlayer.hpp"

#include "Features/Demo.hpp"
#include "Features/Rebinder.hpp"
#include "Features/Session.hpp"
#include "Features/Stats.hpp"
#include "Features/Summary.hpp"
#include "Features/Timer.hpp"

#include "Interfaces.hpp"
#include "Utils.hpp"

using _ClientCmd = int(__cdecl*)(void* thisptr, const char* szCmdString);
using _GetGameDirectory = char*(__cdecl*)();

using _SetSignonState = void(__cdecl*)(void* thisptr, int state);
using _StopRecording = int(__cdecl*)(void* thisptr);
using _StartPlayback = int(__cdecl*)(void* thisptr, const char* filename, bool bAsTimeDemo);
using _Disconnect = int(__cdecl*)(void* thisptr, int bShowMainMenu);

using namespace Commands;

namespace Engine
{
	std::unique_ptr<VMTHook> s_ClientDemoRecorder;
	std::unique_ptr<VMTHook> s_ClientDemoPlayer;
	std::unique_ptr<VMTHook> engine;
	std::unique_ptr<VMTHook> cl;

	CGlobalVarsBase* gpGlobals;
	
	int* TickCount = &gpGlobals->tickcount;
	float* IntervalPerTick = &gpGlobals->interval_per_tick;

	void ExecuteCommand(const char* cmd)
	{
		engine->GetOriginalFunction<_ClientCmd>(Offsets::ClientCmd)(Interfaces::IVEngineClient, cmd);
	}
	int GetTick()
	{
		int result = gpGlobals->tickcount - Session::BaseTick;
		return (result >= 0) ? result : 0;
	}
	std::string GetDir()
	{
		auto dir = engine->GetOriginalFunction<_GetGameDirectory>(Offsets::GetGameDirectory)();
		return std::string(dir);
	}

	bool* LoadGame;
	char** Mapname;

	void Set(uintptr_t loadGameAddr, uintptr_t mapnameAddr)
	{
		LoadGame = *(bool**)(loadGameAddr);
		Mapname = (char**)(mapnameAddr);
	}

	bool IsRecordingDemo;
	bool IsPlayingDemo;

	void __cdecl SetSignonState(void* thisptr, int state)
	{
		Console::PrintActive("SetSignonState!\n");
		//Console::Print("SetSignonState = %i\n", state);
		if (state == SignonState::Prespawn) {
			if (Rebinder::IsSaveBinding || Rebinder::IsReloadBinding) {
				Rebinder::LastIndexNumber = (IsRecordingDemo)
					? *DemoRecorder::DemoNumber
					: Rebinder::LastIndexNumber + 1;

				Rebinder::RebindSave();
				Rebinder::RebindReload();
			}
		}
		// Demo recorder starts syncing from this tick
		else if (state == SignonState::Full) {
			Session::Rebase(*Engine::TickCount);
			Timer::Rebase(*Engine::TickCount);

			if (*DemoRecorder::Recording) {
				IsRecordingDemo = true;
				DemoRecorder::SetCurrentDemo();
			}
		}
		s_ClientDemoRecorder->GetOriginalFunction<_SetSignonState>(Offsets::SetSignonState)(thisptr, state);
	}
	int __cdecl StopRecording(void* thisptr)
	{
		Console::PrintActive("StopRecording!\n");
		const int LastDemoNumber = *DemoRecorder::DemoNumber;

		// This function does:
		// m_bRecording = false
		// m_nDemoNumber = 0
		int result = s_ClientDemoRecorder->GetOriginalFunction<_StopRecording>(Offsets::StopRecording)(thisptr);

		if (IsRecordingDemo && sar_autorecord.GetBool()) {
			*DemoRecorder::DemoNumber = LastDemoNumber;

			// Tell recorder to keep recording
			if (*LoadGame) {
				*DemoRecorder::Recording = true;
				(*DemoRecorder::DemoNumber)++;
			}
		}
		else {
			IsRecordingDemo = false;
		}

		if (IsPlayingDemo) {
			IsPlayingDemo = false;
		}

		return result;
	}
	int __cdecl StartPlayback(void* thisptr, const char *filename, bool bAsTimeDemo)
	{
		Console::PrintActive("StartPlayback!\n");
		int result = s_ClientDemoPlayer->GetOriginalFunction<_StartPlayback>(Offsets::StartPlayback)(thisptr, filename, bAsTimeDemo);

		if (result) {
			IsPlayingDemo = true;

			Demo demo;
			if (demo.Parse(GetDir() + std::string("\\") + std::string(DemoPlayer::DemoName))) {
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
	int __cdecl Disconnect(void* thisptr, bool bShowMainMenu)
	{
		Console::PrintActive("Disconnect!\n");
		if (!*LoadGame && !DemoPlayer::IsPlaying()) {
			int tick = GetTick();

			if (tick != 0) {
				Console::Print("Session: %i (%.3f)\n", tick, tick * *IntervalPerTick);
				Session::LastSession = tick;
			}

			if (Summary::IsRunning) {
				Summary::Add(tick, tick * *IntervalPerTick, *Mapname);
				Console::Print("Total: %i (%.3f)\n", Summary::TotalTicks, Summary::TotalTime);
			}

			if (Timer::IsRunning) {
				if (sar_timer_always_running.GetBool()) {
					Timer::Save(*Engine::TickCount);
					Console::Print("Timer paused: %i (%.3f)!\n", Timer::TotalTicks, Timer::TotalTicks * *IntervalPerTick);
				}
				else {
					Timer::Stop(*Engine::TickCount);
					Console::Print("Timer stopped!\n");
				}
			}

			if (sar_stats_auto_reset.GetInt() >= 1) {
				Stats::Reset();
			}

			DemoRecorder::CurrentDemo = "";
		}
		return cl->GetOriginalFunction<_Disconnect>(Offsets::Disconnect)(thisptr, bShowMainMenu);
	}
}