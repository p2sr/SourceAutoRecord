#pragma once
#include <string>
#include <vector>

#include "Features/Feature.hpp"

#include "Variable.hpp"

struct CommandFrame {
    int FramesLeft;
    std::string Command;
};

class CommandQueuer : public Feature {
public:
    std::vector<CommandFrame> frames;
    bool isRunning;
    int baseIndex;

public:
    CommandQueuer();
    void AddFrame(int framesLeft, std::string command, bool relative = false);
    void AddFrames(int framesLeft, int interval, int lastFrame, std::string command, bool relative = false);
    void Stop();
    void Reset();
    void Start();
};

extern CommandQueuer* tasQueuer;

extern Variable sar_tas_autostart;
