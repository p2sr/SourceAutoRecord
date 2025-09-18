#pragma once
#include "Utils/SDK.hpp"
#include "Features/OverlayRender.hpp"
#include "GhostBendyModel.hpp"

#include <vector>

class GhostEntity;

struct GhostAnimationState {
	bool active = true;
	float controlValue1 = 0.0f;
	float controlValue2 = 0.0f;
	float controlValue3 = 0.0f;
};

using GhostAnimationStateUpdateFunc = std::function<void(const GhostEntity*, float, GhostAnimationState&)>;
using GhostAnimationVertexUpdateFunc = std::function<void(const GhostAnimationState&, GhostBendyModel::VertexInfo, Vector&)>;

struct GhostAnimationDefinition {
	std::string name;
	GhostAnimationStateUpdateFunc stateUpdateFunc;
	GhostAnimationVertexUpdateFunc vertexUpdateFunc;
};

struct GhostAnimationInstance {
	const GhostAnimationDefinition *identity;
	GhostAnimationState state;

	void UpdateState(const GhostEntity *ghost, float deltaTime);
	void UpdateVertex(GhostBendyModel::VertexInfo vertexInfo, Vector &animatedVertex) const;
};

extern std::vector<GhostAnimationDefinition> g_ghostMainAnimationDefinitions;
extern std::vector<GhostAnimationDefinition> g_ghostTauntAnimationDefinitions;

class GhostRenderer {
private:
	GhostEntity *ghost = nullptr;

	float lastUpdateCall = 0.0f;
	std::vector<Vector> animatedVerts = {};
	std::vector<GhostAnimationInstance> animations;
	bool oldGroundedState = false;

private:
	void UpdateAnimation();
	void UpdateAnimationStates(float deltaTime);
	void UpdateAnimatedVertices(float deltaTime);
	void TransformVertexFromLocalToWorldSpace(Vector &vertex);
public:
	GhostRenderer();
	void SetGhost(GhostEntity *ghost);
	void Draw(MeshId &mesh);
	void StartAnimation(const GhostAnimationDefinition &animation);
	float GetHeight();
};
