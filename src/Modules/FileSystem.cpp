#include "FileSystem.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Surface.hpp"
#include "Console.hpp"
#include "Command.hpp"
#include "Modules/Engine.hpp"

#include <filesystem>
#include <fstream>

#ifndef _WIN32
#	include <dirent.h>
#endif

FileSystem *fileSystem;

std::vector<std::string> FileSystem::GetSearchPaths() {
	std::vector<std::string> paths;

	char allpaths[4096];
	int len = GetSearchPath(g_pFullFileSystem->ThisPtr(), "GAME", false, allpaths, sizeof(allpaths));

	int start = 0;
	for (int i = 0; i <= len; i++) {
		if (i != len && allpaths[i] != ';' && allpaths[i] != '\0') continue;

		std::string path(allpaths + start, i - start);
		if (path != "") paths.push_back(path);
		start = i + 1;
	}

	paths.push_back(std::string(engine->GetGameDirectory()) + "/../"); // Portal 2/portal2 => Portal 2
	return paths;
}

bool FileSystem::FileExistsSomewhere(std::string filename) {
	return FindFileSomewhere(filename).has_value();
}

std::optional<std::string> FileSystem::FindFileSomewhere(std::string filepath, std::string gamedir /* = "" */) {
	auto paths = GetSearchPaths();
	if (gamedir.length() == 0) {
		for (auto path : paths) {
			if (std::filesystem::exists(path + filepath)) return path + filepath;
		}
	} else {
		for (auto path : paths) {
			// Fuzzy gamedir search
			if (strstr(path.c_str(), gamedir.c_str()) == NULL) continue; 
			if (std::filesystem::exists(path + filepath)) return path + filepath;
			break;
		}
	}
	return std::nullopt;
}

#ifdef _WIN32
std::string FileSystem::GetSaveDirectory() {
	return engine->GetSaveDirName();
}
#else
static std::string findSubCapitalization(const char *base, const char *sub) {
	DIR *d = opendir(base);
	if (!d) return std::string(sub);

	struct dirent *ent;
	while ((ent = readdir(d))) {
		if (!strcasecmp(ent->d_name, sub)) {
			closedir(d);
			return std::string(ent->d_name);
		}
	}

	closedir(d);
	return std::string(sub);
}
// Apparently, GetSaveDirName has the wrong capitalization sometimes
// kill me
std::string FileSystem::GetSaveDirectory() {
	std::string path = std::string(engine->GetGameDirectory()) + "/";
	std::string dir = findSubCapitalization(path.c_str(), "save");
	dir += (engine->GetSaveDirName() + 4);
	return dir;
}
#endif

bool FileSystem::MapExists(std::string name) {
	if (sar.game->Is(SourceGame_INFRA)) return true; // the maps are inside VPKs
	name = "maps/" + name;
	if (!Utils::EndsWith(name, ".bsp")) name += ".bsp";
	return FileExistsSomewhere(name);
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
