#pragma once
#include "Feature.hpp"

#include "Utils/SDK.hpp"

enum class ResultType {
    UNKNOWN,
    VEC2,
    VEC3
};

class JumpStats {
public:
    int total;
    float distance;
    float distancePeak;
    ResultType type;

    bool isTracing;
    Vector source;

public:
    void StartTrace(Vector source);
    void EndTrace(Vector destination, bool xyOnly);
    void Reset();
};

class StepStats {
public:
    int total;

public:
    void Reset();
};

class VelocityStats {
public:
    float peak;
    ResultType type;

public:
    void Save(Vector velocity, bool xyOnly);
    void Reset();
};

class Stats : public Feature {
public:
    JumpStats* jumps;
    StepStats* steps;
    VelocityStats* velocity;

public:
    Stats();
    ~Stats();
    void ResetAll();
};

extern Stats* stats;
