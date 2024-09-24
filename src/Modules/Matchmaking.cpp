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
#ifdef _WIN32
		Matchmaking::UpdateLeaderboardData = (decltype(Matchmaking::UpdateLeaderboardData))Memory::Scan(matchmaking->Name(), "55 8B EC 83 EC 08 53 8B D9 8B 03 8B 50 08");
#else
		if (sar.game->Is(SourceGame_PortalReloaded) || sar.game->Is(SourceGame_PortalStoriesMel)) {
			Matchmaking::UpdateLeaderboardData = (decltype(Matchmaking::UpdateLeaderboardData))Memory::Scan(matchmaking->Name(), "55 89 E5 57 56 53 83 EC 2C 8B 45 08 8B 5D 0C");
		} else {
			Matchmaking::UpdateLeaderboardData = (decltype(Matchmaking::UpdateLeaderboardData))Memory::Scan(matchmaking->Name(), "55 89 E5 57 56 53 83 EC 3C 8B 55 ? 8B 02 89 14 24 FF 50 ? C7 04 24");
		}
#endif

		g_UpdateLeaderboardDataHook.SetFunc(Matchmaking::UpdateLeaderboardData);
	}

	return this->hasLoaded;
}
void Matchmaking::Shutdown() {
}

Matchmaking *matchmaking;
