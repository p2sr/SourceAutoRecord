#pragma once

#include "Utils/SDK/GameMovement.hpp"

struct StepMoveData {
	bool wasDucking;

	Vector initialPosition;
	Vector initialVelocity;

	Vector afterHitPosition;
	Vector afterHitVelocity;

	Vector afterStepPosition;
	Vector afterStepVelocity;
};

namespace StepSlopeBoostDebug {
	void OnStartStepMove(CMoveData* currMoveData);
	void OnTryPlayerMoveEnd(CMoveData *currMoveData);
	void OnFinishStepMove(CMoveData *currMoveData);
}
