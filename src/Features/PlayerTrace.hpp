#pragma once

#include "Feature.hpp"
#include "Utils.hpp"
#include "Features/Hud/Hud.hpp"

class PlayerTrace : public Feature {
private:
    std::vector<Vector> trace;
    // Records at which indexes into the trace the grounded state changes
    std::vector<size_t> groundframe_swaps;
    // Records whether player started grounded or not
    bool started_grounded;

    void DrawSpeedDelta(HudContext *ctx, int *hud_id, size_t begin_tick, size_t end_tick) const;

public:
    PlayerTrace();
    // Add a point to the player trace
    void AddPoint(const Vector Point);
    // Clear all the points
    void Clear();
    // Add a groundframe to the groundframe list
    void AddGroundFrame(bool grounded);
    // Display the trace in the world
    void DrawInWorld(float time) const;
    // Display XY-speed delta overlay
    void DrawSpeedDeltas(HudContext *ctx) const;
};

extern PlayerTrace* playerTrace;
