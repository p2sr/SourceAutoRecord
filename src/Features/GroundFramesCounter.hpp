#pragma once

#include "Feature.hpp"

#define MAX_GROUNDFRAMES_TRACK 10

class GroundFramesCounter : public Feature {
public:
	int counter[2] = {0};
	bool grounded[2] = {false};
	int totals[2][MAX_GROUNDFRAMES_TRACK] = {0};

public:
	GroundFramesCounter();
	void HandleMovementFrame(int slot, bool grounded);

private:
	void AddToTotal(int slot, int count);
};

extern GroundFramesCounter *groundFramesCounter;
