#include "SteamTimeline.hpp"

#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Modules/SteamAPI.hpp"
#include "Speedrun/SpeedrunTimer.hpp"

/* Timeline can get cluttered up (especially at long recording lengths), nice to have an option to disable adding splits. */
Variable sar_timeline_splits("sar_timeline_splits", "1", "Add split markers to the Steam Timeline.\n");
Variable sar_timeline_show_completed("sar_timeline_show_completed", "0", "Only show speedrun starts and splits with matching finishes. Set to 2 to only show Personal Bests.\n");

Timeline *timeline;

Timeline::Timeline() {
	this->hasLoaded = true;
}

static std::chrono::time_point<std::chrono::system_clock> g_speedrunStart;
static std::vector<std::tuple<std::string, std::string, float>> g_pendingSplits;
static float g_pendingPbFinishOffset;
static std::string g_pendingPbFinishTime;
static std::chrono::time_point<std::chrono::system_clock> g_pendingPbSpeedrunStart;
static std::vector<std::tuple<std::string, std::string, float>> g_pendingPbSplits;
static bool g_hasPendingPbRun = false;

void Timeline::StartSpeedrun() {
	if (!steam->hasLoaded) return;
	g_speedrunStart = std::chrono::system_clock::now();
	g_pendingSplits.clear();

	if (sar_timeline_show_completed.GetBool()) return;
	steam->g_timeline->AddTimelineEvent("steam_timer", "Speedrun Start", "", 1, 0.0f, 0.0f, k_ETimelineEventClipPriority_Standard);
}

void Timeline::Split(std::string name, std::string time) {
	if (!steam->hasLoaded) return;
	std::chrono::duration<float> currentOffset = std::chrono::system_clock::now() - g_speedrunStart;
	if (sar_timeline_splits.GetBool()) {
		if (sar_timeline_show_completed.GetBool()) {
			g_pendingSplits.push_back({name, time, currentOffset.count()});
		} else {
			steam->g_timeline->AddTimelineEvent("steam_bolt", name.c_str(), time.c_str(), 0, 0.0f, 0.0f, k_ETimelineEventClipPriority_None);
		}
	}

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

	auto fl_time = SpeedrunTimer::GetTotalTicks() * engine->GetIPT();
	auto time = SpeedrunTimer::Format(fl_time);

	if (sar_timeline_show_completed.GetInt() >= 2) {
		g_pendingPbFinishOffset = offset.count();
		g_pendingPbFinishTime = time;
		g_pendingPbSpeedrunStart = g_speedrunStart;
		g_pendingPbSplits = g_pendingSplits;
		g_hasPendingPbRun = true;
		g_pendingSplits.clear();
		return;
	}

	if (sar_timeline_show_completed.GetBool()) {
		steam->g_timeline->AddTimelineEvent("steam_timer", "Speedrun Start", "", 1, -offset.count(), 0.0f, k_ETimelineEventClipPriority_Standard);

		for (const auto &[splitName, splitTime, splitOffset] : g_pendingSplits) {
			steam->g_timeline->AddTimelineEvent(
				"steam_bolt",
				splitName.c_str(),
				splitTime.c_str(),
				0,
				splitOffset - offset.count(),
				0.0f,
				k_ETimelineEventClipPriority_None);
		}
		g_pendingSplits.clear();
	}

	steam->g_timeline->SetTimelineStateDescription(("Speedrun " + time).c_str(), -offset.count());
	steam->g_timeline->ClearTimelineStateDescription(0.0f);
	steam->g_timeline->AddTimelineEvent("steam_flag", "Speedrun Finish", time.c_str(), 1, 0.0f, 0.0f, k_ETimelineEventClipPriority_Standard);
}

ON_EVENT(MAYBE_AUTOSUBMIT) {
	if (!steam->hasLoaded) return;
	if (!event.pb) {
		g_pendingPbSplits.clear();
		g_hasPendingPbRun = false;
		return;
	}

	if (sar_timeline_show_completed.GetInt() >= 2 && g_hasPendingPbRun) {
		std::chrono::duration<float> offset = std::chrono::system_clock::now() - g_pendingPbSpeedrunStart;
		float finishOffset = g_pendingPbFinishOffset - offset.count();

		steam->g_timeline->AddTimelineEvent("steam_timer", "Speedrun Start", "", 1, -offset.count(), 0.0f, k_ETimelineEventClipPriority_Standard);

		for (const auto &[splitName, splitTime, splitOffset] : g_pendingPbSplits) {
			steam->g_timeline->AddTimelineEvent(
				"steam_bolt",
				splitName.c_str(),
				splitTime.c_str(),
				0,
				splitOffset - offset.count(),
				0.0f,
				k_ETimelineEventClipPriority_None);
		}
		g_pendingPbSplits.clear();

		steam->g_timeline->SetTimelineStateDescription(("Speedrun " + g_pendingPbFinishTime).c_str(), -offset.count());
		steam->g_timeline->ClearTimelineStateDescription(finishOffset);
		steam->g_timeline->AddTimelineEvent("steam_crown", "Personal Best", SpeedrunTimer::Format(event.score / 100.0f).c_str(), 2, finishOffset, 0.0f, k_ETimelineEventClipPriority_Featured);
		g_hasPendingPbRun = false;
		return;
	}
	g_pendingPbSplits.clear();
	g_hasPendingPbRun = false;

	steam->g_timeline->AddTimelineEvent("steam_crown", "Personal Best", SpeedrunTimer::Format(event.score / 100.0f).c_str(), 2, 0.0f, 0.0f, k_ETimelineEventClipPriority_Featured);
}
