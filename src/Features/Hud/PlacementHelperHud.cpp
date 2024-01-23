#include <Variable.hpp>
#include <Cheats.hpp>
#include <Event.hpp>

#include <Features/EntityList.hpp>
#include <Modules/Server.hpp>
#include <Modules/Engine.hpp>
#include <Features/OverlayRender.hpp>


Variable sar_placement_helper_hud("sar_placement_helper_hud", "0", "Visually displays all portal placement helpers (requires sv_cheats).\n");

static const int g_sphereMeshRings = 8;
static const int g_sphereMeshSegments = 16;
static const int g_sphereMeshVerticesCount = g_sphereMeshRings * g_sphereMeshSegments * 3 * 2;
static Vector g_sphereMeshVertices[g_sphereMeshVerticesCount];
static bool g_sphereMeshGenerated = false;


static void generateSphereMesh() {
	if (g_sphereMeshGenerated) return;

	for (int r = 0; r < g_sphereMeshRings; r++) {
		for (int s = 0; s < g_sphereMeshSegments; s++) {
			float r1 = M_PI * r / g_sphereMeshRings - M_PI * 0.5f;
			float r2 = M_PI * (r + 1) / g_sphereMeshRings - M_PI * 0.5f;
			float s1 = M_PI * 2.0f * s / g_sphereMeshSegments;
			float s2 = M_PI * 2.0f * (s + 1) / g_sphereMeshSegments;

			Vector p1 = (Vector(cos(s1), sin(s1), 0) * cos(r1) + Vector(0, 0, sin(r1)));
			Vector p2 = (Vector(cos(s2), sin(s2), 0) * cos(r1) + Vector(0, 0, sin(r1)));
			Vector p3 = (Vector(cos(s2), sin(s2), 0) * cos(r2) + Vector(0, 0, sin(r2)));
			Vector p4 = (Vector(cos(s1), sin(s1), 0) * cos(r2) + Vector(0, 0, sin(r2)));

			int trindex = (r * g_sphereMeshSegments + s) * 6.0f;
			g_sphereMeshVertices[trindex + 0] = p1;
			g_sphereMeshVertices[trindex + 1] = p2;
			g_sphereMeshVertices[trindex + 2] = p3;
			g_sphereMeshVertices[trindex + 3] = p1;
			g_sphereMeshVertices[trindex + 4] = p3;
			g_sphereMeshVertices[trindex + 5] = p4;
		}
	}
	g_sphereMeshGenerated = true;
}


static void drawSphere(Vector position, float size, Color color) {
	generateSphereMesh();
	MeshId mesh = OverlayRender::createMesh(RenderCallback::constant(color), RenderCallback::none);

	for (int v = 0; v < g_sphereMeshVerticesCount; v += 3) {
		Vector v1 = position + g_sphereMeshVertices[v + 0] * size;
		Vector v2 = position + g_sphereMeshVertices[v + 1] * size;
		Vector v3 = position + g_sphereMeshVertices[v + 2] * size;
		OverlayRender::addTriangle(mesh, v1, v2, v3);
	}
}



ON_EVENT(RENDER) {

	if (!sv_cheats.GetBool()) return;
	if (!engine->hoststate->m_activeGame) return;
	if (!sar_placement_helper_hud.GetBool()) return;

	for (auto index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
		auto info = entityList->GetEntityInfoByIndex(index);
		if (info->m_pEntity == nullptr) continue;

		auto entityClass = server->GetEntityClassName(info->m_pEntity);
		if (!entityClass || std::strcmp(entityClass, "info_placement_helper") != 0) continue;

		auto placementHelper = SE(info->m_pEntity);

		auto position = placementHelper->abs_origin();
#ifdef _WIN32
		// no idea honestly. it's just there.
		auto size = placementHelper->fieldOff<float>("m_flRadius", 648);
#else
		auto size = placementHelper->fieldOff<float>("m_flRadius", 664);
#endif
		auto forcePlacement = placementHelper->field<bool>("m_bForcePlacement");
		auto snapToHelperAngles = placementHelper->field<bool>("m_bSnapToHelperAngles");
		auto disabled = placementHelper->field<bool>("m_bDisabled");
		auto disableTime = placementHelper->field<float>("m_flDisableTime");
		auto deferringToPortal = placementHelper->field<bool>("m_bDeferringToPortal");

		auto currentTime = server->gpGlobals->curtime;
		auto remainingBlockedTimeTicks = (int)(fmaxf(disableTime - currentTime, 0.0f) * 60.0f);
		auto placementBlocked = deferringToPortal || (remainingBlockedTimeTicks > 0);

		auto sphereColor = Color(0, 200, 0, 64);
		std::string state = "enabled";
		if (disabled) {
			sphereColor = Color(255, 0, 0, 64);
			state = "disabled";
		}
		else if (placementBlocked) {
			sphereColor = Color(255, 64, 0, 64);
			if (deferringToPortal) {
				state = "blocked (by portal)";
			} else {
				state = Utils::ssprintf("blocked (%d ticks remaining)", remainingBlockedTimeTicks);
			}
		}
		else if (!forcePlacement) {
			sphereColor = Color(150, 255, 0, 64);
		}
			

		drawSphere(position, size, sphereColor);

		OverlayRender::addText(position, Utils::ssprintf(
			"Snap to helper angles: %s\nForce placement:%s\nState: %s", 
			snapToHelperAngles ? "yes" : "no", 
			forcePlacement ? "yes" : "no", 
			state.c_str()
		), 2.0f, true, true);

		//m_bSnapToHelperAngles
		//m_flRadius
		//m_bForcePlacement
		//m_bDisabled
		//m_flDisableTime
		//m_bDeferringToPortal
	}
}


