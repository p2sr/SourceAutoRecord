#pragma once

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Game.hpp"
#include "Utils.hpp"

#include "Patterns.hpp"

namespace Hooks
{
	// Make sure to enable everything after all
	// signature scans have been completed
	std::vector<uintptr_t> HookAddresses;

	ScanResult Create(ScanResult result, void* detour, void* original)
	{
		if (result.Found) {
			Console::DevMsg("SAR: %s\n", result.Message);

			// TODO
		}
		else {
			Console::DevWarning("SAR: %s\n", result.Message);
		}
		return result;
	}
	ScanResult Create(Pattern* toScan, void* detour, void* original)
	{
		return Create(Scan(toScan), detour, original);
	}
	bool Init()
	{
		// TODO
		return true;
	}
	void EnableAll()
	{
		int hooks = 0;
		// TODO
		/* for (auto &addr : HookAddresses) {
		} */
		Console::DevMsg("SAR: Enabled %i of %i hooks!\n", hooks, HookAddresses.size());
	}
	void CreateAll()
	{
		if (!Init()) return;

		//Create(Patterns::Get("CheckJumpButton"),	(void*)Server::Detour::CheckJumpButton,		&Server::Original::CheckJumpButton);
		/* Create(Patterns::Get("Paint"),				(void*)Client::Detour::Paint,				&Client::Original::Paint);
		Create(Patterns::Get("SetSignonState"),		(void*)Engine::Detour::SetSignonState,		&Engine::Original::SetSignonState);
		Create(Patterns::Get("StopRecording"),		(void*)Engine::Detour::StopRecording,		&Engine::Original::StopRecording);
		Create(Patterns::Get("StartupDemoFile"),	(void*)Engine::Detour::StartupDemoFile,		&Engine::Original::StartupDemoFile);
		Create(Patterns::Get("Stop"),				(void*)Engine::Detour::ConCommandStop,		&Engine::Original::ConCommandStop);
		Create(Patterns::Get("StartPlayback"),		(void*)Engine::Detour::StartPlayback,		&Engine::Original::StartPlayback);
		Create(Patterns::Get("PlayDemo"),			(void*)Engine::Detour::PlayDemo,			&Engine::Original::PlayDemo);
		Create(Patterns::Get("Disconnect"),			(void*)Engine::Detour::Disconnect,			&Engine::Original::Disconnect);
		Create(Patterns::Get("ShouldDraw"),			(void*)Client::Detour::ShouldDraw,			&Client::Original::ShouldDraw);
		Create(Patterns::Get("PlayerUse"),			(void*)Server::Detour::PlayerUse,			&Server::Original::PlayerUse);
		Create(Patterns::Get("HostStateFrame"),		(void*)Engine::Detour::HostStateFrame,		&Engine::Original::HostStateFrame);
		Create(Patterns::Get("CloseDemoFile"),		(void*)Engine::Detour::CloseDemoFile,		&Engine::Original::CloseDemoFile);
		Create(Patterns::Get("FindElement"),		(void*)Client::Detour::FindElement,			&Client::Original::FindElement);

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