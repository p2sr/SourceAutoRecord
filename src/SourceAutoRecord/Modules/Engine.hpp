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
#include "Features/TAS.hpp"
#include "Features/Timer.hpp"

#include "Interfaces.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

using namespace Commands;

namespace Engine
{
	using _ClientCmd = int(__cdecl*)(void* thisptr, const char* szCmdString);
	using _Disconnect = int(__cdecl*)(void* thisptr, int bShowMainMenu);
	using _SetSignonState = int(__cdecl*)(void* thisptr, int state, int count);
	using _GetLocalPlayer = int(__cdecl*)(void* thisptr);
	using _GetViewAngles = int(__cdecl*)(void* thisptr, QAngle& va);
	using _SetViewAngles = int(__cdecl*)(void* thisptr, QAngle& va);

	std::unique_ptr<VMTHook> engine;
	std::unique_ptr<VMTHook> cl;

	_ClientCmd ClientCmd;
	_GetLocalPlayer GetLocalPlayer;
	_GetViewAngles GetViewAngles;
	_SetViewAngles SetViewAngles;

	void ExecuteCommand(const char* cmd)
	{
		ClientCmd(engine->GetThisPtr(), cmd);
	}
	int GetTick()
	{
		int result = *Vars::tickcount - Session::BaseTick;
		return (result >= 0) ? result : 0;
	}
	std::string GetDir()
	{
		return std::string(Vars::GetGameDirectory());
	}
	int GetPlayerIndex()
	{
		return GetLocalPlayer(engine->GetThisPtr());
	}
	QAngle GetAngles()
	{
		auto va = QAngle();
		GetViewAngles(engine->GetThisPtr(), va);
		return va;
	}
	void SetAngles(QAngle va)
	{
		SetViewAngles(engine->GetThisPtr(), va);
	}

	namespace Original
	{
		_Disconnect Disconnect;
		_SetSignonState SetSignonState;
	}

	namespace Detour
	{
		bool IsInGame = true;

		int __cdecl Disconnect(void* thisptr, bool bShowMainMenu)
		{
			//Console::PrintActive("Disconnect!\n");
			if (!*Vars::m_bLoadgame && !DemoPlayer::IsPlaying() && IsInGame) {
				int tick = GetTick();

				if (tick != 0) {
					Console::Print("Session: %i (%.3f)\n", tick, tick * *Vars::interval_per_tick);
					Session::LastSession = tick;
				}

				if (Summary::IsRunning) {
					Summary::Add(tick, tick * *Vars::interval_per_tick, *Vars::m_szLevelName);
					Console::Print("Total: %i (%.3f)\n", Summary::TotalTicks, Summary::TotalTime);
				}

				if (Timer::IsRunning) {
					if (sar_timer_always_running.GetBool()) {
						Timer::Save(*Vars::tickcount);
						Console::Print("Timer paused: %i (%.3f)!\n", Timer::TotalTicks, Timer::TotalTicks * *Vars::interval_per_tick);
					}
					else {
						Timer::Stop(*Vars::tickcount);
						Console::Print("Timer stopped!\n");
					}
				}

				if (sar_stats_auto_reset.GetInt() >= 1) {
					Stats::Reset();
				}

				DemoRecorder::CurrentDemo = "";
			}
			IsInGame = false;
			return Original::Disconnect(thisptr, bShowMainMenu);
		}
		int __cdecl SetSignonState(void* thisptr, int state, int count)
		{
			//Console::PrintActive("SetSignonState = %i\n", state);
			if (state == SignonState::Prespawn) {
				if (Rebinder::IsSaveBinding || Rebinder::IsReloadBinding) {
					Rebinder::LastIndexNumber = (DemoRecorder::IsRecordingDemo)
						? *DemoRecorder::m_nDemoNumber
						: Rebinder::LastIndexNumber + 1;

					Rebinder::RebindSave();
					Rebinder::RebindReload();
				}
			}
			// Demo recorder starts syncing from this tick
			else if (state == SignonState::Full) {
				Session::Rebase(*Vars::tickcount);
				Timer::Rebase(*Vars::tickcount);

				if (*Vars::m_bLoadgame && sar_tas_autostart.GetBool()) {
					Console::DevMsg("---TAS START---\n");
					TAS::Start();
				}
				IsInGame = true;
			}
			return Original::SetSignonState(thisptr, state, count);
		}
	}

	void Hook()
	{
		if (Interfaces::IVEngineClient) {
			engine = std::make_unique<VMTHook>(Interfaces::IVEngineClient);
			ClientCmd = engine->GetOriginalFunction<_ClientCmd>(Offsets::ClientCmd);
			GetLocalPlayer = engine->GetOriginalFunction<_GetLocalPlayer>(Offsets::GetLocalPlayer);
			GetViewAngles = engine->GetOriginalFunction<_GetViewAngles>(Offsets::GetViewAngles);
			SetViewAngles = engine->GetOriginalFunction<_GetViewAngles>(Offsets::SetViewAngles);
			Vars::GetGameDirectory = engine->GetOriginalFunction<Vars::_GetGameDirectory>(Offsets::GetGameDirectory);
			
			typedef void*(*_GetClientState)();
			auto abs = GetAbsoluteAddress((uintptr_t)ClientCmd + Offsets::GetClientState);
			auto GetClientState = reinterpret_cast<_GetClientState>(abs);
			cl = std::make_unique<VMTHook>(GetClientState());

			// Before Disconnect in VMT :^)
			cl->HookFunction((void*)Detour::SetSignonState, Offsets::Disconnect - 1);
			cl->HookFunction((void*)Detour::Disconnect, Offsets::Disconnect);
			Original::SetSignonState = cl->GetOriginalFunction<_SetSignonState>(Offsets::Disconnect - 1);
			Original::Disconnect = cl->GetOriginalFunction<_Disconnect>(Offsets::Disconnect);

			auto ProcessTick = cl->GetOriginalFunction<uintptr_t>(Offsets::ProcessTick);
			Vars::tickcount = *reinterpret_cast<int**>(ProcessTick + Offsets::tickcount);
			Vars::interval_per_tick = *reinterpret_cast<float**>(ProcessTick + Offsets::interval_per_tick);

			DemoPlayer::Hook(**reinterpret_cast<void***>((uintptr_t)Original::Disconnect + Offsets::demoplayer));
			DemoRecorder::Hook(**reinterpret_cast<void***>((uintptr_t)Original::Disconnect + Offsets::demorecorder));
		}

		if (Interfaces::IEngineTool) {
			auto tool = std::make_unique<VMTHook>(Interfaces::IEngineTool);
			auto GetCurrentMap = tool->GetOriginalFunction<uintptr_t>(Offsets::GetCurrentMap);
			Vars::m_szLevelName = reinterpret_cast<char**>(GetCurrentMap + Offsets::m_szLevelName);
		}
	}
}