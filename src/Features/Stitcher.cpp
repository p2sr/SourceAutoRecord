#include "Stitcher.hpp"
#include "Event.hpp"
#include "Modules/InputSystem.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Scheme.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

#include <cmath>
#include <cstdlib>
#include <vector>

#ifdef _WIN32
#	include <direct.h>
#else
#	include <sys/stat.h>
#endif

static void **g_videomode;

void Stitcher::Init(void **videomode) {
	g_videomode = videomode;
}

static struct {
	// The camera position
	long x;
	long y;
	long z;

	// We don't store x and y as floats to make sure the render is
	// precisely unit-aligned. However, we may want to move slower than 1
	// unit per tick
	float x_part;
	float y_part;
	float z_part;

	// Since idk how to un-capture the mouse, we'll make our OWN mouse!
	// Kinda hacky, but works
	int mousex;
	int mousey;

	// Selecting an area of the screen
	bool select_active;
	bool select_done;
	int select_x0;
	int select_y0;
	int select_x1;
	int select_y1;
	int select_edge; // 0 = top, 1 = right, 2 = bottom, 3 = left

	// For viewing the entire stitch
	bool stitch_view;
	int stitch_view_x;
	int stitch_view_y;
	float stitch_view_scale;
	bool stitch_view_mask;
	size_t stitch_view_selected; // SIZE_MAX = none
} g_stitcher;

struct StitchRegion {
	long xmin, xmax, ymin, ymax;
	long z;
	Color mask_color;
	std::vector<uint8_t> data;
	int texture_id;
};

static struct {
	long xmin, xmax, ymin, ymax;
	std::vector<StitchRegion> regions;
} g_stitch;

Variable sar_stitcher("sar_stitcher", "0", "Enable the image stitcher.\n");

static void resetStitcher() {
	// Zero everything
	g_stitcher = {0};

	// Set our initial coords to just above the player position
	void *player = server->GetPlayer(1);
	if (player) {
		Vector pos = server->GetAbsOrigin(player);
		g_stitcher.x = pos.x;
		g_stitcher.y = pos.y;
		g_stitcher.z = pos.z + 100;
	}

	// Remove all the regions
	g_stitch.regions.clear();
}

CON_COMMAND(sar_stitcher_reset, "sar_stitcher_reset - reset the stitcher.\n") {
	resetStitcher();
}

ON_EVENT(SESSION_START) {
	resetStitcher();
}

static void drawSurroundingBox(int x0, int y0, int x1, int y1, Color t, Color r, Color b, Color l) {
	surface->DrawColoredLine(x0 - 1, y0 - 1, x1,     y0 - 1, t);
	surface->DrawColoredLine(x1,     y0 - 1, x1,     y1,     r);
	surface->DrawColoredLine(x1,     y1,     x0 - 1, y1,     b);
	surface->DrawColoredLine(x0 - 1, y1,     x0 - 1, y0 - 1, l);
}

static Color pickColor() {
	const Color colors[] = {
		{ 255, 0, 0 },
		{ 0, 255, 0 },
		{ 0, 0, 255 },
		{ 255, 255, 0 },
		{ 255, 0, 255 },
		{ 0, 255, 255 },
		{ 255, 100, 100 },
		{ 100, 255, 100 },
		{ 100, 100, 255 },
		{ 255, 255, 100 },
		{ 255, 100, 255 },
		{ 100, 255, 255 },
	};

	for (Color c : colors) {
		bool found = false;
		for (auto &r : g_stitch.regions) {
			Color m = r.mask_color;
			if (m.r() == c.r() && m.g() == c.g() && m.b() == c.b()) {
				found = true;
				break;
			}
		}
		if (!found) return c;
	}

	return {
		rand() % 255,
		rand() % 255,
		rand() % 255,
	};
}

static bool isStitching() {
	return sar_stitcher.GetBool() && sv_cheats.GetBool();
}

static void initStitch(bool stitching) {
	static bool was_stitching = false;

	static bool portalsopenall;
	static bool novis;
	static bool drawviewmodel;
	static bool glclear;
	static bool crosshair;
	static bool specular;
	static bool fogoverride;
	static bool fogenable;
	static bool disablebloom;
	static float tonemaprate;

	if (stitching && !was_stitching) {
		// Store the cvars
		portalsopenall = Variable("r_portalsopenall").GetBool();
		novis = Variable("r_novis").GetBool();
		drawviewmodel = Variable("r_drawviewmodel").GetBool();
		glclear = Variable("gl_clear").GetBool();
		crosshair = Variable("crosshair").GetBool();
		specular = Variable("mat_fastspecular").GetBool();
		fogoverride = Variable("fog_override").GetBool();
		fogenable = Variable("fog_enable").GetBool();
		disablebloom = Variable("mat_disable_bloom").GetBool();
		tonemaprate = Variable("mat_hdr_manual_tonemap_rate").GetFloat();

		// Set them to our values
		Variable("r_portalsopenall").SetValue(true);
		Variable("r_novis").SetValue(true);
		Variable("r_drawviewmodel").SetValue(false);
		Variable("gl_clear").SetValue(true);
		Variable("crosshair").SetValue(false);
		Variable("mat_fastspecular").SetValue(false);
		Variable("fog_override").SetValue(true);
		Variable("fog_enable").SetValue(false);
		Variable("mat_disable_bloom").SetValue(true);
		Variable("mat_hdr_manual_tonemap_rate").SetValue(-1.0f);

		// Set our initial coords to just above the player position
		g_stitcher.x = g_stitcher.y = g_stitcher.z = 0;
		void *player = server->GetPlayer(1);
		if (player) {
			Vector pos = server->GetAbsOrigin(player);
			g_stitcher.x = pos.x;
			g_stitcher.y = pos.y;
			g_stitcher.z = pos.z + 100;
		}

		// Set initial mouse position
		int width, height;
		engine->GetScreenSize(nullptr, width, height);
		g_stitcher.mousex = width / 2;
		g_stitcher.mousey = height / 2;
	} else if (!stitching && was_stitching) {
		// Restore the cvar values
		Variable("r_portalsopenall").SetValue(portalsopenall);
		Variable("r_novis").SetValue(novis);
		Variable("r_drawviewmodel").SetValue(drawviewmodel);
		Variable("gl_clear").SetValue(glclear);
		Variable("crosshair").SetValue(crosshair);
		Variable("mat_fastspecular").SetValue(specular);
		Variable("fog_override").SetValue(fogoverride);
		Variable("fog_enable").SetValue(fogenable);
		Variable("mat_disable_bloom").SetValue(disablebloom);
		Variable("mat_hdr_manual_tonemap_rate").SetValue(tonemaprate);
	}

	was_stitching = stitching;
}

ON_EVENT(SAR_UNLOAD) { initStitch(false); }

static void doMovement(float delta) {
	int vx = 0, vy = 0, vz = 0;

	if (inputSystem->IsKeyDown(ButtonCode_t::KEY_W)) vy += 1;
	if (inputSystem->IsKeyDown(ButtonCode_t::KEY_S)) vy -= 1;
	if (inputSystem->IsKeyDown(ButtonCode_t::KEY_D)) vx += 1;
	if (inputSystem->IsKeyDown(ButtonCode_t::KEY_A)) vx -= 1;

	if (inputSystem->IsKeyDown(ButtonCode_t::KEY_SPACE)) vz += 1;
	if (inputSystem->IsKeyDown(ButtonCode_t::KEY_LSHIFT)) vz -= 1;

	if (g_stitcher.stitch_view) {
		const int speed = 200; // pixels per second
		const float zspeed = 1; // scale(?) per second
		g_stitcher.stitch_view_x += vx * speed * delta;
		g_stitcher.stitch_view_y += vy * speed * delta;
		g_stitcher.stitch_view_scale *= 1.0 + (float)-vz * zspeed * delta;
	} else if (!g_stitcher.select_done) {
		const int speed = 300; // ups
		const int zspeed = 100; // ups

		g_stitcher.x_part += vx * speed * delta;
		g_stitcher.y_part += vy * speed * delta;
		g_stitcher.z_part += vz * zspeed * delta;

		g_stitcher.x += (int)g_stitcher.x_part;
		g_stitcher.y += (int)g_stitcher.y_part;
		g_stitcher.z += (int)g_stitcher.z_part;
		g_stitcher.x_part -= (int)g_stitcher.x_part;
		g_stitcher.y_part -= (int)g_stitcher.y_part;
		g_stitcher.z_part -= (int)g_stitcher.z_part;
	}
}

static void submitRegion() {
	int width, height;
	engine->GetScreenSize(nullptr, width, height);

	int minx = std::min(g_stitcher.select_x0, g_stitcher.select_x1);
	int maxx = std::max(g_stitcher.select_x0, g_stitcher.select_x1);
	int miny = std::min(g_stitcher.select_y0, g_stitcher.select_y1);
	int maxy = std::max(g_stitcher.select_y0, g_stitcher.select_y1);

	Color mask_color = pickColor();

	int w = maxx - minx;
	int h = maxy - miny;

	std::vector<uint8_t> buf(w * h * 4);
	Memory::VMT<void(__rescall *)(void *, int, int, int, int, void *, ImageFormat)>(*g_videomode, Offsets::ReadScreenPixels)(*g_videomode, minx, miny, w, h, buf.data(), IMAGE_FORMAT_RGBA8888);

	for (int i = 0; i < w*h; ++i) {
		// Most transparent stuff is OOB space, but unfortunately goo also
		// seems to be transparent. That said, we can replace white OOB with
		// black OOB at least!
		// Also, the game gets the transparency backwards, so we have to fix
		// that.
		if (buf[4*i] == 255 && buf[4*i+1] == 255 && buf[4*i+2] == 255 && buf[4*i+3] != 0) {
			buf[4*i+0] = 0;
			buf[4*i+1] = 0;
			buf[4*i+2] = 0;
		}
		buf[4*i+3] = 255;
	}

	int texture_id = surface->CreateNewTextureID(surface->matsurface->ThisPtr(), true);
	surface->DrawSetTextureRGBA(surface->matsurface->ThisPtr(), texture_id, buf.data(), w, h);

	StitchRegion r;
	r.xmin = g_stitcher.x - width/2 + minx;
	r.xmax = g_stitcher.x - width/2 + maxx;
	r.ymin = g_stitcher.y - height/2 + (height - maxy);
	r.ymax = g_stitcher.y - height/2 + (height - miny);
	r.z = g_stitcher.z;
	r.mask_color = mask_color;
	r.data = std::move(buf);
	r.texture_id = texture_id;

	if (g_stitch.regions.size() == 0) {
		g_stitch.xmin = r.xmin;
		g_stitch.xmax = r.xmax;
		g_stitch.ymin = r.ymin;
		g_stitch.ymax = r.ymax;
	} else {
		g_stitch.xmin = std::min(g_stitch.xmin, r.xmin);
		g_stitch.xmax = std::max(g_stitch.xmax, r.xmax);
		g_stitch.ymin = std::min(g_stitch.ymin, r.ymin);
		g_stitch.ymax = std::max(g_stitch.ymax, r.ymax);
	}

	g_stitch.regions.push_back(r);
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
		w & 0xFF, w >> 8, // Width
		h & 0xFF, h >> 8, // Height
		32, // Bits per pixel
		0, // Image descriptor
	};
	fwrite(header, 1, sizeof header, f);
	fwrite(data, 1, w * h * 4, f);
	fclose(f);
}

static void exportRegions() {
	uint16_t w = g_stitch.xmax - g_stitch.xmin;
	uint16_t h = g_stitch.ymax - g_stitch.ymin;

	long org_x = g_stitch.xmin;
	long org_y = g_stitch.ymin;

	std::vector<uint8_t> image(w * h * 4, 0); // Initially all zeroed
	std::vector<uint8_t> mask(w * h * 4, 0); // Initially all zeroed

	for (auto &region : g_stitch.regions) {
		long rw = region.xmax - region.xmin;
		long rh = region.ymax - region.ymin;
		for (long wy = region.ymin; wy < region.ymax; ++wy) {
			long ry = wy - region.ymin;
			for (long wx = region.xmin; wx < region.xmax; ++wx) {
				long rx = wx - region.xmin;

				size_t i = (wy - org_y) * w + (wx - org_x);
				size_t ri = (rh - ry - 1) * rw + rx;

				image[i*4 + 0] = region.data[ri*4 + 2]; // B
				image[i*4 + 1] = region.data[ri*4 + 1]; // G
				image[i*4 + 2] = region.data[ri*4 + 0]; // R
				image[i*4 + 3] = region.data[ri*4 + 3]; // A

				mask[i*4 + 0] = region.mask_color.b();
				mask[i*4 + 1] = region.mask_color.g();
				mask[i*4 + 2] = region.mask_color.r();
				mask[i*4 + 3] = region.mask_color.a();
			}
		}
	}

#ifdef _WIN32
	_mkdir("stitch_export");
#else
	mkdir("stitch_export", 0777);
#endif

	writeTga("stitch_export/image.tga", image.data(), w, h);
	writeTga("stitch_export/mask.tga", mask.data(), w, h);

	FILE *f = fopen("stitch_export/meta.json", "wb");
	if (f) {
		fputs("{\n", f);
		fprintf(f, "\t\"center_x\": %.1f,\n", (float)org_x + (float)w / 2);
		fprintf(f, "\t\"center_y\": %.1f,\n", (float)org_y + (float)h / 2);
		fputs("\t\"regions\": [\n", f);
		for (size_t i = 0; i < g_stitch.regions.size(); ++i) {
			auto &region = g_stitch.regions[i];
			float x = (float)(region.xmin + region.xmax) / 2;
			float y = (float)(region.ymin + region.ymax) / 2;
			long z = region.z;
			long w = region.xmax - region.xmin;
			long h = region.ymax - region.ymin;
			bool last = i == g_stitch.regions.size() - 1;
			Color col = region.mask_color;
			std::string mask = Utils::ssprintf("%02X%02X%02X", col.r(), col.g(), col.b());
			fprintf(f, "\t\t{ \"mask\": \"%s\", \"x0\": %ld, \"y0\": %ld, \"x1\": %ld, \"y1\": %ld, \"cam_z\": %ld }%s\n", mask.c_str(), region.xmin, region.ymin, region.xmax, region.ymax, z, last ? "" : ",");
		}
		fputs("\t]\n", f);
		fputs("}\n", f);
		fclose(f);
	}
}

static void updateStitchUi() {
	static bool was_m_down = false;
	bool m_down = inputSystem->IsKeyDown(ButtonCode_t::KEY_M);
	if (m_down && !was_m_down) {
		g_stitcher.stitch_view_mask = !g_stitcher.stitch_view_mask;
	}
	was_m_down = m_down;

	static bool was_q_down = false;
	bool q_down = inputSystem->IsKeyDown(ButtonCode_t::KEY_Q);
	if (q_down && !was_q_down) {
		g_stitcher.stitch_view_selected = SIZE_MAX;
	}
	was_q_down = q_down;

	static bool was_del_down = false;
	bool del_down = inputSystem->IsKeyDown(ButtonCode_t::KEY_DELETE);
	if (del_down && !was_del_down && g_stitcher.stitch_view_selected != SIZE_MAX) {
		g_stitch.regions.erase(g_stitch.regions.begin() + g_stitcher.stitch_view_selected);
		if (g_stitch.regions.size() == 0) {
			g_stitcher.stitch_view_selected = SIZE_MAX;
		} else if (g_stitcher.stitch_view_selected >= g_stitch.regions.size()) {
			--g_stitcher.stitch_view_selected;
		}
	}
	was_del_down = del_down;

	static bool had_up = false;
	static bool had_down = false;
	static bool had_left = false;
	static bool had_right = false;

	bool has_up = inputSystem->IsKeyDown(ButtonCode_t::KEY_UP);
	bool has_down = inputSystem->IsKeyDown(ButtonCode_t::KEY_DOWN);
	bool has_left = inputSystem->IsKeyDown(ButtonCode_t::KEY_LEFT);
	bool has_right = inputSystem->IsKeyDown(ButtonCode_t::KEY_RIGHT);

	if (has_right && !had_right) {
		if (g_stitcher.stitch_view_selected == SIZE_MAX) {
			g_stitcher.stitch_view_selected = g_stitch.regions.size() - 1;
		} else if (g_stitcher.stitch_view_selected == 0) {
				g_stitcher.stitch_view_selected = g_stitch.regions.size() - 1;
		} else {
			--g_stitcher.stitch_view_selected;
		}
	}

	if (has_left && !had_left) {
		if (g_stitcher.stitch_view_selected == SIZE_MAX) {
			g_stitcher.stitch_view_selected = 0;
		} else {
			++g_stitcher.stitch_view_selected;
			g_stitcher.stitch_view_selected %= g_stitch.regions.size();
		}
	}

	if (has_up && !had_up) { // move selected to earlier in list
		size_t sel = g_stitcher.stitch_view_selected;
		if (sel != SIZE_MAX) {
			size_t dst = sel == 0 ? g_stitch.regions.size() - 1 : sel - 1;
			auto tmp = g_stitch.regions[sel];
			g_stitch.regions[sel] = g_stitch.regions[dst];
			g_stitch.regions[dst] = tmp;
			g_stitcher.stitch_view_selected = dst;
		}
	}

	if (has_down && !had_down) { // move selected to later in list
		size_t sel = g_stitcher.stitch_view_selected;
		if (sel != SIZE_MAX) {
			size_t dst = (sel + 1) % g_stitch.regions.size();
			auto tmp = g_stitch.regions[sel];
			g_stitch.regions[sel] = g_stitch.regions[dst];
			g_stitch.regions[dst] = tmp;
			g_stitcher.stitch_view_selected = dst;
		}
	}

	had_up = has_up;
	had_down = has_down;
	had_left = has_left;
	had_right = has_right;

	static bool was_e_down = false;
	bool is_e_down = inputSystem->IsKeyDown(ButtonCode_t::KEY_E);
	if (is_e_down && !was_e_down) {
		if (g_stitch.regions.size() > 0) {
			exportRegions();
		}
	}
	was_e_down = is_e_down;
}

static void updateUi() {
	if (engine->IsGamePaused()) return;

	static bool was_tab_down = false;
	bool tab_down = inputSystem->IsKeyDown(ButtonCode_t::KEY_TAB);
	if (tab_down && !was_tab_down) {
		g_stitcher.stitch_view = !g_stitcher.stitch_view;
		if (g_stitcher.stitch_view) {
			int width, height;
			engine->GetScreenSize(nullptr, width, height);

			g_stitcher.stitch_view_x = (g_stitch.xmin + g_stitch.xmax) / 2;
			g_stitcher.stitch_view_y = (g_stitch.ymin + g_stitch.ymax) / 2;

			float xscale = (float)width / (g_stitch.xmax - g_stitch.xmin + 100);
			float yscale = (float)height / (g_stitch.ymax - g_stitch.ymin + 100);

			g_stitcher.stitch_view_scale = xscale < yscale ? xscale : yscale;

			g_stitcher.stitch_view_mask = false;
			g_stitcher.stitch_view_selected = SIZE_MAX;
		}
	}
	was_tab_down = tab_down;

	if (g_stitcher.stitch_view) {
		updateStitchUi();
		return;
	}

	if (inputSystem->IsKeyDown(ButtonCode_t::KEY_Q)) {
		g_stitcher.select_done = false;
	}

	bool selecting = inputSystem->IsKeyDown(ButtonCode_t::MOUSE_LEFT);
	if (selecting && !g_stitcher.select_active) {
		g_stitcher.select_active = true;
		g_stitcher.select_done = false;
		g_stitcher.select_x0 = g_stitcher.mousex;
		g_stitcher.select_y0 = g_stitcher.mousey;
	} else if (!selecting && g_stitcher.select_active) {
		g_stitcher.select_active = false;
		g_stitcher.select_done = true;
	}

	if (inputSystem->IsKeyDown(ButtonCode_t::KEY_T)) {
		Variable("mat_hdr_manual_tonemap_rate").SetValue(0.0f);
	} else {
		Variable("mat_hdr_manual_tonemap_rate").SetValue(-1.0f);
	}

	if (g_stitcher.select_active) {
		g_stitcher.select_x1 = g_stitcher.mousex;
		g_stitcher.select_y1 = g_stitcher.mousey;
	}
	
	if (g_stitcher.select_done) {
		if (inputSystem->IsKeyDown(ButtonCode_t::KEY_W)) g_stitcher.select_edge = 0;
		if (inputSystem->IsKeyDown(ButtonCode_t::KEY_D)) g_stitcher.select_edge = 1;
		if (inputSystem->IsKeyDown(ButtonCode_t::KEY_S)) g_stitcher.select_edge = 2;
		if (inputSystem->IsKeyDown(ButtonCode_t::KEY_A)) g_stitcher.select_edge = 3;

		static bool had_up = false;
		static bool had_down = false;
		static bool had_left = false;
		static bool had_right = false;

		bool has_up = inputSystem->IsKeyDown(ButtonCode_t::KEY_UP);
		bool has_down = inputSystem->IsKeyDown(ButtonCode_t::KEY_DOWN);
		bool has_left = inputSystem->IsKeyDown(ButtonCode_t::KEY_LEFT);
		bool has_right = inputSystem->IsKeyDown(ButtonCode_t::KEY_RIGHT);

		bool hold = inputSystem->IsKeyDown(ButtonCode_t::KEY_LCONTROL);

		int dy = (has_down && (hold || !had_down)) - (has_up && (hold || !had_up));
		int dx = (has_right && (hold || !had_right)) - (has_left && (hold || !had_left));

		int minx = std::min(g_stitcher.select_x0, g_stitcher.select_x1);
		int maxx = std::max(g_stitcher.select_x0, g_stitcher.select_x1);
		int miny = std::min(g_stitcher.select_y0, g_stitcher.select_y1);
		int maxy = std::max(g_stitcher.select_y0, g_stitcher.select_y1);

		switch (g_stitcher.select_edge) {
		case 0: // top
			miny += dy;
			break;
		case 1: // right
			maxx += dx;
			break;
		case 2: // bottom
			maxy += dy;
			break;
		case 3: // left
			minx += dx;
			break;
		default: break;
		}

		int width, height;
		engine->GetScreenSize(nullptr, width, height);

		if (minx < 0) minx = 0;
		if (maxx > width) maxx = width;
		if (miny < 0) miny = 0;
		if (maxy > height) maxy = height;
		
		g_stitcher.select_x0 = minx;
		g_stitcher.select_x1 = maxx;
		g_stitcher.select_y0 = miny;
		g_stitcher.select_y1 = maxy;

		had_up = has_up;
		had_down = has_down;
		had_left = has_left;
		had_right = has_right;

		static bool had_enter = false;
		bool has_enter = inputSystem->IsKeyDown(ButtonCode_t::KEY_ENTER);

		if (has_enter && !had_enter) {
			submitRegion();
			g_stitcher.select_done = false;
		}

		had_enter = has_enter;
	}
}

void Stitcher::OverrideView(CViewSetup *view) {
	initStitch(isStitching());
	if (!isStitching()) return;

	{
		static float last_cl_time = 0;
		float cl_time = engine->GetClientTime();
		float delta = cl_time - last_cl_time;
		if (delta < 0) {
			delta = 0;
			last_cl_time = cl_time;
		} else if (delta >= 1.0f / 60.0f) {
			doMovement(delta);
			last_cl_time = cl_time;
		}
	}

	updateUi();

	int width, height;
	engine->GetScreenSize(nullptr, width, height);

	view->origin = Vector{
		(float)g_stitcher.x,
		(float)g_stitcher.y,
		(float)g_stitcher.z,
	};
	
	view->angles = QAngle{90, 90, 0};

	view->m_nMotionBlurMode = 1; // disable

	view->m_bOrtho = true;
	view->m_OrthoLeft = -width/2;
	view->m_OrthoRight = width/2;
	view->m_OrthoTop = -height/2;
	view->m_OrthoBottom = height/2;

	view->zNear = 1;
}

void Stitcher::OverrideMovement(CUserCmd *cmd) {
	if (!isStitching()) return;

	cmd->buttons = 0;
	cmd->forwardmove = 0;
	cmd->sidemove = 0;
	cmd->upmove = 0;

	g_stitcher.mousex += cmd->mousedx;
	g_stitcher.mousey += cmd->mousedy;

	int width, height;
	engine->GetScreenSize(nullptr, width, height);

	if (g_stitcher.mousex < 0) g_stitcher.mousex = 0;
	if (g_stitcher.mousex > width) g_stitcher.mousex = width;
	if (g_stitcher.mousey < 0) g_stitcher.mousey = 0;
	if (g_stitcher.mousey > height) g_stitcher.mousey = height;
}

static void paintStitchView() {
	int width, height;
	engine->GetScreenSize(nullptr, width, height);

	surface->DrawRect({ 200, 200, 200, 255 }, 0, 0, width, height);

	int cx = g_stitcher.stitch_view_x;
	int cy = g_stitcher.stitch_view_y;
	float scale = g_stitcher.stitch_view_scale;

	for (size_t i = 0; i < g_stitch.regions.size(); ++i) {
		auto &region = g_stitch.regions[i];

		int x0 = (region.xmin - cx) * scale + width/2;
		int x1 = (region.xmax - cx) * scale + width/2;
		int y0 = height - ((region.ymax - cy) * scale + height/2);
		int y1 = height - ((region.ymin - cy) * scale + height/2);

		bool selected = g_stitcher.stitch_view_selected == i;

		if (x0 > width || x1 < 0) continue;
		if (y0 > height || y1 < 0) continue;

		surface->DrawSetColor(surface->matsurface->ThisPtr(), 255, 255, 255, 255);
		surface->DrawSetTexture(surface->matsurface->ThisPtr(), region.texture_id);
		surface->DrawTexturedRect(surface->matsurface->ThisPtr(), x0, y0, x1, y1);

		if (g_stitcher.stitch_view_mask) {
			Color mask = region.mask_color;
			mask._color[3] = selected ? 30 : 10;
			surface->DrawRect(mask, x0, y0, x1, y1);
		}
		
		if (selected) {
			Color w{ 255, 255, 255, 255 };
			drawSurroundingBox(x0, y0, x1, y1, w, w, w, w);
		}
	}

	// Draw a semi-transparent line around the selected region, in case
	// it's obscured
	if (g_stitcher.stitch_view_selected != SIZE_MAX) {
		auto &region = g_stitch.regions[g_stitcher.stitch_view_selected];

		int x0 = (region.xmin - cx) * scale + width/2;
		int x1 = (region.xmax - cx) * scale + width/2;
		int y0 = height - ((region.ymax - cy) * scale + height/2);
		int y1 = height - ((region.ymin - cy) * scale + height/2);

		Color w{ 255, 255, 255, 100 };
		drawSurroundingBox(x0, y0, x1, y1, w, w, w, w);
	}

	// Draw the regions on the right
	{
		int y = height - 10;
		for (size_t i = 0; i < g_stitch.regions.size(); ++i) {
			auto &region = g_stitch.regions[i];

			int x0 = 10, x1 = 100, y0 = y - 40, y1 = y;

			if (g_stitcher.stitch_view_selected == i) {
				x0 -= 5;
				x1 += 5;
				y0 -= 10;
			}

			surface->DrawRect(region.mask_color, x0, y0, x1, y1);

			y = y0;
		}
	}

	{
		long font = scheme->GetDefaultFont() + 1;
		int len = surface->GetFontLength(font, "Viewing stitch with %d regions", g_stitch.regions.size());
		surface->DrawTxt(font, (width - len) / 2, 50, { 255, 255, 255, 255 }, "Viewing stitch with %d regions", g_stitch.regions.size());
	}
}

bool Stitcher::Paint() {
	if (!isStitching()) return false;

	if (engine->IsGamePaused()) return true;

	if (g_stitcher.stitch_view) {
		paintStitchView();
		return true;
	}

	if (g_stitcher.select_active || g_stitcher.select_done) {
		int width, height;
		engine->GetScreenSize(nullptr, width, height);

		int minx = std::min(g_stitcher.select_x0, g_stitcher.select_x1);
		int maxx = std::max(g_stitcher.select_x0, g_stitcher.select_x1);
		int miny = std::min(g_stitcher.select_y0, g_stitcher.select_y1);
		int maxy = std::max(g_stitcher.select_y0, g_stitcher.select_y1);

		surface->DrawRect({ 0, 0, 0, 192 }, 0, 0, minx, height);
		surface->DrawRect({ 0, 0, 0, 192 }, maxx, 0, width, height);
		surface->DrawRect({ 0, 0, 0, 192 }, minx, 0, maxx, miny);
		surface->DrawRect({ 0, 0, 0, 192 }, minx, maxy, maxx, height);

		Color sel = { 255, 100, 100, 255 };
		Color def = { 255, 255, 255, 255 };

		Color top = g_stitcher.select_done && g_stitcher.select_edge == 0 ? sel : def;
		Color right = g_stitcher.select_done && g_stitcher.select_edge == 1 ? sel : def;
		Color bottom = g_stitcher.select_done && g_stitcher.select_edge == 2 ? sel : def;
		Color left = g_stitcher.select_done && g_stitcher.select_edge == 3 ? sel : def;

		drawSurroundingBox(minx, miny, maxx, maxy, top, right, bottom, left);
	}

	// Mouse cursor

	if (!g_stitcher.select_done) {
		for (int dy = 0; dy < 20; ++dy) {
			int x = g_stitcher.mousex;
			int y = g_stitcher.mousey;
			surface->DrawColoredLine(x, y + dy, x + dy, y + dy, { 255, 255, 255, 255 });
		}

		for (int dy = 0; dy < 10; ++dy) {
			int x = g_stitcher.mousex;
			int y = g_stitcher.mousey + 20;
			surface->DrawColoredLine(x, y + dy, x + 20 - 2*dy, y + dy, { 255, 255, 255, 255 });
		}

		surface->DrawColoredLine(g_stitcher.mousex, g_stitcher.mousey, g_stitcher.mousex, g_stitcher.mousey + 30, { 0, 0, 0, 255 });
		surface->DrawColoredLine(g_stitcher.mousex, g_stitcher.mousey, g_stitcher.mousex + 20, g_stitcher.mousey + 20, { 0, 0, 0, 255 });
		surface->DrawColoredLine(g_stitcher.mousex, g_stitcher.mousey + 30, g_stitcher.mousex + 20, g_stitcher.mousey + 20, { 0, 0, 0, 255 });
	}

	return true;
}
