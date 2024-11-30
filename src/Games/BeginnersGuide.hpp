#pragma once
#include "Portal2.hpp"

class BeginnersGuide : public Portal2 {
public:
	BeginnersGuide();
	void LoadOffsets() override;
	const char *Version() override;

	static const char *ModDir();
};
