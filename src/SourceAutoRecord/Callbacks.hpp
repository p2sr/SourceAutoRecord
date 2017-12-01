#pragma once
#include "Modules/Client.hpp"
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"

#include "Demo.hpp"
#include "Rebinder.hpp"
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
		Console::Msg("Game: %s\n", Offsets::GetVariant());
		Console::Msg("Version: %s\n", SAR_VERSION);
		Console::Msg("Build: %s\n", SAR_BUILD);
	}
	void PrintDemoTime(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 2) {
			Console::Msg("sar_time_demo [demo_name] : Parses a demo and prints some information about it.\n");
			return;
		}

		std::string name;
		if (args.At(1)[0] == '\0') {
			if (Recorder::DemoName[0] != '\0' && !Recorder::LastDemo.empty()) {
				name = Recorder::LastDemo;
			}
			else {
				Console::Msg("No demo was recorded!\n");
				return;
			}	
		}
		else {
			name = std::string(args.At(1));
		}

		std::string file = Engine::GetDir() + std::string("\\") + name;
		Console::DevMsg("Trying to parse \"%s\"...\n", file.c_str());

		Demo demo;
		if (demo.Parse(file, false)) {
			demo.Fix();
			Console::Msg("Demo: %s\nClient: %s\nMap: %s\nTicks: %i\nTime: %.3f\nIpT: %.6f\n", name.c_str(), demo.clientName, demo.mapName, demo.playbackTicks, demo.playbackTime, demo.IntervalPerTick());
		}
		else {
			Console::Msg("Could not parse \"%s\"!\n", name.c_str());
		}
	}
	void SetSaveRebind(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 3) {
			Console::Msg("sar_bind_save <key> [save_name] : Automatic save rebinding when server has loaded. File indexing will be synced when recording demos.\n");
			return;
		}

		int button = InputSystem::StringToButtonCode(args.At(1));
		if (button == BUTTON_CODE_INVALID) {
			Console::Msg("\"%s\" isn't a valid key!\n", args.At(1));
			return;
		}
		else if (button == KEY_ESCAPE) {
			Console::Msg("Can't bind ESCAPE key!\n", args.At(1));
			return;
		}
		Rebinder::SetSaveBind(button, args.At(2));
		Rebinder::RebindSave();
	}
	void SetReloadRebind(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 3) {
			Console::Msg("sar_bind_reload <key> [save_name] : Automatic save-reload rebinding when server has loaded. File indexing will be synced when recording demos.\n");
			return;
		}

		int button = InputSystem::StringToButtonCode(args.At(1));
		if (button == BUTTON_CODE_INVALID) {
			Console::Msg("\"%s\" isn't a valid key!\n", args.At(1));
			return;
		}
		else if (button == KEY_ESCAPE) {
			Console::Msg("Can't bind ESCAPE key!\n", args.At(1));
			return;
		}
		Rebinder::SetReloadBind(button, args.At(2));
		Rebinder::RebindReload();
	}
}