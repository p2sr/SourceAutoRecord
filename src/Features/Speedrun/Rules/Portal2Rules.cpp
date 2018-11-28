#include "Portal2Rules.hpp"

#include <vector>

#include "Features/Speedrun/TimerCategory.hpp"
#include "Features/Speedrun/TimerRule.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#ifdef _WIN32
#define Offset_m_iTouchingPortalCount 1124
#else
#define Offset_m_iTouchingPortalCount 1148
#endif

SAR_RULE(view_change, sp_a1_intro1, player, CBasePlayer, m_hViewEntity, SearchMode::Classes)
{
    // Wait some ticks till camera_intro gets enabled
    if (engine->GetSessionTick() > 13 && *m_hViewEntity == -1) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(moon_shot, sp_a4_finale4, moon_portal_detector, SearchMode::Names)
{
    // CFuncPortalDetector aka point_viewcontrol
    auto m_iTouchingPortalCount = reinterpret_cast<int*>((uintptr_t)moon_portal_detector + Offset_m_iTouchingPortalCount);

    if (m_iTouchingPortalCount && *m_iTouchingPortalCount != 0) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_CATEGORY(Portal2, RTA, std::vector<TimerRule*>({ &view_change, &moon_shot }));
