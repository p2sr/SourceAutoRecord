#include "AutoSubmitMod.hpp"

#include "AutoSubmit.hpp"
#include "Cheats.hpp"
#include "Command.hpp"
#include "Event.hpp"
#include "Version.hpp"
#include "Features/Hud/Toasts.hpp"
#include "Features/NetMessage.hpp"
#include "Features/Session.hpp"
#include "Modules/Engine.hpp"
#include "Modules/FileSystem.hpp"
#include "Modules/Server.hpp"

#include <cctype>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#define AUTOSUBMIT_TOAST_TAG "autosubmit"
#define API_KEY_FILE "autosubmit.key"

static std::string g_api_base;
static std::string g_api_key;
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

static std::optional<std::string> request(CURL *curl, std::string url) {
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);

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

static void testApiKey() {
	if (!ensureCurlReady(&g_curl)) {
		THREAD_PRINT("Failed to test API key!\n");
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
		THREAD_PRINT("API key invalid!\n");
		return;
	}

	g_key_valid = true;
	THREAD_PRINT("API key valid!\n");

	response = request(g_curl, g_api_base + "/download-maps");
	if (!response) {
		THREAD_PRINT("Failed to download maps!\n");
		return;
	}

	std::string err;
	auto json = json11::Json::parse(*response, err);

	if (err != "") {
		return;
	}

	for (auto map : json["maps"].array_items()) {
		g_map_ids.insert({map["level_name"].string_value(), map["id"].string_value()});
	}

	THREAD_PRINT("Downloaded %i maps!\n", g_map_ids.size());
}

std::optional<std::string> AutoSubmitMod::GetMapId(std::string map_name) {
	auto it = g_map_ids.find(map_name);
	if (it == g_map_ids.end()) {
		return {};
	}

	return it->second;
}

std::optional<int> getCurrentPbScore(std::string &map_id) {
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

static void startSearching(const char *mapName) {
	auto map_id = AutoSubmitMod::GetMapId(std::string(mapName));
	if (!map_id.has_value()) {
		g_is_querying = false;
		return;	
	}

	auto json = AutoSubmitMod::GetTopScores(*map_id);

	g_times.clear();
	for (auto score : json) {
		g_times.push_back(score);
	}

	g_is_querying = false;
}

void AutoSubmitMod::Search(const char *map) {
	g_is_querying = true;

	if (g_worker_search.joinable()) g_worker_search.join();
	g_worker_search = std::thread(startSearching, map);
}

json11::Json::array AutoSubmitMod::GetTopScores(std::string &map_id) {
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

bool AutoSubmitMod::IsQuerying() {
	return g_is_querying;
}

const std::vector<json11::Json>& AutoSubmitMod::GetTimes() {
	return g_times;
}

static void submitTime(int score, std::string demopath, bool coop, std::string map_id, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb) {
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
			Scheduler::OnMainThread([=]() {
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
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time - couldn't fetch PB");
		return;
	}

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

	auto resp = request(g_curl, g_api_base + "/auto-submit");

	curl_mime_free(form);

	if (!resp) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time - didn't get a response");
	} else {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "Successfully submitted time to boards!");
	}
}

void AutoSubmitMod::LoadApiKey(bool output_nonexist) {
	auto filepath = fileSystem->FindFileSomewhere(API_KEY_FILE);
	if (!filepath.has_value()) {
		if (output_nonexist) {
			console->Print("API key file " API_KEY_FILE " does not exist!\n");
		}
		return;
	}

	std::string base;
	std::string key;

	{
		std::ifstream f(filepath.value());
		std::getline(f, base) && std::getline(f, key);
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

	g_api_base = "https://" + base + "/api-v2";
	g_api_key = key;
	g_key_valid = false;
	console->Print("Set API key! Testing...\n");

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread(testApiKey);
}

void AutoSubmitMod::FinishRun(float final_time, const char *demopath, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb) {
	if (AutoSubmit::g_cheated) {
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

	auto map_id = it->second;

	int score = (int)floor(final_time * 100);

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread(submitTime, score, std::string(demopath), Utils::StartsWith(engine->GetCurrentMapName().c_str(), "mp_"), map_id, rename_if_pb, replay_append_if_pb);
}

ON_EVENT(SAR_UNLOAD) {
	if (g_worker.joinable()) g_worker.join();
	if (g_worker_search.joinable()) g_worker_search.join();
}
