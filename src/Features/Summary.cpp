#include "Summary.hpp"

#include "Command.hpp"
#include "Features/Session.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Variable.hpp"

Variable sar_sum_during_session("sar_sum_during_session", "1", "Updates the summary counter automatically during a session.\n");

Summary *summary;

Summary::Summary()
	: isRunning(false)
	, items()
	, totalTicks(0) {
	this->hasLoaded = true;
}
void Summary::Start() {
	this->items.clear();
	this->totalTicks = 0;
	this->isRunning = true;
}
void Summary::Add(int ticks, float time, std::string map) {
	this->items.push_back(SummaryItem{
		ticks,
		time,
		map});
	this->totalTicks += ticks;
}

CON_COMMAND(sar_sum_here, "sar_sum_here - starts counting total ticks of sessions\n") {
	if (summary->isRunning) {
		return console->Print("Summary has already started!\n");
	}

	summary->Start();
}
CON_COMMAND(sar_sum_stop, "sar_sum_stop - stops summary counter\n") {
	if (!summary->isRunning) {
		return console->Print("There's no summary to stop!\n");
	}

	if (sar_sum_during_session.GetBool()) {
		auto tick = session->GetTick();
		summary->Add(tick, engine->ToTime(tick), engine->GetCurrentMapName().c_str());
	}

	summary->isRunning = false;
}
CON_COMMAND(sar_sum_result, "sar_sum_result - prints result of summary\n") {
	auto sessions = summary->items.size();
	if (summary->isRunning && sessions == 0) {
		console->Print("Summary of this session:\n");
	} else if (summary->isRunning && sessions > 0) {
		console->Print("Summary of %i sessions:\n", sessions + 1);
	} else if (sessions > 0) {
		console->Print("Summary of %i session%s:\n", sessions, (sessions == 1) ? "" : "s");
	} else {
		return console->Print("There's no result of a summary!\n");
	}

	for (size_t i = 0; i < summary->items.size(); ++i) {
		console->Print("%s -> ", summary->items[i].map);
		console->Print("%i ticks", summary->items[i].ticks);
		console->Print("(%.3f)\n", summary->items[i].time);
	}

	auto totalTime = engine->ToTime(summary->totalTicks);
	if (summary->isRunning) {
		auto tick = session->GetTick();
		auto time = engine->ToTime(tick);
		console->PrintActive("%s -> ", engine->GetCurrentMapName().c_str());
		console->PrintActive("%i ticks ", tick);
		console->PrintActive("(%.3f)\n", time);
		console->Print("---------------\n");
		console->Print("Total Ticks: %i ", summary->totalTicks);
		console->PrintActive("(%i)\n", summary->totalTicks + tick);
		console->Print("Total Time: %.3f ", totalTime);
		console->PrintActive("(%.3f)\n", totalTime + time);
	} else {
		console->Print("---------------\n");
		console->Print("Total Ticks: %i\n", summary->totalTicks);
		console->Print("Total Time: %.3f\n", totalTime);
	}
}
