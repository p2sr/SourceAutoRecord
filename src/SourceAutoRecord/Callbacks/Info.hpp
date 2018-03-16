#pragma once
#include "Modules/Console.hpp"
#include "Modules/DemoRecorder.hpp"
#include "Modules/Engine.hpp"

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
}