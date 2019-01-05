#include "WorkshopList.hpp"

#include <cstring>
#include <experimental/filesystem>
#include <stdlib.h>

#include "Modules/Engine.hpp"

#include "Command.hpp"
#include "Utils.hpp"

WorkshopList* workshop;

WorkshopList::WorkshopList()
    : maps()
    , path(std::string(engine->GetGameDirectory()) + std::string("/maps/workshop"))
{
    this->hasLoaded = true;
}
int WorkshopList::Update()
{
    auto before = this->maps.size();
    this->maps.clear();

    auto path = this->path;
    auto index = path.length() + 1;

    // Scan through all directories and find the map file
    for (auto& dir : std::experimental::filesystem::recursive_directory_iterator(path)) {
        if (dir.status().type() == std::experimental::filesystem::file_type::directory) {
            auto curdir = dir.path().string();
            for (auto& dirdir : std::experimental::filesystem::directory_iterator(curdir)) {
                auto file = dirdir.path().string();
                if (Utils::EndsWith(file, std::string(".bsp"))) {
                    auto map = file.substr(index);
                    map = map.substr(0, map.length() - 4);
                    this->maps.push_back(map);
                    break;
                }
            }
        }
    }

    return std::abs((int)before - (int)this->maps.size());
}

// Completion Function

int sar_workshop_CompletionFunc(const char* partial,
    char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    const char* cmd = "sar_workshop ";
    char* match = (char*)partial;
    if (std::strstr(partial, cmd) == partial) {
        match = match + std::strlen(cmd);
    }

    if (workshop->maps.empty()) {
        workshop->Update();
    }

    // Filter items
    static auto items = std::vector<std::string>(COMMAND_COMPLETION_MAXITEMS);
    items.clear();
    for (auto& map : workshop->maps) {
        if (items.size() == COMMAND_COMPLETION_MAXITEMS) {
            break;
        }

        if (std::strlen(match) != std::strlen(cmd)) {
            if (std::strstr(map.c_str(), match)) {
                items.push_back(map);
            }
        } else {
            items.push_back(map);
        }
    }

    // Copy items into list buffer
    auto count = 0;
    for (auto& item : items) {
        std::strcpy(commands[count++], (std::string(cmd) + item).c_str());
    }

    return count;
}

// Commands

CON_COMMAND_F_COMPLETION(sar_workshop, "Same as \"map\" command but lists workshop maps.\n", 0,
    sar_workshop_CompletionFunc)
{
    if (args.ArgC() < 2) {
        return console->Print("sar_workshop <file> : Same as \"map\" command but lists workshop maps.\n");
    }

    engine->ExecuteCommand((std::string("map workshop/") + std::string(args[1])).c_str());
}
CON_COMMAND(sar_workshop_update, "Updates the workshop map list.\n")
{
    console->Print("Added or removed %i map(s) to or from the list.\n", workshop->Update());
}
CON_COMMAND(sar_workshop_list, "Prints all workshop maps.\n")
{
    if (workshop->maps.empty()) {
        workshop->Update();
    }

    for (const auto& map : workshop->maps) {
        console->Print("%s\n", map.c_str());
    }
}
