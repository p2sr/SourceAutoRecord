#include "Portal2Rules.hpp"

#include "Features/Speedrun/TimerRule.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

SAR_RULE(Portal2, any, sp_a1_intro1, player, CBasePlayer, m_hViewEntity)
{
    if (*m_hViewEntity == -1 && engine->GetSessionTick() > 100) {
        console->Print("[%i] %i\n", engine->GetSessionTick(), *m_hViewEntity);
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(Portal2, any, sp_a4_finale4, moon_portal_detector)
{
    auto m_phTouchingPortals = reinterpret_cast<int*>((uintptr_t)moon_portal_detector + 1140);
    //console->Print("[%i] %i\n", engine->GetSessionTick(), *m_phTouchingPortals);
    if (m_phTouchingPortals && *m_phTouchingPortals != -1) {
        console->Print("[%i] %i\n", engine->GetSessionTick(), *m_phTouchingPortals);
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

/* SAR_RULE2(Portal2, any, sp_a4_finale4, ending_vehicle, CPropVehicleChoreoGeneric, m_bEnterAnimOn)
{
    auto m_bPlayerCanShoot = reinterpret_cast<int*>((uintptr_t)ending_vehicle + (m_bEnterAnimOn_offset - 5));
    console->Print("[%i] %i\n", engine->GetSessionTick(), *m_bPlayerCanShoot);
    if (!*m_bPlayerCanShoot) {
        console->Warning("[%i] %i\n", engine->GetSessionTick(), *m_bPlayerCanShoot);
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
} */
