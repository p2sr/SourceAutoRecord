#include "Portal2Rules.hpp"

#include "Features/Speedrun/TimerRule.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#ifdef _WIN32
#define Offset_m_iTouchingPortalCount 1124
#else
#define Offset_m_iTouchingPortalCount 1148
#endif

SAR_RULE(Portal2, any, sp_a1_intro1, player, CBasePlayer, m_hViewEntity, SearchMode::Classes)
{
    // Wait some ticks till camera_intro gets enabled
    if (engine->GetSessionTick() > 13 && *m_hViewEntity == -1) {
        console->DevMsg("[%i] Portal 2 any rule start!\n", engine->GetSessionTick());
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(Portal2, any, sp_a4_finale4, moon_portal_detector, SearchMode::Names)
{
    // CFuncPortalDetector aka point_viewcontrol
    auto m_iTouchingPortalCount = reinterpret_cast<int*>((uintptr_t)moon_portal_detector + Offset_m_iTouchingPortalCount);

    if (m_iTouchingPortalCount && *m_iTouchingPortalCount != 0) {
        console->DevMsg("[%i] Portal 2 any rule end!\n", engine->GetSessionTick());
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}
