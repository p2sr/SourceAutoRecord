#include "AutoSubmit.hpp"
#include "Command.hpp"
#include "Event.hpp"
#include "Features/Hud/Toasts.hpp"
#include "Features/NetMessage.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Utils/json11.hpp"
#include "Version.hpp"
#include <cctype>
#include <fstream>
#include <sstream>
#include <map>
#include <optional>
#include <string>
#include <filesystem>
#include <curl/curl.h>

#define API_BASE "https://board.portal2.sr/api-v2"
#define AUTOSUBMIT_TOAST_TAG "autosubmit"
#define COOP_NAME_MESSAGE_TYPE "coop-name"
#define API_KEY_FILE "autosubmit_key.txt"

std::string g_partner_name;

ON_EVENT(SESSION_START) {
	if (engine->IsCoop()) {
		const char *name = Variable("name").GetString();
		NetMessage::SendMsg(COOP_NAME_MESSAGE_TYPE, name, strlen(name));
	}
	g_partner_name = "(unknown partner)";
}

ON_INIT {
	NetMessage::RegisterHandler(COOP_NAME_MESSAGE_TYPE, +[](const void *data, size_t size) {
		g_partner_name = std::string((char *)data, size);
	});
}

static bool g_cheated = false;

ON_EVENT(SESSION_START) {
	g_cheated = sv_cheats.GetBool();
}

ON_EVENT(PRE_TICK) {
	if (sv_cheats.GetBool()) g_cheated = true;
}

static std::string g_api_key;
static bool g_key_valid;
static CURL *g_curl;
static std::thread g_worker;

static std::map<std::string, const char *> g_map_ids = {
	{ "sp_a1_intro1",  "62761" },
	{ "sp_a1_intro2",  "62758" },
	{ "sp_a1_intro3",  "47458" },
	{ "sp_a1_intro4",  "47455" },
	{ "sp_a1_intro5",  "47452" },
	{ "sp_a1_intro6",  "47106" },
	{ "sp_a1_intro7",  "62763" },
	{ "sp_a1_wakeup",  "62759" },
	{ "sp_a2_intro",   "47735" },

	{ "sp_a2_laser_intro",     "62765" },
	{ "sp_a2_laser_stairs",    "47736" },
	{ "sp_a2_dual_lasers",     "47738" },
	{ "sp_a2_laser_over_goo",  "47742" },
	{ "sp_a2_catapult_intro",  "62767" },
	{ "sp_a2_trust_fling",     "47744" },
	{ "sp_a2_pit_flings",      "47465" },
	{ "sp_a2_fizzler_intro",   "47746" },

	{ "sp_a2_sphere_peek",      "47748" },
	{ "sp_a2_ricochet",         "47751" },
	{ "sp_a2_bridge_intro",     "47752" },
	{ "sp_a2_bridge_the_gap",   "47755" },
	{ "sp_a2_turret_intro",     "47756" },
	{ "sp_a2_laser_relays",     "47759" },
	{ "sp_a2_turret_blocker",   "47760" },
	{ "sp_a2_laser_vs_turret",  "47763" },
	{ "sp_a2_pull_the_rug",     "47764" },

	{ "sp_a2_column_blocker",  "47766" },
	{ "sp_a2_laser_chaining",  "47768" },
	{ "sp_a2_triple_laser",    "47770" },
	{ "sp_a2_bts1",            "47773" },
	{ "sp_a2_bts2",            "47774" },

	{ "sp_a2_bts3",  "47776" },
	{ "sp_a2_bts4",  "47779" },
	{ "sp_a2_bts5",  "47780" },
	{ "sp_a2_core",  "62771" },

	{ "sp_a3_01",            "47783" },
	{ "sp_a3_03",            "47784" },
	{ "sp_a3_jump_intro",    "47787" },
	{ "sp_a3_bomb_flings",   "47468" },
	{ "sp_a3_crazy_box",     "47469" },
	{ "sp_a3_transition01",  "47472" },

	{ "sp_a3_speed_ramp",    "47791" },
	{ "sp_a3_speed_flings",  "47793" },
	{ "sp_a3_portal_intro",  "47795" },
	{ "sp_a3_end",           "47798" },

	{ "sp_a4_intro",           "88350" },
	{ "sp_a4_tb_intro",        "47800" },
	{ "sp_a4_tb_trust_drop",   "47802" },
	{ "sp_a4_tb_wall_button",  "47804" },
	{ "sp_a4_tb_polarity",     "47806" },
	{ "sp_a4_tb_catch",        "47808" },
	{ "sp_a4_stop_the_box",    "47811" },
	{ "sp_a4_laser_catapult",  "47813" },
	{ "sp_a4_laser_platform",  "47815" },
	{ "sp_a4_speed_tb_catch",  "47817" },
	{ "sp_a4_jump_polarity",   "47819" },

	{ "sp_a4_finale1",  "62776" },
	{ "sp_a4_finale2",  "47821" },
	{ "sp_a4_finale3",  "47824" },
	{ "sp_a4_finale4",  "47456" },

	{ "mp_coop_doors",          "47741" },
	{ "mp_coop_race_2",         "47825" },
	{ "mp_coop_laser_2",        "47828" },
	{ "mp_coop_rat_maze",       "47829" },
	{ "mp_coop_laser_crusher",  "45467" },
	{ "mp_coop_teambts",        "46362" },

	{ "mp_coop_fling_3",            "47831" },
	{ "mp_coop_infinifling_train",  "47833" },
	{ "mp_coop_come_along",         "47835" },
	{ "mp_coop_fling_1",            "47837" },
	{ "mp_coop_catapult_1",         "47840" },
	{ "mp_coop_multifling_1",       "47841" },
	{ "mp_coop_fling_crushers",     "47844" },
	{ "mp_coop_fan",                "47845" },

	{ "mp_coop_wall_intro",           "47848" },
	{ "mp_coop_wall_2",               "47849" },
	{ "mp_coop_catapult_wall_intro",  "47854" },
	{ "mp_coop_wall_block",           "47856" },
	{ "mp_coop_catapult_2",           "47858" },
	{ "mp_coop_turret_walls",         "47861" },
	{ "mp_coop_turret_ball",          "52642" },
	{ "mp_coop_wall_5",               "52660" },

	{ "mp_coop_tbeam_redirect",       "52662" },
	{ "mp_coop_tbeam_drill",          "52663" },
	{ "mp_coop_tbeam_catch_grind_1",  "52665" },
	{ "mp_coop_tbeam_laser_1",        "52667" },
	{ "mp_coop_tbeam_polarity",       "52671" },
	{ "mp_coop_tbeam_polarity2",      "52687" },
	{ "mp_coop_tbeam_polarity3",      "52689" },
	{ "mp_coop_tbeam_maze",           "52691" },
	{ "mp_coop_tbeam_end",            "52777" },

	{ "mp_coop_paint_come_along",      "52694" },
	{ "mp_coop_paint_redirect",        "52711" },
	{ "mp_coop_paint_bridge",          "52714" },
	{ "mp_coop_paint_walljumps",       "52715" },
	{ "mp_coop_paint_speed_fling",     "52717" },
	{ "mp_coop_paint_red_racer",       "52735" },
	{ "mp_coop_paint_speed_catch",     "52738" },
	{ "mp_coop_paint_longjump_intro",  "52740" },

	{ "mp_coop_separation_1",      "49341" },
	{ "mp_coop_tripleaxis",        "49343" },
	{ "mp_coop_catapult_catch",    "49345" },
	{ "mp_coop_2paints_1bridge",   "49347" },
	{ "mp_coop_paint_conversion",  "49349" },
	{ "mp_coop_bridge_catch",      "49351" },
	{ "mp_coop_laser_tbeam",       "52757" },
	{ "mp_coop_paint_rat_maze",    "52759" },
	{ "mp_coop_paint_crazy_box",   "48287" },
};

static bool ensureCurlReady() {
	if (!g_curl) {
		g_curl = curl_easy_init();

		if (!g_curl) {
			return false;
		}
	}

	return true;
}

static std::optional<std::string> request(const char *url) {
	curl_easy_setopt(g_curl, CURLOPT_URL, url);
	curl_easy_setopt(g_curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(g_curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(g_curl, CURLOPT_TIMEOUT, 30);

	curl_easy_setopt(
		g_curl,
		CURLOPT_WRITEFUNCTION,
		+[](void *ptr, size_t sz, size_t nmemb, std::string *data) -> size_t {
			data->append((char *)ptr, sz * nmemb);
			return sz * nmemb;
		}
	);

	std::string response;
	curl_easy_setopt(g_curl, CURLOPT_WRITEDATA, &response);

	CURLcode res = curl_easy_perform(g_curl);

	long code;
	curl_easy_getinfo(g_curl, CURLINFO_RESPONSE_CODE, &code);

	if (res != CURLE_OK) {
		THREAD_PRINT("ERROR IN AUTOSUBMIT REQUEST TO %s: %s\n", url, curl_easy_strerror(res));
	}

	return res == CURLE_OK && code == 200 ? response : std::optional<std::string>{};
}

static void testApiKey() {
	if (!ensureCurlReady()) {
		THREAD_PRINT("Failed to test API key!\n");
		return;
	}

	curl_mime *form = curl_mime_init(g_curl);
	curl_mimepart *field;

	field = curl_mime_addpart(form);
	curl_mime_name(field, "auth_hash");
	curl_mime_data(field, g_api_key.c_str(), CURL_ZERO_TERMINATED);

	curl_easy_setopt(g_curl, CURLOPT_MIMEPOST, form);

	auto response = request(API_BASE"/validate-user");

	curl_mime_free(form);

	if (!response) {
		THREAD_PRINT("API key invalid!\n");
	} else {
		g_key_valid = true;
		THREAD_PRINT("API key valid!\n");
	}
}

static std::optional<int> getCurrentPbScore(const char *map_id) {
	if (!ensureCurlReady()) return {};

	curl_mime *form = curl_mime_init(g_curl);
	curl_mimepart *field;

	field = curl_mime_addpart(form);
	curl_mime_name(field, "auth_hash");
	curl_mime_data(field, g_api_key.c_str(), CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "mapId");
	curl_mime_data(field, map_id, CURL_ZERO_TERMINATED);

	curl_easy_setopt(g_curl, CURLOPT_MIMEPOST, form);

	auto response = request(API_BASE"/current-pb");

	curl_mime_free(form);

	if (!response) return {};

	std::string err;
	auto json = json11::Json::parse(*response, err);

	if (err != "") {
		return {};
	}

	if (json["score"].is_null()) {
		return -1; // No current PB
	}

	auto str = json["score"].string_value();
	return atoi(str.c_str());
}

static void submitTime(int score, std::string demopath, bool coop, const char *map_id, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb) {
	auto score_str = std::to_string(score);

	if (!g_key_valid) {
		if (rename_if_pb) {
			std::filesystem::rename(demopath, *rename_if_pb);
		}
		if (replay_append_if_pb) {
			engine->demoplayer->replayName += *replay_append_if_pb;
		}
		return;
	}

	auto cur_pb = getCurrentPbScore(map_id);
	if (cur_pb) {
		if (*cur_pb > -1 && score >= *cur_pb) {
			THREAD_PRINT("Not PB; not submitting.\n");
			Scheduler::OnMainThread([=](){
				Event::Trigger<Event::MAYBE_AUTOSUBMIT>({score, coop, false});
			});
			return;
		}
	}

	// If we couldn't detect if this run PBd, rename the demo anyway to be
	// safe

	if (rename_if_pb) {
		std::filesystem::rename(demopath, *rename_if_pb);
		demopath = *rename_if_pb;
	}

	if (replay_append_if_pb) {
		engine->demoplayer->replayName += *replay_append_if_pb;
	}

	if (!cur_pb) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time");
		return;
	}

	if (!ensureCurlReady()) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time");
		return;
	}

	Scheduler::OnMainThread([=](){
		Event::Trigger<Event::MAYBE_AUTOSUBMIT>({score, coop, true});
	});

	curl_mime *form = curl_mime_init(g_curl);
	curl_mimepart *field;

	field = curl_mime_addpart(form);
	curl_mime_name(field, "auth_hash");
	curl_mime_data(field, g_api_key.c_str(), CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "mapId");
	curl_mime_data(field, map_id, CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "score");
	curl_mime_data(field, score_str.c_str(), CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "demoFile");
	curl_mime_filedata(field, demopath.c_str());

	if (coop) {
		field = curl_mime_addpart(form);
		curl_mime_name(field, "comment");
		curl_mime_data(field, g_partner_name.c_str(), CURL_ZERO_TERMINATED);
	}

	curl_easy_setopt(g_curl, CURLOPT_MIMEPOST, form);

	auto resp = request(API_BASE"/auto-submit");

	curl_mime_free(form);

	if (!resp) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time");
	} else {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "Successfully submitted time to boards!");
	}
}

static void loadApiKey(bool output_nonexist) {
	if (!std::filesystem::exists(API_KEY_FILE)) {
		if (output_nonexist) {
			console->Print("API key file " API_KEY_FILE " does not exist!\n");
		}
		return;
	}

	std::string key;
	{
		std::ifstream f(API_KEY_FILE);
		std::stringstream buf;
		buf << f.rdbuf();
		key = buf.str();
	}

	key.erase(std::remove_if(key.begin(), key.end(), isspace), key.end());

	bool valid = key.size() > 0;
	for (auto c : key) {
		if (c >= 'a' && c <= 'z') continue;
		if (c >= 'A' && c <= 'Z') continue;
		if (c >= '0' && c <= '9') continue;
		valid = false;
	}

	if (!valid) {
		console->Print("Invalid API key!\n");
		return;
	}

	g_api_key = key;
	g_key_valid = false;
	console->Print("Set API key! Testing...\n");

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread(testApiKey);
}

ON_INIT {
	loadApiKey(false);
}

CON_COMMAND_F(sar_challenge_autosubmit_reload_api_key, "sar_challenge_autosubmit_reload_api_key - reload the board.portal2.sr API key from its file.\n", FCVAR_DONTRECORD) {
	if (args.ArgC() != 1) {
		return console->Print(sar_challenge_autosubmit_reload_api_key.ThisPtr()->m_pszHelpString);
	}

	loadApiKey(true);
}

void AutoSubmit::FinishRun(float final_time, const char *demopath, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb) {
	if (g_cheated) {
		console->Print("Cheated; not autosubmitting\n");
		return;
	}

#if defined(SAR_DEV_BUILD)
	console->Print("Dev SAR build; not autosubmitting\n");
	return;
#endif

	auto it = g_map_ids.find(engine->GetCurrentMapName());
	if (it == g_map_ids.end()) {
		console->Print("Unknown map; not autosubmitting\n");
		if (rename_if_pb) {
			std::filesystem::rename(demopath, *rename_if_pb);
		}
		if (replay_append_if_pb) {
			engine->demoplayer->replayName += *replay_append_if_pb;
		}
		return;
	}

	const char *map_id = it->second;

	int score = floor(final_time * 100);

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread(submitTime, score, std::string(demopath), Utils::StartsWith(engine->GetCurrentMapName().c_str(), "mp_"), map_id, rename_if_pb, replay_append_if_pb);
}

ON_EVENT(SAR_UNLOAD) {
	if (g_worker.joinable()) g_worker.detach();
}
