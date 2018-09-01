#pragma once
#include "Feature.hpp"

#include "Utils/SDK.hpp"

class Session : public Feature {
public:
    int baseTick;
    int lastSession;

    bool isInSession;
    unsigned currentFrame;
    unsigned lastFrame;
    HOSTSTATES prevState;

public:
    Session();
    void Rebase(const int from);

    void Started(bool menu = false);
    void Ended();
    void Changed();
    void Changed(int state);
};

extern Session* session;
