#pragma once

#include "Feature.hpp"

class GroundFramesCounter : public Feature {
public:
	int counter[2] = {0};
	bool grounded[2] = {false};

public:
	GroundFramesCounter();
	void HandleMovementFrame(int slot, bool grounded);
};

extern GroundFramesCounter *groundFramesCounter;
