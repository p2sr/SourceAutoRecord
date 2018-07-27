#pragma once
#include "Stats.hpp"

#include "Utils.hpp"

#define FL_ONGROUND (1 << 0)
#define FL_DUCKING (1 << 1)
#define FL_FROZEN (1 << 5)
#define FL_ATCONTROLS (1 << 6)

#define WL_Feet 1
#define WL_Waist 2

#define MOVETYPE_LADDER 9
#define MOVETYPE_NOCLIP 8

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
// Adjust next step
void Increment(int m_fFlags, int m_MoveType, Vector m_vecVelocity, int m_nWaterLevel)
{
    // CBasePlayer::GetStepSoundVelocities
    auto velrun = (m_fFlags & FL_DUCKING || m_MoveType & MOVETYPE_LADDER) ? 80 : 220;

    // CBasePlayer::SetStepSoundTime
    auto bWalking = m_vecVelocity.Length() < velrun;
    if (m_MoveType & MOVETYPE_LADDER) {
        StepSoundTime = 350;
    } else if (m_nWaterLevel == WL_Waist) {
        // The engine skips this every 4th tick but this will be ignored here
        StepSoundTime = 600;
    } else {
        StepSoundTime = (bWalking) ? 400.0f : 300.0f;
    }

    if (m_fFlags & FL_DUCKING || m_MoveType & MOVETYPE_LADDER) {
        StepSoundTime += 100;
    }

    ++Stats::Steps::Total;
}
void ResetTimer()
{
    StepSoundTime = 0;
}
}
