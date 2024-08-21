#pragma once

#include "Feature.hpp"

#define MAX_GROUNDFRAMES_TRACK 10

class GroundFramesCounter : public Feature {
public:
	static inline int counter[2] = {0};
	static inline bool grounded[2] = {false};
	static inline int totals[2][MAX_GROUNDFRAMES_TRACK] = {0};

public:
	GroundFramesCounter();
	static void HandleMovementFrame(int slot, bool grounded);

private:
	static void AddToTotal(int slot, int count);
};

extern GroundFramesCounter *groundFramesCounter;
