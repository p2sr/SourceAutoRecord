#pragma once
#include "Game.hpp"

class Portal2_2011 : public Game {
public:
	Portal2_2011();
	void LoadOffsets() override;
	const char *Version() override;
	const float Tickrate() override;

	static const char *ModDir();
};
