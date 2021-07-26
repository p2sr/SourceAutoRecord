#pragma once
#include "Features/Feature.hpp"
#include "Utils.hpp"

#include <tuple>
#include <vector>

enum class TracerLengthType {
	VEC2,
	VEC3
};

struct TraceResult {
	Vector source = Vector();
	Vector destination = Vector();
};

class Tracer : public Feature {
private:
	std::vector<TraceResult *> traces;

public:
	Tracer();
	~Tracer();

	TraceResult *GetTraceResult(int nSlot);

	void Start(int nSlot, Vector source);
	void Stop(int nSlot, Vector destination);
	void Reset(int nSlot);

	std::tuple<float, float, float> CalculateDifferences(const TraceResult *trace);
	float CalculateLength(const TraceResult *trace, TracerLengthType type);
};

extern Tracer *tracer;
