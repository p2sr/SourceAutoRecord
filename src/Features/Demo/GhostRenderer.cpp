#include "GhostRenderer.hpp"

#include "Modules/Engine.hpp"
#include "Event.hpp"
#include "Features/Camera.hpp"
#include "Features/OverlayRender.hpp"
#include "GhostEntity.hpp"
#include "GhostBendyModel.hpp"
#include "NetworkGhostPlayer.hpp"


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
	for (const auto& anim : animations) {
		if (anim.identity == &animation) {
			return;
		}
	}

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


DECL_COMMAND_COMPLETION(ghost_taunt) {
	for (const auto &def : g_ghostTauntAnimationDefinitions) {
		if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
			break;
		}

		if (std::strlen(match) != std::strlen(cmd)) {
			if (std::strstr(def.name.c_str(), match)) {
				items.push_back(def.name);
			}
		} else {
			items.push_back(def.name);
		}
	}

	FINISH_COMMAND_COMPLETION();
}

CON_COMMAND_F_COMPLETION(ghost_taunt, "ghost_taunt <animation_name> - Plays a taunt animation\n", FCVAR_NONE, AUTOCOMPLETION_FUNCTION(ghost_taunt)) {
	if (args.ArgC() < 2) {
		console->Print(ghost_taunt.ThisPtr()->m_pszHelpString);
		return;
	}

	if (!networkManager.isConnected) {
		return console->Print("Must be connected to a ghost server.\n");
	}

	std::string animName = args[1];
	auto it = std::find_if(
		g_ghostTauntAnimationDefinitions.begin(),
		g_ghostTauntAnimationDefinitions.end(),
		[&](const GhostAnimationDefinition &def) { return def.name == animName; });
	if (it == g_ghostTauntAnimationDefinitions.end()) {
		console->Print("Unknown taunt animation: %s\n", animName.c_str());
		return;
	}

	networkManager.NotifyTaunt(it->name);
}

CON_COMMAND(ghost_locator, "ghost_locator - Sends a coop-like ping to other ghosts\n") {
	if (!networkManager.isConnected) {
		return console->Print("Must be connected to a ghost server.\n");
	}

	Vector cam_pos;
	QAngle cam_ang;
	camera->GetEyePos<false>(GET_SLOT(), cam_pos, cam_ang);

	Vector dir;
	Math::AngleVectors(cam_ang, &dir);
	dir *= 8192.0f;

	CGameTrace tr;

	Ray_t ray;
	ray.m_IsRay = true;
	ray.m_IsSwept = true;
	ray.m_Start = VectorAligned(cam_pos.x, cam_pos.y, cam_pos.z);
	ray.m_Delta = VectorAligned(dir.x, dir.y, dir.z);
	ray.m_StartOffset = VectorAligned();
	ray.m_Extents = VectorAligned();

	CTraceFilterSimple filter;
	filter.SetPassEntity(server->GetPlayer(GET_SLOT() + 1)); // TODO: orange?

	engine->TraceRay(engine->engineTrace->ThisPtr(), ray, MASK_SHOT_PORTAL, &filter, &tr);

	client->ShowLocator(tr.endpos, tr.plane.normal, GhostEntity::set_color);
	networkManager.NotifyLocator(tr.endpos, tr.plane.normal);
}
