#pragma once
#include "Game.hpp"

class Portal2 : public Game {
public:
	Portal2();
	void LoadOffsets() override;
	const char *Version() override;
	const float Tickrate() override;

	static const char *ModDir();
};
