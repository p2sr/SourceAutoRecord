#pragma once
#include "Portal2.hpp"

class ApertureTag : public Portal2 {
public:
	ApertureTag();
	void LoadOffsets() override;
	const char *Version() override;

	static const char *GameDir();
};
