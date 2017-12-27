#pragma once
#include "minhook/MinHook.h"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Game.hpp"
#include "Patterns.hpp"
#include "Utils.hpp"

namespace Hooks
{
	// Make sure to enable everything after all
	// signature scans have been completed
	std::vector<uintptr_t> HookAddresses;

	ScanResult Create(Pattern* toScan, LPVOID detour, LPVOID original)
	{
		auto result = Scan(toScan);
		if (result.Found) {
			Console::DevMsg("SAR: %s\n", result.Message);

			if (MH_CreateHook(reinterpret_cast<LPVOID>(result.Address), detour, reinterpret_cast<LPVOID*>(original)) != MH_OK) {
				Console::DevWarning("SAR: Could not create this hook!\n");
			}
			else {
				HookAddresses.push_back(result.Address);
			}
		}
		else {
			Console::DevWarning("SAR: %s\n", result.Message);
		}
		return result;
	}
	void EnableAll()
	{
		int hooks = 0;
		for (auto &addr : HookAddresses) {
			if (MH_EnableHook(reinterpret_cast<LPVOID>(addr)) != MH_OK) {
				Console::DevWarning("SAR: Could not enable the hook at 0x%p!\n", addr);
			}
			else {
				hooks++;
			}
		}
		Console::DevMsg("SAR: Enabled %i of %i hooks!\n", hooks, HookAddresses.size());
	}
	void CreateAll()
	{
		if (MH_Initialize() != MH_OK) {
			Console::DevWarning("SAR: Failed to init MinHook!\n");
			return;
		}

		Create(Patterns::Get("CheckJumpButton"),	Server::Detour::CheckJumpButton,	&Server::Original::CheckJumpButton);
		Create(Patterns::Get("Paint"),				Client::Detour::Paint,				&Client::Original::Paint);
		Create(Patterns::Get("SetSignonState"),		Engine::Detour::SetSignonState,		&Engine::Original::SetSignonState);
		Create(Patterns::Get("StopRecording"),		Engine::Detour::StopRecording,		&Engine::Original::StopRecording);
		Create(Patterns::Get("StartupDemoFile"),	Engine::Detour::StartupDemoFile,	&Engine::Original::StartupDemoFile);
		Create(Patterns::Get("Stop"),				Engine::Detour::ConCommandStop,		&Engine::Original::ConCommandStop);
		Create(Patterns::Get("StartPlayback"),		Engine::Detour::StartPlayback,		&Engine::Original::StartPlayback);
		Create(Patterns::Get("PlayDemo"),			Engine::Detour::PlayDemo,			&Engine::Original::PlayDemo);
		Create(Patterns::Get("Disconnect"),			Engine::Detour::Disconnect,			&Engine::Original::Disconnect);
		Create(Patterns::Get("ShouldDraw"),			Client::Detour::ShouldDraw,			&Client::Original::ShouldDraw);
		Create(Patterns::Get("PlayerUse"),			Server::Detour::PlayerUse,			&Server::Original::PlayerUse);
		Create(Patterns::Get("HostStateFrame"),		Engine::Detour::HostStateFrame,		&Engine::Original::HostStateFrame);
		Create(Patterns::Get("CloseDemoFile"),		Engine::Detour::CloseDemoFile,		&Engine::Original::CloseDemoFile);
		Create(Patterns::Get("FindElement"),		Client::Detour::FindElement,		&Client::Original::FindElement);

		// Mid-function-hooks
		Server::SetAirMove(Create(Patterns::Get("AirMove"), Server::Detour::AirMove, NULL).Address);
		if (Game::Version == Game::Portal2) {
			Server::SetRunCommand(Create(Patterns::Get("PlayerRunCommand"), Server::Detour::PlayerRunCommand, NULL).Address);
		}
	}
}