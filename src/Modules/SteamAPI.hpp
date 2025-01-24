#pragma once

#include "Module.hpp"
#include "Utils.hpp"

class SteamAPI : public Module {
public:
	ISteamTimeline *g_timeline = nullptr;

	using _SteamFriends = void *(*)();
	using _ActivateGameOverlayToWebPage = void (*)(void *self, const char *pchURL, int eMode);

	_SteamFriends SteamFriends = nullptr;
	_ActivateGameOverlayToWebPage ActivateGameOverlayToWebPage = nullptr;

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
