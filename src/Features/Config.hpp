#pragma once
#include "Feature.hpp"

#include <string>

class Config : public Feature {
public:
	std::string filePath;

public:
	Config();
	bool Save();
	bool Load();
};

extern Config *config;
