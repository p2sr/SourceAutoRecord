#include "PlayerTrace.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/Camera.hpp"
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
Variable sar_trace_bbox_ent_record("sar_trace_bbox_ent_record", "1", "Record hitboxes of nearby entities in the trace. You may want to disable this if memory consumption gets too high.\n");
Variable sar_trace_bbox_ent_draw("sar_trace_bbox_ent_draw", "1", "Draw hitboxes of nearby entities in the trace.\n");
Variable sar_trace_bbox_ent_dist("sar_trace_bbox_ent_dist", "200", 50, "Distance from which to capture entity hitboxes.");

Vector g_playerTraceTeleportLocation;
int g_playerTraceTeleportSlot;
bool g_playerTraceNeedsTeleport = false;

static int tickInternalToUser(int tick, const Trace &trace) {
	if (tick == -1) return -1;
	switch (sar_trace_draw_time.GetInt()) {
	case 2:
		return tick + trace.startSessionTick;
	case 3:
		if (trace.startTasTick > 0) return tick + trace.startTasTick;
	default: // FALLTHROUGH
		return tick;
	}
}

static int tickUserToInternal(int tick, const Trace &trace) {
	if (tick == -1) return -1;
	switch (sar_trace_draw_time.GetInt()) {
	case 2:
		return tick - trace.startSessionTick;
	case 3:
		if (trace.startTasTick > 0) return tick - trace.startTasTick;
	default: // FALLTHROUGH
		return tick;
	}
}

// takes internal tick
static void drawTraceInfo(int tick, int slot, const Trace &trace, std::function<void (const std::string &)> drawCbk) {
	if (trace.positions[slot].size() <= (unsigned)tick) return;

	int usertick = tickInternalToUser(tick, trace);
	const int p = 6; // precision

	Vector pos = trace.positions[slot][tick];
	Vector eyepos = trace.eyepos[slot][tick];
	QAngle ang = trace.angles[slot][tick];
	Vector vel = trace.velocities[slot][tick];
	bool grounded = trace.grounded[slot][tick];
	float velang = RAD2DEG(atan2(vel.y, vel.x));

	drawCbk(Utils::ssprintf("tick: %d", usertick));
	drawCbk(Utils::ssprintf("pos: %.*f %.*f %.*f", p, pos.x, p, pos.y, p, pos.z));
	drawCbk(Utils::ssprintf("eyepos: %.*f %.*f %.*f", p, eyepos.x, p, eyepos.y, p, eyepos.z));
	drawCbk(Utils::ssprintf("ang: %.*f %.*f %.*f", p, ang.x, p, ang.y, p, ang.z));
	drawCbk(Utils::ssprintf("vel: %.*f %.*f (%.*f) %.*f", p, vel.x, p, vel.y, p, vel.Length2D(), p, vel.z));
	drawCbk(Utils::ssprintf("velang: %.*f", p, velang));
	drawCbk(Utils::ssprintf("grounded: %s", grounded ? "yes" : "no"));
}

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
	Vector eyepos;
	QAngle angles;

	unsigned ground_handle;
	if (use_client_offset) {
		ground_handle = *(unsigned *)((uintptr_t)player + Offsets::C_m_hGroundEntity);
		pos = client->GetAbsOrigin(player);
		vel = client->GetLocalVelocity(player);
		eyepos = pos + client->GetViewOffset(player) + client->GetPortalLocal(player).m_vEyeOffset;
		camera->GetEyePos(slot, false, eyepos, angles);
	} else {
		ground_handle = *(unsigned *)((uintptr_t)player + Offsets::S_m_hGroundEntity);
		pos = server->GetAbsOrigin(player);
		vel = server->GetLocalVelocity(player);
		eyepos = pos + server->GetViewOffset(player) + server->GetPortalLocal(player).m_vEyeOffset;
		camera->GetEyePos(slot, true, eyepos, angles);
	}
	bool grounded = ground_handle != 0xFFFFFFFF;
	auto ducked = *reinterpret_cast<bool *>((uintptr_t)player + Offsets::S_m_bDucked);

	HitboxList hitboxes = ConstructHitboxList(pos);

	trace.positions[slot].push_back(pos);
	trace.angles[slot].push_back(angles);
	trace.eyepos[slot].push_back(eyepos);
	trace.velocities[slot].push_back(vel);
	trace.grounded[slot].push_back(grounded);
	trace.crouched[slot].push_back(ducked);
	trace.hitboxes[slot].push_back(hitboxes);
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

	for (const auto &[trace_idx, trace] : traces) {
		for (int slot = 0; slot < 2; slot++) {
			if (trace.positions[slot].size() < 2) continue;

			size_t closest_id = 0;
			float closest_dist = 1.0f; // Something stupid high
			Vector closest_pos;
			float closest_vel;

			MeshId mesh_airlocked = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 150, 75,  0   }, draw_through_walls));
			MeshId mesh_max_turn  = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 255, 220, 0   }, draw_through_walls));
			MeshId mesh_under300  = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 255, 255, 255 }, draw_through_walls));
			MeshId mesh_over300   = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 0,   255, 0   }, draw_through_walls));
			MeshId mesh_grounded  = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 255, 0,   0   }, draw_through_walls));

			Vector pos = trace.positions[slot][0];
			float speed = trace.velocities[slot][0].Length2D();
			unsigned groundframes = trace.grounded[slot][0];

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
					Vector vel = trace.velocities[slot][i];
					MeshId mesh =
						groundframes > 1 ? mesh_grounded :
						speed < 300      ? mesh_under300 :
						fabsf(vel.x) >= 150 && fabsf(vel.y) >= 150 ? mesh_airlocked :
						fabsf(vel.x) >= 60  && fabsf(vel.y) >= 60  ? mesh_max_turn :
						mesh_over300;

					OverlayRender::addLine(mesh, pos, new_pos);
				}
				if (pos_delta > 0.001) pos = new_pos;
			}

			if (closest_dist < 1.0f) {
				OverlayRender::addBoxMesh(
					closest_pos,
					{-1, -1, -1},
					{1, 1, 1},
					{0, 0, 0},
					RenderCallback::constant({255, 0, 255, 20},  draw_through_walls),
					RenderCallback::constant({255, 0, 255, 255}, draw_through_walls)
				);
				hovers.push_back({closest_id, trace_idx, closest_pos, closest_vel});
			}
		}
	}
}
void PlayerTrace::DrawSpeedDeltas() const {
	const Vector hud_offset = {0.0, 0.0, 10.0};

	auto font = scheme->GetFontByID(sar_trace_font.GetInt());

	for (const auto &[trace_idx, trace] : traces) {
		for (int slot = 0; slot < 2; slot++) {
			if (trace.velocities[slot].size() < 2) continue;

			size_t last_delta_end = 0;
			unsigned groundframes = trace.grounded[slot][0];
			for (unsigned i = 1; i < trace.velocities[slot].size(); i++) {
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
		for (const auto &[trace_idx, trace] : traces) {
			if (trace.positions[slot].size() == 0) continue;

			unsigned localtick = tickUserToInternal(tick, trace);

			// Clamp tick to the number of positions in the trace
			if (trace.positions[slot].size() <= localtick)
				localtick = trace.positions[slot].size()-1;

			Vector eyepos = trace.eyepos[slot][localtick];
			QAngle angles = trace.angles[slot][localtick];
			Vector forward;
			Math::AngleVectors(angles, &forward);
			
			Vector player_size = trace.crouched[slot][localtick] ? player_ducked_size : player_standing_size;
			Vector offset = trace.crouched[slot][localtick] ? Vector{0, 0, 18} : Vector{0, 0, 36};
			
			Vector center = trace.positions[slot][localtick] + offset;
			// We trace a big player bbox and a small box to indicate exactly which tick is displayed
			OverlayRender::addBoxMesh(
				center,
				-player_size/2,
				player_size/2,
				{0, 0, 0},
				RenderCallback::constant({255, 255, 0, 20}),
				RenderCallback::constant({255, 255, 0, 255})
			);
			OverlayRender::addBoxMesh(
				trace.positions[slot][localtick],
				{-1, -1, -1},
				{1, 1, 1},
				{0, 0, 0},
				RenderCallback::constant({0, 255, 0, 20}),
				RenderCallback::constant({0, 255, 0, 255})
			);
			MeshId eyeLine = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({0, 255, 255}));
			OverlayRender::addLine(eyeLine, eyepos, eyepos + forward*50.0);
			OverlayRender::addBoxMesh(eyepos, {-1,-1,-1}, {1,1,1}, angles, RenderCallback::constant({0, 255, 255}), RenderCallback::none);

			if (sar_trace_bbox_ent_draw.GetBool()) {
				auto &boxes = trace.hitboxes[slot][localtick];

				for (auto &vphys : boxes.vphys) {
					MeshId mesh = OverlayRender::createMesh(
						RenderCallback::constant({ 255, 0, 0, 20  }),
						RenderCallback::constant({ 255, 0, 0, 255 })
					);
					for (size_t i = 0; i < vphys.verts.size(); i += 3) {
						Vector a = vphys.verts[i+0];
						Vector b = vphys.verts[i+1];
						Vector c = vphys.verts[i+2];
						OverlayRender::addTriangle(mesh, a, b, c, true);
					}
				}

				for (auto &bsp : boxes.bsps) {
					MeshId mesh = OverlayRender::createMesh(
						RenderCallback::constant({ 0, 0, 255, 20  }),
						RenderCallback::constant({ 0, 0, 255, 255 })
					);
					for (size_t i = 0; i < bsp.verts.size(); i += 3) {
						Vector a = bsp.verts[i+0];
						Vector b = bsp.verts[i+1];
						Vector c = bsp.verts[i+2];
						OverlayRender::addTriangle(mesh, a, b, c, true);
					}
				}

				for (auto &obb : boxes.obb) {
					OverlayRender::addBoxMesh(
						obb.pos, obb.mins, obb.maxs, obb.ang,
						RenderCallback::constant({ 0, 255, 0, 20 }),
						RenderCallback::constant({ 0, 255, 0, 255 })
					);
				}
			}
		}
	}
}

void PlayerTrace::TeleportAt(size_t trace_idx, int slot, int tick, bool eye) {
	if (traces.count(trace_idx) == 0) {
		console->Print("No trace with ID %d!\n", trace_idx);
		return;
	}

	if (tick == -1) {
		tick = sar_trace_bbox_at.GetInt();
	}

	tick = tickUserToInternal(tick, traces[trace_idx]);
	if (tick < 0) tick = 0;

	if ((unsigned)tick >= traces[trace_idx].positions[slot].size())
		tick = traces[trace_idx].positions[slot].size()-1;

	if (tick < 0) return;

	engine->SetAngles(slot, traces[trace_idx].angles[slot][tick]); // FIXME: borked in remote coop

	if (eye) {
		void *player = server->GetPlayer(slot + 1);
		Vector view_off = player ? server->GetViewOffset(player) : Vector{0,0,72};
		g_playerTraceTeleportLocation = traces[trace_idx].eyepos[slot][tick] - view_off;
	} else {
		g_playerTraceTeleportLocation = traces[trace_idx].positions[slot][tick];
	}

	g_playerTraceTeleportSlot = slot;
	g_playerTraceNeedsTeleport = true;
}

HitboxList PlayerTrace::ConstructHitboxList(Vector center) const {
	if (!sar_trace_bbox_ent_record.GetBool()) return HitboxList{};

	const float d = sar_trace_bbox_ent_dist.GetFloat();

	Vector incl_mins = center - Vector{d, d, d};
	Vector incl_maxs = center + Vector{d, d, d};

	HitboxList list;

	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;
		if (server->IsPlayer(ent)) continue;

		ICollideable *coll = (ICollideable *)((uintptr_t)ent + Offsets::S_m_Collision);

		if (coll->GetSolidFlags() & FSOLID_NOT_SOLID) continue;

		Vector mins, maxs;
		coll->WorldSpaceSurroundingBounds(&mins, &maxs);
		if (maxs.x < incl_mins.x || mins.x > incl_maxs.x) continue;
		if (maxs.y < incl_mins.y || mins.y > incl_maxs.y) continue;
		if (maxs.z < incl_mins.z || mins.z > incl_maxs.z) continue;

		switch (coll->GetSolid()) {
		case SOLID_BBOX:
			list.obb.push_back(HitboxList::ObbBox{mins, maxs, {0,0,0}, {0,0,0}});
			break;
		case SOLID_OBB:
		case SOLID_OBB_YAW:
			list.obb.push_back(HitboxList::ObbBox{
				coll->OBBMins(),
				coll->OBBMaxs(),
				coll->GetCollisionOrigin(),
				coll->GetCollisionAngles(),
			});
			break;
		case SOLID_BSP:
		case SOLID_VPHYSICS:
			{
				IPhysicsObject *phys = coll->GetVPhysicsObject();
				if (!phys) break;

				using _GetCollide = CPhysCollide *(__rescall *)(const void *thisptr);
				auto GetCollide = Memory::VMT<_GetCollide>(phys, Offsets::GetCollide);
				const CPhysCollide *phys_coll = GetCollide(phys);

				Vector *verts;
				int vert_count = Engine::CreateDebugMesh(engine->g_physCollision, phys_coll, &verts);

				matrix3x4_t trans = coll->CollisionToWorldTransform();

				std::vector<Vector> verts_copy(vert_count);
				for (int i = 0; i < vert_count; ++i) {
					verts_copy[i] = trans.VectorTransform(verts[i]);
				}

				if (coll->GetSolid() == SOLID_VPHYSICS) {
					list.vphys.push_back(HitboxList::VphysBox{verts_copy});
				} else {
					list.bsps.push_back(HitboxList::VphysBox{verts_copy});
				}

				Engine::DestroyDebugMesh(engine->g_physCollision, vert_count, verts);
			}
			break;
		case SOLID_NONE:
		case SOLID_CUSTOM:
		default:
			break;
		}
	}

	return list;
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

void PlayerTrace::DrawTraceHud(HudContext *ctx) {
	for (const auto &[trace_idx, trace] : playerTrace->traces) {
		int tick = tickUserToInternal(sar_trace_bbox_at.GetInt(), trace);
		for (int slot = 0; slot < 2; slot++) {
			drawTraceInfo(tick, slot, trace, [=](const std::string &line) {
				ctx->DrawElement("trace %d%s: %s", trace_idx, slot == 1 ? " (orange)" : "", line.c_str());
			});
		}
	}
}

HUD_ELEMENT2(trace, "0", "Draws info about current trace bbox tick.\n", HudType_InGame | HudType_Paused) {
	if (!sv_cheats.GetBool()) return;
	playerTrace->DrawTraceHud(ctx);
}

CON_COMMAND(sar_trace_dump, "sar_trace_dump <tick> [player slot] [trace index] - dump the player state from the given trace tick on the given trace ID (defaults to 1) in the given slot (defaults to 0).\n") {
	if (!sv_cheats.GetBool()) return;

	if (args.ArgC() < 2 || args.ArgC() > 4)
		return console->Print(sar_trace_dump.ThisPtr()->m_pszHelpString);

	size_t trace_idx = (args.ArgC()==4) ? std::atoi(args[3]) : 1;
	int slot = (args.ArgC()>=3 && engine->IsCoop()) ? std::atoi(args[2]) : 0;
	int usertick = std::atoi(args[1]);
	if (usertick == -1) usertick = sar_trace_bbox_at.GetInt();

	if (slot > 1) slot = 1;
	if (slot < 0) slot = 0;

	auto trace = playerTrace->GetTrace(trace_idx);
	if (trace) {
		int tick = tickUserToInternal(usertick, *trace);
		drawTraceInfo(tick, slot, *trace, [](const std::string &line) {
			console->Print("%s\n", line.c_str());
		});
	}
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
		auto trace = playerTrace->GetTrace(trace_idx);
		if (trace) tick = tickInternalToUser(tick, *trace);
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
				tick = tickInternalToUser(tick, *trace);
			}
			OverlayRender::addText(h.pos + hud_offset, 0, -2*font_height, Utils::ssprintf("tick: %d", tick), font);
		}
		OverlayRender::addText(h.pos + hud_offset, 0, -font_height, Utils::ssprintf("pos: %.1f %.1f %.1f", h.pos.x, h.pos.y, h.pos.z), font);
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

	playerTrace->TeleportAt(trace_idx, slot, tick, false);
}

CON_COMMAND(sar_trace_teleport_eye, "sar_trace_teleport_eye <tick> [player slot] [trace index] - teleports the player to the eye position at the given trace tick on the given trace ID (defaults to 1) in the given slot (defaults to 0).\n") {
	if (!sv_cheats.GetBool()) return;

	if (args.ArgC() < 2 || args.ArgC() > 4)
		return console->Print(sar_trace_teleport_eye.ThisPtr()->m_pszHelpString);

	size_t trace_idx = (args.ArgC()==4) ? std::atoi(args[3]) : 1;
	int slot = (args.ArgC()>=3 && engine->IsCoop()) ? std::atoi(args[2]) : 0;
	int tick = std::atoi(args[1]);

	if (slot > 1) slot = 1;
	if (slot < 0) slot = 0;

	playerTrace->TeleportAt(trace_idx, slot, tick, true);
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

	for (size_t i = 0; i < size; i++) {
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
