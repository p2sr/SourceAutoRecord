#pragma once
#include "Command.hpp"
#include "Interface.hpp"
#include "Module.hpp"
#include "Utils.hpp"

#include <string>
#include <utility>
#include <vector>

// Ticks before demo autostop
#define DEMO_AUTOSTOP_DELAY 15

// Where the game registers no demo custom data callback (Portal 2 4554), SAR
// data is recorded as callback index -1 (the callback table) with this prefix.
// The engine reads the first 4 bytes as the table's entry count: 0 makes it
// clear an already-empty table and ignore the rest. See RecordData.
// Same length as the cursor-position prefix used with index 0, so the SAR
// data always starts 8 bytes into the payload.
inline constexpr char SAR_DEMO_DATA_TABLE_PREFIX[8] = {0, 0, 0, 0, 'S', 'A', 'R', 1};

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
	bool customDataReady = false;
	int autorecordStartNum = 1;

	std::vector<std::string> queuedCommands = {};
	std::vector<std::pair<std::string, uint32_t>> queuedVScriptChecksums = {};

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
	void Stop();
};
