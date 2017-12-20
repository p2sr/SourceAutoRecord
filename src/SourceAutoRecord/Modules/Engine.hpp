#pragma once
#include <string>

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

using _GetGameDir = void(__cdecl*)(char* szGetGameDir, int maxlength);
using _ClientCmd = void(__fastcall*)(void* thisptr, const char* szCmdString);

using _SetSignonState = bool(__thiscall*)(void* thisptr, int state, int spawncount);
using _CloseDemoFile = int(__thiscall*)(void* thisptr);
using _StopRecording = int(__thiscall*)(void* thisptr);
using _StartupDemoFile = void(__thiscall*)(void* thisptr);
using _ConCommandStop = void(__cdecl*)();
using _Disconnect = void(__thiscall*)(void* thisptr, int bShowMainMenu);
using _PlayDemo = void(__cdecl*)(void* thisptr);
using _StartPlayback = bool(__fastcall*)(void* thisptr, int edx, const char *filename, bool bAsTimeDemo);
using _StopPlayback = int(__fastcall*)(void* thisptr, int edx);
using _HostStateFrame = void(__cdecl*)(float time);

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

// TODO
enum HostState {
	NewGame = 0,
	LoadGame = 1,
	ChangeLevelSp = 2,
	ChangeLevelMp = 3,
	Run = 4,
	GameShutdown = 5,
	Shutdown = 6,
	Restart = 7
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

// engine.dll
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
		ClientCmd(ClientPtr, cmd);
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

	namespace Original
	{
		_SetSignonState SetSignonState;
		_CloseDemoFile CloseDemoFile;
		_StopRecording StopRecording;
		_StartupDemoFile StartupDemoFile;
		_ConCommandStop ConCommandStop;
		_Disconnect Disconnect;
		_PlayDemo PlayDemo;
		_StartPlayback StartPlayback;
		_HostStateFrame HostStateFrame;
	}

	namespace Detour
	{
		bool PlayerRequestedStop;
		bool IsRecordingDemo;

		bool PlayerRequestedPlayback;
		bool IsPlayingDemo;

		int LastHostState;
		bool CallFromHostStateFrame;

		bool __fastcall SetSignonState(void* thisptr, int edx, int state, int spawncount)
		{
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
			return Original::SetSignonState(thisptr, state, spawncount);
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

			return Original::CloseDemoFile(thisptr);
		}
		bool __fastcall StopRecording(void* thisptr, int edx)
		{
			const int LastDemoNumber = *DemoRecorder::DemoNumber;

			// This function does:
			// m_bRecording = false
			// m_nDemoNumber = 0
			bool result = Original::StopRecording(thisptr);

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
			Original::StartupDemoFile(thisptr);
		}
		void __cdecl ConCommandStop()
		{
			PlayerRequestedStop = true;
			Original::ConCommandStop();
			PlayerRequestedStop = false;
		}
		void __fastcall Disconnect(void* thisptr, int edx, bool bShowMainMenu)
		{
			SessionEnded();
			Original::Disconnect(thisptr, bShowMainMenu);
		}
		void __cdecl PlayDemo(void* thisptr)
		{
			PlayerRequestedPlayback = true;
			Original::PlayDemo(thisptr);
			PlayerRequestedPlayback = false;
		}
		bool __fastcall StartPlayback(void* thisptr, int edx, const char *filename, bool bAsTimeDemo)
		{
			bool result = Original::StartPlayback(thisptr, edx, filename, bAsTimeDemo);

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
		void __fastcall HostStateFrame(void* thisptr, int edx, float time)
		{
			HostStateData state = *reinterpret_cast<HostStateData*>(CurrentStatePtr);

			if (state.m_currentState != LastHostState) {
				if (state.m_currentState == HostState::ChangeLevelSp) {
					SessionEnded();

					// CloseDemoFile gets called too late when changing the level
					// which causes to write invalid ticks into the demo header and redundant packets
					if (IsRecordingDemo) {
						CallFromHostStateFrame = true;
						StopRecording(DemoRecorder::Ptr, NULL);
						CallFromHostStateFrame = false;
					}
				}

				// Start new session when in menu
				if (state.m_currentState == HostState::Run && !state.m_activeGame && !DemoPlayer::IsPlaying()) {
					//Console::Print("Detected menu!\n");
					Session::Rebase(*Engine::TickCount);
				}

				LastHostState = state.m_currentState;
			}
			Original::HostStateFrame(time);
		}
	}
}