#include "Features/Hud/Hud.hpp"
#include "Modules/Scheme.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Client.hpp"
#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"
#include "Variable.hpp"

Variable sar_cheat_hud("sar_cheat_hud", "1", 0, 2, "Display a warning in the HUD when cheats are active. 0 = disable, 1 = display if sv_cheats off, 2 = display always\n");
Variable sar_cheat_hud_x("sar_cheat_hud_x", "-4", "X position of the cheat warning HUD.\n", 0);
Variable sar_cheat_hud_y("sar_cheat_hud_y", "4", "Y position of the cheat warning HUD.\n", 0);

struct Cheat {
	bool (*isActive)();
	const char *warning;
	const char *fix;
};

static Cheat g_cheats[] = {
	{ +[]() {
		return sv_cheats.GetBool();
	}, "cheats are enabled", "run 'sv_cheats 0'" },

	{ +[]() {
		if (sar.game->Is(SourceGame_PortalReloaded)) return false;
		if (client->GetChallengeStatus() != CMStatus::CHALLENGE) return false;
		return Variable("sv_cheats_flagged").GetBool();
	}, "cheats were flagged in this session", "reload the game from the menu" },

	{ +[]() {
		return Variable("cl_cmdrate").GetFloat() != 30.0f;
	}, "cl_cmdrate is at a non-default value", "run 'cl_cmdrate 30'" },

	{ +[]() {
		return Variable("cl_updaterate").GetFloat() != 20.0f;
	}, "cl_updaterate is at a non-default value", "run 'cl_updaterate 20'" },

	{ +[]() {
		return Variable("phys_timescale").GetFloat() != 1.0f;
	}, "phys_timescale is at a non-default value", "run 'phys_timescale 1'" },

	{ +[]() {
		return Variable("mat_filtertextures").GetInt() != 1;
	}, "mat_filtertextures is at a non-default value", "run 'mat_filtertextures 1'" },

	{ +[]() {
		return Variable("fps_max").GetFloat() <= 29.0f || Variable("fps_max").GetFloat() >= 1000.0f;
	}, "fps_max is at a banned value", "set 'fps_max' between 30 and 999" },

	{ +[]() {
		return Variable("sv_portal_placement_debug").GetBool();
	}, "portal placement debug is enabled", "run 'sv_portal_placement_debug 0'" },

	{ +[]() {
		auto map = engine->GetCurrentMapName();
		if (!sar.game->Is(SourceGame_Portal2)) return false;
		if (map == "sp_a2_bts5") return false;
		if (engine->GetMapIndex(map) == -1) return false;
		return Variable("sv_allow_mobile_portals").GetBool();
	}, "mobile portals enabled", "set 'sv_allow_mobile_portals 0'" },

	{ +[]() {
		return fabsf(*engine->interval_per_tick - 1.0f / 60.0f) > 0.00001f;
	}, "tickrate is not 60", "remove '-tickrate' from the game launch options" },
};

class CheatWarnHud : public Hud {
public:
	CheatWarnHud()
		: Hud(HudType_InGame | HudType_Paused | HudType_Menu | HudType_LoadingScreen, false) {
	}

	bool ShouldDraw() override {
		if (!Hud::ShouldDraw()) return false;

		switch (sar_cheat_hud.GetInt()) {
			case 0:
				return false;
			case 1:
				return !sv_cheats.GetBool();
			case 2:
			default:
				return true;
		}
	}

	bool GetCurrentSize(int &w, int &h) override {
		return false;
	}

	void drawLine(bool draw, int x, int &y, int &width, const char *text, int font_idx, Color col) {
		Surface::HFont f = scheme->GetFontByID(font_idx);

		if (draw) {
			surface->DrawTxt(f, x, y, col, "%s", text);
		}

		int font_h = surface->GetFontHeight(f);
		int font_w = surface->GetFontLength(f, "%s", text);

		if (font_w > width) width = font_w;
		y += font_h;
	}

	// x and y ignored if draw is false
	// will output text area size into w and h if not null
	void drawWarnings(bool draw, int x, int y, int *w_out, int *h_out) {
		int width = 0;
		int init_y = y;

		for (auto &cheat : g_cheats) {
			if (cheat.isActive()) {
				drawLine(draw, x, y, width, cheat.warning, 17, Color{255, 100, 100});
				y -= 3; // go a few px above the bottom of the line
				drawLine(draw, x, y, width, cheat.fix, 18, Color{150, 150, 150});
				y += 2; // 2px padding before next line
			}
		}

		if (y > init_y) {
			// remove final padding
			y -= 2;
		}

		if (w_out) *w_out = width;
		if (h_out) *h_out = y - init_y;
	}

	void Paint(int slot) override {
		int width, height;
		drawWarnings(false, 0, 0, &width, &height);

		if (width == 0 || height == 0) return;

		// 4px horizontal padding, 2px vertical padding
		width += 8;
		height += 4;

		int scr_width, scr_height;
		engine->GetScreenSize(nullptr, scr_width, scr_height);

		int x = sar_cheat_hud_x.GetInt();
		if (x < 0) x += scr_width - width;

		int y = sar_cheat_hud_y.GetInt();
		if (y < 0) y += scr_height - height;

		surface->DrawRect(Color{0, 0, 0, 192}, x, y, x + width, y + height);
		drawWarnings(true, x + 4, y + 2, nullptr, nullptr);
	}
};

CheatWarnHud cheatWarnHud;
