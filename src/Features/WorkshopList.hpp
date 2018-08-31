#pragma once
#include "Feature.hpp"

#include <string>
#include <vector>

class WorkshopList : public Feature {
public:
    std::vector<std::string> maps;

public:
    WorkshopList();
    std::string Path();
    int Update();
};

extern WorkshopList* workshop;
