#include "EngineDemoRecorder.hpp"

#include "Console.hpp"
#include "Engine.hpp"

#include "Features/Rebinder.hpp"
#include "Features/Session.hpp"
#include "Features/Timer/Timer.hpp"

#include "Cheats.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

REDECL(EngineDemoRecorder::SetSignonState)
REDECL(EngineDemoRecorder::StopRecording)

int EngineDemoRecorder::GetTick()
{
    return this->GetRecordingTick(this->s_ClientDemoRecorder->ThisPtr());
}

// CDemoRecorder::SetSignonState
DETOUR(EngineDemoRecorder::SetSignonState, int state)
{
    if (state == SignonState::Full && *engine->demorecorder->m_bRecording) {
        engine->demorecorder->IsRecordingDemo = true;
        engine->demorecorder->CurrentDemo = std::string(engine->demorecorder->m_szDemoBaseName);
        if (*engine->demorecorder->m_nDemoNumber > 1) {
            engine->demorecorder->CurrentDemo += std::string("_") + std::to_string(*engine->demorecorder->m_nDemoNumber);
        }
    }
    return EngineDemoRecorder::SetSignonState(thisptr, state);
}

// CDemoRecorder::StopRecording
DETOUR(EngineDemoRecorder::StopRecording)
{
    const int LastDemoNumber = *engine->demorecorder->m_nDemoNumber;

    // This function does:
    //   m_bRecording = false
    //   m_nDemoNumber = 0
    auto result = EngineDemoRecorder::StopRecording(thisptr);

    if (engine->demorecorder->IsRecordingDemo && sar_autorecord.GetBool()) {
        *engine->demorecorder->m_nDemoNumber = LastDemoNumber;

        // Tell recorder to keep recording
        if (*engine->m_bLoadgame) {
            *engine->demorecorder->m_bRecording = true;
            ++(*engine->demorecorder->m_nDemoNumber);
            console->DevMsg("SAR: Recording!");
        }
    } else {
        engine->demorecorder->IsRecordingDemo = false;
    }

    return result;
}

bool EngineDemoRecorder::Init()
{
    auto disconnect = engine->cl->Original(Offsets::Disconnect);
    auto demorecorder = Memory::DerefDeref<void*>(disconnect + Offsets::demorecorder);
    if (this->s_ClientDemoRecorder = Interface::Create(demorecorder)) {
        this->s_ClientDemoRecorder->Hook(SetSignonState_Hook, this->SetSignonState, Offsets::SetSignonState);
        this->s_ClientDemoRecorder->Hook(StopRecording_Hook, this->StopRecording, Offsets::StopRecording);

        this->GetRecordingTick = s_ClientDemoRecorder->Original<_GetRecordingTick>(Offsets::GetRecordingTick);
        this->m_szDemoBaseName = reinterpret_cast<char*>((uintptr_t)demorecorder + Offsets::m_szDemoBaseName);
        this->m_nDemoNumber = reinterpret_cast<int*>((uintptr_t)demorecorder + Offsets::m_nDemoNumber);
        this->m_bRecording = reinterpret_cast<bool*>((uintptr_t)demorecorder + Offsets::m_bRecording);
    }

    return this->hasLoaded = this->s_ClientDemoRecorder;
}
void EngineDemoRecorder::Shutdown()
{
    Interface::Delete(this->s_ClientDemoRecorder);
}
