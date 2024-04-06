
#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Tas/TasPlayer.hpp"
#include "Session.hpp"
#include "Speedrun/SpeedrunTimer.hpp"
#include "Timer/PauseTimer.hpp"
#include "Renderer.hpp"

#include "discord-rpc/discord_register.h"
#include "discord-rpc/discord_rpc.h"

static Variable sar_discord_rpc_enabled("sar_discord_rpc_enabled", "0", 0, 1, "Enables Discord Rich Presence integration.\n");
static Variable sar_discord_rpc_app_id("sar_discord_rpc_app_id", "1084419823737524294", "Defines Discord Rich Presence's application ID.\n", 0);

static bool g_discordRpcInitialized;
static std::string g_activeTasScriptThisSession;
static int g_challengeModeAttempts;
static int64_t g_gameStartTimestamp;
static DiscordEventHandlers g_discordHandlers;

static void HandleDiscordReady(const DiscordUser *connectedUser) {
	console->Print("Discord: Connected to user %s#%s - %s\n", connectedUser->username, connectedUser->discriminator, connectedUser->userId);
}

static void HandleDiscordDisconnected(int errcode, const char *message) {
	console->Print("Discord: Disconnected (%d: %s)\n", errcode, message);
}

static void HandleDiscordError(int errcode, const char *message) {
	console->Print("Discord: Error (%d: %s)\n", errcode, message);
}

static void HandleDiscordJoin(const char *secret) {
	// Not implemented
}

static void HandleDiscordSpectate(const char *secret) {
	// Not implemented
}

static void HandleDiscordJoinRequest(const DiscordUser *request) {
	// Not implemented
}

static DiscordRichPresence rp;
std::string categoryName;
std::string cmAttemptsString;
std::string mapName;
static void UpdateDiscordRichPresence() {
	if (!g_discordRpcInitialized) return;


	rp.startTimestamp = g_gameStartTimestamp;
	
	categoryName = SpeedrunTimer::GetCategoryName();
	cmAttemptsString = Utils::ssprintf("Attempts: %d", g_challengeModeAttempts);

	if (!session->isRunning) {
	}
	else if (!engine->hoststate->m_activeGame) {
		// not in session, presumably in main menu
		rp.details = "Main Menu";
		rp.state = "";
		rp.largeImageKey = "menu";
		rp.largeImageText = "In menu";
	} else {
		// rendering
		if (Renderer::IsRunning()) {
			if (engine->demoplayer->IsPlaying()) {
				rp.details = "Rendering Demo";
				rp.state = engine->demoplayer->DemoName;
			} else if (g_activeTasScriptThisSession.length() > 0) {
				rp.details = "Rendering TAS";
				rp.state = g_activeTasScriptThisSession.c_str();
			} else {
				rp.details = "Rendering";
			}
		}
		// tas player
		else if (g_activeTasScriptThisSession.length() > 0) {
			rp.details = "TAS-ing";
			rp.state = g_activeTasScriptThisSession.c_str();
		}
		// demo player
		else if(engine->demoplayer->IsPlaying()) {
			rp.details = "Playing Demo";
			rp.state = engine->demoplayer->DemoName;
		}
		// challenge mode
		else if(sv_bonus_challenge.GetBool()) {
			rp.details = "Challenge Mode";
			rp.state = cmAttemptsString.c_str();
		} 
		// speedrun timer
		else if(SpeedrunTimer::IsRunning()) {
			rp.details = "Speedrunning";
			rp.state = categoryName.c_str();
		}
		// regular playback
		else {
			rp.details = "Playing";
		}

		// figure out current map for thumbnail
		mapName = engine->GetCurrentMapName();
		auto it = std::find_if(Game::maps.begin(), Game::maps.end(), [](const MapData &map) {
			return map.fileName == mapName;
		});
		if (it != Game::maps.end()) {
			rp.largeImageKey = mapName.c_str();
			rp.largeImageText = rp.state;
		} else {
			rp.largeImageKey = "workshop";
			rp.largeImageText = rp.state;
		}
	}
	

	Discord_UpdatePresence(&rp);
}

ON_INIT {
	g_discordRpcInitialized = false;
	g_gameStartTimestamp = time(0);

	memset(&g_discordHandlers, 0, sizeof(g_discordHandlers));

	g_discordHandlers.ready = HandleDiscordReady;
	g_discordHandlers.disconnected = HandleDiscordDisconnected;
	g_discordHandlers.errored = HandleDiscordError;
	g_discordHandlers.joinGame = HandleDiscordJoin;
	g_discordHandlers.spectateGame = HandleDiscordSpectate;
	g_discordHandlers.joinRequest = HandleDiscordJoinRequest;
}

ON_EVENT(FRAME) {
	static int lastExecutedPresence = 0;

	if (sar_discord_rpc_enabled.GetBool() != g_discordRpcInitialized) {
		if (!g_discordRpcInitialized) {
			Discord_Initialize(sar_discord_rpc_app_id.GetString(), &g_discordHandlers, 1, "");
			lastExecutedPresence = 0;
		} else {
			Discord_Shutdown();
		}
		g_discordRpcInitialized = !g_discordRpcInitialized;
	}

	if (!g_discordRpcInitialized) return;

	int host, server, client;
	engine->GetTicks(host, server, client);

	if (host > lastExecutedPresence + 60) {
		lastExecutedPresence = host;
		UpdateDiscordRichPresence();
	}
}

ON_EVENT(SESSION_START) {
	if (tasPlayer->IsActive()) {
		g_activeTasScriptThisSession = tasPlayer->GetScriptName(0);
	} else {
		g_activeTasScriptThisSession = "";
	}

	if (sv_bonus_challenge.GetBool()) {
		g_challengeModeAttempts++;
	} else {
		g_challengeModeAttempts = 0;
	}
}

ON_EVENT(SESSION_END) {
	g_activeTasScriptThisSession = "";
}

ON_EVENT(SAR_UNLOAD) {
	if (g_discordRpcInitialized) {
		Discord_Shutdown();
	}
}
