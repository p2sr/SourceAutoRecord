#pragma once
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

namespace Vars
{
	CGlobalVarsBase* gpGlobals;

	using _GetGameDirectory = char*(__cdecl*)();
	_GetGameDirectory GetGameDirectory;

	bool* LoadGame;
	char** Mapname;

	void Hook()
	{
		if (Interfaces::IServerGameDLL) {
			auto serverdll = std::make_unique<VMTHook>(Interfaces::IServerGameDLL);
			auto LevelInit = serverdll->GetOriginalFunction<uintptr_t>(Offsets::LevelInit);
			gpGlobals = **reinterpret_cast<CGlobalVarsBase***>(LevelInit + Offsets::LevelInit_gpGlobals);
		}

		auto ldg = SAR::Find("m_bLoadgame");
		auto mpn = SAR::Find("m_szMapname");
		if (ldg.Found && mpn.Found) {
			LoadGame = *reinterpret_cast<bool**>(ldg.Address);
			Mapname = reinterpret_cast<char**>(mpn.Address);
		}
	}
}