#pragma once

#include "Module.hpp"
#include "Utils.hpp"

class SteamAPI : public Module {
public:
	ISteamTimeline *g_timeline = nullptr;

public:
	bool Init() override;
	void Shutdown() override;
	const char *Name() override {	
#ifdef _WIN32
		return MODULE("steam_api");
#else
		return MODULE("libsteam_api");
#endif
	}
};

extern SteamAPI *steam;
