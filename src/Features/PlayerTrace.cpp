#include "PlayerTrace.hpp"

#include <vector>

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Client.hpp"

#include "Features/GroundFramesCounter.hpp"

#include "Command.hpp"
#include "Event.hpp"

PlayerTrace* playerTrace;

#ifdef _WIN32
#define ADD_LINE_OVERLAY(...) engine->AddLineOverlay(__VA_ARGS__)
#else
#define ADD_LINE_OVERLAY(...) engine->AddLineOverlay(nullptr, __VA_ARGS__)
#endif

Variable sar_player_trace_record("sar_player_trace_record", "0", "Record player trace while set to 1\n");
Variable sar_player_trace_size("sar_player_trace_size", "0", 0, "Maximum size of the player trace (in ticks), 0 for unlimited\n");
Variable sar_player_trace_autoclear("sar_player_trace_autoclear", "1", "Automatically clear the trace on session start\n");

Variable sar_player_trace_draw("sar_player_trace_draw", "0", "Display the recorded player trace. Requires cheats\n");
Variable sar_player_trace_draw_through_walls("sar_player_trace_draw_through_walls", "1", "Display the player trace through walls. Requires sar_player_trace_draw\n");
Variable sar_player_trace_draw_speed_deltas("sar_player_trace_draw_speed_deltas", "1", "Display the speed deltas. Requires sar_player_trace_draw\n");

PlayerTrace::PlayerTrace()
{
    this->hasLoaded = true;
}
void PlayerTrace::AddPoint(const Vector point)
{
    if (sar_player_trace_size.GetInt() && (trace.size() > sar_player_trace_size.GetInt())) {
        trace.erase(trace.begin());
        for (size_t &swap: groundframe_swaps) {
            swap--;
        }
        if (groundframe_swaps[0] == -1)
            groundframe_swaps.erase(groundframe_swaps.begin());
    }

    trace.push_back(point);
}
void PlayerTrace::Clear()
{
    this->trace.clear();
    this->groundframe_swaps.clear();
}
void PlayerTrace::AddGroundFrame(bool grounded)
{
    static bool last_grounded_state;

    // If this is a new trace, init the variables
    if (trace.size() == 1) {
        started_grounded = grounded;
        last_grounded_state = grounded;
    }
    
    if (last_grounded_state != grounded)
        this->groundframe_swaps.push_back(trace.size());
    
    last_grounded_state = grounded;
}
void PlayerTrace::DrawInWorld(float time) const
{
    int r, g, b;
    size_t groundframes_idx = 0;
    bool draw_through_walls = sar_player_trace_draw_through_walls.GetBool();

    if (trace.size() == 0) return;

    bool is_grounded = started_grounded;
    for (size_t i = 0; i < trace.size()-1; i++) {
        if (groundframes_idx < groundframe_swaps.size() && groundframe_swaps[groundframes_idx] == i) {
            groundframes_idx++;
            is_grounded = !is_grounded;
        }

        if (is_grounded) {
            r = 255; g = 0; b = 0;
        } else {
            r = 255; g = 255; b = 255;
        }

        ADD_LINE_OVERLAY(
            trace[i], trace[i+1],
            r, g, b,
            draw_through_walls,
            time
        );
    }
}
void PlayerTrace::DrawSpeedDeltas(HudContext *ctx) const
{
    if (trace.size() < 5) return;

    int hud_id = 10;

    switch (groundframe_swaps.size()) {
        case 0:
            DrawSpeedDelta(ctx, &hud_id, 0, trace.size()-1);
            break;
        case 1:
            DrawSpeedDelta(ctx, &hud_id, 0, groundframe_swaps[0]);
            DrawSpeedDelta(ctx, &hud_id, groundframe_swaps[0], trace.size()-1);
            break;
        default:
            DrawSpeedDelta(ctx, &hud_id, 0, groundframe_swaps[0]);
            DrawSpeedDelta(ctx, &hud_id, groundframe_swaps[groundframe_swaps.size()-1], trace.size()-1);
            for (size_t i = 1; i < groundframe_swaps.size(); i++) {
                DrawSpeedDelta(ctx, &hud_id, groundframe_swaps[i-1], groundframe_swaps[i]);
            }
            break;
    }
}
void PlayerTrace::DrawSpeedDelta(HudContext *ctx, int *hud_id, size_t begin_tick, size_t end_tick) const {
    const Vector hud_offset = {0.0, 0.0, 10.0};
    const size_t idx_offset = 2; // To account for alt ticks
    Vector screen_pos;

    if (begin_tick == 0) begin_tick += idx_offset;
    Vector speed_a = (trace[begin_tick - idx_offset] - trace[begin_tick])/idx_offset;
    Vector speed_b = (trace[end_tick - idx_offset] - trace[end_tick])/idx_offset;
    float speed_delta = speed_b.Length2D() - speed_a.Length2D();

    engine->PointToScreen(trace[begin_tick + (end_tick - begin_tick)/2] + hud_offset, screen_pos);
    ctx->DrawElementOnScreen((*hud_id)++, screen_pos.x, screen_pos.y, "%10.2f", speed_delta * 60);
}

ON_EVENT(PRE_TICK) {

    // Record trace
    if (sar_player_trace_record.GetBool() && !engine->IsGamePaused()) {
        auto nSlot = GET_SLOT();
        auto player = client->GetPlayer(nSlot + 1);
        if (player) {
            playerTrace->AddPoint(client->GetAbsOrigin(player));
            playerTrace->AddGroundFrame(groundFramesCounter->grounded[nSlot]);
        }
    }

    // Draw trace
    const int drawDelta = 30;
    static int lastDrawTick = -1000;

    if (!sar_player_trace_draw.GetBool()) return;
    if (!sv_cheats.GetBool()) return;

    int tick = engine->GetTick();
    if (tick > lastDrawTick && tick < lastDrawTick + drawDelta) return;

    lastDrawTick = tick;
    
    playerTrace->DrawInWorld((drawDelta + 1) * *engine->interval_per_tick);
}

ON_EVENT(SESSION_START) {
    if (sar_player_trace_autoclear.GetBool())
        playerTrace->Clear();
}

HUD_ELEMENT2_NO_DISABLE(player_trace_draw_speed, HudType_InGame) {

    if (!sar_player_trace_draw.GetBool()) return;
    if (!sar_player_trace_draw_speed_deltas.GetBool()) return;
    if (!sv_cheats.GetBool()) return;

    playerTrace->DrawSpeedDeltas(ctx);
}

CON_COMMAND(sar_player_trace_clear, "sar_player_trace_clear - Clear the recorded player trace\n") {
    playerTrace->Clear();
}
