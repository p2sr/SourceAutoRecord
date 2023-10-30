#pragma once
#include "Module.hpp"
#include "Utils.hpp"

class Matchmaking : public Module {
public:
	// PlayerLocal::UpdateLeaderboardData
	DECL_DETOUR(UpdateLeaderboardData, KeyValues *a2);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("matchmaking"); }
};

extern Matchmaking *matchmaking;
