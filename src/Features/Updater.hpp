#pragma once

enum class Channel {
	Release,
	PreRelease,
	Canary,
};

void checkUpdate(Channel channel, bool print, std::function<void(int)> cb);
