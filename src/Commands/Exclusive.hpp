#pragma once
#include "Command.hpp"

// TSP only
extern Command startbhop;
extern Command endbhop;
extern Command sar_anti_anti_cheat;

// TSP & TBG only
extern DECL_DECLARE_AUTOCOMPLETION_FUNCTION(map);
extern DECL_DECLARE_AUTOCOMPLETION_FUNCTION(changelevel);
extern DECL_DECLARE_AUTOCOMPLETION_FUNCTION(changelevel2);

// P2 only
extern Command sar_workshop;
extern Command sar_workshop_update;
extern Command sar_workshop_list;
extern Command sar_togglewait;
