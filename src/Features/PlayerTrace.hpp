#pragma once

#include "Feature.hpp"
#include "Features/Hud/Hud.hpp"
#include "Utils.hpp"

#include <map>
#include "Trace/TraceHitbox.hpp"
#include "Trace/TracePortal.hpp"


namespace Trace {
	// Add a point to the player trace
	void AddPoint(std::string trace_name, void *player, int slot, bool use_client_offset);
	// Returns default trace name
	std::string GetDefaultTraceName();
	// Display the trace in the world
	void DrawInWorld();
	// Display XY-speed delta overlay
	void DrawSpeedDeltas();
	// Display a bbox at the given tick
	void DrawBboxAt(int tick);
	// Display the portals at the given tick
	void DrawPortalsAt(int tick);
	// Teleport to given tick on given trace
	void TeleportAt(std::string trace_name, int slot, int tick, bool eye);
	// Draw info about all traces to a HUD context
	void DrawTraceHud(HudContext *ctx);
	// Corrects latest eye offset according to given CMoveData, to make it correct for portal shooting preview
	void TweakLatestEyeOffsetForPortalShot(CMoveData *moveData, int slot, bool clientside);
};

extern Vector g_playerTraceTeleportLocation;
extern int g_playerTraceTeleportSlot;
extern bool g_playerTraceNeedsTeleport;
