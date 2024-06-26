#pragma once

#include "Feature.hpp"

#include <string>

class Timeline : public Feature {
public:
	Timeline();
	void StartSpeedrun();
	void Split(std::string name, std::string time);
};

extern Timeline *timeline;
