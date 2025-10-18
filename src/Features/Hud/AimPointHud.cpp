#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Features/Session.hpp"
#include "Features/Camera.hpp"
#include "Features/OverlayRender.hpp"

#include <list>
#include <vector>

#define TRACE_LENGTH 2500

Variable sar_aim_point_hud("sar_aim_point_hud", "0", "Overlays a marker with coordinates at the point you're aiming at\n");

static bool g_last_trace_valid;
static CGameTrace g_last_trace;

std::vector<CGameTrace> g_frozen;

CON_COMMAND(sar_aim_point_add, "sar_aim_point_add - add frozen aimpoint at current position\n") {
	if (!g_last_trace_valid) {
		console->Print("Cannot freeze aimpoint; no point\n");
		return;
	}

	g_frozen.push_back(g_last_trace);
}

CON_COMMAND(sar_aim_point_clear, "sar_aim_point_clear - clear all frozen aimpoints\n") {
	console->Print("Unfreezing all aimpoints\n");
	g_frozen.clear();
}

ON_EVENT(SESSION_START) {
	g_last_trace_valid = false;
	g_frozen.clear();
}

static bool shouldDraw() {
	if (!sv_cheats.GetBool()) return false;
	if (!sar_aim_point_hud.GetBool()) return false;
	return true;
}

static bool updateTrace(int slot) {
	if (!session->isRunning) return false;

	CGameTrace tr;
	engine->TraceFromCamera<false>(TRACE_LENGTH, MASK_VISIBLE, tr);

	g_last_trace = tr;
	g_last_trace_valid = tr.plane.normal.Length() > 0.9;

	return true;
}

static void renderTrace(const CGameTrace &tr) {
	if (engine->IsGamePaused()) return;

	auto p = tr.endpos;
	Vector norm = tr.plane.normal;
	
	MeshId cross_mesh = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 255, 255, 255 }));
	MeshId norm_mesh = OverlayRender::createMesh(RenderCallback::none, RenderCallback::constant({ 255, 0, 0 }));

	for (auto dir : std::list<Vector>{
		{0,0,1},
		{0,0,-1},
		{0,1,0},
		{0,-1,0},
		{1,0,0},
		{-1,0,0},
	}) {
		if (dir.Dot(norm) > 0.99) {
			// Basically parallel to the normal vector; skip
			continue;
		}

		OverlayRender::addLine(cross_mesh, p, p + dir * 7);
	}

	OverlayRender::addLine(norm_mesh, p, p + norm * 7);
	OverlayRender::addText(p + Vector{0,0,2}, Utils::ssprintf("%.3f %.3f %.3f", p.x, p.y, p.z), 3.0, true, true, OverlayRender::TextAlign::BOTTOM);
}

ON_EVENT(RENDER) {
	if (!shouldDraw()) return;
	if (!updateTrace(0)) return;
	for (auto &tr : g_frozen) renderTrace(tr);
	if (g_last_trace_valid) renderTrace(g_last_trace);
}
