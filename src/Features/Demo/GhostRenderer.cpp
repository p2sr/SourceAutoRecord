#include "GhostRenderer.hpp"

#include "Modules/Engine.hpp"
#include "Event.hpp"
#include "Features/Camera.hpp"
#include "Features/OverlayRender.hpp"
#include "GhostEntity.hpp"
#include "GhostBendyModel.hpp"


GhostRenderer::GhostRenderer() {
	animatedVerts.resize(GhostBendyModel::GetVerticesCount());

	for (const auto &animationDef : g_ghostMainAnimationDefinitions) {
		StartAnimation(animationDef);
	}
}

void GhostRenderer::UpdateAnimation() {
	float time = engine->GetHostTime();
	float dt = fminf(fmaxf(time - lastUpdateCall, 0.0f), 0.1f);
	lastUpdateCall = time;

	UpdateAnimationStates(dt);
	UpdateAnimatedVertices(dt);
}

void GhostRenderer::UpdateAnimationStates(float deltaTime) {
	for (auto &animation : animations) {
		animation.UpdateState(ghost, deltaTime);
	}

	animations.erase(
		std::remove_if(animations.begin(), animations.end(), [](const GhostAnimationInstance &animation) {
			return !animation.state.active;
		}),
	animations.end());
}

void GhostRenderer::UpdateAnimatedVertices(float deltaTime) {
	for (int vertexIndex = 0; vertexIndex < GhostBendyModel::GetVerticesCount(); vertexIndex++) {
		auto vertexInfo = GhostBendyModel::GetVertexInfo(vertexIndex);

		Vector animatedPos = vertexInfo.position;

		for (const auto &animation : animations) {
			animation.UpdateVertex(vertexInfo, animatedPos);
		}

		TransformVertexFromLocalToWorldSpace(animatedPos);

		animatedVerts[vertexIndex] = animatedPos;
	}
}

void GhostRenderer::TransformVertexFromLocalToWorldSpace(Vector &vertex) {
	float yawCos = cosf(DEG2RAD(ghost->data.view_angle.y));
	float yawSin = sinf(DEG2RAD(ghost->data.view_angle.y));

	vertex = {
		vertex.x * yawCos - vertex.y * yawSin,
		vertex.x * yawSin + vertex.y * yawCos,
		vertex.z
	};

	vertex += ghost->data.position;
}

void GhostRenderer::Draw(MeshId &mesh) {
	if (ghost == nullptr) return;

	if (ghost->name == "jeb_" || ghost->name == "AMJ") {
		int host, server, client;
		engine->GetTicks(host, server, client);
		int hue = (server / 4) % 360;
		ghost->color = Utils::HSVToRGB(hue, 100, 100);
	}

	//update verts before drawing
	UpdateAnimation();
	
	// draw each triangle
	int triangleIndex = 0;
	short i1, i2, i3;
	while(GhostBendyModel::TryGetTriangleIndices(triangleIndex, &i1, &i2, &i3)) {
		Vector p1 = animatedVerts[i1];
		Vector p2 = animatedVerts[i2];
		Vector p3 = animatedVerts[i3];

		OverlayRender::addTriangle(mesh, p1, p2, p3);
		triangleIndex++;
	}
}

void GhostRenderer::SetGhost(GhostEntity* ghost) {
	this->ghost = ghost;
}

void GhostRenderer::StartAnimation(const GhostAnimationDefinition &animation) {
	GhostAnimationInstance instance;
	instance.identity = &animation;
	instance.state.active = true;

	animations.push_back(instance);
}

float GhostRenderer::GetHeight() {
	return this->ghost->data.view_offset * 36.0f / 64.0f + 36.0f;
}

void GhostAnimationInstance::UpdateState(const GhostEntity *ghost, float deltaTime) {
	if (state.active && identity->stateUpdateFunc != nullptr) {
		identity->stateUpdateFunc(ghost, deltaTime, state);
	}
}

void GhostAnimationInstance::UpdateVertex(GhostBendyModel::VertexInfo vertexInfo, Vector &animatedVertex) const {
	if (identity->vertexUpdateFunc != nullptr) {
		identity->vertexUpdateFunc(state, vertexInfo, animatedVertex);
	}
}
