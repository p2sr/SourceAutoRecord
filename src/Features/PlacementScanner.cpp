// this code sucks but it works lol

#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Features/Camera.hpp"
#include "Features/Session.hpp"
#include "Features/OverlayRender.hpp"
#include "Features/EntityList.hpp"
#include "Features/Hud/Hud.hpp"

#include <cstdio>
#include <optional>

#define TRACE_LENGTH 2500
#define TEST_DIST 10
#define TEST_RESOLUTION 0.1f

static std::optional<CGameTrace> camTrace() {
	if (!session->isRunning) return {};

	void *player = server->GetPlayer(1);
	if (!player) return {};

	CTraceFilterSimple filter;
	filter.SetPassEntity(player);

	Vector cam = camera->GetPosition(GET_SLOT());
	Vector delta = camera->GetForwardVector(GET_SLOT()) * TRACE_LENGTH;

	Ray_t ray;
	ray.m_IsRay = true;
	ray.m_IsSwept = true;
	ray.m_Start = VectorAligned(cam.x, cam.y, cam.z);
	ray.m_Delta = VectorAligned(delta.x, delta.y, delta.z);
	ray.m_StartOffset = VectorAligned();

	CGameTrace tr;

	engine->TraceRay(engine->engineTrace->ThisPtr(), ray, MASK_SHOT_PORTAL, &filter, &tr);

	return tr.plane.normal.Length() > 0.9 ? tr : std::optional<CGameTrace>{};
}

// Gets a pair of perpendicular axes on a plane
static inline void getAxesForPlane(cplane_t plane, Vector *ax1, Vector *ax2) {
	Vector up = fabsf(plane.normal.z) < 0.9 ? Vector{0,0,1} : Vector{1,0,0};
	*ax1 = plane.normal.Cross(up);
	*ax2 = plane.normal.Cross(*ax1);

	*ax1 = ax1->Normalize();
	*ax2 = ax2->Normalize();
}

static Vector intersectPlaneView(cplane_t plane) {
	Vector start = camera->GetPosition(GET_SLOT());
	Vector dir = camera->GetForwardVector(GET_SLOT());

	float w = dir.Dot(plane.normal);
	if (w == 0) {
		// Line parallel to plane - just return some point on the plane
		return plane.normal * plane.dist;
	}
	float p = (plane.dist - start.Dot(plane.normal)) / w;
	if (p < 0) {
		// Camera looking away from plane - return a random point again
		return plane.normal * plane.dist;
	}
	return start + dir*p;
}

static void drawRectOnPlane(cplane_t plane, Vector a, Vector c, Color col, float draw_dist) {
	Vector ax1, ax2;
	getAxesForPlane(plane, &ax1, &ax2);

	Vector v = a - c;
	Vector b = c + ax1*v.Dot(ax1);
	Vector d = c + ax2*v.Dot(ax2);

	Vector zf = plane.normal*draw_dist; // z-fighting

	MeshId m = OverlayRender::createMesh(RenderCallback::constant(col), RenderCallback::none);
	OverlayRender::addQuad(m, a+zf, b+zf, c+zf, d+zf);
}

static void drawPointOnPlane(cplane_t plane, Vector p, Color col, float draw_dist) {
	Vector ax1, ax2;
	getAxesForPlane(plane, &ax1, &ax2);

	Vector a = p - ax1 - ax2;
	Vector b = p + ax1 - ax2;
	Vector c = p + ax1 + ax2;
	Vector d = p - ax1 + ax2;

	Vector zf = plane.normal*draw_dist; // z-fighting

	MeshId m = OverlayRender::createMesh(RenderCallback::constant(col), RenderCallback::none);
	OverlayRender::addQuad(m, a+zf, b+zf, c+zf, d+zf);
}

enum class SetupState {
	NONE,
	SET_WALL,
	SET_SCAN_POINT_A,
	SET_SCAN_POINT_B,
	SET_MATCH_POINT_A,
	SET_MATCH_POINT_B,
	READY,
	RUNNING,
};

static struct {
	SetupState state;
	cplane_t wall_plane;
	Vector scan_a;
	Vector scan_b;
	Vector match_a;
	Vector match_b;
} g_setup;

#define NTHREADS 4

static struct {
	Vector start;
	int maxd1i;
	int maxd2i;
	int d1i;
	Vector ax1;
	Vector ax2;
	uint8_t *image;
} g_scan;

static bool liesInMatchArea(Vector p) {
	Vector ax1 = g_scan.ax1;
	Vector ax2 = g_scan.ax2;

	float m1a = g_setup.match_a.Dot(ax1);
	float m2a = g_setup.match_a.Dot(ax2);

	float m1b = g_setup.match_b.Dot(ax1);
	float m2b = g_setup.match_b.Dot(ax2);

	float min1 = m1a < m1b ? m1a : m1b;
	float min2 = m2a < m2b ? m2a : m2b;

	float max1 = m1a > m1b ? m1a : m1b;
	float max2 = m2a > m2b ? m2a : m2b;

	float d1 = p.Dot(ax1);
	float d2 = p.Dot(ax2);

	return min1 <= d1 && d1 <= max1 && min2 <= d2 && d2 <= max2;
}

static bool testPoint(uintptr_t portalgun, Vector point) {
	Vector origin = point + g_setup.wall_plane.normal * TEST_DIST;
	Vector dir = -g_setup.wall_plane.normal;

	TracePortalPlacementInfo_t info;
	server->TraceFirePortal(portalgun, origin, dir, false, 2, info);

	bool success;
	switch (info.ePlacementResult) {
	case PORTAL_PLACEMENT_SUCCESS:
	case PORTAL_PLACEMENT_USED_HELPER:
	case PORTAL_PLACEMENT_BUMPED:
		success = true;
		break;
	default:
		success = false;
		break;
	}

	return success && liesInMatchArea(info.finalPos);
}

static uintptr_t initScan() {
	void *player = server->GetPlayer(1);
	if (!player) return 0;

	uintptr_t portalgun = (uintptr_t)entityList->LookupEntity(SE(player)->active_weapon());
	if (!portalgun || !entityList->IsPortalGun(SE(player)->active_weapon())) return 0;

	uint8_t linkage = SE(portalgun)->field<unsigned char>("m_iPortalLinkageGroupID");

	auto m_hPrimaryPortal = SE(portalgun)->field<CBaseHandle>("m_hPrimaryPortal");
	auto m_hSecondaryPortal = SE(portalgun)->field<CBaseHandle>("m_hSecondaryPortal");
	auto bluePortal = (uintptr_t)entityList->LookupEntity(m_hPrimaryPortal);
	auto orangePortal = (uintptr_t)entityList->LookupEntity(m_hSecondaryPortal);

	if (!bluePortal) {
		bluePortal = server->FindPortal(linkage, false, true);
		SE(portalgun)->field<CBaseHandle>("m_hPrimaryPortal") = ((IHandleEntity *)bluePortal)->GetRefEHandle();
	}

	if (!orangePortal) {
		orangePortal = server->FindPortal(linkage, true, true);
		SE(portalgun)->field<CBaseHandle>("m_hSecondaryPortal") = ((IHandleEntity *)orangePortal)->GetRefEHandle();
	}

	return portalgun;
}

static void writeTga(const char *path, const uint8_t *data, uint16_t w, uint16_t h) {
	FILE *f = fopen(path, "wb");
	if (!f) return;
	uint8_t header[] = {
		0, // ID length
		0, // Color map type
		2, // Image type (uncompressed true color)
		0, 0, 0, 0, 0, // Color map info (n/a)
		0, 0, // X origin
		0, 0, // Y origin
		uint8_t(w & 0xFF), uint8_t(w >> 8), // Width
		uint8_t(h & 0xFF), uint8_t(h >> 8), // Height
		32, // Bits per pixel
		0, // Image descriptor
	};
	fwrite(header, 1, sizeof header, f);
	fwrite(data, 1, w * h * 4, f);
	fclose(f);
}

static void startScan() {
	Vector ax1, ax2;
	getAxesForPlane(g_setup.wall_plane, &ax1, &ax2);

	Vector start = g_setup.scan_a;
	Vector delta = g_setup.scan_b - start;
	float maxd1 = delta.Dot(ax1);
	float maxd2 = delta.Dot(ax2);

	if (maxd1 < 0) {
		start = start + ax1*maxd1;
		maxd1 = -maxd1;
	}

	if (maxd2 < 0) {
		start = start + ax2*maxd2;
		maxd2 = -maxd2;
	}

	g_scan.start = start;
	g_scan.maxd1i = maxd1 / TEST_RESOLUTION;
	g_scan.maxd2i = maxd2 / TEST_RESOLUTION;
	g_scan.d1i = 0;
	g_scan.ax1 = ax1;
	g_scan.ax2 = ax2;
	g_scan.image = new uint8_t[g_scan.maxd1i * g_scan.maxd2i * 4];

	g_setup.state = SetupState::RUNNING;
}

static void endScan(bool success) {
	if (success) {
		console->Print("Success! Wrote %d points to pp_scan.tga\n", g_scan.maxd1i * g_scan.maxd2i);
		writeTga("pp_scan.tga", g_scan.image, g_scan.maxd1i, g_scan.maxd2i);
	} else {
		console->Print("Scanning failed\n");
	}
	delete[] g_scan.image;
	g_setup.state = SetupState::NONE;
}

static void runScan() {
	uintptr_t portalgun = initScan();
	if (!portalgun) {
		endScan(false);
		return;
	}

	long initial_percentage = 1000L * (long)g_scan.d1i / (long)g_scan.maxd1i;

	Vector ax1 = g_scan.ax1;
	Vector ax2 = g_scan.ax2;
	Vector start = g_scan.start;
	uint8_t *image = g_scan.image;
	int maxd1i = g_scan.maxd1i;
	int maxd2i = g_scan.maxd2i;

	for (int d1i = g_scan.d1i; d1i < maxd1i; ++d1i) {
		float d1 = (maxd1i - 1 - d1i) * TEST_RESOLUTION;
		for (int d2i = 0; d2i < maxd2i; ++d2i) {
			float d2 = (maxd2i - 1 - d2i) * TEST_RESOLUTION;
			int i = d2i*maxd1i + d1i;
			bool success = testPoint(portalgun, start + ax1*d1 + ax2*d2);
			if (success) {
				image[i*4 + 0] = 0;
				image[i*4 + 1] = 255;
				image[i*4 + 2] = 0;
				image[i*4 + 3] = 255;
			} else {
				image[i*4 + 0] = 0;
				image[i*4 + 1] = 0;
				image[i*4 + 2] = 255;
				image[i*4 + 3] = 255;
			}
		}

		long percentage = 1000L * (long)d1i / (long)maxd1i;
		if (percentage > initial_percentage && percentage < 1000) {
			g_scan.d1i = d1i;
			return;
		}
	}

	endScan(true);
}

ON_EVENT(RENDER) {
	switch (g_setup.state) {
	case SetupState::NONE:
		return;
	case SetupState::SET_WALL:
		{
			auto tr = camTrace();
			if (!tr) return;

			Vector ax1, ax2;
			getAxesForPlane(tr->plane, &ax1, &ax2);

			Vector center = tr->endpos + tr->plane.normal*0.04; // Bump away to prevent z-fighting

			Vector a = center + ax1*100 + ax2*100;
			Vector b = center + ax1*100 - ax2*100;
			Vector c = center - ax1*100 - ax2*100;
			Vector d = center - ax1*100 + ax2*100;

			MeshId m = OverlayRender::createMesh(RenderCallback::constant({255,0,0,100}), RenderCallback::none);
			OverlayRender::addQuad(m, a, b, c, d);
		}
		return;
	case SetupState::SET_SCAN_POINT_A:
		drawPointOnPlane(g_setup.wall_plane, intersectPlaneView(g_setup.wall_plane), {255, 0, 0, 100}, 0.04);
		return;
	case SetupState::SET_SCAN_POINT_B:
		drawRectOnPlane(g_setup.wall_plane, g_setup.scan_a, intersectPlaneView(g_setup.wall_plane), {255, 0, 0, 100}, 0.04);
		return;
	case SetupState::SET_MATCH_POINT_A:
		drawRectOnPlane(g_setup.wall_plane, g_setup.scan_a, g_setup.scan_b, {255, 0, 0, 100}, 0.04);
		drawPointOnPlane(g_setup.wall_plane, intersectPlaneView(g_setup.wall_plane), {0, 0, 255, 100}, 0.12);
		return;
	case SetupState::SET_MATCH_POINT_B:
		drawRectOnPlane(g_setup.wall_plane, g_setup.scan_a, g_setup.scan_b, {255, 0, 0, 100}, 0.04);
		drawRectOnPlane(g_setup.wall_plane, g_setup.match_a, intersectPlaneView(g_setup.wall_plane), {0, 0, 255, 100}, 0.12);
		return;
	case SetupState::READY:
		drawRectOnPlane(g_setup.wall_plane, g_setup.scan_a, g_setup.scan_b, {255, 0, 0, 100}, 0.04);
		drawRectOnPlane(g_setup.wall_plane, g_setup.match_a, g_setup.match_b, {0, 0, 255, 100}, 0.12);
		return;
	case SetupState::RUNNING:
		return;
	}
}

ON_EVENT(FRAME) {
	if (!sv_cheats.GetBool()) {
		g_setup.state = SetupState::NONE;
	}

	if (g_setup.state == SetupState::RUNNING) {
		runScan();
	}
}

ON_EVENT(SESSION_START) {
	g_setup.state = SetupState::NONE;
}

HUD_ELEMENT2_NO_DISABLE(pp_scan, HudType_InGame) {
	if (g_setup.state == SetupState::NONE) return;

	std::string status_text = "";
	int sw, sh;
	engine->GetScreenSize(nullptr, sw, sh);

	switch (g_setup.state) {
	case SetupState::SET_WALL:
		status_text = "Select the wall plane to scan on.";
		break;
	case SetupState::SET_SCAN_POINT_A:
		status_text = "Select first corner of the scanning point.";
		break;
	case SetupState::SET_SCAN_POINT_B:
		status_text = "Select second corner of the scanning point.";
		break;
	case SetupState::SET_MATCH_POINT_A:
		status_text = "Select first corner of portal success detection area.";
		break;
	case SetupState::SET_MATCH_POINT_B:
		status_text = "Select second corner of portal success detection area.";
		break;
	case SetupState::READY:
		status_text = "Placement scan is ready. Use sar_pp_scan_set to begin.";
		break;
	case SetupState::RUNNING:
		status_text = "Scanning to pp_scan.tga... use sar_pp_scan_set to cancel.";
		break;
	default:
		break;
	}

	surface->DrawRectAndCenterTxt(Color{0, 0, 0, 0}, 0, 0, sw, sh - 50, 6, Color{255, 255, 255}, status_text.c_str());

	if (g_setup.state == SetupState::RUNNING) {
		float percentage = 100.0f * (float)g_scan.d1i / (float)g_scan.maxd1i;
		surface->DrawRectAndCenterTxt(Color{0, 0, 0, 200}, 0, 0, sw, sh + 50, 6, Color{255, 255, 255}, "%.1f%%", percentage);
	}
}

CON_COMMAND(sar_pp_scan_set, "sar_pp_scan_set - set the ppscan point where you're aiming.\n") {
	switch (g_setup.state) {
	case SetupState::NONE:
		g_setup.state = SetupState::SET_WALL;
		return;
	case SetupState::SET_WALL:
		{
			auto tr = camTrace();
			if (!tr) return;
			g_setup.wall_plane = tr->plane;
			g_setup.state = SetupState::SET_SCAN_POINT_A;
		}
		return;
	case SetupState::SET_SCAN_POINT_A:
		g_setup.scan_a = intersectPlaneView(g_setup.wall_plane);
		g_setup.state = SetupState::SET_SCAN_POINT_B;
		return;
	case SetupState::SET_SCAN_POINT_B:
		g_setup.scan_b = intersectPlaneView(g_setup.wall_plane);
		g_setup.state = SetupState::SET_MATCH_POINT_A;
		return;
	case SetupState::SET_MATCH_POINT_A:
		g_setup.match_a = intersectPlaneView(g_setup.wall_plane);
		g_setup.state = SetupState::SET_MATCH_POINT_B;
		return;
	case SetupState::SET_MATCH_POINT_B:
		g_setup.match_b = intersectPlaneView(g_setup.wall_plane);
		g_setup.state = SetupState::READY;
		return;
	case SetupState::READY:
		startScan();
		return;
	case SetupState::RUNNING:
		endScan(false);
		return;
	}
}

CON_COMMAND(sar_pp_scan_reset, "sar_pp_scan_reset - reset ppscan.\n") {
	g_setup.state = SetupState::NONE;
}
