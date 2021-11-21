#include "Scheduler.hpp"
#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Features/Session.hpp"
#include <vector>

static std::vector<std::pair<int, std::function<void()>>> g_serverScheds;
static std::vector<std::pair<int, std::function<void()>>> g_hostScheds;

void Scheduler::InServerTicks(int ticks, std::function<void()> fn) {
	if (!session->isRunning) return;
	g_serverScheds.push_back({ session->GetTick() + ticks, fn });
}

void Scheduler::InHostTicks(int ticks, std::function<void()> fn) {
	int host, server, client;
	engine->GetTicks(host, server, client);
	g_hostScheds.push_back({ host + ticks, fn });
}

ON_EVENT(SESSION_START) {
	g_serverScheds.clear();
}

ON_EVENT(FRAME) {
	int host, server, client;
	engine->GetTicks(host, server, client);

	for (size_t i = 0; i < g_hostScheds.size(); ++i) {
		if (host >= g_hostScheds[i].first) {
			g_hostScheds[i].second();
			g_hostScheds.erase(g_hostScheds.begin() + i);
			--i;
		}
	}
}

ON_EVENT(PRE_TICK) {
	if (!session->isRunning) return;
	for (size_t i = 0; i < g_serverScheds.size(); ++i) {
		if (session->GetTick() >= g_serverScheds[i].first) {
			g_serverScheds[i].second();
			g_serverScheds.erase(g_serverScheds.begin() + i);
			--i;
		}
	}
}
