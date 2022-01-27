#pragma once
#include <Utils/SDK.hpp>

#include <vector>

class GhostEntity;

class GhostRenderer {
private:
	GhostEntity *ghost;

	float lastUpdateCall;
	std::vector<Vector> animatedVerts;
	float walkingCycle;
	float squishForce;
	bool oldGroundedState;

private:
	void UpdateAnimatedVerts();
public:
	GhostRenderer();
	void SetGhost(GhostEntity *ghost);
	void Draw();
	int GetLODLevel();
	void StartGesture(int id);
	float GetHeight();
};