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
	SourceGame_INFRA = (1 << 12),
	SourceGame_BeginnersGuide = (1 << 13),
	SourceGame_StanleyParable = (1 << 14),

#ifndef _WIN32
	SourceGame_EIPRelPIC = SourceGame_Portal2,
#endif
};

struct AchievementData {
	const char *keyName;
	const char *displayName;
	bool coop;
	bool claimed = false;
};

struct MapData {
	const char *fileName;
	const char *displayName;
	const char *chamberId = ""; // For boards

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
	static std::vector<MapData> maps;
	static std::vector<AchievementData> achievements;

	static bool IsSpeedrunMod();
};
