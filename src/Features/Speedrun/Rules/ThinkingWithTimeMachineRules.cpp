#include "ThinkingWithTimeMachineRules.hpp"

#include <cstdint>

#include "Features/Speedrun/TimerCategory.hpp"
#include "Features/Speedrun/TimerRule.hpp"

#ifdef _WIN32
#define Offset_m_iGoalSequence 1604
#define Offset_m_iDisabled 864
#else
#define Offset_m_iGoalSequence 1628
#define Offset_m_iDisabled 888
#endif

SAR_RULE3(movie_ends, "tm_intro_01", "wall_fall", SearchMode::Names)
{
    // CDynamicProp aka prop_dynamic_override
    auto m_iGoalSequence = reinterpret_cast<int*>((uintptr_t)entity + Offset_m_iGoalSequence);

    if (*m_iGoalSequence == 2) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(entered_funnel, "tm_map_final", "player_br", SearchMode::Names)
{
    // CFuncBrush aka func_brush
    auto m_iDisabled = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_iDisabled);

    if (!*m_iDisabled) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_CATEGORY(ThinkingWithTimeMachine, RTA, _Rules({ &movie_ends, &entered_funnel }));
