#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Game.hpp"
#include "Interfaces.hpp"
#include "Patterns.hpp"
#include "Utils.hpp"

namespace Hooks
{
	ScanResult Find(const char* pattern)
	{
		auto result = Scan(Patterns::Get(pattern));
		if (result.Found) {
			Console::DevMsg("%s\n", result.Message);
		}
		else {
			Console::DevWarning("%s\n", result.Message);
		}
		return result;
	}
	void Load()
	{
		Interfaces::IGameMovement = Interfaces::Get("./portal2/bin/server.so", "GameMovement001");
		if (Interfaces::IGameMovement) {
			Console::PrintActive("Found interface IGameMovement (GameMovement001) in server.so!\n");
			Server::g_GameMovement = std::make_unique<VMTHook>(Interfaces::IGameMovement);
			Server::g_GameMovement->HookFunction((void*)Server::CheckJumpButton, Offsets::CheckJumpButton);
		}
		else {
			Console::Warning("Failed to get interface IGameMovement (GameMovement001) in server.so!\n");
		}

		Interfaces::IVEngineClient = Interfaces::Get("./bin/engine.so", "VEngineClient015");
		if (Interfaces::IVEngineClient) {
			Console::PrintActive("Found interface IVEngineClient (VEngineClient015) in engine.so!\n");
			Engine::engine = std::make_unique<VMTHook>(Interfaces::IVEngineClient);
		}
		else {
			Console::Warning("Failed to get interface IVEngineClient (VEngineClient015) in engine.so!\n");
		}

		auto drecorder = Find("demorecorder");
		if (drecorder.Found) {
			Console::PrintActive("Found CDemoRecorder s_ClientDemoRecorder in engine.so!\n");
			DemoRecorder::Set(drecorder.Address);
			Engine::s_ClientDemoRecorder = std::make_unique<VMTHook>(**(void***)drecorder.Address);
			Engine::s_ClientDemoRecorder->HookFunction((void*)Engine::SetSignonState, Offsets::SetSignonState);
			Engine::s_ClientDemoRecorder->HookFunction((void*)Engine::StopRecording, Offsets::StopRecording);
		}
		else {
			Console::Warning("Failed to get CDemoRecorder s_ClientDemoRecorder in engine.so!\n");
		}

		auto dplayer = Find("demoplayer");
		if (drecorder.Found) {
			DemoPlayer::Set(dplayer.Address);
			Console::PrintActive("Found CDemoPlayer s_ClientDemoPlayer in engine.so!\n");
			Engine::s_ClientDemoPlayer = std::make_unique<VMTHook>(**(void***)dplayer.Address);
			Engine::s_ClientDemoPlayer->HookFunction((void*)Engine::StartPlayback, Offsets::StartPlayback);
		}
		else {
			Console::Warning("Failed to get CDemoPlayer s_ClientDemoPlayer in engine.so!\n");
		}

		// CClientState cl (function)
		// \x55\x89\xE5\x83\xEC\x00\xC7\x44\x24\x00\x00\x00\x00\x00\xC7\x04\x24\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC9\xC3\x8D\x74\x26\x00\x55\x89\xE5\x57\x56\x53\x83\xEC\x00\xA1\x00\x00\x00\x00 xxxxx?xxx?????xxx????x????xxxxxxxxxxxxxx?x????
		auto cstate = Scan("engine.so", "55 89 E5 83 EC ? C7 44 24 ? ? ? ? ? C7 04 24 ? ? ? ? E8 ? ? ? ? C9 C3 8D 74 26 00 55 89 E5 57 56 53 83 EC ? A1 ? ? ? ? ");
		if (cstate.Found) {
			Console::PrintActive("Found CClientState function in engine.so!\n");
			typedef int (*_GetClientState)();
			auto GetClientState = reinterpret_cast<_GetClientState>(cstate.Address);
			Engine::cl = std::make_unique<VMTHook>((void*)GetClientState());
			Engine::cl->HookFunction((void*)Engine::Disconnect, Offsets::Disconnect);
		}
		else {
			Console::Warning("Failed test in engine.so!\n");
		}

		Interfaces::IBaseClientDLL = Interfaces::Get("./portal2/bin/client.so", "VClient016");
		if (Interfaces::IBaseClientDLL) {
			Console::PrintActive("Found interface IBaseClientDLL (VClient016) in client.so!\n");
			auto clientdll = std::make_unique<VMTHook>(Interfaces::IBaseClientDLL);
			auto HudUpdate = clientdll->GetOriginalFunction<uintptr_t>(Offsets::HudUpdate) + 13;
			Engine::gpGlobals = **reinterpret_cast<CGlobalVarsBase***>(HudUpdate);
		}
		else {
			Console::Warning("Failed to get interface IBaseClientDLL (VClient016) in client.so!\n");
		}

		auto ldg = Find("m_bLoadgame");
		auto mpn = Find("m_szMapname");
		auto ins = Find("g_pInputSystem");
		auto ksb = Find("Key_SetBinding");

		if (ldg.Found && mpn.Found && ins.Found && ksb.Found) {
			Console::PrintActive("Found required engine stuff in engine.so!\n");
			Engine::Set(ldg.Address, mpn.Address);
			InputSystem::Set(ins.Address, ksb.Address);
		}
		else {
			Console::Warning("Failed to get required engine stuff engine.so!\n");
		}
	}
	bool LoadTier1()
	{
		auto cvr = Find("CvarPtr");
		auto cnv = Find("ConVar_Ctor3");
		auto cnc = Find("ConCommand_Ctor1");
		auto cnc2 = Find("ConCommand_Ctor2");

		if (!cvr.Found || !cnv.Found || !cnc.Found || !cnc2.Found)
			return false;

		Cvar::Set(cvr.Address);
		Tier1::SetConVar(cnv.Address);
		Tier1::SetConCommand(cnc.Address, cnc2.Address);
		return true;
	}
	bool LoadClient()
	{
		auto sts = Find("SetSize");
		auto glp = Find("GetLocalPlayer");
		auto gtp = Find("GetPos");
		auto mss = Find("g_pMatSystemSurface");

		if (!sts.Found || !glp.Found || !gtp.Found|| !mss.Found)
			return false;

		Client::Set(sts.Address, glp.Address, gtp.Address);
		Surface::Set(mss.Address);
		return true;
	}
}

//Interfaces::s_EntityList = GetInterface("./portal2/bin/client.so", "VClientEntityList003");
/* class IClientEntityList {
	public:
		virtual void* GetClientNetworkable(int entindex) = 0;
		virtual void* GetClientNetworkableFromHandle(int handle) = 0;
		virtual void* GetClientUnknownFromHandle(int handle) = 0;
		virtual void* GetClientEntity(int entindex) = 0;
		virtual void* GetClientEntityFromHandle(int handle) = 0;
		virtual int NumberOfEntities(bool include_non_networkable) = 0;
		virtual int GetHighestEntityIndex(void) = 0;
		virtual void SetMaxEntities(int max_entities) = 0;
		virtual int GetMaxEntities() = 0;
}; */