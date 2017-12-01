#pragma once
#include <string>
#include "Console.hpp"
#include "../Rebinder.hpp"
#include "../Recorder.hpp"

using _GetGameDir = void(__cdecl*)(char* szGetGameDir, int maxlength);
using _ClientCommand = void(__fastcall*)(void* thisptr, void* edx, const char* str);

using _SetSignonState = bool(__thiscall*)(void* thisptr, int state, int spawncount);
using _CloseDemoFile = bool(__thiscall*)(void* thisptr);
using _StopRecording = int(__thiscall*)(void* thisptr);
using _StartupDemoFile = void(__thiscall*)(void* thisptr);
using _ConCommandStop = void(__cdecl*)();

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
	_ClientCommand ClientCommand;

	int* TickCount;
	bool* LoadGame;

	int BaseTick = 0;

	void Set(uintptr_t clientPtr, uintptr_t gameDirAddr, uintptr_t curtimeAddr, uintptr_t loadGameAddr)
	{
		ClientPtr = **(void***)(clientPtr);
		ClientCommand = (_ClientCommand)GetVirtualFunctionByIndex(ClientPtr, Offsets::ClientCommand);
		GetGameDir = (_GetGameDir)gameDirAddr;
		TickCount = (int*)reinterpret_cast<uintptr_t*>(*((uintptr_t*)curtimeAddr) + 12);
		LoadGame = *(bool**)(loadGameAddr);

		if (!*LoadGame) {
			BaseTick = *TickCount;
		}
	}
	void ExecuteCommand(const char* str)
	{
		ClientCommand(ClientPtr, nullptr, str);
	}
	int GetCurrentTick()
	{
		int result = *TickCount - BaseTick;
		return (result >= 0) ? result : 0;
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
	}

	namespace Detour
	{
		bool PlayerRequestedStop;
		bool IsRecording;

		bool __fastcall SetSignonState(void* thisptr, int edx, int state, int spawncount)
		{
			if (state == SignonState::Prespawn) {
				if (sar_rebinder_save.GetBool()) {
					Rebinder::RebindSave(*Recorder::Recording ? *Recorder::DemoNumber : -1);
				}
				if (sar_rebinder_reload.GetBool()) {
					Rebinder::RebindReload(*Recorder::Recording ? *Recorder::DemoNumber : -1);
				}
			}
			if (state == SignonState::Full) {
				BaseTick = *TickCount;
			}
			return Original::SetSignonState(thisptr, state, spawncount);
		}
		// TODO: try to fix demo bug here (Portal 2)
		bool __fastcall CloseDemoFile(void* thisptr, int edx)
		{
			return Original::CloseDemoFile(thisptr);
		}
		bool __fastcall StopRecording(void* thisptr, int edx)
		{
			const int LastDemoNumber = *Recorder::DemoNumber;
			bool result = Original::StopRecording(thisptr);

			if (!PlayerRequestedStop) {
				if (!*LoadGame) {
					Console::ColorMsg(COL_YELLOW, "Session: %i ticks\n", Engine::GetCurrentTick());
				}
				if (IsRecording) {
					*Recorder::DemoNumber = LastDemoNumber;
					if (*LoadGame) {
						Recorder::SetLastDemo();
						*Recorder::Recording = true;
						(*Recorder::DemoNumber)++;
					}
				}
			}
			else {
				IsRecording = false;
			}
			return result;
		}
		void __fastcall StartupDemoFile(void* thisptr, int edx)
		{
			IsRecording = true;
			Original::StartupDemoFile(thisptr);
		}
		void __cdecl ConCommandStop()
		{
			PlayerRequestedStop = true;
			Original::ConCommandStop();
			PlayerRequestedStop = false;
		}
	}
}