#pragma once
#include <string>

#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "DemoRecorder.hpp"
#include "DemoPlayer.hpp"
#include "Server.hpp"
#include "Vars.hpp"

#include "Features/Session.hpp"
#include "Features/Stats.hpp"
#include "Features/Summary.hpp"
#include "Features/TAS.hpp"
#include "Features/Timer.hpp"

#include "Game.hpp"
#include "Interfaces.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

using namespace Commands;

namespace Engine
{
	using _ClientCmd = int(__cdecl*)(void* thisptr, const char* szCmdString);
	using _Disconnect = int(__cdecl*)(void* thisptr, int bShowMainMenu);
	using _Disconnect2 = int(__cdecl*)(void* thisptr, int* unk1, char bShowMainMenu);
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
	float GetTime(int tick)
	{
		return tick * *Vars::interval_per_tick;
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

	bool IsInGame = false;

	void SessionStarted()
	{
		Console::PrintActive("m_bLoadgame = %i\n", *Vars::m_bLoadgame);
		Session::Rebase(*Vars::tickcount);
		Timer::Rebase(*Vars::tickcount);

		if (Rebinder::IsSaveBinding || Rebinder::IsReloadBinding) {
			if (DemoRecorder::IsRecordingDemo) {
				Rebinder::UpdateIndex(*DemoRecorder::m_nDemoNumber);
			}
			else {
				Rebinder::UpdateIndex(Rebinder::LastIndexNumber + 1);
			}

			Rebinder::RebindSave();
			Rebinder::RebindReload();
		}

		if (*Vars::m_bLoadgame && sar_tas_autostart.GetBool()) {
			Console::DevMsg("---TAS START---\n");
			TAS::Start();
		}

		Server::StepSoundTime = 0;
		IsInGame = true;
	}
	void SessionEnded()
	{
		if (!DemoPlayer::IsPlaying() && IsInGame) {
			int tick = GetTick();

			if (tick != 0) {
				Console::Print("Session: %i (%.3f)\n", tick, Engine::GetTime(tick));
				Session::LastSession = tick;
			}

			if (Summary::IsRunning) {
				Summary::Add(tick, Engine::GetTime(tick), *Vars::m_szLevelName);
				Console::Print("Total: %i (%.3f)\n", Summary::TotalTicks, Engine::GetTime(Summary::TotalTicks));
			}

			if (Timer::IsRunning) {
				if (sar_timer_always_running.GetBool()) {
					Timer::Save(*Vars::tickcount);
					Console::Print("Timer paused: %i (%.3f)!\n", Timer::TotalTicks, Engine::GetTime(Timer::TotalTicks));
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
	}

	namespace Original
	{
		_Disconnect Disconnect;
		_Disconnect2 Disconnect2;
		_SetSignonState SetSignonState;
	}

	namespace Detour
	{
		int LastState;

		int __cdecl Disconnect(void* thisptr, bool bShowMainMenu)
		{
			//Console::PrintActive("Disconnect!\n");
			Console::PrintActive("Session ended (Disconnect)!\n");
			SessionEnded();
			return Original::Disconnect(thisptr, bShowMainMenu);
		}
		int __cdecl Disconnect2(void* thisptr, int* unk1, bool bShowMainMenu)
		{
			//Console::PrintActive("Disconnect!\n");
			Console::PrintActive("Session ended (Disconnect)!\n");
			SessionEnded();
			return Original::Disconnect2(thisptr, unk1, bShowMainMenu);
		}
		int __cdecl SetSignonState(void* thisptr, int state, int count)
		{
			//Console::PrintActive("SetSignonState = %i\n", state);
			if (state != LastState && LastState == SignonState::Full) {
				Console::PrintActive("Session ended (SetSignonState)!\n");
				SessionEnded();
			}

			// Demo recorder starts syncing from this tick
			if (state == SignonState::Full) {
				Console::PrintActive("Session started!\n");
				SessionStarted();
			}

			LastState = state;
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

			if (Offsets::GetClientStateFunction != 0) {
				typedef void*(*_GetClientStateFunction)();
				auto abs = GetAbsoluteAddress((uintptr_t)ClientCmd + Offsets::GetClientStateFunction);
				auto GetClientStateFunction = reinterpret_cast<_GetClientStateFunction>(abs);
				cl = std::make_unique<VMTHook>(GetClientStateFunction());
			}
			else {
				auto module = MODULEINFO();
				GetModuleInformation("engine.so", &module);
				Console::PrintActive("%i\n", 0xAE9420);
				//auto unk = engine->GetOriginalFunction<uintptr_t>(128);
				//auto offset = *reinterpret_cast<int*>(unk + 6);
				auto ptr = reinterpret_cast<void*>(0xAE9420 + module.lpBaseOfDll);
				cl = std::make_unique<VMTHook>(ptr);
			}

			// Before Disconnect in VMT :^)
			cl->HookFunction((void*)Detour::SetSignonState, Offsets::Disconnect - 1);
			Original::SetSignonState = cl->GetOriginalFunction<_SetSignonState>(Offsets::Disconnect - 1);

			uintptr_t disconnect;
			if (Game::Version == Game::Portal2) {
				cl->HookFunction((void*)Detour::Disconnect, Offsets::Disconnect);
				Original::Disconnect = cl->GetOriginalFunction<_Disconnect>(Offsets::Disconnect);
				disconnect = (uintptr_t)Original::Disconnect;
			}
			else {
				cl->HookFunction((void*)Detour::Disconnect2, Offsets::Disconnect);
				Original::Disconnect2 = cl->GetOriginalFunction<_Disconnect2>(Offsets::Disconnect);
				disconnect = (uintptr_t)Original::Disconnect2;
			}

			DemoPlayer::Hook(**reinterpret_cast<void***>(disconnect + Offsets::demoplayer));
			DemoRecorder::Hook(**reinterpret_cast<void***>(disconnect + Offsets::demorecorder));

			auto ProcessTick = cl->GetOriginalFunction<uintptr_t>(Offsets::ProcessTick);
			Vars::tickcount = *reinterpret_cast<int**>(ProcessTick + Offsets::tickcount);
			Vars::interval_per_tick = *reinterpret_cast<float**>(ProcessTick + Offsets::interval_per_tick);
		}

		if (Interfaces::IEngineTool) {
			auto tool = std::make_unique<VMTHook>(Interfaces::IEngineTool);
			auto GetCurrentMap = tool->GetOriginalFunction<uintptr_t>(Offsets::GetCurrentMap);
			Vars::m_szLevelName = reinterpret_cast<char**>(GetCurrentMap + Offsets::m_szLevelName);
		}
	}
	void Unhook()
	{
		if (engine) {
			engine->~VMTHook();
			engine.release();
			ClientCmd = nullptr;
			GetLocalPlayer = nullptr;
			GetViewAngles = nullptr;
			SetViewAngles = nullptr;
			Vars::GetGameDirectory = nullptr;

			cl->UnhookFunction(Offsets::Disconnect - 1);
			cl->UnhookFunction(Offsets::Disconnect);
			cl->~VMTHook();
			cl.release();
			Original::SetSignonState = nullptr;
			Original::Disconnect = nullptr;
			Original::Disconnect2 = nullptr;

			Vars::tickcount = nullptr;
			Vars::interval_per_tick = nullptr;

			DemoPlayer::Unhook();
			DemoRecorder::Unhook();
		}

		if (Vars::m_szLevelName) {
			Vars::m_szLevelName = nullptr;
		}
	}
}