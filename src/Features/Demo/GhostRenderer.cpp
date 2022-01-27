#include "GhostRenderer.hpp"

#include "Modules/Engine.hpp"
#include "Event.hpp"
#include "Features/Camera.hpp"
#include "Features/OverlayRender.hpp"
#include "GhostEntity.hpp"

#include <vector>


// Bendy model data
// 2D (Y,Z) coordinates of vertices.
const float BENDY_VERTS[] = {-13, 16, 13, 16, -13, 40, 13, 40, -12.55, 42.69, 12.55, 42.69, -11.26, 45.2, 11.26, 45.2, -9.19, 47.35, 9.19, 47.35, -6.5, 49.01, 6.5, 49.01, -3.36, 50.04, 3.36, 50.04, 0, 50.4, 11, 61, 10.63, 63.85, 9.53, 66.5, 7.78, 68.78, 5.5, 70.53, 2.85, 71.63, 0, 72, -2.85, 71.63, -5.5, 70.53, -7.78, 68.78, -9.53, 66.5, -10.63, 63.85, -11, 61, -10.63, 58.15, -9.53, 55.5, -7.78, 53.22, -5.5, 51.47, -2.85, 50.37, 0, 50, 2.85, 50.37, 5.5, 51.47, 7.78, 53.22, 9.53, 55.5, 10.63, 58.15, 9, 16, 1, 16, 1, 0, 9, 0, -1, 16, -9, 16, -9, 0, -1, 0, 9, 44, 3, 44, 3, 20, 9, 20, -3, 44, -9, 44, -9, 20, -3, 20, -13, 19, -13, 22, -13, 25, -13, 28, -13, 31, -13, 34, -13, 37, 13, 19, 13, 22, 13, 25, 13, 28, 13, 31, 13, 34, 13, 37};
// Vertex group ID each vertex belongs to.
const short BENDY_GROUPS[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
enum BENDY_GROUP {BODY, HEAD, LEG_LEFT, LEG_RIGHT, ARM_LEFT, ARM_RIGHT};

// Triangles of different LODs of the model
const short BENDY_MODEL_1[] = {6, 4, 2, 3, 5, 7, 2, 12, 10, 7, 9, 3, 9, 11, 3, 11, 13, 3, 13, 14, 3, 14, 12, 2, 3, 14, 2, 10, 8, 2, 8, 6, 2, 27, 28, 29, 29, 24, 27, 24, 25, 27, 26, 27, 25, 30, 31, 29, 21, 22, 24, 22, 23, 24, 33, 34, 35, 38, 15, 37, 36, 37, 35, 37, 15, 35, 15, 16, 35, 18, 35, 16, 16, 17, 18, 31, 32, 33, 33, 35, 31, 29, 31, 35, 35, 18, 29, 18, 19, 29, 24, 29, 19, 21, 24, 19, 19, 20, 21, 40, 41, 42, 42, 39, 40, 44, 45, 46, 46, 43, 44, 48, 49, 50, 50, 47, 48, 52, 53, 54, 54, 51, 52, 1, 55, 0, 62, 55, 1, 55, 62, 56, 62, 63, 56, 56, 63, 57, 63, 64, 57, 57, 64, 58, 64, 65, 58, 58, 65, 59, 65, 66, 59, 59, 66, 60, 66, 67, 60, 60, 67, 61, 67, 68, 61, 61, 68, 2, 68, 3, 2, -1};
const short BENDY_MODEL_2[] = {3, 14, 2, 3, 7, 11, 14, 3, 11, 14, 10, 2, 10, 6, 2, 25, 27, 29, 21, 23, 29, 25, 29, 23, 37, 15, 35, 19, 35, 15, 15, 17, 19, 33, 35, 31, 29, 31, 35, 35, 19, 29, 21, 29, 19, 40, 41, 42, 42, 39, 40, 44, 45, 46, 46, 43, 44, 48, 49, 50, 50, 47, 48, 52, 53, 54, 54, 51, 52, 1, 56, 0, 63, 56, 1, 56, 63, 58, 63, 65, 58, 58, 65, 60, 65, 67, 60, 60, 67, 2, 67, 3, 2, -1};
const short BENDY_MODEL_3[] = {3, 14, 2, 3, 9, 14, 2, 14, 8, 24, 27, 30, 36, 15, 18, 33, 36, 30, 36, 18, 30, 24, 30, 18, 21, 24, 18, 40, 41, 42, 42, 39, 40, 44, 45, 46, 46, 43, 44, 48, 49, 50, 50, 47, 48, 52, 53, 54, 54, 51, 52, 1, 58, 0, 65, 58, 1, 58, 65, 2, 65, 3, 2, -1};
const short* BENDY_MODELS[] = {BENDY_MODEL_1, BENDY_MODEL_2, BENDY_MODEL_3};
// The lowest LOD in which the vertex is used
const short BENDY_LOD_LEVELS[] = {3, 3, 3, 3, 1, 1, 2, 2, 3, 3, 2, 2, 1, 1, 3, 3, 1, 2, 3, 2, 1, 3, 1, 2, 3, 2, 1, 3, 1, 2, 3, 2, 1, 3, 1, 2, 3, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 2, 1, 3, 1, 2, 1, 1, 2, 1, 3, 1, 2, 1};


Variable ghost_bendy_lod_proximity("ghost_bendy_lod_proximity", "512", 0, 99999, "Distance from which Bendy ghosts should be drawn in lower level of detail.\n");
Variable ghost_bendy_force_lod("ghost_bendy_force_lod", "0", 0, 2, "LOD level that should be enforced for Bendy ghosts drawing.");



GhostRenderer::GhostRenderer() {
	float vertCount = sizeof(BENDY_VERTS) / (sizeof(float) * 2);
	animatedVerts.resize(vertCount);
}

void GhostRenderer::UpdateAnimatedVerts() {
	
	// time
	float time = engine->GetClientTime();
	float dt = fminf(fmaxf(time - lastUpdateCall, 0.0f), 0.1f);
	lastUpdateCall = time;

	// prepare ducking
	float duckFactor = (64.0f - ghost->data.view_offset) / 36.0f;

	// update walking cycle
	float vel = ghost->velocity.Length2D();
	if (vel >= 1.0f) {
		if (vel < 1.0) {
			walkingCycle = 0;
		} else {
			walkingCycle += dt * fminf(vel * 0.05f, 9.0f);
		}
		if (walkingCycle > M_PI) walkingCycle = fmodf(walkingCycle, M_PI);
	}

	// update squishing
	if (oldGroundedState != ghost->data.grounded) {
		if (ghost->velocity.z > 50.0f) {
			squishForce = 1.5f;
		} else if (!oldGroundedState) {
			squishForce = 0.5f;
		}
		oldGroundedState = ghost->data.grounded;
	}
	if (squishForce > 0) {
		squishForce -= dt * 8.0f;
		if (squishForce < 0) squishForce = 0; 
	}

	// update model
	int lod = GetLODLevel();
	for (int v = 0; v < animatedVerts.size(); v++) {
		// do not waste time on vertices that won't be used for drawing
		if (BENDY_LOD_LEVELS[v] < lod) continue;

		int vertGroup = BENDY_GROUPS[v];

		// skip hands if not gesturing
		if (vertGroup == BENDY_GROUP::ARM_LEFT || vertGroup == BENDY_GROUP::ARM_RIGHT) {
			animatedVerts[v] = {0, 0, 0};
			continue;
		}

		// raw vertex
		Vector rawPos = {0, BENDY_VERTS[v * 2], BENDY_VERTS[v * 2 + 1]};
		Vector localPos = rawPos;

		// pitch angle animation
		float floppyForce = powf(localPos.z / 72.0f, 3.5f);

		float floppyFactor = sinf(DEG2RAD(ghost->data.view_angle.x*0.5f));
		Vector offset = {sinf(floppyFactor) * floppyForce * 48.0f, 0, (cosf(floppyFactor) - 1.0f) * floppyForce * 72.0f};

		localPos += offset;

		// running animation
		if (ghost->data.grounded && vel >= 1.0f) {
			float animScale = fminf((vel - 1.0f) * 0.02f, 1.0f);
			// I want to move only low vertices of legs, so I'm detecting them this way lol
			if (rawPos.z < 10.0f) {
				float cycle = walkingCycle;
				if (vertGroup == BENDY_GROUP::LEG_LEFT) cycle += M_PI*0.5f;
				localPos.z += (fabsf(sinf(cycle)) * 8.0f) * animScale;
			} else {
				localPos.z += (fabsf(sinf(walkingCycle * 2.0f)) * 4.0f - 4.0f) * animScale;
			}
		}

		// ducking
		if (duckFactor > 0) {
			if (localPos.z < 40.0f) {
				localPos.z *= 0.5f + 0.5f * (1.0f-duckFactor);
			} else {
				localPos.z -= 20.0f * duckFactor;
			}
		}

		// squishing when jumping and landing
		if (squishForce > 0) {
			localPos.z *= 1.0f - squishForce * 0.2f;
			if (!ghost->data.grounded) {
				localPos.z += 36.0f * squishForce * 0.2f;
			}
		}

		// yaw angle rotation
		float yawCos = cosf(DEG2RAD(ghost->data.view_angle.y));
		float yawSin = sinf(DEG2RAD(ghost->data.view_angle.y));
		localPos = {
			localPos.x * yawCos - localPos.y * yawSin,
			localPos.x * yawSin + localPos.y * yawCos,
			localPos.z
		};

		// transform it to global coordinates
		Vector globalPos = ghost->data.position + localPos;

		// save the processed vertex
		animatedVerts[v] = globalPos;
	}
}


void GhostRenderer::Draw() {
	if (ghost == nullptr) return;

	//update verts before drawing
	UpdateAnimatedVerts();

	Vector camPos = camera->GetPosition(GET_SLOT());
	Vector camForward = camera->GetForwardVector(GET_SLOT());
	
	// get model depending on assigned LOD level
	// each LOD is sharing the same vertices table
	const short *model = BENDY_MODELS[GetLODLevel()];

	Color col = ghost->GetColor();
#define TRIANGLE(p1, p2, p3) OverlayRender::addTriangle(p1, p2, p3, col, false)
	// draw each triangle
	for (int t = 0; model[t] >= 0; t+=3) {
		Vector p1 = animatedVerts[model[t]]; 
		Vector p2 = animatedVerts[model[t+1]]; 
		Vector p3 = animatedVerts[model[t+2]]; 

		// triangles are drawn only from one side. draw both sides.
		TRIANGLE(p1, p2, p3);
	}
#undef TRIANGLE
}

void GhostRenderer::SetGhost(GhostEntity* ghost) {
	this->ghost = ghost;
}

int GhostRenderer::GetLODLevel() {
	if (ghost == nullptr) return 0;

	int forceLod = ghost_bendy_force_lod.GetInt();
	if (forceLod > 0) return forceLod;

	Vector camPos = camera->GetPosition(GET_SLOT());
	float dist = (camPos - ghost->data.position).Length();

	float lodDist = ghost_bendy_lod_proximity.GetFloat();
	if (dist < lodDist) {
		return 0;
	} else if (dist < lodDist * 2) {
		return 1;
	} else {
		return 2;
	}
}

void GhostRenderer::StartGesture(int id) {

}

float GhostRenderer::GetHeight() {
	return 72.0f;
}