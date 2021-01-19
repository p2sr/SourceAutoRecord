#pragma once

#include "Feature.hpp"

class GroundFramesCounter : public Feature {
public:
    int counter = 0;
    bool grounded = false;
public:
    GroundFramesCounter();
    void HandleJump();
    void HandleMovementFrame(bool newGrounded);
};

extern GroundFramesCounter* groundFramesCounter;
