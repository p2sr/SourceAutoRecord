#include "PortalPlacement.hpp"

#include "Command.hpp"
#include "Variable.hpp"
#include "Event.hpp"
#include "Features/Camera.hpp"
#include "Features/OverlayRender.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Features/Hud/Hud.hpp" 
#include "Features/EntityList.hpp"

#include <string>

PortalPlacementHud portalplacementHud;

Variable sar_pp_hud("sar_pp_hud", "0", 0, "Enables or disables the portals placement HUD.\n");
Variable sar_pp_hud_show_blue("sar_pp_hud_show_blue", "0", 0, "Enables or disables blue portal preview.\n");
Variable sar_pp_hud_show_orange("sar_pp_hud_show_orange", "0", 0, "Enables or disables orange portal preview.\n");
Variable sar_pp_hud_x("sar_pp_hud_x", "5", "x pos of portal placement hud.\n", 0);
Variable sar_pp_hud_y("sar_pp_hud_y", "5", "y pos of portal placement hud.\n", 0);
Variable sar_pp_hud_opacity("sar_pp_hud_opacity", "100", 0, 255, "Opacity of portal previews.\n", 0);
Variable sar_pp_hud_font("sar_pp_hud_font", "0", 0, "Change font of portal placement hud.\n");

bool g_hasPortalGun;
bool g_canPlaceBlue;
bool g_canPlaceOrange;
TracePortalPlacementInfo_t g_bluePlacementInfo;
TracePortalPlacementInfo_t g_orangePlacementInfo;

PortalPlacementHud::PortalPlacementHud()
	: Hud(HudType_InGame, true) {
}
bool PortalPlacementHud::ShouldDraw() {
	bool shouldDraw = sv_cheats.GetBool() && sar_pp_hud.GetBool() && Hud::ShouldDraw();
	return shouldDraw;
}
void PortalPlacementHud::Paint(int slot) {
	auto font = scheme->GetFontByID(sar_pp_hud_font.GetInt());

	int cX = PositionFromString(sar_pp_hud_x.GetString(), true);
	int cY = PositionFromString(sar_pp_hud_y.GetString(), false);
	int charHeight = surface->GetFontHeight(font);
	int padding = 5;

	auto white =  Color(255,255,255,255);
	auto gray =   Color(150,150,150,255);
	auto blue =   Color(111,184,255,255);
	auto orange = Color(255,184, 86,255);

	surface->DrawTxt(font, cX, cY, white, "Portal placement HUD");
	cY += charHeight + padding;

	if (!g_hasPortalGun) {
		surface->DrawTxt(font, cX, cY, gray, "No portal gun");
		return;
	}

	auto drawPlacementInfo = [&](Color portalColor, TracePortalPlacementInfo_t info, const char *name) {
		std::string result;
		switch (info.ePlacementResult) {
		// Success
		case PORTAL_PLACEMENT_SUCCESS:     result = "Success";             break;
		case PORTAL_PLACEMENT_USED_HELPER: result = "Success with helper"; break;
		case PORTAL_PLACEMENT_BUMPED:      result = "Success with bump";   break;

		// Fail
		case PORTAL_PLACEMENT_CANT_FIT:               result = "Can't fit";                break;
		case PORTAL_PLACEMENT_CLEANSER:               result = "Fizzler";                  break;
		case PORTAL_PLACEMENT_OVERLAP_LINKED:         result = "Overlaps existing portal"; break;
		case PORTAL_PLACEMENT_OVERLAP_PARTNER_PORTAL: result = "Overlaps partner portal";  break;
		case PORTAL_PLACEMENT_INVALID_VOLUME:         result = "Invalid volume";           break;
		case PORTAL_PLACEMENT_INVALID_SURFACE:        result = "Invalid surface";          break;
		case PORTAL_PLACEMENT_PASSTHROUGH_SURFACE:    result = "Passthrough surface";      break;
		default: result = "Unknown result"; break;
		}

		auto color = info.ePlacementResult<=2 ? portalColor : gray;

		surface->DrawTxt(font, cX, cY, color, "%s: %s", name, result.c_str());
		cY += charHeight + padding;
	};

	drawPlacementInfo(blue, g_bluePlacementInfo, "Blue");
	drawPlacementInfo(orange, g_orangePlacementInfo, "Orange");
}
bool PortalPlacementHud::GetCurrentSize(int &xSize, int &ySize) {
	return false;
}

ON_EVENT(PRE_TICK) {
	// update portal placement info
	// Will fizzle partner portals in coop
	if (sv_cheats.GetBool() && sar_pp_hud.GetBool() && event.simulating) {
		void *player = server->GetPlayer(GET_SLOT() + 1);

		if (player == nullptr || (int)player == -1)
			return;

		Vector camPos;
		QAngle angle;
		camera->GetEyePos<true>(GET_SLOT(), camPos, angle);

		float X = DEG2RAD(angle.x), Y = DEG2RAD(angle.y);
		auto cosX = std::cos(X), cosY = std::cos(Y);
		auto sinX = std::sin(X), sinY = std::sin(Y);

		Vector dir(cosY * cosX, sinY * cosX, -sinX);

		// Get all the stuffs
		uintptr_t portalgun = (uintptr_t)entityList->LookupEntity(SE(player)->active_weapon());
		g_hasPortalGun = !!portalgun;
		if (!g_hasPortalGun)
			return;

		uint8_t linkage = SE(portalgun)->field<unsigned char>("m_iPortalLinkageGroupID");

		auto m_hPrimaryPortal = SE(portalgun)->field<CBaseHandle>("m_hPrimaryPortal");
		auto m_hSecondaryPortal = SE(portalgun)->field<CBaseHandle>("m_hSecondaryPortal");
		auto bluePortal = (uintptr_t)entityList->LookupEntity(m_hPrimaryPortal);
		auto orangePortal = (uintptr_t)entityList->LookupEntity(m_hSecondaryPortal);

		if (!bluePortal) {
			// spawn the portal
			bluePortal = server->FindPortal(linkage, false, true);
			SE(portalgun)->field<CBaseHandle>("m_hPrimaryPortal") = ((IHandleEntity *)bluePortal)->GetRefEHandle();
		}
		if (!orangePortal) {
			// spawn the portal
			orangePortal = server->FindPortal(linkage, true, true);
			SE(portalgun)->field<CBaseHandle>("m_hSecondaryPortal") = ((IHandleEntity *)orangePortal)->GetRefEHandle();
		}

		// Check blue
		g_canPlaceBlue = server->TraceFirePortal(portalgun, camPos, dir, false, 2, g_bluePlacementInfo);
		// Check Orange
		g_canPlaceOrange = server->TraceFirePortal(portalgun, camPos, dir, true, 2, g_orangePlacementInfo);
	}
}

ON_EVENT(RENDER) {
	if (sv_cheats.GetBool() && sar_pp_hud.GetBool()) {
		// Draw the shits in world

		auto blue =   Color(111, 184, 255, 255);
		auto orange = Color(255, 184,  86, 255);
		auto red =    Color(255,   0,   0, 255);

		auto drawPortal = [&](Color portalColor, TracePortalPlacementInfo_t info) {
			Vector origin = info.finalPos;
			QAngle angles = info.finalAngle;

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

			if (!(info.ePlacementResult<=2)) portalColor = red;
			portalColor.a = (uint8_t)sar_pp_hud_opacity.GetInt();

			MeshId mesh = OverlayRender::createMesh(RenderCallback::constant(portalColor), RenderCallback::none);

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
		};

		if (sar_pp_hud_show_blue.GetBool()) drawPortal(blue, g_bluePlacementInfo);
		if (sar_pp_hud_show_orange.GetBool()) drawPortal(orange, g_orangePlacementInfo);
	}
}
