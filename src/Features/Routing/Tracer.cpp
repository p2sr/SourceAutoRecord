#include "Tracer.hpp"

#include <tuple>

Tracer::Tracer()
    : source()
    , destination()
{
    this->hasLoaded = true;
}
void Tracer::Start(Vector source)
{
    this->source = source;
}
void Tracer::Stop(Vector destination)
{
    this->destination = destination;
}
void Tracer::Reset()
{
    this->source = Vector();
    this->destination = Vector();
}
std::tuple<float, float, float> Tracer::GetDifferences()
{
    return std::make_tuple(
        this->destination.x - this->source.x,
        this->destination.y - this->source.y,
        this->destination.z - this->source.z);
}
float Tracer::GetResult(TracerResultType type)
{
    auto x = this->destination.x - this->source.x;
    auto y = this->destination.y - this->source.y;
    auto z = this->destination.z - this->source.z;
    return (type == TracerResultType::VEC2)
        ? std::sqrt(x * x + y * y)
        : std::sqrt(x * x + y * y + z * z);
}

Tracer* tracer;
