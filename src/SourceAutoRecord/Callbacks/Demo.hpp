#pragma once
#include "Modules/ConCommandArgs.hpp"
#include "Modules/Console.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"

#include "Features/Demo.hpp"

#include "Utils.hpp"

namespace Callbacks
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
			if (DemoPlayer::DemoName[0] != '\0') {
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
		if (demo.Parse(Engine::GetDir() + std::string("/") + name, sar_time_demo_dev.GetInt())) {
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
		std::string dir = Engine::GetDir() + std::string("/");
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