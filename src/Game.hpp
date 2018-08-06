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
    virtual void LoadOffsets() = 0;
    virtual void LoadRules() = 0;
    virtual const char* Version();
    static bool IsPortal2Engine();
    static bool IsHalfLife2Engine();
    static bool HasChallengeMode();
    static bool HasJumpDisabled();
    static bool IsPortalGame();
    static bool IsSupported();
};

extern Game* game;
