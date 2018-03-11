#pragma once
#include "vmthook/vmthook.h"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Game.hpp"
#include "Utils.hpp"

#include "Interfaces.hpp"
#include "Patterns.hpp"

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

namespace Hooks
{
	void Load()
	{
		Interfaces::g_GameMovement = GetInterface("./portal2/bin/server.so", "GameMovement001");
		Interfaces::s_EntityList = GetInterface("./portal2/bin/client.so", "VClientEntityList003");

		if (Interfaces::g_GameMovement) {
			Console::Msg("Found interface IGameMovement (GameMovement001) in server.so\n");

			Server::Hooks::CheckJumpButton = std::make_unique<VMTHook>(Interfaces::g_GameMovement);
			Server::Hooks::CheckJumpButton
				->HookFunction((void*)Server::Detour::CheckJumpButton, Offsets::CheckJumpButton);
		}
		else
			Console::Msg("Failed to get interface IGameMovement (GameMovement001) in server.so!\n");
		
		if (Interfaces::s_EntityList) {
			Console::Msg("Found interface IClientEntityList (VClientEntityList003) in client.so\n");

			/* using _GetClientEntity = void*(__cdecl*)(void* thisptr, int entnum);
			auto GetClientEntity = *(_GetClientEntity*)(*((uintptr_t*)Interfaces::s_EntityList) + 12);

			auto player = GetClientEntity(Interfaces::s_EntityList , 1);
			if (!player) Console::DevWarning("Failed to find C_BasePlayer!\n");
			else {
				Server::Hooks::PlayerUse = std::make_unique<VMTHook>(player);
				Server::Hooks::PlayerUse
					->HookFunction((void*)Server::Detour::PlayerUse, Offsets::PlayerUse);
			} */
		}
		else
			Console::Msg("Failed to get interface IClientEntityList (VClientEntityList003) in client.so!\n");

		// server.so
		//Create("CheckJumpButton", &Server::Hooks::CheckJumpButton, (void*)Server::Detour::CheckJumpButton);
		/* Create("PlayerUse", Server::Hooks::PlayerUse, (void*)Server::Detour::PlayerUse);

		// client.so
		Create("PlayerUse", Client::Hooks::Paint, (void*)Client::Detour::Paint);
		Create("PlayerUse", Client::Hooks::ShouldDraw, (void*)Client::Detour::ShouldDraw);
		Create("PlayerUse", Client::Hooks::FindElement, (void*)Client::Detour::FindElement);

		// engine.so
		Create("SetSignonState", Engine::Hooks::SetSignonState, (void*)Engine::Detour::SetSignonState);
		Create("CloseDemoFile", Engine::Hooks::SetSignonState, (void*)Engine::Detour::CloseDemoFile);
		Create("StopRecording", Engine::Hooks::SetSignonState, (void*)Engine::Detour::StopRecording);
		Create("StartupDemoFile", Engine::Hooks::SetSignonState, (void*)Engine::Detour::StartupDemoFile);
		Create("ConCommandStop", Engine::Hooks::SetSignonState, (void*)Engine::Detour::ConCommandStop);
		Create("Disconnect", Engine::Hooks::SetSignonState, (void*)Engine::Detour::Disconnect);
		Create("PlayDemo", Engine::Hooks::SetSignonState, (void*)Engine::Detour::PlayDemo);
		Create("StartPlayback", Engine::Hooks::SetSignonState, (void*)Engine::Detour::StartPlayback);
		Create("HostStateFrame", Engine::Hooks::SetSignonState, (void*)Engine::Detour::HostStateFrame); */
		
		/*
		// Mid-function-hooks
		auto arm = Scan(Patterns::Get("AirMove"));
		Server::Original::AirMove = (void*)(arm.Address + 7);
		JumpToAt((void*)Server::Detour::AirMove, (void*)arm.Address);
		DoNothingAt(arm.Address + 5, 2);
		Server::Original::AirMoveSkip = (void*)(arm.Address + Offsets::AirMoveSkip);

		if (Game::Version == Game::Portal2) {
			auto prc = Scan(Patterns::Get("PlayerRunCommand"));
			Server::Original::PlayerRunCommand = (void*)(prc.Address + 7);
			JumpToAt((void*)Server::Detour::PlayerRunCommand, (void*)prc.Address);
			DoNothingAt(prc.Address + 5, 2);
			Server::Original::PlayerRunCommandSkip = (void*)(prc.Address + Offsets::PlayerRunCommandSkip);
		} */
	}
}