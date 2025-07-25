#pragma once
#include "Utils/SDK.hpp"
#include "Features/OverlayRender.hpp"

#include <vector>

class GhostEntity;

class GhostRenderer {
private:
	GhostEntity *ghost = nullptr;

	float lastUpdateCall = 0.0f;
	std::vector<Vector> animatedVerts = {};
	float walkingCycle = 0.0f;
	float squishForce = 0.0f;
	bool oldGroundedState = false;

private:
	void UpdateAnimatedVerts();
public:
	GhostRenderer();
	void SetGhost(GhostEntity *ghost);
	void Draw(MeshId &mesh);
	void StartGesture(int id);
	float GetHeight();
};
