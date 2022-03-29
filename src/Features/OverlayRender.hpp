#pragma once

#include "Utils/SDK.hpp"
#include <string>
#include <climits>
#include <functional>

#define FONT_DEFAULT ULONG_MAX

// A callback to determine the rendering parameters for a mesh based on
// a given CViewSetup.
struct RenderCallback {
	std::function<void (CViewSetup, Color &, bool &)> cbk;

	// A trivial "never render" callback
	static RenderCallback none;

	// A callback to return constant color and depth values
	static RenderCallback constant(Color col, bool nodepth = false);

	// A callback to fade towards a different color when a certain point
	// is close to the camera
	static RenderCallback prox_fade(float min, float max, Color target, Vector point, RenderCallback base);

	// A callback to tint a mesh's color based on the lighting at a
	// certain position in the world
	static RenderCallback shade(Vector point, RenderCallback base);
};

typedef size_t MeshId;

namespace OverlayRender {
	// INTERNAL FUNCTIONS - DO NOT USE
	bool createMeshInternal(const void *collision, Vector **vertsOut, size_t *nverts);
	bool destroyMeshInternal(Vector *verts, size_t nverts);
	//void drawTranslucents();
	//void drawOpaques();
	void drawMeshes(void *viewrender);
	void initMaterials();

	// Every primitive has to be within a mesh
	MeshId createMesh(RenderCallback solid, RenderCallback wireframe);

	// Primitives that can be drawn during a mesh
	void addTriangle(MeshId mesh, Vector a, Vector b, Vector c, bool cull_back = false);
	void addLine(MeshId mesh, Vector a, Vector b);
	void addQuad(MeshId mesh, Vector a, Vector b, Vector c, Vector d, bool cull_back = false);

	// Standalone overlay functions - don't use these within a mesh
	void addBoxMesh(Vector origin, Vector mins, Vector maxs, QAngle ang, RenderCallback solid, RenderCallback wireframe);
	void addText(Vector pos, int x_off, int y_off, std::string text, unsigned long font = FONT_DEFAULT, Color col = {255,255,255}, bool center = true);
}
