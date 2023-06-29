#include "PlayerTrace.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/Camera.hpp"
#include "Features/EntityList.hpp"
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

Variable sar_trace_autoclear("sar_trace_autoclear", "1", 0, 1, "Automatically clear the trace on session start\n");
Variable sar_trace_override("sar_trace_override", "1", 0, 1, "Clears old trace when you start recording to it instead of recording on top of it.\n");
Variable sar_trace_record("sar_trace_record", "0", "Record the trace to a slot. Set to 0 for not recording\n", 0);
Variable sar_trace_use_shot_eyeoffset("sar_trace_use_shot_eyeoffset", "1", 0, 1, "Uses eye offset and angles accurate for portal shooting.\n");

Variable sar_trace_draw("sar_trace_draw", "0", 0, 1, "Display the recorded player trace. Requires cheats\n");
Variable sar_trace_draw_through_walls("sar_trace_draw_through_walls", "1", "Display the player trace through walls. Requires sar_trace_draw\n");
Variable sar_trace_draw_speed_deltas("sar_trace_draw_speed_deltas", "0", "Display the speed deltas. Requires sar_trace_draw\n");
Variable sar_trace_draw_time("sar_trace_draw_time", "3", 0, 3, 
	"Display tick above trace hover info\n"
	"0 = hide tick info\n"
	"1 = ticks since trace recording started\n"
	"2 = session timer\n"
	"3 = TAS timer (if no TAS was played, uses 1 instead)\n"
);
Variable sar_trace_font_size("sar_trace_font_size", "3.0", 0.1, "The size of text overlaid on recorded traces.\n");

Variable sar_trace_bbox_at("sar_trace_bbox_at", "-1", -1, "Display a player-sized bbox at the given tick.\n");
Variable sar_trace_bbox_use_hover("sar_trace_bbox_use_hover", "0", 0, 1, "Move trace bbox to hovered trace point tick on given trace.\n");
Variable sar_trace_bbox_ent_record("sar_trace_bbox_ent_record", "1", "Record hitboxes of nearby entities in the trace. You may want to disable this if memory consumption gets too high.\n");
Variable sar_trace_bbox_ent_draw("sar_trace_bbox_ent_draw", "1", "Draw hitboxes of nearby entities in the trace.\n");
Variable sar_trace_bbox_ent_dist("sar_trace_bbox_ent_dist", "200", 50, "Distance from which to capture entity hitboxes.\n");

Variable sar_trace_portal_record("sar_trace_portal_record", "1", "Record portal locations.\n");
Variable sar_trace_portal_oval("sar_trace_portal_oval", "0", "Draw trace portals as ovals rather than rectangles.\n");
Variable sar_trace_portal_opacity("sar_trace_portal_opacity", "100", 0, 255, "Opacity of trace portal previews.\n");

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
	std::string trace_name;
	Vector pos;
	float speed;
	float dist;
};

std::vector<TraceHoverInfo> hovers;

PlayerTrace::PlayerTrace() {
	this->lastRecordedTrace = "0";
	this->hasLoaded = true;
}

bool PlayerTrace::ShouldRecord() {
	return IsTraceNameValid(sar_trace_record.GetString()) && !engine->IsGamePaused();
}

bool PlayerTrace::IsTraceNameValid(std::string trace_name) {
	// for legacy reasons, 0 is treated as no recording
	if (trace_name == "0") return false;

	return trace_name.length() > 0;
}

void PlayerTrace::AddPoint(std::string trace_name, void *player, int slot, bool use_client_offset) {
	if (traces.count(trace_name) == 0) {
		traces[trace_name] = Trace();
		traces[trace_name].startSessionTick = session->GetTick();
	}

	Trace &trace = traces[trace_name];

	// update this bad boy every tick because it doesn't like being tinkered with at the
	// very beginning of the level. fussy guy, lemme tell ya
	if (tasPlayer->IsRunning()) {
		int ticksSinceStartup = (int)trace.positions[0].size() + (int)(tasPlayer->GetTick() == 0); // include point we're about to add
		traces[trace_name].startTasTick = tasPlayer->GetTick() - ticksSinceStartup;
	}
	

	Vector pos;
	Vector vel;
	Vector eyepos;
	QAngle angles;

	bool grounded;
	bool ducked;
	if (use_client_offset) {
		grounded = CE(player)->ground_entity();
		ducked = CE(player)->ducked();
		pos = client->GetAbsOrigin(player);
		vel = client->GetLocalVelocity(player);
		//eyepos = pos + client->GetViewOffset(player) + client->GetPortalLocal(player).m_vEyeOffset;
		camera->GetEyePos<false>(slot, eyepos, angles);
	} else {
		grounded = SE(player)->ground_entity();
		ducked = SE(player)->ducked();
		pos = server->GetAbsOrigin(player);
		vel = server->GetLocalVelocity(player);
		//eyepos = pos + server->GetViewOffset(player) + server->GetPortalLocal(player).m_vEyeOffset;
		camera->GetEyePos<true>(slot, eyepos, angles);
	}

	HitboxList hitboxes = ConstructHitboxList(pos);

	trace.positions[slot].push_back(pos);
	trace.angles[slot].push_back(angles);
	trace.eyepos[slot].push_back(eyepos);
	trace.velocities[slot].push_back(vel);
	trace.grounded[slot].push_back(grounded);
	trace.crouched[slot].push_back(ducked);
	trace.hitboxes[slot].push_back(hitboxes);

	// Only do it for one of the slots since we record all the portals in the map at once
	if (slot == 0) {
		PortalLocations portals = ConstructPortalLocations();
		trace.portals.push_back(portals);
	}
}
Trace *PlayerTrace::GetTrace(std::string trace_name) {
	auto trace = traces.find(trace_name);
	if (trace == traces.end()) return nullptr;
	return &traces.find(trace_name)->second;
}
std::string PlayerTrace::GetDefaultTraceName() {
	// first, try to look for currently hovered traces, find the smallest one
	std::string trace_name = "";

	float hoverDist = 1.0f;
	for (auto &h : hovers) {
		if (h.dist < hoverDist) {
			hoverDist = h.dist;
			trace_name = h.trace_name;
		}
	}

	if (trace_name == "" && traces.size() > 0) {
		trace_name = traces.begin()->first;
	}

	return trace_name;
}
int PlayerTrace::GetTraceCount() {
	return traces.size();
}
void PlayerTrace::Clear(std::string trace_name) {
	traces.erase(trace_name);
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

	for (auto it = playerTrace->traces.begin(); it != playerTrace->traces.end(); ++it) {
		std::string trace_name = it->first;
		const Trace &trace = it->second;
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

			for (size_t i = 0; i < trace.positions[slot].size(); i++) {
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
				hovers.push_back({closest_id, trace_name, closest_pos, closest_vel, closest_dist});
			}
		}
	}
}
void PlayerTrace::DrawSpeedDeltas() const {
	const Vector hud_offset = {0.0, 0.0, 10.0};

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

					OverlayRender::addText(update_pos + hud_offset, Utils::ssprintf("%10.2f", speed_delta), sar_trace_font_size.GetFloat(), true, sar_trace_draw_through_walls.GetBool());

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
void PlayerTrace::DrawPortalsAt(int tick) const {
	if (engine->IsSkipping()) return;

	for (const auto &[trace_idx, trace] : traces) {
		if (trace.portals.size() == 0) continue;

		unsigned localtick = tickUserToInternal(tick, trace);

		// Clamp tick to the number of positions in the trace
		if (trace.portals.size() <= localtick)
			localtick = trace.portals.size()-1;

		// Draw portals
		Color blue         { 111, 184, 255, 255 };
		Color orange       { 255, 184,  86, 255 };
		Color atlas_prim   {  32, 128, 210, 255 };
		Color atlas_second {  16,   0, 210, 255 };
		Color pbody_prim   { 255, 180,  32, 255 };
		Color pbody_second {  58,   3,   3, 255 };

		auto drawPortal = [&](Color portalColor, Vector origin, QAngle angles) {
			portalColor.a = (uint8_t)sar_trace_portal_opacity.GetInt();

			// Bump portal by slightly more than DIST_EPSILON
			Vector tmp;
			Math::AngleVectors(angles, &tmp);
			origin = origin + tmp*0.04;

			// Convert to radians!
			double syaw = sin(angles.y * M_PI/180);
			double cyaw = cos(angles.y * M_PI/180);
			double spitch = sin(angles.x * M_PI/180);
			double cpitch = cos(angles.x * M_PI/180);

			// yaw+pitch rotation matrix
			Matrix rot{3, 3, 0};
			rot(0, 0) = cyaw * cpitch;
			rot(0, 1) = -syaw;
			rot(0, 2) = cyaw * spitch;
			rot(1, 0) = syaw * cpitch;
			rot(1, 1) = cyaw;
			rot(1, 2) = syaw * spitch;
			rot(2, 0) = -spitch;
			rot(2, 1) = 0;
			rot(2, 2) = cpitch;

			MeshId mesh = OverlayRender::createMesh(RenderCallback::constant(portalColor), RenderCallback::none);

			if (sar_trace_portal_oval.GetBool()) {
				int tris = 20;
				for (int i = 0; i < tris; ++i) {
					double lang = M_PI * 2 * i / tris;
					double rang = M_PI * 2 * (i + 1) / tris;

					Vector dl(0, 32 * cos(lang), 56 * sin(lang));
					Vector dr(0, 32 * cos(rang), 56 * sin(rang));

					Vector l = origin + rot * dl;
					Vector r = origin + rot * dr;

					OverlayRender::addTriangle(mesh, l, r, origin);
				}
			} else {
				OverlayRender::addQuad(
					mesh,
					origin + rot * Vector{0, -32, -56},
					origin + rot * Vector{0, -32,  56},
					origin + rot * Vector{0,  32,  56},
					origin + rot * Vector{0,  32, -56}
				);
			}

			// Add a little tick on the top of the portal so we can compare
			// orientations
			OverlayRender::addTriangle(
				mesh,
				origin + rot * Vector{0, -5, 56},
				origin + rot * Vector{0, 0, 64},
				origin + rot * Vector{0, 5, 56}
			);
		};

		for (auto portal : trace.portals[localtick].locations) {
			Color color;
			if (portal.is_coop) {
				if (portal.is_atlas)
					color = portal.is_primary? atlas_prim: atlas_second;
				else
					color = portal.is_primary? pbody_prim: pbody_second;
			} else {
				color = portal.is_primary? blue: orange;
			}

			drawPortal(
				color,
				portal.pos,
				portal.ang
			);
		}
	}
}

void PlayerTrace::TeleportAt(std::string trace_name, int slot, int tick, bool eye) {
	if (traces.count(trace_name) == 0) {
		console->Print("No trace named %s!\n", trace_name.c_str());
		return;
	}

	if (tick == -1) {
		tick = sar_trace_bbox_at.GetInt();
	}

	tick = tickUserToInternal(tick, traces[trace_name]);
	if (tick < 0) tick = 0;

	if ((unsigned)tick >= traces[trace_name].positions[slot].size())
		tick = traces[trace_name].positions[slot].size() - 1;

	if (tick < 0) return;

	QAngle angles = traces[trace_name].angles[slot][tick];
	engine->SetAngles(slot, angles);  // FIXME: borked in remote coop
	//FIXME FIXME: for whatever reason it doesn't deal properly with precise angles. Figure out why!

	if (eye) {
		void *player = server->GetPlayer(slot + 1);
		Vector view_off = player ? server->GetViewOffset(player) : Vector{0,0,64};
		g_playerTraceTeleportLocation = traces[trace_name].eyepos[slot][tick] - view_off;
	} else {
		g_playerTraceTeleportLocation = traces[trace_name].positions[slot][tick];
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

		ICollideable *coll = &SE(ent)->collision();

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
				int vert_count = engine->CreateDebugMesh(engine->g_physCollision, phys_coll, &verts);

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

				engine->DestroyDebugMesh(engine->g_physCollision, vert_count, verts);
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

PortalLocations PlayerTrace::ConstructPortalLocations() const {
	if (!sar_trace_portal_record.GetBool()) return PortalLocations{};

	PortalLocations portals;

	for (int i = 0; i < Offsets::NUM_ENT_ENTRIES; ++i) {
		void *ent = server->m_EntPtrArray[i].m_pEntity;
		if (!ent) continue;
		if (server->IsPlayer(ent)) continue;
		if (strcmp(server->GetEntityClassName(ent), "prop_portal")) continue;
		if (!SE(ent)->field<bool>("m_bActivated")) continue;

		PortalLocations::PortalLocation portal;

		portal.pos = server->GetAbsOrigin(ent);
		portal.ang = server->GetAbsAngles(ent);
		portal.is_primary = !SE(ent)->field<bool>("m_bIsPortal2");
		portal.is_coop = engine->IsCoop();

		if (portal.is_coop) {
			CBaseHandle shooter_handle = SE(ent)->field<CBaseHandle>("m_hFiredByPlayer");
			void *shooter = entityList->LookupEntity(shooter_handle);
			portal.is_atlas = shooter == server->GetPlayer(1);
		}

		portals.locations.push_back(portal);
	}

	return portals;
}

ON_EVENT(PROCESS_MOVEMENT) {
	// detect trace switch
	playerTrace->CheckTraceChanged();

	// Record trace
	if (playerTrace->ShouldRecord()) {
		if (engine->IsOrange()) {
			sar_trace_record.SetValue(0);
			console->Print("The trace only works for the host! Turning off trace recording.\n");
			return;
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
			playerTrace->AddPoint(sar_trace_record.GetString(), player, event.slot, use_client_offset);
		}
	}
}

void PlayerTrace::TweakLatestEyeOffsetForPortalShot(CMoveData *moveData, int slot, bool clientside) {

	if (!sar_trace_use_shot_eyeoffset.GetBool()) return;
	if (!ShouldRecord()) return;

	Trace *trace = playerTrace->GetTrace(sar_trace_record.GetString());
	if (trace == nullptr) return;

	// portal shooting position is funky. Basically, shooting happens after movement
	// has happened, but before angles or eye offset are updated (portal gun uses different
	// values for angles than movement, which makes it even weirder). The solution here is
	// to record eye offset right after movement tick processing has happened
	Vector eyepos;
	QAngle angles;
	if (clientside) {
		camera->GetEyePosFromOrigin<false>(slot, moveData->m_vecAbsOrigin, eyepos, angles);
	} else {
		camera->GetEyePosFromOrigin<true>(slot, moveData->m_vecAbsOrigin, eyepos, angles);
	}
	int lastTick = trace->positions[slot].size() - 1;
	trace->eyepos[slot][lastTick] = eyepos;
}

void PlayerTrace::CheckTraceChanged() {
	std::string currentTrace = sar_trace_record.GetString();
	if (currentTrace == lastRecordedTrace) return;
	
	// trace changed!
	lastRecordedTrace = currentTrace;

	// clean old trace if needed
	if (sar_trace_override.GetBool()) {
		Clear(currentTrace);
	}
	
}

ON_EVENT(SESSION_START) {
	if (sar_trace_autoclear.GetBool())
		playerTrace->ClearAll();
}

void PlayerTrace::DrawTraceHud(HudContext *ctx) {
	for (auto it = playerTrace->traces.begin(); it != playerTrace->traces.end(); ++it) {
		const char *name = it->first.c_str();
		const Trace &t = it->second;
		int tick = tickUserToInternal(sar_trace_bbox_at.GetInt(), t);
		for (int slot = 0; slot < 2; slot++) {
			drawTraceInfo(tick, slot, t, [=](const std::string &line) {
				ctx->DrawElement("trace %s %s: %s", name, slot == 1 ? " (orange)" : "", line.c_str());
			});
		}
	}
}

int PlayerTrace::GetTasTraceTick() {
	if (sar_trace_draw_time.GetInt() != 3) return -1;

	int max_tas_tick = -1;

	// we want the highest tastick number that some trace is having its bbox drawn
	// at, i.e. the highest tastick <= sar_trace_bbox_at with some trace being at
	// least that long

	for (auto it = playerTrace->traces.begin(); it != playerTrace->traces.end(); ++it) {
		const Trace &trace = it->second;
		int tick = tickUserToInternal(sar_trace_bbox_at.GetInt(), trace);
		for (int slot = 0; slot < 2; slot++) {
			if ((int)trace.positions[slot].size() <= tick) {
				// we're missing data - correct the tick number to the highest possible
				tick = trace.positions[slot].size() - 1;
			}
			int tas_tick = tickInternalToUser(tick, trace);
			if (tas_tick > max_tas_tick) max_tas_tick = tas_tick;
		}
	}

	return max_tas_tick;
}

HUD_ELEMENT2(trace, "0", "Draws info about current trace bbox tick.\n", HudType_InGame | HudType_Paused) {
	if (!sv_cheats.GetBool()) return;
	playerTrace->DrawTraceHud(ctx);
}

CON_COMMAND(sar_trace_dump, "sar_trace_dump <tick> [player slot] [trace name] - dump the player state from the given trace tick on the given trace ID (defaults to 1) in the given slot (defaults to 0).\n") {
	if (!sv_cheats.GetBool()) return;

	if (args.ArgC() < 2 || args.ArgC() > 4)
		return console->Print(sar_trace_dump.ThisPtr()->m_pszHelpString);

	std::string trace_name = (args.ArgC() == 4) ? args[3] : playerTrace->GetDefaultTraceName();
	int slot = (args.ArgC()>=3 && engine->IsCoop()) ? std::atoi(args[2]) : 0;
	int usertick = std::atoi(args[1]);
	if (usertick == -1) usertick = sar_trace_bbox_at.GetInt();

	if (slot > 1) slot = 1;
	if (slot < 0) slot = 0;

	auto trace = playerTrace->GetTrace(trace_name);
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
	if (sar_trace_bbox_use_hover.GetBool()) {

		// find closest trace
		int tick = -1;
		float dist = 1.0f;
		std::string trace_name = "";
		for (auto &h : hovers) {
			if (h.dist < dist) {
				tick = (int)h.tick;
				dist = h.dist;
				trace_name = h.trace_name;
			}
		}

		auto trace = playerTrace->GetTrace(trace_name);
		if (trace) tick = tickInternalToUser(tick, *trace);
		sar_trace_bbox_at.SetValue(tick);
	}

	playerTrace->DrawInWorld();

	int tick = sar_trace_bbox_at.GetInt();
	if (tick != -1) {
		playerTrace->DrawBboxAt(tick);
		playerTrace->DrawPortalsAt(tick);
	}

	const Vector hud_offset = {0.0, 0.0, 2.0};

	for (auto &h : hovers) {
		std::string hover_str;

		int timeType = sar_trace_draw_time.GetInt();
		if (timeType > 0) {
			int tick = h.tick;
			auto trace = playerTrace->GetTrace(h.trace_name);
			if (trace) {
				tick = tickInternalToUser(tick, *trace);
			}
			hover_str += Utils::ssprintf("tick: %d\n", tick);
		}
		if (playerTrace->GetTraceCount() > 1) {
			hover_str += Utils::ssprintf("trace: %s\n", h.trace_name.c_str());
		}
		hover_str += Utils::ssprintf("pos: %.1f %.1f %.1f\n", h.pos.x, h.pos.y, h.pos.z);
		hover_str += Utils::ssprintf("horiz. speed: %.2f\n", h.speed);

		OverlayRender::addText(h.pos + hud_offset, hover_str, sar_trace_font_size.GetFloat(), true, true);
	}

	if (sar_trace_draw_speed_deltas.GetBool()) {
		playerTrace->DrawSpeedDeltas();
	}
}

CON_COMMAND(sar_trace_clear, "sar_trace_clear <name> - Clear player trace with a given name\n") {
	if (args.ArgC() != 2)
		return console->Print(sar_trace_clear.ThisPtr()->m_pszHelpString);

	const char *trace_name = args[1];
	playerTrace->Clear(trace_name);
}

CON_COMMAND(sar_trace_clear_all, "sar_trace_clear_all - Clear all the traces\n") {
	playerTrace->ClearAll();
}

CON_COMMAND(sar_trace_teleport_at, "sar_trace_teleport_at <tick> [player slot] [trace name] - teleports the player at the given trace tick on the given trace ID (defaults to hovered one or the first one ever made) in the given slot (defaults to 0).\n") {
	if (!sv_cheats.GetBool()) return;

	if (args.ArgC() < 2 || args.ArgC() > 4)
		return console->Print(sar_trace_teleport_at.ThisPtr()->m_pszHelpString);

	std::string trace_name = (args.ArgC() == 4) ? args[3] : playerTrace->GetDefaultTraceName();
	int slot = (args.ArgC()>=3 && engine->IsCoop()) ? std::atoi(args[2]) : 0;
	int tick = std::atoi(args[1]);

	if (slot > 1) slot = 1;
	if (slot < 0) slot = 0;

	playerTrace->TeleportAt(trace_name, slot, tick, false);
}

CON_COMMAND(sar_trace_teleport_eye, "sar_trace_teleport_eye <tick> [player slot] [trace name] - teleports the player to the eye position at the given trace tick on the given trace (defaults to hovered one or the first one ever made) in the given slot (defaults to 0).\n") {
	if (!sv_cheats.GetBool()) return;

	if (args.ArgC() < 2 || args.ArgC() > 4)
		return console->Print(sar_trace_teleport_eye.ThisPtr()->m_pszHelpString);

	std::string trace_name = (args.ArgC() == 4) ? args[3] : playerTrace->GetDefaultTraceName();
	int slot = (args.ArgC()>=3 && engine->IsCoop()) ? std::atoi(args[2]) : 0;
	int tick = std::atoi(args[1]);

	if (slot > 1) slot = 1;
	if (slot < 0) slot = 0;

	playerTrace->TeleportAt(trace_name, slot, tick, true);
}

CON_COMMAND(sar_trace_export, "sar_trace_export <filename> [trace name] - Export trace data into a csv file.\n") {
	if (args.ArgC() < 2 || args.ArgC() > 3)
		return console->Print(sar_trace_export.ThisPtr()->m_pszHelpString);

	std::string trace_name = (args.ArgC() == 3) ? args[2] : playerTrace->GetDefaultTraceName();

	auto trace = playerTrace->GetTrace(trace_name);

	if (trace == nullptr) {
		console->Print("Invalid trace name!\n");
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
