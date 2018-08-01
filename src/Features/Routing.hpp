#pragma once
#include <tuple>

#include "Utils.hpp"

namespace Routing {

namespace Tracer {

    enum ResultType {
        VEC2,
        VEC3
    };

    Vector Source;
    Vector Destination;

    void Start(Vector source)
    {
        Source = source;
    }
    void Stop(Vector destination)
    {
        Destination = destination;
    }
    void Reset()
    {
        Source = Vector();
        Destination = Vector();
    }
    std::tuple<float, float, float> GetDifferences()
    {
        return std::make_tuple(Destination.x - Source.x, Destination.y - Source.y, Destination.z - Source.z);
    }
    float GetResult(ResultType type)
    {
        auto x = Destination.x - Source.x;
        auto y = Destination.y - Source.y;
        auto z = Destination.z - Source.z;
        return (type == ResultType::VEC2)
            ? std::sqrt(x * x + y * y)
            : std::sqrt(x * x + y * y + z * z);
    }
}
}