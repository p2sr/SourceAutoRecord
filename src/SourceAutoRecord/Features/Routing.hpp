#pragma once
#include "Utils.hpp"

namespace Routing {

namespace Tracer {
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
    float GetResult()
    {
        auto x = Destination.x - Source.x;
        auto y = Destination.y - Source.y;
        auto z = Destination.z - Source.z;
        return std::sqrt(x * x + y * y + z * z);
    }
}
}