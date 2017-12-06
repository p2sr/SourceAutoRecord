#pragma once
#include <string>
#include "Console.hpp"
#include "DemoRecorder.hpp"
#include "DemoPlayer.hpp"
#include "../Demo.hpp"
#include "../Rebinder.hpp"
#include "../Summary.hpp"

using _GetGameDir = void(__cdecl*)(char* szGetGameDir, int maxlength);
using _ClientCmd = void(__fastcall*)(void* thisptr, const char* szCmdString);

using _SetSignonState = bool(__thiscall*)(void* thisptr, int state, int spawncount);
using _CloseDemoFile = bool(__thiscall*)(void* thisptr);
using _StopRecording = int(__thiscall*)(void* thisptr);
using _StartupDemoFile = void(__thiscall*)(void* thisptr);
using _ConCommandStop = void(__cdecl*)();
using _Disconnect = void(__thiscall*)(void* thisptr, int bShowMainMenu);
using _PlayDemo = void(__cdecl*)(void* thisptr);
using _StartPlayback = bool(__fastcall*)(void* thisptr, int edx, const char *filename, bool bAsTimeDemo);
using _StopPlayback = int(__fastcall*)(void* thisptr, int edx);

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

	int BaseTick = 0;

	void Set(uintptr_t clientPtr, uintptr_t gameDirAddr, uintptr_t curtimeAddr, uintptr_t loadGameAddr, uintptr_t mapnameAddr)
	{
		ClientPtr = **(void***)(clientPtr);
		ClientCmd = (_ClientCmd)GetVirtualFunctionByIndex(ClientPtr, Offsets::ClientCommand);
		GetGameDir = (_GetGameDir)gameDirAddr;

		TickCount = (int*)reinterpret_cast<uintptr_t*>(*((uintptr_t*)curtimeAddr) + Offsets::tickcount);
		IntervalPerTick = (float*)reinterpret_cast<uintptr_t*>(*((uintptr_t*)curtimeAddr) + Offsets::interval_per_tick);
		LoadGame = *(bool**)(loadGameAddr);
		Mapname = (char**)(mapnameAddr);
	}
	void ExecuteCommand(const char* cmd)
	{
		ClientCmd(ClientPtr, cmd);
	}
	int GetTick()
	{
		int result = *TickCount - BaseTick;
		return (result >= 0) ? result : 0;
	}
	float GetTime()
	{
		return GetTick() * *IntervalPerTick;
	}
	std::string GetDir()
	{
		char dir[256];
		Engine::GetGameDir(dir, 256);
		return std::string(dir);
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
		_StopPlayback StopPlayback;
	}

	namespace Detour
	{
		bool PlayerRequestedStop;
		bool IsRecordingDemo;

		bool PlayerRequestedPlayback;
		bool IsPlayingDemo;

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
			else if (state == SignonState::Full) {
				BaseTick = *TickCount;
			}
			return Original::SetSignonState(thisptr, state, spawncount);
		}
		bool __fastcall CloseDemoFile(void* thisptr, int edx)
		{
			return Original::CloseDemoFile(thisptr);
		}
		bool __fastcall StopRecording(void* thisptr, int edx)
		{
			const int LastDemoNumber = *DemoRecorder::DemoNumber;
			bool result = Original::StopRecording(thisptr);

			if (!PlayerRequestedStop) {
				if (!*LoadGame && !IsPlayingDemo) {
					int tick = Engine::GetTick();

					if (tick != 0)
						Console::ColorMsg(COL_YELLOW, "Session: %i ticks (%.3fs)\n", tick, GetTime());

					if (Summary::HasStarted) {
						Summary::Add(tick, GetTime(), *Mapname);
						Console::ColorMsg(COL_YELLOW, "Total: %i ticks (%.3fs)\n", Summary::TotalTicks, Summary::TotalTime);
					}
				}
				if (IsRecordingDemo) {
					*DemoRecorder::DemoNumber = LastDemoNumber;

					if (*LoadGame) {
						DemoRecorder::SetLastDemo();
						*DemoRecorder::Recording = true;
						(*DemoRecorder::DemoNumber)++;
					}
				}
			}
			else {
				IsRecordingDemo = false;
			}

			if (!PlayerRequestedPlayback && IsPlayingDemo)
				IsPlayingDemo = false;

			return result;
		}
		void __fastcall StartupDemoFile(void* thisptr, int edx)
		{
			IsRecordingDemo = true;
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
			//Console::ColorMsg(COL_YELLOW, "Disconnected at: %i\n", DemoRecorder::GetCurrentTick());
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
				std::string file = GetDir() + "\\" + std::string(DemoPlayer::DemoName);
				Demo demo;
				if (demo.Parse(file, false)) {
					demo.Fix();
					Console::Msg("Client: %s\n", demo.clientName);
					Console::Msg("Map: %s\n", demo.mapName);
					Console::Msg("Ticks: %i\n", demo.playbackTicks);
					Console::Msg("Time: %.3f\n", demo.playbackTime);
					Console::Msg("IpT: %.6f\n", demo.IntervalPerTick());
				}
				else {
					Console::Msg("Could not parse \"%s\"!\n", DemoPlayer::DemoName);
				}
			}
			return result;
		}
		int __fastcall StopPlayback(void* thisptr, int edx)
		{
			return Original::StopPlayback(thisptr, edx);
		}
	}
}