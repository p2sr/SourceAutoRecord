#pragma once

#include <curl/curl.h>

enum class Channel {
	Release,
	PreRelease,
	Canary,
};

void checkUpdate(CURL *curl, Channel channel, bool print, std::function<void(int)> cb);
