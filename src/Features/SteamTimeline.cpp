#include "SteamTimeline.hpp"

#include "Event.hpp"
#include "Modules/SteamAPI.hpp"
#include "Speedrun/SpeedrunTimer.hpp"

Timeline *timeline;

Timeline::Timeline() {
	this->hasLoaded = true;
}

static std::chrono::time_point<std::chrono::system_clock> g_speedrunStart;

void Timeline::StartSpeedrun() {
	if (!steam->hasLoaded) return;
	g_speedrunStart = std::chrono::system_clock::now();

	steam->g_timeline->AddTimelineEvent("steam_timer", "Speedrun Start", "", 1, 0.0f, 0.0f, k_ETimelineEventClipPriority_Standard);
}

void Timeline::Split(std::string name, std::string time) {
	if (!steam->hasLoaded) return;
	steam->g_timeline->AddTimelineEvent("steam_bolt", name.c_str(), time.c_str(), 0, 0.0f, 0.0f, k_ETimelineEventClipPriority_None);
}

ON_EVENT(SESSION_START) {
	if (!steam->hasLoaded) return;
	steam->g_timeline->SetTimelineGameMode(k_ETimelineGameMode_Playing);
}

ON_EVENT(SESSION_END) {
	if (!steam->hasLoaded) return;
	steam->g_timeline->SetTimelineGameMode((event.load || event.transition) ? k_ETimelineGameMode_LoadingScreen : k_ETimelineGameMode_Menus);
}

ON_EVENT(SPEEDRUN_FINISH) {
	if (!steam->hasLoaded) return;
	std::chrono::duration<float> offset = std::chrono::system_clock::now() - g_speedrunStart;

	auto fl_time = SpeedrunTimer::GetTotalTicks() / 60.0f;
	auto time = SpeedrunTimer::Format(fl_time);

	steam->g_timeline->SetTimelineStateDescription(("Speedrun " + time).c_str(), -offset.count());
	steam->g_timeline->ClearTimelineStateDescription(0.0f);
	steam->g_timeline->AddTimelineEvent("steam_flag", "Speedrun Finish", time.c_str(), 1, 0.0f, 0.0f, k_ETimelineEventClipPriority_Standard);
}

ON_EVENT(MAYBE_AUTOSUBMIT) {
	if (!steam->hasLoaded) return;
	if (!event.pb)
		return;

	steam->g_timeline->AddTimelineEvent("steam_crown", "Personal Best", SpeedrunTimer::Format(event.score / 100.0f).c_str(), 2, 0.0f, 0.0f, k_ETimelineEventClipPriority_Featured);
}
