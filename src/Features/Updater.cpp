extern "C" {
#include <curl/curl.h>
};

#include "Command.hpp"
#include "Event.hpp"
#include "Features/Hud/Toasts.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Updater.hpp"
#include "Utils.hpp"
#include "Utils/json11.hpp"
#include "Version.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#ifdef _WIN32
#	define ASSET_NAME "sar.dll"
#	define PDB_ASSET_NAME "sar.pdb"
#	define PDB_PATH "sar.pdb"
#else
#	define ASSET_NAME "sar.so"
#endif

static CURL *g_curl;
static curl_slist *g_httpHdrs;
static std::thread g_worker;

#define MAX_VERSION_COMPONENTS 6

struct SarVersion {
	unsigned components[MAX_VERSION_COMPONENTS];
	unsigned pre;
	bool canary;
};

#define DL_SAR_HOST "https://dl.sar.portal2.sr"

enum class Channel {
	Release,
	PreRelease,
	Canary,
};

static std::optional<SarVersion> getVersionComponents(const char *str) {
	SarVersion v = {0, 0, false};
	v.pre = UINT_MAX;

	size_t i = 0;
	while (true) {
		char *end;
		v.components[i++] = strtol(str, &end, 10);
		str = end;

		if (*str == '.') {
			++str;
		} else {
			break;
		}

		if (i == MAX_VERSION_COMPONENTS) {
			return {};
		}
	}

	if (!*str) return v;

	if (Utils::EndsWith(std::string(str), "-canary")) {
		v.canary = true;
		return v;
	}

	if (memcmp(str, "-pre", 4)) {
		return {};
	}
	str += 4;
	char *end;
	v.pre = strtol(str, &end, 10);
	if (str == end || *end) return {};
	return v;
}

static bool isNewerVersion(std::string& verStr) {
	auto version = getVersionComponents(verStr.c_str());
	auto current = getVersionComponents(SAR_VERSION);

	if ((version && version->canary) || (current && current->canary)) {
		return strcmp(SAR_VERSION, verStr.c_str()) != 0;
	}

	if (!current) {
		THREAD_PRINT("Cannot compare version numbers on non-release version\n");
		return false;
	}

	if (!version) {
		return false;
	}

	for (size_t i = 0; i < MAX_VERSION_COMPONENTS; ++i) {
		if (version->components[i] > current->components[i]) {
			return true;
		} else if (version->components[i] < current->components[i]) {
			return false;
		}
	}

	return version->pre > current->pre;
}

static bool curlPrepare(const char *url, int timeout) {
	if (!g_curl) {
		g_curl = curl_easy_init();

		if (!g_curl) {
			return false;
		}

		g_httpHdrs = curl_slist_append(g_httpHdrs, "Accept: application/vnd.github.v3+json");
		g_httpHdrs = curl_slist_append(g_httpHdrs, "User-Agent: SourceAutoRecord");
	}

	curl_easy_setopt(g_curl, CURLOPT_URL, url);
	curl_easy_setopt(g_curl, CURLOPT_NOPROGRESS, 1);
	curl_easy_setopt(g_curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(g_curl, CURLOPT_TIMEOUT, timeout);
	curl_easy_setopt(g_curl, CURLOPT_HTTPHEADER, g_httpHdrs);

	return true;
}

static bool downloadFile(const char *url, const char *path) {
	if (!curlPrepare(url, 60)) {
		return false;
	}

	FILE *f = fopen(path, "wb");

	if (!f) {
		return false;
	}

	curl_easy_setopt(g_curl, CURLOPT_WRITEFUNCTION, &fwrite);
	curl_easy_setopt(g_curl, CURLOPT_WRITEDATA, f);

	CURLcode res = curl_easy_perform(g_curl);

	fclose(f);

	long code;
	curl_easy_getinfo(g_curl, CURLINFO_RESPONSE_CODE, &code);

	return res == CURLE_OK && code == 200;
}

static std::string request(const char *url) {
	if (!curlPrepare(url, 10)) {
		return "";
	}

	curl_easy_setopt(
		g_curl, CURLOPT_WRITEFUNCTION, +[](void *ptr, size_t sz, size_t nmemb, std::string *data) -> size_t {
			data->append((char *)ptr, sz * nmemb);
			return sz * nmemb;
		});

	std::string response;
	curl_easy_setopt(g_curl, CURLOPT_WRITEDATA, &response);

	CURLcode res = curl_easy_perform(g_curl);

	long code;
	curl_easy_getinfo(g_curl, CURLINFO_RESPONSE_CODE, &code);

	return res == CURLE_OK && code == 200 ? response : "";
}

static bool getLatestVersion(std::string *name, std::string *dlUrl, std::string *pdbUrl, Channel channel) {
	// FIXME: This will fail if the API rate limit is saturated (bruteforcing? many instances?)
	//        maybe cache the response for a couple minutes - AMJ 2024-04-25
	json11::Json res;
	if (channel == Channel::PreRelease) {
		std::string err;
		res = json11::Json::parse(request("https://api.github.com/repos/p2sr/SourceAutoRecord/releases"), err);
		if (err != "") {
			return false;
		}
		try {
			res = res.array_items()[0];
		} catch (...) {
			return false;
		}
	} else if (channel == Channel::Canary) {
		std::string err;
		res = json11::Json::parse(request(DL_SAR_HOST "/api/v1/latest/canary"), err);
		if (err != "") {
			return false;
		}
		try {
			*name = res["sar_version"].string_value();

			auto url = std::string(DL_SAR_HOST "/") + res["version"].string_value();
#if _WIN32
			auto systemPath = "/windows/";
			*pdbUrl = url + systemPath + PDB_ASSET_NAME;
#else
			auto systemPath = "/linux/";
#endif
			*dlUrl = url + systemPath + ASSET_NAME;
			return true;
		} catch (...) {
			return false;
		}
	} else {
		std::string err;
		res = json11::Json::parse(request("https://api.github.com/repos/p2sr/SourceAutoRecord/releases/latest"), err);
		if (err != "") {
			return false;
		}
	}

	try {
		*name = res["tag_name"].string_value();
	} catch (...) {
		return false;
	}

	if (*name == "") {
		return false;
	}

	for (auto asset : res["assets"].array_items()) {
		if (asset["name"].string_value() == ASSET_NAME) {
			*dlUrl = asset["browser_download_url"].string_value();
		}
#ifdef _WIN32
		if (asset["name"].string_value() == PDB_ASSET_NAME) {
			*pdbUrl = asset["browser_download_url"].string_value();
		}
#endif
	}

	if (*dlUrl == "") {
		return false;
	}

#ifdef _WIN32
	if (*pdbUrl == "") {
		return false;
	}
#endif

	return true;
}

static std::string createTempPath(const char *filename) {
	auto base = std::filesystem::temp_directory_path().append(filename);
	if (!std::filesystem::exists(base)) {
		return base.string();
	}

	int n = 0;
	std::filesystem::path p;

	do {
		p = base.concat(Utils::ssprintf("_%d", n++));
	} while (std::filesystem::exists(p));

	return p.string();
}

void checkUpdate(Channel channel) {
	std::string name, dlUrl, pdbUrl;

	THREAD_PRINT("Querying for latest version...\n");

	if (!getLatestVersion(&name, &dlUrl, &pdbUrl, channel)) {
		THREAD_PRINT("An error occurred\n");
		return;
	}

	THREAD_PRINT("Latest version is %s\n", name.c_str());

	if (!isNewerVersion(name)) {
		THREAD_PRINT("You're all up-to-date!\n");
	} else {
		THREAD_PRINT("Update with sar_update, or at %s\n", dlUrl.c_str());
	}
}

void doUpdate(Channel channel, bool exitOnSuccess, bool force) {
	std::string name, dlUrl, pdbUrl;

	THREAD_PRINT("Querying for latest version...\n");

	if (!getLatestVersion(&name, &dlUrl, &pdbUrl, channel)) {
		THREAD_PRINT("An error occurred\n");
		return;
	}

	if (!force && !isNewerVersion(name)) {
		THREAD_PRINT("You're already up-to-date!\n");
		return;
	}

	std::string sar = Utils::GetSARPath();
	std::string tmp = createTempPath(ASSET_NAME);
#ifdef _WIN32
	std::string tmpPdb = createTempPath(PDB_ASSET_NAME);
#endif

	// Step 1: download SAR to the given temporary file
	THREAD_PRINT("Downloading SAR %s...\n", name.c_str());
	if (!downloadFile(dlUrl.c_str(), tmp.c_str())) {
		THREAD_PRINT("An error occurred\n");
		return;
	}

#ifdef _WIN32
	if (!downloadFile(pdbUrl.c_str(), tmpPdb.c_str())) {
		THREAD_PRINT("An error occurred\n");
		return;
	}
#endif

	// Step 2: delete the current SAR image. For some reason, on Linux
	// we have to delete it, while on Windows we have to move it. Don't
	// ask because I don't know
	THREAD_PRINT("Deleting old version...\n");
#ifdef _WIN32
	std::filesystem::rename(sar, "sar.dll.old-auto");
	if (std::filesystem::exists(PDB_PATH)) {
		std::filesystem::rename(PDB_PATH, "sar.pdb.old-auto");
	}
#else
	std::filesystem::remove(sar);
#endif

	// Step 3: copy the new SAR image to the location of the old one,
	// and then delete the temporary file. We can't just move it
	// for........reasons
	THREAD_PRINT("Installing...\n", name.c_str());
	std::filesystem::copy(tmp, sar);
	std::filesystem::remove(tmp);
#ifdef _WIN32
	std::filesystem::copy(tmpPdb, PDB_PATH);
	std::filesystem::remove(tmpPdb);
#endif

	THREAD_PRINT("Success! You should now restart your game.\n");

	if (exitOnSuccess) {
		Scheduler::OnMainThread([]() {
			toastHud.AddToast("update", "SAR has been updated. Your game will now exit.\n");
			Scheduler::InHostTicks(120, []() {
				engine->ExecuteCommand("quit");
			});
		});
	}
}

CON_COMMAND(sar_check_update, "sar_check_update [release|pre|canary] - check whether the latest version of SAR is being used\n") {
	if (args.ArgC() > 2) {
		return THREAD_PRINT(sar_check_update.ThisPtr()->m_pszHelpString);
	}

	auto channel = Channel::Release;

	if (!strcmp(args[1], "pre")) {
		channel = Channel::PreRelease;
	} else if (!strcmp(args[1], "pre")) {
		channel = Channel::Canary;
	}

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread(checkUpdate, channel);
}

CON_COMMAND(sar_update, "sar_update [release|pre|canary] [exit] [force] - update SAR to the latest version. If exit is given, exit the game upon successful update; if force is given, always re-install, even if it may be a downgrade\n") {
	auto channel = Channel::Release;
	auto exitOnSuccess = false;
	auto force = false;

	for (int i = 1; i < args.ArgC(); ++i) {
		if (!strcmp(args[i], "pre")) {
			channel = Channel::PreRelease;
		} else if (!strcmp(args[i], "canary")) {
			channel = Channel::Canary;
		} else if (!strcmp(args[i], "release")) {
			channel = Channel::Release;
		} else if (!strcmp(args[i], "exit")) {
			exitOnSuccess = true;
		} else if (!strcmp(args[i], "force")) {
			force = true;
		} else {
			console->Print("Invalid argument '%s'\n", args[i]);
			console->Print(sar_update.ThisPtr()->m_pszHelpString);
			return;
		}
	}

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread(doUpdate, channel, exitOnSuccess, force);
}

ON_EVENT(SAR_UNLOAD) {
	if (g_worker.joinable()) g_worker.join();
}
