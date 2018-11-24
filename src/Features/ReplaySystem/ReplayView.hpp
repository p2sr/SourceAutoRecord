#pragma once
#include <vector>

#include "ReplayFrame.hpp"

#include "Utils/SDK.hpp"

class ReplayView {
public:
    std::vector<ReplayFrame> frames;
    int playIndex;

public:
    ReplayView();
    bool Ended();
    void Reset();
    void Resize();
};
