#pragma once
#include "minhook/MinHook.h"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Offsets.hpp"
#include "Patterns.hpp"
#include "Utils.hpp"

namespace Hooks
{
	// Make sure to enable everything after all
	// signature scans have been completed
	std::vector<uintptr_t> HookAddresses;

	ScanResult Create(Pattern toScan, LPVOID detour, LPVOID* original)
	{
		auto result = Scan(toScan);
		if (result.Found) {
			Console::DevMsg("SAR: %s\n", result.Message);

			if (MH_CreateHook(reinterpret_cast<LPVOID>(result.Address), detour, original) != MH_OK) {
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

		Create(Patterns::Get("CheckJumpButton"),	Server::Detour::CheckJumpButton,	reinterpret_cast<LPVOID*>(&Server::Original::CheckJumpButton));
		Create(Patterns::Get("Paint"),				Client::Detour::Paint,				reinterpret_cast<LPVOID*>(&Client::Original::Paint));
		Create(Patterns::Get("SetSignonState"),		Engine::Detour::SetSignonState,		reinterpret_cast<LPVOID*>(&Engine::Original::SetSignonState));
		Create(Patterns::Get("StopRecording"),		Engine::Detour::StopRecording,		reinterpret_cast<LPVOID*>(&Engine::Original::StopRecording));
		Create(Patterns::Get("StartupDemoFile"),	Engine::Detour::StartupDemoFile,	reinterpret_cast<LPVOID*>(&Engine::Original::StartupDemoFile));
		Create(Patterns::Get("Stop"),				Engine::Detour::ConCommandStop,		reinterpret_cast<LPVOID*>(&Engine::Original::ConCommandStop));
		Create(Patterns::Get("StartPlayback"),		Engine::Detour::StartPlayback,		reinterpret_cast<LPVOID*>(&Engine::Original::StartPlayback));
		Create(Patterns::Get("PlayDemo"),			Engine::Detour::PlayDemo,			reinterpret_cast<LPVOID*>(&Engine::Original::PlayDemo));
		Create(Patterns::Get("Disconnect"),			Engine::Detour::Disconnect,			reinterpret_cast<LPVOID*>(&Engine::Original::Disconnect));
		Create(Patterns::Get("ShouldDraw"),			Client::Detour::ShouldDraw,			reinterpret_cast<LPVOID*>(&Client::Original::ShouldDraw));
		Create(Patterns::Get("PlayerUse"),			Server::Detour::PlayerUse,			reinterpret_cast<LPVOID*>(&Server::Original::PlayerUse));
		Create(Patterns::Get("HostStateFrame"),		Engine::Detour::HostStateFrame,		reinterpret_cast<LPVOID*>(&Engine::Original::HostStateFrame));
		Create(Patterns::Get("CloseDemoFile"),		Engine::Detour::CloseDemoFile,		reinterpret_cast<LPVOID*>(&Engine::Original::CloseDemoFile));
		
		// Mid-function-hook
		auto amv =
		Create(Patterns::Get("AirMove"),			Server::Detour::AirMove,			NULL);
		Server::Set(amv.Address);
	}
}