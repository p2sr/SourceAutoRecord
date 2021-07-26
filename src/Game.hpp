#pragma once
#include <string>
#include <vector>

enum SourceGameVersion {
	SourceGame_Unknown = 0,

	SourceGame_Portal2 = (1 << 0),
	SourceGame_ApertureTag = (1 << 5),
	SourceGame_PortalStoriesMel = (1 << 6),
	SourceGame_ThinkingWithTimeMachine = (1 << 7),
	SourceGame_PortalReloaded = (1 << 11),

#ifndef _WIN32
	SourceGame_EIPRelPIC = SourceGame_Portal2,
#endif
};

class Game {
protected:
	SourceGameVersion version = SourceGame_Unknown;

public:
	virtual ~Game() = default;
	virtual void LoadOffsets() = 0;
	virtual const char *Version();
	virtual const float Tickrate() = 0;
	inline bool Is(int game) { return this->version & game; }
	inline SourceGameVersion GetVersion() { return this->version; }

	static Game *CreateNew();

	static std::string VersionToString(int version);
	static std::vector<std::string> mapNames;
};
