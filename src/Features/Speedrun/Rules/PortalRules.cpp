#include "PortalRules.hpp"

#include <cstdint>

#include "Features/Speedrun/TimerCategory.hpp"
#include "Features/Speedrun/TimerRule.hpp"

#include "Games/Windows/PortalUnpack.hpp"

#include "Modules/Engine.hpp"

#include "Utils/SDK.hpp"
#include "SAR.hpp"

#ifdef _WIN32
#define testchmb_a_00 "testchmb_a_00"
#define escape_02 "escape_02"
#define Offset_m_state 872
#define Offset_m_iDisabled 828
#else
#define testchmb_a_00 "maps/testchmb_a_00.bsp"
#define escape_02 "maps/escape_02.bsp"
#define Offset_m_state 892
#define Offset_m_iDisabled 848
#endif

#define Offset_m_state_Unpack 816
#define Offset_m_iDisabled_Unpack 772

SAR_RULE3(waking_up, testchmb_a_00, "blackout_viewcontroller", SearchMode::Names)
{
#ifdef _WIN32
    static auto isUnpack = typeid(*sar.game) == typeid(PortalUnpack);
#else
    static auto isUnpack = false;
#endif

    // CTriggerCamera aka point_viewcontrol    
    auto m_state = reinterpret_cast<int*>((uintptr_t)entity + (isUnpack ? Offset_m_state_Unpack : Offset_m_state));

    // Give it some time to get enabled...
    if (engine->GetSessionTick() > 10 && *m_state == USE_OFF) {
        return TimerAction::Start;
    }

    return TimerAction::DoNothing;
}

SAR_RULE3(glados_beaten, escape_02, "player_clip_glados", SearchMode::Names)
{
#ifdef _WIN32
    static auto isUnpack = typeid(*sar.game) == typeid(PortalUnpack);
#else
    static auto isUnpack = false;
#endif

    // CFuncBrush aka func_brush
    auto m_iDisabled = reinterpret_cast<int*>((uintptr_t)entity + (isUnpack ? Offset_m_iDisabled_Unpack : Offset_m_iDisabled));

    if (*m_iDisabled) {
        return TimerAction::End;
    }

    return TimerAction::DoNothing;
}

SAR_CATEGORY(Portal, RTA, _Rules({ &waking_up, &glados_beaten }));
