#include "SegmentedTools.hpp"

#include "Event.hpp"
#include "Scheduler.hpp"
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

void wait_func(ConCommand* source, const CCommand &args, bool mode) {
	if (args.ArgC() < 3) {
		return console->Print(source->m_pszHelpString);
	}

	int tick = std::atoi(args[1]);
	if (mode) tick += session->GetTick();

	const char *cmd = Utils::ArgContinuation(args, 2);

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

// mlugg 2021-01-11: DO NOT USE CON_COMMAND FOR THIS. That macro creates
// a global named 'wait', which on Linux, conflicts with
// 'pid_t wait(int *wstatus)' from stdlib. This results in very bad
// things happening on initialization; C++ gets them mixed up somehow
// and tries to send the function pointer as the Command constructor's
// 'this', with predictably disastrous results.
void wait_callback(const CCommand &args) {
	wait_func(waitCmd.ThisPtr(), args, wait_mode.GetBool());
}
Command waitCmd("wait", wait_callback, "wait <tick> <commands> - wait for the amount of ticks specified\n", FCVAR_DONTRECORD);

CON_COMMAND_F(wait_to, "wait_to <tick> <commands> - run this command on the specified session tick\n", FCVAR_DONTRECORD) {
	wait_func(wait_to.ThisPtr(), args, false);
}
CON_COMMAND_F(wait_for, "wait_for <tick> <commands> - wait for the amount of ticks specified\n", FCVAR_DONTRECORD) {
	wait_func(wait_for.ThisPtr(), args, true);
}

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

CON_COMMAND_F(hwait, "hwait <tick> <command> [args...] - run a command after the given number of host ticks\n", FCVAR_DONTRECORD) {
	if (args.ArgC() < 3) {
		return console->Print(hwait.ThisPtr()->m_pszHelpString);
	}

	int ticks = std::atoi(args[1]);

	const char *cmd = Utils::ArgContinuation(args, 2);

	std::string cmdstr = cmd;

	if (engine->demorecorder->isRecordingDemo) {
		size_t size = cmdstr.size() + 6;
		char *data = new char[size];
		data[0] = 0x0D;
		*(int *)(data + 1) = ticks;
		strcpy(data + 5, cmd);
		engine->demorecorder->RecordData(data, size);
		delete[] data;
	}

	Scheduler::InHostTicks(ticks, [=]() {
		engine->ExecuteCommand(cmdstr.c_str(), true);
	});
}
