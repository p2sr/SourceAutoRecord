#include "EngineDemoRecorder.hpp"

#include "Checksum.hpp"
#include "Command.hpp"
#include "Console.hpp"
#include "Engine.hpp"
#include "Event.hpp"
#include "Scheduler.hpp"
#include "Features/AutoSubmit.hpp"
#include "Features/EntityList.hpp"
#include "Features/FovChanger.hpp"
#include "Features/Session.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/TimescaleDetect.hpp"
#include "Offsets.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include "Version.hpp"

#include <cstdio>
#include <filesystem>

REDECL(EngineDemoRecorder::SetSignonState);
REDECL(EngineDemoRecorder::StartRecording);
REDECL(EngineDemoRecorder::StopRecording);
REDECL(EngineDemoRecorder::RecordCustomData);
REDECL(EngineDemoRecorder::stop_callback);
REDECL(EngineDemoRecorder::record_callback);

Variable sar_demo_overwrite_bak("sar_demo_overwrite_bak", "0", 0, "Rename demos to (name)_bak if they would be overwritten by recording\n");

int EngineDemoRecorder::GetTick() {
	return this->GetRecordingTick(this->s_ClientDemoRecorder->ThisPtr());
}

std::string EngineDemoRecorder::GetDemoFilename() {
#ifdef _WIN32
#	define PATH_SEP "\\"
#else
#	define PATH_SEP "/"
#endif
	return std::string(engine->GetGameDirectory()) + PATH_SEP + this->currentDemo + ".dem";
#undef PATH_SEP
}

bool needToRecordInitialVals = false;

static void RecordInitialVal(const char *name, const char *val) {
	size_t nameLen = strlen(name);
	size_t valLen = strlen(val);

	size_t bufLen = nameLen + valLen + 3;

	char *buf = (char *)malloc(bufLen);
	buf[0] = 0x02;
	strcpy(buf + 1, name);
	buf[nameLen + 1] = 0x00;
	strcpy(buf + nameLen + 2, val);
	buf[nameLen + valLen + 2] = 0x00;

	engine->demorecorder->RecordData(buf, bufLen);

	free(buf);
}

static void RecordTimestamp() {
	time_t t = time(NULL);
	struct tm *tm = gmtime(&t);

	uint8_t buf[8];

	buf[0] = 0x0B;
	*(uint16_t *)(buf + 1) = tm->tm_year + 1900;
	buf[3] = tm->tm_mon;
	buf[4] = tm->tm_mday;
	buf[5] = tm->tm_hour;
	buf[6] = tm->tm_min;
	buf[7] = tm->tm_sec;

	engine->demorecorder->RecordData(buf, sizeof buf);
}

ON_EVENT(SESSION_END) {
	if (*engine->demorecorder->m_bRecording && sar_autorecord.GetInt() == -1) {
		engine->demorecorder->Stop();
	}
}

// CDemoRecorder::SetSignonState
DETOUR(EngineDemoRecorder::SetSignonState, int state) {
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
		RecordTimestamp();
		AddDemoFileChecksums();
		/*
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
		*/
		for (ConCommandBase *cmd = tier1->m_pConCommandList; cmd; cmd = cmd->m_pNext) {
			if (!cmd->m_bRegistered) continue;
			if (cmd->IsCommand()) continue;
			if (Utils::StartsWith(cmd->m_pszName, "sar_")) continue;
			ConVar *var = (ConVar *)cmd;
			if (!strcmp(var->m_pszString, var->m_pszDefaultValue)) continue;
			RecordInitialVal(var->m_pszName, var->m_pszString);
		}
	}

	return result;
}

static std::string getDemoBakName(const char *base, int idx) {
	if (idx == 0) return Utils::ssprintf("%s/%s.dem", engine->GetGameDirectory(), base);
	if (idx == 1 && sar_demo_overwrite_bak.GetInt() == 1) return Utils::ssprintf("%s/%s_bak.dem", engine->GetGameDirectory(), base);
	return Utils::ssprintf("%s/%s_bak%d.dem", engine->GetGameDirectory(), base, idx);
}

static void preventOverwrite(const char *filename, int idx) {
	if (idx >= sar_demo_overwrite_bak.GetInt()) return;
	try {
		auto cur = getDemoBakName(filename, idx);
		auto bak = getDemoBakName(filename, idx+1);
		if (std::filesystem::exists(cur)) {
			preventOverwrite(filename, idx+1);
			std::filesystem::rename(cur, bak);
		}
	} catch (...) {
	}
}

// CDemoRecorder::StartRecording
DETOUR(EngineDemoRecorder::StartRecording, const char *filename, bool continuously) {
	fovChanger->needToUpdate = true;

	timescaleDetect->Spawn();

	if (sar_demo_overwrite_bak.GetBool()) {
		preventOverwrite(filename, 0);
	}

	if (!engine->demorecorder->isRecordingDemo) {
		engine->demorecorder->autorecordStartNum = *engine->demorecorder->m_nDemoNumber + 1;
	}

	auto result = EngineDemoRecorder::StartRecording(thisptr, filename, continuously);

	needToRecordInitialVals = true;

	return result;
}

// CDemoRecorder::StopRecording
DETOUR(EngineDemoRecorder::StopRecording) {
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

	if (engine->demorecorder->isRecordingDemo && sar_autorecord.GetInt() == 1 && !engine->demorecorder->requestedStop) {
		*engine->demorecorder->m_nDemoNumber = engine->demorecorder->lastDemoNumber;
		*engine->demorecorder->m_bRecording = true;
	} else if (sar_record_at_increment.GetBool()) {
		*engine->demorecorder->m_nDemoNumber = engine->demorecorder->lastDemoNumber;
		engine->demorecorder->isRecordingDemo = false;
	} else {
		engine->demorecorder->isRecordingDemo = false;
		engine->demorecorder->lastDemoNumber = 1;
	}

	if (!engine->demorecorder->isRecordingDemo) {
		// only set replay name once the autorecord chain is done
		std::string name = engine->demorecorder->m_szDemoBaseName;
		if (engine->demorecorder->autorecordStartNum > 1) {
			name += std::string("_") + std::to_string(engine->demorecorder->autorecordStartNum);
		}
		engine->demoplayer->replayName = name;
	}

	engine->demorecorder->hasNotified = false;

	return result;
}

DETOUR(EngineDemoRecorder::RecordCustomData, int id, const void *data, unsigned long length) {
	if (id == 0 && length == 8) {
		// Radial menu mouse pos data - store it so we can put it in
		// our custom data messages
		memcpy(engine->demorecorder->coopRadialMenuLastPos, data, 8);
	}
	return EngineDemoRecorder::RecordCustomData(thisptr, id, data, length);
}

DETOUR_COMMAND(EngineDemoRecorder::stop) {
	engine->demorecorder->requestedStop = true;
	EngineDemoRecorder::stop_callback(args);
	engine->demorecorder->requestedStop = false;
}

Variable sar_record_prefix("sar_record_prefix", "", "A string to prepend to recorded demo names. Can include strftime format codes.\n", 0);
Variable sar_record_mkdir("sar_record_mkdir", "1", "Automatically create directories for demos if necessary.\n");

DETOUR_COMMAND(EngineDemoRecorder::record) {
	CCommand newArgs = args;
	bool prefixed = false;
	bool suppress = false;

	if (args.ArgC() >= 2 && sar_record_prefix.GetString()[0]) {
		time_t t = time(nullptr);
		struct tm *tm = localtime(&t);
		char *buf = new char[MAX_PATH + 1];
		size_t timelen = strftime(buf, MAX_PATH + 1, sar_record_prefix.GetString(), tm);
		if (timelen) {
			strncat(buf, args[1], MAX_PATH - timelen);
			newArgs.m_ppArgv[1] = buf;
			prefixed = true;
		} else {
			console->Print("failed to add sar_record_prefix\n");
			delete[] buf;
		}
	}

	bool menu = engine->GetCurrentMapName() == "" && engine->hoststate->m_currentState == HS_RUN;

	if (newArgs.ArgC() >= 2 && sar_record_mkdir.GetBool() && !menu && !engine->demorecorder->isRecordingDemo) {
		try {
			std::string pStr = engine->GetGameDirectory();
			pStr += '/';
			pStr += newArgs[1];
			std::filesystem::path p(pStr);
			auto dir = p.parent_path();
			if (!std::filesystem::exists(dir)) {
				std::filesystem::create_directories(dir);
			}
		} catch (std::filesystem::filesystem_error &e) {
		}
	}

	if (newArgs.ArgC() >= 2 && !menu && !engine->demorecorder->isRecordingDemo) {
		try {
			std::string pStr = engine->GetGameDirectory();
			pStr += '/';
			pStr += newArgs[1];
			std::filesystem::path p(pStr);
			auto dir = p.parent_path();
			if (!std::filesystem::is_directory(dir)) {
				console->Print("directory %s does not exist\n", dir.string().c_str());
				suppress = true;
			}
		} catch (std::filesystem::filesystem_error &e) {
		}
	}

	if (!suppress) {
		EngineDemoRecorder::record_callback(newArgs);
	}

	if (prefixed) {
		delete[] newArgs.m_ppArgv[1];
	}
}

bool EngineDemoRecorder::Init() {
	auto disconnect = engine->cl->Original(Offsets::Disconnect);
	void *demorecorder;
	demorecorder = Memory::DerefDeref<void *>(disconnect + Offsets::demorecorder);
	if (this->s_ClientDemoRecorder = Interface::Create(demorecorder)) {
		this->s_ClientDemoRecorder->Hook(EngineDemoRecorder::SetSignonState_Hook, EngineDemoRecorder::SetSignonState, Offsets::SetSignonState);
		this->s_ClientDemoRecorder->Hook(EngineDemoRecorder::StartRecording_Hook, EngineDemoRecorder::StartRecording, Offsets::StartRecording);
		this->s_ClientDemoRecorder->Hook(EngineDemoRecorder::StopRecording_Hook, EngineDemoRecorder::StopRecording, Offsets::StopRecording);
		this->s_ClientDemoRecorder->Hook(EngineDemoRecorder::RecordCustomData_Hook, EngineDemoRecorder::RecordCustomData, Offsets::RecordCustomData);

		this->GetRecordingTick = s_ClientDemoRecorder->Original<_GetRecordingTick>(Offsets::GetRecordingTick);
		this->m_szDemoBaseName = reinterpret_cast<char *>((uintptr_t)demorecorder + Offsets::m_szDemoBaseName);
		this->m_nDemoNumber = reinterpret_cast<int *>((uintptr_t)demorecorder + Offsets::m_nDemoNumber);
		this->m_bRecording = reinterpret_cast<bool *>((uintptr_t)demorecorder + Offsets::m_bRecording);

		engine->net_time = Memory::Deref<double *>((uintptr_t)this->GetRecordingTick + Offsets::net_time);
	}

	Command::Hook("stop", EngineDemoRecorder::stop_callback_hook, EngineDemoRecorder::stop_callback);
	Command::Hook("record", EngineDemoRecorder::record_callback_hook, EngineDemoRecorder::record_callback);

	return this->hasLoaded = this->s_ClientDemoRecorder;
}
void EngineDemoRecorder::Shutdown() {
	Interface::Delete(this->s_ClientDemoRecorder);
	Command::Unhook("stop", EngineDemoRecorder::stop_callback);
	Command::Unhook("record", EngineDemoRecorder::record_callback);
}
void EngineDemoRecorder::RecordData(const void *data, unsigned long length) {
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
	char *buf = (char *)malloc(length + 8);
	memcpy(buf, engine->demorecorder->coopRadialMenuLastPos, 8);  // Actual cursor x and y pos
	memcpy(buf + 8, data, length);
	EngineDemoRecorder::RecordCustomData(this->s_ClientDemoRecorder->ThisPtr(), 0, buf, length + 8);
	free(buf);
}

ON_EVENT(PRE_TICK) {

	if (engine->demorecorder->m_bRecording) {
		if (event.simulating && !engine->demorecorder->hasNotified) {
			const char *cmd = "echo \"SAR " SAR_VERSION " (Built " SAR_BUILT ")\"";
			engine->SendToCommandBuffer(cmd, 300);
			engine->demorecorder->hasNotified = true;
		}

		std::deque<EntitySlotSerial>::iterator val = g_ent_slot_serial.begin();
		while (val != g_ent_slot_serial.end()) {
			if (val->done) {
				size_t size = 9;
				char *data = new char[size];
				data[0] = 0x0E;
				*(int *)(data + 1) = val->slot;
				*(int *)(data + 5) = val->serial;
				engine->demorecorder->RecordData(data, size);
				delete[] data;
				val = g_ent_slot_serial.erase(val);
			} else {
				++val;
			}
		}
	}
}

ON_EVENT(PRE_TICK) {
	if (!engine->demoplayer->IsPlaying()) {
		if (sar_record_at.GetInt() == -1) {
			engine->hasRecorded = true;  // We don't want to randomly start recording if the user sets sar_record_at in this session
		} else if (!engine->hasRecorded && session->isRunning && event.tick >= sar_record_at.GetInt()) {
			std::string cmd = std::string("record \"") + sar_record_at_demo_name.GetString() + "\"";
			engine->ExecuteCommand(cmd.c_str(), true);
			engine->hasRecorded = true;
		}
	}
}

ON_EVENT(CM_FLAGS) {
	if (engine->demorecorder->isRecordingDemo && event.end) {
		char data[2] = {0x06, (char)event.slot};
		engine->demorecorder->RecordData(data, sizeof data);

		Scheduler::InHostTicks(DEMO_AUTOSTOP_DELAY, [=]() {
			if (!engine->demorecorder->isRecordingDemo) return; // manual stop before autostop
			if (sar_challenge_autostop.GetInt() > 0) {
				std::string demoFile = engine->demorecorder->GetDemoFilename();

				engine->demorecorder->Stop();

				std::optional<std::string> rename_if_pb = {};
				std::optional<std::string> replay_append_if_pb = {};

				if (sar_challenge_autostop.GetInt() == 2 || sar_challenge_autostop.GetInt() == 3) {
					unsigned total = floor(event.time * 100);
					unsigned cs = total % 100;
					total /= 100;
					unsigned secs = total % 60;
					total /= 60;
					unsigned mins = total % 60;
					total /= 60;
					unsigned hrs = total;

					std::string time;

					if (hrs) {
						time = Utils::ssprintf("%d-%02d-%02d-%02d", hrs, mins, secs, cs);
					} else if (mins) {
						time = Utils::ssprintf("%d-%02d-%02d", mins, secs, cs);
					} else {
						time = Utils::ssprintf("%d-%02d", secs, cs);
					}

					auto newName = Utils::ssprintf("%s_%s.dem", demoFile.substr(0, demoFile.size() - 4).c_str(), time.c_str());
					if (sar_challenge_autostop.GetInt() == 2) {
						std::filesystem::rename(demoFile, newName);
						demoFile = newName;
						engine->demoplayer->replayName += "_";
						engine->demoplayer->replayName += time;
					} else { // autostop 3
						rename_if_pb = newName;
						replay_append_if_pb = std::string("_") + time;
					}
				}

				AutoSubmit::FinishRun(event.time, demoFile.c_str(), rename_if_pb, replay_append_if_pb);
			}
		});
	}
}

void EngineDemoRecorder::Stop() {
	this->requestedStop = true;
#ifdef _WIN32
	this->StopRecording_Hook(this->s_ClientDemoRecorder->ThisPtr(), 0);
#else
	this->StopRecording_Hook(this->s_ClientDemoRecorder->ThisPtr());
#endif
	this->requestedStop = false;
}

CON_COMMAND(sar_stop, "sar_stop <name> - stop recording the current demo and rename it to 'name' (not considering sar_record_prefix)\n") {
	if (args.ArgC() != 2) return console->Print(sar_stop.ThisPtr()->m_pszHelpString);

	std::string name = args[1];
	if (name.size() == 0) return console->Print("Demo name cannot be blank\n");
	name += ".dem";

	if (!engine->demorecorder->isRecordingDemo) return console->Print("Demo recording not active\n");

	std::string cur_name = engine->demorecorder->GetDemoFilename();
	engine->demorecorder->Stop();
	try {
		std::filesystem::rename(cur_name, name);
	} catch (...) {
		console->Print("An error occurred renaming the demo. Saved as %s\n", cur_name.c_str());
	}
}
