#pragma once
#include "Portal2.hpp"

class PortalReloaded : public Portal2 {
public:
	PortalReloaded();
	void LoadOffsets() override;
	const char *Version() override;

	static const char *ModDir();
};
