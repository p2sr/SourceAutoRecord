#pragma once
#include "Feature.hpp"

struct Vector;

class StepCounter : public Feature {
public:
	// Global version of CBasePlayer::m_flStepSoundTime
	float stepSoundTime;

public:
	StepCounter();
	void ReduceTimer(float frametime);
	void Increment(int m_fFlags, int m_MoveType, Vector m_vecVelocity, int m_nWaterLevel);
	void ResetTimer();
};

extern StepCounter *stepCounter;
