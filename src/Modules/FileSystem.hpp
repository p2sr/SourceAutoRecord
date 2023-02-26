#pragma once
#include "Interface.hpp"
#include "Module.hpp"
#include "Utils.hpp"

// TODO: Custom fonts
class FileSystem : public Module {
public:
	Interface *g_pFullFileSystem = nullptr;

	using _GetSearchPath = int(__rescall*)(void* thisptr, const char *pathID, bool bGetPackFiles, char *pPath, int nMaxLen );

	_GetSearchPath GetSearchPath = nullptr;

public:
	std::vector<std::string> GetSearchPaths();
	bool FileExistsSomewhere(std::string filename);
	std::string GetSaveDirectory();
	bool MapExists(std::string name);

public:
	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("filesystem_stdio"); }
};

extern FileSystem *fileSystem;
