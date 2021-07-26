#pragma once
#include "Portal2.hpp"

class PortalStoriesMel : public Portal2 {
public:
	PortalStoriesMel();
	void LoadOffsets() override;
	const char *Version() override;

	static const char *ModDir();
};
