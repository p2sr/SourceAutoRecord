#pragma once
#include <tuple>

#include "Features/Feature.hpp"

#include "Utils.hpp"

enum TracerResultType {
    VEC2,
    VEC3
};

class Tracer : public Feature {
public:
    Vector source;
    Vector destination;

public:
    Tracer();
    void Start(Vector source);
    void Stop(Vector destination);
    void Reset();
    std::tuple<float, float, float> GetDifferences();
    float GetResult(TracerResultType type);
};

extern Tracer* tracer;
