#include "SegmentedTools.hpp"

#include "Event.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Session.hpp"

struct WaitEntry {
	bool persistAcrossLoads;
	bool ignoreThisSession;
	int tick;
	std::string cmd;
};

static std::vector<WaitEntry> g_entries;

Variable wait_persist_across_loads("wait_persist_across_loads", "0", 0, 1, "Whether pending commands should be carried across loads (1) or just be dropped (0).\n");

Variable wait_mode("wait_mode", "0", "When the pending commands should be executed. 0 is absolute, 1 is relative to when you entered the wait command.\n");

// mlugg 2021-01-11: DO NOT USE CON_COMMAND FOR THIS. That macro creates
// a global named 'wait', which on Linux, conflicts with
// 'pid_t wait(int *wstatus)' from stdlib. This results in very bad
// things happening on initialization; C++ gets them mixed up somehow
// and tries to send the function pointer as the Command constructor's
// 'this', with predictably disastrous results.
void wait_callback(const CCommand &args) {
	if (args.ArgC() < 3) {
		return console->Print(waitCmd.ThisPtr()->m_pszHelpString);
	}

	int tick = std::atoi(args[1]);
	if (wait_mode.GetBool()) tick += session->GetTick();

	const char *cmd;

	if (args.ArgC() == 3) {
		cmd = args[2];
	} else {
		cmd = args.m_pArgSBuffer + args.m_nArgv0Size;

		while (isspace(*cmd)) ++cmd;

		if (*cmd == '"') {
			cmd += strlen(args[1]) + 2;
		} else {
			cmd += strlen(args[1]);
		}

		while (isspace(*cmd)) ++cmd;
	}

	if (engine->demorecorder->isRecordingDemo) {
		size_t size = strlen(cmd) + 6;
		char *data = new char[size];
		data[0] = 0x09;
		*(int *)(data + 1) = tick;
		strcpy(data + 5, cmd);
		engine->demorecorder->RecordData(data, size);
		delete[] data;
	}

	g_entries.push_back({wait_persist_across_loads.GetBool(), session->GetTick() >= tick, tick, cmd});
}
Command waitCmd("wait", wait_callback, "wait <tick> <commands> - wait for the amount of ticks specified\n", FCVAR_DONTRECORD);

ON_EVENT(SESSION_END) {
	for (size_t i = 0; i < g_entries.size(); ++i) {
		if (!g_entries[i].persistAcrossLoads) {
			g_entries.erase(g_entries.begin() + i--);
		} else {
			g_entries[i].ignoreThisSession = false;
		}
	}
}

ON_EVENT(PRE_TICK) {
	for (size_t i = 0; i < g_entries.size(); ++i) {
		auto ent = g_entries[i];
		if (event.tick >= ent.tick && !ent.ignoreThisSession) {
			engine->ExecuteCommand(ent.cmd.c_str(), true);
			g_entries.erase(g_entries.begin() + i--);
		}
	}
}
