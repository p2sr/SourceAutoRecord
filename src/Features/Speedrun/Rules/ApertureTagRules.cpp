#include "ApertureTagRules.hpp"

#include <cstdint>

#include "Features/Speedrun/TimerCategory.hpp"
#include "Features/Speedrun/TimerRule.hpp"

#include "Utils/SDK.hpp"

#ifdef _WIN32
#define Offset_m_bDisabled 980
#define Offset_m_bHitMax 869
#define Offset_m_toggle_state 860
#else
#define Offset_m_bDisabled 1004
#define Offset_m_bHitMax 893
#define Offset_m_toggle_state 884
#endif

SAR_RULE3(out_of_shower, "gg_intro_wakeup", "tele_out_shower", SearchMode::Names)
{
    // CTriggerTeleport aka trigger_teleport
    auto m_bDisabled = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bDisabled);

    if (!*m_bDisabled) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(end_credits, "gg_stage_theend", "die_ending_math_final", SearchMode::Names)
{
    // CMathCounter aka math_counter
    auto m_bHitMax = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bHitMax);

    if (*m_bHitMax) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(door_closes, "gg_stage_theend", "ele_exit_door", SearchMode::Names)
{
    // CRotDoor aka func_door_rotating
    auto m_toggle_state = reinterpret_cast<int*>((uintptr_t)entity + Offset_m_toggle_state);

    if (*m_toggle_state == TS_GOING_DOWN) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_CATEGORY(ApertureTag, RTA, _Rules({ &out_of_shower, &end_credits, &door_closes }));
