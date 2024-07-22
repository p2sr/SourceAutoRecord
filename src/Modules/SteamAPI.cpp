#include "SteamAPI.hpp"

#include "SAR.hpp"

bool SteamAPI::Init() {
	const auto steam_api = Memory::GetModuleHandleByName(this->Name());
	if (!steam_api)
		return false;

	// TODO: Mods support, this needs to be remade too :/
	this->SteamFriends = Memory::GetSymbolAddress<_SteamFriends>(steam_api, "SteamAPI_SteamFriends_v017");
	this->ActivateGameOverlayToWebPage = Memory::GetSymbolAddress<_ActivateGameOverlayToWebPage>(steam_api, "SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage");

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

#ifdef _WIN32
		auto scanResult = reinterpret_cast<void ***>(Memory::Scan(this->Name(), Offsets::interfaceMgrSig, Offsets::interfaceMgrOff));
		
		if (scanResult && *scanResult && **scanResult) {
			void *interfaceMgr = **scanResult;
			const auto fn = *(ISteamTimeline * (__rescall **)(void *, void *, void *, const char *))(*(uintptr_t *)interfaceMgr + 48);
			g_timeline = fn(interfaceMgr, GetHSteamUser(), GetHSteamPipe(), "STEAMTIMELINE_INTERFACE_V001");
		}
#else
		// TODO: Linux mods support
		return false;
#endif
	}
	if (!g_timeline)
		return false;

	g_timeline->SetTimelineGameMode(k_ETimelineGameMode_Menus);

	return this->hasLoaded = this->g_timeline;
}
void SteamAPI::Shutdown() {
}

SteamAPI *steam;
