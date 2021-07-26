#include "StepCounter.hpp"

#include "Event.hpp"
#include "Utils/SDK.hpp"

StepCounter *stepCounter;

StepCounter::StepCounter()
	: stepSoundTime(0) {
	this->hasLoaded = true;
}
// Calculate when to play next step sound
void StepCounter::ReduceTimer(float frametime) {
	if (this->stepSoundTime > 0) {
		this->stepSoundTime -= 1000.0f * frametime;
		if (this->stepSoundTime < 0) {
			this->stepSoundTime = 0;
		}
	}
}
// Adjust next step
void StepCounter::Increment(int m_fFlags, int m_MoveType, Vector m_vecVelocity, int m_nWaterLevel) {
	// CBasePlayer::GetStepSoundVelocities
	auto velrun = (m_fFlags & FL_DUCKING || m_MoveType & MOVETYPE_LADDER) ? 80 : 220;

	// CBasePlayer::SetstepSoundTime
	auto bWalking = m_vecVelocity.Length() < velrun;
	if (m_MoveType & MOVETYPE_LADDER) {
		this->stepSoundTime = 350;
	} else if (m_nWaterLevel == WL_Waist) {
		// The engine skips this every 4th tick but this will be ignored here
		this->stepSoundTime = 600;
	} else {
		this->stepSoundTime = (bWalking) ? 400.0f : 300.0f;
	}

	if (m_fFlags & FL_DUCKING || m_MoveType & MOVETYPE_LADDER) {
		this->stepSoundTime += 100;
	}
}
void StepCounter::ResetTimer() {
	this->stepSoundTime = 0;
}

ON_EVENT(SESSION_START) {
	stepCounter->ResetTimer();
}
