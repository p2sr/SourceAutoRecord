#include "PortalStoriesMelRules.hpp"

#include <cstdint>

#include "Features/Speedrun/TimerCategory.hpp"
#include "Features/Speedrun/TimerRule.hpp"

#ifdef _WIN32
#define Offset_m_bWaitForRefire 909
#define Offset_m_bDisabled 936
#else
#define Offset_m_bWaitForRefire 933
#define Offset_m_bDisabled 960
#endif

SAR_RULE3(tram_teleport, "sp_a1_tramride", "tramstart_relay", SearchMode::Names)
{
    // CLogicRelay aka logic_relay
    auto m_bWaitForRefire = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bWaitForRefire);

    if (*m_bWaitForRefire) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}
SAR_RULE3(tram_teleport2, "st_a1_tramride", "tramstart_relay", SearchMode::Names)
{
    // CLogicRelay aka logic_relay
    auto m_bWaitForRefire = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bWaitForRefire);

    if (*m_bWaitForRefire) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(aegis_shutdown, "sp_a4_finale", "soundscape", SearchMode::Names)
{
    // CEnvSoundscape aka env_soundscape
    auto m_bDisabled = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bDisabled);

    if (*m_bDisabled) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}
SAR_RULE3(aegis_shutdown2, "st_a4_finale", "soundscape", SearchMode::Names)
{
    // CEnvSoundscape aka env_soundscape
    auto m_bDisabled = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bDisabled);

    if (*m_bDisabled) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_CATEGORY(PortalStoriesMel, RTA, _Rules({ &tram_teleport, &tram_teleport2, &aegis_shutdown, &aegis_shutdown2 }));
