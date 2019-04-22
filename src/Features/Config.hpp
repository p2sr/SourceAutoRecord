#pragma once
#include <string>

#include "Feature.hpp"

class Config : public Feature {
public:
    std::string filePath;

public:
    Config();
    bool Save();
    bool Load();
};

extern Config* config;
