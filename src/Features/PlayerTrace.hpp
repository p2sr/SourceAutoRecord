#pragma once

#include "Feature.hpp"
#include "Features/Hud/Hud.hpp"
#include "Utils.hpp"

#include <map>

#define TRACE_SCALE_DELTA 10
#define TRACE_SCALE_UPDATE 32
// Stores a position delta as a fixed point number
struct TraceDelta {
	int16_t dx : 10;
	int16_t dy : 10;
	int16_t dz : 10;

	inline TraceDelta(const Vector vect)
		: dx(vect.x)
		, dy(vect.y)
		, dz(vect.z) {}
	inline Vector asVector() const {
		return Vector(dx, dy, dz);
	}
};

// Stores accurate enough info about the player for the trace
// Tick is an index into the position deltas
struct TraceUpdate {
	size_t tick;
	float speed;
	int32_t x : 21;
	int32_t y : 21;
	int32_t z : 21;
	bool grounded : 1;
};

struct Trace {
	// Keeps track of player pos
	// The interval between them is not constant
	std::vector<TraceDelta> deltas;
	// Keeps track of groudframes, speed at important moments in the hops
	std::vector<TraceUpdate> updates;
	// Remember the last pos, in order to compute deltas
	// Note: it is actually position times TRACE_SCALE
	Vector last_pos;
};

class PlayerTrace : public Feature {
private:
	// In order to arbitrarily number traces
	std::map<size_t, Trace> traces;

public:
	PlayerTrace();
	// Add a point to the player trace
	void AddPoint(size_t trace_idx, void *player);
	// Clear all the points
	void Clear(const size_t trace_idx);
	// Clear all the traces
	void ClearAll();
	// Display the trace in the world
	void DrawInWorld(float time) const;
	// Display XY-speed delta overlay
	void DrawSpeedDeltas(HudContext *ctx) const;
};

extern PlayerTrace *playerTrace;
