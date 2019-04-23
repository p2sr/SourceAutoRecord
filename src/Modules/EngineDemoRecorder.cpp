#include "EngineDemoRecorder.hpp"

#include "Features/Rebinder.hpp"
#include "Features/Session.hpp"
#include "Features/Timer/Timer.hpp"

#include "Console.hpp"
#include "Engine.hpp"

#include "Command.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

REDECL(EngineDemoRecorder::SetSignonState);
REDECL(EngineDemoRecorder::StopRecording);
REDECL(EngineDemoRecorder::stop_callback);

int EngineDemoRecorder::GetTick()
{
    return this->GetRecordingTick(this->s_ClientDemoRecorder->ThisPtr());
}

// CDemoRecorder::SetSignonState
DETOUR(EngineDemoRecorder::SetSignonState, int state)
{
    if (state == SIGNONSTATE_FULL && *engine->demorecorder->m_bRecording) {
        engine->demorecorder->isRecordingDemo = true;
        engine->demorecorder->currentDemo = std::string(engine->demorecorder->m_szDemoBaseName);

        if (*engine->demorecorder->m_nDemoNumber > 1) {
            engine->demorecorder->currentDemo += std::string("_") + std::to_string(*engine->demorecorder->m_nDemoNumber);
        }
    }

    return EngineDemoRecorder::SetSignonState(thisptr, state);
}

// CDemoRecorder::StopRecording
DETOUR(EngineDemoRecorder::StopRecording)
{
    const auto lastDemoNumber = *engine->demorecorder->m_nDemoNumber;

    // This function does:
    //   m_bRecording = false
    //   m_nDemoNumber = 0
    auto result = EngineDemoRecorder::StopRecording(thisptr);

    if (engine->demorecorder->isRecordingDemo && sar_autorecord.GetBool() && !engine->demorecorder->requestedStop) {
        *engine->demorecorder->m_nDemoNumber = lastDemoNumber;

        // Tell recorder to keep recording
        if (*engine->m_bLoadgame) {
            *engine->demorecorder->m_bRecording = true;
            ++(*engine->demorecorder->m_nDemoNumber);
            console->DevMsg("Auto-Recording!");
        }
    } else {
        engine->demorecorder->isRecordingDemo = false;
    }

    return result;
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
        this->s_ClientDemoRecorder->Hook(EngineDemoRecorder::StopRecording_Hook, EngineDemoRecorder::StopRecording, Offsets::StopRecording);

        this->GetRecordingTick = s_ClientDemoRecorder->Original<_GetRecordingTick>(Offsets::GetRecordingTick);
        this->m_szDemoBaseName = reinterpret_cast<char*>((uintptr_t)demorecorder + Offsets::m_szDemoBaseName);
        this->m_nDemoNumber = reinterpret_cast<int*>((uintptr_t)demorecorder + Offsets::m_nDemoNumber);
        this->m_bRecording = reinterpret_cast<bool*>((uintptr_t)demorecorder + Offsets::m_bRecording);
    }

    Command::Hook("stop", EngineDemoRecorder::stop_callback_hook, EngineDemoRecorder::stop_callback);

    return this->hasLoaded = this->s_ClientDemoRecorder;
}
void EngineDemoRecorder::Shutdown()
{
    Interface::Delete(this->s_ClientDemoRecorder);
    Command::Unhook("stop", EngineDemoRecorder::stop_callback);
}
