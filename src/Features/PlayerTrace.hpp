#pragma once

#include "Feature.hpp"
#include "Features/Hud/Hud.hpp"
#include "Utils.hpp"

#include <map>

struct Trace {
	std::vector<Vector> positions;
	std::vector<Vector> velocities;
	std::vector<bool> grounded;
	std::vector<bool> crouched;
};

class PlayerTrace : public Feature {
private:
	// In order to arbitrarily number traces
	std::map<size_t, Trace> traces;

public:
	PlayerTrace();
	// Add a point to the player trace
	void AddPoint(size_t trace_idx, void *player, bool use_client_offset);
	// Clear all the points
	void Clear(const size_t trace_idx);
	// Clear all the traces
	void ClearAll();
	// Display the trace in the world
	void DrawInWorld(float time) const;
	// Display XY-speed delta overlay
	void DrawSpeedDeltas(HudContext *ctx) const;
	// Display a bbox at the given tick
	void DrawBboxAt(int tick) const;
};

extern PlayerTrace *playerTrace;
