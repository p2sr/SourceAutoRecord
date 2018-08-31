#pragma once

enum class SourceGame {
    Unknown,
    Portal2,
    Portal,
    TheStanleyParable,
    TheBeginnersGuide,
    HalfLife2
};

class Game {
public:
    SourceGame version;

public:
    virtual ~Game() = default;
    virtual void LoadOffsets() = 0;
    virtual void LoadRules() = 0;
    virtual const char* Version();

    bool IsPortal2Engine();
    bool IsHalfLife2Engine();
    bool HasChallengeMode();
    bool HasJumpDisabled();
    bool IsPortalGame();

    static Game* CreateNew();
};
