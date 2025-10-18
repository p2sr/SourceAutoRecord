#include "Crosshair.hpp"

#include "Features/EntityList.hpp"
#include "Features/Hud/Hud.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/FileSystem.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Utils/lodepng.hpp"

#include <cctype>
#include <cfloat>
#include <filesystem>
#include <string>

Variable sar_crosshair_mode("sar_crosshair_mode", "0", 0, 2,
                            "Set the crosshair mode :\n"
                            "0: Default crosshair\n"
                            "1: Customizable crosshair\n"
                            "2: Crosshair from .png\n");
Variable sar_quickhud_mode("sar_quickhud_mode", "0", 0, 2,
                           "Set the quickhud mode :\n"
                           "0: Default quickhud\n"
                           "1: Customizable quickhud\n"
                           "2: quickhud from .png\n");

Variable sar_crosshair_P1("sar_crosshair_P1", "0", "Use the P1 crosshair style.\n");

Variable cl_crosshair_t("cl_crosshair_t", "0",
                        "Removes the top line from the crosshair :"
                        "0: normal crosshair,"
                        "1: crosshair without top.\n");
Variable cl_crosshairgap("cl_crosshairgap", "9", 0, "Changes the distance of the crosshair lines from the center of screen.\n");
Variable cl_crosshaircolor_r("cl_crosshaircolor_r", "255", 0, 255, "Changes the color of the crosshair.\n");
Variable cl_crosshaircolor_g("cl_crosshaircolor_g", "255", 0, 255, "Changes the color of the crosshair.\n");
Variable cl_crosshaircolor_b("cl_crosshaircolor_b", "255", 0, 255, "Changes the color of the crosshair.\n");
Variable cl_crosshairsize("cl_crosshairsize", "1", -100, "Changes the size of the crosshair.\n");
Variable cl_crosshairthickness("cl_crosshairthickness", "0", 0, "Changes the thickness of the crosshair lines.\n");
Variable cl_crosshairalpha("cl_crosshairalpha", "255", 0, 255, "Change the amount of transparency.\n");
Variable cl_crosshairdot("cl_crosshairdot", "1", "Decides if there is a dot in the middle of the crosshair\n");

Variable cl_quickhudleftcolor_r("cl_quickhudleftcolor_r", "255", 0, 255, "Changes the color of the left quickhud.\n");
Variable cl_quickhudleftcolor_g("cl_quickhudleftcolor_g", "184", 0, 255, "Changes the color of the left quickhud.\n");
Variable cl_quickhudleftcolor_b("cl_quickhudleftcolor_b", "86", 0, 255, "Changes the color of the left quickhud.\n");
Variable cl_quickhudrightcolor_r("cl_quickhudrightcolor_r", "111", 0, 255, "Changes the color of the right quickhud.\n");
Variable cl_quickhudrightcolor_g("cl_quickhudrightcolor_g", "184", 0, 255, "Changes the color of the right quickhud.\n");
Variable cl_quickhudrightcolor_b("cl_quickhudrightcolor_b", "255", 0, 255, "Changes the color of the right quickhud.\n");
Variable cl_quickhud_alpha("cl_quickhud_alpha", "255", 0, 255, "Change the amount of transparency.\n");

Crosshair crosshair;

Crosshair::Crosshair()
	: Hud(HudType_InGame, true)
	, crosshairTextureID(0)
	, quickhudTextureID{-1, -1, -1, -1}
	, isCustomCrosshairReady(false)
	, isCustomQuickHudReady(false)
	, images() {
}

bool Crosshair::ShouldDraw() {
	return Hud::ShouldDraw() && client->ShouldDrawCrosshair();
}

bool Crosshair::GetCurrentSize(int &xSize, int &ySize) {
	return false;
}

bool Crosshair::IsSurfacePortalable() {
	CGameTrace tr;
	if (!engine->TraceFromCamera<false>(65536.0, MASK_SHOT_PORTAL, tr)) {
		return false;
	}

	return !(tr.surface.flags & SURF_NOPORTAL) && std::strcmp(tr.surface.name, "**studio**") != 0;
}

static ClientEnt *getPortalGun(int slot) {
	ClientEnt *pl = client->GetPlayer(slot + 1);
	if (!pl) return nullptr;
	CBaseHandle hnd = pl->active_weapon();
	if (!hnd || !entityList->IsPortalGun(hnd)) return nullptr;
	return client->GetPlayer(hnd.GetEntryIndex());
}

int Crosshair::GetPortalUpgradeState(int slot) {
	ClientEnt *gun = getPortalGun(slot);

	if (gun) {
		bool prim = gun->field<bool>("m_bCanFirePortal1");
		bool sec = gun->field<bool>("m_bCanFirePortal2");
		return prim | (sec << 1);
	}

	return 0;
}

void Crosshair::GetPortalsStates(int slot, int &portalUpgradeState, bool &isBlueActive, bool &isOrangeActive) {
	isBlueActive = false;
	isOrangeActive = false;

	portalUpgradeState = GetPortalUpgradeState(slot);
	if (portalUpgradeState) {
		if (sar_crosshair_P1.GetBool() && sv_cheats.GetBool()) {
			if (this->IsSurfacePortalable()) {
				isBlueActive = true;
				isOrangeActive = true;
			} else {
				isBlueActive = false;
				isOrangeActive = false;
			}
			return;
		}

		ClientEnt *gun = getPortalGun(slot);
		if (gun) {
			// this doesn't actually work "correctly" regarding portals closing, but it mirrors the base game behaviour
			auto bluePos = gun->field<Vector>("m_vecBluePortalPos");
			auto orangePos = gun->field<Vector>("m_vecOrangePortalPos");
			if (bluePos != Vector{ FLT_MAX, FLT_MAX, FLT_MAX }) isBlueActive = true;
			if (orangePos != Vector{ FLT_MAX, FLT_MAX, FLT_MAX }) isOrangeActive = true;
			// with single portal gun, the crosshair is always active
			if (portalUpgradeState == 0b01) isBlueActive = true;
			if (portalUpgradeState == 0b10) isOrangeActive = true;
		}
	}
}

void Crosshair::Paint(int slot) {
	if (!sar_crosshair_mode.GetBool() && !sar_quickhud_mode.GetBool() && !sar_crosshair_P1.GetBool()) {
		return;
	}

	int xScreen, yScreen, xCenter, yCenter;
	engine->GetScreenSize(nullptr, xScreen, yScreen);

	if (engine->IsSplitscreen()) {
		if (ss_pipsplit.GetInt() == 2) {
			if (slot == 1) {
				xScreen *= ss_pipscale.GetFloat();
				yScreen *= ss_pipscale.GetFloat();
			}
		} else {
			if (ss_verticalsplit.GetBool()) {
				xScreen /= 2;
			} else {
				yScreen /= 2;
			}
		}
	}

	xCenter = xScreen / 2;
	yCenter = yScreen / 2;

	if (sar_crosshair_mode.GetInt() == 1) {  // Customizable crosshair
		Color c(cl_crosshaircolor_r.GetInt(), cl_crosshaircolor_g.GetInt(), cl_crosshaircolor_b.GetInt(), cl_crosshairalpha.GetInt());

		int gap = cl_crosshairgap.GetInt();
		int size = cl_crosshairsize.GetInt();
		int thickness = cl_crosshairthickness.GetInt() / 2;

		int x1_start, x1_end;
		x1_start = xCenter - gap;
		x1_end = x1_start - size;
		surface->DrawRect(c, x1_end, yCenter - thickness, x1_start, yCenter + thickness + 1);

		int x2_start, x2_end;
		x2_start = xCenter + gap;
		x2_end = x2_start + size;
		surface->DrawRect(c, x2_start + 1, yCenter - thickness, x2_end + 1, yCenter + thickness + 1);  //Right

		if (!cl_crosshair_t.GetBool()) {
			int y1_start, y1_end;
			y1_start = yCenter - gap;
			y1_end = y1_start - size;
			surface->DrawRect(c, xCenter - thickness, y1_end, xCenter + thickness + 1, y1_start);  //Top
		}

		int y2_start, y2_end;
		y2_start = yCenter + gap;
		y2_end = y2_start + size;
		surface->DrawRect(c, xCenter - thickness, y2_start + 1, xCenter + thickness + 1, y2_end + 1);  //Bottom

		if (cl_crosshairdot.GetBool()) {
			surface->DrawRect(c, xCenter, yCenter, xCenter + 1, yCenter + 1);
		}

	} else if (sar_crosshair_mode.GetInt() == 2 && this->isCustomCrosshairReady) {  // Crosshair from .png
		int halfSize = cl_crosshairsize.GetInt() / 2;

		surface->DrawSetColor(surface->matsurface->ThisPtr(), 255, 255, 255, cl_crosshairalpha.GetInt());
		surface->DrawSetTexture(surface->matsurface->ThisPtr(), this->crosshairTextureID);
		surface->DrawTexturedRect(surface->matsurface->ThisPtr(), xCenter - halfSize, yCenter - halfSize, xCenter + halfSize, yCenter + halfSize);
	}

	int portalGunUpgradeState;
	bool bluePortalState, orangePortalState;
	this->GetPortalsStates(slot, portalGunUpgradeState, bluePortalState, orangePortalState);

	if ((sar_quickhud_mode.GetInt() == 1 || sar_crosshair_P1.GetBool()) && portalGunUpgradeState) {  // Customizable quickhud
		Color cl(cl_quickhudleftcolor_r.GetInt(), cl_quickhudleftcolor_g.GetInt(), cl_quickhudleftcolor_b.GetInt(), cl_quickhud_alpha.GetInt());
		Color cr(cl_quickhudrightcolor_r.GetInt(), cl_quickhudrightcolor_g.GetInt(), cl_quickhudrightcolor_b.GetInt(), cl_quickhud_alpha.GetInt());

		int x1 = xCenter - 45;
		int x2 = xCenter + 45;
		int y1 = yCenter;
		int size = 15;

		if (bluePortalState) {
			surface->DrawFilledCircle(x1, y1, size, cr);
		} else {
			surface->DrawCircle(x1, y1, size, cr);
		}

		if (portalGunUpgradeState & 2) {
			if (orangePortalState) {
				surface->DrawFilledCircle(x2, y1, size, cl);
			} else {
				surface->DrawCircle(x2, y1, size, cl);
			}
		}

	} else if ((sar_quickhud_mode.GetInt() == 2 || sar_crosshair_P1.GetBool()) && this->isCustomQuickHudReady && portalGunUpgradeState) {  // Quickhud from .png
		int width, height;

		auto team = slot == 1 ? 2 : engine->IsCoop() ? 3 : 0;
		Color prim = SARUTIL_Portal_Color(1, team);
		Color sec  = SARUTIL_Portal_Color(2, team);

		// HACKHACK: Stupid color adjustment to match vanilla crosshair
		// https://www.desmos.com/calculator/7gd17bkmos
#define COLORADJUST(col) \
		if (col.r > 4) col.r = col.r * 0.75f + 64; \
		if (col.g > 4) col.g = col.g * 0.75f + 64; \
		if (col.b > 4) col.b = col.b * 0.75f + 64;
		COLORADJUST(prim);
		COLORADJUST(sec);
#undef COLORADJUST

		prim = (portalGunUpgradeState & 1) ? prim : sec;
		sec  = (portalGunUpgradeState & 2) ? sec  : prim;

		bool prim_state = (portalGunUpgradeState & 1) ? bluePortalState   : orangePortalState;
		bool sec_state  = (portalGunUpgradeState & 2) ? orangePortalState : bluePortalState;

		surface->DrawGetTextureSize(surface->matsurface->ThisPtr(), this->quickhudTextureID[prim_state], width, height);
		surface->DrawSetTexture(surface->matsurface->ThisPtr(), this->quickhudTextureID[prim_state]);
		surface->DrawSetColor(surface->matsurface->ThisPtr(), prim.r, prim.g, prim.b, cl_quickhud_alpha.GetInt());
		surface->DrawTexturedRect(surface->matsurface->ThisPtr(), xCenter + width/2 - width, yCenter - height/2, xCenter + width/2, yCenter - height/2 + height);

		surface->DrawGetTextureSize(surface->matsurface->ThisPtr(), this->quickhudTextureID[sec_state + 2], width, height);
		surface->DrawSetTexture(surface->matsurface->ThisPtr(), this->quickhudTextureID[sec_state + 2]);
		surface->DrawSetColor(surface->matsurface->ThisPtr(), sec.r, sec.g, sec.b, cl_quickhud_alpha.GetInt());
		surface->DrawTexturedRect(surface->matsurface->ThisPtr(), xCenter - width/2, yCenter - height/2, xCenter - width/2 + width, yCenter - height/2 + height);
	}
}

int Crosshair::SetCrosshairTexture(const std::string filename) {
	std::vector<unsigned char> png;
	std::vector<unsigned char> tex;
	unsigned width, height;

	auto path = filename;
	if (!Utils::EndsWith(path, ".png")) path += ".png";
	auto filepath = fileSystem->FindFileSomewhere(path).value_or(path);
	lodepng::load_file(png, filepath);
	if (png.empty()) {
		console->Warning(
			"Failed to load \"%s\"\n"
			"Make sure both the path and the filename are correct.\n",
			filename.c_str());
		return -1;
	}

	unsigned int error = lodepng::decode(tex, width, height, png);
	if (error) {
		console->Warning(lodepng_error_text(error));
		return -1;
	}

	int ID = surface->CreateNewTextureID(surface->matsurface->ThisPtr(), true);

	unsigned char *data = tex.data();
	surface->DrawSetTextureRGBA(surface->matsurface->ThisPtr(), ID, data, width, height);

	this->crosshairTextureID = ID;
	return ID;
}

bool Crosshair::SetQuickHudTexture(const std::string basefile) {
	for (int i = 1; i < 5; ++i) {
		std::vector<unsigned char> png;
		std::vector<unsigned char> tex;
		unsigned width, height;

		std::string filename(basefile);
		filename += std::to_string(i) + ".png";

		auto filepath = fileSystem->FindFileSomewhere(filename).value_or(filename);
		lodepng::load_file(png, filepath);
		if (png.empty()) {
			console->Warning(
				"Failed to load \"%s\"\n"
				"Make sure both the path and the filename are correct.\n",
				filename.c_str());
			return false;
		}

		unsigned int error = lodepng::decode(tex, width, height, png);
		if (error) {
			console->Warning(lodepng_error_text(error));
			return false;
		}

		int ID = surface->CreateNewTextureID(surface->matsurface->ThisPtr(), true);

		unsigned char *data = tex.data();
		surface->DrawSetTextureRGBA(surface->matsurface->ThisPtr(), ID, data, width, height);

		this->quickhudTextureID[i - 1] = ID;
	}

	return true;
}

// Commands

DECL_COMMAND_FILE_COMPLETION(sar_crosshair_set_texture, ".png", "", 1);
CON_COMMAND_F_COMPLETION(sar_crosshair_set_texture, "sar_crosshair_set_texture <filepath>\n", 0, AUTOCOMPLETION_FUNCTION(sar_crosshair_set_texture)) {
	if (args.ArgC() < 2) {
		return console->Print(sar_crosshair_set_texture.ThisPtr()->m_pszHelpString);
	}

	auto filePath = std::string(args[1]);
	if (filePath.substr(filePath.length() - 4, 4) == ".png")
		filePath.erase(filePath.end() - 4, filePath.end());

	if (crosshair.SetCrosshairTexture(filePath) == -1) {
		crosshair.isCustomCrosshairReady = false;
		return;
	}

	crosshair.isCustomCrosshairReady = true;
}

DECL_COMMAND_FILE_COMPLETION(sar_quickhud_set_texture, ".png", "", 1)
CON_COMMAND_F_COMPLETION(sar_quickhud_set_texture,
                         "sar_quickhud_set_texture <filepath> - enter the base name, it will search for <filepath>1.png, <filepath>2.png, <filepath>3.png and <filepath>4.png\n"
                         "ex: sar_quickhud_set_texture \"crosshair/basic\" looks for \"crosshair/basic1.png\", etc\n",
                         0,
                         AUTOCOMPLETION_FUNCTION(sar_quickhud_set_texture)) {
	if (args.ArgC() < 2) {
		return console->Print(sar_quickhud_set_texture.ThisPtr()->m_pszHelpString);
	}

	auto filePath = std::string(args[1]);
	if (Utils::EndsWith(filePath, ".png"))
		filePath.erase(filePath.end() - 4, filePath.end());

	// A bit of smartness to help out (make autocomplete work better)
	if (!std::filesystem::exists(filePath + "1.png")
		&& std::isdigit(filePath[filePath.length() - 1])) {
		filePath.erase(filePath.end() - 1, filePath.end());
	}

	if (!crosshair.SetQuickHudTexture(filePath)) {
		crosshair.isCustomQuickHudReady = false;
		return;
	}

	crosshair.isCustomQuickHudReady = true;
}
