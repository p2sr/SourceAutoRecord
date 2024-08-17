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
#include "Modules/FileSystem.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"

#include <vector>

Variable sar_trace_autoclear("sar_trace_autoclear", "1", 0, 1, "Automatically clear the trace on session start\n");
Variable sar_trace_override("sar_trace_override", "1", 0, 1, "Clears old trace when you start recording to it instead of recording on top of it.\n");
Variable sar_trace_record("sar_trace_record", "0", "Record the trace to a slot. Set to 0 for not recording\n", 0);
Variable sar_trace_use_shot_eyeoffset("sar_trace_use_shot_eyeoffset", "1", 0, 1, "Uses eye offset and angles accurate for portal shooting.\n");

Variable sar_trace_draw("sar_trace_draw", "0", 0, 1, "Display the recorded player trace. Requires cheats\n");
Variable sar_trace_draw_hover("sar_trace_draw_hover", "1", "Display information about the trace at the hovered tick.\n");
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

Vector g_playerTraceTeleportLocation;
int g_playerTraceTeleportSlot;
bool g_playerTraceNeedsTeleport = false;
std::map<std::string, TraceData> traces;
std::string lastRecordedTrace = "0";

static int tickInternalToUser(int tick, const TraceData &trace) {
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

static int tickUserToInternal(int tick, const TraceData &trace) {
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
static void drawTraceInfo(int tick, int slot, const TraceData &trace, std::function<void(const std::string &)> drawCbk) {
	if (!trace.draw) return;
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
	Vector position;
	float speed;
	float dist;
};

std::vector<TraceHoverInfo> hovers;

bool Trace::ShouldRecord() {
	return IsTraceNameValid(sar_trace_record.GetString()) && !engine->IsGamePaused();
}

bool Trace::IsTraceNameValid(std::string trace_name) {
	// for legacy reasons, 0 is treated as no recording
	if (trace_name == "0") return false;

	return trace_name.length() > 0;
}

void Trace::AddPoint(std::string trace_name, void *player, int slot, bool use_client_offset) {
	if (traces.count(trace_name) == 0) {
		traces[trace_name] = TraceData();
		traces[trace_name].startSessionTick = session->GetTick();
	}

	TraceData &trace = traces[trace_name];

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

	Trace::HitboxList hitboxes = Trace::HitboxList::FetchAllNearPosition(pos);

	trace.positions[slot].push_back(pos);
	trace.angles[slot].push_back(angles);
	trace.eyepos[slot].push_back(eyepos);
	trace.velocities[slot].push_back(vel);
	trace.grounded[slot].push_back(grounded);
	trace.crouched[slot].push_back(ducked);
	trace.hitboxes[slot].push_back(hitboxes);

	// Only do it for one of the slots since we record all the portals in the map at once
	if (slot == 0) {
		auto portals = Trace::PortalsList::FetchCurrentLocations();
		trace.portals.push_back(portals);
	}
}
TraceData *Trace::GetTrace(std::string trace_name) {
	auto trace = traces.find(trace_name);
	if (trace == traces.end()) return nullptr;
	return &traces.find(trace_name)->second;
}
std::string Trace::GetDefaultTraceName() {
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
int Trace::GetTraceCount() {
	return traces.size();
}
void Trace::Clear(std::string trace_name) {
	traces.erase(trace_name);
}
void Trace::ClearAll() {
	traces.clear();
}
void Trace::DrawInWorld() {
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

	for (auto it = traces.begin(); it != traces.end(); ++it) {
		std::string trace_name = it->first;
		const TraceData &trace = it->second;
		if (!trace.draw) continue;
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

			if (sar_trace_draw_hover.GetBool() && closest_dist < 1.0f) {
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
void Trace::DrawSpeedDeltas() {
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
void Trace::DrawBboxAt(int tick) {
	if (engine->IsSkipping()) return;

	static const Vector player_standing_size = {32, 32, 72};
	static const Vector player_ducked_size = {32, 32, 36};
		
	for (int slot = 0; slot < 2; slot++) {
		for (const auto &[trace_idx, trace] : traces) {
			if (!trace.draw) continue;
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

			trace.hitboxes[slot][localtick].Draw();
		}
	}
}
void Trace::DrawPortalsAt(int tick) {
	if (engine->IsSkipping()) return;

	for (const auto &[trace_idx, trace] : traces) {
		if (!trace.draw) continue;
		if (trace.portals.size() == 0) continue;

		unsigned localtick = tickUserToInternal(tick, trace);
		unsigned portals_index = std::min(localtick, trace.portals.size() - 1);

		trace.portals[portals_index].Draw();
	}
}

void Trace::TeleportAt(std::string trace_name, int slot, int tick, bool eye) {
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

ON_EVENT(PROCESS_MOVEMENT) {
	// detect trace switch
	CheckTraceChanged();

	// Record trace
	if (Trace::ShouldRecord()) {
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
			Trace::AddPoint(sar_trace_record.GetString(), player, event.slot, use_client_offset);
		}
	}
}

void CheckTraceChanged() {
	std::string currentTrace = sar_trace_record.GetString();
	if (currentTrace != lastRecordedTrace) {
		lastRecordedTrace = currentTrace;

		if (sar_trace_override.GetBool()) {
			Trace::Clear(currentTrace);
		}
	}
}

void Trace::TweakLatestEyeOffsetForPortalShot(CMoveData *moveData, int slot, bool clientside) {

	if (!sar_trace_use_shot_eyeoffset.GetBool()) return;
	if (!Trace::ShouldRecord()) return;

	TraceData *trace = GetTrace(sar_trace_record.GetString());
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

ON_EVENT(SESSION_START) {
	if (sar_trace_autoclear.GetBool())
		Trace::ClearAll();
}

void Trace::DrawTraceHud(HudContext *ctx) {
	for (auto it = traces.begin(); it != traces.end(); ++it) {
		const char *name = it->first.c_str();
		const TraceData &t = it->second;
		int tick = tickUserToInternal(sar_trace_bbox_at.GetInt(), t);
		for (int slot = 0; slot < 2; slot++) {
			drawTraceInfo(tick, slot, t, [=](const std::string &line) {
				ctx->DrawElement("trace %s %s: %s", name, slot == 1 ? " (orange)" : "", line.c_str());
			});
		}
	}
}

int Trace::GetTasTraceTick() {
	if (sar_trace_draw_time.GetInt() != 3) return -1;

	int max_tas_tick = -1;

	// we want the highest tastick number that some trace is having its bbox drawn
	// at, i.e. the highest tastick <= sar_trace_bbox_at with some trace being at
	// least that long

	for (auto it = traces.begin(); it != traces.end(); ++it) {
		const TraceData &trace = it->second;
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
	Trace::DrawTraceHud(ctx);
}

CON_COMMAND(sar_trace_hide, "sar_trace_hide [trace name] - hide the trace with the given name\n") {
	if (args.ArgC() < 2)
		return console->Print(sar_trace_hide.ThisPtr()->m_pszHelpString);

	std::string trace_name = args[1];
	auto trace = Trace::GetTrace(trace_name);
	if (trace) {
		trace->draw = false;
	}
}

CON_COMMAND(sar_trace_show, "sar_trace_show [trace name] - show the trace with the given name\n") {
	if (args.ArgC() < 2)
		return console->Print(sar_trace_show.ThisPtr()->m_pszHelpString);

	std::string trace_name = args[1];
	auto trace = Trace::GetTrace(trace_name);
	if (trace) {
		trace->draw = true;
	}
}

CON_COMMAND(sar_trace_dump, "sar_trace_dump <tick> [player slot] [trace name] - dump the player state from the given trace tick on the given trace ID (defaults to 1) in the given slot (defaults to 0).\n") {
	if (!sv_cheats.GetBool()) return;

	if (args.ArgC() < 2 || args.ArgC() > 4)
		return console->Print(sar_trace_dump.ThisPtr()->m_pszHelpString);

	std::string trace_name = (args.ArgC() == 4) ? args[3] : Trace::GetDefaultTraceName();
	int slot = (args.ArgC()>=3 && engine->IsCoop()) ? std::atoi(args[2]) : 0;
	int usertick = std::atoi(args[1]);
	if (usertick == -1) usertick = sar_trace_bbox_at.GetInt();

	if (slot > 1) slot = 1;
	if (slot < 0) slot = 0;

	auto trace = Trace::GetTrace(trace_name);
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

		auto trace = Trace::GetTrace(trace_name);
		if (trace) tick = tickInternalToUser(tick, *trace);
		sar_trace_bbox_at.SetValue(tick);
	}

	Trace::DrawInWorld();

	int tick = sar_trace_bbox_at.GetInt();
	if (tick != -1) {
		Trace::DrawBboxAt(tick);
		Trace::DrawPortalsAt(tick);
	}

	const Vector hud_offset = {0.0, 0.0, 2.0};

	for (auto &h : hovers) {
		std::string hover_str;

		int timeType = sar_trace_draw_time.GetInt();
		if (timeType > 0) {
			int tick = h.tick;
			auto trace = Trace::GetTrace(h.trace_name);
			if (trace) {
				tick = tickInternalToUser(tick, *trace);
			}
			hover_str += Utils::ssprintf("tick: %d\n", tick);
		}
		if (Trace::GetTraceCount() > 1) {
			hover_str += Utils::ssprintf("trace: %s\n", h.trace_name.c_str());
		}
		hover_str += Utils::ssprintf("pos: %.1f %.1f %.1f\n", h.position.x, h.position.y, h.position.z);
		hover_str += Utils::ssprintf("horiz. speed: %.2f\n", h.speed);

		OverlayRender::addText(h.position + hud_offset, hover_str, sar_trace_font_size.GetFloat(), true, true);
	}

	if (sar_trace_draw_speed_deltas.GetBool()) {
		Trace::DrawSpeedDeltas();
	}
}

CON_COMMAND(sar_trace_clear, "sar_trace_clear <name> - Clear player trace with a given name\n") {
	if (args.ArgC() != 2)
		return console->Print(sar_trace_clear.ThisPtr()->m_pszHelpString);

	const char *trace_name = args[1];
	Trace::Clear(trace_name);
}

CON_COMMAND(sar_trace_clear_all, "sar_trace_clear_all - Clear all the traces\n") {
	Trace::ClearAll();
}

CON_COMMAND(sar_trace_teleport_at, "sar_trace_teleport_at <tick> [player slot] [trace name] - teleports the player at the given trace tick on the given trace ID (defaults to hovered one or the first one ever made) in the given slot (defaults to 0).\n") {
	if (!sv_cheats.GetBool()) return;

	if (args.ArgC() < 2 || args.ArgC() > 4)
		return console->Print(sar_trace_teleport_at.ThisPtr()->m_pszHelpString);

	std::string trace_name = (args.ArgC() == 4) ? args[3] : Trace::GetDefaultTraceName();
	int slot = (args.ArgC()>=3 && engine->IsCoop()) ? std::atoi(args[2]) : 0;
	int tick = std::atoi(args[1]);

	if (slot > 1) slot = 1;
	if (slot < 0) slot = 0;

	Trace::TeleportAt(trace_name, slot, tick, false);
}

CON_COMMAND(sar_trace_teleport_eye, "sar_trace_teleport_eye <tick> [player slot] [trace name] - teleports the player to the eye position at the given trace tick on the given trace (defaults to hovered one or the first one ever made) in the given slot (defaults to 0).\n") {
	if (!sv_cheats.GetBool()) return;

	if (args.ArgC() < 2 || args.ArgC() > 4)
		return console->Print(sar_trace_teleport_eye.ThisPtr()->m_pszHelpString);

	std::string trace_name = (args.ArgC() == 4) ? args[3] : Trace::GetDefaultTraceName();
	int slot = (args.ArgC()>=3 && engine->IsCoop()) ? std::atoi(args[2]) : 0;
	int tick = std::atoi(args[1]);

	if (slot > 1) slot = 1;
	if (slot < 0) slot = 0;

	Trace::TeleportAt(trace_name, slot, tick, true);
}

CON_COMMAND(sar_trace_export, "sar_trace_export <filename> [trace name] - Export trace data into a csv file.\n") {
	if (args.ArgC() < 2 || args.ArgC() > 3)
		return console->Print(sar_trace_export.ThisPtr()->m_pszHelpString);

	std::string trace_name = (args.ArgC() == 3) ? args[2] : Trace::GetDefaultTraceName();

	auto trace = Trace::GetTrace(trace_name);

	if (trace == nullptr) {
		console->Print("Invalid trace name!\n");
		return;
	}

	bool is_coop_trace = trace->positions[0].size() == trace->positions[1].size();
	size_t size = trace->positions[0].size();

	std::string filename = args[1];
	if (!Utils::EndsWith(filename, ".csv")) filename += ".csv";

	auto filepath = fileSystem->FindFileSomewhere(filename).value_or(filename);
	FILE *f = fopen(filepath.c_str(), "w");
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
