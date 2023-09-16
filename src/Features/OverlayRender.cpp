#include "OverlayRender.hpp"
#include "Modules/Engine.hpp"
#include "Modules/MaterialSystem.hpp"
#include "Modules/Surface.hpp"
#include "Modules/Scheme.hpp"
#include "Event.hpp"
#include "Features/Session.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Utils/FontAtlas.hpp"

#include <array>
#include <set>

#define FONT_HPAD 48
#define FONT_VPAD 24

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

struct OverlayText {
	Vector pos;
	OverlayRender::TextAlign align;
	std::string text;
	Color col;
	float x_height; // in units
	bool visibility_scale; // should we scale the text size to make it more visible from afar?
	bool no_depth;
};

static std::vector<OverlayText> g_text;

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

void OverlayRender::addText(Vector pos, const std::string &text, float x_height, bool visibility_scale, bool no_depth, OverlayRender::TextAlign align, Color col) {
	g_text.push_back({pos, align, text, col, x_height, visibility_scale, no_depth});
}

static IMaterial *createMaterial(KeyValues *kv, const char *name) {
	return materialSystem->CreateMaterial(materialSystem->materials->ThisPtr(), name, kv);
}

static void destroyMaterial(IMaterial *mat) {
	if (!mat) return;

	auto material = reinterpret_cast<CMaterial_QueueFriendly*>(mat)->m_pRealTimeVersion;

	auto DecrementReferenceCount = Memory::VMT<void (__rescall *)(IMaterialInternal *thisptr)>(material, Offsets::DecrementReferenceCount);
	DecrementReferenceCount(material);

	materialSystem->RemoveMaterial(materialSystem->materials->ThisPtr(), material);
	mat = nullptr;
}

static IMaterial *g_mat_solid_opaque,     *g_mat_solid_opaque_noz,     *g_mat_solid_alpha,     *g_mat_solid_alpha_noz;
static IMaterial *g_mat_wireframe_opaque, *g_mat_wireframe_opaque_noz, *g_mat_wireframe_alpha, *g_mat_wireframe_alpha_noz;
static IMaterial *g_mat_font, *g_mat_font_noz;
static ITexture *g_tex_font_atlas;

void OverlayRender::initMaterials() {
	KeyValues *kv;

	kv = new KeyValues("unlitgeneric");
	kv->SetInt("$vertexcolor", 1);
	g_mat_solid_opaque = createMaterial(kv, "_SAR_UnlitSolidOpaque");

	kv = new KeyValues("unlitgeneric");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$ignorez", 1);
	g_mat_solid_opaque_noz = createMaterial(kv, "_SAR_UnlitSolidOpaqueNoDepth");

	kv = new KeyValues("unlitgeneric");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$vertexalpha", 1);
	g_mat_solid_alpha = createMaterial(kv, "_SAR_UnlitSolidAlpha");

	kv = new KeyValues("unlitgeneric");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$vertexalpha", 1);
	kv->SetInt("$ignorez", 1);
	g_mat_solid_alpha_noz = createMaterial(kv, "_SAR_UnlitSolidAlphaNoDepth");

	kv = new KeyValues("wireframe");
	kv->SetInt("$vertexcolor", 1);
	g_mat_wireframe_opaque = createMaterial(kv, "_SAR_UnlitWireframeOpaque");

	kv = new KeyValues("wireframe");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$ignorez", 1);
	g_mat_wireframe_opaque_noz = createMaterial(kv, "_SAR_UnlitWireframeOpaqueNoDepth");

	kv = new KeyValues("wireframe");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$vertexalpha", 1);
	g_mat_wireframe_alpha = createMaterial(kv, "_SAR_UnlitWireframeAlpha");

	kv = new KeyValues("wireframe");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$vertexalpha", 1);
	kv->SetInt("$ignorez", 1);
	g_mat_wireframe_alpha_noz = createMaterial(kv, "_SAR_UnlitWireframeAlphaNoDepth");

	g_tex_font_atlas = materialSystem->CreateTexture("_SAR_FontAtlasTex", FONT_ATLAS_WIDTH, FONT_ATLAS_HEIGHT, FONT_ATLAS_DATA);

	kv = new KeyValues("unlitgeneric");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$vertexalpha", 1);
	kv->SetInt("$translucent", 1);
	kv->SetString("$basetexture", "_SAR_FontAtlasTex");
	g_mat_font = createMaterial(kv, "_SAR_FontAtlas");

	kv = new KeyValues("unlitgeneric");
	kv->SetInt("$vertexcolor", 1);
	kv->SetInt("$vertexalpha", 1);
	kv->SetInt("$translucent", 1);
	kv->SetInt("$ignorez", 1);
	kv->SetString("$basetexture", "_SAR_FontAtlasTex");
	g_mat_font_noz = createMaterial(kv, "_SAR_FontAtlasNoDepth");
}

ON_EVENT(SAR_UNLOAD) {
	destroyMaterial(g_mat_solid_opaque);
	destroyMaterial(g_mat_solid_opaque_noz);
	destroyMaterial(g_mat_solid_alpha);
	destroyMaterial(g_mat_solid_alpha_noz);
	destroyMaterial(g_mat_wireframe_alpha);
	destroyMaterial(g_mat_wireframe_alpha_noz);
	destroyMaterial(g_mat_wireframe_opaque);
	destroyMaterial(g_mat_wireframe_opaque_noz);
	destroyMaterial(g_mat_font);
	destroyMaterial(g_mat_font_noz);
	materialSystem->DestroyTexture(g_tex_font_atlas);
}

static void drawVerts(IMaterial *mat, bool lines, Vector *verts, int nverts, Color col) {
	int prims = lines ? nverts / 2 : nverts / 3;
	MeshBuilder mb(mat, lines ? PrimitiveType::LINES : PrimitiveType::TRIANGLES, prims);

	for (int i = 0; i < nverts; ++i) {
		mb.Position(verts[i]);
		mb.Color(col);
		mb.AdvanceVertex();
	}

	mb.Draw();
}

static void drawMesh(const CViewSetup &setup, OverlayMesh &m, bool translucent) {
	Color solid_color, wf_color;
	bool solid_nodepth, wf_nodepth;
	m.solid.cbk(setup, solid_color, solid_nodepth);
	m.wireframe.cbk(setup, wf_color, wf_nodepth);

	if (solid_color.a != 0 && (translucent ^ (solid_color.a == 255))) {
		IMaterial *mat = solid_nodepth ?
			(translucent ? g_mat_solid_alpha_noz : g_mat_solid_opaque_noz) :
			(translucent ? g_mat_solid_alpha     : g_mat_solid_opaque);

		// Tris
		drawVerts(mat, false, m.tri_verts.data(), m.tri_verts.size(), solid_color);
	}

	if (wf_color.a != 0 && (translucent ^ (wf_color.a == 255))) {
		IMaterial *mat = wf_nodepth ?
			(translucent ? g_mat_wireframe_alpha_noz : g_mat_wireframe_opaque_noz) :
			(translucent ? g_mat_wireframe_alpha     : g_mat_wireframe_opaque);

		// Lines
		drawVerts(mat, true, m.line_verts.data(), m.line_verts.size(), wf_color);

		// Tris
		drawVerts(mat, true, m.tri_verts.data(), m.tri_verts.size(), wf_color);
	}
}

static Matrix createTextRotationMatrix(Vector text_pos, CViewSetup setup) {
	(void)text_pos; // not used for now

	QAngle ang = QAngle{ -setup.angles.x, fmodf(setup.angles.y + 180.0, 360.0), 0 };

	// create yaw+pitch rotation matrix for angles (roll ignored)
	double syaw = sin(ang.y * M_PI/180);
	double cyaw = cos(ang.y * M_PI/180);
	double spitch = sin(ang.x * M_PI/180);
	double cpitch = cos(ang.x * M_PI/180);
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

	return rot;
}

static float drawTextLine(const char *str, Vector top_center, Color text_color, float scale, Matrix rot, bool no_depth) {
	int width = 0;
	int min_height = 0;
	int max_height = 0;
	for (const char *ptr = str; *ptr; ++ptr) {
		auto info = FONT_ATLAS_INFO[*ptr];
		width += info.advance;
		if (info.origin_y > max_height) max_height = info.origin_y;
		if (info.origin_y - info.height < min_height) min_height = info.origin_y - info.height;
	}

	Vector center_baseline = top_center + rot * Vector{ 0, 0, -(float)max_height } * scale;

	{
		Vector base = center_baseline + rot * Vector{ 0.0, -(float)width * scale * 0.5f, 0.0 };

		MeshBuilder text(no_depth ? g_mat_font_noz : g_mat_font, PrimitiveType::QUADS, strlen(str));
		for (const char *ptr = str; *ptr; ++ptr) {
			auto info = FONT_ATLAS_INFO[*ptr];

			Vector bl = base + rot * Vector{ 0.0, -(float)info.origin_x, (float)(info.origin_y - info.height) } * scale;
			Vector tl = base + rot * Vector{ 0.0, -(float)info.origin_x, (float)info.origin_y } * scale;
			Vector br = base + rot * Vector{ 0.0, (float)(-info.origin_x + info.width), (float)(info.origin_y - info.height) } * scale;
			Vector tr = base + rot * Vector{ 0.0, (float)(-info.origin_x + info.width), (float)info.origin_y } * scale;

			float tex[4] = {
				(float)info.x / FONT_ATLAS_WIDTH,
				(float)(info.y + info.height) / FONT_ATLAS_HEIGHT,
				(float)(info.x + info.width) / FONT_ATLAS_WIDTH,
				(float)info.y / FONT_ATLAS_HEIGHT,
			};

			text.Position(bl); text.Color(text_color); text.TexCoord(0, tex[0], tex[1]); text.AdvanceVertex();
			text.Position(tl); text.Color(text_color); text.TexCoord(0, tex[0], tex[3]); text.AdvanceVertex();
			text.Position(tr); text.Color(text_color); text.TexCoord(0, tex[2], tex[3]); text.AdvanceVertex();
			text.Position(br); text.Color(text_color); text.TexCoord(0, tex[2], tex[1]); text.AdvanceVertex();

			base += rot * Vector{ 0.0, (float)info.advance, 0.0 } * scale;
		}
		text.Draw();
	}

	return (max_height - min_height + FONT_VPAD) * scale;
}

static void drawText(CViewSetup setup, OverlayText &t) {
	std::vector<std::string> lines;
	int height = FONT_VPAD;
	int last_base_delta = 0;
	int max_width = 0;

	{
		std::string all = t.text;
		while (!all.empty()) {
			size_t pos = all.find("\n");
			std::string line;
			if (pos != std::string::npos) {
				line = all.substr(0, pos);
				all.erase(0, pos + 1);
			} else {
				line = all;
				all.clear();
			}

			int width = 0;
			int min_height = 0;
			int max_height = 0;
			for (char c : line) {
				auto info = FONT_ATLAS_INFO[c];
				width += info.advance;
				if (info.origin_y > max_height) max_height = info.origin_y;
				if (info.origin_y - info.height < min_height) min_height = info.origin_y - info.height;
			}

			lines.push_back(line);
			if (width > max_width) max_width = width;
			height += max_height - min_height + FONT_VPAD;
			last_base_delta = -min_height + FONT_VPAD;
		}
	}

	float scale = t.x_height / (float)FONT_ATLAS_INFO['x'].height;
	if (t.visibility_scale) {
		float dist = (setup.origin - t.pos).Length();
		if (dist > 100) {
			// this seems to work fairly well just from briefly messing around
			scale *= sqrt((dist - 60) / 40);
		}
	}

	Matrix rotation = createTextRotationMatrix(t.pos, setup);

	// the top-center of the top line of text, including padding
	Vector top_center = t.pos;

	switch (t.align) {
	case OverlayRender::TextAlign::BOTTOM:
		top_center += rotation * Vector{ 0, 0, (float)height } * scale;
		break;
	case OverlayRender::TextAlign::CENTER:
		top_center += rotation * Vector{ 0, 0, (float)height * 0.5f } * scale;
		break;
	case OverlayRender::TextAlign::TOP:
		break;
	case OverlayRender::TextAlign::BASELINE:
		top_center += rotation * Vector{ 0, 0, (float)(height - last_base_delta)} * scale;
		break;
	}

	// draw background
	{
		Color bg_color{ 0, 0, 0, 200 }; 

		Vector bl = top_center + rotation * Vector{ 0.0, -(float)max_width * 0.5f - FONT_HPAD, -(float)height } * scale;
		Vector tl = top_center + rotation * Vector{ 0.0, -(float)max_width * 0.5f - FONT_HPAD, 0 } * scale;
		Vector br = top_center + rotation * Vector{ 0.0, (float)max_width * 0.5f + FONT_HPAD, -(float)height } * scale;
		Vector tr = top_center + rotation * Vector{ 0.0, (float)max_width * 0.5f + FONT_HPAD, 0 } * scale;

		MeshBuilder bg(t.no_depth ? g_mat_solid_alpha_noz : g_mat_solid_alpha, PrimitiveType::QUADS, 1);
		bg.Position(bl); bg.Color(bg_color); bg.AdvanceVertex();
		bg.Position(tl); bg.Color(bg_color); bg.AdvanceVertex();
		bg.Position(tr); bg.Color(bg_color); bg.AdvanceVertex();
		bg.Position(br); bg.Color(bg_color); bg.AdvanceVertex();
		bg.Draw();
	}

	top_center -= rotation * Vector{ 0, 0, FONT_VPAD } * scale;

	for (auto &line : lines) {
		float draw_height = drawTextLine(line.c_str(), top_center, t.col, scale, rotation, t.no_depth);
		top_center -= rotation * Vector{ 0, 0, draw_height };
	}
}

void OverlayRender::drawOpaques(void *viewrender) {
	CViewSetup setup = *(CViewSetup *)((uintptr_t)viewrender + 8); // CRendering3dView inherits CViewSetup! this is handy

	for (size_t i = 0; i < g_num_meshes; ++i) {
		drawMesh(setup, g_meshes[i], false);
	}
}

void OverlayRender::drawTranslucents(void *viewrender) {
	CViewSetup setup = *(CViewSetup *)((uintptr_t)viewrender + 8); // CRendering3dView inherits CViewSetup! this is handy

	// Order meshes!
	struct MeshCompare {
		bool operator()(std::pair<bool, void *> a, std::pair<bool, void *> b) const {
			float dist_a;
			if (a.first) {
				OverlayText *t = (OverlayText *)a.second;
				dist_a = (t->pos - this->origin).SquaredLength();
			} else {
				OverlayMesh *m = (OverlayMesh *)a.second;
				int npa = m->num_points_in_pos == 0 ? 1 : m->num_points_in_pos;
				dist_a = (m->pos / npa - this->origin).SquaredLength();
			}

			float dist_b;
			if (b.first) {
				OverlayText *t = (OverlayText *)b.second;
				dist_b = (t->pos - this->origin).SquaredLength();
			} else {
				OverlayMesh *m = (OverlayMesh *)b.second;
				int npb = m->num_points_in_pos == 0 ? 1 : m->num_points_in_pos;
				dist_b = (m->pos / npb - this->origin).SquaredLength();
			}

			if (dist_a == dist_b) return a.second < b.second; // Define *some* kind of ordering for meshes in the same place
			return dist_a > dist_b;
		}
		Vector origin;
	};
	std::set<std::pair<bool, void *>, MeshCompare> meshes(MeshCompare{setup.origin});
	for (size_t i = 0; i < g_num_meshes; ++i) {
		meshes.insert({ false, &g_meshes[i] });
	}
	for (auto &text : g_text) {
		meshes.insert({ true, &text });
	}

	for (auto mesh : meshes) {
		if (mesh.first) {
			drawText(setup, *(OverlayText *)mesh.second);
		} else {
			drawMesh(setup, *(OverlayMesh *)mesh.second, true);
		}
	}
}
