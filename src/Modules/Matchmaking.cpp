#include "Matchmaking.hpp"

#include "Hook.hpp"
#include "SAR.hpp"

REDECL(Matchmaking::UpdateLeaderboardData);

extern Hook g_UpdateLeaderboardDataHook;
DETOUR(Matchmaking::UpdateLeaderboardData, KeyValues *a2) {
	return 0;
}
Hook g_UpdateLeaderboardDataHook(&Matchmaking::UpdateLeaderboardData_Hook);

bool Matchmaking::Init() {
	if (!sar.game->Is(SourceGame_Portal2)) {
		Matchmaking::UpdateLeaderboardData = (decltype(Matchmaking::UpdateLeaderboardData))Memory::Scan(this->Name(), Offsets::UpdateLeaderboardData);

		g_UpdateLeaderboardDataHook.SetFunc(Matchmaking::UpdateLeaderboardData);
	}

	return this->hasLoaded;
}
void Matchmaking::Shutdown() {
}

Matchmaking *matchmaking;
