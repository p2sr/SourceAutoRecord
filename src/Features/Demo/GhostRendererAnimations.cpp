#include "GhostEntity.hpp"
#include "GhostRenderer.hpp"
#include "GhostBendyModel.hpp"

GhostAnimationDefinition walkingAnimation{
	"walking",
	[](const GhostEntity *ghost, float dt, GhostAnimationState &state) {
		float ghostHorizontalSpeed = ghost->velocity.Length2D();

		float animScale = fminf((ghostHorizontalSpeed - 1.0f) * 0.02f, 1.0f);
		float targetControlValue = animScale * (ghost->data.grounded ? 1.0f : 0.0f);
		state.controlValue2 = Math::Lerp(state.controlValue2, targetControlValue, dt * 10.0f);

		if (ghostHorizontalSpeed <= 1.0f) {
			state.controlValue1 = 0;
		} else {
			float valueIncrease = dt * fminf(ghostHorizontalSpeed * 0.05f, 9.0f);
			state.controlValue1 = fmodf(state.controlValue1 + valueIncrease, M_PI);
		}
	},
	[](const GhostAnimationState &state, GhostBendyModel::VertexInfo vertexInfo, Vector &animatedVertex) {
		bool lowVerticesOfLeg = vertexInfo.position.z < 10.0f;
		bool isLeftLeg = vertexInfo.group == GhostBendyModel::LEFT_LEG;

		float offset;
		if (lowVerticesOfLeg) {
			float cycle = state.controlValue1 + (isLeftLeg ? M_PI * 0.5f : 0);
			offset = fabsf(sinf(cycle)) * 8.0f;
		} else {
			offset = fabsf(sinf(state.controlValue1 * 2.0f)) * 4.0f - 4.0f;
		}

		animatedVertex.z += offset * state.controlValue2;
	},
};

GhostAnimationDefinition duckingAnimation {
	"ducking",
	[](const GhostEntity *ghost, float dt, GhostAnimationState &state) {
		state.controlValue1 = (64.0f - ghost->data.view_offset) / 36.0f;
	},
	[](const GhostAnimationState &state, GhostBendyModel::VertexInfo vertexInfo, Vector &animatedVertex) {
		if (vertexInfo.position.z < 40.0f) {
			animatedVertex.z *= 0.5f + 0.5f * (1.0f - state.controlValue1);
		} else {
			animatedVertex.z -= 20.0f * state.controlValue1;
		}
	}
};

GhostAnimationDefinition hideHandsAnimation{
	"hide_hands",
	nullptr,
	[](const GhostAnimationState &state, GhostBendyModel::VertexInfo vertexInfo, Vector &animatedVertex) {
		if (vertexInfo.group == GhostBendyModel::LEFT_ARM || vertexInfo.group == GhostBendyModel::RIGHT_ARM) {
			animatedVertex.y = 0;
		}
	},
};

GhostAnimationDefinition pitchRotationAnimation {
	"pitch_rotation",
	[](const GhostEntity *ghost, float dt, GhostAnimationState &state) {
		state.controlValue1 = ghost->data.view_angle.x;
	},
	[](const GhostAnimationState &state, GhostBendyModel::VertexInfo vertexInfo, Vector &animatedVertex) {
		float floppyForce = powf(vertexInfo.position.z / 72.0f, 3.5f);
		float floppyFactor = sinf(DEG2RAD(state.controlValue1 * 0.5f));

		animatedVertex.x += sinf(floppyFactor) * floppyForce * 48.0f;
		animatedVertex.z += (cosf(floppyFactor) - 1.0f) * floppyForce * 72.0f;
	}
};

GhostAnimationDefinition squishAnimation {
	"squish",
	[](const GhostEntity *ghost, float dt, GhostAnimationState &state) {
		bool oldGroundedState = state.controlValue1 > 0.0f;
		if (oldGroundedState != ghost->data.grounded) {
			if (ghost->velocity.z > 50.0f) {
				state.controlValue2 = 1.5f;
			} else if (!oldGroundedState) {
				state.controlValue2 = 0.5f;
			}
			state.controlValue1 = ghost->data.grounded ? 1.0f : 0.0f;
		}

		state.controlValue2 = fmaxf(0.0f, state.controlValue2 - 8.0f * dt);
	},
	[](const GhostAnimationState &state, GhostBendyModel::VertexInfo vertexInfo, Vector &animatedVertex) {
		animatedVertex.z *= 1.0f - state.controlValue2 * 0.2f;
		if (state.controlValue1 == 0.0f) {
			animatedVertex.z += 36.0f * state.controlValue2 * 0.2f;
		}
	}
};

GhostAnimationDefinition flipAnimation{
	"flip_haha",
	[](const GhostEntity *ghost, float dt, GhostAnimationState &state) {
		bool shouldFlip = (ghost->name == "Dinnerbone" || ghost->name == "Grumm");
		state.controlValue1 = shouldFlip ? 1.0f : 0.0f;
	},
	[](const GhostAnimationState &state, GhostBendyModel::VertexInfo vertexInfo, Vector &animatedVertex) {
		if (state.controlValue1 != 0.0f) {
			animatedVertex.z = 72.0f - animatedVertex.z;
		}
	},
};

GhostAnimationDefinition waveAnimation{
	"wave",
	[](const GhostEntity *ghost, float dt, GhostAnimationState &state) {
		if (ghost->velocity.Length() > 20.0f && state.controlValue2 > 1.0f) {
			state.controlValue1 = 1.0f;
		}

		state.controlValue2 += dt * 14.0f;
		if (state.controlValue2 > M_PI * 4 && state.controlValue1 == 0.0f) {
			state.controlValue2 = fmodf(state.controlValue2, M_PI * 2) + M_PI * 2;
		}

		if (state.controlValue2 > M_PI * 6 && state.controlValue1 == 1.0f) {
			state.active = false;
		}

		state.controlValue3 = ghost->data.view_offset;
	},
	[](const GhostAnimationState &state, GhostBendyModel::VertexInfo vertexInfo, Vector &animatedVertex) {
		float blend = state.controlValue1 == 0.0f
			? state.controlValue2 / (M_PI * 2)
			: (M_PI * 6 - state.controlValue2) / (M_PI * 2);
		blend = fminf(fmaxf(blend, 0.0f), 1.0f);

		if (vertexInfo.group != GhostBendyModel::RIGHT_ARM) {
			return;
		}

		if (state.controlValue3 < 48.0f) {
			return; // FIXME: crouching looks broken, for now just kill it
		}

		animatedVertex.y = vertexInfo.position.y;

		float rotationRadians = DEG2RAD(-100.0f + 30.0f * sinf(state.controlValue2));
		Vector armPivot = {0, -6.0f, state.controlValue3 - 24.0f};
		float cosRotation = cosf(rotationRadians);
		float sinRotation = sinf(rotationRadians);

		Vector originalVertex = animatedVertex;
		Vector rotatedVector = originalVertex - armPivot;
		rotatedVector = {
			rotatedVector.x,
			rotatedVector.y * cosRotation - rotatedVector.z * sinRotation,
			rotatedVector.y * sinRotation + rotatedVector.z * cosRotation,
		};
		rotatedVector *= 1.2f;
		rotatedVector += armPivot;

		Math::Lerp(originalVertex, rotatedVector, blend, animatedVertex);
	},
};

std::vector<GhostAnimationDefinition> g_ghostMainAnimationDefinitions = {
	hideHandsAnimation,
	pitchRotationAnimation,
	walkingAnimation,
	duckingAnimation,
	squishAnimation,
	flipAnimation,
};


std::vector<GhostAnimationDefinition> g_ghostTauntAnimationDefinitions = {
	waveAnimation,
};
