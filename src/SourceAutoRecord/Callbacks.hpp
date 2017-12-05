#pragma once
#include "Modules/Client.hpp"
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Console.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"

#include "Demo.hpp"
#include "Rebinder.hpp"
#include "Summary.hpp"
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
		Console::Msg("Variant: %s\n", Offsets::GetStringVariant());
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

		std::string file = Engine::GetDir() + "\\" + name;
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
	void PrintDemoInfos(const void* ptr)
	{
		ConCommandArgs args(ptr);

		int totalTicks = 0;
		float totalTime = 0;
		bool printTotal = false;

		std::string name;
		for (size_t i = 1; i < args.Count(); i++)
		{
			name = std::string(args.At(i));

			std::string file = Engine::GetDir() + "\\" + name;// ".dem";
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
				Console::Msg("---------------\n");
				totalTicks += demo.playbackTicks;
				totalTime += demo.playbackTime;
				printTotal = true;
			}
			else {
				Console::Msg("Could not parse \"%s\"!\n", name.c_str());
			}
		}
		if (printTotal) {
			Console::Msg("Total Ticks: %i\n", totalTicks);
			Console::Msg("Total Time: %.3f\n", totalTime);
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
	void StartSummary()
	{
		if (Summary::HasStarted) {
			Console::Msg("Summary has already started!\n");
			return;
		}
		Summary::Start();
	}
	void ResetSummary()
	{
		if (!Summary::HasStarted) {
			Console::Msg("There's no summary to reset!\n");
			return;
		}
		Summary::Reset();
	}
	void PrintSummary()
	{
		if (!Summary::HasStarted || Summary::Items.size() == 0) {
			Console::Msg("There is no result for a summary!\n");
			return;
		}

		Console::Msg("Summary of %i sessions:\n", Summary::Items.size());
		for (size_t i = 0; i < Summary::Items.size(); i++) {
			Console::Msg("%s -> ", Summary::Items[i].Map);
			Console::Msg("%i ticks", Summary::Items[i].Ticks);
			Console::Msg("(%.3fs)\n", Summary::Items[i].Time);
		}
		Console::Msg("---------------\n");
		Console::Msg("Total Ticks: %i\n", Summary::TotalTicks);
		Console::Msg("Total Time: %.3f\n", Summary::TotalTime);
	}
}