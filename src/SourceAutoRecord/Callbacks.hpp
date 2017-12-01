#pragma once
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Console.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Demo.hpp"
#include "Utils.hpp"

namespace Callbacks
{
	void PrintCurrentTick()
	{
		Console::Msg("Current Tick: %i\n", Engine::GetCurrentTick());
	}
	void PrintAbout()
	{
		Console::Msg("SourceAutoRecord allows you to record demos after loading from a save.\n");
		Console::Msg("Plugin is open-source at: https://github.com/NeKzor/SourceAutoRecord\n");
		Console::Msg("Version: %s\n", SAR_VERSION);
		Console::Msg("Build: %s\n", SAR_BUILD);
	}
	void PrintDemoTime()
	{
		if (Recorder::DemoName[0] == '\0' || Recorder::LastDemo.empty()) {
			Console::Msg("No demo was recorded!\n");
			return;
		}

		std::string file = Engine::GetDir() + std::string("\\") + Recorder::LastDemo;
		Console::DevMsg("Parsing %s\n", file.c_str());

		Demo demo;
		if (demo.Parse(file, false)) {
			Console::Msg("Demo: %s\nTicks: %i\n", Recorder::LastDemo.c_str(), demo.GetLastTick());
		}
		else {
			Console::Msg("Parsing failed!\n");
		}
	}
	void SetSaveRebind(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 2) {
			Console::Msg("Usage: <key> <name_of_save>\n");
			return;
		}
		// TODO: test if valid key
		Console::Msg("%s!\n", args.FullArgs());
	}
	void SetReloadRebind(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 2) {
			Console::Msg("Usage: <key> <name_of_save>\n");
			return;
		}
		// TODO: test if valid key
		Console::Msg("%s!\n", args.FullArgs());
	}
}