#pragma once
#include <string>

#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "DemoRecorder.hpp"
#include "DemoPlayer.hpp"

#include "Demo.hpp"
#include "Rebinder.hpp"
#include "Session.hpp"
#include "Stats.hpp"
#include "Summary.hpp"
#include "Timer.hpp"
#include "Utils.hpp"

using _ClientCmd = void(__fastcall*)(void* thisptr, int edx, const char* szCmdString);
using _GetGameDir = void(__cdecl*)(char* szGetGameDir, int maxlength);

using _SetSignonState = bool(__cdecl*)(void* thisptr, int state, int spawncount);
using _CloseDemoFile = int(__cdecl*)(void* thisptr);
using _StopRecording = int(__cdecl*)(void* thisptr);
using _StartupDemoFile = void(__cdecl*)(void* thisptr);
using _ConCommandStop = void(__cdecl*)();
using _Disconnect = void(__cdecl*)(void* thisptr, int bShowMainMenu);
using _PlayDemo = void(__cdecl*)(void* thisptr);
using _StartPlayback = bool(__cdecl*)(void* thisptr, const char* filename, bool bAsTimeDemo);
using _StopPlayback = int(__fastcall*)(void* thisptr, int edx);
using _HostStateFrame = int(__cdecl*)(float time);

enum SignonState {
	None = 0,
	Challenge = 1,
	Connected = 2,
	New = 3,
	Prespawn = 4,
	Spawn = 5,
	Full = 6,
	Changelevel = 7
};

struct HostStateData {
	int	m_currentState;
	int	m_nextState;
	Vector m_vecLocation;
	QAngle m_angLocation;
	char m_levelName[256];
	char m_landmarkName[256];
	char m_saveName[256];
	float m_flShortFrameTime;
	bool m_activeGame;
	bool m_bRememberLocation;
	bool m_bBackgroundLevel;
	bool m_bWaitingForConnection;
};

using namespace Commands;

namespace Engine
{
	void* ClientPtr;
	_GetGameDir GetGameDir;
	_ClientCmd ClientCmd;

	int* TickCount;
	float* IntervalPerTick;
	bool* LoadGame;
	char** Mapname;

	void* CurrentStatePtr;

	void Set(uintptr_t clientPtr, uintptr_t gameDirAddr, uintptr_t curtimeAddr,
		uintptr_t loadGameAddr, uintptr_t mapnameAddr, uintptr_t currentStateAddr)
	{
		ClientPtr = **(void***)(clientPtr);
		ClientCmd = (_ClientCmd)GetVirtualFunctionByIndex(ClientPtr, Offsets::ClientCommand);
		GetGameDir = (_GetGameDir)gameDirAddr;

		TickCount = (int*)reinterpret_cast<uintptr_t*>(*((uintptr_t*)curtimeAddr) + Offsets::tickcount);
		IntervalPerTick = (float*)reinterpret_cast<uintptr_t*>(*((uintptr_t*)curtimeAddr) + Offsets::interval_per_tick);
		LoadGame = *(bool**)(loadGameAddr);
		Mapname = (char**)(mapnameAddr);

		CurrentStatePtr = (void*)reinterpret_cast<uintptr_t*>(*((uintptr_t*)currentStateAddr) - 4);
	}
	void ExecuteCommand(const char* cmd)
	{
		ClientCmd(ClientPtr, 0, cmd);
	}
	int GetTick()
	{
		int result = *TickCount - Session::BaseTick;
		return (result >= 0) ? result : 0;
	}
	std::string GetDir()
	{
		char dir[256];
		GetGameDir(dir, 256);
		return std::string(dir);
	}
	void SessionEnded()
	{
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
	}

	namespace Hooks
	{
		std::unique_ptr<VMTHook> SetSignonState;
		std::unique_ptr<VMTHook> CloseDemoFile;
		std::unique_ptr<VMTHook> StopRecording;
		std::unique_ptr<VMTHook> StartupDemoFile;
		std::unique_ptr<VMTHook> ConCommandStop;
		std::unique_ptr<VMTHook> Disconnect;
		std::unique_ptr<VMTHook> PlayDemo;
		std::unique_ptr<VMTHook> StartPlayback;
		std::unique_ptr<VMTHook> HostStateFrame;
	}

	namespace Detour
	{
		bool PlayerRequestedStop;
		bool IsRecordingDemo;

		bool PlayerRequestedPlayback;
		bool IsPlayingDemo;

		int LastHostState;
		bool CallFromHostStateFrame;

		/* bool __fastcall SetSignonState(void* thisptr, int edx, int state, int spawncount)
		{
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
			}
			return ((_SetSignonState)subhook_get_trampoline(Hooks::SetSignonState))(thisptr, state, spawncount);
		}
		int __fastcall CloseDemoFile(void* thisptr, int edx)
		{
			bool* m_bIsDemoHeader = (bool*)((uintptr_t)thisptr + Offsets::m_bIsDemoHeader);

			if (!*m_bIsDemoHeader && !*LoadGame && IsRecordingDemo) {
				int tick = DemoRecorder::GetTick();
				float time = tick * *IntervalPerTick;
				Console::Print("Demo: %i (%.3f)\n", tick, time);
				DemoRecorder::SetLastDemo(tick);
			}

			return ((_CloseDemoFile)subhook_get_trampoline(Hooks::CloseDemoFile))(thisptr);
		}
		bool __fastcall StopRecording(void* thisptr, int edx)
		{
			const int LastDemoNumber = *DemoRecorder::DemoNumber;

			// This function does:
			// m_bRecording = false
			// m_nDemoNumber = 0
			bool result = ((_StopRecording)subhook_get_trampoline(Hooks::StopRecording))(thisptr);

			if (IsRecordingDemo && !PlayerRequestedStop) {
				*DemoRecorder::DemoNumber = LastDemoNumber;

				// Tell recorder to keep recording
				if (*LoadGame || CallFromHostStateFrame) {
					*DemoRecorder::Recording = true;
					(*DemoRecorder::DemoNumber)++;
				}
			}
			else {
				IsRecordingDemo = false;
			}

			if (IsPlayingDemo && !PlayerRequestedPlayback) {
				IsPlayingDemo = false;
			}

			return result;
		}
		void __fastcall StartupDemoFile(void* thisptr, int edx)
		{
			IsRecordingDemo = true;
			DemoRecorder::SetCurrentDemo();
			((_StartupDemoFile)subhook_get_trampoline(Hooks::StartupDemoFile))(thisptr);
		}
		void __cdecl ConCommandStop()
		{
			PlayerRequestedStop = true;
			((_ConCommandStop)subhook_get_trampoline(Hooks::ConCommandStop))();
			PlayerRequestedStop = false;
		}
		void __fastcall Disconnect(void* thisptr, int edx, bool bShowMainMenu)
		{
			SessionEnded();
			((_Disconnect)subhook_get_trampoline(Hooks::Disconnect))(thisptr, bShowMainMenu);
		}
		void __cdecl PlayDemo(void* thisptr)
		{
			PlayerRequestedPlayback = true;
			((_PlayDemo)subhook_get_trampoline(Hooks::PlayDemo))(thisptr);
			PlayerRequestedPlayback = false;
		}
		bool __fastcall StartPlayback(void* thisptr, int edx, const char *filename, bool bAsTimeDemo)
		{
			bool result = ((_StartPlayback)subhook_get_trampoline(Hooks::StartPlayback))(thisptr, filename, bAsTimeDemo);

			if (result && PlayerRequestedPlayback) {
				IsPlayingDemo = true;

				// Allows sar_time_demo to parse last played demo
				DemoRecorder::LastDemo == "";

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
		int __fastcall HostStateFrame(void* thisptr, int edx, float time)
		{
			HostStateData state = *reinterpret_cast<HostStateData*>(CurrentStatePtr);

			if (state.m_currentState != LastHostState) {
				//Console::Print("m_currentState = %i\n", state.m_currentState);
				if (state.m_currentState == Offsets::HS_CHANGE_LEVEL_SP) {
					SessionEnded();

					// CloseDemoFile gets called too late when changing the level
					// which causes to write invalid ticks into the demo header and redundant packets
					if (IsRecordingDemo) {
						CallFromHostStateFrame = true;
						StopRecording(DemoRecorder::Ptr, 0);
						CallFromHostStateFrame = false;
					}
				}

				// Start new session when in menu
				if (state.m_currentState == Offsets::HS_RUN && !state.m_activeGame && !DemoPlayer::IsPlaying()) {
					//Console::Print("Detected menu!\n");
					Session::Rebase(*Engine::TickCount);
				}

				LastHostState = state.m_currentState;
			}
			return ((_HostStateFrame)subhook_get_trampoline(Hooks::HostStateFrame))(time);
		} */
	}
}