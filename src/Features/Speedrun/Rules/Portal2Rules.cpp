#include "Portal2Rules.hpp"

#include "Features/Speedrun/TimerRule.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Console.hpp"

#ifndef _WIN32
#define CPropVehicleChoreoGeneric_SetCanShoot_Offset 2200
#else
#define CPropVehicleChoreoGeneric_SetCanShoot_Offset 2176
#endif

SAR_RULE(Portal2, any, sp_a1_intro1, player, CBasePlayer, m_hViewEntity)
{
    if (*m_hViewEntity == -1 && engine->GetSessionTick() > 100) {
        console->Print("[%i] %i\n", engine->GetSessionTick(), *m_hViewEntity);
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE2(Portal2, any, sp_a4_finale4, ending_vehicle)
{
    auto canShoot = *reinterpret_cast<int*>((uintptr_t)ending_vehicle + CPropVehicleChoreoGeneric_SetCanShoot_Offset);
    if (!canShoot) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}
