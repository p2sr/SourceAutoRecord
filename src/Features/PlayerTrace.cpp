#include "PlayerTrace.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/OverlayRender.hpp"
#include "Features/Session.hpp"
#include "Features/Tas/TasPlayer.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"

#include <vector>

PlayerTrace *playerTrace;

Variable sar_trace_autoclear("sar_trace_autoclear", "1", "Automatically clear the trace on session start\n");
Variable sar_trace_record("sar_trace_record", "0", 0, "Record the trace to a slot. Set to 0 for not recording\n");

Variable sar_trace_draw("sar_trace_draw", "0", "Display the recorded player trace. Requires cheats\n");
Variable sar_trace_draw_through_walls("sar_trace_draw_through_walls", "1", "Display the player trace through walls. Requires sar_trace_draw\n");
Variable sar_trace_draw_speed_deltas("sar_trace_draw_speed_deltas", "1", "Display the speed deltas. Requires sar_trace_draw\n");
Variable sar_trace_draw_time("sar_trace_draw_time", "3", 0, 3, 
	"Display tick above trace hover info\n"
	"0 = hide tick info\n"
	"1 = ticks since trace recording started\n"
	"2 = session timer\n"
	"3 = TAS timer (if no TAS was played, uses 1 instead)\n"
);
Variable sar_trace_font("sar_trace_font", "0", 0, "Font index to display player trace info in\n");

Variable sar_trace_bbox_at("sar_trace_bbox_at", "-1", -1, "Display a player-sized bbox at the given tick.");
Variable sar_trace_bbox_use_hover("sar_trace_bbox_use_hover", "0", 0, "Move trace bbox to hovered trace point tick on given trace.");

Vector g_playerTraceTeleportLocation;
int g_playerTraceTeleportSlot;
bool g_playerTraceNeedsTeleport = false;

struct TraceHoverInfo {
	size_t tick;
	unsigned trace_idx;
	Vector pos;
	float speed;
};

std::vector<TraceHoverInfo> hovers;

PlayerTrace::PlayerTrace() {
	this->hasLoaded = true;
}
void PlayerTrace::AddPoint(size_t trace_idx, void *player, int slot, bool use_client_offset) {
	if (traces.count(trace_idx) == 0) {
		traces[trace_idx] = Trace();
		traces[trace_idx].startSessionTick = session->GetTick();
	}

	Trace &trace = traces[trace_idx];

	// update this bad boy every tick because it doesn't like being tinkered with at the
	// very beginning of the level. fussy guy, lemme tell ya
	if (tasPlayer->IsRunning()) {
		int ticksSinceStartup = (int)trace.positions[0].size() + 1; // include point we're about to add
		traces[trace_idx].startTasTick = tasPlayer->GetTick() - ticksSinceStartup;
	}
	

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
	auto ducked = *reinterpret_cast<bool *>((uintptr_t)player + Offsets::S_m_bDucked);

	trace.positions[slot].push_back(pos);
	trace.velocities[slot].push_back(vel);
	trace.grounded[slot].push_back(grounded);
	trace.crouched[slot].push_back(ducked);
}
Trace* PlayerTrace::GetTrace(const size_t trace_idx) {
	auto trace = traces.find(trace_idx);
	if (trace == traces.end()) return nullptr;
	return &traces.find(trace_idx)->second;
}
void PlayerTrace::Clear(const size_t trace_idx) {
	traces.erase(trace_idx);
}
void PlayerTrace::ClearAll() {
	traces.clear();
}
void PlayerTrace::DrawInWorld() const {
	if (engine->IsSkipping()) return;

	int r, g, b;
	bool draw_through_walls = sar_trace_draw_through_walls.GetBool();

	hovers.clear();

	Vector cam_pos{0, 0, 0};
	{
		void *player = client->GetPlayer(GET_SLOT()+1);
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
		for (int slot = 0; slot < 2; slot++) {
			if (trace.positions[slot].size() < 2) continue;

			size_t closest_id = 0;
			float closest_dist = 1.0f; // Something stupid high
			Vector closest_pos;
			float closest_vel;

			Vector pos = trace.positions[slot][0];
			float speed = trace.velocities[slot][0].Length2D();
			unsigned groundframes = trace.grounded[slot][0];

			size_t update_idx = 1;
			for (size_t i = 1; i < trace.positions[slot].size(); i++) {
				Vector new_pos = trace.positions[slot][i];
				speed = trace.velocities[slot][i].Length2D();
				
				if (trace.grounded[slot][i]) {
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
							filter.SetPassEntity(server->GetPlayer(GET_SLOT()+1));

							engine->TraceRay(engine->engineTrace->ThisPtr(), ray, MASK_VISIBLE, &filter, &tr);
						}

						if (draw_through_walls || tr.plane.normal.Length() <= 0.9) {
							// Didn't hit anything; use this point
							closest_id = i;
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
						Vector vel = trace.velocities[slot][i];
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

					OverlayRender::addLine(pos, new_pos, {r,g,b}, draw_through_walls);
				}
				if (pos_delta > 0.001) pos = new_pos;
			}

			if (closest_dist < 1.0f) {
				OverlayRender::addBox(closest_pos, {-1, -1, -1}, {1, 1, 1}, {0, 0, 0}, {255, 0, 255, 20});
				hovers.push_back({closest_id, trace_idx, closest_pos, closest_vel});
			}
		}
	}
}
void PlayerTrace::DrawSpeedDeltas() const {
	const Vector hud_offset = {0.0, 0.0, 10.0};

	auto font = scheme->GetFontByID(sar_trace_font.GetInt());

	for (const auto [trace_idx, trace] : traces) {
		for (int slot = 0; slot < 2; slot++) {
			if (trace.velocities[slot].size() < 2) continue;

			size_t last_delta_end = 0;
			unsigned groundframes = trace.grounded[slot][0];
			for (int i = 1; i < trace.velocities[slot].size(); i++) {
				unsigned last_groundframes = groundframes;
				
				if (trace.grounded[slot][i]) {
					groundframes++;
				} else {
					groundframes = 0;
				}

				if ((groundframes == 2) || (!groundframes && last_groundframes>0)) {
					
					float speed_delta = trace.velocities[slot][i].Length2D() - trace.velocities[slot][last_delta_end].Length2D();
					Vector update_pos = trace.positions[slot][(last_delta_end + i) / 2];
					Vector draw_pos = update_pos + hud_offset;

					OverlayRender::addText(draw_pos, 0, 0, Utils::ssprintf("%10.2f", speed_delta), font);

					last_delta_end = i;
				}
			}
		}
	}
}
void PlayerTrace::DrawBboxAt(int tick) const {
	if (engine->IsSkipping()) return;

	static const Vector player_standing_size = {32, 32, 72};
	static const Vector player_ducked_size = {32, 32, 36};
		
	for (int slot = 0; slot < 2; slot++) {
		for (const auto [trace_idx, trace] : traces) {
			if (trace.positions[slot].size() == 0) continue;

			int localtick = tick;

			// Clamp tick to the number of positions in the trace
			if (trace.positions[slot].size() <= localtick)
				localtick = trace.positions[slot].size()-1;
			
			Vector player_size = trace.crouched[slot][localtick] ? player_ducked_size : player_standing_size;
			Vector offset = trace.crouched[slot][localtick] ? Vector{0, 0, 18} : Vector{0, 0, 36};
			
			Vector center = trace.positions[slot][localtick] + offset;
			// We trace a big player bbox and a small box to indicate exactly which tick is displayed
			OverlayRender::addBox(center, -player_size/2, player_size/2, {0, 0, 0}, {255, 255, 0, 20});
			OverlayRender::addBox(trace.positions[slot][localtick], {-1, -1, -1}, {1, 1, 1}, {0, 0, 0}, {0, 255, 0, 20});
		}
	}
}

void PlayerTrace::TeleportAt(size_t trace_idx, int slot, int tick) {
	if (traces.count(trace_idx) == 0) {
		console->Print("No trace with ID %d!\n", trace_idx);
		return;
	}

	switch (sar_trace_draw_time.GetInt()) {
	case 2:
		tick -= traces[trace_idx].startSessionTick;
		break;
	case 3:
		if (traces[trace_idx].startTasTick > 0) tick -= traces[trace_idx].startTasTick;
		break;
	default:
		break;
	}
	
	if (tick < 0) tick = 0;

	if (tick >= traces[trace_idx].positions[slot].size())
		tick = traces[trace_idx].positions[slot].size()-1;

	if (tick < 0) return;

	g_playerTraceTeleportLocation = traces[trace_idx].positions[slot][tick];
	g_playerTraceTeleportSlot = slot;
	g_playerTraceNeedsTeleport = true;
}

ON_EVENT(PROCESS_MOVEMENT) {
	// Record trace
	if (sar_trace_record.GetInt() && !engine->IsGamePaused()) {
		if (engine->IsOrange()) {
			sar_trace_record.SetValue(0);
			console->Print("The trace only works for the host! Turning off trace recording.\n");
		}
		
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
			playerTrace->AddPoint(sar_trace_record.GetInt(), player, event.slot, use_client_offset);
		}
	}
}

ON_EVENT(SESSION_START) {
	if (sar_trace_autoclear.GetBool())
		playerTrace->ClearAll();
}

HUD_ELEMENT2_NO_DISABLE(trace_draw_speed, HudType_InGame) {
	if (!sar_trace_draw.GetBool()) return;
	if (!sar_trace_draw_speed_deltas.GetBool()) return;
	if (!sv_cheats.GetBool()) return;

}

ON_EVENT(RENDER) {
	if (!sar_trace_draw.GetBool()) return;
	if (!sv_cheats.GetBool()) return;

	// overriding the value of sar_trace_bbox_at if hovered position is used
	size_t trace_idx = sar_trace_bbox_use_hover.GetInt();
	if (trace_idx>0) {
		int tick = -1;
		for (auto &h : hovers) {
			if (h.trace_idx == trace_idx) {
				tick = (int)h.tick;
				break;
			}
		}
		sar_trace_bbox_at.SetValue(tick);
	}

	playerTrace->DrawInWorld();

	int tick = sar_trace_bbox_at.GetInt();
	if (tick != -1) {
		playerTrace->DrawBboxAt(tick);
	}

	const Vector hud_offset = {0.0, 0.0, 10.0};

	auto font = scheme->GetFontByID(sar_trace_font.GetInt());
	auto font_height = surface->GetFontHeight(font);

	for (auto &h : hovers) {
		int timeType = sar_trace_draw_time.GetInt();
		if (timeType > 0) {
			int tick = h.tick;
			auto trace = playerTrace->GetTrace(h.trace_idx);
			if (trace) {
				if (timeType == 2) tick += trace->startSessionTick;
				if (timeType == 3 && trace->startTasTick > 0) tick += trace->startTasTick;
			}
			OverlayRender::addText(h.pos + hud_offset, 0, -2*font_height, Utils::ssprintf("tick: %d", tick), font);
		}
		OverlayRender::addText(h.pos + hud_offset, 0, -font_height, Utils::ssprintf("pos: %1.f %.1f %.1f", h.pos.x, h.pos.y, h.pos.z), font);
		OverlayRender::addText(h.pos + hud_offset, 0, 0, Utils::ssprintf("horiz. speed: %.2f", h.speed), font);
	}

	if (sar_trace_draw_speed_deltas.GetBool()) {
		playerTrace->DrawSpeedDeltas();
	}
}

CON_COMMAND(sar_trace_clear, "sar_trace_clear <index> - Clear the index player trace\n") {
	if (args.ArgC() != 2)
		return console->Print(sar_trace_clear.ThisPtr()->m_pszHelpString);

	int trace_idx = std::atoi(args[1]);
	if (trace_idx < 0)
		return console->Print("Trace index must be 0 or positive.\n");

	playerTrace->Clear(trace_idx);
}

CON_COMMAND(sar_trace_clear_all, "sar_trace_clear_all - Clear all the traces\n") {
	playerTrace->ClearAll();
}

CON_COMMAND(sar_trace_teleport_at, "sar_trace_teleport_at <tick> [player slot] [trace index] - teleports the player at the given trace tick on the given trace ID (defaults to 1) in the given slot (defaults to 0).\n") {
	if (!sv_cheats.GetBool()) return;

	if (args.ArgC() < 2 || args.ArgC() > 4)
		return console->Print(sar_trace_teleport_at.ThisPtr()->m_pszHelpString);
	
	size_t trace_idx = (args.ArgC()==4) ? std::atoi(args[3]) : 1;
	int slot = (args.ArgC()>=3 && engine->IsCoop()) ? std::atoi(args[2]) : 0;
	int tick = std::atoi(args[1]);

	if (slot > 1) slot = 1;
	if (slot < 0) slot = 0;

	playerTrace->TeleportAt(trace_idx, slot, tick);
}

CON_COMMAND(sar_trace_export, "sar_trace_export <filename> [trace index] - Export trace data into a csv file.\n") {
	if (args.ArgC() < 2 || args.ArgC() > 3)
		return console->Print(sar_trace_export.ThisPtr()->m_pszHelpString);

	size_t trace_idx = (args.ArgC()==3) ? std::atoi(args[2]) : 1;

	auto trace = playerTrace->GetTrace(trace_idx);

	if (trace == nullptr) {
		console->Print("Invalid trace ID!\n");
		return;
	}

	bool is_coop_trace = trace->positions[0].size() == trace->positions[1].size();
	size_t size = trace->positions[0].size();

	std::string filename = args[1];
	if (filename.length() < 4 || filename.substr(filename.length() - 4, 4) != ".csv") {
		filename += ".csv";
	}

	FILE *f = fopen(filename.c_str(), "w");
	if (!f) {
		console->Print("Could not open file '%s'\n", filename.c_str());
		return;
	}

#ifdef _WIN32
	fputs(MICROSOFT_PLEASE_FIX_YOUR_SOFTWARE_SMHMYHEAD "\n", f);
#endif
	if (!is_coop_trace) {
		fputs("x,y,z,vx,vy,vz,grounded,crouched\n", f);
	} else {
		fputs("blue, x,y,z,vx,vy,vz,grounded,crouched, orange, x,y,z,vx,vy,vz,grounded,crouched\n", f);
	}

	for (int i = 0; i < size; i++) {
		if (is_coop_trace) {
			fputs(",", f);
		}

		auto pos = trace->positions[0][i];
		auto vel = trace->velocities[0][i];
		auto grounded = trace->grounded[0][i];
		auto crouched = trace->crouched[0][i];

		fprintf(
			f, "%f,%f,%f, %f,%f,%f, %s,%s",
			pos.x, pos.y, pos.z,
			vel.x, vel.y, vel.z,
			grounded?"true":"false", crouched?"true":"false"
		);

		if (is_coop_trace) {
			pos = trace->positions[1][i];
			vel = trace->velocities[1][i];
			grounded = trace->grounded[1][i];
			crouched = trace->crouched[1][i];

			fprintf(
				f, ",%f,%f,%f, %f,%f,%f, %s,%s",
				pos.x, pos.y, pos.z,
				vel.x, vel.y, vel.z,
				grounded?"true":"false", crouched?"true":"false"
			);
		}

		fputs("\n", f);
	}

	fclose(f);

	console->Print("Trace successfully exported to '%s'!\n", filename.c_str());
}
