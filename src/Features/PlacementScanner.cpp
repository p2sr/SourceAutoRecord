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
#include "Features/Renderer.hpp"

#include <cstdio>
#include <optional>

#define TRACE_LENGTH 2500
#define TEST_DIST 2
#define TEST_RESOLUTION 0.1f
#define TEST_SCAN_PROGRESS_PER_FRAME 0.001f
#define TEST_FLOOR_UP_CONTRIBUTION 0.1f;

Variable sar_pp_scan_color_success("sar_pp_scan_color_success", "131 182 146", "The color rendered by pp scan when portal placement is successful.\n");
Variable sar_pp_scan_color_fail("sar_pp_scan_color_fail", "194 69 45", "The color rendered by pp scan where portal placement fails.\n");
Variable sar_pp_scan_savepath("sar_pp_scan_filename", "pp_scan", "The name of the TGA image to save after pp scan, without extension.\n"); 
Variable sar_pp_scan_screenshot_overlay("sar_pp_scan_screenshot_overlay", "0", "Whether to overlay pp scan screenshot on top of gameplay screenshot.\n");

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
	bool isPerspective;
	cplane_t wallPlane;
	Vector scanUpDirection;
	Vector scanAreaCorner1;
	Vector scanAreaCorner2;
	Vector matchAreaCorner1;
	Vector matchAreaCorner2;
} g_setup;

static struct {
	Vector startPoint;
	int maxWidth;
	int maxHeight;
	int widthProgress;
	Vector wallWidthAxis;
	Vector wallHeightAxis;
	float fovOffset;
	Color successColor;
	Color failColor;
	uint8_t *image;
} g_scan;

static std::optional<CGameTrace> camTrace() {
	if (!session->isRunning) return {};

	CGameTrace tr;

	engine->TraceFromCamera<false>(TRACE_LENGTH, MASK_SHOT_PORTAL, tr);

	return tr.plane.normal.Length() > 0.9 ? tr : std::optional<CGameTrace>{};
}

// Gets a pair of perpendicular axes on a plane
static inline void getAxesForScanAreaPlane(cplane_t plane, Vector *ax1, Vector *ax2) {
	*ax1 = plane.normal.Cross(g_setup.scanUpDirection);
	*ax2 = plane.normal.Cross(*ax1);

	*ax1 = ax1->Normalize();
	*ax2 = ax2->Normalize();
}

static Vector intersectPlaneView(cplane_t plane) {
	Vector start = camera->GetPosition(GET_SLOT());
	QAngle angles = camera->GetAngles(GET_SLOT());

	Vector dir;
	Math::AngleVectors(angles, &dir);

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
	getAxesForScanAreaPlane(plane, &ax1, &ax2);

	Vector v = a - c;
	Vector b = c + ax1*v.Dot(ax1);
	Vector d = c + ax2*v.Dot(ax2);

	Vector zf = plane.normal*draw_dist; // z-fighting

	MeshId m = OverlayRender::createMesh(RenderCallback::constant(col), RenderCallback::none);
	OverlayRender::addQuad(m, a+zf, b+zf, c+zf, d+zf);
}

static void drawPointOnPlane(cplane_t plane, Vector p, Color col, float draw_dist) {
	Vector ax1, ax2;
	getAxesForScanAreaPlane(plane, &ax1, &ax2);

	Vector a = p - ax1 - ax2;
	Vector b = p + ax1 - ax2;
	Vector c = p + ax1 + ax2;
	Vector d = p - ax1 + ax2;

	Vector zf = plane.normal*draw_dist; // z-fighting

	MeshId m = OverlayRender::createMesh(RenderCallback::constant(col), RenderCallback::none);
	OverlayRender::addQuad(m, a+zf, b+zf, c+zf, d+zf);
}

static void drawTracePlane() {
	auto tr = camTrace();
	if (!tr) return;

	Vector ax1, ax2;
	getAxesForScanAreaPlane(tr->plane, &ax1, &ax2);

	Vector center = tr->endpos + tr->plane.normal * 0.04;  // Bump away to prevent z-fighting

	Vector a = center + ax1 * 100 + ax2 * 100;
	Vector b = center + ax1 * 100 - ax2 * 100;
	Vector c = center - ax1 * 100 - ax2 * 100;
	Vector d = center - ax1 * 100 + ax2 * 100;

	MeshId m = OverlayRender::createMesh(RenderCallback::constant({255, 0, 0, 100}), RenderCallback::none);
	OverlayRender::addQuad(m, a, b, c, d);
}

static void updateScanAreaUpDirection() {
	auto tr = camTrace();
	if (!tr) return;

	if (fabsf(tr->plane.normal.z) > 0.999f) {
		QAngle angles = camera->GetAngles(GET_SLOT());
		Math::AngleVectors(angles, &g_setup.scanUpDirection);
	} else {
		g_setup.scanUpDirection = Vector(0, 0, 1);
	}
}

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

static void loadScreenPixelsIntoImage() {
	std::vector<uint8_t> buf(g_scan.maxWidth * g_scan.maxHeight * 4);
	Renderer::ReadScreenPixels(0, 0, g_scan.maxWidth, g_scan.maxHeight, buf.data(), IMAGE_FORMAT_RGBA8888);

	// full alpha, inverted y axis, RGBA to BGRA
	for (int x = 0; x < g_scan.maxWidth; ++x) {
		for (int y = 0; y < g_scan.maxHeight; ++y) {
			size_t i = (y * g_scan.maxWidth + x) * 4;
			size_t j = ((g_scan.maxHeight - 1 - y) * g_scan.maxWidth + x) * 4;
			g_scan.image[i + 0] = buf[j + 2];
			g_scan.image[i + 1] = buf[j + 1];
			g_scan.image[i + 2] = buf[j + 1];
			g_scan.image[i + 3] = 255;
		}
	}
}

static void prepareForScan() {
	g_scan.image = new uint8_t[g_scan.maxWidth * g_scan.maxHeight * 4] {};

	if (sar_pp_scan_screenshot_overlay.GetBool()) {
		loadScreenPixelsIntoImage();
	}

	g_scan.successColor = Utils::GetColor(sar_pp_scan_color_success.GetString(), false).value_or(Color(0, 0, 255, 255));
	g_scan.failColor = Utils::GetColor(sar_pp_scan_color_fail.GetString(), false).value_or(Color(255, 0, 0, 255));

	g_scan.widthProgress = 0;
	g_setup.state = SetupState::RUNNING;
}

static void startAreaScan() {
	g_setup.isPerspective = false;

	Vector wallWidthAxis, wallHeightAxis;
	getAxesForScanAreaPlane(g_setup.wallPlane, &wallWidthAxis, &wallHeightAxis);

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
	g_scan.maxWidth = maxWidth / TEST_RESOLUTION;
	g_scan.maxHeight = maxHeight / TEST_RESOLUTION;
	g_scan.wallWidthAxis = wallWidthAxis;
	g_scan.wallHeightAxis = wallHeightAxis;

	prepareForScan();
}

static void startPerspectiveScan() {
	g_setup.isPerspective = true;

	engine->GetScreenSize(nullptr, g_scan.maxWidth, g_scan.maxHeight);

	Vector cameraPos = camera->GetPosition(GET_SLOT(), false);
	g_scan.startPoint = cameraPos;

	QAngle cameraAngles = camera->GetAngles(GET_SLOT(), false);
	Vector cameraForward, cameraRight, cameraUp;
	Math::AngleVectors(cameraAngles, &cameraForward, &cameraRight, &cameraUp);
	g_setup.wallPlane = {cameraForward};
	g_scan.wallWidthAxis = cameraRight;
	g_scan.wallHeightAxis = cameraUp;

	// scaled fov calculations (https://developer.valvesoftware.com/wiki/Field_of_View)
	float fov = DEG2RAD(camera->GetFieldOfView(GET_SLOT(), false));
	float aspectRatio = (float)g_scan.maxWidth / (float)g_scan.maxHeight;
	float standardRatio = 4.0f / 3.0f;
	float scaledFov = atanf(tanf(fov * 0.5f) * aspectRatio / standardRatio) * 2.0f;
	g_scan.fovOffset = 1.0f / tanf(scaledFov * 0.5f);
	
	prepareForScan();
}

static void endScan(bool success) {
	if (success) {
		console->Print("Success! Wrote %d points to %s.tga\n", g_scan.maxWidth * g_scan.maxHeight, sar_pp_scan_savepath.GetString());
		std::string path = Utils::ssprintf("%s.tga", sar_pp_scan_savepath.GetString());
		writeTga(path.c_str(), g_scan.image, g_scan.maxWidth, g_scan.maxHeight);
	} else {
		console->Print("Scanning failed\n");
	}
	delete[] g_scan.image;
	g_setup.state = SetupState::NONE;
	Event::Trigger<Event::PP_SCAN_FINISH>({});
}

static bool testRay(uintptr_t portalgun, Vector origin, Vector dir, bool checkMatchArea) {
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

	return success && (!checkMatchArea || liesInMatchArea(info.finalPos));
}

static bool testPerspectivePixel(uintptr_t portalgun, int x, int y) {
	Vector2 screenPointNormalized(
		((float)(x) + 0.5f) / (float)(g_scan.maxWidth) * 2.0f - 1.0f,
		((float)(y) + 0.5f) / (float)(g_scan.maxHeight) * 2.0f - 1.0f);

	screenPointNormalized.y *= ((float)g_scan.maxHeight / (float)g_scan.maxWidth);  // aspect ratio

	Vector dir = g_setup.wallPlane.normal * g_scan.fovOffset 
		+ g_scan.wallWidthAxis * screenPointNormalized.x 
		+ g_scan.wallHeightAxis * screenPointNormalized.y;

	return testRay(portalgun, g_scan.startPoint, dir.Normalize(), false);
}

static bool testAreaPixel(uintptr_t portalgun, int x, int y) {
	float widthUnits = (g_scan.maxWidth - 1 - x) * TEST_RESOLUTION;
	float heightUnits = (g_scan.maxHeight - 1 - y) * TEST_RESOLUTION;

	Vector dir = -g_setup.wallPlane.normal;
	
	// fix for floor/ceiling portals, so that the scan can actually check with correct angles
	if (fabsf(g_setup.wallPlane.normal.z) > 0.999) {
		dir += g_setup.scanUpDirection * TEST_FLOOR_UP_CONTRIBUTION;
		dir = dir.Normalize();
	}

	Vector point = g_scan.startPoint 
		+ g_scan.wallWidthAxis * widthUnits 
		+ g_scan.wallHeightAxis * heightUnits 
		- dir * TEST_DIST;

	return testRay(portalgun, point, dir, true);
}

static bool testPixel(uintptr_t portalgun, int x, int y) {
	if (g_setup.isPerspective) {
		return testPerspectivePixel(portalgun, x, y);
	} else {
		return testAreaPixel(portalgun, x, y);
	}
}

static void runScan() {
	uintptr_t portalgun = initScan();
	if (!portalgun) {
		endScan(false);
		return;
	}
	
	uint8_t *image = g_scan.image;
	int maxWidth = g_scan.maxWidth;
	int maxHeight = g_scan.maxHeight;

	float initialProgress = (float)g_scan.widthProgress / (float)g_scan.maxWidth;

	for (int width = g_scan.widthProgress; width < maxWidth; ++width) {
		for (int height = 0; height < maxHeight; ++height) {
			int pixelIndex = height * maxWidth + width;
			bool success = testPixel(portalgun, width, height);

			Color color = success ? g_scan.successColor : g_scan.failColor;

			uint8_t baseB = image[pixelIndex * 4 + 0];
			uint8_t baseG = image[pixelIndex * 4 + 1];
			uint8_t baseR = image[pixelIndex * 4 + 2];
			uint8_t baseA = image[pixelIndex * 4 + 3];

			image[pixelIndex * 4 + 0] = 255 - (255 - baseB * baseA / 255) * (255 - color.b * color.a / 255) / 255;
			image[pixelIndex * 4 + 1] = 255 - (255 - baseG * baseA / 255) * (255 - color.g * color.a / 255) / 255;
			image[pixelIndex * 4 + 2] = 255 - (255 - baseR * baseA / 255) * (255 - color.r * color.a / 255) / 255;
			image[pixelIndex * 4 + 3] = 255 - (255 - baseA) * (255 - color.a) / 255;
		}

		float currentProgress = (float)width / (float)maxWidth;
		if (currentProgress < 1.0f && currentProgress > initialProgress + TEST_SCAN_PROGRESS_PER_FRAME) {
			g_scan.widthProgress = width + 1;
			return;
		}
	}

	endScan(true);
}

static bool checkScanningAvailable() {
	if (!session->isRunning) {
		console->Print("Cannot start placement scan while not in a session.\n");
		return false;
	}

	if (!sv_cheats.GetBool()) {
		console->Print("Cannot start placement scan while sv_cheats is disabled.\n");
		return false;
	}

	return true;
}

ON_EVENT(RENDER) {
	switch (g_setup.state) {
	case SetupState::NONE:
		return;
	case SetupState::SET_WALL:
		drawTracePlane();
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
	if (g_setup.state == SetupState::NONE) {
		return;
	}

	if (!checkScanningAvailable()) {
		g_setup.state = SetupState::NONE;
		return;
	}

	if (g_setup.state == SetupState::SET_WALL) {
		updateScanAreaUpDirection();
	}

	if (g_setup.state == SetupState::RUNNING) {
		runScan();
	}
}

ON_EVENT(SESSION_START) {
	g_setup.state = SetupState::NONE;
}

HUD_ELEMENT2_NO_DISABLE(pp_scan, HudType_InGame | HudType_Paused) {
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
		status_text = Utils::ssprintf("Scanning to %s.tga... use sar_pp_scan_set to cancel.", sar_pp_scan_savepath.GetString());
		break;
	default:
		break;
	}

	surface->DrawRectAndCenterTxt(Color{0, 0, 0, 0}, 0, 0, sw, sh - 50, 6, Color{255, 255, 255}, status_text.c_str());

	if (g_setup.state == SetupState::RUNNING) {
		float percentage = 100.0f * (float)g_scan.widthProgress / (float)g_scan.maxWidth;
		surface->DrawRectAndCenterTxt(Color{0, 0, 0, 200}, 0, 0, sw, sh + 50, 6, Color{255, 255, 255}, "%.1f%%", percentage);
	}
}

CON_COMMAND(sar_pp_scan_set, "sar_pp_scan_set - set the ppscan point where you're aiming.\n") {
	if (!checkScanningAvailable()) {
		g_setup.state = SetupState::NONE;
		return;
	}

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
		startAreaScan();
		return;
	case SetupState::RUNNING:
		endScan(false);
		return;
	}
}

CON_COMMAND(sar_pp_scan_reset, "sar_pp_scan_reset - reset ppscan.\n") {
	g_setup.state = SetupState::NONE;
}

CON_COMMAND(sar_pp_scan_screenshot, "sar_pp_scan_screenshot - renders current camera perspective using ppscan rays\n") {
	if (g_setup.state != SetupState::NONE) {
		console->Print("Cannot start perspective scan while another scan is in progress.\n");
		return;
	}

	if (!checkScanningAvailable()) {
		return;
	}

	startPerspectiveScan();
}
