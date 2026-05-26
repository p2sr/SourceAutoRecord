#include "DataCacheTools.hpp"

#include "Command.hpp"
#include "Game.hpp"
#include "Interface.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Utils.hpp"
#include "Utils/Memory.hpp"
#include "Variable.hpp"

#include <cerrno>
#include <climits>
#include <cstring>
#include <cstdlib>

namespace {
constexpr int DATA_CACHE_SET_SIZE_VTABLE_INDEX = 8;
constexpr int BYTES_PER_MIB = 1024 * 1024;

using DataCacheSetSizeFn = void(__rescall *)(void *thisptr, int bytes);

Variable sar_demo_datacache_flush_between_demos(
	"sar_demo_datacache_flush_between_demos",
	"0",
	0,
	2,
	"Experimental repeated-demo cache flush before queued demos. 0=off, 1=flush, 2=flush_locked. Waits for idle menu state before flushing. Mode 2 also requires sar_demo_datacache_allow_unsafe_flush_locked 1.\n");
Variable sar_demo_datacache_flush_delay_frames(
	"sar_demo_datacache_flush_delay_frames",
	"120",
	0,
	"Experimental repeated-demo cache flush delay. Number of idle menu frames to wait before flushing.\n");
Variable sar_demo_datacache_allow_unsafe_flush_locked(
	"sar_demo_datacache_allow_unsafe_flush_locked",
	"0",
	"Unsafe experiment. Allows queued flush_locked between demos; this has crashed during the next demo load.\n");

bool g_pendingBetweenDemoFlush = false;
int g_waitingForMenuFrames = 0;
int g_idleMenuFrames = 0;

void *GetDataCache() {
	static void *dataCache = nullptr;
	if (!dataCache) {
		dataCache = Interface::GetPtr(MODULE("datacache"), "VDataCache003");
	}
	return dataCache;
}

bool ParseMiB(const char *text, int &out) {
	char *end = nullptr;
	errno = 0;
	auto value = std::strtol(text, &end, 10);
	if (errno || end == text || *end || value <= 0 || value > INT_MAX / BYTES_PER_MIB) {
		return false;
	}
	out = static_cast<int>(value);
	return true;
}

bool IsIdleMenuState() {
	if (!engine || !engine->hoststate || !engine->m_szLevelName || !engine->demoplayer) return false;

	auto hoststateRun = HS_RUN;
	if (sar.game->Is(SourceGame_INFRA)) hoststateRun = INFRA_HS_RUN;

	return engine->hoststate->m_currentState == hoststateRun
		&& !engine->hoststate->m_activeGame
		&& !engine->demoplayer->IsPlaying()
		&& std::strlen(engine->m_szLevelName) == 0
		&& engine->GetMaxClients() <= 1;
}

const char *FlushCommandForMode(int mode) {
	return mode >= 2 ? "flush_locked" : "flush";
}
} // namespace

namespace DataCacheTools {
void AfterDemoStop() {
	if (sar_demo_datacache_flush_between_demos.GetInt() > 0) {
		g_pendingBetweenDemoFlush = true;
		g_waitingForMenuFrames = 0;
		g_idleMenuFrames = 0;
	}
}

bool BeforeQueuedDemoStart() {
	auto mode = sar_demo_datacache_flush_between_demos.GetInt();
	if (mode <= 0) {
		g_pendingBetweenDemoFlush = false;
		return true;
	}

	if (!g_pendingBetweenDemoFlush) return true;
	if (mode >= 2 && !sar_demo_datacache_allow_unsafe_flush_locked.GetBool()) {
		console->Warning("SAR: Refusing queued flush_locked; set sar_demo_datacache_allow_unsafe_flush_locked 1 to force this unsafe experiment.\n");
		g_pendingBetweenDemoFlush = false;
		g_waitingForMenuFrames = 0;
		g_idleMenuFrames = 0;
		return true;
	}

	if (!IsIdleMenuState()) {
		if (g_waitingForMenuFrames == 0 || g_waitingForMenuFrames % 60 == 0) {
			console->Print("Waiting for idle menu state before demo datacache flush.\n");
		}
		++g_waitingForMenuFrames;
		g_idleMenuFrames = 0;
		return false;
	}

	auto delayFrames = sar_demo_datacache_flush_delay_frames.GetInt();
	if (g_idleMenuFrames < delayFrames) {
		if (g_idleMenuFrames == 0 || g_idleMenuFrames % 60 == 0) {
			console->Print("Waiting %d idle menu frames before demo datacache flush.\n", delayFrames - g_idleMenuFrames);
		}
		++g_idleMenuFrames;
		return false;
	}

	auto command = FlushCommandForMode(mode);
	console->Print("Running %s before queued demo start.\n", command);
	engine->ExecuteCommand(command, true);
	g_pendingBetweenDemoFlush = false;
	g_waitingForMenuFrames = 0;
	g_idleMenuFrames = 0;
	return true;
}
} // namespace DataCacheTools

CON_COMMAND(sar_datacache_set_size_mb, "sar_datacache_set_size_mb <mb> - directly sets VDataCache capacity, bypassing datacachesize cvar limits\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_datacache_set_size_mb.ThisPtr()->m_pszHelpString);
	}

	int mib = 0;
	if (!ParseMiB(args[1], mib)) {
		return console->Print("Invalid datacache size. Use a positive integer MiB value.\n");
	}

	auto dataCache = GetDataCache();
	if (!dataCache) {
		return console->Warning("SAR: Failed to get VDataCache003 from datacache.\n");
	}

	auto setSize = Memory::VMT<DataCacheSetSizeFn>(dataCache, DATA_CACHE_SET_SIZE_VTABLE_INDEX);
	setSize(dataCache, mib * BYTES_PER_MIB);
	console->Print("Set VDataCache capacity to %d MB. Run cache_print_summary to confirm.\n", mib);
}
