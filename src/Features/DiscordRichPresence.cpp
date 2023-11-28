
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

static std::map<std::string, const char *> g_map_names = {
	{"sp_a1_intro1", "Container Ride"},
	{"sp_a1_intro2", "Portal Carousel"},
	{"sp_a1_intro3", "Portal Gun"},
	{"sp_a1_intro4", "Smooth Jazz"},
	{"sp_a1_intro5", "Cube Momentum"},
	{"sp_a1_intro6", "Future Starter"},
	{"sp_a1_intro7", "Secret Panel"},
	{"sp_a1_wakeup", "Wakeup"},
	{"sp_a2_intro", "Incinerator"},

	{"sp_a2_laser_intro", "Laser Intro"},
	{"sp_a2_laser_stairs", "Laser Stairs"},
	{"sp_a2_dual_lasers", "Dual Lasers"},
	{"sp_a2_laser_over_goo", "Laser Over Goo"},
	{"sp_a2_catapult_intro", "Catapult Intro"},
	{"sp_a2_trust_fling", "Trust Fling"},
	{"sp_a2_pit_flings", "Pit Flings"},
	{"sp_a2_fizzler_intro", "Fizzler Intro"},

	{"sp_a2_sphere_peek", "Ceiling Catapult"},
	{"sp_a2_ricochet", "Ricochet"},
	{"sp_a2_bridge_intro", "Bridge Intro"},
	{"sp_a2_bridge_the_gap", "Bridge The Gap"},
	{"sp_a2_turret_intro", "Turret Intro"},
	{"sp_a2_laser_relays", "Laser Relays"},
	{"sp_a2_turret_blocker", "Turret Blocker"},
	{"sp_a2_laser_vs_turret", "Laser Vs Turret"},
	{"sp_a2_pull_the_rug", "Pull The Rug"},

	{"sp_a2_column_blocker", "Column Blocker"},
	{"sp_a2_laser_chaining", "Laser Chaining"},
	{"sp_a2_triple_laser", "Triple Laser"},
	{"sp_a2_bts1", "Jail Break"},
	{"sp_a2_bts2", "Escape"},

	{"sp_a2_bts3", "Turret Factory"},
	{"sp_a2_bts4", "Turret Sabotage"},
	{"sp_a2_bts5", "Neurotoxin Sabotage"},
	{"sp_a2_bts6", "Tube Ride"},
	{"sp_a2_core", "Core"},

	{"sp_a3_00", "Long Fall"},
	{"sp_a3_01", "Underground"},
	{"sp_a3_03", "Cave Johnson"},
	{"sp_a3_jump_intro", "Repulsion Intro"},
	{"sp_a3_bomb_flings", "Bomb Flings"},
	{"sp_a3_crazy_box", "Crazy Box"},
	{"sp_a3_transition01", "PotatOS"},

	{"sp_a3_speed_ramp", "Propulsion Intro"},
	{"sp_a3_speed_flings", "Propulsion Flings"},
	{"sp_a3_portal_intro", "Conversion Intro"},
	{"sp_a3_end", "Three Gels"},

	{"sp_a4_intro", "Test"},
	{"sp_a4_tb_intro", "Funnel Intro"},
	{"sp_a4_tb_trust_drop", "Ceiling Button"},
	{"sp_a4_tb_wall_button", "Wall Button"},
	{"sp_a4_tb_polarity", "Polarity"},
	{"sp_a4_tb_catch", "Funnel Catch"},
	{"sp_a4_stop_the_box", "Stop The Box"},
	{"sp_a4_laser_catapult", "Laser Catapult"},
	{"sp_a4_laser_platform", "Laser Platform"},
	{"sp_a4_speed_tb_catch", "Propulsion Catch"},
	{"sp_a4_jump_polarity", "Repulsion Polarity"},

	{"sp_a4_finale1", "Finale 1"},
	{"sp_a4_finale2", "Finale 2"},
	{"sp_a4_finale3", "Finale 3"},
	{"sp_a4_finale4", "Finale 4"},
	{"sp_a5_credits", "Credits"},

	{"mp_coop_start", "Calibration"},
	{"mp_coop_lobby_2", "Cooperative Hub (pre-DLC)"},
	{"mp_coop_lobby_3", "Cooperative Hub"},

	{"mp_coop_doors", "Doors"},
	{"mp_coop_race_2", "Buttons"},
	{"mp_coop_laser_2", "Lasers"},
	{"mp_coop_rat_maze", "Rat Maze"},
	{"mp_coop_laser_crusher", "Laser Crusher"},
	{"mp_coop_teambts", "Behind The Scenes"},

	{"mp_coop_fling_3", "Flings"},
	{"mp_coop_infinifling_train", "Infinifling"},
	{"mp_coop_come_along", "Team Retrieval"},
	{"mp_coop_fling_1", "Vertical Flings"},
	{"mp_coop_catapult_1", "Catapults"},
	{"mp_coop_multifling_1", "Multifling"},
	{"mp_coop_fling_crushers", "Fling Crushers"},
	{"mp_coop_fan", "Industrial Fan"},

	{"mp_coop_wall_intro", "Cooperative Bridges"},
	{"mp_coop_wall_2", "Bridge Swap"},
	{"mp_coop_catapult_wall_intro", "Fling Block"},
	{"mp_coop_wall_block", "Catapult Block"},
	{"mp_coop_catapult_2", "Bridge Flings"},
	{"mp_coop_turret_walls", "Turret Walls"},
	{"mp_coop_turret_ball", "Turret Assassin"},
	{"mp_coop_wall_5", "Bridge Testing"},

	{"mp_coop_tbeam_redirect", "Cooperative Funnels"},
	{"mp_coop_tbeam_drill", "Funnel Drill"},
	{"mp_coop_tbeam_catch_grind_1", "Funnel Catch"},
	{"mp_coop_tbeam_laser_1", "Funnel Laser"},
	{"mp_coop_tbeam_polarity", "Cooperative Polarity"},
	{"mp_coop_tbeam_polarity2", "Funnel Hop"},
	{"mp_coop_tbeam_polarity3", "Advanced Polarity"},
	{"mp_coop_tbeam_maze", "Funnel Maze"},
	{"mp_coop_tbeam_end", "Turret Warehouse"},

	{"mp_coop_paint_come_along", "Repulsion Jumps"},
	{"mp_coop_paint_redirect", "Double Bounce"},
	{"mp_coop_paint_bridge", "Bridge Repulsion"},
	{"mp_coop_paint_walljumps", "Wall Repulsion"},
	{"mp_coop_paint_speed_fling", "Propulsion Crushers"},
	{"mp_coop_paint_red_racer", "Turret Ninja"},
	{"mp_coop_paint_speed_catch", "Propulsion Retrieval"},
	{"mp_coop_paint_longjump_intro", "Vault Entrance"},

	{"mp_coop_separation_1", "Separation"},
	{"mp_coop_tripleaxis", "Triple Axis"},
	{"mp_coop_catapult_catch", "Catapult Catch"},
	{"mp_coop_2paints_1bridge", "Bridge Gels"},
	{"mp_coop_paint_conversion", "Maintenance"},
	{"mp_coop_bridge_catch", "Bridge Catch"},
	{"mp_coop_laser_tbeam", "Double Lift"},
	{"mp_coop_paint_rat_maze", "Gel Maze"},
	{"mp_coop_paint_crazy_box", "Crazier Box"},
};

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

static void UpdateDiscordRichPresence() {
	if (!g_discordRpcInitialized) return;

	DiscordRichPresence rp;
	memset(&rp, 0, sizeof(rp));

	rp.startTimestamp = g_gameStartTimestamp;
	
	auto categoryName = SpeedrunTimer::GetCategoryName();
	auto cmAttemptsString = Utils::ssprintf("Attempts: %d", g_challengeModeAttempts);
	auto mapName = std::string("");

	if (!session->isRunning) {
		rp.details = "Loading";
		rp.state = "";
		rp.largeImageKey = "menu";
		rp.largeImageText = "Loading";
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
		if (g_map_names.find(mapName) != g_map_names.end()) {
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
