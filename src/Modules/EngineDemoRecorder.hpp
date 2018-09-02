#pragma once
#include "Module.hpp"

#include <string>

#include "Interface.hpp"
#include "Utils.hpp"

class EngineDemoRecorder : public Module {
public:
    Interface* s_ClientDemoRecorder = nullptr;

    using _GetRecordingTick = int(__func*)(void* thisptr);
    _GetRecordingTick GetRecordingTick;

    char* m_szDemoBaseName = nullptr;
    int* m_nDemoNumber = nullptr;
    bool* m_bRecording = nullptr;

    std::string currentDemo = std::string();
    bool isRecordingDemo = false;

public:
    int GetTick();

    // CDemoRecorder::SetSignonState
    DECL_DETOUR(SetSignonState, int state)

    // CDemoRecorder::StopRecording
    DECL_DETOUR(StopRecording)

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("engine"); }
};
