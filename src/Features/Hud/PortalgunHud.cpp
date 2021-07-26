#include "PortalgunHud.hpp"

#include "Features/EntityList.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"

#include <cstdint>

PortalgunHud portalgunHud;

Variable sar_portalgun_hud("sar_portalgun_hud", "0", "Enables the portalgun HUD.\n");
Variable sar_portalgun_hud_x("sar_portalgun_hud_x", "5", 0, "The x position of the portalgun HUD.\n");
Variable sar_portalgun_hud_y("sar_portalgun_hud_y", "5", 0, "The y position of the portalgun HUD.\n");

PortalgunHud::PortalgunHud()
	: Hud(HudType_InGame, true) {
}

bool PortalgunHud::ShouldDraw() {
	return sar_portalgun_hud.GetBool() && Hud::ShouldDraw() && sv_cheats.GetBool();
}

bool PortalgunHud::GetCurrentSize(int &w, int &h) {
	return false;
}

void PortalgunHud::Paint(int slot) {
	if (!engine->isRunning()) {
		// we're not the server
		return;
	}

	int x = sar_portalgun_hud_x.GetInt();
	int y = sar_portalgun_hud_y.GetInt();

	auto font = scheme->GetDefaultFont() + 1;

	int lineHeight = surface->GetFontHeight(font) + 5;

	void *player = server->GetPlayer(slot + 1);
	if (!player) {
		return;
	}

	auto m_hActiveWeapon = *(CBaseHandle *)((uintptr_t)player + Offsets::m_hActiveWeapon);
	uintptr_t portalgun = (uintptr_t)entityList->LookupEntity(m_hActiveWeapon);

	if (!portalgun) {
		surface->DrawTxt(font, x, y, Color{255, 150, 150, 255}, "no held portalgun");
		return;
	}

	uint8_t linkage = *(unsigned char *)(portalgun + Offsets::m_iPortalLinkageGroupID);

	surface->DrawTxt(font, x, y, Color{255, 255, 255, 255}, "linkage: %d", (int)linkage);
	y += lineHeight + 10;

	auto m_hPrimaryPortal = *(CBaseHandle *)(portalgun + Offsets::m_hPrimaryPortal);
	auto m_hSecondaryPortal = *(CBaseHandle *)(portalgun + Offsets::m_hSecondaryPortal);

	auto bluePortal = (uintptr_t)entityList->LookupEntity(m_hPrimaryPortal);
	auto orangePortal = (uintptr_t)entityList->LookupEntity(m_hSecondaryPortal);

	for (int i = 0; i < 2; ++i) {
		uintptr_t portal = i == 0 ? bluePortal : orangePortal;
		CBaseHandle handle = i == 0 ? m_hPrimaryPortal : m_hSecondaryPortal;

		Color col = Color{255, 150, 150, 255};

		if (portal) {
			bool active = *(bool *)(portal + Offsets::m_bActivated);
			col = active ? Color{150, 255, 150, 255} : Color{255, 200, 150, 255};
		}

		surface->DrawTxt(font, x, y, col, "%s: 0x%08X", i == 0 ? "primary" : "secondary", handle.m_Index);
		if (portal) {
			Vector pos = server->GetAbsOrigin((void *)portal);
			surface->DrawTxt(font, x + 20, y + lineHeight, Color{255, 255, 255, 255}, "%.3f, %.3f, %.3f", pos.x, pos.y, pos.z);
		}

		y += 2 * lineHeight + 10;
	}
}
