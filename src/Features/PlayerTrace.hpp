#pragma once

#include "Feature.hpp"
#include "Features/Hud/Hud.hpp"
#include "Utils.hpp"

#include <map>

struct ContactList {
	struct Contact {
		Vector point, normal;
		std::string otherName;
	};

	std::vector<Contact> contacts;
};

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

struct Trace {
	// Tick inits
	int startSessionTick;
	int startTasTick;

	// Player data
	std::vector<Vector> positions[2];
	std::vector<Vector> eyepos[2];
	std::vector<QAngle> angles[2];
	std::vector<Vector> velocities[2];
	std::vector<bool> grounded[2];
	std::vector<bool> crouched[2];
	// Contact points
	std::vector<ContactList> contacts[2];

	// Other data
	std::vector<HitboxList> hitboxes[2];
};

class PlayerTrace : public Feature {
private:
	// In order to arbitrarily number traces
	std::map<std::string, Trace> traces;

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
	// Display a bbox and contact points at the given tick
	void DrawBboxAt(int tick) const;
	// Teleport to given tick on given trace
	void TeleportAt(std::string trace_name, int slot, int tick, bool eye);
	// Construct a list of the hitboxes of all entities near a point
	HitboxList ConstructHitboxList(Vector center) const;
	// Contstruct a list of the contact points of the given player
	ContactList ConstructContactList(void* player) const;
	// Draw info about all traces to a HUD context
	void DrawTraceHud(HudContext *ctx);
	// Corrects latest eye offset according to given CMoveData, to make it correct for portal shooting preview
	void TweakLatestEyeOffsetForPortalShot(CMoveData *moveData, int slot, bool clientside);
};

extern PlayerTrace *playerTrace;

extern Vector g_playerTraceTeleportLocation;
extern int g_playerTraceTeleportSlot;
extern bool g_playerTraceNeedsTeleport;
