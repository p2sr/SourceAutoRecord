#include "Event.hpp"

#include <map>

std::vector<SarInitHandler *> SarInitHandler::handlers;

SarInitHandler::SarInitHandler(std::function<void()> cb)
	: cb(cb) {
	handlers.push_back(this);
}

void SarInitHandler::RunAll() {
	for (auto h : SarInitHandler::handlers) {
		h->cb();
	}
}
