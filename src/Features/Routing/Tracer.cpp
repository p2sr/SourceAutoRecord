#include "Tracer.hpp"

#include "Command.hpp"
#include "Features/Hud/Hud.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Offsets.hpp"

#include <cstdlib>
#include <tuple>

Tracer *tracer;

Tracer::Tracer()
	: traces() {
	for (auto i = 0; i < Offsets::MAX_SPLITSCREEN_PLAYERS; ++i) {
		this->traces.push_back(new TraceResult());
	}
	this->hasLoaded = true;
}
Tracer::~Tracer() {
	for (const auto &trace : this->traces) {
		delete trace;
	}
	this->traces.clear();
}

TraceResult *Tracer::GetTraceResult(int nSlot) {
	return this->traces[nSlot];
}
void Tracer::Start(int nSlot, Vector source) {
	this->GetTraceResult(nSlot)->source = source;
}
void Tracer::Stop(int nSlot, Vector destination) {
	this->GetTraceResult(nSlot)->destination = destination;
}
void Tracer::Reset(int nSlot) {
	auto trace = this->GetTraceResult(nSlot);
	trace->source = Vector();
	trace->destination = Vector();
}
std::tuple<float, float, float> Tracer::CalculateDifferences(const TraceResult *trace) {
	return std::make_tuple(
		trace->destination.x - trace->source.x,
		trace->destination.y - trace->source.y,
		trace->destination.z - trace->source.z);
}
float Tracer::CalculateLength(const TraceResult *trace, TracerLengthType type) {
	auto x = trace->destination.x - trace->source.x;
	auto y = trace->destination.y - trace->source.y;
	auto z = trace->destination.z - trace->source.z;
	return (type == TracerLengthType::VEC2)
		? std::sqrt(x * x + y * y)
		: std::sqrt(x * x + y * y + z * z);
}

// Commands

CON_COMMAND(sar_trace_a, "sar_trace_a - saves location A for tracing\n") {
	auto nSlot = GET_SLOT();
	auto player = server->GetPlayer(nSlot + 1);
	if (player) {
		tracer->Start(nSlot, server->GetAbsOrigin(player));
		console->Print("Saved location A for tracing!\n");
	}
}
CON_COMMAND(sar_trace_b, "sar_trace_b - saves location B for tracing\n") {
	auto nSlot = GET_SLOT();
	auto player = server->GetPlayer(nSlot + 1);
	if (player) {
		tracer->Stop(nSlot, server->GetAbsOrigin(player));
		console->Print("Saved location B for tracing!\n");
	}
}
CON_COMMAND(sar_trace_result, "sar_trace_result - prints tracing result\n") {
	auto result = tracer->GetTraceResult(GET_SLOT());
	auto xyz = tracer->CalculateDifferences(result);
	console->Print("A: %.3f/%.3f/%.3f\n", result->source.x, result->source.y, result->source.z);
	console->Print("B: %.3f/%.3f/%.3f\n", result->destination.x, result->destination.y, result->destination.z);
	console->Print("dX: %.3f\n", std::get<0>(xyz));
	console->Print("dY: %.3f\n", std::get<1>(xyz));
	console->Print("dZ: %.3f\n", std::get<2>(xyz));
	console->Print("dXY: %.3f\n", tracer->CalculateLength(result, TracerLengthType::VEC2));
	console->Print("dXYZ: %.3f\n", tracer->CalculateLength(result, TracerLengthType::VEC3));
}
CON_COMMAND(sar_trace_reset, "sar_trace_reset - resets tracer\n") {
	tracer->Reset(GET_SLOT());
}

// HUD

HUD_ELEMENT_MODE2(trace, "0", 0, 2,
                  "Draws distance values of tracer. "
                  "0 = Default,\n"
                  "1 = Vec3,\n"
                  "2 = Vec2.\n",
                  HudType_InGame | HudType_Paused) {
	auto result = tracer->GetTraceResult(ctx->slot);
	auto xyz = tracer->CalculateDifferences(result);
	auto length = (mode == 1)
		? tracer->CalculateLength(result, TracerLengthType::VEC3)
		: tracer->CalculateLength(result, TracerLengthType::VEC2);
	ctx->DrawElement("trace: %.3f (%.3f/%.3f/%.3f)", length, std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz));
}
