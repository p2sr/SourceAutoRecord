#pragma once

#include "Feature.hpp"
#include "Features/Hud/Hud.hpp"
#include "Utils.hpp"

#include <map>

struct HitboxList {
	struct VphysBox {
		std::vector<Vector> verts;
	};

	struct ObbBox {
		Vector mins, maxs;
		Vector pos;
		QAngle ang;
	};

	std::vector<VphysBox> vphys;
	std::vector<VphysBox> bsps; // this is really lazy but it works
	std::vector<ObbBox> obb;
};

struct VphysLocationList {
	struct Location {
		std::string className;
		Vector pos;
		QAngle ang;
	};
	std::map<int, Location> locations;
};

struct PortalLocations {
	struct PortalLocation {
		Vector pos;
		QAngle ang;
		bool is_primary;
		bool is_coop;
		bool is_atlas;
	};

	// This should contain all portals, including
	// partner ones.
	std::vector<PortalLocation> locations;
};

struct Trace {
	int startSessionTick;
	int startTasTick;
	std::vector<Vector> positions[2];
	std::vector<Vector> eyepos[2];
	std::vector<QAngle> angles[2];
	std::vector<Vector> velocities[2];
	std::vector<bool> grounded[2];
	std::vector<bool> crouched[2];
	std::vector<HitboxList> hitboxes[2];
	// Only have one of those, store all the portals in the map
	// indiscriminately of player (also ones placed by pedestals etc)
	std::vector<PortalLocations> portals;
	std::vector<VphysLocationList> vphysLocations;

	std::vector<std::string> log_scope_stack;
	std::vector<std::string> log_lines;
	bool draw = true;
};

class PlayerTrace : public Feature {
private:
	// In order to arbitrarily number traces
	std::map<std::string, Trace> traces;
	std::string lastRecordedTrace;
public:
	PlayerTrace();
	// Tells us whether trace should be recorded
	bool ShouldRecord();
	// Tells us if the trace name is valid
	bool IsTraceNameValid(std::string trace_name);
	// Add a point to the player trace
	void AddPoint(std::string trace_name, void *player, int slot, bool use_client_offset);
	// Returns trace with given id
	Trace *GetTrace(std::string trace_name);
	// Returns default trace name
	std::string GetDefaultTraceName();
	// Returns number of recorded traces
	int GetTraceCount();
	// Clear all the points
	void Clear(std::string trace_name);
	// Clear all the traces
	void ClearAll();
	// Display the trace in the world
	void DrawInWorld() const;
	// Display XY-speed delta overlay
	void DrawSpeedDeltas() const;
	// Display a bbox at the given tick
	void DrawBboxAt(int tick) const;
	// Display the portals at the given tick
	void DrawPortalsAt(int tick) const;
	// Teleport to given tick on given trace
	void TeleportAt(std::string trace_name, int slot, int tick, bool eye);
	// Construct a list of the hitboxes of all entities near a point
	HitboxList ConstructHitboxList(Vector center) const;
	// Construct a list of locations of all dynamic entities for verification purposes
	VphysLocationList ConstructVphysLocationList() const;
	// Construct a list of all portals in the map
	PortalLocations ConstructPortalLocations() const;
	// Draw info about all traces to a HUD context
	void DrawTraceHud(HudContext *ctx);
	// Corrects latest eye offset according to given CMoveData, to make it correct for portal shooting preview
	void TweakLatestEyeOffsetForPortalShot(CMoveData *moveData, int slot, bool clientside);
	// Checks if trace recording ID has been changed
	void CheckTraceChanged();
	// Get the current trace bbox tick for TAS stuff, or -1 if there isn't one
	int GetTasTraceTick();

	// Returns an identifier for the scope which should be passed to ExitLogScope
	void EnterLogScope(const char *name);
	void ExitLogScope();
	void EmitLog(const char *msg);
};

extern PlayerTrace *playerTrace;

extern Vector g_playerTraceTeleportLocation;
extern int g_playerTraceTeleportSlot;
extern bool g_playerTraceNeedsTeleport;
