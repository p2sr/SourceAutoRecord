#pragma once
#include <unistd.h>

namespace Game
{
	enum SourceGame
	{
		Portal2		// Portal 2 7054
	};

	SourceGame Version;

	bool IsSupported()
	{
		char link[20];
		char temp[260] = {0};
		sprintf(link, "/proc/%d/exe", getpid());
		readlink(link, temp, sizeof(temp));

		std::string exe = std::string(temp);
		int index = exe.find_last_of("\\/");
		exe = exe.substr(index + 1, exe.length() - index);

		if (exe == "portal2_linux") {
			Version = SourceGame::Portal2;
		}
		else {
			return false;
		}
		return true;
	}
	const char* GetVersion() {
		switch (Version) {
		case 0:
			return "Portal 2 (7054)";
		}
		return "Unknown";
	}
}