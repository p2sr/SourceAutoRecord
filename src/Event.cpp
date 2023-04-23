#include "Event.hpp"

#include "Signal.hpp"
#include "Modules/Server.hpp"

#include <map>

SarInitHandler::SarInitHandler(std::function<void()> cb)
	: cb(cb) {
		SarInitHandler::GetHandlers().push_back(this);
}

void SarInitHandler::RunAll() {
	for (auto h : SarInitHandler::GetHandlers()) {
		h->cb();
	}
}

std::vector<SarInitHandler *> &SarInitHandler::GetHandlers() {
	static std::vector<SarInitHandler *> handlers;
	return handlers;
}

SIGNAL_LISTENER(500, Server::ProcessMovement, void *player, CMoveData *move) {
	Event::Trigger<Event::PROCESS_MOVEMENT>({server->GetSplitScreenPlayerSlot(player), true});

	return signal->CallNext(thisptr, player, move);
}
