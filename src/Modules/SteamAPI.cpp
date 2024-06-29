#include "SteamAPI.hpp"

bool SteamAPI::Init() {
	const auto steam_api = Memory::GetModuleHandleByName(this->Name());
	if (!steam_api)
		return false;

	const auto GetHSteamUser = Memory::GetSymbolAddress<void *(*)()>(steam_api, "SteamAPI_GetHSteamUser");
	if (!GetHSteamUser)
		return false;

	const auto FindOrCreateUserInterface = Memory::GetSymbolAddress<void *(*)(void *, const char *)>(steam_api, "SteamInternal_FindOrCreateUserInterface");
	if (FindOrCreateUserInterface) {
		g_timeline = (ISteamTimeline *)FindOrCreateUserInterface(GetHSteamUser(), "STEAMTIMELINE_INTERFACE_V001");
	} else {
		/* FindOrCreateUserInterface remade */
		const auto GetHSteamPipe = Memory::GetSymbolAddress<void *(*)()>(steam_api, "SteamAPI_GetHSteamPipe");
		if (!GetHSteamPipe)
			return false;

		void *interfaceMgr = **reinterpret_cast<void ***>(Memory::Scan(this->Name(), "89 0D ? ? ? ? 85 C9 0F", 2));
		const auto fn = *(ISteamTimeline * (__rescall **)(void *, void *, void *, const char *))(*(uintptr_t *)interfaceMgr + 48);
		g_timeline = fn(interfaceMgr, GetHSteamUser(), GetHSteamPipe(), "STEAMTIMELINE_INTERFACE_V001");
	}
	if (!g_timeline)
		return false;

	g_timeline->SetTimelineGameMode(k_ETimelineGameMode_Menus);

	return this->hasLoaded = this->g_timeline;
}
void SteamAPI::Shutdown() {
}

SteamAPI *steam;
