#include "OverlayRender.hpp"
#include "Modules/Engine.hpp"
#include "Modules/MaterialSystem.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Scheme.hpp"
#include "Event.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Session.hpp"
#include "Features/Timer/PauseTimer.hpp"

#include <map>

// The address of this variable is used as a placeholder to be detected
// by createMeshInternal and friends. We use g_group_idx to keep track
// of which mesh (overlay group) we're actually rendering.
static int g_placeholder;

struct OverlayText {
	Vector pos;
	bool center;
	int xOff;
	int yOff;
	std::string text;
	Color col;
	unsigned long font;
};

static std::vector<OverlayText> g_text;

HUD_ELEMENT2_NO_DISABLE(overlay_text, HudType_InGame | HudType_Menu | HudType_Paused | HudType_LoadingScreen) {
	if (session->isRunning && !pauseTimer->IsActive()) {
		for (auto &t : g_text) {
			Vector scr_pos;
			engine->PointToScreen(t.pos, scr_pos);
			scr_pos.x += t.xOff;
			scr_pos.y += t.yOff;
			if (t.center) {
				scr_pos.x -= surface->GetFontLength(t.font, "%s", t.text.c_str()) / 2;
			}
			surface->DrawTxt(t.font, scr_pos.x, scr_pos.y, t.col, "%s", t.text.c_str());
		}
	}
	g_text.clear();
}

struct OverlayGroup {
	Color col;
	bool wireframe;
	bool noz;
	bool line;
	std::vector<Vector> verts;
	bool was_used;
};

static std::vector<OverlayGroup> g_groups;
static size_t g_group_idx;

static size_t g_last_group = SIZE_MAX;
static std::vector<Vector> &getGroupVertVector(Color col, bool wireframe, bool line, bool noz = false) {
	static Color last_col;
	static bool last_wireframe;
	static bool last_line;
	static bool last_noz;

	if (g_last_group == SIZE_MAX || last_col != col || last_wireframe != wireframe || last_line != line || last_noz != noz) {
		last_col = col;
		last_wireframe = wireframe;
		last_line = line;
		last_noz = noz;

		g_last_group = SIZE_MAX;

		for (size_t i = 0; i < g_groups.size(); ++i) {
			auto &g = g_groups[i];
			if (g.col != col) continue;
			if (g.wireframe != wireframe) continue;
			if (g.noz != noz) continue;
			if (g.line != line) continue;
			if (g.verts.size() > 1000) continue; // Stupid big meshes is probably a bad idea

			g.was_used = true;
			g_last_group = i;
			break;
		}

		if (g_last_group == SIZE_MAX) {
			g_groups.push_back({
				col,
				wireframe,
				noz,
				line,
				{},
				true,
			});
			g_last_group = g_groups.size() - 1;
		}
	}

	return g_groups[g_last_group].verts;
}

bool OverlayRender::createMeshInternal(void *collision, Vector **vertsOut, size_t *nvertsOut) {
	if (collision != &g_placeholder) return false;
	*vertsOut = g_groups[g_group_idx].verts.data();
	*nvertsOut = g_groups[g_group_idx].verts.size();
	return true;
}

bool OverlayRender::destroyMeshInternal(Vector *verts, size_t nverts) {
	if (g_group_idx >= g_groups.size()) return false;
	return g_groups[g_group_idx].verts.data() == verts;
}

// Dispatched just before RENDER
ON_EVENT(FRAME) {
	for (size_t i = 0; i < g_groups.size(); ++i) {
		auto &g = g_groups[i];
		if (g.was_used) {
			g.verts.clear();
			g.was_used = false;
		} else {
			if (i + 1 < g_groups.size()) std::iter_swap(g_groups.begin() + i, g_groups.end() - 1);
			g_groups.pop_back();
			--i;
		}
	}
	g_last_group = SIZE_MAX;
}

void OverlayRender::addTriangle(Vector a, Vector b, Vector c, Color col, bool cullBack) {
	auto &vs = getGroupVertVector(col, false, false);
	vs.insert(vs.end(), { a, b, c });
	if (!cullBack) vs.insert(vs.end(), { a, c, b });
}

void OverlayRender::addQuad(Vector a, Vector b, Vector c, Vector d, Color col, bool cullBack) {
	OverlayRender::addTriangle(a, b, c, col, cullBack);
	OverlayRender::addTriangle(a, c, d, col, cullBack);
}

void OverlayRender::addLine(Vector a, Vector b, Color col, bool throughWalls) {
	auto &vs = getGroupVertVector(col, true, true, throughWalls);
	vs.insert(vs.end(), { a, b });
}

void OverlayRender::addBox(Vector origin, Vector mins, Vector maxs, QAngle ang, Color col, bool wireframe, bool wireframeThroughWalls) {
	float spitch, cpitch;
	Math::SinCos(DEG2RAD(ang.x), &spitch, &cpitch);
	float syaw, cyaw;
	Math::SinCos(DEG2RAD(ang.y), &syaw, &cyaw);
	float sroll, croll;
	Math::SinCos(DEG2RAD(ang.z), &sroll, &croll);

	Matrix rot{3, 3, 0};
	rot(0, 0) = cyaw * cpitch;
	rot(0, 1) = cyaw * spitch * sroll - syaw * croll;
	rot(0, 2) = cyaw * spitch * croll + syaw * sroll;
	rot(1, 0) = syaw * cpitch;
	rot(1, 1) = syaw * spitch * sroll + cyaw * croll;
	rot(1, 2) = syaw * spitch * croll - cyaw * sroll;
	rot(2, 0) = -spitch;
	rot(2, 1) = cpitch * sroll;
	rot(2, 2) = cpitch * croll;

	Vector verts[8];
	for (int i = 0; i < 8; ++i) {
		Vector v;
		v.x = (i & 1) ? maxs[0] : mins[0];
		v.y = (i & 2) ? maxs[1] : mins[1];
		v.z = (i & 4) ? maxs[2] : mins[2];
		verts[i] = origin + rot * v;
	}

	for (auto i : std::array<std::array<int, 4>, 6>{
		std::array<int, 4>{ 2, 6, 4, 0 },
		std::array<int, 4>{ 7, 3, 1, 5 },
		std::array<int, 4>{ 4, 5, 1, 0 },
		std::array<int, 4>{ 3, 7, 6, 2 },
		std::array<int, 4>{ 1, 3, 2, 0 },
		std::array<int, 4>{ 6, 7, 5, 4 },
	}) {
		OverlayRender::addQuad(verts[i[0]], verts[i[1]], verts[i[2]], verts[i[3]], col, true);
	}

	if (wireframe) {
		Color wf_col = col;
		wf_col._color[3] = 255;
		for (auto i : std::array<std::array<int, 2>, 12>{
			std::array<int, 2>{ 0, 1 },
			std::array<int, 2>{ 0, 2 },
			std::array<int, 2>{ 0, 4 },
			std::array<int, 2>{ 1, 3 },
			std::array<int, 2>{ 1, 5 },
			std::array<int, 2>{ 2, 6 },
			std::array<int, 2>{ 2, 3 },
			std::array<int, 2>{ 3, 7 },
			std::array<int, 2>{ 4, 5 },
			std::array<int, 2>{ 4, 6 },
			std::array<int, 2>{ 5, 7 },
			std::array<int, 2>{ 6, 7 },
		}) {
			OverlayRender::addLine(verts[i[0]], verts[i[1]], wf_col, wireframeThroughWalls);
		}
	}
}

void OverlayRender::addText(Vector pos, int xOff, int yOff, const std::string &text, unsigned long font, Color col, bool center) {
	if (font == FONT_DEFAULT) font = scheme->GetDefaultFont();
	g_text.push_back({pos, center, xOff, yOff, text, col, font});
}

static void setPrimitiveType(uint8_t type) {
#ifdef _WIN32
	// The method is a call (well, actually a jump) straight to a plain
	// function. Get the actual function
	uintptr_t fn = Memory::Read((uintptr_t)engine->DebugDrawPhysCollide + 22);

	// We want to overwrite a couple bytes of this function
	Memory::UnProtect((char *)fn + 196, 14);
	*(uint32_t *)(fn + 196) = type;
	*(uint8_t *)(fn + 209) = type;
#else
	if (sar.game->Is(SourceGame_EIPRelPIC)) {
		// The method is a call (well, actually a jump) straight to a plain
		// function. Get the actual function
		uintptr_t fn = Memory::Read((uintptr_t)engine->DebugDrawPhysCollide + 43);

		// We want to overwrite a couple bytes of this function
		Memory::UnProtect((char *)fn + 371, 14);
		*(uint32_t *)(fn + 371) = type;
		*(uint8_t *)(fn + 384) = type;
	} else {
		// The method is a call (well, actually a jump) straight to a plain
		// function. Get the actual function
		uintptr_t fn = Memory::Read((uintptr_t)engine->DebugDrawPhysCollide + 38);

		// We want to overwrite a couple bytes of this function
		Memory::UnProtect((char *)fn + 355, 20);
		*(uint32_t *)(fn + 355) = type;
		*(uint32_t *)(fn + 371) = type;
	}
#endif
}

void OverlayRender::drawMeshes() {
	IMaterial *mat_solid = materialSystem->FindMaterial("debug/debugtranslucentvertexcolor", "Other textures");
	IMaterial *mat_wireframe = materialSystem->FindMaterial("debug/debugwireframevertexcolor", "Other textures");
	IMaterial *mat_wireframe_noz = materialSystem->FindMaterial("debug/debugwireframevertexcolorignorez", "Other textures");

	matrix3x4_t transform{0};
	transform.m_flMatVal[0][0] = 1;
	transform.m_flMatVal[1][1] = 1;
	transform.m_flMatVal[2][2] = 1;

	for (size_t i = 0; i < g_groups.size(); ++i) {
		g_group_idx = i;
		auto &g = g_groups[i];

		if (g.line) {
			setPrimitiveType(1);
			// There need to be some multiple of 3 verts for this to work
			// properly. Push some garbage lines until we're matching
			while (g.verts.size() % 6) { // lcm(2,3)
				g.verts.push_back({0,0,0});
			}
		} else {
			setPrimitiveType(2);
		}

		IMaterial *mat = g.wireframe ? (g.noz ? mat_wireframe_noz : mat_wireframe) : mat_solid;
		engine->DebugDrawPhysCollide(engine->engineClient->ThisPtr(), &g_placeholder, mat, transform, g.col);
	}

	// Make sure we're back to normal
	setPrimitiveType(2);
}
