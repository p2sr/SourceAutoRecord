#pragma once
#include "Command.hpp"
#include "Variable.hpp"

class Cheats {
public:
    void Init();
    void Shutdown();
};

extern Variable sar_autorecord;
extern Variable sar_autojump;
extern Variable sar_jumpboost;
extern Variable sar_aircontrol;
extern Variable sar_disable_challenge_stats_hud;
extern Variable sar_debug_event_queue;

extern Command startbhop;
extern Command endbhop;
extern Command sar_anti_anti_cheat;
extern Command sar_togglewait;

extern DECL_DECLARE_AUTOCOMPLETION_FUNCTION(map);
extern DECL_DECLARE_AUTOCOMPLETION_FUNCTION(changelevel);
extern DECL_DECLARE_AUTOCOMPLETION_FUNCTION(changelevel2);
