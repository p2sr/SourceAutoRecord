#pragma once
#include "Module.hpp"

#include "Interface.hpp"
#include "Utils.hpp"

class EngineDemoRecorder : public Module {
public:
    Interface* s_ClientDemoRecorder;

    using _GetRecordingTick = int(__func*)(void* thisptr);
    _GetRecordingTick GetRecordingTick;

    char* m_szDemoBaseName;
    int* m_nDemoNumber;
    bool* m_bRecording;

    std::string CurrentDemo;
    bool IsRecordingDemo;

public:
    int GetTick();

private:
    // CDemoRecorder::SetSignonState
    DECL_DETOUR(SetSignonState, int state)

    // CDemoRecorder::StopRecording
    DECL_DETOUR(StopRecording)

public:
    bool Init() override;
    void Shutdown() override;
};
