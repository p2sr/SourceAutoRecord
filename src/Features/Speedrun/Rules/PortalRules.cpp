#include "PortalRules.hpp"

#include <cstdint>

#include "Features/Speedrun/TimerCategory.hpp"
#include "Features/Speedrun/TimerRule.hpp"

#include "Utils/SDK.hpp"

#ifdef _WIN32
#define Offset_m_state 868
#define Offset_m_iDisabled 822
#else
#define Offset_m_state 892
#define Offset_m_iDisabled 848
#endif

SAR_RULE3(waking_up, "maps/testchmb_a_00.bsp", "blackout_viewcontroller", SearchMode::Names)
{
    // CTriggerCamera aka point_viewcontrol
    auto m_state = reinterpret_cast<int*>((uintptr_t)entity + Offset_m_state);

    if (*m_state == USE_OFF) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(glados_beaten, "maps/escape_02.bsp", "player_clip_glados", SearchMode::Names)
{
    // CFuncBrush aka func_brush
    auto m_iDisabled = reinterpret_cast<int*>((uintptr_t)entity + Offset_m_iDisabled);

    if (*m_iDisabled) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_CATEGORY(Portal, RTA, _Rules({ &waking_up, &glados_beaten }));
