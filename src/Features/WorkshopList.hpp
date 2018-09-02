#pragma once
#include "Feature.hpp"

#include <string>
#include <vector>

#include "Command.hpp"

class WorkshopList : public Feature {
public:
    std::vector<std::string> maps;

public:
    WorkshopList();
    std::string Path();
    int Update();
};

extern WorkshopList* workshop;

extern Command sar_workshop;
extern Command sar_workshop_update;
extern Command sar_workshop_list;
