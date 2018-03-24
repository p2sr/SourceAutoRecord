#pragma once
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "SourceAutoRecord.hpp"
#include "Utils.hpp"

namespace Vars
{
	int* tickcount;
	float* interval_per_tick;

	using _GetGameDirectory = char*(__cdecl*)();
	_GetGameDirectory GetGameDirectory;

	bool* LoadGame;
	char** Mapname;

	void Hook()
	{
		auto ldg = SAR::Find("m_bLoadgame");
		auto mpn = SAR::Find("m_szMapname");
		if (ldg.Found && mpn.Found) {
			LoadGame = *reinterpret_cast<bool**>(ldg.Address);
			Mapname = reinterpret_cast<char**>(mpn.Address);
		}
	}
}