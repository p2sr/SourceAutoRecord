#pragma once
#include "Features/WorkshopList.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"

#include "Command.hpp"

#include "Utils.hpp"

// TSP only
void IN_BhopDown(const CCommand& args) { Client::KeyDown(Client::in_jump, (args.ArgC() > 1) ? args[1] : NULL); }
void IN_BhopUp(const CCommand& args) { Client::KeyUp(Client::in_jump, (args.ArgC() > 1) ? args[1] : NULL); }

Command startbhop("+bhop", IN_BhopDown, "Client sends a key-down event for the in_jump state.\n");
Command endbhop("-bhop", IN_BhopUp, "Client sends a key-up event for the in_jump state.\n");

CON_COMMAND(sar_anti_anti_cheat, "Sets sv_cheats to 1.\n")
{
    sv_cheats.ThisPtr()->m_nValue = 1;
}

// TSP & TBG only
DECLARE_AUTOCOMPLETION_FUNCTION(map, "maps", bsp);
DECLARE_AUTOCOMPLETION_FUNCTION(changelevel, "maps", bsp);
DECLARE_AUTOCOMPLETION_FUNCTION(changelevel2, "maps", bsp);

// P2 only
int sar_workshop_CompletionFunc(const char* partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    const char* cmd = "sar_workshop ";
    char* match = (char*)(partial + std::strlen(cmd));

    if (workshop->maps.size() == 0) {
        workshop->Update();
    }

    std::vector<std::string> items;
    auto index = workshop->Path().length() + 1;

    // Filter items
    for (auto& dir : workshop->maps) {
        auto file = dir.substr(index);
        file = file.substr(0, file.length() - 4);
        if (std::strlen(match) != 0) {
            if (std::strstr(file.c_str(), match)) {
                items.push_back(file);
            }
        } else {
            items.push_back(file);
        }

        if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
            break;
        }
    }

    // Copy items into list buffer
    auto count = 0;
    for (auto& item : items) {
        std::strcpy(commands[count++], (std::string(cmd) + item).c_str());
    }

    return count;
}

CON_COMMAND_F_COMPLETION(sar_workshop, "Same as \"map\" command but this can list workshop maps.\n", 0, sar_workshop_CompletionFunc)
{
    if (args.ArgC() < 2) {
        console->Print("sar_workshop <file> : Same as \"map\" command but this can list workshop maps.\n");
        return;
    }

    Engine::ExecuteCommand((std::string("map workshop/") + std::string(args[1])).c_str());
}

CON_COMMAND(sar_workshop_update, "Updates the workshop map list.\n")
{
    console->Print("Added or removed %i map(s) to or from the list.\n", workshop->Update());
}
