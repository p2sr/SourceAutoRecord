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

Variable sar_player_trace_draw("sar_player_trace_draw", "0", "Display the recorded player trace\n");
Variable sar_player_trace_record("sar_player_trace_record", "0", "Record player trace while set to 1\n");
Variable sar_player_trace_size("sar_player_trace_size", "0", 0, "Maximum size of the player trace (in ticks), 0 for unlimited\n");

PlayerTrace::PlayerTrace()
{
    this->hasLoaded = true;
}
void PlayerTrace::AddPoint(const Vector point)
{
    if (sar_player_trace_size.GetInt() && (trace.size() > sar_player_trace_size.GetInt())) {
        trace.erase(trace.begin());
        for (size_t &groundframe: groundframes) {
            groundframe--;
        }
        if (groundframes[0] == -1)
            groundframes.erase(groundframes.begin());
    }

    trace.push_back(point);
}
void PlayerTrace::Clear()
{
    this->trace.clear();
    this->groundframes.clear();
}
void PlayerTrace::AddGroundFrame() {
    this->groundframes.push_back(trace.size());
}
void PlayerTrace::DrawInWorld(float time) const
{
    int r, g, b;
    size_t groundframes_idx = 0;

    if (trace.size() == 0) return;
    for (size_t i = 0; i < trace.size()-1; i++) {

        if (groundframes_idx < groundframes.size() && groundframes[groundframes_idx] == i) {
            groundframes_idx++;
            r = 255; g = 0; b = 0;
        } else {
            r = 255; g = 255; b = 255;
        }

        ADD_LINE_OVERLAY(
            trace[i], trace[i+1],
            r, g, b,
            true,
            time
        );
    }
}
void PlayerTrace::DrawSpeedDeltas(HudContext *ctx) const
{
    if (groundframes.size() == 0) return;

    int hud_id = 10;

    size_t first_groundframe_tick = 0;
    size_t last_groundframe_tick = 0;
    size_t idx_offset = 2; // To account for alt ticks

    // We're only interested in the places where we change grounded state
    for (size_t i = 0; i < groundframes.size(); i++) {
        if (groundframes[i] == last_groundframe_tick + 1) {
            last_groundframe_tick++;
        } else {
            const Vector hud_offset = {0.0, 0.0, 10.0};
            Vector screen_pos;
            Vector speed_a, speed_b;
            float speed_delta;

            if (first_groundframe_tick == 0) first_groundframe_tick += idx_offset;
            if (first_groundframe_tick != last_groundframe_tick) {
                // Display speed delta between first and last groundframe
                speed_a = (trace[first_groundframe_tick - idx_offset] - trace[first_groundframe_tick])/idx_offset;
                speed_b = (trace[last_groundframe_tick - idx_offset] - trace[last_groundframe_tick])/idx_offset;
                speed_delta = sqrt(speed_b.x*speed_b.x + speed_b.y*speed_b.y) - sqrt(speed_a.x*speed_a.x + speed_a.y*speed_a.y);

                engine->PointToScreen(trace[first_groundframe_tick + (last_groundframe_tick - first_groundframe_tick)/2] + hud_offset, screen_pos);
                ctx->DrawElementOnScreen(hud_id++, screen_pos.x, screen_pos.y, "%10.2f", speed_delta * 60);
            }
            // Display speed delta between last groundframe and current tick
            // aka speed delta for the current hop
            speed_a = (trace[last_groundframe_tick - idx_offset] - trace[last_groundframe_tick])/idx_offset;
            speed_b = (trace[groundframes[i] - idx_offset] - trace[groundframes[i]])/idx_offset;
            speed_delta = sqrt(speed_b.x*speed_b.x + speed_b.y*speed_b.y) - sqrt(speed_a.x*speed_a.x + speed_a.y*speed_a.y);

            engine->PointToScreen(trace[last_groundframe_tick + (groundframes[i] - last_groundframe_tick)/2] + hud_offset, screen_pos);
            ctx->DrawElementOnScreen(hud_id++, screen_pos.x, screen_pos.y, "%10.2f", speed_delta * 60);

            first_groundframe_tick = groundframes[i];
            last_groundframe_tick = groundframes[i];
        }
    }
}

ON_EVENT(PRE_TICK) {

    // Record trace
    if (sar_player_trace_record.GetBool() && !engine->IsGamePaused()) {
        auto nSlot = GET_SLOT();
        auto player = client->GetPlayer(nSlot + 1);
        if (player) {
            playerTrace->AddPoint(client->GetAbsOrigin(player));
            if (groundFramesCounter->grounded[nSlot]) {
                playerTrace->AddGroundFrame();
            }
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
    playerTrace->Clear();
}

HUD_ELEMENT2(player_trace_draw_speed, "1", "Toggles drawing the speed deltas on the player traces\n", HudType_InGame) {

    if (!sar_player_trace_draw.GetBool()) return;
    if (!sv_cheats.GetBool()) return;

    playerTrace->DrawSpeedDeltas(ctx);

}

CON_COMMAND(sar_player_trace_clear, "sar_player_trace_clear - Clear the recorded player trace\n") {
    playerTrace->Clear();
}
