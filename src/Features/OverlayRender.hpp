#pragma once

#include "Utils/SDK.hpp"
#include <string>
#include <climits>

#define FONT_DEFAULT ULONG_MAX

namespace OverlayRender {
	bool createMeshInternal(void *collision, Vector **vertsOut, size_t *nverts);
	bool destroyMeshInternal(Vector *verts, size_t nverts);
	void drawMeshes();

	void startShading(Vector point);
	void endShading();

	void addTriangle(Vector a, Vector b, Vector c, Color col, bool cullBack = false);
	void addQuad(Vector a, Vector b, Vector c, Vector d, Color col, bool cullBack = false);
	void addLine(Vector a, Vector b, Color col, bool throughWalls = false);
	void addBox(Vector origin, Vector mins, Vector maxs, QAngle ang, Color col, bool wireframe = true, bool wireframeThroughWalls = false);
	
	void addText(Vector pos, int xOff, int yOff, const std::string &text, unsigned long font = FONT_DEFAULT, Color col = {255,255,255}, bool center = true);
}
