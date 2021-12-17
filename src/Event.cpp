#include "Event.hpp"

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
