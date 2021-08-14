extern "C" {
#include <curl/curl.h>
};

#include "Command.hpp"
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
};

static std::optional<SarVersion> getVersionComponents(const char *str) {
	SarVersion v = {0};
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
	if (memcmp(str, "-pre", 4)) return {};
	str += 4;
	char *end;
	v.pre = strtol(str, &end, 10);
	if (str == end || *end) return {};
	return v;
}

static bool isNewerVersion(const char *verStr) {
	auto current = getVersionComponents(SAR_VERSION);
	auto version = getVersionComponents(verStr);

	if (!current) {
		console->Print("Cannot compare version numbers on non-release version\n");
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

	return res == CURLE_OK;
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

	return res == CURLE_OK ? response : "";
}

static bool getLatestVersion(std::string *name, std::string *dlUrl, bool allowPre) {
	json11::Json res;
	if (allowPre) {
		std::string err;
		res = json11::Json::parse(request("https://api.github.com/repos/p2sr/SourceAutoRecord/releases"), err);
		if (err != "") {
			return false;
		}
		res = res.array_items()[0];
	} else {
		std::string err;
		res = json11::Json::parse(request("https://api.github.com/repos/p2sr/SourceAutoRecord/releases/latest"), err);
		if (err != "") {
			return false;
		}
	}

	*name = res["tag_name"].string_value();

	if (*name == "") {
		return false;
	}

	for (auto asset : res["assets"].array_items()) {
		if (asset["name"].string_value() == ASSET_NAME) {
			*dlUrl = asset["browser_download_url"].string_value();
			break;
		}
	}

	if (*dlUrl == "") {
		return false;
	}

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

void checkUpdate(bool allowPre) {
	std::string name, dlUrl;

	console->Print("Querying for latest version...\n");

	if (!getLatestVersion(&name, &dlUrl, allowPre)) {
		console->Print("An error occurred\n");
		return;
	}

	console->Print("Latest version is %s\n", name.c_str());

	if (!isNewerVersion(name.c_str())) {
		console->Print("You're all up-to-date!\n");
	} else {
		console->Print("Update with sar_update, or at %s\n", dlUrl.c_str());
	}
}

void doUpdate(bool allowPre, bool exitOnSuccess, bool force) {
	std::string name, dlUrl;

	console->Print("Querying for latest version...\n");

	if (!getLatestVersion(&name, &dlUrl, allowPre)) {
		console->Print("An error occurred\n");
		return;
	}

	if (!force && !isNewerVersion(name.c_str())) {
		console->Print("You're already up-to-date!\n");
		return;
	}

	std::string sar = Utils::GetSARPath();
	std::string tmp = createTempPath(ASSET_NAME);

	// Step 1: download SAR to the given temporary file
	console->Print("Downloading SAR %s...\n", name.c_str());
	if (!downloadFile(dlUrl.c_str(), tmp.c_str())) {
		console->Print("An error occurred\n");
		return;
	}

	// Step 2: delete the current SAR image. For some reason, on Linux
	// we have to delete it, while on Windows we have to move it. Don't
	// ask because I don't know
	console->Print("Deleting old version...\n");
#ifdef _WIN32
	std::filesystem::rename(sar, "sar.dll.old-auto");
#else
	std::filesystem::remove(sar);
#endif

	// Step 3: copy the new SAR image to the location of the old one,
	// and then delete the temporary file. We can't just move it
	// for........reasons
	console->Print("Installing...\n", name.c_str());
	std::filesystem::copy(tmp, sar);
	std::filesystem::remove(tmp);

	console->Print("Success! You should now restart your game.\n");

	if (exitOnSuccess) {
		engine->ExecuteCommand("quit");
	}
}

CON_COMMAND(sar_check_update, "sar_check_update [release|pre] - check whether the latest version of SAR is being used\n") {
	if (args.ArgC() > 2) {
		return console->Print(sar_check_update.ThisPtr()->m_pszHelpString);
	}

	bool allowPre = args.ArgC() == 2 && !strcmp(args[1], "pre");

	if (g_worker.joinable()) g_worker.join();
	g_worker = std::thread(checkUpdate, allowPre);
}

CON_COMMAND(sar_update, "sar_update [release|pre] [exit] [force] - update SAR to the latest version. If exit is given, exit the game upon successful update; if force is given, always re-install, even if it may be a downgrade\n") {
	bool allowPre = false, exitOnSuccess = false, force = false;

	for (int i = 1; i < args.ArgC(); ++i) {
		if (!strcmp(args[i], "pre")) {
			allowPre = true;
		} else if (!strcmp(args[i], "release")) {
			allowPre = false;
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
	g_worker = std::thread(doUpdate, allowPre, exitOnSuccess, force);
}
