#include "SpeedrunTimer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
#	include <io.h>
#endif

#include "Event.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/Hud/Toasts.hpp"
#include "Features/NetMessage.hpp"
#include "Features/Session.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Utils.hpp"

#define SPEEDRUN_PACKET_TYPE "srtimer"
#define SYNC_INTERVAL 60  // Sync every second, just in case

enum PacketType {
	SYNC,
	START,
	PAUSE,
	RESUME,
	STOP,
	SPLIT,
	RESET,
};

// TimerAction {{{

enum class TimerAction {
	NONE,
	START,
	RESTART,
	SPLIT,
	END,
	RESET,
	// The old interface also had Pause and Resume but they were kinda
	// pointless
};

// }}}

// TimerInterface {{{

struct TimerInterface {
	char start[16];
	int total;
	float ipt;
	TimerAction action;
	char end[14];

	TimerInterface();
};

TimerInterface::TimerInterface()
	: start("SAR_TIMER_START")
	, total(0)
	, ipt(0.0f)
	, action(TimerAction::NONE)
	, end("SAR_TIMER_END") {
}

// }}}

// Segment, SplitInfo {{{

struct Segment {
	std::string name;
	int ticks;
};

struct SplitInfo {
	std::vector<Segment> segments;
	std::string name;
	int ticks;
};

// }}}

// This really should just be a plain global rather than being
// heap-allocated; however, since it was previously heap-allocated,
// timers may rely on this for detection, hence we still put it on the
// heap for back compat
static TimerInterface *g_timerInterface;

static struct
{
	bool isRunning;
	bool isPaused;
	bool isReset;

	bool hasSplitLoad;

	int saved;
	int base;

	std::vector<Segment> currentSplit;
	std::vector<SplitInfo> splits;

	std::vector<std::string> visitedMaps;
	std::string lastMap;
} g_speedrun;

static std::map<std::string, int> g_activeRun;
static std::vector<std::map<std::string, int>> g_runs;

static void handleCoopPacket(void *data, size_t size);

void SpeedrunTimer::Init() {
	g_timerInterface = new TimerInterface();
	SpeedrunTimer::Reset(false);
	NetMessage::RegisterHandler(SPEEDRUN_PACKET_TYPE, &handleCoopPacket);
	SpeedrunTimer::InitCategories();
}

void SpeedrunTimer::SetIpt(float ipt) {
	g_timerInterface->ipt = ipt;
}

// Interface action fuckery {{{

static std::chrono::time_point<std::chrono::steady_clock> g_actionResetTime;

static void setTimerAction(TimerAction action) {
	g_timerInterface->action = action;
	g_actionResetTime = NOW_STEADY() + std::chrono::milliseconds(50);  // Bit of a hack - should be enough time for timers to pick up on it
}

// }}}

// Getting time {{{

// Both Orange and Blue - the tick we synced
static int g_coopLastSyncTick;
// Orange only - the tick we synced, as reported by the engine
static int g_coopLastSyncEngineTick;

static void handleCoopPacket(void *data, size_t size) {
	if (!engine->IsOrange()) return;

	char *data_ = (char *)data;

	if (size < 5) return;

	PacketType t = (PacketType)data_[0];
	int tick = *(int *)(data_ + 1);

	g_coopLastSyncTick = tick;
	g_coopLastSyncEngineTick = engine->GetTick();

	g_timerInterface->total = SpeedrunTimer::GetTotalTicks();

	switch (t) {
	case PacketType::SYNC:
		break;
	case PacketType::START:
		SpeedrunTimer::Start();
		break;
	case PacketType::PAUSE:
		SpeedrunTimer::Pause();
		break;
	case PacketType::RESUME:
		SpeedrunTimer::Resume();
		break;
	case PacketType::STOP:
		SpeedrunTimer::Stop(std::string(data_ + 5, size - 5));
		break;
	case PacketType::SPLIT:
		if (size < 6) return;
		SpeedrunTimer::Split(data_[5], std::string(data_ + 6, size - 6));
		break;
	case PacketType::RESET:
		SpeedrunTimer::Reset();
		break;
	}
}

static bool g_inDemoLoad = false;

ON_EVENT_P(SESSION_START, 1000) {
	if (engine->demoplayer->IsPlaying()) g_inDemoLoad = true;
}
ON_EVENT_P(SESSION_START, -1000) {
	g_inDemoLoad = false;
}

static int getCurrentTick() {
	if (engine->IsOrange()) {
		static int lastEngine;
		if (session->isRunning) {
			lastEngine = engine->GetTick();
		}
		int delta = lastEngine - g_coopLastSyncEngineTick;
		if (delta < 0) delta = 0;
		return g_coopLastSyncTick + delta;
	}

	if (client->GetChallengeStatus() == CMStatus::CHALLENGE) {
		if (g_inDemoLoad) return 0;  // HACKHACK
		return roundf(server->GetCMTimer() / *engine->interval_per_tick);
	}

	return engine->GetTick();
}

static void sendCoopPacket(PacketType t, std::string *splitName = NULL, int newSplit = -1) {
	if (engine->IsOrange()) return;

	size_t size = 5;

	if (newSplit != -1) {
		++size;
	}

	if (splitName) {
		size += splitName->size();
	}

	char *buf = (char *)malloc(size);

	buf[0] = (char)t;
	*(int *)(buf + 1) = getCurrentTick();

	char *ptr = buf + 5;

	if (newSplit != -1) {
		*(ptr++) = newSplit;
	}

	if (splitName) {
		memcpy(ptr, splitName->c_str(), splitName->size());
	}

	NetMessage::SendMsg(SPEEDRUN_PACKET_TYPE, buf, size);

	free(buf);
}

int SpeedrunTimer::GetSegmentTicks() {
	if (g_speedrun.isReset) {
		return sar_speedrun_offset.GetInt();
	}

	if (!g_speedrun.isRunning) {
		return 0;
	}

	int ticks = 0;
	ticks += g_speedrun.saved;
	if (!g_speedrun.isPaused) {
		ticks += getCurrentTick() - g_speedrun.base;
	}

	if (ticks < 0) {
		// This can happen for precisely one tick if you
		// sar_speedrun_start then unpause, because for some dumb
		// reason, console unpausing makes the engine go back one tick
		ticks = 0;
	}

	return ticks;
}

int SpeedrunTimer::GetSplitTicks() {
	int ticks = 0;

	for (Segment seg : g_speedrun.currentSplit) {
		ticks += seg.ticks;
	}

	ticks += SpeedrunTimer::GetSegmentTicks();

	return ticks;
}

int SpeedrunTimer::GetTotalTicks() {
	int ticks = 0;

	for (SplitInfo split : g_speedrun.splits) {
		ticks += split.ticks;
	}

	ticks += SpeedrunTimer::GetSplitTicks();

	return ticks;
}

// }}}

static std::string getEffectiveMapName() {
	std::string map = engine->GetCurrentMapName();
	if (map == "") {
		return "(menu)";
	}
	return map;
}

void SpeedrunTimer::Update() {
	if (g_timerInterface->action != TimerAction::NONE && NOW_STEADY() >= g_actionResetTime) {
		g_timerInterface->action = TimerAction::NONE;
	}

	std::string map = getEffectiveMapName();

	if (map != g_speedrun.lastMap && SpeedrunTimer::IsRunning() && !engine->IsOrange()) {
		bool visited = false;

		for (std::string v : g_speedrun.visitedMaps) {
			if (map == v) {
				visited = true;
				break;
			}
		}

		if (map == "(menu)") {
			// We're in the menu - we don't want to split here, just
			// treat it as if we've visited
			visited = true;
		}

		if (!visited) {
			g_speedrun.visitedMaps.push_back(map);
		}

		bool newSplit = !visited || !sar_speedrun_smartsplit.GetBool();
		SpeedrunTimer::Split(newSplit, g_speedrun.lastMap);
		if (newSplit && networkManager.isConnected) {
			int total = SpeedrunTimer::GetTotalTicks();
			int prevTotal = networkManager.splitTicksTotal == -1 ? 0 : networkManager.splitTicksTotal;
			networkManager.splitTicks = total - prevTotal;
			networkManager.splitTicksTotal = total;
		}

		g_speedrun.hasSplitLoad = true;

		g_speedrun.lastMap = map;
	}

	if (engine->IsCoop() && !engine->IsOrange()) {
		int tick = getCurrentTick();
		if (tick < g_coopLastSyncTick || tick >= g_coopLastSyncTick + SYNC_INTERVAL) {
			sendCoopPacket(PacketType::SYNC);
			g_coopLastSyncTick = tick;
		}
	}

	g_timerInterface->total = SpeedrunTimer::GetTotalTicks();
}

ON_EVENT(PRE_TICK) {
	if (!session->isRunning || !pauseTimer->IsActive()) {
		return;
	}

	if (!g_speedrun.isRunning || g_speedrun.isPaused || !sar_speedrun_time_pauses.GetBool()) {
		return;
	}

	if (engine->IsCoop()) {
		return;
	}

	++g_speedrun.saved;
}

void SpeedrunTimer::FinishLoad() {
	if (!g_speedrun.hasSplitLoad && !engine->IsOrange()) {
		// We went through a load that kept us on the same map; perform
		// a segment split
		SpeedrunTimer::Split(false, getEffectiveMapName());
	}

	// Ready for next load
	g_speedrun.hasSplitLoad = false;
}

// Timer control {{{

void SpeedrunTimer::Start() {
	bool wasRunning = g_speedrun.isRunning;

	SpeedrunTimer::Reset(false);

	setTimerAction(wasRunning ? TimerAction::RESTART : TimerAction::START);

	std::string map = getEffectiveMapName();

	g_speedrun.isRunning = true;
	g_speedrun.isReset = false;
	g_speedrun.base = getCurrentTick();
	g_speedrun.saved = sar_speedrun_offset.GetInt();
	g_speedrun.lastMap = map;
	g_speedrun.visitedMaps.push_back(map);

	sendCoopPacket(PacketType::START);
	toastHud.AddToast(SPEEDRUN_TOAST_TAG, "Speedrun started!");
}

void SpeedrunTimer::Pause() {
	if (!g_speedrun.isRunning || g_speedrun.isPaused) {
		return;
	}

	// On resume, the base will be replaced, so save the full segment
	// time so far
	g_speedrun.saved = SpeedrunTimer::GetSegmentTicks();

	g_speedrun.isPaused = true;

	sendCoopPacket(PacketType::PAUSE);
	console->Print("Speedrun paused!\n");
}

void SpeedrunTimer::Resume() {
	if (!g_speedrun.isRunning || !g_speedrun.isPaused) {
		return;
	}

	g_speedrun.base = getCurrentTick();

	g_speedrun.isPaused = false;

	sendCoopPacket(PacketType::RESUME);
	console->Print("Speedrun resumed!\n");
}

static inline void appendStr(const std::string &str, std::vector<uint8_t> &vec) {
	vec.reserve(vec.size() + str.size());
	std::copy(str.begin(), str.end(), std::back_inserter(vec));
	vec.push_back(0);
}

static inline void appendI32(int x, std::vector<uint8_t> &vec) {
	vec.push_back((x >> 0)  & 0xFF);
	vec.push_back((x >> 8)  & 0xFF);
	vec.push_back((x >> 16) & 0xFF);
	vec.push_back((x >> 24) & 0xFF);
}

static void recordDemoResult() {
	std::vector<uint8_t> data({0x0A});

	appendI32(g_speedrun.splits.size(), data);

	for (SplitInfo split : g_speedrun.splits) {
		appendStr(split.name, data);
		appendI32(split.segments.size(), data);
		for (Segment seg : split.segments) {
			appendStr(seg.name, data);
			appendI32(seg.ticks, data);
		}
	}

	engine->demorecorder->RecordData(data.data(), data.size());
}

void SpeedrunTimer::Stop(std::string segName) {
	if (!g_speedrun.isRunning) {
		return;
	}

	int total = SpeedrunTimer::GetTotalTicks();

	stats->Get(0)->statsCounter->IncrementRunFinished(total);

	SpeedrunTimer::Split(true, segName, false);

	g_runs.push_back(g_activeRun);

	setTimerAction(TimerAction::END);

	g_speedrun.isRunning = false;

	sendCoopPacket(PacketType::STOP, &segName);

	if (networkManager.isConnected) {
		networkManager.NotifySpeedrunFinished(false);
	}

	if (engine->demorecorder->isRecordingDemo) {
		recordDemoResult();
	}

	switch (sar_speedrun_autostop.GetInt()) {
		case 1:
			EngineDemoRecorder::stop_callback_hook({});
			break;
		case 2:
			{
				std::string base = std::string(engine->GetGameDirectory()) + "/" + engine->demorecorder->m_szDemoBaseName;
				int min_demo_num = engine->demorecorder->autorecordStartNum;
				int max_demo_num = *engine->demorecorder->m_nDemoNumber;

				EngineDemoRecorder::stop_callback_hook({});

				auto time_str = SpeedrunTimer::Format(total * *engine->interval_per_tick);
				std::replace(time_str.begin(), time_str.end(), ':', '-');
				std::replace(time_str.begin(), time_str.end(), '.', '-');

				const char *c_base = base.c_str();
				const char *c_time = time_str.c_str();

				for (int i = min_demo_num; i <= max_demo_num; ++i) {
					std::string demo_file =
						i == 1 ? Utils::ssprintf("%s.dem", c_base)
						: Utils::ssprintf("%s_%d.dem", c_base, i);

					std::string new_name =
						i == 1 ? Utils::ssprintf("%s_%s.dem", c_base, c_time)
						: Utils::ssprintf("%s_%s_%d.dem", c_base, c_time, i);

					std::filesystem::rename(demo_file, new_name);
				}
			}
			break;
		default:
			break;
	}
}

void SpeedrunTimer::Split(bool newSplit, std::string segName, bool requested) {
	if (!g_speedrun.isRunning) {
		return;
	}

	g_speedrun.currentSplit.push_back(Segment{
		segName,
		SpeedrunTimer::GetSegmentTicks(),
	});

	if (newSplit) {
		int ticks = 0;

		for (Segment seg : g_speedrun.currentSplit) {
			ticks += seg.ticks;
		}

		g_speedrun.splits.push_back(SplitInfo{
			g_speedrun.currentSplit,
			segName,
			ticks,
		});

		g_activeRun[segName] = ticks;

		g_speedrun.currentSplit.clear();
	}

	g_speedrun.saved = 0;
	g_speedrun.base = getCurrentTick();

	if (requested) {
		sendCoopPacket(PacketType::SPLIT, &segName, newSplit);
	}

	if (newSplit) {
		setTimerAction(TimerAction::SPLIT);
		float totalTime = SpeedrunTimer::GetTotalTicks() * *engine->interval_per_tick;
		float splitTime = g_speedrun.splits.back().ticks * *engine->interval_per_tick;
		std::string text = Utils::ssprintf("%s\n%s (%s)", segName.c_str(), SpeedrunTimer::Format(totalTime).c_str(), SpeedrunTimer::Format(splitTime).c_str());
		toastHud.AddToast(SPEEDRUN_TOAST_TAG, text);
	}
}

void SpeedrunTimer::Reset(bool requested) {
	SpeedrunTimer::ResetCategory();

	if (g_speedrun.isRunning) {
		stats->Get(0)->statsCounter->IncrementReset(SpeedrunTimer::GetTotalTicks());
		g_runs.push_back(g_activeRun);
	}

	g_activeRun.clear();

	g_speedrun.isRunning = false;
	g_speedrun.isPaused = false;
	g_speedrun.isReset = true;

	g_speedrun.hasSplitLoad = false;

	g_speedrun.saved = 0;
	g_speedrun.base = 0;
	g_speedrun.currentSplit.clear();
	g_speedrun.splits.clear();
	g_speedrun.visitedMaps.clear();

	if (networkManager.isConnected) {
		networkManager.splitTicksTotal = -1;
	}

	if (requested) {
		sendCoopPacket(PacketType::RESET);
		setTimerAction(TimerAction::RESET);
		console->Print("Ready for new speedrun!\n");
	}
}

// }}}

bool SpeedrunTimer::IsRunning() {
	return g_speedrun.isRunning;
}

void SpeedrunTimer::OnLoad() {
	SpeedrunTimer::TestLoadRules();

	if (!sar_speedrun_start_on_load.isRegistered) {
		return;
	}

	if (sar_speedrun_start_on_load.GetInt() == 2) {
		SpeedrunTimer::Start();
	} else if (sar_speedrun_start_on_load.GetInt() == 1 && !SpeedrunTimer::IsRunning()) {
		SpeedrunTimer::Start();
	}
}

ON_EVENT(SESSION_START) {
	if (!engine->IsCoop() || (client->GetChallengeStatus() == CMStatus::CHALLENGE && !engine->IsOrange())) {
		if (!engine->IsOrange()) {
			SpeedrunTimer::Resume();
		}

		SpeedrunTimer::OnLoad();
	}
}

ON_EVENT(POST_TICK) {
	if (event.simulating || engine->demoplayer->IsPlaying()) {
		SpeedrunTimer::TickRules();
	}
}

// Time formatting {{{

std::string SpeedrunTimer::Format(float raw) {
	char format[32];

	auto sec = int(std::floor(raw));
	auto ms = int(std::round((raw - sec) * 1000));

	if (sec >= 60) {
		auto min = sec / 60;
		sec = sec % 60;
		if (min >= 60) {
			auto hrs = min / 60;
			min = min % 60;
			snprintf(format, sizeof(format), "%i:%02i:%02i.%03i", hrs, min, sec, ms);
		} else {
			snprintf(format, sizeof(format), "%i:%02i.%03i", min, sec, ms);
		}
	} else {
		snprintf(format, sizeof(format), "%i.%03i", sec, ms);
	}

	return std::string(format);
}

std::string SpeedrunTimer::SimpleFormat(float raw) {
	char format[32];

	auto sec = int(std::floor(raw));
	auto ms = int(std::round((raw - sec) * 1000));

	auto min = sec / 60;
	sec = sec % 60;
	auto hrs = min / 60;
	min = min % 60;
	snprintf(format, sizeof(format), "%i:%02i:%02i.%03i", hrs, min, sec, ms);

	return std::string(format);
}

float SpeedrunTimer::UnFormat(std::string &formated_time) {
	int h, m, s;
	float ms, total = 0;

	if (sscanf(formated_time.c_str(), "%d:%d:%d.%f", &h, &m, &s, &ms) >= 2) {
		total = h * 3600 + m * 60 + s + 0.001 * ms;
	}

	return total;
}

// }}}

Variable sar_speedrun_smartsplit("sar_speedrun_smartsplit", "1", "Only split the speedrun timer a maximum of once per map.\n");
Variable sar_speedrun_time_pauses("sar_speedrun_time_pauses", "0", "Include time spent paused in the speedrun timer.\n");
Variable sar_speedrun_stop_in_menu("sar_speedrun_stop_in_menu", "0", "Automatically stop the speedrun timer when the menu is loaded.\n");
Variable sar_speedrun_start_on_load("sar_speedrun_start_on_load", "0", 0, 2, "Automatically start the speedrun timer when a map is loaded. 2 = restart if active.\n");
Variable sar_speedrun_offset("sar_speedrun_offset", "0", 0, "Start speedruns with this many ticks on the timer.\n");
Variable sar_speedrun_autostop("sar_speedrun_autostop", "0", 0, 2, "Automatically stop recording demos when a speedrun finishes. If 2, automatically append the run time to the demo name.\n");

CON_COMMAND(sar_speedrun_start, "sar_speedrun_start - start the speedrun timer\n") {
	SpeedrunTimer::Start();
}

CON_COMMAND(sar_speedrun_stop, "sar_speedrun_start - stop the speedrun timer\n") {
	SpeedrunTimer::Stop(getEffectiveMapName());
}

CON_COMMAND(sar_speedrun_split, "sar_speedrun_split - perform a split on the speedrun timer\n") {
	SpeedrunTimer::Split(true, getEffectiveMapName());
}

CON_COMMAND(sar_speedrun_pause, "sar_speedrun_pause - pause the speedrun timer\n") {
	SpeedrunTimer::Pause();
}

CON_COMMAND(sar_speedrun_resume, "sar_speedrun_resume - resume the speedrun timer\n") {
	SpeedrunTimer::Resume();
}

CON_COMMAND(sar_speedrun_reset, "sar_speedrun_reset - reset the speedrun timer\n") {
	SpeedrunTimer::Reset();
}

CON_COMMAND(sar_speedrun_result, "sar_speedrun_result - print the speedrun result\n") {
	if (g_speedrun.isReset) {
		console->Print("No active or completed speedrun!\n");
		return;
	}

	int total = 0;

	for (SplitInfo split : g_speedrun.splits) {
		total += split.ticks;
		auto ticksStr = SpeedrunTimer::Format(split.ticks * *engine->interval_per_tick);
		auto totalStr = SpeedrunTimer::Format(total * *engine->interval_per_tick);
		console->Print("%s - %s (%d) - %s\n", split.name.c_str(), ticksStr.c_str(), split.ticks, totalStr.c_str());
		if (split.segments.size() > 1) {
			total -= split.ticks;
			for (Segment seg : split.segments) {
				total += seg.ticks;
				auto ticksStr = SpeedrunTimer::Format(seg.ticks * *engine->interval_per_tick);
				auto totalStr = SpeedrunTimer::Format(total * *engine->interval_per_tick);
				console->Print("    %s - %s (%d) - %s\n", seg.name.c_str(), ticksStr.c_str(), seg.ticks, totalStr.c_str());
			}
		}
	}

	if (g_speedrun.isRunning) {
		console->Print("[current split]\n");
		for (Segment seg : g_speedrun.currentSplit) {
			total += seg.ticks;
			auto ticksStr = SpeedrunTimer::Format(seg.ticks * *engine->interval_per_tick);
			auto totalStr = SpeedrunTimer::Format(total * *engine->interval_per_tick);
			console->Print("    %s - %s (%d) - %s\n", seg.name.c_str(), ticksStr.c_str(), seg.ticks, totalStr.c_str());
		}
		int segTicks = SpeedrunTimer::GetSegmentTicks();
		total += segTicks;
		auto ticksStr = SpeedrunTimer::Format(segTicks * *engine->interval_per_tick);
		auto totalStr = SpeedrunTimer::Format(total * *engine->interval_per_tick);
		console->Print("    [current segment] - %s (%d) - %s\n", ticksStr.c_str(), segTicks, totalStr.c_str());
	}

	total = SpeedrunTimer::GetTotalTicks();
	console->Print("Total: %d (%s)\n", total, SpeedrunTimer::Format(total * *engine->interval_per_tick).c_str());
}

CON_COMMAND(sar_speedrun_export, "sar_speedrun_export <filename> - export the speedrun result to the specified CSV file\n") {
	if (args.ArgC() != 2) {
		console->Print(sar_speedrun_export.ThisPtr()->m_pszHelpString);
		return;
	}

	if (g_speedrun.isReset) {
		console->Print("No active or completed speedrun!\n");
		return;
	}

	if (g_speedrun.isRunning) {
		console->Print("Only completed speedruns can be exported!\n");
		return;
	}

	std::string filename = args[1];
	if (filename.length() < 4 || filename.substr(filename.length() - 4, 4) != ".csv") {
		filename += ".csv";
	}

	FILE *f = fopen(filename.c_str(), "w");
	if (!f) {
		console->Print("Could not open file '%s'\n", filename.c_str());
		return;
	}

	// I'll give in and do Microsoft's stupid thing only on the platform
	// where people are probably using Excel.
#ifdef _WIN32
	fputs(MICROSOFT_PLEASE_FIX_YOUR_SOFTWARE_SMHMYHEAD "\n", f);
#endif

	fputs("Split Name,Ticks,Time,Total Ticks,Total Time\n", f);

	int total = 0;

	for (SplitInfo split : g_speedrun.splits) {
		total += split.ticks;
		auto fmtdTicks = SpeedrunTimer::Format(split.ticks * *engine->interval_per_tick);
		auto fmtdTotal = SpeedrunTimer::Format(total * *engine->interval_per_tick);
		fprintf(f, "%s,%d,%s,%d,%s\n", split.name.c_str(), split.ticks, fmtdTicks.c_str(), total, fmtdTotal.c_str());
	}

	fclose(f);

	console->Print("Speedrun successfully exported to '%s'!\n", filename.c_str());
}

CON_COMMAND(sar_speedrun_export_all, "sar_speedrun_export_all <filename> - export the results of many speedruns to the specified CSV file\n") {
	// TODO: this kinda isn't good, and should probably be revamped when the
	// speedrun system is modified to track all splits.

	if (args.ArgC() != 2) {
		return console->Print(sar_speedrun_export_all.ThisPtr()->m_pszHelpString);
	}

	if (SpeedrunTimer::IsRunning()) {
		console->Print("Note: incomplete speedruns are not included in the export\n");
	}

	if (g_runs.size() == 0) {
		return console->Print("No completed runs to export!\n");
	}

	std::vector<std::string> header;
	for (std::string ruleName : SpeedrunTimer::GetCategoryRules()) {
		auto rule = SpeedrunTimer::GetRule(ruleName);
		if (!rule) continue;
		if (rule->action != RuleAction::SPLIT && rule->action != RuleAction::STOP) continue;
		header.push_back(ruleName);
	}

	std::string filename = args[1];
	if (filename.length() < 4 || filename.substr(filename.length() - 4, 4) != ".csv") {
		filename += ".csv";
	}

#ifdef _WIN32
	bool exists = !_access(filename.c_str(), 0);
#else
	bool exists = !access(filename.c_str(), F_OK);
#endif

	if (exists) {
		console->Print("File exists; appending data. Warning: if the file is for a different set of splits, the data exported will be incorrect!\n");
	}

	FILE *f = fopen(filename.c_str(), "a");
	if (!f) {
		console->Print("Could not open file '%s'\n", filename.c_str());
		return;
	}

	// I'll give in and do Microsoft's stupid thing only on the platform
	// where people are probably using Excel.
#ifdef _WIN32
	fputs(MICROSOFT_PLEASE_FIX_YOUR_SOFTWARE_SMHMYHEAD "\n", f);
#endif

	if (!exists) {
		for (size_t i = 0; i < header.size(); ++i) {
			if (i != 0) fputc(',', f);
			fputs(header[i].c_str(), f);
		}

		fputc('\n', f);
	}

	for (auto &run : g_runs) {
		int total = 0;
		for (size_t i = 0; i < header.size(); ++i) {
			if (i != 0) fputc(',', f);
			auto it = run.find(header[i]);
			if (it != run.end()) {
				int ticks = it->second;
				total += ticks;
				auto fmtdTicks = SpeedrunTimer::Format(ticks * *engine->interval_per_tick);
				auto fmtdTotal = SpeedrunTimer::Format(total * *engine->interval_per_tick);
				fprintf(f, "%s (%s)", fmtdTotal.c_str(), fmtdTicks.c_str());
			}
		}
		fputc('\n', f);
	}

	fclose(f);

	g_runs.clear();

	console->Print("Speedruns successfully exported to '%s'!\n", filename.c_str());
}

void SpeedrunTimer::CategoryChanged() {
	g_runs.clear();
}

CON_COMMAND(sar_speedrun_reset_export, "sar_speedrun_reset_export - reset the log of complete and incomplete runs to be exported\n") {
	g_runs.clear();
}
