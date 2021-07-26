#include "Teleporter.hpp"

#include "Command.hpp"
#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

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
	auto player = server->GetPlayer(nSlot + 1);
	if (player) {
		auto location = this->GetLocation(nSlot);
		location->origin = server->GetAbsOrigin(player);
		location->angles = engine->GetAngles(nSlot);
		location->isSet = true;

		console->Print("Saved location: %.3f %.3f %.3f\n", location->origin.x, location->origin.y, location->origin.z);
	}
}
void Teleporter::Teleport(int nSlot) {
	auto location = this->GetLocation(nSlot);

	char setpos[64];
	std::snprintf(setpos, sizeof(setpos), "setpos %f %f %f", location->origin.x, location->origin.y, location->origin.z);

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
	if (!teleporter->GetLocation(slot)->isSet) {
		return console->Print("Location not set! Use sar_teleport_setpos.\n");
	}

	teleporter->Teleport(slot);
}
CON_COMMAND(sar_teleport_setpos, "sar_teleport_setpos - saves current location for teleportation\n") {
	teleporter->Save(GET_SLOT());
}
