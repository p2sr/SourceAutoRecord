#pragma once
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Tier1.hpp"

namespace TAS {

struct TasFrame {
    int FramesLeft;
    std::string Command;
};

std::vector<TasFrame> Frames;
bool IsRunning = false;
int BaseIndex = 0;

void AddFrame(int framesLeft, std::string command, bool relative = false)
{
    if (relative) {
        framesLeft += BaseIndex;
    } else {
        BaseIndex = framesLeft;
    }

    Frames.push_back(TasFrame{
        framesLeft,
        command });
}
void Reset()
{
    IsRunning = false;
    BaseIndex = 0;
    Frames.clear();
}
void Start()
{
    IsRunning = true;
}
}