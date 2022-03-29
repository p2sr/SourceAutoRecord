#include "OverlayRender.hpp"
#include "Modules/Engine.hpp"
#include "Modules/MaterialSystem.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Scheme.hpp"
#include "Event.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Session.hpp"
#include "Features/Timer/PauseTimer.hpp"

#include <set>

///////////////////////////////////


RenderCallback RenderCallback::none = {
	[](CViewSetup vs, Color &col_out, bool &nodepth_out) {
		col_out = Color{0,0,0,0};
		nodepth_out = false;
	},
};

RenderCallback RenderCallback::constant(Color col, bool nodepth) {
	return {
		[=](CViewSetup vs, Color &col_out, bool &nodepth_out) {
			col_out = col;
		 	nodepth_out = nodepth;
		},
	};
}

RenderCallback RenderCallback::prox_fade(float min, float max, Color target, Vector point, RenderCallback base) {
	return {
		[=](CViewSetup vs, Color &col_out, bool &nodepth_out) {
			base.cbk(vs, col_out, nodepth_out);
			float dist = (vs.origin - point).Length();
			if (dist < min) {
				col_out = target;
			} else if (dist < max) {
				float ratio = (dist - min) / (max - min);
				float r = (float)col_out.r * ratio + (float)target.r * (1-ratio);
				float g = (float)col_out.g * ratio + (float)target.g * (1-ratio);
				float b = (float)col_out.b * ratio + (float)target.b * (1-ratio);
				float a = (float)col_out.a * ratio + (float)target.a * (1-ratio);
				col_out = { (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };
			}
		},
	};
}

RenderCallback RenderCallback::shade(Vector point, RenderCallback base) {
	return {
		[=](CViewSetup vs, Color &col_out, bool &nodepth_out) {
			base.cbk(vs, col_out, nodepth_out);

			Color light = engine->GetLightAtPoint(point);
			float r = (float)light.r / 255.0;
			float g = (float)light.g / 255.0;
			float b = (float)light.b / 255.0;

			// Scale the numbers in a way that seems reasonable
			r *= 15.0f;
			g *= 15.0f;
			b *= 15.0f;
			if (r > 1.0f) r = 1.0f;
			if (g > 1.0f) g = 1.0f;
			if (b > 1.0f) b = 1.0f;
			r = sqrt(r);
			g = sqrt(g);
			b = sqrt(b);

			// Bring all components close to the max to make the shading subtle
			float max = fmaxf(r, fmaxf(g, b));
			r += (max - r) * 0.3f;
			g += (max - g) * 0.3f;
			b += (max - b) * 0.3f;

			// Make sure it's at least slightly lit
			if (r < 0.2f) r = 0.2f;
			if (g < 0.2f) g = 0.2f;
			if (b < 0.2f) b = 0.2f;

			// Tint!
			col_out.r *= r;
			col_out.g *= g;
			col_out.b *= b;
		},
	};
}


///////////////////////////////////

// The address of this variable is used as a placeholder to be detected
// by createMeshInternal and friends. We use g_render_verts to keep
// track of which vertex array we're actually rendering.
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

struct OverlayMesh {
	RenderCallback solid;
	RenderCallback wireframe;
	Vector pos;
	int num_points_in_pos;
	std::vector<Vector> tri_verts;
	std::vector<Vector> line_verts;
};

static std::vector<OverlayMesh> g_meshes;
static size_t g_num_meshes;

static std::vector<Vector> *g_render_verts;

bool OverlayRender::createMeshInternal(const void *collision, Vector **vertsOut, size_t *nvertsOut) {
	if (collision != &g_placeholder) return false;
	*vertsOut = g_render_verts->data();
	*nvertsOut = g_render_verts->size();
	return true;
}

bool OverlayRender::destroyMeshInternal(Vector *verts, size_t nverts) {
	if (!g_render_verts) return false;
	return g_render_verts->data() == verts;
}

// Dispatched just before RENDER
ON_EVENT(FRAME) {
	// Garbage collection - remove any unused slots
	g_meshes.resize(g_num_meshes);

	// Clear the vertex arrays for each mesh that we'll keep around
	for (size_t i = 0; i < g_num_meshes; ++i) {
		auto &m = g_meshes[i];
		m.tri_verts.clear();
		m.line_verts.clear();
		m.pos = {0,0,0};
		m.num_points_in_pos = 0;
	}

	g_num_meshes = 0;

	g_text.clear();
}

MeshId OverlayRender::createMesh(RenderCallback solid, RenderCallback wireframe) {
	MeshId id = g_num_meshes;
	if (g_num_meshes == g_meshes.size()) g_meshes.push_back({});
	g_num_meshes += 1;
	g_meshes[id].solid = solid;
	g_meshes[id].wireframe = wireframe;
	return id;
}

void OverlayRender::addTriangle(MeshId mesh, Vector a, Vector b, Vector c, bool cull_back) {
	auto &vs = g_meshes[mesh].tri_verts;
	vs.insert(vs.end(), { a, b, c });
	if (!cull_back) vs.insert(vs.end(), { a, c, b });
	g_meshes[mesh].pos += (a + b + c) / 3.0;
	g_meshes[mesh].num_points_in_pos += 1;
}

void OverlayRender::addLine(MeshId mesh, Vector a, Vector b) {
	auto &vs = g_meshes[mesh].line_verts;
	vs.insert(vs.end(), { a, b });
	g_meshes[mesh].pos += (a + b) / 2.0;
	g_meshes[mesh].num_points_in_pos += 1;
}

void OverlayRender::addQuad(MeshId mesh, Vector a, Vector b, Vector c, Vector d, bool cull_back) {
	OverlayRender::addTriangle(mesh, a, b, c, cull_back);
	OverlayRender::addTriangle(mesh, a, c, d, cull_back);
}

void OverlayRender::addBoxMesh(Vector origin, Vector mins, Vector maxs, QAngle ang, RenderCallback solid, RenderCallback wireframe) {
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

	MeshId solid_mesh = OverlayRender::createMesh(solid, RenderCallback::none);
	for (auto i : std::array<std::array<int, 4>, 6>{
		std::array<int, 4>{ 2, 6, 4, 0 },
		std::array<int, 4>{ 7, 3, 1, 5 },
		std::array<int, 4>{ 4, 5, 1, 0 },
		std::array<int, 4>{ 3, 7, 6, 2 },
		std::array<int, 4>{ 1, 3, 2, 0 },
		std::array<int, 4>{ 6, 7, 5, 4 },
	}) {
		OverlayRender::addQuad(solid_mesh, verts[i[0]], verts[i[1]], verts[i[2]], verts[i[3]], true);
	}

	MeshId wf_mesh = OverlayRender::createMesh(RenderCallback::none, wireframe);
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
		OverlayRender::addLine(wf_mesh, verts[i[0]], verts[i[1]]);
	}
}

void OverlayRender::addText(Vector pos, int x_off, int y_off, std::string text, unsigned long font, Color col, bool center) {
	if (font == FONT_DEFAULT) font = scheme->GetDefaultFont();
	g_text.push_back({pos, center, x_off, y_off, std::move(text), col, font});
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

static IMaterial *createMaterial(KeyValues *kv, const char *name) {
	IMaterial *mat = (IMaterial *)materialSystem->CreateMaterial(materialSystem->materials->ThisPtr(), name, kv);
	auto IncrementReferenceCount = Memory::VMT<void (__rescall *)(IMaterial *thisptr)>(mat, 12);
	IncrementReferenceCount(mat);
	return mat;
}

static void destroyMaterial(IMaterial *mat) {
	if (!mat) return;
	auto DecrementReferenceCount = Memory::VMT<void (__rescall *)(IMaterial *thisptr)>(mat, 13);
	DecrementReferenceCount(mat);
}

static IMaterial *g_mat_solid, *g_mat_solid_noz, *g_mat_wireframe, *g_mat_wireframe_noz;

void OverlayRender::initMaterials() {
	KeyValues *kv;

	kv = new KeyValues("unlitgeneric");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$vertexalpha", 1);
	g_mat_solid = createMaterial(kv, "__utilVertexColor");

	kv = new KeyValues("unlitgeneric");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$vertexalpha", 1);
	kv->SetInt("$ignorez", 1);
	g_mat_solid_noz = createMaterial(kv, "__utilVertexColorIgnoreZ");

	kv = new KeyValues("wireframe");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$vertexalpha", 1);
	g_mat_wireframe = createMaterial(kv, "__utilWireframe");

	kv = new KeyValues("wireframe");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$vertexalpha", 1);
	kv->SetInt("$ignorez", 1);
	g_mat_wireframe_noz = createMaterial(kv, "__utilWireframeIgnoreZ");
}

ON_EVENT(SAR_UNLOAD) {
	destroyMaterial(g_mat_solid);
	destroyMaterial(g_mat_solid_noz);
	destroyMaterial(g_mat_wireframe);
	destroyMaterial(g_mat_wireframe_noz);
}

void OverlayRender::drawMeshes(void *viewrender) {
	matrix3x4_t transform{0};
	transform.m_flMatVal[0][0] = 1;
	transform.m_flMatVal[1][1] = 1;
	transform.m_flMatVal[2][2] = 1;

	CViewSetup setup = *(CViewSetup *)((uintptr_t)viewrender + 8); // CRendering3dView inherits CViewSetup! this is handy

	// Order meshes!
	struct MeshCompare {
		bool operator()(const OverlayMesh *a, const OverlayMesh *b) const {
			if (a->num_points_in_pos == 0) return false;
			if (b->num_points_in_pos == 0) return false;
			Vector dist_a = a->pos / a->num_points_in_pos - this->origin;
			Vector dist_b = b->pos / b->num_points_in_pos - this->origin;
			return dist_a.SquaredLength() > dist_b.SquaredLength();
		}
		Vector origin;
	};
	std::set<OverlayMesh *, MeshCompare> meshes(MeshCompare{setup.origin});
	for (size_t i = 0; i < g_num_meshes; ++i) {
		meshes.insert(&g_meshes[i]);
	}

	for (auto mp : meshes) {
		OverlayMesh &m = *mp;

		Color solid_color, wf_color;
		bool solid_nodepth, wf_nodepth;
		m.solid.cbk(setup, solid_color, solid_nodepth);
		m.wireframe.cbk(setup, wf_color, wf_nodepth);

		if (solid_color.a != 0) {
			IMaterial *mat = solid_nodepth ? g_mat_solid_noz : g_mat_solid;
			// Tris
			g_render_verts = &m.tri_verts;
			engine->DebugDrawPhysCollide(engine->engineClient->ThisPtr(), &g_placeholder, mat, transform, solid_color);
		}

		if (wf_color.a != 0) {
			IMaterial *mat = wf_nodepth ? g_mat_wireframe_noz : g_mat_wireframe;

			// Lines
			// There need to be some multiple of 3 verts for line drawing to work
			// properly. Push some garbage lines until we're matching
			while (m.line_verts.size() % 6) { // lcm(2,3)
				m.line_verts.push_back({0,0,0});
			}
			setPrimitiveType(1);
			g_render_verts = &m.line_verts;
			engine->DebugDrawPhysCollide(engine->engineClient->ThisPtr(), &g_placeholder, mat, transform, wf_color);

			// Tris
			setPrimitiveType(2);
			g_render_verts = &m.tri_verts;
			engine->DebugDrawPhysCollide(engine->engineClient->ThisPtr(), &g_placeholder, mat, transform, wf_color);
		}
	}
}
