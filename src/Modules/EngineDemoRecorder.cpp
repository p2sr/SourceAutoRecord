#include "EngineDemoRecorder.hpp"

#include "Features/Rebinder.hpp"
#include "Features/Session.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/TimescaleDetect.hpp"
#include "Features/FovChanger.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"

#include "Console.hpp"
#include "Engine.hpp"
#include "Server.hpp"

#include "Command.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Checksum.hpp"

#include <cstdio>

REDECL(EngineDemoRecorder::SetSignonState);
REDECL(EngineDemoRecorder::StartRecording);
REDECL(EngineDemoRecorder::StopRecording);
REDECL(EngineDemoRecorder::RecordCustomData);
REDECL(EngineDemoRecorder::stop_callback);

int EngineDemoRecorder::GetTick()
{
    return this->GetRecordingTick(this->s_ClientDemoRecorder->ThisPtr());
}

std::string EngineDemoRecorder::GetDemoFilename()
{
#ifdef _WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif
    return std::string(engine->GetGameDirectory()) + PATH_SEP + this->currentDemo + ".dem";
#undef PATH_SEP
}

bool needToRecordInitialVals = false;

static void RecordInitialVal(const char* name)
{
    size_t nameLen = strlen(name);

    const char* val = Variable(name).GetString();
    size_t valLen = strlen(val);

    size_t bufLen = nameLen + valLen + 3;

    char* buf = (char*)malloc(bufLen);
    buf[0] = 0x02;
    strcpy(buf + 1, name);
    buf[nameLen + 1] = 0x00;
    strcpy(buf + nameLen + 2, val);
    buf[nameLen + valLen + 2] = 0x00;

    engine->demorecorder->RecordData(buf, bufLen);

    free(buf);
}

// CDemoRecorder::SetSignonState
DETOUR(EngineDemoRecorder::SetSignonState, int state)
{
    bool wasRecording = engine->demorecorder->isRecordingDemo;

    //SIGNONSTATE_FULL is set twice during first CM load. Using SINGONSTATE_SPAWN for demo number increase instead
    if (state == SIGNONSTATE_SPAWN) {
        if (engine->demorecorder->isRecordingDemo || *engine->demorecorder->m_bRecording || sar_record_at_increment.GetBool()) {
            engine->demorecorder->lastDemoNumber++;
        }
    }
    if (state == SIGNONSTATE_FULL) {
        //autorecording in different session (save deletion)
        if (engine->demorecorder->isRecordingDemo) {
            *engine->demorecorder->m_bRecording = true;
        }

        if (*engine->demorecorder->m_bRecording) {
            engine->demorecorder->isRecordingDemo = true;
            *engine->demorecorder->m_nDemoNumber = engine->demorecorder->lastDemoNumber;
            engine->demorecorder->currentDemo = std::string(engine->demorecorder->m_szDemoBaseName);

            if (*engine->demorecorder->m_nDemoNumber > 1) {
                engine->demorecorder->currentDemo += std::string("_") + std::to_string(*engine->demorecorder->m_nDemoNumber);
            }
        }
    }

    // When we hit SIGNONSTATE_NEW, the recording is about to start, but
    // the custom data handler info won't be written immediately.
    // Therefore, suppress any custom data til SIGNONSTATE_FULL happens

    if (state == SIGNONSTATE_NEW) {
        engine->demorecorder->customDataReady = false;
    }

    if (state == SIGNONSTATE_FULL) {
        engine->demorecorder->customDataReady = true;
    }

    auto result = EngineDemoRecorder::SetSignonState(thisptr, state);

    if (wasRecording && state == SIGNONSTATE_NEW) {
        // We've switched level, the old demo has just finished recording
        std::string lastName = std::string(engine->GetGameDirectory()) + "/" + engine->demorecorder->m_szDemoBaseName;
        if (engine->demorecorder->lastDemoNumber > 1) {
            lastName += std::string("_") + std::to_string(engine->demorecorder->lastDemoNumber);
        }

        lastName += ".dem";

        if (!AddDemoChecksum(lastName.c_str())) {
            // TODO: report failure?
        }

        timescaleDetect->Spawn();
        needToRecordInitialVals = true;
    }

    if (state == SIGNONSTATE_FULL && needToRecordInitialVals) {
        needToRecordInitialVals = false;
        RecordInitialVal("host_timescale");
        RecordInitialVal("m_yaw");
        RecordInitialVal("cl_fov");
        RecordInitialVal("sv_allow_mobile_portals");
        RecordInitialVal("fps_max");
        RecordInitialVal("sv_portal_placement_debug");
        RecordInitialVal("sv_use_trace_duration");
        RecordInitialVal("sv_alternateticks");
        RecordInitialVal("cl_cmdrate");
        RecordInitialVal("cl_updaterate");
    }

    return result;
}

// CDemoRecorder::StartRecording
DETOUR(EngineDemoRecorder::StartRecording, const char* filename, bool continuously)
{
    fovChanger->needToUpdate = true;

    timescaleDetect->Spawn();

    auto result = EngineDemoRecorder::StartRecording(thisptr, filename, continuously);

    needToRecordInitialVals = true;

    return result;
}

// CDemoRecorder::StopRecording
DETOUR(EngineDemoRecorder::StopRecording)
{
    // This function does:
    //   m_bRecording = false
    //   m_nDemoNumber = 0
    auto result = EngineDemoRecorder::StopRecording(thisptr);

    if (engine->demorecorder->isRecordingDemo) {
        std::string demoName = engine->demorecorder->GetDemoFilename();
        if (!AddDemoChecksum(demoName.c_str())) {
            // TODO: report failure?
        }
    }

    if (engine->demorecorder->isRecordingDemo && sar_autorecord.GetBool() && !engine->demorecorder->requestedStop) {
        *engine->demorecorder->m_nDemoNumber = engine->demorecorder->lastDemoNumber;
        *engine->demorecorder->m_bRecording = true;
    } else if (sar_record_at_increment.GetBool()) {
        *engine->demorecorder->m_nDemoNumber = engine->demorecorder->lastDemoNumber;
        engine->demorecorder->isRecordingDemo = false;
    } else {
        engine->demorecorder->isRecordingDemo = false;
        engine->demorecorder->lastDemoNumber = 1;
    }

    engine->demorecorder->hasNotified = false;

    return result;
}

DETOUR(EngineDemoRecorder::RecordCustomData, int id, const void* data, unsigned long length)
{
    if (id == 0 && length == 8) {
        // Radial menu mouse pos data - store it so we can put it in
        // our custom data messages
        memcpy(engine->demorecorder->coopRadialMenuLastPos, data, 8);
    }
    return EngineDemoRecorder::RecordCustomData(thisptr, id, data, length);
}

DETOUR_COMMAND(EngineDemoRecorder::stop)
{
    engine->demorecorder->requestedStop = true;
    EngineDemoRecorder::stop_callback(args);
    engine->demorecorder->requestedStop = false;
}

bool EngineDemoRecorder::Init()
{
    auto disconnect = engine->cl->Original(Offsets::Disconnect);
    auto demorecorder = Memory::DerefDeref<void*>(disconnect + Offsets::demorecorder);
    if (this->s_ClientDemoRecorder = Interface::Create(demorecorder)) {
        this->s_ClientDemoRecorder->Hook(EngineDemoRecorder::SetSignonState_Hook, EngineDemoRecorder::SetSignonState, Offsets::SetSignonState);
        this->s_ClientDemoRecorder->Hook(EngineDemoRecorder::StartRecording_Hook, EngineDemoRecorder::StartRecording, Offsets::StartRecording);
        this->s_ClientDemoRecorder->Hook(EngineDemoRecorder::StopRecording_Hook, EngineDemoRecorder::StopRecording, Offsets::StopRecording);
        this->s_ClientDemoRecorder->Hook(EngineDemoRecorder::RecordCustomData_Hook, EngineDemoRecorder::RecordCustomData, Offsets::RecordCustomData);

        this->GetRecordingTick = s_ClientDemoRecorder->Original<_GetRecordingTick>(Offsets::GetRecordingTick);
        this->m_szDemoBaseName = reinterpret_cast<char*>((uintptr_t)demorecorder + Offsets::m_szDemoBaseName);
        this->m_nDemoNumber = reinterpret_cast<int*>((uintptr_t)demorecorder + Offsets::m_nDemoNumber);
        this->m_bRecording = reinterpret_cast<bool*>((uintptr_t)demorecorder + Offsets::m_bRecording);

        engine->net_time = Memory::Deref<double*>((uintptr_t)this->GetRecordingTick + Offsets::net_time);
    }

    Command::Hook("stop", EngineDemoRecorder::stop_callback_hook, EngineDemoRecorder::stop_callback);

    return this->hasLoaded = this->s_ClientDemoRecorder;
}
void EngineDemoRecorder::Shutdown()
{
    Interface::Delete(this->s_ClientDemoRecorder);
    Command::Unhook("stop", EngineDemoRecorder::stop_callback);
}
void EngineDemoRecorder::RecordData(const void* data, unsigned long length)
{
    // We record custom data as type 0. This custom data type is present
    // in the base game (the only one in fact), so we won't cause
    // crashes by using it. It corresponds to RadialMenuMouseCallback -
    // a callback for setting the cursor position in the co-op radial
    // menu. We hook RecordCustomData so that when this custom data type
    // is actually used, we remember the last cursor position - this
    // allows us to use the last-recorded position at the start of our
    // custom data, and stops us from doing weird things in co-op demos
    // with menus.

    if (!this->customDataReady) return;

    // once again, what the fuck c++, i just want a vla
    char *buf = (char*)malloc(length+8);
    memcpy(buf, engine->demorecorder->coopRadialMenuLastPos, 8); // Actual cursor x and y pos
    memcpy(buf+8, data, length);
    EngineDemoRecorder::RecordCustomData(this->s_ClientDemoRecorder->ThisPtr(), 0, buf, length+8);
    free(buf);
}
