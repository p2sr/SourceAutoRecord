#pragma once
#include "Modules/Client.hpp"
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Console.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"

#include "Demo.hpp"
#include "Rebinder.hpp"
#include "Utils.hpp"

namespace Callbacks
{
	void PrintSessionTick()
	{
		Console::Msg("Session Tick: %i\n", Engine::GetCurrentTick());
		Console::Msg("Recorder Tick: %i\n", DemoRecorder::GetCurrentTick());
	}
	void PrintAbout()
	{
		Console::Msg("SourceAutoRecord allows you to record demos after loading from a save.\n");
		Console::Msg("Plugin is open-source at: https://github.com/NeKzor/SourceAutoRecord\n");
		Console::Msg("Game: %s\n", Offsets::GetVariant());
		Console::Msg("Version: %s\n", SAR_VERSION);
		Console::Msg("Build: %s\n", SAR_BUILD);
	}
	void PrintDemoInfo(const void* ptr)
	{
		ConCommandArgs args(ptr);
		if (args.Count() != 2) {
			Console::Msg("sar_time_demo [demo_name] : Parses a demo and prints some information about it.\n");
			return;
		}

		std::string name;
		if (args.At(1)[0] == '\0') {
			if (DemoRecorder::DemoName[0] != '\0' && !DemoRecorder::LastDemo.empty()) {
				name = DemoRecorder::LastDemo;
			}
			else if (DemoPlayer::DemoName[0] != '\0') {
				name = std::string(DemoPlayer::DemoName);
			}
			else {
				Console::Msg("No demo was recorded or played back!\n");
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
			Console::Msg("Demo: %s\n", name.c_str());
			Console::Msg("Client: %s\n", demo.clientName);
			Console::Msg("Map: %s\n", demo.mapName);
			Console::Msg("Ticks: %i\n", demo.playbackTicks);
			Console::Msg("Time: %.3f\n", demo.playbackTime);
			Console::Msg("IpT: %.6f\n", demo.IntervalPerTick());
		}
		else {
			Console::Msg("Could not parse \"%s\"!\n", name.c_str());
		}
	}
	void BindSaveRebinder(const void* ptr)
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
	void BindReloadRebinder(const void* ptr)
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
	void UnbindSaveRebinder()
	{
		if (!Rebinder::IsSaveBinding) {
			Console::Msg("There's nothing to unbind.\n");
			return;
		}
		Rebinder::ResetSaveBind();
	}
	void UnbindReloadRebinder()
	{
		if (!Rebinder::IsReloadBinding) {
			Console::Msg("There's nothing to unbind.\n");
			return;
		}
		Rebinder::ResetReloadBind();
	}
}