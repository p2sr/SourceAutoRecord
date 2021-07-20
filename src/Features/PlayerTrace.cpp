#include "PlayerTrace.hpp"

#include <vector>

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Client.hpp"

#include "Features/Session.hpp"

#include "Command.hpp"
#include "Event.hpp"

PlayerTrace* playerTrace;

#ifdef _WIN32
#define ADD_LINE_OVERLAY(...) engine->AddLineOverlay(__VA_ARGS__)
#else
#define ADD_LINE_OVERLAY(...) engine->AddLineOverlay(nullptr, __VA_ARGS__)
#endif

Variable sar_player_trace_record("sar_player_trace_record", "0", "Record player trace while set to 1\n");
Variable sar_player_trace_autoclear("sar_player_trace_autoclear", "1", "Automatically clear the trace on session start\n");

Variable sar_player_trace_draw("sar_player_trace_draw", "0", "Display the recorded player trace. Requires cheats\n");
Variable sar_player_trace_draw_through_walls("sar_player_trace_draw_through_walls", "1", "Display the player trace through walls. Requires sar_player_trace_draw\n");
Variable sar_player_trace_draw_speed_deltas("sar_player_trace_draw_speed_deltas", "1", "Display the speed deltas. Requires sar_player_trace_draw\n");

PlayerTrace::PlayerTrace()
{
    this->hasLoaded = true;
}
void PlayerTrace::AddPoint(size_t trace_idx, void *player)
{
    // Multiply by the scaling factor for fixed point
    auto pos = client->GetAbsOrigin(player)*TRACE_SCALE;

    if (traces.count(trace_idx) == 0) {
        traces[trace_idx] = Trace();
        traces[trace_idx].last_pos = pos;
    }
    Trace &trace = traces[trace_idx];

    unsigned ground_handle = *(unsigned *)((uintptr_t)player + Offsets::C_m_hGroundEntity);
	bool grounded = ground_handle != 0xFFFFFFFF;

    bool update = (trace.updates.size() == 0) || (grounded != trace.updates.back().grounded);
    if (update) {
        trace.updates.push_back({
            trace.deltas.size(),
            client->GetLocalVelocity(player).Length2D(),
            (int32_t) pos.x,
            (int32_t) pos.y,
            (int32_t) pos.z,
            grounded
        });
    }
    
    bool new_pos_needed = update || (session->GetTick() % 2 == 0);
    if (new_pos_needed) {
        auto delta = pos - trace.last_pos;
        trace.deltas.push_back(TraceDelta(delta));
    }
    trace.last_pos = pos;
}
void PlayerTrace::Clear(const size_t trace_idx)
{
    traces.erase(trace_idx);
}
void PlayerTrace::DrawInWorld(float time) const
{
    int r, g, b;
    size_t update_idx = 0;
    bool draw_through_walls = sar_player_trace_draw_through_walls.GetBool();

    for (const auto [trace_idx, trace]: traces) {
        if (trace.deltas.size() < 2) continue;
        
        bool is_grounded = trace.updates[0].grounded;
        auto pos = Vector(trace.updates[0].x, trace.updates[0].y, trace.updates[0].z);
        for (size_t i = 1; i < trace.deltas.size(); i++) {
            if (update_idx < trace.updates.size() && i-1 == trace.updates[update_idx].tick) {
                is_grounded = trace.updates[update_idx].grounded;
                update_idx++;
            }

            if (is_grounded) {
                r = 255; g = 0; b = 0;
            } else {
                r = 255; g = 255; b = 255;
            }

            auto new_pos = pos + trace.deltas[i-1].asVector();

            ADD_LINE_OVERLAY(
                pos/TRACE_SCALE, new_pos/TRACE_SCALE,
                r, g, b,
                draw_through_walls,
                time
            );
            pos = new_pos;
        }
    }
}
void PlayerTrace::DrawSpeedDeltas(HudContext *ctx) const
{
    const Vector hud_offset = {0.0, 0.0, 10.0};
    Vector screen_pos;
    int hud_id = 10;

    for (const auto [trace_idx, trace]: traces) {
        if (trace.updates.size() < 2) continue;

        for (int i = 0; i < trace.updates.size()-1; i++) {
            int begin_tick = trace.updates[i].tick;
            int end_tick = trace.updates[i+1].tick;
            float speed_delta = trace.updates[i+1].speed - trace.updates[i].speed;

            Vector update_pos = Vector(trace.updates[i].x, trace.updates[i].y, trace.updates[i].z)/TRACE_SCALE;
            Vector draw_pos = update_pos + hud_offset;
            for (int j = begin_tick+1; j < begin_tick + (end_tick - begin_tick)/2; j++) 
                draw_pos = draw_pos + trace.deltas[j].asVector()/TRACE_SCALE;
            
            engine->PointToScreen(draw_pos/TRACE_SCALE, screen_pos);
            ctx->DrawElementOnScreen(hud_id++, screen_pos.x, screen_pos.y, "%10.2f", speed_delta);
        }
    }
}

ON_EVENT(PRE_TICK) {

    // Record trace
    if (sar_player_trace_record.GetBool() && !engine->IsGamePaused()) {
        auto nSlot = GET_SLOT();
        auto player = client->GetPlayer(nSlot + 1);
        if (player) {
            playerTrace->AddPoint(1, player);
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
        playerTrace->Clear(1);
}

HUD_ELEMENT2_NO_DISABLE(player_trace_draw_speed, HudType_InGame) {

    if (!sar_player_trace_draw.GetBool()) return;
    if (!sar_player_trace_draw_speed_deltas.GetBool()) return;
    if (!sv_cheats.GetBool()) return;

    playerTrace->DrawSpeedDeltas(ctx);
}

CON_COMMAND(sar_player_trace_clear, "sar_player_trace_clear - Clear the recorded player trace\n") {
    playerTrace->Clear(1);
}
