#pragma once
#include "Feature.hpp"

class Session : public Feature {
public:
    int baseTick;
    int lastSession;

public:
    Session();
    void Rebase(const int from);
};

extern Session* session;
