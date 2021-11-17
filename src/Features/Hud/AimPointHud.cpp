#include "AimPointHud.hpp"

#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Features/Session.hpp"

#include <list>

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

CON_COMMAND(sar_aim_point_clear, "sar_aim_point_cleat - clear all frozen aimpoints\n") {
	console->Print("Unfreezing all aimpoints\n");
	g_frozen.clear();
}

ON_EVENT(SESSION_START) {
	g_last_trace_valid = false;
	g_frozen.clear();
}

bool AimPointHud::ShouldDraw() {
	if (!sv_cheats.GetBool()) return false;
	if (!sar_aim_point_hud.GetBool()) return false;
	return true;
}

bool AimPointHud::GetCurrentSize(int &xSize, int &ySize) {
	return false;
}

static void updateTrace(int slot) {
	g_last_trace_valid = false;

	void *player = server->GetPlayer(slot + 1);
	if (!player) {
		return;
	}

	Vector cam_pos = server->GetAbsOrigin(player) + server->GetViewOffset(player);

	QAngle ang = engine->GetAngles(slot);
	Vector view_vec = Vector{
		cosf(DEG2RAD(ang.y)) * cosf(DEG2RAD(ang.x)),
		sinf(DEG2RAD(ang.y)) * cosf(DEG2RAD(ang.x)),
		-sinf(DEG2RAD(ang.x)),
	}.Normalize();

	Vector dir = view_vec * TRACE_LENGTH;

	CGameTrace tr;

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

	g_last_trace = tr;
	g_last_trace_valid = tr.plane.normal.Length() > 0.9;
}

static void renderTrace(const CGameTrace &tr) {
	auto p = tr.endpos;
	Vector norm = tr.plane.normal;
	
	auto font = scheme->GetDefaultFont() + 1;
	int font_height = surface->GetFontHeight(font);

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

		engine->AddLineOverlay(nullptr, p, p + dir * 7, 255, 255, 255, false, 0.06);
	}

	engine->AddLineOverlay(nullptr, p, p + norm * 7, 255, 0, 0, false, 0.06);

	Vector screen_pos;
	engine->PointToScreen(p + Vector{0,0,7}, screen_pos);

	auto str = Utils::ssprintf("%.3f %.3f %.3f", p.x, p.y, p.z);
	int len = surface->GetFontLength(font, "%s", str.c_str());

	surface->DrawTxt(font, screen_pos.x - len/2, screen_pos.y - font_height - 2, {255, 255, 255, 255}, "%s", str.c_str());
}

void AimPointHud::Paint(int slot) {
	if (slot != 0) return;

	updateTrace(slot);

	for (auto &tr : g_frozen) renderTrace(tr);
	if (g_last_trace_valid) renderTrace(g_last_trace);
}

AimPointHud aimPointHud;
