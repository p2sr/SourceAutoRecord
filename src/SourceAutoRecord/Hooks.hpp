#pragma once
#include "minhook/MinHook.h"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Offsets.hpp"
#include "Patterns.hpp"
#include "Utils.hpp"

namespace Hooks
{
	ScanResult cjb, pnt, sst, cdf, str, sdf, stp, spb, pld, dsc, spl;

	void Load()
	{
		cjb = Scan(Patterns::CheckJumpButton);
		pnt = Scan(Patterns::Paint);
		sst = Scan(Patterns::SetSignonState);
		cdf = Scan(Patterns::CloseDemoFile);
		str = Scan(Patterns::StopRecording);
		sdf = Scan(Patterns::StartupDemoFile);
		stp = Scan(Patterns::Stop);
		spb = Scan(Patterns::StartPlayback);
		pld = Scan(Patterns::PlayDemo);
		dsc = Scan(Patterns::Disconnect);
		spl = Scan(Patterns::StopPlayback);

		Offsets::Init(cjb.Index);
	}

	void CreateAndEnable()
	{
		MH_Initialize();
		MH_CreateHook(reinterpret_cast<LPVOID>(cjb.Address), Server::Detour::CheckJumpButton, reinterpret_cast<LPVOID*>(&Server::Original::CheckJumpButton));
		MH_CreateHook(reinterpret_cast<LPVOID>(pnt.Address), Client::Detour::Paint, reinterpret_cast<LPVOID*>(&Client::Original::Paint));
		MH_CreateHook(reinterpret_cast<LPVOID>(sst.Address), Engine::Detour::SetSignonState, reinterpret_cast<LPVOID*>(&Engine::Original::SetSignonState));
		MH_CreateHook(reinterpret_cast<LPVOID>(cdf.Address), Engine::Detour::CloseDemoFile, reinterpret_cast<LPVOID*>(&Engine::Original::CloseDemoFile));
		MH_CreateHook(reinterpret_cast<LPVOID>(str.Address), Engine::Detour::StopRecording, reinterpret_cast<LPVOID*>(&Engine::Original::StopRecording));
		MH_CreateHook(reinterpret_cast<LPVOID>(sdf.Address), Engine::Detour::StartupDemoFile, reinterpret_cast<LPVOID*>(&Engine::Original::StartupDemoFile));
		MH_CreateHook(reinterpret_cast<LPVOID>(stp.Address), Engine::Detour::ConCommandStop, reinterpret_cast<LPVOID*>(&Engine::Original::ConCommandStop));
		MH_CreateHook(reinterpret_cast<LPVOID>(spb.Address), Engine::Detour::StartPlayback, reinterpret_cast<LPVOID*>(&Engine::Original::StartPlayback));
		MH_CreateHook(reinterpret_cast<LPVOID>(pld.Address), Engine::Detour::PlayDemo, reinterpret_cast<LPVOID*>(&Engine::Original::PlayDemo));
		MH_CreateHook(reinterpret_cast<LPVOID>(dsc.Address), Engine::Detour::Disconnect, reinterpret_cast<LPVOID*>(&Engine::Original::Disconnect));
		MH_CreateHook(reinterpret_cast<LPVOID>(spl.Address), Engine::Detour::StopPlayback, reinterpret_cast<LPVOID*>(&Engine::Original::StopPlayback));
		MH_EnableHook(reinterpret_cast<LPVOID>(cjb.Address));
		MH_EnableHook(reinterpret_cast<LPVOID>(pnt.Address));
		MH_EnableHook(reinterpret_cast<LPVOID>(sst.Address));
		MH_EnableHook(reinterpret_cast<LPVOID>(cdf.Address));
		MH_EnableHook(reinterpret_cast<LPVOID>(str.Address));
		MH_EnableHook(reinterpret_cast<LPVOID>(sdf.Address));
		MH_EnableHook(reinterpret_cast<LPVOID>(stp.Address));
		MH_EnableHook(reinterpret_cast<LPVOID>(spb.Address));
		MH_EnableHook(reinterpret_cast<LPVOID>(pld.Address));
		MH_EnableHook(reinterpret_cast<LPVOID>(dsc.Address));
		MH_EnableHook(reinterpret_cast<LPVOID>(spl.Address));
	}
}