#include "PlayerTrace.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include <vector>

PlayerTrace *playerTrace;

#ifdef _WIN32
#	define ADD_LINE_OVERLAY(...) engine->AddLineOverlay(__VA_ARGS__)
#else
#	define ADD_LINE_OVERLAY(...) engine->AddLineOverlay(nullptr, __VA_ARGS__)
#endif

Variable sar_player_trace_autoclear("sar_player_trace_autoclear", "1", "Automatically clear the trace on session start\n");

Variable sar_player_trace_draw("sar_player_trace_draw", "0", "Display the recorded player trace. Requires cheats\n");
Variable sar_player_trace_draw_through_walls("sar_player_trace_draw_through_walls", "1", "Display the player trace through walls. Requires sar_player_trace_draw\n");
Variable sar_player_trace_draw_speed_deltas("sar_player_trace_draw_speed_deltas", "1", "Display the speed deltas. Requires sar_player_trace_draw\n");

// Index of the index we are currently recording at
// 0 stands for not recording
static unsigned recording_trace_to = 0;

PlayerTrace::PlayerTrace() {
	this->hasLoaded = true;
}
void PlayerTrace::AddPoint(size_t trace_idx, void *player) {
	// Multiply by the scaling factor for fixed point
	auto pos = client->GetAbsOrigin(player);

	if (traces.count(trace_idx) == 0) {
		traces[trace_idx] = Trace();
		traces[trace_idx].last_pos = pos;
	}
	Trace &trace = traces[trace_idx];
	auto delta = pos - trace.last_pos;

	unsigned ground_handle = *(unsigned *)((uintptr_t)player + Offsets::C_m_hGroundEntity);
	bool grounded = ground_handle != 0xFFFFFFFF;

	bool update = (trace.updates.size() == 0) || (grounded != trace.updates.back().grounded) || (delta.Length() > 127);
	if (update) {
		trace.updates.push_back({trace.deltas.size(), client->GetLocalVelocity(player).Length2D(), (int32_t)pos.x * TRACE_SCALE_UPDATE, (int32_t)pos.y * TRACE_SCALE_UPDATE, (int32_t)pos.z * TRACE_SCALE_UPDATE, grounded});
		delta = {0, 0, 0};
	}

	bool new_pos_needed = update || (session->GetTick() % 2 == 0);
	if (new_pos_needed) {
		trace.deltas.push_back(TraceDelta(delta * TRACE_SCALE_DELTA));
	}
	trace.last_pos = pos;
}
void PlayerTrace::Clear(const size_t trace_idx) {
	traces.erase(trace_idx);
}
void PlayerTrace::ClearAll() {
	traces.clear();
}
void PlayerTrace::DrawInWorld(float time) const {
	int r, g, b;
	bool draw_through_walls = sar_player_trace_draw_through_walls.GetBool();

	for (const auto [trace_idx, trace] : traces) {
		if (trace.deltas.size() < 2) continue;

		size_t update_idx = 1;
		bool is_grounded = trace.updates[0].grounded;
		Vector pos = Vector(trace.updates[0].x, trace.updates[0].y, trace.updates[0].z) / TRACE_SCALE_UPDATE;
		for (size_t i = 1; i < trace.deltas.size(); i++) {
			Vector new_pos = pos + trace.deltas[i].asVector() / TRACE_SCALE_DELTA;

			if (update_idx < trace.updates.size() && i == trace.updates[update_idx].tick) {
				// We want to show only when there is more than 1 groundframe
				is_grounded = trace.updates[update_idx].grounded && !(update_idx < trace.updates.size() && trace.updates[update_idx + 1].tick == i + 1);
				new_pos = Vector(trace.updates[update_idx].x, trace.updates[update_idx].y, trace.updates[update_idx].z) / TRACE_SCALE_UPDATE;
				update_idx++;
			}

			// Don't draw a line when going through a portal
			if ((pos - new_pos).Length() < 127) {
				if (is_grounded) {
					r = 255;
					g = 0;
					b = 0;
				} else {
					r = 255;
					g = 255;
					b = 255;
				}

				ADD_LINE_OVERLAY(
					pos, new_pos, r, g, b, draw_through_walls, time);
			}
			pos = new_pos;
		}
	}
}
void PlayerTrace::DrawSpeedDeltas(HudContext *ctx) const {
	const Vector hud_offset = {0.0, 0.0, 10.0};
	Vector screen_pos;
	int hud_id = 10;

	for (const auto [trace_idx, trace] : traces) {
		if (trace.updates.size() < 2) continue;

		TraceUpdate begin_update = trace.updates[0];
		for (int i = 1; i < trace.updates.size(); i++) {
			TraceUpdate end_update = trace.updates[i];

			// Ignore speed delta when only 1 groundframe
			if (begin_update.grounded && (begin_update.tick + 1 == end_update.tick)) continue;

			float speed_delta = end_update.speed - begin_update.speed;
			int begin_tick = begin_update.tick;
			int end_tick = end_update.tick;

			Vector update_pos = Vector(begin_update.x, begin_update.y, begin_update.z) / TRACE_SCALE_UPDATE;
			Vector draw_pos = update_pos + hud_offset;
			for (int j = begin_tick + 1; j < begin_tick + (end_tick - begin_tick) / 2; j++)
				draw_pos = draw_pos + trace.deltas[j].asVector() / TRACE_SCALE_DELTA;

			engine->PointToScreen(draw_pos, screen_pos);
			ctx->DrawElementOnScreen(hud_id++, screen_pos.x, screen_pos.y, "%10.2f", speed_delta);

			begin_update = end_update;
		}
	}
}

ON_EVENT(PRE_TICK) {
	// Record trace
	if (recording_trace_to && !engine->IsGamePaused()) {
		auto nSlot = GET_SLOT();
		auto player = client->GetPlayer(nSlot + 1);
		if (player) {
			playerTrace->AddPoint(recording_trace_to, player);
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
		playerTrace->ClearAll();
}

HUD_ELEMENT2_NO_DISABLE(player_trace_draw_speed, HudType_InGame) {
	if (!sar_player_trace_draw.GetBool()) return;
	if (!sar_player_trace_draw_speed_deltas.GetBool()) return;
	if (!sv_cheats.GetBool()) return;

	playerTrace->DrawSpeedDeltas(ctx);
}

CON_COMMAND(sar_player_trace_clear, "sar_player_trace_clear <index> - Clear the index player trace\n") {
	if (args.ArgC() != 2)
		return console->Print(sar_player_trace_clear.ThisPtr()->m_pszHelpString);

	int trace_idx = std::stoi(args[1]);
	if (trace_idx < 0)
		return console->Print("Trace index must be 0 or positive.\n");

	playerTrace->Clear(trace_idx);
}

CON_COMMAND(sar_player_trace_clear_all, "sar_player_trace_clear_all - Clear all the traces\n") {
	playerTrace->ClearAll();
}

CON_COMMAND(sar_player_trace_record, "sar_player_trace_record <index> - Record the player trace to the given slot. Set to 0 for don't record\n") {
	if (args.ArgC() != 2)
		return console->Print(sar_player_trace_record.ThisPtr()->m_pszHelpString);

	int trace_idx = std::stoi(args[1]);
	if (trace_idx < 0)
		return console->Print("Trace index must be 0 or positive.\n");

	recording_trace_to = trace_idx;
}
