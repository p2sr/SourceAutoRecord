#pragma once
#include "Modules/Console.hpp"

#include "Features/Config.hpp"

namespace Callbacks
{
	void SaveCvars()
	{
		if (!Config::Save()) {
			Console::Print("Failed to create config file!\n");
		}
		else {
			Console::Print("Saved important settings in /cfg/_sar_cvars.cfg!\n");
		}
	}
	void LoadCvars()
	{
		if (!Config::Load()) {
			Console::Print("Config file not found!\n");
		}
	}
}