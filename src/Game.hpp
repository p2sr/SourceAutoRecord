#pragma once

enum SourceGameVersion {
    SourceGame_Unknown = 0,

    SourceGame_Portal2 = (1 << 0),
    SourceGame_Portal = (1 << 1),
    SourceGame_TheStanleyParable = (1 << 2),
    SourceGame_TheBeginnersGuide = (1 << 3),
    SourceGame_HalfLife2 = (1 << 4),

    SourceGame_Portal2Engine = SourceGame_Portal2 | SourceGame_TheStanleyParable | SourceGame_TheBeginnersGuide,
    SourceGame_HalfLife2Engine = SourceGame_Portal | SourceGame_HalfLife2
};

class Game {
public:
    SourceGameVersion version;

public:
    virtual ~Game() = default;
    virtual void LoadOffsets() = 0;
    virtual const char* Version();
    virtual const float Tickrate() = 0;

    static Game* CreateNew();
};
