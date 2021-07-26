#include "Timer.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Session.hpp"
#include "Features/Stats/Stats.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "PauseTimer.hpp"
#include "TimerAverage.hpp"
#include "TimerCheckPoints.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

Variable sar_timer_always_running("sar_timer_always_running", "1", "Timer will save current value when disconnecting.\n");
Variable sar_timer_time_pauses("sar_timer_time_pauses", "1", "Timer adds non-simulated ticks when server pauses.\n");

Timer *timer;

Timer::Timer()
	: isRunning(false)
	, isPaused(true)
	, baseTick(0)
	, totalTicks(0)
	, avg(new TimerAverage())
	, cps(new TimerCheckPoints()) {
	this->hasLoaded = this->avg && this->cps;
}
Timer::~Timer() {
	SAFE_DELETE(this->avg)
	SAFE_DELETE(this->cps)
}
void Timer::Start(int engineTick) {
	this->isRunning = true;
	this->isPaused = false;
	this->baseTick = engineTick;
	this->totalTicks = 0;
}
void Timer::Rebase(int engineTick) {
	if (this->isRunning) {
		this->baseTick = engineTick;
		this->isPaused = false;
	}
}
int Timer::GetTick(int engineTick) {
	if (!this->isRunning) {
		return this->totalTicks;
	}

	auto tick = engineTick - this->baseTick;
	return (tick >= 0) ? tick + this->totalTicks : this->totalTicks;
}
void Timer::Save(int engineTick) {
	this->isPaused = true;
	this->totalTicks = this->GetTick(engineTick);
}
void Timer::Stop(int engineTick) {
	this->Save(engineTick);
	this->isRunning = false;
}

// Commands

CON_COMMAND(sar_timer_start, "sar_timer_start - starts timer\n") {
	if (timer->isRunning) {
		console->DevMsg("Restarting timer!\n");
	} else {
		console->DevMsg("Starting timer!\n");
	}

	timer->Start(engine->GetTick());

	if (sar_stats_auto_reset.GetInt() >= 2) {
		stats->ResetAll();
	}
}
CON_COMMAND(sar_timer_stop, "sar_timer_stop - stops timer\n") {
	if (!timer->isRunning) {
		return console->DevMsg("Timer isn't running!\n");
	}

	timer->Stop(engine->GetTick());

	if (timer->avg->isEnabled) {
		auto tick = timer->GetTick(engine->GetTick());
		timer->avg->Add(tick, engine->ToTime(tick), engine->GetCurrentMapName());
	}
}
CON_COMMAND(sar_timer_result, "sar_timer_result - prints result of timer\n") {
	auto tick = timer->GetTick(engine->GetTick());
	auto time = engine->ToTime(tick);

	if (timer->isRunning) {
		console->PrintActive("Result: %i (%.3f)\n", tick, time);
	} else {
		console->Print("Result: %i (%.3f)\n", tick, time);
	}
}
CON_COMMAND(sar_avg_start, "sar_avg_start - starts calculating the average when using timer\n") {
	timer->avg->Start();
}
CON_COMMAND(sar_avg_stop, "sar_avg_stop - stops average calculation\n") {
	timer->avg->isEnabled = false;
}
CON_COMMAND(sar_avg_result, "sar_avg_result - prints result of average\n") {
	auto average = timer->avg->items.size();
	if (!average) {
		return console->Print("No result!\n");
	}

	console->Print("Average of %i:\n", average);
	for (size_t i = 0; i < average; ++i) {
		console->Print("%s -> ", timer->avg->items[i].map);
		console->Print("%i ticks", timer->avg->items[i].ticks);
		console->Print("(%.3f)\n", timer->avg->items[i].time);
	}

	if (timer->isRunning) {
		console->PrintActive("Result: %i (%.3f)\n", timer->avg->averageTicks, timer->avg->averageTime);
	} else {
		console->Print("Result: %i (%.3f)\n", timer->avg->averageTicks, timer->avg->averageTime);
	}
}
CON_COMMAND(sar_cps_add, "sar_cps_add - saves current time of timer\n") {
	if (!timer->isRunning) {
		return console->DevMsg("Timer isn't running!\n");
	}

	auto tick = timer->GetTick(session->GetTick());
	timer->cps->Add(tick, engine->ToTime(tick), engine->GetCurrentMapName());
}
CON_COMMAND(sar_cps_clear, "sar_cps_clear - resets saved times of timer\n") {
	timer->cps->Reset();
}
CON_COMMAND(sar_cps_result, "sar_cps_result - prints result of timer checkpoints\n") {
	auto cps = timer->cps->items.size();
	if (cps > 0) {
		console->Print("Result of %i checkpoint%s:\n", cps, (cps == 1) ? "" : "s");
	} else {
		return console->Print("No result!\n");
	}

	for (size_t i = 0; i < cps; ++i) {
		if (i == cps - 1 && timer->isRunning) {
			console->PrintActive("%s -> ", timer->cps->items[i].map);
			console->PrintActive("%i ticks", timer->cps->items[i].ticks);
			console->PrintActive("(%.3f)\n", timer->cps->items[i].time);
		} else {
			console->Print("%s -> ", timer->cps->items[i].map);
			console->Print("%i ticks", timer->cps->items[i].ticks);
			console->Print("(%.3f)\n", timer->cps->items[i].time);
		}
	}

	if (!timer->isRunning) {
		auto tick = timer->GetTick(engine->GetTick());
		auto time = engine->ToTime(tick);
		console->Print("Result: %i (%.3f)\n", tick, time);
	}
}

// HUD

HUD_ELEMENT2(timer, "0", "Draws current value of timer.\n", HudType_InGame | HudType_Paused) {
	auto tick = (!timer->isPaused) ? timer->GetTick(engine->GetTick()) : timer->totalTicks;
	auto time = engine->ToTime(tick);
	ctx->DrawElement("timer: %i (%.3f)", tick, time);
}
HUD_ELEMENT2(avg, "0", "Draws calculated average of timer.\n", HudType_InGame | HudType_Paused) {
	ctx->DrawElement("avg: %i (%.3f)", timer->avg->averageTicks, timer->avg->averageTime);
}
HUD_ELEMENT2(cps, "0", "Draws latest checkpoint of timer.\n", HudType_InGame | HudType_Paused) {
	ctx->DrawElement("last cp: %i (%.3f)", timer->cps->latestTick, timer->cps->latestTime);
}

ON_EVENT(PRE_TICK) {
	if (engine->IsOrange() || engine->demoplayer->IsPlaying()) {
		return;
	}

	if (session->isRunning && pauseTimer->IsActive() && timer->isRunning && sar_timer_time_pauses.GetBool()) {
		++timer->totalTicks;
	}
}
