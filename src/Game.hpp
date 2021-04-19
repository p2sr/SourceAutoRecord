#pragma once
#include <string>
#include <vector>

enum SourceGameVersion {
    SourceGame_Unknown = 0,

    SourceGame_Portal2 = (1 << 0),
    SourceGame_Portal = (1 << 1),
    SourceGame_TheStanleyParable = (1 << 2),
    SourceGame_TheBeginnersGuide = (1 << 3),
    SourceGame_HalfLife2 = (1 << 4),

    SourceGame_ApertureTag = (1 << 5),
    SourceGame_PortalStoriesMel = (1 << 6),
    SourceGame_ThinkingWithTimeMachine = (1 << 7),

    SourceGame_INFRA = (1 << 8),

    SourceGame_HalfLife2Episodic = (1 << 9),
    SourceGame_HalfLifeSource = (1 << 10),

    SourceGame_PortalReloaded = (1 << 11),

    SourceGame_Portal2Game = SourceGame_Portal2 | SourceGame_ApertureTag | SourceGame_PortalStoriesMel | SourceGame_ThinkingWithTimeMachine | SourceGame_PortalReloaded,
    SourceGame_Portal2Engine = SourceGame_Portal2Game | SourceGame_TheStanleyParable | SourceGame_TheBeginnersGuide | SourceGame_INFRA,
    SourceGame_HalfLife2Engine = SourceGame_Portal | SourceGame_HalfLife2 | SourceGame_HalfLife2Episodic | SourceGame_HalfLifeSource,

    SourceGame_SupportsS3 = SourceGame_Portal2Game | SourceGame_Portal,
};

class Game {
protected:
    SourceGameVersion version = SourceGame_Unknown;

public:
    virtual ~Game() = default;
    virtual void LoadOffsets() = 0;
    virtual const char* Version();
    virtual const float Tickrate() = 0;
    inline bool Is(int game) { return this->version & game; }
    inline SourceGameVersion GetVersion() { return this->version; }

    static Game* CreateNew();

    static std::string VersionToString(int version);
    static std::vector<std::string> mapNames;
};
