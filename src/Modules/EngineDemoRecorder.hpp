#pragma once
#include <string>

#include "Module.hpp"

#include "Command.hpp"
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
    bool requestedStop = false;

public:
    int GetTick();

    // CDemoRecorder::SetSignonState
    DECL_DETOUR(SetSignonState, int state)

    // CDemoRecorder::StopRecording
    DECL_DETOUR(StopRecording)

    DECL_DETOUR_COMMAND(stop)

    bool Init() override;
    void Shutdown() override;
    const char* Name() override { return MODULE("engine"); }
};
