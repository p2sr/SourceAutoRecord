#include "AutoSubmit.hpp"

#include "../Games/Portal2.hpp"
#include "Cheats.hpp"
#include "Command.hpp"
#include "Event.hpp"
#include "Features/Hud/Toasts.hpp"
#include "Features/NetMessage.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/FileSystem.hpp"
#include "Modules/Server.hpp"
#include "Version.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "Utils/stb_image.h"

#include <cctype>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>

#define API_BASE_AUTORENDER "https://autorender.portal2.sr/api"
#define AUTOSUBMIT_TOAST_TAG "autosubmit"
#define COOP_NAME_MESSAGE_TYPE "coop-name"
#define API_KEY_FILE "autosubmit.key"
#define OLD_API_KEY_FILE "autosubmit_key.txt"

bool AutoSubmit::g_cheated = false;
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
}

ON_EVENT(PRE_TICK) {
	if (sv_cheats.GetBool()) AutoSubmit::g_cheated = true;
}

static std::string g_api_base;
static std::string g_api_key;
static bool g_key_valid;
static CURL *g_curl;
static CURL *g_curl_search;
static std::thread g_worker;
static std::thread g_worker_search;
static std::map<std::string, std::string> g_map_ids;
static bool g_is_querying;
static std::vector<PortalLeaderboardItem_t> g_times;

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
		THREAD_PRINT("Failed to download maps!\n");
		return;
	}

	std::string err;
	auto json = json11::Json::parse(*response, err);

	if (err != "") {
		THREAD_PRINT("Failed to parse maps JSON: %s\n", err.c_str());
		return;
	}

	for (auto &map : json["maps"].array_items()) {
		g_map_ids.insert({map["level_name"].string_value(), map["id"].string_value()});
	}

	THREAD_PRINT("Downloaded %i maps!\n", g_map_ids.size());

	client->EnableCustomLeaderboards();
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

static uint8_t *getAvatar(std::string url) {
	if (!ensureCurlReady(&g_curl_search)) return {};

	auto response = request(g_curl_search, url);

	if (!response) return {};

	auto avatar = *response;
	uint8_t *bytes = (uint8_t *)avatar.c_str();
	size_t len = avatar.length();

	/* decode avatar jpg */
	int w, h;
	int channels;
	auto img = stbi_load_from_memory(bytes, len, &w, &h, &channels, 0);

	if (!img) return {};

	/* avatar doesnt have alpha channel, but IImage needs it */
	size_t size = w * h * channels;
	size_t new_size = w * h * 4;
	uint8_t *new_img = (uint8_t *)malloc(new_size);  // needs to be free'd somewhere!

	for (uint8_t *p = img, *new_p = new_img; p != img + size; p += channels, new_p += 4) {
		*(new_p + 0) = *(p + 0);
		*(new_p + 1) = *(p + 1);
		*(new_p + 2) = *(p + 2);
		*(new_p + 3) = 0xFF;
	}

	/* free old decoded jpg */
	stbi_image_free(img);

	return new_img;
}

static void startSearching(std::string mapName) {
	auto map_id = AutoSubmit::GetMapId(mapName);
	if (!map_id.has_value()) {
		g_is_querying = false;
		return;
	}

	auto json = AutoSubmit::GetTopScores(*map_id);

	/* move map into vector so we can sort it */
	std::vector<std::pair<std::string, json11::Json>> times;
	for (const auto &it : json) {
		times.push_back(it);
	}

	/* sort by rank */
	std::sort(times.begin(), times.end(), [](const std::pair<std::string, json11::Json> &lhs, const std::pair<std::string, json11::Json> &rhs) {
		return lhs.second["scoreData"]["playerRank"].int_value() < rhs.second["scoreData"]["playerRank"].int_value();
	});

	g_times.clear();
	size_t i = 0;
	for (const auto &time : times) {
		if (i == 40)
			break;

		PortalLeaderboardItem_t data;
		strncpy(data.name, time.second["userData"]["boardname"].string_value().c_str(), sizeof(data.name));
		strncpy(data.autorender, time.second["scoreData"]["autorender_id"].string_value().c_str(), sizeof(data.autorender));
		data.avatarTex = getAvatar(time.second["userData"]["avatar"].string_value());
		data.rank = time.second["scoreData"]["playerRank"].int_value();
		data.score = time.second["scoreData"]["score"].int_value();

		g_times.push_back(data);

		++i;
	}

	g_is_querying = false;
}

void AutoSubmit::Search(std::string map) {
	g_is_querying = true;

	if (g_worker_search.joinable()) g_worker_search.join();
	g_worker_search = std::thread(startSearching, map);
}

json11::Json::object AutoSubmit::GetTopScores(std::string &map_id) {
	if (!ensureCurlReady(&g_curl_search)) return {};

	auto response = request(g_curl_search, g_api_base.substr(0, g_api_base.length() - 6) + "chamber/" + map_id + "/json");

	if (!response) return {};

	std::string err;
	auto json = json11::Json::parse(*response, err);

	if (err != "") {
		return {};
	}

	return json.object_items();
}

bool AutoSubmit::IsQuerying() {
	return g_is_querying;
}

const std::vector<PortalLeaderboardItem_t> &AutoSubmit::GetTimes() {
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

	auto resp = request(g_curl, g_api_base +  "/auto-submit");

	curl_mime_free(form);

	if (!resp) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time - didn't get a response");
	} else {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "Successfully submitted time to boards!");
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
		/* if they use p2 common, the file is going to be a list of endpoints */
		while (!f.eof()) {
			std::getline(f, base) && std::getline(f, key);
			if (sar.game->Is(SourceGame_Portal2) && base == "board.portal2.sr") break;
			if (sar.game->Is(SourceGame_PortalStoriesMel) && base == "mel.board.portal2.sr") break;
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

	if (!valid) {
		console->Print("Invalid API key!\n");
		return;
	}

	g_api_base = "https://"	+ base + "/api-v2";
	g_api_key = key;
	g_key_valid = false;
	console->Print("Set API key! Testing...\n");

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread(testApiKey);
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


void AutoSubmit::FinishRun(float final_time, const char *demopath, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb) {
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
