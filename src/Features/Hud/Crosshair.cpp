#include "Crosshair.hpp"

#include "Features/EntityList.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/OffsetFinder.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Utils/lodepng.hpp"

#include <cctype>
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
Variable cl_crosshairgap("cl_crosshairgap", "5", 0, "Changes the distance of the crosshair lines from the center of screen.\n");
Variable cl_crosshaircolor_r("cl_crosshaircolor_r", "0", 0, 255, "Changes the color of the crosshair.\n");
Variable cl_crosshaircolor_g("cl_crosshaircolor_g", "255", 0, 255, "Changes the color of the crosshair.\n");
Variable cl_crosshaircolor_b("cl_crosshaircolor_b", "0", 0, 255, "Changes the color of the crosshair.\n");
Variable cl_crosshairsize("cl_crosshairsize", "5", -100, "Changes the size of the crosshair.\n");
Variable cl_crosshairthickness("cl_crosshairthickness", "0", 0, "Changes the thinkness of the crosshair lines.\n");
Variable cl_crosshairalpha("cl_crosshairalpha", "255", 0, 255, "Change the amount of transparency.\n");
Variable cl_crosshairdot("cl_crosshairdot", "1", "Decides if there is a dot in the middle of the crosshair\n");

Variable cl_quickhud_x("sar_quickhud_x", "45", -1000, "Horizontal distance of the custom quickhud.\n");
Variable cl_quickhud_y("sar_quickhud_y", "0", -1000, "Vertical distance of the custom quickhud.\n");
Variable cl_quickhud_size("sar_quickhud_size", "15", -100, "Size of the custom quickhud.\n");
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
	void *player = server->GetPlayer(GET_SLOT() + 1);

	if (player == nullptr || (int)player == -1)
		return false;

	Vector camPos = server->GetAbsOrigin(player) + server->GetViewOffset(player);

	QAngle angle = engine->GetAngles(GET_SLOT());

	float X = DEG2RAD(angle.x), Y = DEG2RAD(angle.y);
	auto cosX = std::cos(X), cosY = std::cos(Y);
	auto sinX = std::sin(X), sinY = std::sin(Y);

	Vector dir(cosY * cosX, sinY * cosX, -sinX);

	Vector finalDir = Vector(dir.x, dir.y, dir.z).Normalize() * 65536.0;

	Ray_t ray;
	ray.m_IsRay = true;
	ray.m_IsSwept = true;
	ray.m_Start = VectorAligned(camPos.x, camPos.y, camPos.z);
	ray.m_Delta = VectorAligned(finalDir.x, finalDir.y, finalDir.z);
	ray.m_StartOffset = VectorAligned();
	ray.m_Extents = VectorAligned();

	CTraceFilterSimple filter;
	filter.SetPassEntity(server->GetPlayer(GET_SLOT() + 1));

	CGameTrace tr;
	engine->TraceRay(engine->engineTrace->ThisPtr(), ray, MASK_SHOT_PORTAL, &filter, &tr);

	if (tr.fraction >= 1) {
		return false;
	}

	return !(tr.surface.flags & SURF_NOPORTAL) && std::strcmp(tr.surface.name, "**studio**") != 0;
}

int Crosshair::GetPortalUpgradeState() {
	if (server->portalGun != nullptr) {
		return (*reinterpret_cast<bool *>((uintptr_t)server->portalGun + Offsets::m_bCanFirePortal1) + *reinterpret_cast<bool *>((uintptr_t)server->portalGun + Offsets::m_bCanFirePortal2));
	}

	return 0;
}

std::vector<IHandleEntity *> Crosshair::GetPortalsShotByPlayer() {
	std::vector<IHandleEntity *> v;

	if (server->portalGun) {
		auto bluePortal = entityList->LookupEntity(*reinterpret_cast<CBaseHandle *>((uintptr_t)server->portalGun + Offsets::m_hPrimaryPortal));
		auto orangePortal = entityList->LookupEntity(*reinterpret_cast<CBaseHandle *>((uintptr_t)server->portalGun + Offsets::m_hSecondaryPortal));

		if (bluePortal != NULL) {  //If player hasn't shot blue portal
			v.push_back(bluePortal);
		}

		if (orangePortal != NULL) {  //If player hasn't shot orange portal
			v.push_back(orangePortal);
		}
	}

	return v;
}

void Crosshair::GetPortalsStates(int &portalUpgradeState, bool &isBlueActive, bool &isOrangeActive) {
	portalUpgradeState = GetPortalUpgradeState();
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

		isBlueActive = false;
		isOrangeActive = false;
		for (auto &portal : GetPortalsShotByPlayer()) {
			bool isP2 = *reinterpret_cast<bool *>((uintptr_t)portal + Offsets::m_bIsPortal2);   //IsOrange
			bool isAct = *reinterpret_cast<bool *>((uintptr_t)portal + Offsets::m_bActivated);  //IsActive
			if (!isP2) {
				isBlueActive = isAct;
			} else {
				isOrangeActive = isAct;
			}
		}
	}
}

void Crosshair::Paint(int slot) {
	if (!sar_crosshair_mode.GetBool() && !sar_quickhud_mode.GetBool() && !sar_crosshair_P1.GetBool()) {
		return;
	}

	surface->StartDrawing(surface->matsurface->ThisPtr());

	int xScreen, yScreen, xCenter, yCenter;
	engine->GetScreenSize(nullptr, xScreen, yScreen);

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
	this->GetPortalsStates(portalGunUpgradeState, bluePortalState, orangePortalState);

	if ((sar_quickhud_mode.GetInt() == 1 || sar_crosshair_P1.GetBool()) && portalGunUpgradeState) {  // Customizable quickhud
		Color cl(cl_quickhudleftcolor_r.GetInt(), cl_quickhudleftcolor_g.GetInt(), cl_quickhudleftcolor_b.GetInt(), cl_quickhud_alpha.GetInt());
		Color cr(cl_quickhudrightcolor_r.GetInt(), cl_quickhudrightcolor_g.GetInt(), cl_quickhudrightcolor_b.GetInt(), cl_quickhud_alpha.GetInt());

		int x1 = xCenter - cl_quickhud_x.GetInt();
		int x2 = xCenter + cl_quickhud_x.GetInt();
		int y1 = yCenter + cl_quickhud_y.GetInt();
		int size = cl_quickhud_size.GetInt();

		if (bluePortalState) {
			surface->DrawFilledCircle(x1, y1, size, cr);
		} else {
			surface->DrawCircle(x1, y1, size, cr);
		}

		if (portalGunUpgradeState == 2) {
			if (orangePortalState) {
				surface->DrawFilledCircle(x2, y1, size, cl);
			} else {
				surface->DrawCircle(x2, y1, size, cl);
			}
		}

	} else if ((sar_quickhud_mode.GetInt() == 2 || sar_crosshair_P1.GetBool()) && this->isCustomQuickHudReady && portalGunUpgradeState) {  // Quickhud from .png

		surface->DrawSetColor(surface->matsurface->ThisPtr(), 255, 255, 255, cl_quickhud_alpha.GetInt());
		int halfSize = cl_quickhud_size.GetInt() / 2;
		int xOffset = cl_quickhud_x.GetInt();
		int yOffset = cl_quickhud_y.GetInt();

		surface->DrawSetTexture(surface->matsurface->ThisPtr(), this->quickhudTextureID[bluePortalState]);  //Blue
		surface->DrawTexturedRect(surface->matsurface->ThisPtr(), xCenter - halfSize * 2 - xOffset, yCenter - halfSize - yOffset, xCenter - xOffset, yCenter + halfSize - yOffset);

		if (portalGunUpgradeState) {
			surface->DrawSetTexture(surface->matsurface->ThisPtr(), this->quickhudTextureID[orangePortalState + 2]);  //Orange
			surface->DrawTexturedRect(surface->matsurface->ThisPtr(), xCenter + xOffset, yCenter - halfSize - yOffset, xCenter + halfSize * 2 + xOffset, yCenter + halfSize - yOffset);
		}
	}

	surface->FinishDrawing();
}

int Crosshair::SetCrosshairTexture(const std::string filename) {
	std::vector<unsigned char> png;
	std::vector<unsigned char> tex;
	unsigned width, height;

	lodepng::load_file(png, filename + ".png");
	if (png.empty()) {
		console->Warning(
			"Failed to load \"%s\"\n"
			"Make sure both the path and the filename are correct.\n",
			filename);
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

		lodepng::load_file(png, filename);
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

void Crosshair::UpdateImages() {
	this->images.clear();
	auto path = std::string(engine->GetGameDirectory());
	auto index = path.length() + 1;

	// Scan through all directories and find the image file
	for (auto &dir : std::filesystem::recursive_directory_iterator(path)) {
		if (dir.status().type() == std::filesystem::file_type::directory) {
			auto curdir = dir.path().string();
			for (auto &dirdir : std::filesystem::directory_iterator(curdir)) {
				auto file = dir.path().string();
				if (Utils::EndsWith(file, std::string(".png"))) {
					auto img = file.substr(index);
					if (std::isdigit(img[img.length() - 5])) {  //Take only images with a digit as last character
						img = img.substr(0, img.length() - 5);
						this->images.push_back(img);
					}
					break;
				}
			}
		} else {
			auto file = dir.path().string();
			if (Utils::EndsWith(file, std::string(".png"))) {
				auto img = file.substr(index);
				if (std::isdigit(img[img.length() - 5])) {  //Take only images with a digit as last character
					img = img.substr(0, img.length() - 5);
					this->images.push_back(img);
				}
				break;
			}
		}
	}
}

// Commands

CON_COMMAND_AUTOCOMPLETEFILE(sar_crosshair_set_texture, "sar_crosshair_set_texture <filepath>\n", 0, 0, png) {
	if (args.ArgC() < 2) {
		return console->Print(sar_crosshair_set_texture.ThisPtr()->m_pszHelpString);
	}

	auto filePath = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
	if (filePath.substr(filePath.length() - 4, 4) == ".png")
		filePath.erase(filePath.end() - 4, filePath.end());

	if (crosshair.SetCrosshairTexture(filePath) == -1) {
		crosshair.isCustomCrosshairReady = false;
		return;
	}

	crosshair.isCustomCrosshairReady = true;
}

DECL_COMMAND_COMPLETION(sar_quickhud_set_texture) {
	crosshair.UpdateImages();

	for (auto &image : crosshair.images) {
		if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
			break;
		}

		if (std::strlen(match) != std::strlen(cmd)) {
			if (std::strstr(image.c_str(), match)) {
				items.push_back(image);
			}
		} else {
			items.push_back(image);
		}
	}

	FINISH_COMMAND_COMPLETION();
}

CON_COMMAND_F_COMPLETION(sar_quickhud_set_texture,
                         "sar_quickhud_set_texture <filepath> - enter the base name, it will search for <filepath>1.png, <filepath>2.png, <filepath>3.png and <filepath>4.png\n"
                         "ex: sar_quickhud_set_texture \"E:\\Steam\\steamapps\\common\\Portal 2\\portal2\\krzyhau\"\n",
                         0,
                         sar_quickhud_set_texture_CompletionFunc) {
	if (args.ArgC() < 2) {
		return console->Print(sar_quickhud_set_texture.ThisPtr()->m_pszHelpString);
	}

	auto filePath = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
	if (filePath.substr(filePath.length() - 4, 4) == ".png")
		filePath.erase(filePath.end() - 4, filePath.end());

	if (!crosshair.SetQuickHudTexture(filePath)) {
		crosshair.isCustomQuickHudReady = false;
		return;
	}

	crosshair.isCustomQuickHudReady = true;
}
