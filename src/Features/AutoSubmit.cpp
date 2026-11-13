#include "AutoSubmit.hpp"

#include "../Games/Portal2.hpp"
#include "../Modules/Client.hpp"
#include "Cheats.hpp"
#include "Command.hpp"
#include "Event.hpp"
#include "Features/Hud/Toasts.hpp"
#include "Features/NetMessage.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Engine.hpp"
#include "Modules/FileSystem.hpp"
#include "Modules/Server.hpp"
#include "Utils/json11.hpp"
#include "Version.hpp"

#include <cctype>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <string>

#define API_BASE_AUTORENDER "https://autorender.portal2.sr/api"
#define AUTOSUBMIT_TOAST_TAG "autosubmit"
#define COOP_NAME_MESSAGE_TYPE "coop-name"
#define API_KEY_FILE "autosubmit.key"
#define OLD_API_KEY_FILE "autosubmit_key.txt"
#define LPHUB_API_BASE "https://lp.portal2.sr/api/v1"
#define LPHUB_VALIDATE_PATH "/token/autosubmit"
#define LPHUB_PB_PATH "/record/pb"
#define LPHUB_SUBMIT_PATH "/record"
#define LPHUB_AUTOSUBMIT_TIMEOUT 120

bool AutoSubmit::g_cheated = false;
int AutoSubmit::g_paused = 0;
std::string AutoSubmit::g_partner_name = "";

ON_EVENT(SESSION_START) {
	if (engine->IsCoop()) {
		const char *name = Variable("name").GetString();
		NetMessage::SendMsg(COOP_NAME_MESSAGE_TYPE, name, strlen(name));
	}
	AutoSubmit::g_partner_name = "(unknown partner)";
}

ON_INIT {
	NetMessage::RegisterHandler(COOP_NAME_MESSAGE_TYPE, +[](const void *data, size_t size) {
		AutoSubmit::g_partner_name = std::string((char *)data, size);
	});
}

ON_EVENT(SESSION_START) {
	AutoSubmit::g_cheated = sv_cheats.GetBool();
	AutoSubmit::g_paused = 0;
}

ON_EVENT(PRE_TICK) {
	if (sv_cheats.GetBool()) AutoSubmit::g_cheated = true;
	if (engine->IsGamePaused() && SpeedrunTimer::IsRunning() && !client->g_leaderboardOpen) AutoSubmit::g_paused++;
}

static std::string g_api_base;
static std::string g_api_key;
static std::string g_lphub_key;
static bool g_key_valid;
static CURL *g_curl;
static CURL *g_curl_search;
static std::thread g_worker;
static std::thread g_worker_search;
static std::map<std::string, std::string> g_map_ids;
static bool g_is_querying;
static std::vector<json11::Json> g_times;

static bool ensureCurlReady(CURL **curl) {
	if (!*curl) {
		*curl = curl_easy_init();

		if (!*curl) {
			return false;
		}
	}

	return true;
}

static std::optional<std::string> request(CURL *curl, std::string url, long timeout = 30) {
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

#ifdef UNSAFELY_IGNORE_CERTIFICATE_ERROR
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
#endif

	curl_easy_setopt(
		curl,
		CURLOPT_WRITEFUNCTION,
		+[](void *ptr, size_t sz, size_t nmemb, std::string *data) -> size_t {
			data->append((char *)ptr, sz * nmemb);
			return sz * nmemb;
		});

	std::string response;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	CURLcode res = curl_easy_perform(curl);

	long code;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);

	if (res != CURLE_OK) {
		THREAD_PRINT("ERROR IN AUTOSUBMIT REQUEST TO %s: %s\n", url.c_str(), curl_easy_strerror(res));
	}

	return res == CURLE_OK && code == 200 ? response : std::optional<std::string>{};
}

static void testLpApiKey() {
	if (g_lphub_key.empty()) return;
	CURL *curl = curl_easy_init();
	if (!curl) {
		THREAD_PRINT("LPHUB: couldn't test autosubmit token; couldn't ready CURL.\n");
		return;
	}
	std::string authorization = "Authorization: Bearer " + g_lphub_key;
	curl_slist *headers = curl_slist_append(nullptr, authorization.c_str());
	if (!headers) {
		curl_easy_cleanup(curl);
		THREAD_PRINT("LPHUB: couldn't test autosubmit token; couldn't set authorization header.\n");
		return;
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
	auto response = request(curl, std::string(LPHUB_API_BASE) + LPHUB_VALIDATE_PATH);
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	if (!response) {
		THREAD_PRINT("LPHUB: couldn't validate autosubmit token; request failed.\n");
		return;
	}
	std::string err;
	auto json = json11::Json::parse(*response, err);
	if (!err.empty() || !json["success"].is_bool()) {
		THREAD_PRINT("LPHUB: couldn't validate autosubmit token; invalid response.\n");
	} else if (!json["success"].bool_value()) {
		auto message = json["message"].string_value();
		THREAD_PRINT("LPHUB: autosubmit token rejected: %s\n", message.empty() ? "validation failed" : message.c_str());
	} else {
		THREAD_PRINT("LPHUB: autosubmit token valid!\n");
	}
}

static void testApiKey() {
	auto host = g_api_base.substr(g_api_base.find("://") + 3);
	host = host.substr(0, host.find('/'));
	if (!ensureCurlReady(&g_curl)) {
		THREAD_PRINT("%s: couldn't test API key; couldn't ready CURL.\n", host.c_str());
		return;
	}

	curl_mime *form = curl_mime_init(g_curl);
	curl_mimepart *field;

	field = curl_mime_addpart(form);
	curl_mime_name(field, "auth_hash");
	curl_mime_data(field, g_api_key.c_str(), CURL_ZERO_TERMINATED);

	curl_easy_setopt(g_curl, CURLOPT_MIMEPOST, form);

	auto response = request(g_curl, g_api_base + "/validate-user");

	curl_mime_free(form);

	if (!response) {
		THREAD_PRINT("%s: couldn't validate API key; request failed or key was rejected.\n", host.c_str());
		return;
	}

	g_key_valid = true;
	THREAD_PRINT("%s: API key valid!\n", host.c_str());

	// FIXME: add API maps endpoint on board.portal2.sr
	if (sar.game->Is(SourceGame_Portal2)) {
		for (const auto &map : Game::maps) {
			if (strlen(map.chamberId) > 0) {
				g_map_ids.insert({map.fileName, map.chamberId});
			}
		}
		return;
	}

	response = request(g_curl, g_api_base + "/download-maps");
	if (!response) {
		THREAD_PRINT("%s: failed to download maps!\n", host.c_str());
		return;
	}

	std::string err;
	auto json = json11::Json::parse(*response, err);

	if (err != "") {
		THREAD_PRINT("%s: failed to parse maps JSON: %s\n", host.c_str(), err.c_str());
		return;
	}

	for (auto &map : json["maps"].array_items()) {
		g_map_ids.insert({map["level_name"].string_value(), map["id"].string_value()});
	}

	THREAD_PRINT("%s: downloaded %zu maps!\n", host.c_str(), g_map_ids.size());
}

std::optional<std::string> AutoSubmit::GetMapId(std::string map_name) {
	auto it = g_map_ids.find(map_name);
	if (it == g_map_ids.end()) {
		return {};
	}

	return it->second;
}

static std::optional<int> getCurrentPbScore(std::string map_id) {
	if (!ensureCurlReady(&g_curl)) return {};

	curl_mime *form = curl_mime_init(g_curl);
	curl_mimepart *field;

	field = curl_mime_addpart(form);
	curl_mime_name(field, "auth_hash");
	curl_mime_data(field, g_api_key.c_str(), CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "mapId");
	curl_mime_data(field, map_id.c_str(), CURL_ZERO_TERMINATED);

	curl_easy_setopt(g_curl, CURLOPT_MIMEPOST, form);

	auto response = request(g_curl, g_api_base + "/current-pb");

	curl_mime_free(form);

	if (!response) return {};

	std::string err;
	auto json = json11::Json::parse(*response, err);

	if (err != "") {
		return {};
	}

	if (json["score"].is_null()) {
		return -1;  // No current PB
	}

	auto str = json["score"].string_value();
	return atoi(str.c_str());
}

static void startSearching(std::string mapName) {
	auto map_id = AutoSubmit::GetMapId(mapName);
	if (!map_id.has_value()) {
		g_is_querying = false;
		return;
	}

	auto json = AutoSubmit::GetTopScores(*map_id);

	g_times.clear();
	for (auto score : json) {
		g_times.push_back(score);
	}

	g_is_querying = false;
}

void AutoSubmit::Search(std::string map) {
	g_is_querying = true;

	if (g_worker_search.joinable()) g_worker_search.join();
	g_worker_search = std::thread(startSearching, map);
}

json11::Json::array AutoSubmit::GetTopScores(std::string &map_id) {
	if (!ensureCurlReady(&g_curl_search)) return {};

	curl_mime *form = curl_mime_init(g_curl_search);
	curl_mimepart *field;

	field = curl_mime_addpart(form);
	curl_mime_name(field, "auth_hash");
	curl_mime_data(field, g_api_key.c_str(), CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "mapId");
	curl_mime_data(field, map_id.c_str(), CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "before");
	curl_mime_data(field, "3", CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "after");
	curl_mime_data(field, "2", CURL_ZERO_TERMINATED);

	curl_easy_setopt(g_curl_search, CURLOPT_MIMEPOST, form);

	auto response = request(g_curl_search, g_api_base + "/top-scores");

	curl_mime_free(form);

	if (!response) return {};

	std::string err;
	auto json = json11::Json::parse(*response, err);

	if (err != "") {
		return {};
	}

	return json.array_items();
}

bool AutoSubmit::IsQuerying() {
	return g_is_querying;
}

const std::vector<json11::Json>& AutoSubmit::GetTimes() {
	return g_times;
}

static bool renameDemo(std::string &path, const std::string &target, const std::optional<std::string> &replay_suffix) {
	std::error_code error;
	std::filesystem::rename(path, target, error);
	if (error) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "Couldn't rename demo: " + error.message());
		return false;
	}
	path = target;
	if (replay_suffix) {
		Scheduler::OnMainThread([suffix = *replay_suffix]() {
			engine->demoplayer->replayName += suffix;
		});
	}
	return true;
}

enum class PbOutcome { Unknown, NotPb, Pb };

static PbOutcome checkTimePb(int score, const std::string &map_id) {
	if (!g_key_valid) {
		return PbOutcome::Unknown;
	}

	auto cur_pb = getCurrentPbScore(map_id);
	if (cur_pb) {
		if (*cur_pb > -1 && score >= *cur_pb) {
			THREAD_PRINT("Not PB; not submitting.\n");
			return PbOutcome::NotPb;
		}
	}

	if (!cur_pb) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time - couldn't fetch PB");
		return PbOutcome::Unknown;
	}

	return PbOutcome::Pb;
}

static void submitTime(int score, const std::string &demopath, bool coop, const std::string &map_id) {
	auto score_str = std::to_string(score);

	if (!ensureCurlReady(&g_curl)) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time - couldn't ready CURL");
		return;
	}

	Scheduler::OnMainThread([=]() {
		Event::Trigger<Event::MAYBE_AUTOSUBMIT>({score, coop, true});
	});

	curl_mime *form = curl_mime_init(g_curl);
	curl_mimepart *field;

	field = curl_mime_addpart(form);
	curl_mime_name(field, "auth_hash");
	curl_mime_data(field, g_api_key.c_str(), CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "mapId");
	curl_mime_data(field, map_id.c_str(), CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "score");
	curl_mime_data(field, score_str.c_str(), CURL_ZERO_TERMINATED);

	field = curl_mime_addpart(form);
	curl_mime_name(field, "demoFile");
	curl_mime_filedata(field, demopath.c_str());

	if (coop) {
		field = curl_mime_addpart(form);
		curl_mime_name(field, "comment");
		curl_mime_data(field, AutoSubmit::g_partner_name.c_str(), CURL_ZERO_TERMINATED);
	}

	curl_easy_setopt(g_curl, CURLOPT_MIMEPOST, form);

	auto resp = request(g_curl, g_api_base +  "/auto-submit");

	curl_mime_free(form);

	if (!resp) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time - didn't get a response");
	} else {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "Successfully submitted time to boards!");
	}
}

static std::optional<json11::Json> postLP(const char *path, const std::string &key, const std::string &data, bool pb_check) {
	if (!sar.game->Is(SourceGame_Portal2) && !sar.game->Is(SourceGame_PortalStoriesMel)) return {};
	if (key.empty()) return {};
	CURL *curl = curl_easy_init();
	if (!curl) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "LPHUB: couldn't ready CURL");
		return {};
	}
	std::string authorization = "Authorization: Bearer " + key;
	curl_slist *headers = curl_slist_append(nullptr, authorization.c_str());
	if (!headers) {
		curl_easy_cleanup(curl);
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "LPHUB: couldn't set authorization header");
		return {};
	}
	curl_mime *form = nullptr;
	if (pb_check) {
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
	} else {
		form = curl_mime_init(curl);
		auto field = curl_mime_addpart(form);
		curl_mime_name(field, "host_demo");
		curl_mime_filedata(field, data.c_str());
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	auto response = request(curl, std::string(LPHUB_API_BASE) + path, LPHUB_AUTOSUBMIT_TIMEOUT);
	curl_mime_free(form);
	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	if (!response) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "LPHUB: didn't get a response");
		return {};
	}
	std::string err;
	auto json = json11::Json::parse(*response, err);
	if (!err.empty() || !json["success"].is_bool()) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "LPHUB: invalid response");
		return {};
	}
	if (!json["success"].bool_value()) {
		auto message = json["message"].string_value();
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "LPHUB: " + (message.empty() ? "request failed" : message));
		return {};
	}
	return json;
}

static PbOutcome checkLpPb(int score, const std::string &key, const std::string &map_name, int time_taken) {
	json11::Json payload = json11::Json::object{
		{"engine_map_name", map_name},
		{"portal_count", score},
		{"score_time", time_taken}, // Challenge time in ticks.
	};
	auto pb = postLP(LPHUB_PB_PATH, key, payload.dump(), true);
	if (!pb) return PbOutcome::Unknown;
	if (!(*pb)["data"]["is_personal_best"].is_bool()) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "LPHUB: couldn't fetch PB");
		return PbOutcome::Unknown;
	}
	if (!(*pb)["data"]["is_personal_best"].bool_value()) {
		THREAD_PRINT("LPHUB: not PB; not submitting.\n");
		return PbOutcome::NotPb;
	}
	return PbOutcome::Pb;
}

static void submitLP(const std::string &demopath, const std::string &key) {
	if (postLP(LPHUB_SUBMIT_PATH, key, demopath, false)) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "Successfully submitted to LPHUB!");
	}
}

static void convertOldApiKeys() {
	auto old_filepath = fileSystem->FindFileSomewhere(OLD_API_KEY_FILE);
	if (!old_filepath.has_value())
		return;

	/* read key from old file */
	std::string key;
	{
		std::ifstream f(old_filepath.value());
		std::stringstream buf;
		buf << f.rdbuf();
		key = buf.str();
	}

	key.erase(std::remove_if(key.begin(), key.end(), isspace), key.end());

	std::ofstream f;

	/* check if user has a key with new format already, if yes, append to end, if not, create new file */
	auto new_filepath = fileSystem->FindFileSomewhere(API_KEY_FILE);
	if (!new_filepath.has_value()) {
		/* create new file path */
		auto path = old_filepath.value();
		path = path.substr(0, path.length() - strlen(OLD_API_KEY_FILE));

		f.open(path + API_KEY_FILE);
	} else
		f.open(new_filepath.value(), std::ios::app);  // open in append mode

	/* file should have newline at end! */
	f << "board.portal2.sr" << std::endl;
	f << key << std::endl;

	f.close();

	/* remove old api key file */
	std::filesystem::remove(old_filepath.value());

	console->Print("Successfully converted API key file!\n");
}

void AutoSubmit::LoadApiKey(bool output_nonexist) {
	convertOldApiKeys();
	if (g_worker.joinable())
	    g_worker.join();
	g_lphub_key.clear();
	g_key_valid = false;

	auto filepath = fileSystem->FindFileSomewhere(API_KEY_FILE);
	if (!filepath.has_value()) {
		if (output_nonexist) {
			console->Print("API key file " API_KEY_FILE " does not exist!\n");
		}
		return;
	}

	// Read the LPHUB entry separately so the other boards stays unchanged.
	{
		std::ifstream f(filepath.value());
		std::string host, hash;
		while (std::getline(f, host)) {
			host.erase(std::remove_if(host.begin(), host.end(), [](unsigned char c) { return std::isspace(c); }), host.end());
			if (host.empty()) continue;
			if (!std::getline(f, hash)) break;
			if (host == "lp.portal2.sr") {
				hash.erase(std::remove_if(hash.begin(), hash.end(), [](unsigned char c) { return std::isspace(c); }), hash.end());
				g_lphub_key = hash;
				break;
			}
		}
	}
	if (!g_lphub_key.empty()) console->Print("LPHUB: autosubmit token loaded.\n");

	std::string base;
	std::string key;

	{
		std::ifstream f(filepath.value());
		/* if they use p2 common, the file is going to be a list of endpoints */
		while (!f.eof()) {
			std::getline(f, base) && std::getline(f, key);
			if (sar.game->Is(SourceGame_Portal2) && base == "board.portal2.sr") break;
			if (sar.game->Is(SourceGame_PortalStoriesMel) && base == "mel.portal2.sr") break;
			if (sar.game->Is(SourceGame_PortalStoriesMel) && base == "melboard.portal2.sr") {
				base = "mel.portal2.sr";
				break;
			}
			if (sar.game->Is(SourceGame_PortalStoriesMel) && base == "mel.board.portal2.sr") {
				base = "mel.portal2.sr";
				break;
			}
			/* if we got here, no valid key */
			base.clear();
			key.clear();
		}
	}

	key.erase(std::remove_if(key.begin(), key.end(), isspace), key.end());

	bool valid = key.size() > 0;
	for (auto c : key) {
		if (c >= 'a' && c <= 'z') continue;
		if (c >= 'A' && c <= 'Z') continue;
		if (c >= '0' && c <= '9') continue;
		valid = false;
	}

	if (valid) {
		g_api_base = "https://" + base + "/api-v2";
		g_api_key = key;
		console->Print("%s: API key loaded; testing...\n", base.c_str());
	} else {
		console->Print("Invalid API key!\n");
	}

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread([valid]() {
		testLpApiKey();
		if (valid) testApiKey();
	});
}

size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *buffer) {
	size_t realsize = size * nmemb;
	buffer->append((char *)contents, realsize);
	return realsize;
}

void retrieveMtriggers(int rank, std::string map_name) {
	bool keyFound = false;
	if (map_name.empty())
		return THREAD_PRINT("Not playing a map.\n");

	std::string map_lower = map_name;
	std::transform(map_lower.begin(), map_lower.end(), map_lower.begin(), tolower);
	for (const auto &map : Game::maps) {
		if (map.fileName == map_lower) {
			keyFound = true;
			break;
		}
	}

	if (rank < 1)
		return THREAD_PRINT("Invalid rank.\n");

	if (keyFound) {
		CURL *curl;
		CURLcode res;
		std::string buffer;
		curl = curl_easy_init();
		if (curl) {
			std::string apiCall = std::string(API_BASE_AUTORENDER) + "/v1/mtriggers/search?game_dir=portal2&map_name=" + map_name + "&board_rank=" + std::to_string(rank);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
			curl_easy_setopt(curl, CURLOPT_URL, apiCall.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				THREAD_PRINT("Failed to retrieve.\n");
				return curl_easy_cleanup(curl);
			} else {
				std::string err;
				auto json = json11::Json::parse(buffer.c_str(), err);

				if (!err.empty()) {
					THREAD_PRINT("Failed to retrieve.\n");
					return curl_easy_cleanup(curl);
				} else {
					if (json["data"].is_array()) {
						const json11::Json::array &dataArr = json["data"].array_items();
						if (dataArr.empty()) {
							THREAD_PRINT("No data.\n");
							return curl_easy_cleanup(curl);
						}
						const json11::Json::array &segmentArr = json["data"][0]["demo_metadata"]["segments"].array_items();
						if (segmentArr.empty()) {
							THREAD_PRINT("No segment data.\n");
							return curl_easy_cleanup(curl);
						}
						auto &splits = json["data"][0]["demo_metadata"]["segments"];
						float time = 0.0f;
						for (const auto &split : splits.array_items()) {
							auto ticks = split["ticks"].int_value();
							auto segmentTime = ticks * engine->GetIPT();
							time += ticks * engine->GetIPT();
							auto timeS        = SpeedrunTimer::Format(time);
							auto segmentTimeS = SpeedrunTimer::Format(segmentTime);
							THREAD_PRINT("[%s] - %s (%s) (%i)\n", split["name"].string_value().c_str(), timeS.c_str(), segmentTimeS.c_str(), ticks);
						}
						time = 0.0f;
					}
				}
			}
			curl_easy_cleanup(curl);
		}
	} else
		THREAD_PRINT("Invalid map name.\n");

	return;
}

CON_COMMAND_COMPLETION(sar_speedrun_get_mtriggers, "sar_speedrun_get_mtriggers <rank=wr> - prints mtriggers of specific run.\n", ({"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"})) {
	if (args.ArgC() != 2) {
		if (g_worker.joinable()) g_worker.join();
		g_worker = std::thread(retrieveMtriggers, 1, engine->GetCurrentMapName());
		return;
	}

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread(retrieveMtriggers, std::atoi(args[1]), engine->GetCurrentMapName());
}

CON_COMMAND_COMPLETION(sar_speedrun_get_mtriggers_map, "sar_speedrun_get_mtriggers_map <map=current> <rank=wr> - prints mtriggers of specific run on specific map.\n", (Portal2::mapNames)) {
	if (args.ArgC() != 3) {
		if (args.ArgC() == 2) {
			if (g_worker.joinable()) g_worker.join();
			g_worker = std::thread(retrieveMtriggers, 1, args[1]);
			return;
		}

		if (g_worker.joinable()) g_worker.join();
		g_worker = std::thread(retrieveMtriggers, 1, engine->GetCurrentMapName());
		return;
	}

	if (g_worker.joinable()) g_worker.join();
	auto rank = std::atoi(args[2]);
	g_worker = std::thread(retrieveMtriggers, rank, args[1]);
}


void AutoSubmit::FinishRun(float final_time, const char *demopath, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb, std::optional<int> portals) {
	if (AutoSubmit::g_cheated) {
		console->Print("Cheated; not autosubmitting\n");
		return;
	}

	int allowedPauses = 0;
	if (sar_disable_challenge_stats_hud.GetInt() != -1) allowedPauses = 1;
	if (AutoSubmit::g_paused > allowedPauses && engine->GetCurrentMapName() == "sp_a1_wakeup") {
		console->Print("Pause detected in Wakeup (pause abuse is not allowed in CM); not autosubmitting\nManually submit this demo if you believe this was a mistake\n");
		return;
	}	
	
	bool lp_supported = sar.game->Is(SourceGame_Portal2) || sar.game->Is(SourceGame_PortalStoriesMel);
	bool lp_enabled = portals && *portals >= 0 && !g_lphub_key.empty() && lp_supported;
	if (lp_supported && !lp_enabled) {
		if (g_lphub_key.empty()) {
			console->Print("LPHUB: skipping autosubmit; no token loaded. Check autosubmit.key and run sar_challenge_autosubmit_reload_api_key.\n");
		} else if (!portals) {
			console->Print("LPHUB: skipping autosubmit; no portal count available (singleplayer only).\n");
		} else {
			console->Print("LPHUB: skipping autosubmit; invalid portal count (%d).\n", *portals);
		}
	}
	int score = (int)floor(final_time * 100);
	int time_ticks = TIME_TO_TICKS(final_time);
	std::string map_name = engine->GetCurrentMapName();
	auto map_id = GetMapId(map_name);
	if (!map_id) console->Print("Unknown map; not autosubmitting\n");

	if (g_worker.joinable()) g_worker.join();
	std::string path(demopath), lp_key = g_lphub_key;
	g_worker = std::thread([=]() mutable {
		auto time_outcome = map_id
			? checkTimePb(score, *map_id)
			: PbOutcome::Unknown;
		// Preserve time naming for a PB or an unknown result (rename just in case).
		if (rename_if_pb && time_outcome != PbOutcome::NotPb) {
			renameDemo(path, *rename_if_pb, replay_append_if_pb);
		}
		bool coop = Utils::StartsWith(map_name.c_str(), "mp_");
		if (time_outcome == PbOutcome::Pb) {
			submitTime(score, path, coop, *map_id);
		}
		if (lp_enabled) {
			auto lp_outcome = checkLpPb(*portals, lp_key, map_name, time_ticks);
			// LP naming requires a confirmed time non-PB.
			if (time_outcome == PbOutcome::NotPb && lp_outcome == PbOutcome::Pb && rename_if_pb && replay_append_if_pb) {
				std::string suffix = "_" + std::to_string(*portals) + "p" + *replay_append_if_pb;
				std::string target = rename_if_pb->substr(0, rename_if_pb->size() - 4 - replay_append_if_pb->size()) + suffix + ".dem";
				renameDemo(path, target, suffix);
			}
			if (lp_outcome == PbOutcome::Pb) {
				submitLP(path, lp_key);
			}
		}
		// Event hooks may join this worker, so queue this after all other scheduler calls.
		if (time_outcome == PbOutcome::NotPb) {
			Scheduler::OnMainThread([=]() {
				Event::Trigger<Event::MAYBE_AUTOSUBMIT>({score, coop, false});
			});
		}
	});
}

ON_EVENT(SAR_UNLOAD) {
	if (g_worker.joinable()) g_worker.join();
	if (g_worker_search.joinable()) g_worker_search.join();
}
