#pragma once
#include "Portal2.hpp"

class StanleyParable : public Portal2 {
public:
	StanleyParable();
	void LoadOffsets() override;
	const char *Version() override;

	static const char *ModDir();
};
