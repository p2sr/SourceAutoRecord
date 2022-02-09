#include "Teleporter.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/NetMessage.hpp"
#include "Features/Session.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

#define TELEPORT_MESSAGE_TYPE "teleport"

ON_INIT {
	NetMessage::RegisterHandler(TELEPORT_MESSAGE_TYPE, +[](void *data, size_t size) {
		if (size != 2) return;

		bool save = ((char *)data)[0];
		int slot = ((char *)data)[1];

		if (slot >= engine->GetMaxClients()) return;
		if (engine->IsOrange() || !session->isRunning) return;

		if (save) {
			teleporter->Save(slot);
		} else {
			teleporter->Teleport(slot);
		}
	});
}

Teleporter *teleporter;

Teleporter::Teleporter()
	: locations() {
	for (auto i = 0; i < Offsets::MAX_SPLITSCREEN_PLAYERS; ++i) {
		this->locations.push_back(new TeleportLocation());
	}

	this->hasLoaded = true;
}
Teleporter::~Teleporter() {
	for (auto &location : this->locations) {
		delete location;
	}
	this->locations.clear();
}
TeleportLocation *Teleporter::GetLocation(int nSlot) {
	return this->locations[nSlot];
}
void Teleporter::Save(int nSlot) {
	if (engine->IsOrange()) {
		char buf[2];
		buf[0] = 1;
		buf[1] = (char)nSlot;
		NetMessage::SendMsg(TELEPORT_MESSAGE_TYPE, buf, sizeof buf);

		auto location = this->GetLocation(nSlot);
		location->isSet = true;
		location->angles = engine->GetAngles(0);

		return;
	}

	auto player = server->GetPlayer(nSlot + 1);
	if (player) {
		auto location = this->GetLocation(nSlot);
		location->origin = server->GetAbsOrigin(player);
		location->angles = engine->GetAngles(nSlot);
		location->velocity = server->GetLocalVelocity(player);
		location->isSet = true;

		console->Print("Saved location: %.3f %.3f %.3f\n", location->origin.x, location->origin.y, location->origin.z);
	}
}
void Teleporter::Teleport(int nSlot) {
	if (engine->IsOrange()) {
		char buf[2];
		buf[0] = 0;
		buf[1] = (char)nSlot;
		NetMessage::SendMsg(TELEPORT_MESSAGE_TYPE, buf, sizeof buf);

		auto location = this->GetLocation(nSlot);
		if (!location->isSet) return;

		engine->SetAngles(0, location->angles);

		return;
	}

	auto location = this->GetLocation(nSlot);
	if (!location->isSet) return;

	char setpos[64];
	std::snprintf(setpos, sizeof(setpos), "setpos_player %d %f %f %f", nSlot + 1, location->origin.x, location->origin.y, location->origin.z);

	uintptr_t player = (uintptr_t)server->GetPlayer(nSlot + 1);
	if (!player) return;

	*(Vector *)(player + Offsets::S_m_vecVelocity) = location->velocity;
	*(int *)(player + Offsets::m_iEFlags) |= (1<<12); // EFL_DIRTY_ABSVELOCITY

	engine->SetAngles(nSlot, location->angles);
	engine->ExecuteCommand(setpos);
}

// Commands

CON_COMMAND(sar_teleport, "sar_teleport - teleports the player to the last saved location\n") {
	IGNORE_DEMO_PLAYER();

	if (!sv_cheats.GetBool()) {
		return console->Print("Cannot teleport without sv_cheats 1!\n");
	}

	auto slot = GET_SLOT();
	if (!engine->IsOrange() && !teleporter->GetLocation(slot)->isSet) {
		return console->Print("Location not set! Use sar_teleport_setpos.\n");
	}

	teleporter->Teleport(slot);
}
CON_COMMAND(sar_teleport_setpos, "sar_teleport_setpos - saves current location for teleportation\n") {
	teleporter->Save(GET_SLOT());
}
