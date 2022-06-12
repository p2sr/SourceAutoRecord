#include "FileSystem.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Surface.hpp"
#include "Console.hpp"
#include "Command.hpp"

#include <fstream>

FileSystem *fileSystem;

std::vector<std::string> FileSystem::GetSearchPaths() {
	std::vector<std::string> paths;

	char allpaths[4096];
	int len = fileSystem->GetSearchPath(fileSystem->g_pFullFileSystem->ThisPtr(), "GAME", false, allpaths, sizeof(allpaths));

	int start = 0;
	for (int i = 0; i <= len; i++) {
		if (i != len && allpaths[i] != ';') continue;

		std::string path(allpaths + start, i - start);
		paths.push_back(path);
		start = i + 1;
	}

	return paths;
}

bool FileSystem::FileExistsSomewhere(std::string filename) {
	std::vector<std::string> paths = GetSearchPaths();
	for (std::string path : paths) {
		if (std::ifstream(path + "/" + filename).good()) return true;
	}
	return false;
}

bool FileSystem::Init() {
	g_pFullFileSystem = Interface::Create(this->Name(), "VFileSystem017", false);

	if (g_pFullFileSystem != nullptr) {
		GetSearchPath = this->g_pFullFileSystem->Original<_GetSearchPath>(Offsets::GetSearchPath);
	}

	return this->hasLoaded = this->g_pFullFileSystem;
}
void FileSystem::Shutdown() {
	Interface::Delete(this->g_pFullFileSystem);
}
