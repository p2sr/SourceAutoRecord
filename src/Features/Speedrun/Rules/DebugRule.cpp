#include "DebugRule.hpp"
#ifdef DBG_RULE
#include <cstdint>

#include "Features/EntityList.hpp"
#include "Features/Speedrun/TimerRule.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"

#include "Command.hpp"
#include "Variable.hpp"

Variable dbg_print("dbg_print", "0", 0, "\n");
Variable dbg_offset("dbg_offset", "0", 0, "\n");
Variable dbg_type("dbg_type", "0", 0, "\n");
Variable dbg_ent("dbg_ent", "player", "\n", 0);

void* globalEnt = nullptr;
bool entChecked = false;

CON_COMMAND(dbg_recheck, "\n")
{
    entChecked = false;
}

SAR_RULE0(debug_rule)
{
    if (!entChecked) {
        auto ent = dbg_ent.GetString();
        auto info = entityList->GetEntityInfoByName(ent);

        if (info) {
            globalEnt = info->m_pEntity;
            console->Print("Found %s!\n", ent);
        } else {
            console->Print("Failed to find %s!\n", ent);
        }

        entChecked = true;
    }

    if (globalEnt && dbg_print.GetBool()) {
        auto tick = engine->GetSessionTick();
        auto ent = dbg_ent.GetString();
        auto offset = dbg_offset.GetInt();

        if (dbg_type.GetBool()) {
            console->Print("[%i] %s+%i = %b\n", tick, ent, offset, *reinterpret_cast<bool*>((uintptr_t)globalEnt + offset));
        } else {
            console->Print("[%i] %s+%i = %i\n", tick, ent, offset, *reinterpret_cast<int*>((uintptr_t)globalEnt + offset));
        }
    }

    return TimerAction::DoNothing;
}
#endif
