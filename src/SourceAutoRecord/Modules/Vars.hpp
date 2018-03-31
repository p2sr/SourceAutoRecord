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

	bool* m_bLoadgame;
	char** m_szLevelName;

	void Hook()
	{
		auto ldg = SAR::Find("m_bLoadgame");
		if (ldg.Found) {
			m_bLoadgame = *reinterpret_cast<bool**>(ldg.Address);
		}
	}
	void Unhook()
	{
		if (m_bLoadgame) {
			m_bLoadgame = nullptr;
		}
	}
}