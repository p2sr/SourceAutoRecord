#pragma once
#include <experimental/filesystem>
#include <stdlib.h>

#include "Modules/Engine.hpp"

#include "Utils.hpp"

class WorkshopList : public Feature {
public:
    std::vector<std::string> maps;

public:
    WorkshopList();
    std::string Path();
    int Update();
};

WorkshopList::WorkshopList()
    : maps()
{
}
std::string WorkshopList::Path()
{
    return std::string(Engine::GetGameDirectory()) + std::string("/maps/workshop");
}
int WorkshopList::Update()
{
    auto before = this->maps.size();
    this->maps.clear();

    auto path = this->Path();
    auto index = path.length() + 1;

    // Scan through all directories and find the map file
    for (auto& dir : std::experimental::filesystem::recursive_directory_iterator(path)) {
        if (dir.status().type() == std::experimental::filesystem::file_type::directory) {
            auto curdir = dir.path().string();
            for (auto& dirdir : std::experimental::filesystem::directory_iterator(curdir)) {
                auto file = dirdir.path().string();
                if (endsWith(file, std::string(".bsp"))) {
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

WorkshopList* workshop;
extern WorkshopList* workshop;
