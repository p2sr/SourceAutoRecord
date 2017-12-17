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
	ScanResult CreateAndEnable(Pattern toScan, LPVOID detour, LPVOID* original)
	{
		auto result = Scan(toScan);
		if (result.Found) {
			Console::DevMsg("SAR: %s\n", result.Message);
			auto created = MH_CreateHook(reinterpret_cast<LPVOID>(result.Address), detour, original);
			auto enabled = MH_EnableHook(reinterpret_cast<LPVOID>(result.Address));
			if (created != MH_OK || enabled != MH_OK) Console::DevWarning("SAR: Could not create or enable this hook!\n");
		}
		else {
			Console::DevWarning("SAR: %s\n", result.Message);
		}
		return result;
	}

	void Load()
	{
		if (MH_Initialize() != MH_OK) {
			Console::DevWarning("SAR: Failed to init MinHook!\n");
			return;
		}

		CreateAndEnable(Patterns::CheckJumpButton,	Server::Detour::CheckJumpButton,	reinterpret_cast<LPVOID*>(&Server::Original::CheckJumpButton));
		CreateAndEnable(Patterns::Paint,			Client::Detour::Paint,				reinterpret_cast<LPVOID*>(&Client::Original::Paint));
		CreateAndEnable(Patterns::SetSignonState,	Engine::Detour::SetSignonState,		reinterpret_cast<LPVOID*>(&Engine::Original::SetSignonState));
		CreateAndEnable(Patterns::StopRecording,	Engine::Detour::StopRecording,		reinterpret_cast<LPVOID*>(&Engine::Original::StopRecording));
		CreateAndEnable(Patterns::StartupDemoFile,	Engine::Detour::StartupDemoFile,	reinterpret_cast<LPVOID*>(&Engine::Original::StartupDemoFile));
		CreateAndEnable(Patterns::Stop,				Engine::Detour::ConCommandStop,		reinterpret_cast<LPVOID*>(&Engine::Original::ConCommandStop));
		CreateAndEnable(Patterns::StartPlayback,	Engine::Detour::StartPlayback,		reinterpret_cast<LPVOID*>(&Engine::Original::StartPlayback));
		CreateAndEnable(Patterns::PlayDemo,			Engine::Detour::PlayDemo,			reinterpret_cast<LPVOID*>(&Engine::Original::PlayDemo));
		CreateAndEnable(Patterns::Disconnect,		Engine::Detour::Disconnect,			reinterpret_cast<LPVOID*>(&Engine::Original::Disconnect));
		CreateAndEnable(Patterns::ShouldDraw,		Client::Detour::ShouldDraw,			reinterpret_cast<LPVOID*>(&Client::Original::ShouldDraw));
		CreateAndEnable(Patterns::PlayerUse,		Server::Detour::PlayerUse,			reinterpret_cast<LPVOID*>(&Server::Original::PlayerUse));
		CreateAndEnable(Patterns::HostStateFrame,	Engine::Detour::HostStateFrame,		reinterpret_cast<LPVOID*>(&Engine::Original::HostStateFrame));
		CreateAndEnable(Patterns::CloseDemoFile,	Engine::Detour::CloseDemoFile,		reinterpret_cast<LPVOID*>(&Engine::Original::CloseDemoFile));
		
		auto amv =
		CreateAndEnable(Patterns::AirMove,			Engine::Detour::AirMove,			reinterpret_cast<LPVOID*>(&Engine::Original::AirMove));
		Engine::Detour::SetAirMove(amv.Address);
	}
}