#include "Portal2Rules.hpp"

#include <vector>

#include "Features/Speedrun/TimerCategory.hpp"
#include "Features/Speedrun/TimerRule.hpp"

#include "Modules/Engine.hpp"

#ifdef _WIN32
#define Offset_m_bDisabled 980
#define Offset_m_bHitMax 869
#else
#define Offset_m_bDisabled 1004
#define Offset_m_bHitMax 893
#endif

SAR_RULE3(out_of_shower, "gg_intro_wakeup", "tele_out_shower", SearchMode::Names)
{
    // CTriggerTeleport aka trigger_teleport
    auto m_bDisabled = reinterpret_cast<int*>((uintptr_t)entity + Offset_m_bDisabled);

    if (!*m_bDisabled) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(end_credits, "gg_stage_theend", "die_ending_math_final", SearchMode::Names)
{
    // CMathCounter aka math_counter
    auto m_bHitMax  = reinterpret_cast<int*>((uintptr_t)entity + Offset_m_bHitMax);
    if (m_bHitMax) console->Print(": %i\n", *m_bHitMax);

    if (*m_bHitMax == 1) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(end_credits2, "gg_stage_theend", "ele_exit_door", SearchMode::Names)
{
    // CRotDoor aka func_door_rotating
    auto idk  = reinterpret_cast<int*>((uintptr_t)entity + 884);
    if (idk) console->Print(": %i\n", *idk);

    if (*idk == 3) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_CATEGORY(ApertureTag, RTA, std::vector<TimerRule*>({ &out_of_shower, &end_credits/* , &end_credits2 */ }));

/*
    (112.53) input incinarator_path_die_2: die_ending_math_final.Add(1)
    (112.53) output: (math_counter,die_ending_math_final) -> (credits_video,PlayMovie)()
    (112.53) output: (math_counter,die_ending_math_final) -> (atw_c_note,Volume)(0)
    (112.53) output: (math_counter,die_ending_math_final) -> (atw_guitar_loop_2,StopSound)()
    (112.53) output: (math_counter,die_ending_math_final) -> (atw_death_ride,Volume)(0)
    (112.53) input die_ending_math_final: credits_video.PlayMovie()
    (112.53) input die_ending_math_final: atw_c_note.Volume(0)
    (112.53) input die_ending_math_final: atw_guitar_loop_2.StopSound()
    (112.53) input die_ending_math_final: atw_death_ride.Volume(0)
*/

/*
    (116.75) output: (trigger_once,death_clip_brush) -> (@gelgun,Kill)()
    (116.75) output: (trigger_once,death_clip_brush) -> (die_ending_math_final,Add,4.5)(1)
    (116.75) output: (trigger_once,death_clip_brush) -> (red_fade,Fade)()
    (116.75) output: (trigger_once,death_clip_brush) -> (death_clip_brush,Enable)()
    unhandled input: (Kill) -> (@gelgun), from (trigger_once,death_clip_brush); target entity not found
    (116.77) input death_clip_brush: red_fade.Fade()
    (116.77) input death_clip_brush: death_clip_brush.Enable()
    (116.77) input death_clip_brush: death_clip_brush.Enable()
    (116.93) input spherebot_train_1_chassis_1: spherebot_train_1_path_start115.InPass()
    (116.93) output: (path_track,spherebot_train_1_path_start115) -> (stop_nigel_on tracks,Trigger)()
*/

/*
    (20.82) output: (trigger_once,) -> (elevator_proj,TurnOff)()
    (20.82) output: (trigger_once,) -> (shake_particle,Start,5.0)()
    (20.82) output: (trigger_once,) -> (end_truman_relay,Trigger,6.0)()
    (20.82) output: (trigger_once,) -> (shake_ent,StartShake,5.0)()
    (20.82) output: (trigger_once,) -> (shake_sound,PlaySound,5.0)()
    (20.82) output: (trigger_once,) -> (ele_exit_door,Close)()

    [12] 0x2098ca00 -> env_projectedtexture
    [548] 0x2093a700 -> info_particle_system
    [2260] 0x209a8800 -> logic_relay
    [545] 0x20a94400 -> env_shake
    [546] 0x20a94800 -> ambient_generic
    [587] 0x20953700 -> func_door_rotating
*/
