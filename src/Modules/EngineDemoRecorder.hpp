#pragma once
#include "Command.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Utils.hpp"

#include <string>

class EngineDemoRecorder : public Module {
public:
	Interface *s_ClientDemoRecorder = nullptr;

	using _GetRecordingTick = int(__rescall *)(void *thisptr);
	_GetRecordingTick GetRecordingTick = nullptr;

	char *m_szDemoBaseName = nullptr;
	int *m_nDemoNumber = nullptr;
	bool *m_bRecording = nullptr;

	std::string currentDemo = std::string();
	bool isRecordingDemo = false;
	bool requestedStop = false;
	int lastDemoNumber = 1;
	bool hasNotified = false;
	bool customDataReady = false;
	int autorecordStartNum = 1;

	char coopRadialMenuLastPos[8];

public:
	int GetTick();
	std::string GetDemoFilename();

	// CDemoRecorder::SetSignonState
	DECL_DETOUR(SetSignonState, int state);

	// CDemoRecorder::StartRecording
	DECL_DETOUR(StartRecording, const char *filename, bool continuously);

	// CDemoRecorder::StopRecording
	DECL_DETOUR(StopRecording);

	// CDemoRecorder::RecordCustomData
	DECL_DETOUR(RecordCustomData, int id, const void *data, unsigned long length);

	DECL_DETOUR_COMMAND(stop);

	DECL_DETOUR_COMMAND(record);

	bool Init() override;
	void Shutdown() override;
	const char *Name() override { return MODULE("engine"); }
	void RecordData(const void *data, unsigned long length);
};
