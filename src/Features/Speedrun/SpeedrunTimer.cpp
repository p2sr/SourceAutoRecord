#include "SpeedrunTimer.hpp"

#include <cmath>
#include <cstring>
#include <vector>
#include <string>

#include "Modules/Client.hpp"
#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"
#include "Features/NetMessage.hpp"
#include "Features/Hud/Toasts.hpp"

#define SPEEDRUN_PACKET_TYPE "srtimer"
#define SYNC_INTERVAL 300 // Sync every 5 seconds, just in case

Variable sar_speedrun_notify_duration("sar_speedrun_notify_duration", "6", "Number of seconds to show the speedrun notification on-screen for.\n");

static int g_notifyR = 255;
static int g_notifyG = 255;
static int g_notifyB = 255;

CON_COMMAND(sar_speedrun_notify_set_color, "sar_speedrun_notify_set_color <hex code> - sets the speedrun notification color to the specified sRGB color code.\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_speedrun_notify_set_color.ThisPtr()->m_pszHelpString);
    }

    const char *color = args[1];
    if (color[0] == '#') {
        ++color;
    }

    int r, g, b;
    int end;
    if (sscanf(color, "%2x%2x%2x%n", &r, &g, &b, &end) != 3 || end != 6) {
        return console->Print("Invalid color code!\n");
    }

    g_notifyR = Utils::ConvertFromSrgb(r);
    g_notifyG = Utils::ConvertFromSrgb(g);
    g_notifyB = Utils::ConvertFromSrgb(b);
}

// FIXME: because of how NetMessage is currently implemented, some
// splits will be lost for orange as there is a cap on how quickly you
// can send chat messages

enum PacketType
{
    SYNC,
    START,
    PAUSE,
    RESUME,
    STOP,
    SPLIT,
    RESET,
};

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

static void handleCoopPacket(void *data, size_t size);

void SpeedrunTimer::Init()
{
    g_timerInterface = new TimerInterface();
    SpeedrunTimer::Reset(false);
    NetMessage::RegisterHandler(SPEEDRUN_PACKET_TYPE, &handleCoopPacket);
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

// Both Orange and Blue - the tick we synced
static int g_coopLastSyncTick;
// Orange only - the tick we synced, as reported by the engine
static int g_coopLastSyncEngineTick;

static void handleCoopPacket(void *data, size_t size)
{
    if (!engine->IsOrange()) return;

    char *data_ = (char *)data;

    if (size < 5) return;

    PacketType t = (PacketType)data_[0];
    int tick = *(int *)(data_ + 1);

    g_coopLastSyncTick = tick;
    g_coopLastSyncEngineTick = engine->GetTick();

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

static int getCurrentTick()
{
    if (engine->IsOrange()) {
        int delta = engine->GetTick() - g_coopLastSyncEngineTick;
        if (delta < 0) delta = 0;
        return g_coopLastSyncTick + delta;
    }

    if (server->GetChallengeStatus() == CMStatus::CHALLENGE) {
        return client->GetCMTimer() / *engine->interval_per_tick;
    }

    return engine->GetTick();
}

static void sendCoopPacket(PacketType t, std::string *splitName = NULL, int newSplit = -1) {
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

int SpeedrunTimer::GetSplitTicks()
{
    int ticks = 0;

    for (Segment seg : g_speedrun.currentSplit) {
        ticks += seg.ticks;
    }

    ticks += SpeedrunTimer::GetSegmentTicks();

    return ticks;
}

int SpeedrunTimer::GetTotalTicks()
{
    int ticks = 0;

    for (SplitInfo split : g_speedrun.splits) {
        ticks += split.ticks;
    }

    ticks += SpeedrunTimer::GetSplitTicks();

    return ticks;
}

// }}}

static std::string getEffectiveMapName()
{
    std::string map = engine->GetCurrentMapName();
    if (map == "") {
        return "(menu)";
    }
    return map;
}

void SpeedrunTimer::Update()
{
    if (g_timerInterface->action != TimerAction::NONE && NOW_STEADY() >= g_actionResetTime) {
        g_timerInterface->action = TimerAction::NONE;
    }

    std::string map = getEffectiveMapName();

    if (map != g_speedrun.lastMap && !engine->IsOrange()) {
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
    if (!g_speedrun.hasSplitLoad && !engine->IsOrange()) {
        // We went through a load that kept us on the same map; perform
        // a segment split
        SpeedrunTimer::Split(false, getEffectiveMapName());
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

    std::string map = getEffectiveMapName();

    g_speedrun.isRunning = true;
    g_speedrun.isReset = false;
    g_speedrun.base = getCurrentTick();
    g_speedrun.saved = sar_speedrun_offset.GetInt();
    g_speedrun.lastMap = map;
    g_speedrun.visitedMaps.push_back(map);

    sendCoopPacket(PacketType::START);
    if (sar_speedrun_notify_duration.GetFloat() > 0) {
        toastHud.AddToast("Speedrun started!", { g_notifyR, g_notifyG, g_notifyB, 255 }, sar_speedrun_notify_duration.GetFloat());
    }
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

    sendCoopPacket(PacketType::PAUSE);
    console->Print("Speedrun paused!\n");
}

void SpeedrunTimer::Resume()
{
    if (!g_speedrun.isRunning || !g_speedrun.isPaused) {
        return;
    }

    g_speedrun.base = getCurrentTick();

    g_speedrun.isPaused = false;

    sendCoopPacket(PacketType::RESUME);
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

    sendCoopPacket(PacketType::STOP, &segName);
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
        sendCoopPacket(PacketType::SPLIT, &segName, newSplit);
    }

    if (newSplit) {
        setTimerAction(TimerAction::SPLIT);
        if (sar_speedrun_notify_duration.GetFloat() > 0) {
            float totalTime = SpeedrunTimer::GetTotalTicks() * *engine->interval_per_tick;
            float splitTime = g_speedrun.splits.back().ticks;
            std::string text = Utils::ssprintf("%s\n%s (%s)", segName.c_str(), SpeedrunTimer::Format(totalTime).c_str(), SpeedrunTimer::Format(splitTime).c_str());
            toastHud.AddToast(text, { g_notifyR, g_notifyG, g_notifyB, 255 }, sar_speedrun_notify_duration.GetFloat());
        }
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
        sendCoopPacket(PacketType::RESET);
        setTimerAction(TimerAction::RESET);
        console->Print("Ready for new speedrun!\n");
    }
}

// }}}

bool SpeedrunTimer::IsRunning()
{
    return g_speedrun.isRunning;
}

void SpeedrunTimer::OnLoad()
{
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
    SpeedrunTimer::Stop(getEffectiveMapName());
}

CON_COMMAND(sar_speedrun_split, "sar_speedrun_split - perform a split on the speedrun timer.\n")
{
    SpeedrunTimer::Split(true, getEffectiveMapName());
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
