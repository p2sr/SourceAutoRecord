#include "SpeedrunTimer.hpp"

#include <cmath>
#include <cstring>
#include <vector>
#include <string>

#include "Modules/Client.hpp"
#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"

// TimerAction {{{

enum class TimerAction
{
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

struct TimerInterface
{
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
    , end("SAR_TIMER_END")
{
}

// }}}

// Segment, SplitInfo {{{

struct Segment
{
    std::string name;
    int ticks;
};

struct SplitInfo
{
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

void SpeedrunTimer::Init()
{
    g_timerInterface = new TimerInterface();
    SpeedrunTimer::Reset(false);
}

void SpeedrunTimer::SetIpt(float ipt)
{
    g_timerInterface->ipt = ipt;
}

// Interface action fuckery {{{

static std::chrono::time_point<std::chrono::steady_clock> g_actionResetTime;

static void setTimerAction(TimerAction action)
{
    g_timerInterface->action = action;
    g_actionResetTime = NOW_STEADY() + std::chrono::milliseconds(50); // Bit of a hack - should be enough time for timers to pick up on it
}

// }}}

// Getting time {{{

static int getCurrentTick()
{
    if (server->GetChallengeStatus() == CMStatus::CHALLENGE) {
        return client->GetCMTimer() / *engine->interval_per_tick;
    }

    return engine->GetTick();
}

int SpeedrunTimer::GetSegmentTicks()
{
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

int SpeedrunTimer::GetTotalTicks()
{
    int ticks = 0;

    for (SplitInfo split : g_speedrun.splits) {
        ticks += split.ticks;
    }

    for (Segment seg : g_speedrun.currentSplit) {
        ticks += seg.ticks;
    }

    ticks += SpeedrunTimer::GetSegmentTicks();

    return ticks;
}

// }}}

void SpeedrunTimer::Update()
{
    if (g_timerInterface->action != TimerAction::NONE && NOW_STEADY() >= g_actionResetTime) {
        g_timerInterface->action = TimerAction::NONE;
    }

    std::string map = engine->GetCurrentMapName();
    if (map != g_speedrun.lastMap) {
        bool visited = false;

        for (std::string v : g_speedrun.visitedMaps) {
            if (map == v) {
                visited = true;
                break;
            }
        }

        if (!visited) {
            g_speedrun.visitedMaps.push_back(map);
        }

        bool newSplit = !visited || !sar_speedrun_smartsplit.GetBool();
        SpeedrunTimer::Split(newSplit, g_speedrun.lastMap);

        g_speedrun.hasSplitLoad = true;

        g_speedrun.lastMap = map;
    }

    g_timerInterface->total = SpeedrunTimer::GetTotalTicks();
}

void SpeedrunTimer::AddPauseTick()
{
    if (!g_speedrun.isRunning || g_speedrun.isPaused || !sar_speedrun_time_pauses.GetBool()) {
        return;
    }

    if (engine->IsCoop()) {
        return;
    }

    ++g_speedrun.saved;
}

void SpeedrunTimer::FinishLoad()
{
    if (!g_speedrun.hasSplitLoad) {
        // We went through a load that kept us on the same map; perform
        // a segment split
        SpeedrunTimer::Split(false, engine->GetCurrentMapName(), false);
    }

    // Ready for next load
    g_speedrun.hasSplitLoad = false;
}

// Timer control {{{

void SpeedrunTimer::Start()
{
    bool wasRunning = g_speedrun.isRunning;

    SpeedrunTimer::Reset(false);

    setTimerAction(wasRunning ? TimerAction::RESTART : TimerAction::START);

    g_speedrun.isRunning = true;
    g_speedrun.isReset = false;
    g_speedrun.base = getCurrentTick();
    g_speedrun.saved = sar_speedrun_offset.GetInt();

    console->Print("Speedrun started!\n");
}

void SpeedrunTimer::Pause()
{
    if (!g_speedrun.isRunning || g_speedrun.isPaused) {
        return;
    }

    // On resume, the base will be replaced, so save the full segment
    // time so far
    g_speedrun.saved = SpeedrunTimer::GetSegmentTicks();

    g_speedrun.isPaused = true;

    console->Print("Speedrun paused!\n");
}

void SpeedrunTimer::Resume()
{
    if (!g_speedrun.isRunning || !g_speedrun.isPaused) {
        return;
    }

    g_speedrun.base = getCurrentTick();

    g_speedrun.isPaused = false;

    console->Print("Speedrun resumed!\n");
}

void SpeedrunTimer::Stop(std::string segName)
{
    if (!g_speedrun.isRunning) {
        return;
    }

    SpeedrunTimer::Split(true, segName, false);

    setTimerAction(TimerAction::END);

    g_speedrun.isRunning = false;

    console->Print("Speedrun stopped!\n");
}

void SpeedrunTimer::Split(bool newSplit, std::string segName, bool requested)
{
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

        g_speedrun.currentSplit.clear();
    }

    g_speedrun.saved = 0;
    g_speedrun.base = getCurrentTick();

    if (requested) {
        if (newSplit) {
            setTimerAction(TimerAction::SPLIT);
        }
        console->Print("Speedrun split!\n");
    }
}

void SpeedrunTimer::Reset(bool requested)
{
    SpeedrunTimer::ResetCategory();

    g_speedrun.isRunning = false;
    g_speedrun.isPaused = false;
    g_speedrun.isReset = true;

    g_speedrun.hasSplitLoad = false;

    g_speedrun.saved = 0;
    g_speedrun.base = 0;
    g_speedrun.currentSplit.clear();
    g_speedrun.splits.clear();
    g_speedrun.visitedMaps.clear();

    if (requested) {
        setTimerAction(TimerAction::RESET);
        console->Print("Ready for new speedrun!\n");
    }
}

// }}}

bool SpeedrunTimer::IsRunning()
{
    return g_speedrun.isRunning;
}

bool SpeedrunTimer::ShouldStartOnLoad()
{
    if (!sar_speedrun_start_on_load.isRegistered) return false;
    if (sar_speedrun_start_on_load.GetInt() == 2) return true;
    if (sar_speedrun_start_on_load.GetInt() == 1) return SpeedrunTimer::IsRunning();
    return false;
}

// Time formatting {{{

std::string SpeedrunTimer::Format(float raw)
{
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

std::string SpeedrunTimer::SimpleFormat(float raw)
{
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

float SpeedrunTimer::UnFormat(std::string& formated_time)
{
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

CON_COMMAND(sar_speedrun_start, "sar_speedrun_start - start the speedrun timer.\n")
{
    SpeedrunTimer::Start();
}

CON_COMMAND(sar_speedrun_stop, "sar_speedrun_start - stop the speedrun timer.\n")
{
    SpeedrunTimer::Stop(engine->GetCurrentMapName());
}

CON_COMMAND(sar_speedrun_split, "sar_speedrun_split - perform a split on the speedrun timer.\n")
{
    SpeedrunTimer::Split(true, engine->GetCurrentMapName());
}

CON_COMMAND(sar_speedrun_pause, "sar_speedrun_pause - pause the speedrun timer.\n")
{
    SpeedrunTimer::Pause();
}

CON_COMMAND(sar_speedrun_resume, "sar_speedrun_resume - resume the speedrun timer.\n")
{
    SpeedrunTimer::Resume();
}

CON_COMMAND(sar_speedrun_reset, "sar_speedrun_reset - reset the speedrun timer.\n")
{
    SpeedrunTimer::Reset();
}

CON_COMMAND(sar_speedrun_result, "sar_speedrun_result - print the speedrun result.\n")
{
    if (g_speedrun.isReset) {
        console->Print("No active or completed speedrun!\n");
        return;
    }

    for (SplitInfo split : g_speedrun.splits) {
        console->Print("%s (%d -> %s)\n", split.name.c_str(), split.ticks, SpeedrunTimer::Format(split.ticks * *engine->interval_per_tick).c_str());
        if (split.segments.size() > 1) {
            for (Segment seg : split.segments) {
                console->Print("    %s (%d -> %s)\n", seg.name.c_str(), seg.ticks, SpeedrunTimer::Format(seg.ticks * *engine->interval_per_tick).c_str());
            }
        }
        console->Print("\n");
    }

    if (g_speedrun.isRunning) {
        console->Print("[current split]\n");
        for (Segment seg : g_speedrun.currentSplit) {
            console->Print("    %s (%d -> %s)\n", seg.name.c_str(), seg.ticks, SpeedrunTimer::Format(seg.ticks * *engine->interval_per_tick).c_str());
        }
        int segTicks = SpeedrunTimer::GetSegmentTicks();
        console->Print("    [current segment] (%d -> %s)\n", segTicks, SpeedrunTimer::Format(segTicks * *engine->interval_per_tick).c_str());
        console->Print("\n");
    }

    int total = SpeedrunTimer::GetTotalTicks();
    console->Print("Total: %d -> %s\n", total, SpeedrunTimer::Format(total * *engine->interval_per_tick).c_str());
}

CON_COMMAND(sar_speedrun_export, "sar_speedrun_export <filename> - export the speedrun result to the specified CSV file.\n")
{
    // TODO
}
