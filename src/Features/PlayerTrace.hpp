#pragma once

#include "Feature.hpp"
#include "Features/Hud/Hud.hpp"
#include "Utils.hpp"

#include <map>

#define TRACE_SCALE_DELTA 16
#define TRACE_SCALE_UPDATE 32
// Stores a position delta as a fixed point number
struct TraceDelta {
	int16_t dx : 12;
	int16_t dy : 12;
	int16_t dz : 12;
	int16_t dv : 10;
	bool speedlocked : 1; // true if player has >150ups on both axes and >300ups xy
	bool maxed_turn : 1;  // true if player has >60ups on an axis and >300ups xy

	inline TraceDelta(const Vector vect, int vel, bool speedlocked, bool maxed_turn)
		: dx(vect.x)
		, dy(vect.y)
		, dz(vect.z)
		, dv(vel)
		, speedlocked(speedlocked)
		, maxed_turn(maxed_turn) {}
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
	Vector last_pos;
	float last_speed;
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
