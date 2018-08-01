#pragma once

namespace Stats {

enum ResultType {
    UNKNOWN,
    VEC2,
    VEC3
};

namespace Jumps {
    int Total;
    float Distance;
    float DistancePeak;
    ResultType Type;

    bool IsTracing;
    Vector Source;

    void StartTrace(Vector source)
    {
        Source = source;
        IsTracing = true;
    }
    void EndTrace(Vector destination, bool xyOnly)
    {
        auto x = destination.x - Source.x;
        auto y = destination.y - Source.y;

        if (xyOnly) {
            Distance = std::sqrt(x * x + y * y);
            Type = ResultType::VEC2;
        } else {
            auto z = destination.z - Source.z;
            Distance = std::sqrt(x * x + y * y + z * z);
            Type = ResultType::VEC3;
        }

        if (Distance > DistancePeak)
            DistancePeak = Distance;

        IsTracing = false;
    }
    void Reset()
    {
        Total = 0;
        Distance = 0;
        DistancePeak = 0;
        Type = ResultType::UNKNOWN;
        IsTracing = false;
    }
}
namespace Steps {
    int Total;

    void Reset()
    {
        Total = 0;
    }
}
namespace Velocity {
    float Peak;
    ResultType Type;

    void Save(Vector velocity, bool xyOnly)
    {
        float vel = 0;
        if (xyOnly) {
            vel = velocity.Length2D();
            Type = ResultType::VEC2;
        } else {
            vel = velocity.Length();
            Type = ResultType::VEC3;
        }

        if (vel > Peak)
            Peak = vel;
    }
    void Reset()
    {
        Peak = 0;
        Type = ResultType::UNKNOWN;
    }
}

void ResetAll()
{
    Jumps::Reset();
    Steps::Reset();
    Velocity::Reset();
}
}