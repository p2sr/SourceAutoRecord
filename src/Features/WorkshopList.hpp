#pragma once
#include <filesystem>
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

    // Scan through all directories and find the map file
    for (auto& dir : std::experimental::filesystem::recursive_directory_iterator(this->Path())) {
        if (dir.status().type() == std::experimental::filesystem::file_type::directory) {
            auto path = dir.path().string();
            for (auto& dirdir : std::experimental::filesystem::directory_iterator(path)) {
                auto file = dirdir.path().string();
                if (endsWith(file, std::string(".bsp"))) {
                    this->maps.push_back(file);
                    break;
                }
            }
        }
    }

    return std::abs((int)before - (int)this->maps.size());
}

WorkshopList* workshop;
extern WorkshopList* workshop;
