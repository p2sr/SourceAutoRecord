#pragma once
#include "Portal2.hpp"

class ThinkingWithTimeMachine : public Portal2 {
public:
	ThinkingWithTimeMachine();
	void LoadOffsets() override;
	const char *Version() override;

	static const char *GameDir();
};
