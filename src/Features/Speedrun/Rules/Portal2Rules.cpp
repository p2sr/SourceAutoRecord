#include "Portal2Rules.hpp"

#include <cstdint>

#include "Features/Speedrun/TimerCategory.hpp"
#include "Features/Speedrun/TimerRule.hpp"

#include "Modules/Engine.hpp"

#ifdef _WIN32
#define Offset_m_iTouchingPortalCount 1124
#define Offset_m_bDisabled 980
#define Offset_m_bPlayerStateA 960
#define Offset_m_bPlayerStateB 961
#define Offset_m_flFOV 1736
#else
#define Offset_m_iTouchingPortalCount 1148
#define Offset_m_bDisabled 1004
#define Offset_m_bPlayerStateA 984
#define Offset_m_bPlayerStateB 985
#define Offset_m_flFOV 1760
#endif

SAR_RULE(view_change, "sp_a1_intro1", "player", "CBasePlayer", m_hViewEntity, SearchMode::Classes)
{
    // Wait some ticks till camera_intro gets enabled
    if (engine->GetSessionTick() > 13 && *m_hViewEntity == -1) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(moon_shot, "sp_a4_finale4", "moon_portal_detector", SearchMode::Names)
{
    // CFuncPortalDetector aka point_viewcontrol
    auto m_iTouchingPortalCount = reinterpret_cast<int*>((uintptr_t)entity + Offset_m_iTouchingPortalCount);

    if (*m_iTouchingPortalCount != 0) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(players_teleport, "mp_coop_start", "teleport_start", SearchMode::Names)
{
    // CTriggerTeleport aka trigger_teleport
    auto m_bDisabled = reinterpret_cast<int*>((uintptr_t)entity + Offset_m_bDisabled);

    if (!*m_bDisabled) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(players_taunt, "mp_coop_paint_longjump_intro", "vault-coopman_taunt", SearchMode::Names)
{
    // CLogicCoopManager aka logic_coop_manager
    auto m_bPlayerStateA = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bPlayerStateA);
    auto m_bPlayerStateB = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bPlayerStateB);

    if (*m_bPlayerStateA && *m_bPlayerStateB) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(gate_opens, "mp_coop_paint_crazy_box", "coopman_airlock_success", SearchMode::Names)
{
    // CLogicCoopManager aka logic_coop_manager
    auto m_bPlayerStateA = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bPlayerStateA);
    auto m_bPlayerStateB = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bPlayerStateB);

    if (*m_bPlayerStateA && *m_bPlayerStateB) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_RULE(gain_control, "e1912", "player", "CBasePlayer", m_hVehicle, SearchMode::Classes)
{
    // Wait some ticks till we get into the vehicle
    if (engine->GetSessionTick() > 13 && *m_hVehicle == -1) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(vehicle_lock, "e1912", "crash-vehicle_outro", SearchMode::Names)
{
    // CPropVehicleChoreoGeneric aka prop_vehicle_choreo_generic
    // m_savedVehicleView.flFOV
    auto flFOV = reinterpret_cast<float*>((uintptr_t)entity + Offset_m_flFOV);

    if (*flFOV != 0) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_CATEGORY(Portal2, SinglePlayer, _Rules({ &view_change, &moon_shot }));
SAR_CATEGORY(Portal2, Cooperative,  _Rules({ &players_teleport, &players_taunt, &gate_opens }));
SAR_CATEGORY(Portal2, Super8,       _Rules({ &gain_control, &vehicle_lock }));
