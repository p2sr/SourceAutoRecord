#pragma once

namespace Game
{
	enum SourceGame
	{
		Portal2,	// Portal 2 6879
		INFRA		// INFRA 6905
	};

	SourceGame Version;

	bool IsSupported()
	{
		TCHAR temp[MAX_PATH];
		GetModuleFileName(NULL, temp, _countof(temp));
		std::string exe = std::string(temp);
		int index = exe.find_last_of("\\/");
		exe = exe.substr(index + 1, exe.length() - index);

		if (exe == "portal2.exe") {
			Version = SourceGame::Portal2;
		}
		else if (exe == "infra.exe") {
			Version = SourceGame::INFRA;
		}
		else {
			return false;
		}
		return true;
	}
	const char* GetVersion() {
		switch (Version) {
		case 0:
			return "Portal 2 (6879)";
		case 1:
			return "INFRA (6905)";
		}
		return "Unknown";
	}
}