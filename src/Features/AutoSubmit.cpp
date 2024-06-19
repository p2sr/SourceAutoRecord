#include "AutoSubmit.hpp"

#include "../Games/Portal2.hpp"
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

#define API_BASE "https://board.portal2.sr/api-v2"
#define API_BASE_AUTORENDER "https://autorender.portal2.sr/api"
#define AUTOSUBMIT_TOAST_TAG "autosubmit"
#define COOP_NAME_MESSAGE_TYPE "coop-name"
#define API_KEY_FILE "autosubmit_key.txt"

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

static std::string g_api_key;
static bool g_key_valid;
static CURL *g_curl;
static std::thread g_worker;

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
		});

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

	auto response = request(API_BASE "/validate-user");

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

	auto response = request(API_BASE "/current-pb");

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
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time");
		return;
	}

	if (!ensureCurlReady()) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time");
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
		curl_mime_data(field, AutoSubmit::g_partner_name.c_str(), CURL_ZERO_TERMINATED);
	}

	curl_easy_setopt(g_curl, CURLOPT_MIMEPOST, form);

	auto resp = request(API_BASE "/auto-submit");

	curl_mime_free(form);

	if (!resp) {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "An error occurred submitting this time");
	} else {
		toastHud.AddToast(AUTOSUBMIT_TOAST_TAG, "Successfully submitted time to boards!");
	}
}

void AutoSubmit::LoadApiKey(bool output_nonexist) {
	auto filepath = fileSystem->FindFileSomewhere(API_KEY_FILE);
	if (!filepath.has_value()) {
		if (output_nonexist) {
			console->Print("API key file " API_KEY_FILE " does not exist!\n");
		}
		return;
	}

	std::string key;
	{
		std::ifstream f(filepath.value());
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

size_t writeCallback(void *contents, size_t size, size_t nmemb, std::string *buffer) {
	size_t realsize = size * nmemb;
	buffer->append((char *)contents, realsize);
	return realsize;
}

void retrieveMtriggers(int rank, std::string map_name) {
	bool keyFound = false;
	if (map_name.empty())
		return THREAD_PRINT("Not playing a map.\n");

	for (const auto &map : Game::maps) {
		if (map.fileName == map_name) {
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
							auto segmentTime = ticks * *engine->interval_per_tick;
							time += ticks * *engine->interval_per_tick;
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

	auto it = std::find_if(Game::maps.begin(), Game::maps.end(), [](const MapData &data) {
		return data.fileName == engine->GetCurrentMapName();
	});
	if (it == Game::maps.end()) {
		console->Print("Unknown map; not autosubmitting\n");
		if (rename_if_pb) {
			std::filesystem::rename(demopath, *rename_if_pb);
		}
		if (replay_append_if_pb) {
			engine->demoplayer->replayName += *replay_append_if_pb;
		}
		return;
	}

	const char *map_id = it->chamberId;
	if (std::string(map_id) == "") {
		console->Print("Map not on boards; not autosubmitting\n");
		if (rename_if_pb) {
			std::filesystem::rename(demopath, *rename_if_pb);
		}
		if (replay_append_if_pb) {
			engine->demoplayer->replayName += *replay_append_if_pb;
		}
		return;
	}

	int score = floor(final_time * 100);

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread(submitTime, score, std::string(demopath), Utils::StartsWith(engine->GetCurrentMapName().c_str(), "mp_"), map_id, rename_if_pb, replay_append_if_pb);
}

ON_EVENT(SAR_UNLOAD) {
	if (g_worker.joinable()) g_worker.detach();
}
