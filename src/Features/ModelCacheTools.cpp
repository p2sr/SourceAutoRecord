#include "ModelCacheTools.hpp"

#include "Command.hpp"
#include "Modules/Console.hpp"
#include "Utils.hpp"
#include "Utils/Memory.hpp"
#include "Variable.hpp"

namespace {
constexpr int CMODELLOADER_FULL_RESET_VTABLE_INDEX = 1;
constexpr int CMODELLOADER_STRONG_CLEANUP_VTABLE_INDEX = 12;
constexpr int CMODELLOADER_ENTRY_ARRAY_OFFSET = 0x8;
constexpr int CMODELLOADER_ENTRY_COUNT_OFFSET = 0x16;
constexpr int CMODELLOADER_ENTRY_STRIDE = 0x10;
constexpr int CMODELLOADER_ENTRY_MODEL_OFFSET = 0xC;
constexpr int MODEL_FLAGS_OFFSET = 0x108;
constexpr unsigned int MODEL_FLAG_PROTECTED_MASK = 0x7E;
constexpr unsigned int MODEL_FLAG_PROTECTED_PRELOAD = 0x4;
constexpr unsigned int MODEL_FLAG_SKIP_PROTECTED_CLEAR = 0x40;
constexpr unsigned int MAX_REASONABLE_MODEL_COUNT = 16384;

using CModelLoaderStrongCleanupFn = void(__rescall *)(void *thisptr);
using CModelLoaderFullResetFn = void(__rescall *)(void *thisptr);

Variable sar_demo_modelcache_cleanup(
	"sar_demo_modelcache_cleanup",
	"0",
	0,
	3,
	"Experimental repeated-demo cleanup. 0=off, 1=after demo stop, 2=before next demo start, 3=both.\n");
Variable sar_demo_modelcache_unprotect_models(
	"sar_demo_modelcache_unprotect_models",
	"0",
	"Experimental repeated-demo patch. During demo playback, load client model precache/consistency models with flag 0 instead of protected flag 4.\n");
Variable sar_demo_modelcache_clear_flag4(
	"sar_demo_modelcache_clear_flag4",
	"0",
	0,
	3,
	"Experimental repeated-demo cleanup. 0=off, 1=after demo stop, 2=before next demo start, 3=both. Clears CModelLoader model flag 4; automatic strong cleanup is skipped unless sar_demo_modelcache_cleanup_after_clear is set.\n");
Variable sar_demo_modelcache_clear_protected_flags(
	"sar_demo_modelcache_clear_protected_flags",
	"0",
	0,
	3,
	"Experimental repeated-demo cleanup. 0=off, 1=after demo stop, 2=before next demo start, 3=both. Clears CModelLoader protected flag mask 0x7E without unloading models; models with bit 0x40 are skipped.\n");
Variable sar_demo_modelcache_cleanup_after_clear(
	"sar_demo_modelcache_cleanup_after_clear",
	"0",
	"Unsafe experiment. If set, automatic strong cleanup is allowed to run while CModelLoader protected-flag clearing is enabled.\n");
Variable sar_demo_modelcache_full_reset(
	"sar_demo_modelcache_full_reset",
	"0",
	0,
	3,
	"Experimental repeated-demo CModelLoader full reset. 0=off, 1=after demo stop, 2=before next demo start, 3=both.\n");
Variable sar_demo_modelcache_report(
	"sar_demo_modelcache_report",
	"0",
	0,
	3,
	"Repeated-demo diagnostic. 0=off, 1=after demo stop, 2=before next demo start, 3=both. Reports CModelLoader model flag counts without changing state.\n");

bool g_cleanupPendingBeforeStart = false;
Memory::Patch *g_modelPrecacheFlagPatch = nullptr;
Memory::Patch *g_modelConsistencyFlagPatch = nullptr;

bool IsAddressInModule(uintptr_t address, const char *moduleName) {
	Memory::ModuleInfo info = {};
	if (!Memory::TryGetModule(moduleName, &info)) return false;

	return address >= info.base && address < info.base + info.size;
}

uintptr_t ReadEngineGlobalFromPattern(uintptr_t site, int immediateOffset, const char *name) {
	if (!site) return 0;

	auto candidate = *reinterpret_cast<uintptr_t *>(site + immediateOffset);
	if (!IsAddressInModule(candidate, MODULE("engine"))) {
		console->Warning("SAR: Ignoring suspicious %s global address %p.\n", name, reinterpret_cast<void *>(candidate));
		return 0;
	}

	return candidate;
}

uintptr_t FindModelLoaderGlobal() {
#ifdef _WIN32
	static uintptr_t global = 0;
	if (global) return global;

	// SV_ActivateServer-specific context for: mov ecx, g_pModelLoader; vtable +0x30.
	uintptr_t site = Memory::Scan(MODULE("engine"), "80 3D ? ? ? ? 00 74 0D 8B 0D ? ? ? ? 8B 01 8B 50 30 FF D2 83 3D ? ? ? ? 01 7F");
	global = ReadEngineGlobalFromPattern(site, 11, "SV_ActivateServer CModelLoader");
	if (global) return global;

	// Client modelprecache-specific context for: g_pModelLoader->FindOrLoadModel(name, 4).
	site = Memory::Scan(MODULE("engine"), "8B 0D ? ? ? ? 8B 11 6A 04 50 8B 42 1C FF D0 50 EB 02 6A 00");
	global = ReadEngineGlobalFromPattern(site, 2, "modelprecache CModelLoader");
	if (global) return global;

	// Client consistency-check model path uses the same global.
	site = Memory::Scan(MODULE("engine"), "8B 0D ? ? ? ? 8B 01 8B 50 1C 6A 04 53 FF D2 8B F8");
	global = ReadEngineGlobalFromPattern(site, 2, "consistency CModelLoader");
	if (global) return global;

	return 0;
#else
	return 0;
#endif
}

void *GetModelLoader() {
	auto global = FindModelLoaderGlobal();
	if (!global) return nullptr;

	return *reinterpret_cast<void **>(global);
}

bool GetModelTable(uintptr_t &modelLoader, uintptr_t &entries, unsigned short &count, const char *action) {
	modelLoader = reinterpret_cast<uintptr_t>(GetModelLoader());
	if (!modelLoader) {
		console->Warning("SAR: Failed to find CModelLoader for %s.\n", action);
		return false;
	}

	entries = *reinterpret_cast<uintptr_t *>(modelLoader + CMODELLOADER_ENTRY_ARRAY_OFFSET);
	count = *reinterpret_cast<unsigned short *>(modelLoader + CMODELLOADER_ENTRY_COUNT_OFFSET);
	if (!entries || count > MAX_REASONABLE_MODEL_COUNT) {
		console->Warning(
			"SAR: Refusing CModelLoader %s with suspicious state (loader=%p entries=%p count=%u).\n",
			action,
			reinterpret_cast<void *>(modelLoader),
			reinterpret_cast<void *>(entries),
			count);
		return false;
	}

	return true;
}

bool RunStrongCleanup(const char *reason, bool verbose) {
	auto modelLoader = GetModelLoader();
	if (!modelLoader) {
		if (verbose) console->Warning("SAR: Failed to find CModelLoader for model-cache cleanup.\n");
		return false;
	}

	auto cleanup = Memory::VMT<CModelLoaderStrongCleanupFn>(modelLoader, CMODELLOADER_STRONG_CLEANUP_VTABLE_INDEX);
	if (!cleanup) {
		if (verbose) console->Warning("SAR: Failed to find CModelLoader strong cleanup method.\n");
		return false;
	}

	cleanup(modelLoader);
	if (verbose) console->Print("Ran CModelLoader strong cleanup (%s).\n", reason);
	return true;
}

bool RunFullReset(const char *reason, bool verbose) {
	auto modelLoader = GetModelLoader();
	if (!modelLoader) {
		if (verbose) console->Warning("SAR: Failed to find CModelLoader for model-cache full reset.\n");
		return false;
	}

	auto fullReset = Memory::VMT<CModelLoaderFullResetFn>(modelLoader, CMODELLOADER_FULL_RESET_VTABLE_INDEX);
	if (!fullReset) {
		if (verbose) console->Warning("SAR: Failed to find CModelLoader full reset method.\n");
		return false;
	}

	fullReset(modelLoader);
	if (verbose) console->Print("Ran CModelLoader full reset (%s).\n", reason);
	return true;
}

bool ShouldCleanup(int bit) {
	return (sar_demo_modelcache_cleanup.GetInt() & bit) != 0;
}

bool ShouldClearFlag4(int bit) {
	return (sar_demo_modelcache_clear_flag4.GetInt() & bit) != 0;
}

bool ShouldClearProtectedFlags(int bit) {
	return (sar_demo_modelcache_clear_protected_flags.GetInt() & bit) != 0;
}

bool ShouldFullReset(int bit) {
	return (sar_demo_modelcache_full_reset.GetInt() & bit) != 0;
}

bool ShouldReport(int bit) {
	return (sar_demo_modelcache_report.GetInt() & bit) != 0;
}

bool RunAutomaticStrongCleanup(const char *reason) {
	if ((sar_demo_modelcache_clear_flag4.GetInt() || sar_demo_modelcache_clear_protected_flags.GetInt())
		&& !sar_demo_modelcache_cleanup_after_clear.GetBool()) {
		console->Print("Skipping CModelLoader strong cleanup (%s) because protected-flag clearing is active.\n", reason);
		return false;
	}

	return RunStrongCleanup(reason, true);
}

int ClearModelFlag4(const char *reason, bool verbose) {
	uintptr_t modelLoader = 0;
	uintptr_t entries = 0;
	unsigned short count = 0;
	if (!GetModelTable(modelLoader, entries, count, "model flag cleanup")) return 0;

	if (verbose) {
		console->Print(
			"Scanning CModelLoader flag 4 (%s, loader=%p entries=%p count=%u).\n",
			reason,
			reinterpret_cast<void *>(modelLoader),
			reinterpret_cast<void *>(entries),
			count);
	}

	int cleared = 0;
	for (unsigned int i = 0; i < count; ++i) {
		auto model = *reinterpret_cast<uintptr_t *>(entries + i * CMODELLOADER_ENTRY_STRIDE + CMODELLOADER_ENTRY_MODEL_OFFSET);
		if (!model) continue;

		auto flags = reinterpret_cast<unsigned int *>(model + MODEL_FLAGS_OFFSET);
		if ((*flags & MODEL_FLAG_PROTECTED_PRELOAD) == 0) continue;

		*flags &= ~MODEL_FLAG_PROTECTED_PRELOAD;
		++cleared;
	}

	if (verbose) console->Print("Cleared CModelLoader flag 4 from %d models (%s).\n", cleared, reason);
	return cleared;
}

int ClearModelProtectedFlags(const char *reason, bool verbose) {
	uintptr_t modelLoader = 0;
	uintptr_t entries = 0;
	unsigned short count = 0;
	if (!GetModelTable(modelLoader, entries, count, "model protected-flag cleanup")) return 0;

	if (verbose) {
		console->Print(
			"Scanning CModelLoader protected flags (%s, loader=%p entries=%p count=%u).\n",
			reason,
			reinterpret_cast<void *>(modelLoader),
			reinterpret_cast<void *>(entries),
			count);
	}

	int cleared = 0;
	int skippedBit40 = 0;
	for (unsigned int i = 0; i < count; ++i) {
		auto model = *reinterpret_cast<uintptr_t *>(entries + i * CMODELLOADER_ENTRY_STRIDE + CMODELLOADER_ENTRY_MODEL_OFFSET);
		if (!model) continue;

		auto flags = reinterpret_cast<unsigned int *>(model + MODEL_FLAGS_OFFSET);
		if ((*flags & MODEL_FLAG_SKIP_PROTECTED_CLEAR) != 0) {
			++skippedBit40;
			continue;
		}

		if ((*flags & MODEL_FLAG_PROTECTED_MASK) == 0) continue;
		*flags &= ~MODEL_FLAG_PROTECTED_MASK;
		++cleared;
	}

	if (verbose) {
		console->Print(
			"Cleared CModelLoader protected flags 0x7E from %d models; skipped %d bit0x40 models (%s).\n",
			cleared,
			skippedBit40,
			reason);
	}
	return cleared;
}

void ReportModelFlags(const char *reason) {
	uintptr_t modelLoader = 0;
	uintptr_t entries = 0;
	unsigned short count = 0;
	if (!GetModelTable(modelLoader, entries, count, "model flag report")) return;

	unsigned int models = 0;
	unsigned int zero = 0;
	unsigned int protectedFlags = 0;
	unsigned int bit2 = 0;
	unsigned int bit4 = 0;
	unsigned int bit8 = 0;
	unsigned int bit10 = 0;
	unsigned int bit20 = 0;
	unsigned int bit40 = 0;

	for (unsigned int i = 0; i < count; ++i) {
		auto model = *reinterpret_cast<uintptr_t *>(entries + i * CMODELLOADER_ENTRY_STRIDE + CMODELLOADER_ENTRY_MODEL_OFFSET);
		if (!model) continue;

		++models;
		auto flags = *reinterpret_cast<unsigned int *>(model + MODEL_FLAGS_OFFSET);
		if (flags == 0) ++zero;
		if (flags & MODEL_FLAG_PROTECTED_MASK) ++protectedFlags;
		if (flags & 0x2) ++bit2;
		if (flags & 0x4) ++bit4;
		if (flags & 0x8) ++bit8;
		if (flags & 0x10) ++bit10;
		if (flags & 0x20) ++bit20;
		if (flags & 0x40) ++bit40;
	}

	console->Print(
		"CModelLoader report (%s): loader=%p entries=%p count=%u models=%u.\n",
		reason,
		reinterpret_cast<void *>(modelLoader),
		reinterpret_cast<void *>(entries),
		count,
		models);
	console->Print(
		"CModelLoader flags (%s): zero=%u protected0x7e=%u bit0x02=%u bit0x04=%u bit0x08=%u bit0x10=%u bit0x20=%u bit0x40=%u.\n",
		reason,
		zero,
		protectedFlags,
		bit2,
		bit4,
		bit8,
		bit10,
		bit20,
		bit40);
}

bool InitFlagPatch(Memory::Patch *&patch, const char *pattern, int immediateOffset, const char *name) {
	if (!patch) patch = new Memory::Patch();
	if (patch->IsInit()) return true;

#ifdef _WIN32
	auto site = Memory::Scan(MODULE("engine"), pattern);
	if (!site) {
		console->Warning("SAR: Failed to find %s model flag patch site.\n", name);
		return false;
	}

	unsigned char zeroFlag = 0;
	if (!patch->Execute(site + immediateOffset, &zeroFlag, 1)) {
		console->Warning("SAR: Failed to apply %s model flag patch.\n", name);
		return false;
	}
	patch->Restore();
	return true;
#else
	return false;
#endif
}

void SetFlagPatches(bool enabled) {
	if (!sar_demo_modelcache_unprotect_models.GetBool()) enabled = false;
	if (!enabled) {
		if (g_modelPrecacheFlagPatch) g_modelPrecacheFlagPatch->Restore();
		if (g_modelConsistencyFlagPatch) g_modelConsistencyFlagPatch->Restore();
		return;
	}

	bool ready = InitFlagPatch(
		g_modelPrecacheFlagPatch,
		"8B 0D ? ? ? ? 8B 11 6A 04 50 8B 42 1C FF D0 50 EB 02 6A 00",
		9,
		"modelprecache");
	ready = InitFlagPatch(
		g_modelConsistencyFlagPatch,
		"8B 0D ? ? ? ? 8B 01 8B 50 1C 6A 04 53 FF D2 8B F8",
		12,
		"consistency") && ready;
	if (!ready) return;

	g_modelPrecacheFlagPatch->Execute();
	g_modelConsistencyFlagPatch->Execute();
	console->Print("Patched demo model loads to use unprotected flag 0.\n");
}
} // namespace

namespace ModelCacheTools {
void CleanupAfterDemoStop() {
	g_cleanupPendingBeforeStart = true;
	bool report = ShouldReport(1);
	bool mutating = ShouldFullReset(1) || ShouldClearProtectedFlags(1) || ShouldClearFlag4(1) || ShouldCleanup(1);
	if (report && mutating) ReportModelFlags("after demo stop before cleanup");
	if (ShouldFullReset(1)) RunFullReset("after demo stop", true);
	if (ShouldClearProtectedFlags(1)) ClearModelProtectedFlags("after demo stop", true);
	if (ShouldClearFlag4(1)) ClearModelFlag4("after demo stop", true);
	if (ShouldCleanup(1)) {
		RunAutomaticStrongCleanup("after demo stop");
	}
	if (report) ReportModelFlags(mutating ? "after demo stop after cleanup" : "after demo stop");
	SetFlagPatches(false);
}

void CleanupBeforeDemoStart() {
	if (g_cleanupPendingBeforeStart) {
		g_cleanupPendingBeforeStart = false;
		bool report = ShouldReport(2);
		bool mutating = ShouldFullReset(2) || ShouldClearProtectedFlags(2) || ShouldClearFlag4(2) || ShouldCleanup(2);
		if (report && mutating) ReportModelFlags("before demo start before cleanup");
		if (ShouldFullReset(2)) RunFullReset("before demo start", true);
		if (ShouldClearProtectedFlags(2)) ClearModelProtectedFlags("before demo start", true);
		if (ShouldClearFlag4(2)) ClearModelFlag4("before demo start", true);

		if (ShouldCleanup(2)) {
			RunAutomaticStrongCleanup("before demo start");
		}
		if (report) ReportModelFlags(mutating ? "before demo start after cleanup" : "before demo start");
	}
	SetFlagPatches(true);
}
} // namespace ModelCacheTools

CON_COMMAND(sar_modelcache_strong_cleanup, "sar_modelcache_strong_cleanup - manually runs CModelLoader's stronger stale-flag cleanup\n") {
	RunStrongCleanup("manual command", true);
}

CON_COMMAND(sar_modelcache_full_reset, "sar_modelcache_full_reset - manually runs CModelLoader's full reset/unload path\n") {
	RunFullReset("manual command", true);
}

CON_COMMAND(sar_modelcache_clear_flag4, "sar_modelcache_clear_flag4 - manually clears CModelLoader model flag 4 without running stronger cleanup\n") {
	ClearModelFlag4("manual command", true);
}

CON_COMMAND(sar_modelcache_clear_protected_flags, "sar_modelcache_clear_protected_flags - manually clears CModelLoader protected flags 0x7E without unloading models\n") {
	ClearModelProtectedFlags("manual command", true);
}

CON_COMMAND(sar_modelcache_report, "sar_modelcache_report - reports CModelLoader model flag counts without changing state\n") {
	ReportModelFlags("manual command");
}
