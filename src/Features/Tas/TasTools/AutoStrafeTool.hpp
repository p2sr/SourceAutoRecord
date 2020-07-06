#pragma once

#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

enum AutoStraferType {
    NONE,
    VECTORIAL,
    ANGULAR
};

enum AutoStraferDirectionType {
    CURRENT,
    LEFT,
    RIGHT,
    SPECIFIED
};

struct AutoStraferDirection {
    AutoStraferDirectionType type;
    float angle;
    bool processed = false;
};

struct AutoStraferSpeed {
    bool max;
    float speed;
};

struct AutoStraferParams : public TasToolParams {
    AutoStraferType strafeType = NONE;
    AutoStraferDirection strafeDir{CURRENT, 0};
    AutoStraferSpeed strafeSpeed{true, 0};
    AutoStraferParams() : TasToolParams() {}
};


class AutoStrafeTool : public TasTool {
public:
    AutoStrafeTool(const char* name) : TasTool(name){};
    virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
    virtual void Apply(TasFramebulk& fb); 
    virtual void Reset();
};

extern AutoStrafeTool autoStrafeTool;