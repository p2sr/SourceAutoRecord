// this code sucks but it works lol

#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Modules/FileSystem.hpp"
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
#define TEST_SCAN_PERCENTAGE_PER_FRAME 0.1f

static std::optional<CGameTrace> camTrace() {
	if (!session->isRunning) return {};

	CGameTrace tr;

	engine->TraceFromCamera<false>(TRACE_LENGTH, MASK_SHOT_PORTAL, tr);

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
	cplane_t wallPlane;
	Vector scanAreaCorner1;
	Vector scanAreaCorner2;
	Vector matchAreaCorner1;
	Vector matchAreaCorner2;
} g_setup;

static struct {
	Vector startPoint;
	int maxHeight;
	int maxWidth;
	int heightProgress;
	Vector wallWidthAxis;
	Vector wallHeightAxis;
	uint8_t *image;
} g_scan;

static bool liesInMatchArea(Vector p) {
	Vector ax1 = g_scan.wallWidthAxis;
	Vector ax2 = g_scan.wallHeightAxis;

	float m1a = g_setup.matchAreaCorner1.Dot(ax1);
	float m2a = g_setup.matchAreaCorner1.Dot(ax2);

	float m1b = g_setup.matchAreaCorner2.Dot(ax1);
	float m2b = g_setup.matchAreaCorner2.Dot(ax2);

	float min1 = m1a < m1b ? m1a : m1b;
	float min2 = m2a < m2b ? m2a : m2b;

	float max1 = m1a > m1b ? m1a : m1b;
	float max2 = m2a > m2b ? m2a : m2b;

	float d1 = p.Dot(ax1);
	float d2 = p.Dot(ax2);

	return min1 <= d1 && d1 <= max1 && min2 <= d2 && d2 <= max2;
}

static bool testPoint(uintptr_t portalgun, Vector point) {
	Vector origin = point + g_setup.wallPlane.normal * TEST_DIST;
	Vector dir = -g_setup.wallPlane.normal;

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
	auto filepath = fileSystem->FindFileSomewhere(path).value_or(path);
	FILE *f = fopen(filepath.c_str(), "wb");
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
	Vector wallWidthAxis, wallHeightAxis;
	getAxesForPlane(g_setup.wallPlane, &wallWidthAxis, &wallHeightAxis);

	Vector startPoint = g_setup.scanAreaCorner1;
	Vector delta = g_setup.scanAreaCorner2 - startPoint;
	float maxWidth = delta.Dot(wallWidthAxis);
	float maxHeight = delta.Dot(wallHeightAxis);

	if (maxWidth < 0) {
		startPoint = startPoint + wallWidthAxis*maxWidth;
		maxWidth = -maxWidth;
	}

	if (maxHeight < 0) {
		startPoint = startPoint + wallHeightAxis*maxHeight;
		maxHeight = -maxHeight;
	}

	g_scan.startPoint = startPoint;
	g_scan.maxHeight = maxWidth / TEST_RESOLUTION;
	g_scan.maxWidth = maxHeight / TEST_RESOLUTION;
	g_scan.heightProgress = 0;
	g_scan.wallWidthAxis = wallWidthAxis;
	g_scan.wallHeightAxis = wallHeightAxis;
	g_scan.image = new uint8_t[g_scan.maxHeight * g_scan.maxWidth * 4];

	g_setup.state = SetupState::RUNNING;
}

static void endScan(bool success) {
	if (success) {
		console->Print("Success! Wrote %d points to pp_scan.tga\n", g_scan.maxHeight * g_scan.maxWidth);
		writeTga("pp_scan.tga", g_scan.image, g_scan.maxHeight, g_scan.maxWidth);
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

	float initialProgress = (float)g_scan.heightProgress / (float)g_scan.maxHeight;

	Vector wallAxisWidth = g_scan.wallWidthAxis;
	Vector wallAxisHeight = g_scan.wallHeightAxis;
	Vector startPoint = g_scan.startPoint;
	uint8_t *image = g_scan.image;
	int maxHeight = g_scan.maxHeight;
	int maxWidth = g_scan.maxWidth;

	for (int height = g_scan.heightProgress; height < maxHeight; ++height) {
		float heightUnits = (maxHeight - 1 - height) * TEST_RESOLUTION;
		for (int width = 0; width < maxWidth; ++width) {
			float widthUnits = (maxWidth - 1 - width) * TEST_RESOLUTION;
			int pixelIndex = width*maxHeight + height;
			bool success = testPoint(portalgun, startPoint + wallAxisWidth*heightUnits + wallAxisHeight*widthUnits);
			if (success) {
				image[pixelIndex*4 + 0] = 0;
				image[pixelIndex*4 + 1] = 255;
				image[pixelIndex*4 + 2] = 0;
				image[pixelIndex*4 + 3] = 255;
			} else {
				image[pixelIndex*4 + 0] = 0;
				image[pixelIndex*4 + 1] = 0;
				image[pixelIndex*4 + 2] = 255;
				image[pixelIndex*4 + 3] = 255;
			}
		}

		float currentProgress = (float)height / (float)maxHeight;
		if (currentProgress < 1.0f && currentProgress > initialProgress + TEST_SCAN_PERCENTAGE_PER_FRAME) {
			g_scan.heightProgress = height;
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
		drawPointOnPlane(g_setup.wallPlane, intersectPlaneView(g_setup.wallPlane), {255, 0, 0, 100}, 0.04);
		return;
	case SetupState::SET_SCAN_POINT_B:
		drawRectOnPlane(g_setup.wallPlane, g_setup.scanAreaCorner1, intersectPlaneView(g_setup.wallPlane), {255, 0, 0, 100}, 0.04);
		return;
	case SetupState::SET_MATCH_POINT_A:
		drawRectOnPlane(g_setup.wallPlane, g_setup.scanAreaCorner1, g_setup.scanAreaCorner2, {255, 0, 0, 100}, 0.04);
		drawPointOnPlane(g_setup.wallPlane, intersectPlaneView(g_setup.wallPlane), {0, 0, 255, 100}, 0.12);
		return;
	case SetupState::SET_MATCH_POINT_B:
		drawRectOnPlane(g_setup.wallPlane, g_setup.scanAreaCorner1, g_setup.scanAreaCorner2, {255, 0, 0, 100}, 0.04);
		drawRectOnPlane(g_setup.wallPlane, g_setup.matchAreaCorner1, intersectPlaneView(g_setup.wallPlane), {0, 0, 255, 100}, 0.12);
		return;
	case SetupState::READY:
		drawRectOnPlane(g_setup.wallPlane, g_setup.scanAreaCorner1, g_setup.scanAreaCorner2, {255, 0, 0, 100}, 0.04);
		drawRectOnPlane(g_setup.wallPlane, g_setup.matchAreaCorner1, g_setup.matchAreaCorner2, {0, 0, 255, 100}, 0.12);
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
		float percentage = 100.0f * (float)g_scan.heightProgress / (float)g_scan.maxHeight;
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
			g_setup.wallPlane = tr->plane;
			g_setup.state = SetupState::SET_SCAN_POINT_A;
		}
		return;
	case SetupState::SET_SCAN_POINT_A:
		g_setup.scanAreaCorner1 = intersectPlaneView(g_setup.wallPlane);
		g_setup.state = SetupState::SET_SCAN_POINT_B;
		return;
	case SetupState::SET_SCAN_POINT_B:
		g_setup.scanAreaCorner2 = intersectPlaneView(g_setup.wallPlane);
		g_setup.state = SetupState::SET_MATCH_POINT_A;
		return;
	case SetupState::SET_MATCH_POINT_A:
		g_setup.matchAreaCorner1 = intersectPlaneView(g_setup.wallPlane);
		g_setup.state = SetupState::SET_MATCH_POINT_B;
		return;
	case SetupState::SET_MATCH_POINT_B:
		g_setup.matchAreaCorner2 = intersectPlaneView(g_setup.wallPlane);
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
