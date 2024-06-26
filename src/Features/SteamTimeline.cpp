#include "SteamTimeline.hpp"

#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Speedrun/SpeedrunTimer.hpp"

#include <steam_api.h>

Timeline *timeline;

Timeline::Timeline() {
	this->hasLoaded = true;

	SteamTimeline()->SetTimelineGameMode(k_ETimelineGameMode_Menus);
}

static std::chrono::time_point<std::chrono::system_clock> g_speedrunStart;

void Timeline::StartSpeedrun() {
	g_speedrunStart = std::chrono::system_clock::now();

	SteamTimeline()->AddTimelineEvent("steam_timer", "Speedrun Start", "", 1, 0.0f, 0.0f, k_ETimelineEventClipPriority_Standard);
}

void Timeline::Split(std::string name, std::string time) {
	SteamTimeline()->AddTimelineEvent("steam_bolt", name.c_str(), time.c_str(), 0, 0.0f, 0.0f, k_ETimelineEventClipPriority_None);
}

ON_EVENT(SESSION_START) {
	SteamTimeline()->SetTimelineGameMode(k_ETimelineGameMode_Playing);
}

ON_EVENT(SESSION_END) {
	SteamTimeline()->SetTimelineGameMode((event.load || event.transition) ? k_ETimelineGameMode_LoadingScreen : k_ETimelineGameMode_Menus);
}

ON_EVENT(SPEEDRUN_FINISH) {
	std::chrono::duration<float> offset = std::chrono::system_clock::now() - g_speedrunStart;

	auto fl_time = SpeedrunTimer::GetTotalTicks() / 60.0f;
	auto time = SpeedrunTimer::Format(fl_time);

	SteamTimeline()->SetTimelineStateDescription(("Speedrun " + time).c_str(), -offset.count());
	SteamTimeline()->ClearTimelineStateDescription(0.0f);
	SteamTimeline()->AddTimelineEvent("steam_flag", "Speedrun Finish", time.c_str(), 1, 0.0f, 0.0f, k_ETimelineEventClipPriority_Standard);
}

ON_EVENT(MAYBE_AUTOSUBMIT) {
	if (!event.pb)
		return;

	SteamTimeline()->AddTimelineEvent("steam_crown", "Personal Best", SpeedrunTimer::Format(event.score / 100.0f).c_str(), 2, 0.0f, 0.0f, k_ETimelineEventClipPriority_Featured);
}
