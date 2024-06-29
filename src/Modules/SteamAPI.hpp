#pragma once

#include "Module.hpp"
#include "Utils.hpp"

class SteamAPI : public Module {
public:
	ISteamTimeline *g_timeline = nullptr;

public:
	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("steam_api"); }
};

extern SteamAPI *steam;
