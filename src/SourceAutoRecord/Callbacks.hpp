#pragma once
#include "Modules/Client.hpp"
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Console.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"
#include "Modules/InputSystem.hpp"

#include "Demo.hpp"
#include "Rebinder.hpp"
#include "Stats.hpp"
#include "Summary.hpp"
#include "Teleporter.hpp"
#include "Timer.hpp"
#include "TimerAverage.hpp"
#include "TimerCheckPoints.hpp"
#include "Utils.hpp"

namespace Callbacks
{
	void PrintSession()
	{
		int tick = Engine::GetTick();
		Console::Print("Session Tick: %i (%.3f)\n", tick, tick * *Engine::IntervalPerTick);
		if (*DemoRecorder::Recording) {
			tick = DemoRecorder::GetTick();
			Console::Print("Demo Recorder Tick: %i (%.3f)\n", tick, tick * *Engine::IntervalPerTick);
		}
		if (DemoPlayer::IsPlaying()) {
			tick = DemoPlayer::GetTick();
			Console::Print("Demo Player Tick: %i (%.3f)\n", tick, tick * *Engine::IntervalPerTick);
		}
	}
	void PrintAbout()
	{
		Console::Print("SourceAutoRecord tells the engine to keep recording when loading a save.\n");
		Console::Print("More information at: https://nekzor.github.io/SourceAutoRecord\n");
		Console::Print("Game: %s\n", Game::GetVersion());
		Console::Print("Version: %s\n", SAR_VERSION);
		Console::Print("Build: %s\n", SAR_BUILD);
	}
	// Demo parsing
	namespace
	{
		void PrintDemoInfo(const void* ptr)
		{
			ConCommandArgs args(ptr);
			if (args.Count() != 2) {
				Console::Print("sar_time_demo [demo_name] : Parses a demo and prints some information about it.\n");
				return;
			}

			std::string name;
			if (args.At(1)[0] == '\0') {
				if (!DemoRecorder::LastDemo.empty()) {
					name = DemoRecorder::LastDemo;
				}
				else if (DemoPlayer::DemoName[0] != '\0') {
					name = std::string(DemoPlayer::DemoName);
				}
				else {
					Console::Print("No demo was recorded or played back!\n");
					return;
				}
			}
			else {
				name = std::string(args.At(1));
			}

			Demo demo;
			if (demo.Parse(Engine::GetDir() + std::string("\\") + name, sar_time_demo_dev.GetInt())) {
				demo.Fix();
				Console::Print("Demo: %s\n", name.c_str());
				Console::Print("Client: %s\n", demo.clientName);
				Console::Print("Map: %s\n", demo.mapName);
				Console::Print("Ticks: %i\n", demo.playbackTicks);
				Console::Print("Time: %.3f\n", demo.playbackTime);
				Console::Print("IpT: %.6f\n", demo.IntervalPerTick());
			}
			else {
				Console::Print("Could not parse \"%s\"!\n", name.c_str());
			}
		}
		void PrintDemoInfos(const void* ptr)
		{
			ConCommandArgs args(ptr);
			if (args.Count() <= 1) {
				Console::Print("sar_time_demos [demo_name] [demo_name2] [etc.] : Parses multiple demos and prints the total sum of them.\n");
				return;
			}

			int totalTicks = 0;
			float totalTime = 0;
			bool printTotal = false;

			std::string name;
			std::string dir = Engine::GetDir() + std::string("\\");
			for (int i = 1; i < args.Count(); i++)
			{
				name = std::string(args.At(i));

				Demo demo;
				if (demo.Parse(dir + name)) {
					demo.Fix();
					Console::Print("Demo: %s\n", name.c_str());
					Console::Print("Client: %s\n", demo.clientName);
					Console::Print("Map: %s\n", demo.mapName);
					Console::Print("Ticks: %i\n", demo.playbackTicks);
					Console::Print("Time: %.3f\n", demo.playbackTime);
					Console::Print("IpT: %.6f\n", demo.IntervalPerTick());
					Console::Print("---------------\n");
					totalTicks += demo.playbackTicks;
					totalTime += demo.playbackTime;
					printTotal = true;
				}
				else {
					Console::Print("Could not parse \"%s\"!\n", name.c_str());
				}
			}
			if (printTotal) {
				Console::Print("Total Ticks: %i\n", totalTicks);
				Console::Print("Total Time: %.3f\n", totalTime);
			}
		}
	}
	// Rebinder
	namespace
	{
		void BindSaveRebinder(const void* ptr)
		{
			ConCommandArgs args(ptr);
			if (args.Count() != 3) {
				Console::Print("sar_bind_save <key> [save_name] : Automatic save rebinding when server has loaded. File indexing will be synced when recording demos.\n");
				return;
			}

			int button = InputSystem::StringToButtonCode(args.At(1));
			if (button == BUTTON_CODE_INVALID) {
				Console::Print("\"%s\" isn't a valid key!\n", args.At(1));
				return;
			}
			else if (button == KEY_ESCAPE) {
				Console::Print("Can't bind ESCAPE key!\n", args.At(1));
				return;
			}

			if (Rebinder::IsReloadBinding && button == Rebinder::ReloadButton) {
				Rebinder::ResetReloadBind();
			}

			Rebinder::SetSaveBind(button, args.At(2));
			Rebinder::RebindSave();
		}
		void BindReloadRebinder(const void* ptr)
		{
			ConCommandArgs args(ptr);
			if (args.Count() != 3) {
				Console::Print("sar_bind_reload <key> [save_name] : Automatic save-reload rebinding when server has loaded. File indexing will be synced when recording demos.\n");
				return;
			}

			int button = InputSystem::StringToButtonCode(args.At(1));
			if (button == BUTTON_CODE_INVALID) {
				Console::Print("\"%s\" isn't a valid key!\n", args.At(1));
				return;
			}
			else if (button == KEY_ESCAPE) {
				Console::Print("Can't bind ESCAPE key!\n", args.At(1));
				return;
			}

			if (Rebinder::IsSaveBinding && button == Rebinder::SaveButton) {
				Rebinder::ResetSaveBind();
			}

			Rebinder::SetReloadBind(button, args.At(2));
			Rebinder::RebindReload();
		}
		void UnbindSaveRebinder()
		{
			if (!Rebinder::IsSaveBinding) {
				Console::Print("There's nothing to unbind.\n");
				return;
			}
			Rebinder::ResetSaveBind();
		}
		void UnbindReloadRebinder()
		{
			if (!Rebinder::IsReloadBinding) {
				Console::Print("There's nothing to unbind.\n");
				return;
			}
			Rebinder::ResetReloadBind();
		}
	}
	// Summary
	namespace
	{
		void StartSummary()
		{
			if (Summary::IsRunning) {
				Console::Print("Summary has already started!\n");
				return;
			}
			Summary::Start();
		}
		void StopSummary()
		{
			if (!Summary::IsRunning) {
				Console::Print("There's no summary to stop!\n");
				return;
			}

			if (sar_sum_during_session.GetBool()) {
				int tick = Engine::GetTick();
				Summary::Add(tick, tick * *Engine::IntervalPerTick, *Engine::Mapname);
			}
			Summary::IsRunning = false;
		}
		void PrintSummary()
		{
			int sessions = Summary::Items.size();
			if (Summary::IsRunning && sessions == 0) {
				Console::Print("Summary of this session:\n");
			}
			else if (Summary::IsRunning && sessions > 0) {
				Console::Print("Summary of %i sessions:\n", sessions + 1);
			}
			else if (sessions > 0) {
				Console::Print("Summary of %i session%s:\n", sessions, (sessions == 1) ? "" : "s");
			}
			else {
				Console::Print("There's no result of a summary!\n");
				return;
			}

			for (size_t i = 0; i < Summary::Items.size(); i++) {
				Console::Print("%s -> ", Summary::Items[i].Map);
				Console::Print("%i ticks", Summary::Items[i].Ticks);
				Console::Print("(%.3f)\n", Summary::Items[i].Time);
			}

			if (Summary::IsRunning) {
				int tick = Engine::GetTick();
				float time = tick * *Engine::IntervalPerTick;
				Console::PrintActive("%s -> ", *Engine::Mapname);
				Console::PrintActive("%i ticks ", tick);
				Console::PrintActive("(%.3f)\n", time);
				Console::Print("---------------\n");
				Console::Print("Total Ticks: %i ", Summary::TotalTicks);
				Console::PrintActive("(%i)\n", Summary::TotalTicks + tick);
				Console::Print("Total Time: %.3f ", Summary::TotalTime);
				Console::PrintActive("(%.3f)\n", Summary::TotalTime + time);
			}
			else {
				Console::Print("---------------\n");
				Console::Print("Total Ticks: %i\n", Summary::TotalTicks);
				Console::Print("Total Time: %.3f\n", Summary::TotalTime);
			}
		}
	}
	// Timer
	namespace
	{
		void StartTimer()
		{
			if (Timer::IsRunning)
				Console::DevMsg("Restarting timer!\n");
			else
				Console::DevMsg("Starting timer!\n");
			Timer::Start(*Engine::TickCount);

			if (sar_stats_auto_reset.GetInt() >= 2) {
				Stats::Reset();
			}
		}
		void StopTimer()
		{
			if (!Timer::IsRunning) {
				Console::DevMsg("Timer isn't running!\n");
				return;
			}

			Timer::Stop(*Engine::TickCount);

			if (Timer::Average::IsEnabled) {
				int tick = Timer::GetTick(*Engine::TickCount);
				Timer::Average::Add(tick, tick * *Engine::IntervalPerTick, *Engine::Mapname);
			}
		}
		void PrintTimer() {
			int tick = Timer::GetTick(*Engine::TickCount);
			float time = tick * *Engine::IntervalPerTick;

			if (Timer::IsRunning) {
				Console::PrintActive("Result: %i (%.3f)\n", tick, time);
			}
			else {
				Console::Print("Result: %i (%.3f)\n", tick, time);
			}
		}
		void StartAverage() {
			Timer::Average::Start();
		}
		void StopAverage() {
			Timer::Average::IsEnabled = false;
		}
		void PrintAverage() {
			int average = Timer::Average::Items.size();
			if (average > 0) {
				Console::Print("Average of %i:\n", average);
			}
			else {
				Console::Print("No result!\n");
				return;
			}

			for (int i = 0; i < average; i++) {
				Console::Print("%s -> ", Timer::Average::Items[i].Map);
				Console::Print("%i ticks", Timer::Average::Items[i].Ticks);
				Console::Print("(%.3f)\n", Timer::Average::Items[i].Time);
			}

			if (Timer::IsRunning) {
				Console::PrintActive("Result: %i (%.3f)\n", Timer::Average::AverageTicks, Timer::Average::AverageTime);
			}
			else {
				Console::Print("Result: %i (%.3f)\n", Timer::Average::AverageTicks, Timer::Average::AverageTime);
			}
		}
		void AddCheckpoint() {
			if (!Timer::IsRunning) {
				Console::DevMsg("Timer isn't running!\n");
				return;
			}

			int tick = Timer::GetTick(Engine::GetTick());
			Timer::CheckPoints::Add(tick, tick * *Engine::IntervalPerTick, *Engine::Mapname);
		}
		void ClearCheckpoints() {
			Timer::CheckPoints::Reset();
		}
		void PrintCheckpoints() {
			int cps = Timer::CheckPoints::Items.size();
			if (cps > 0) {
				Console::Print("Result of %i checkpoint%s:\n", cps, (cps == 1) ? "" : "s");
			}
			else {
				Console::Print("No result!\n");
				return;
			}

			for (int i = 0; i < cps; i++) {
				if (i == cps - 1 && Timer::IsRunning) {
					Console::PrintActive("%s -> ", Timer::CheckPoints::Items[i].Map);
					Console::PrintActive("%i ticks", Timer::CheckPoints::Items[i].Ticks);
					Console::PrintActive("(%.3f)\n", Timer::CheckPoints::Items[i].Time);
				}
				else {
					Console::Print("%s -> ", Timer::CheckPoints::Items[i].Map);
					Console::Print("%i ticks", Timer::CheckPoints::Items[i].Ticks);
					Console::Print("(%.3f)\n", Timer::CheckPoints::Items[i].Time);
				}
			}

			if (!Timer::IsRunning) {
				int tick = Timer::GetTick(*Engine::TickCount);
				float time = tick * *Engine::IntervalPerTick;
				Console::Print("Result: %i (%.3f)\n", tick, time);
			}
		}
	}
	// Stats
	namespace
	{
		void ResetJumps()
		{
			Stats::TotalJumps = 0;
		}
		void ResetUses()
		{
			Stats::TotalUses = 0;
		}
	}
	// Cheats
	namespace
	{
		void Teleport()
		{
			if (sv_cheats.GetBool()) {
				if (Teleporter::IsSet) {
					Engine::ExecuteCommand(Teleporter::GetSetpos().c_str());
					Engine::ExecuteCommand(Teleporter::GetSetang().c_str());
				}
				else {
					Console::Print("Location not set. Use sar_set_teleport.\n");
				}
			}
			else {
				Console::Print("Cannot teleport without sv_cheats 1.\n");
			}
		}
		void SetTeleport()
		{
			Teleporter::Save();
		}
	}
}