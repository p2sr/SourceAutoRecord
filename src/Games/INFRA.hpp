#pragma once
#include "Game.hpp"

class INFRA : public Game {
public:
	INFRA();
	void LoadOffsets() override;
	const char *Version() override;
	const float Tickrate() override;

	static const char *ModDir();
};
