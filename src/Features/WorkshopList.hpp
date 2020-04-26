#pragma once
#include <string>
#include <vector>

#include "Feature.hpp"

#include "Command.hpp"

class WorkshopList : public Feature {
public:
    std::vector<std::string> maps;
    std::string path;

public:
    WorkshopList();
    std::string Cinematic();
    int Update();
};

extern WorkshopList* workshop;

extern Command sar_workshop;
extern Command sar_workshop_update;
extern Command sar_workshop_list;
