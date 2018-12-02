#include "Portal2Rules.hpp"

#include <vector>

#include "Features/Speedrun/TimerCategory.hpp"
#include "Features/Speedrun/TimerRule.hpp"

#include "Modules/Engine.hpp"

#ifdef _WIN32
#define Offset_m_bWaitForRefire 909
#define Offset_m_bDisabled 936
#else
#define Offset_m_bWaitForRefire 933
#define Offset_m_bDisabled 960
#endif

/*
    (12.10) input tramstart_relay: Subway_TankTrain.StartForward()
    TRAIN(Subway_TankTrain), speed to 115.00
    (12.13) input tramstart_relay: tramstart_relay.EnableRefire()
    (12.13) input <NULL>: intro_Soundscape.Enable()
    (12.13) input <NULL>: StartFade.FadeReverse()
    (12.13) input <NULL>: Intro_Viewcontroller.Disable()
*/

SAR_RULE3(tram_teleport, "sp_a1_tramride", "tramstart_relay", SearchMode::Names)
{
    // CLogicRelay aka logic_relay
    auto m_bWaitForRefire = reinterpret_cast<bool*>((uintptr_t)entity + Offset_m_bWaitForRefire);
    if (m_bWaitForRefire) console->Print("[%i] m_bWaitForRefire = %i\n", engine->GetSessionTick(), *m_bWaitForRefire);

    /* if (engine->GetSessionTick() > 10 && !*m_bWaitForRefire) {
        return TimerAction::Start;
    } */

    return TimerAction::DoNothing;
}
SAR_RULE3(tram_teleport2, "st_a1_tramride", "tramstart_relay", SearchMode::Names)
{
    // CLogicRelay aka logic_relay
    auto m_bWaitForRefire = reinterpret_cast<bool*>((uintptr_t)entity + 933);

    if (engine->GetSessionTick() > 10 && !*m_bWaitForRefire) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

/*
    (49.75) input : press_e_to_hire_us_valve.Use()
    (49.75) output: (func_button,press_e_to_hire_us_valve) -> (cs*,Cancel)()
    (49.75) output: (func_button,press_e_to_hire_us_valve) -> (cs*,Kill,0.0)()
    (49.75) output: (func_button,press_e_to_hire_us_valve) -> (soundscape,Kill,0.0)()
    (49.75) output: (func_button,press_e_to_hire_us_valve) -> (soundscape,Disable)()
    (49.75) output: (func_button,press_e_to_hire_us_valve) -> (end_teleport,Teleport)()
    (49.75) output: (func_button,press_e_to_hire_us_valve) -> (ending_speedmod,ModifySpeed)(0)
    (49.75) output: (func_button,press_e_to_hire_us_valve) -> (weaponstrip,StripWeaponsAndSuit)()
    (49.75) output: (func_button,press_e_to_hire_us_valve) -> (!self,Kill)()
*/

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

SAR_CATEGORY(PortalStoriesMel, RTA, std::vector<TimerRule*>({ &tram_teleport, &tram_teleport2, &aegis_shutdown, &aegis_shutdown2 }));
