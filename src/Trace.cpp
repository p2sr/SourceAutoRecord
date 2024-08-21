#include "Trace.hpp"

#include "Command.hpp"
#include "Variable.hpp"
#include "Event.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Client.hpp"
#include "Modules/Server.hpp"
#include "Features/Session.hpp"
#include "Features/Camera.hpp"
#include "Features/Tas/TasPlayer.hpp"

#include <map>
#include <string>

using namespace Trace;

Variable sar_trace_record("sar_trace_record", "0", "Record the trace to a slot. Set to 0 for not recording\n", 0);
Variable sar_trace_autoclear("sar_trace_autoclear", "1", 0, 1, "Automatically clear the trace on session start\n");
Variable sar_trace_override("sar_trace_override", "1", 0, 1, "Clears old trace when you start recording to it instead of recording on top of it.\n");

Variable sar_trace_use_shot_eyeoffset("sar_trace_use_shot_eyeoffset", "1", 0, 1, "Uses eye offset and angles accurate for portal shooting.\n");

std::map<std::string, TraceData> g_traces;
std::string g_lastRecordedTrace = "0";

Vector g_playerTraceTeleportLocation;
int g_playerTraceTeleportSlot;
bool g_playerTraceNeedsTeleport = false;

TraceData *Trace::GetByName(std::string name) {
	auto trace_it = g_traces.find(name);
	if (trace_it == g_traces.end()) return nullptr;
	return &trace_it->second;
}

TraceData *Trace::GetOrCreateByName(std::string name) {
	auto trace = GetByName(name);
    if (!trace) trace = CreateByName(name);
    return trace;
}

TraceData *Trace::CreateByName(std::string name) {
	if (!IsTraceNameValidForRecording(name)) {
		return nullptr;
	}

    auto trace = new TraceData();
	trace->startSessionTick = session->GetTick();
    g_traces[name] = *trace;
    return trace;
}

TraceData *Trace::GetCurrent() {
	auto trace_name = sar_trace_record.GetString();
	return GetByName(trace_name);
}

TraceData *Trace::GetOrCreateCurrent() {
	auto trace_name = sar_trace_record.GetString();
	return GetOrCreateByName(trace_name);
}

void Trace::ClearByName(std::string name) {
	g_traces.erase(name);
}

void Trace::ClearAll() {
	g_traces.clear();
}

void CorrectStartTasTickForTrace(TraceData &trace) {
	// update this bad boy every tick because it doesn't like being tinkered with at the
	// very beginning of the level. fussy guy, lemme tell ya
	if (tasPlayer->IsRunning()) {
		// include point we're about to add
		int ticksSinceStartup = (int)trace.players[0].size() + (int)(tasPlayer->GetTick() == 0);
		trace.startTasTick = tasPlayer->GetTick() - ticksSinceStartup;
	}
}

PlayerState GetPlayerStateForClient(void *player, int slot) {
	bool grounded = CE(player)->ground_entity();
	bool ducked = CE(player)->ducked();
	Vector pos = client->GetAbsOrigin(player);
	Vector vel = client->GetLocalVelocity(player);

	Vector eyepos;
	QAngle angles;
	camera->GetEyePos<false>(slot, eyepos, angles);

	return {pos, eyepos, angles, vel, grounded, ducked};
}

PlayerState GetPlayerStateForServer(void *player, int slot) {
	bool grounded = SE(player)->ground_entity();
	bool ducked = SE(player)->ducked();
	Vector pos = server->GetAbsOrigin(player);
	Vector vel = server->GetLocalVelocity(player);
	
	Vector eyepos;
	QAngle angles;
	camera->GetEyePos<true>(slot, eyepos, angles);

	return {pos, eyepos, angles, vel, grounded, ducked};
}

PlayerState GetPlayerState(void* player, int slot, bool client_side) {
    return client_side ? GetPlayerStateForClient(player, slot) : GetPlayerStateForServer(player, slot);
}

void TraceData::RecordNewPoint(void *player, int slot, bool server_side) {
	CorrectStartTasTickForTrace(*this);

	PlayerState player_state = GetPlayerState(player, slot, server_side);
	this->players[slot].push_back(player_state);

	// Only do it for one of the slots since we record all the portals in the map at once
	if (slot == 0) {
		auto portals = Trace::PortalsList::FetchCurrentLocations();
		this->portals.push_back(portals);
		auto hitboxes = Trace::HitboxList::FetchAllNearPosition(player_state.position);
		this->hitboxes.push_back(hitboxes);
	}
}

bool IsTraceNameValidForRecording(std::string name) {
	// for legacy reasons, 0 is treated as no recording
	if (name == "0") return false;
	return name.length() > 0;
}

bool Trace::ShouldRecord() {
	return IsTraceNameValidForRecording(sar_trace_record.GetString()) 
		&& !engine->IsGamePaused();
}

void OnTraceChanged() {
	if (sar_trace_override.GetBool()) {
		Trace::ClearByName(sar_trace_record.GetString());
	}
}

void CheckTraceChanged() {
	auto currentTrace = sar_trace_record.GetString();
	if (currentTrace != g_lastRecordedTrace) {
		g_lastRecordedTrace = currentTrace;
		OnTraceChanged();
	}
}

bool TryPreventTraceRecordingForOrange() {
	if (engine->IsOrange()) {
		sar_trace_record.SetValue(0);
		console->Print("The trace only works for the host! Turning off trace recording.\n");
		return true;
	}
	return false;
}

void RecordCurrentTrace(int slot, bool server_side) {
	if (!ShouldRecord()) return;
	if (TryPreventTraceRecordingForOrange()) return;

	auto trace = GetOrCreateCurrent();
	void *player = server_side 
		? (void*)server->GetPlayer(slot + 1) 
		: (void*)client->GetPlayer(slot + 1);

    if (trace && player) {
		trace->RecordNewPoint(player, slot, server_side);
    }
}

void HandleCachedTraceTeleport(CMoveData* moveData, int slot) {
	// We edit pos after process movement to get accurate teleportation
	// This is for sar_trace_teleport_at
	if (g_playerTraceNeedsTeleport && slot == g_playerTraceTeleportSlot) {
		moveData->m_vecAbsOrigin = g_playerTraceTeleportLocation;
		g_playerTraceNeedsTeleport = false;
	}
}

void TweakLatestTraceEyeOffsetForPortalShot(CMoveData *moveData, int slot, bool clientside) {
	if (!sar_trace_use_shot_eyeoffset.GetBool()) return;
	if (!Trace::ShouldRecord()) return;

	TraceData *trace = Trace::GetCurrent();
	if (trace == nullptr) return;

	// portal shooting position is funky. Basically, shooting happens after movement
	// has happened, but before angles or eye offset are updated (portal gun uses different
	// values for angles than movement, which makes it even weirder). The solution here is
	// to record eye offset right after movement tick processing has happened
	Vector eyepos;
	QAngle angles;
	if (clientside) {
		camera->GetEyePosFromOrigin<false>(slot, moveData->m_vecAbsOrigin, eyepos, angles);
	} else {
		camera->GetEyePosFromOrigin<true>(slot, moveData->m_vecAbsOrigin, eyepos, angles);
	}
	int lastTick = trace->players[slot].size() - 1;
	trace->players[slot][lastTick].eye_position = eyepos;
}

#pragma region Events

ON_EVENT(PROCESS_MOVEMENT) {
	CheckTraceChanged();
	RecordCurrentTrace(event.slot, event.server);
}

ON_EVENT(POST_PROCESS_MOVEMENT) {
	TweakLatestTraceEyeOffsetForPortalShot(event.moveData, event.slot, !event.server);
	HandleCachedTraceTeleport(event.moveData, event.slot);
}

ON_EVENT(SESSION_START) {
	if (sar_trace_autoclear.GetBool()) {
		Trace::ClearAll();
	}
}

#pragma region Commands

CON_COMMAND(sar_trace_clear, "sar_trace_clear <name> - Clear player trace with a given name\n") {
	if (args.ArgC() != 2)
		return console->Print(sar_trace_clear.ThisPtr()->m_pszHelpString);

	Trace::ClearByName(args[1]);
}

CON_COMMAND(sar_trace_clear_all, "sar_trace_clear_all - Clear all the traces\n") {
	Trace::ClearAll();
}

#pragma endregion