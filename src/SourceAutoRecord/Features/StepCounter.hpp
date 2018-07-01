#pragma once
#include "Stats.hpp"

#include "Utils.hpp"

#define FL_DUCKING (1 << 1)

#define WL_Waist 2

namespace StepCounter {

// Static version of CBasePlayer::m_flStepSoundTime to keep track
// of a step globally and to not mess up sounds etc.
float StepSoundTime;

// Calculate when to play next step sound
void ReduceTimer(float frametime)
{
    if (StepSoundTime > 0) {
        StepSoundTime -= 1000.0f * frametime;
        if (StepSoundTime < 0) {
            StepSoundTime = 0;
        }
    }
}
// TODO: hl2?
void Increment(int m_fFlags, Vector m_vecVelocity, int m_nWaterLevel)
{
    // Adjust next step
    auto velrun = (m_fFlags & FL_DUCKING) ? 80 : 220;
    auto bWalking = m_vecVelocity.Length() < velrun;

    if (m_nWaterLevel == WL_Waist) {
        StepSoundTime = 600;
    } else {
        StepSoundTime = (bWalking) ? 400.0f : 300.0f;
    }

    if (m_fFlags & FL_DUCKING) {
        StepSoundTime += 100;
    }

    Stats::Steps::Total++;
}
void ResetTimer()
{
    StepSoundTime = 0;
}
}