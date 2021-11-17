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

Variable sar_player_trace_autoclear("sar_player_trace_autoclear", "1", "Automatically clear the trace on session start\n");
Variable sar_player_trace_record("sar_player_trace_record", "0", 0, "Record the trace to a slot. Set to 0 for not recording\n");

Variable sar_player_trace_draw("sar_player_trace_draw", "0", "Display the recorded player trace. Requires cheats\n");
Variable sar_player_trace_draw_through_walls("sar_player_trace_draw_through_walls", "1", "Display the player trace through walls. Requires sar_player_trace_draw\n");
Variable sar_player_trace_draw_speed_deltas("sar_player_trace_draw_speed_deltas", "1", "Display the speed deltas. Requires sar_player_trace_draw\n");

Variable sar_player_trace_bbox_at("sar_player_trace_bbox_at", "-1", -1, "Display a player-sized bbox at the given tick.");

struct TraceHoverInfo {
	unsigned trace_idx;
	Vector pos;
	float speed;
};

std::vector<TraceHoverInfo> hovers;

PlayerTrace::PlayerTrace() {
	this->hasLoaded = true;
}
void PlayerTrace::AddPoint(size_t trace_idx, void *player, bool use_client_offset) {
	if (traces.count(trace_idx) == 0) {
		traces[trace_idx] = Trace();
	}
	Trace &trace = traces[trace_idx];

	Vector pos;
	Vector vel;

	unsigned ground_handle;
	if (use_client_offset) {
		ground_handle = *(unsigned *)((uintptr_t)player + Offsets::C_m_hGroundEntity);
		pos = client->GetAbsOrigin(player);
		vel = client->GetLocalVelocity(player);
	} else {
		ground_handle = *(unsigned *)((uintptr_t)player + Offsets::S_m_hGroundEntity);
		pos = server->GetAbsOrigin(player);
		vel = server->GetLocalVelocity(player);
	}
	bool grounded = ground_handle != 0xFFFFFFFF;
	auto ducked = *reinterpret_cast<bool *>((uintptr_t)player + Offsets::m_bDucked);

	trace.positions.push_back(pos);
	trace.velocities.push_back(vel);
	trace.grounded.push_back(grounded);
	trace.crouched.push_back(ducked);
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

	hovers.clear();

	Vector cam_pos{0, 0, 0};
	{
		void *player = client->GetPlayer(1);
		if (player) {
			cam_pos = client->GetAbsOrigin(player) + client->GetViewOffset(player);
		}
	}

	Vector view_vec{0, 0, 0};
	{
		QAngle ang = engine->GetAngles(0);
		view_vec = Vector{
			cosf(DEG2RAD(ang.y)) * cosf(DEG2RAD(ang.x)),
			sinf(DEG2RAD(ang.y)) * cosf(DEG2RAD(ang.x)),
			-sinf(DEG2RAD(ang.x)),
		}.Normalize();
	}

	for (const auto [trace_idx, trace] : traces) {
		if (trace.positions.size() < 2) continue;

		float closest_dist = 1.0f; // Something stupid high
		Vector closest_pos;
		float closest_vel;

		Vector pos = trace.positions[0];
		float speed = trace.velocities[0].Length2D();
		unsigned groundframes = trace.grounded[0];

		size_t update_idx = 1;
		for (size_t i = 1; i < trace.positions.size(); i++) {
			Vector new_pos = trace.positions[i];
			speed = trace.velocities[i].Length2D();
			
			if (trace.grounded[i]) {
				groundframes++;
			} else {
				groundframes = 0;
			}

			if ((new_pos - cam_pos).SquaredLength() < 300*300) {
				// It's close enough to test
				Vector dir = new_pos - cam_pos;
				float dist = fabsf(1 - dir.Normalize().Dot(view_vec));
				if (dist < 0.1 && dist < closest_dist) {
					// Check whether the point is actually visible
					CGameTrace tr;

					if (!draw_through_walls) {
						Ray_t ray;
						ray.m_IsRay = true;
						ray.m_IsSwept = true;
						ray.m_Start = VectorAligned(cam_pos.x, cam_pos.y, cam_pos.z);
						ray.m_Delta = VectorAligned(dir.x, dir.y, dir.z);
						ray.m_StartOffset = VectorAligned();
						ray.m_Extents = VectorAligned();

						CTraceFilterSimple filter;
						filter.SetPassEntity(server->GetPlayer(1));

						engine->TraceRay(engine->engineTrace->ThisPtr(), ray, MASK_VISIBLE, &filter, &tr);
					}

					if (draw_through_walls || tr.plane.normal.Length() <= 0.9) {
						// Didn't hit anything; use this point
						closest_dist = dist;
						closest_pos = new_pos;
						closest_vel = speed;
					}
				}
			}

			// Don't draw a line when going through a portal or 0 length line
			float pos_delta = (pos - new_pos).Length();
			if (pos_delta < 127 && pos_delta > 0.001) {
				// Colors:
				// red: grounded
				// brown: speedlocked
				// yellow: can't turn further
				// green: speed>300
				if (groundframes > 1) {
					r = 255;
					g = 0;
					b = 0;
				} else if (speed > 300) {
					Vector vel = trace.velocities[i];
					if (fabsf(vel.x) >= 150 && fabsf(vel.y) >= 150) { // Speedlocked
						r = 150;
						g = 75;
						b = 0;
					} else if (fabsf(vel.x) >= 60 && fabsf(vel.y) >= 60) { // Max turn
						r = 255;
						g = 220;
						b = 0;
					} else {
						r = 0;
						g = 255;
						b = 0;
					}
				} else {
					r = 255;
					g = 255;
					b = 255;
				}

				engine->AddLineOverlay(
					nullptr,
					pos, new_pos,
					r, g, b,
					draw_through_walls,
					time
				);
			}
			if (pos_delta > 0.001) pos = new_pos;
		}

		if (closest_dist < 1.0f) {
			engine->AddBoxOverlay(nullptr, closest_pos, {-1,-1,-1}, {1,1,1}, {0,0,0}, 255, 0, 255, draw_through_walls, time);
			hovers.push_back({ trace_idx, closest_pos, closest_vel });
		}
	}
}
void PlayerTrace::DrawSpeedDeltas(HudContext *ctx) const {
	const Vector hud_offset = {0.0, 0.0, 10.0};
	Vector screen_pos;
	int hud_id = 10;

	for (const auto [trace_idx, trace] : traces) {
		if (trace.velocities.size() < 2) continue;

		size_t last_delta_end = 0;
		unsigned groundframes = trace.grounded[0];
		for (int i = 1; i < trace.velocities.size(); i++) {
			unsigned last_groundframes = groundframes;
			
			if (trace.grounded[i]) {
				groundframes++;
			} else {
				groundframes = 0;
			}

			if ((groundframes == 2) || (!groundframes && last_groundframes>0)) {
				
				float speed_delta = trace.velocities[i].Length2D() - trace.velocities[last_delta_end].Length2D();
				Vector update_pos = trace.positions[(last_delta_end + i) / 2];
				Vector draw_pos = update_pos + hud_offset;

				engine->PointToScreen(draw_pos, screen_pos);
				ctx->DrawElementOnScreen(hud_id++, screen_pos.x, screen_pos.y, "%10.2f", speed_delta);

				last_delta_end = i;
			}
		}
	}
}
void PlayerTrace::DrawBboxAt(int tick) const {
	static const Vector player_standing_size = {32, 32, 72};
	static const Vector player_ducked_size = {32, 32, 36};

	for (const auto [trace_idx, trace] : traces) {
		// Clamp tick to the number of positions in the trace
		if (trace.positions.size() < tick)
			tick = trace.positions.size()-1;
		
		Vector player_size = trace.crouched[tick] ? player_ducked_size : player_standing_size;
		Vector offset = trace.crouched[tick] ? Vector{0, 0, 18} : Vector{0, 0, 36};
		
		Vector center = trace.positions[tick] + offset;
		// We trace a big player bbox and a small box to indicate exactly which tick is displayed
		engine->AddBoxOverlay(
			nullptr,
			center,
			-player_size/2,
			player_size/2,
			{0, 0, 0},
			255, 255, 0,
			sar_player_trace_draw_through_walls.GetBool(),
			0.05
		);
		engine->AddBoxOverlay(
			nullptr,
			trace.positions[tick],
			{-1,-1,-1},
			{1,1,1},
			{0, 0, 0},
			0, 255, 0,
			sar_player_trace_draw_through_walls.GetBool(),
			0.05
		);
	}
}

ON_EVENT(PROCESS_MOVEMENT) {
	// Record trace
	if (sar_player_trace_record.GetInt() && !engine->IsGamePaused()) {
		void* player = NULL;
		bool use_client_offset;

		// Get info from the server in game (for accuracy)
		// In demos get info from the client
		if (event.server) {
			player = server->GetPlayer(event.slot + 1);
			use_client_offset = false;
		} else {
			player = client->GetPlayer(event.slot + 1);
			use_client_offset = true;
		}

		if (player) {
			playerTrace->AddPoint(sar_player_trace_record.GetInt(), player, use_client_offset);
		}
	}

	// Draw trace
	if (!sar_player_trace_draw.GetBool()) return;
	if (!sv_cheats.GetBool()) return;

	// Kind of an arbitrary number, prevents flickers
	playerTrace->DrawInWorld(.05);
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

HUD_ELEMENT2_NO_DISABLE(player_trace_bbox, HudType_InGame) {
	if (!sar_player_trace_draw.GetBool()) return;
	if (!sv_cheats.GetBool()) return;

	int tick = sar_player_trace_bbox_at.GetInt();
	if (tick == -1) return;

	playerTrace->DrawBboxAt(tick);
}

HUD_ELEMENT2_NO_DISABLE(player_trace_draw_hover, HudType_InGame) {
	if (!sar_player_trace_draw.GetBool()) return;
	if (!sv_cheats.GetBool()) return;

	const Vector hud_offset = {0.0, 0.0, 10.0};
	int hud_id = 70;
	Vector screen_pos;

	for (auto &h : hovers) {
		engine->PointToScreen(h.pos + hud_offset, screen_pos);
		ctx->DrawElementOnScreen(hud_id++, screen_pos.x, screen_pos.y - 15, "pos: %.1f %.1f %.1f", h.pos.x, h.pos.y, h.pos.z);
		ctx->DrawElementOnScreen(hud_id++, screen_pos.x, screen_pos.y, "horiz. speed: %.2f", h.speed);
	}
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
