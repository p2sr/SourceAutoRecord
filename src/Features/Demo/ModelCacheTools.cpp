#include "Event.hpp"
#include "Modules/Console.hpp"
#include "Modules/Server.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Utils/Memory.hpp"
#include "Variable.hpp"

namespace {
constexpr unsigned int MODEL_FLAG_PROTECTED_MASK = 0x7E;
constexpr unsigned int MODEL_FLAG_SKIP_PROTECTED_CLEAR = 0x40;
constexpr unsigned int MAX_REASONABLE_MODEL_COUNT = 16384;

DECL_CVAR_CALLBACK(sar_demo_modelcache_clear_protected_flags);

Variable sar_demo_modelcache_clear_protected_flags(
	"sar_demo_modelcache_clear_protected_flags",
	"0",
	"Fix demo model-cache growth by clearing stale CModelLoader protected flags on demo stop. Requires sv_cheats 1 to enable.\n",
	FCVAR_NONE,
	sar_demo_modelcache_clear_protected_flags_callback);

DECL_CVAR_CALLBACK(sar_demo_modelcache_clear_protected_flags) {
	if (sar_demo_modelcache_clear_protected_flags.GetBool() && !sv_cheats.GetBool()) {
		console->Print("sar_demo_modelcache_clear_protected_flags requires sv_cheats 1.\n");
		sar_demo_modelcache_clear_protected_flags.SetValue(0);
	}
}

void *GetModelLoader() {
	static uintptr_t global = 0;
	if (!global) {
		auto site = Memory::Scan(MODULE("engine"), Offsets::CModelLoaderModelPrecache, Offsets::CModelLoaderModelPrecacheGlobal);
		if (!site) return nullptr;

		global = Memory::Deref<uintptr_t>(site);
	}

	return Memory::Deref<void *>(global);
}

void ClearProtectedModelFlags() {
	auto modelLoader = reinterpret_cast<uintptr_t>(GetModelLoader());
	if (!modelLoader) {
		static bool warned = false;
		if (!warned) {
			warned = true;
			console->Warning("SAR: Failed to find CModelLoader for repeated-demo model-cache cleanup.\n");
		}
		return;
	}

	auto entries = *reinterpret_cast<uintptr_t *>(modelLoader + Offsets::CModelLoaderEntryArray);
	auto count = *reinterpret_cast<unsigned short *>(modelLoader + Offsets::CModelLoaderEntryCount);
	if (!entries || count > MAX_REASONABLE_MODEL_COUNT) {
		static bool warned = false;
		if (!warned) {
			warned = true;
			console->Warning(
				"SAR: Invalid CModelLoader state (loader=%p entries=%p count=%u).\n",
				reinterpret_cast<void *>(modelLoader),
				reinterpret_cast<void *>(entries),
				count);
		}
		return;
	}

	for (unsigned int i = 0; i < count; ++i) {
		auto model = *reinterpret_cast<uintptr_t *>(entries + i * Offsets::CModelLoaderEntryStride + Offsets::CModelLoaderEntryModel);
		if (!model) continue;

		auto flags = reinterpret_cast<unsigned int *>(model + Offsets::CModelLoaderModelFlags);
		if ((*flags & MODEL_FLAG_SKIP_PROTECTED_CLEAR) != 0) continue;

		*flags &= ~MODEL_FLAG_PROTECTED_MASK;
	}
}
} // namespace

ON_EVENT(DEMO_STOP) {
	if (sar_demo_modelcache_clear_protected_flags.GetBool() && sv_cheats.GetBool()) {
		ClearProtectedModelFlags();
	}
}
