#include "EntityInspector.hpp"

#include "Command.hpp"
#include "Features/EntityList.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Session.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Variable.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

Variable sar_inspection_save_every_tick("sar_inspection_save_every_tick", "0", "Saves inspection data even when session tick does not increment.\n");

EntityInspector *inspector;

EntityInspector::EntityInspector()
	: entityIndex(1)
	, isRunning(false)
	, lastSession(0)
	, latest()
	, data() {
	this->hasLoaded = true;
}
void EntityInspector::Start() {
	this->data.clear();
	this->isRunning = true;
}
void EntityInspector::Record() {
	auto sessionTick = session->GetTick();

	auto entity = entityList->GetEntityInfoByIndex(this->entityIndex);
	if (entity && entity->m_pEntity) {
		this->latest = InspectionItem{
			sessionTick,
			server->GetAbsOrigin(entity->m_pEntity),
			server->GetAbsAngles(entity->m_pEntity),
			server->GetLocalVelocity(entity->m_pEntity),
			server->GetFlags(entity->m_pEntity),
			server->GetEFlags(entity->m_pEntity),
			server->GetMaxSpeed(entity->m_pEntity),
			server->GetGravity(entity->m_pEntity),
			server->GetViewOffset(entity->m_pEntity)};
	}

	if (this->isRunning) {
		if (sessionTick != this->lastSession || sar_inspection_save_every_tick.GetBool()) {
			this->data.push_back(this->latest);
			this->lastSession = sessionTick;
		}
	}
}
void EntityInspector::Stop() {
	this->isRunning = false;
}
bool EntityInspector::IsRunning() {
	return this->isRunning;
}
InspectionItem EntityInspector::GetData() {
	return this->latest;
}
void EntityInspector::PrintData() {
	auto current = 1;
	auto total = data.size();
	for (const auto &item : data) {
		console->Print("[%i of %i] Tick %i\n", current++, total, item.session);
		console->Msg("    -> pos: %.6f %.6f %.6f\n", item.origin.x, item.origin.y, item.origin.z);
		console->Msg("    -> ang: %.6f %.6f %.6f\n", item.angles.x, item.angles.y, item.angles.z);
		console->Msg("    -> vel: %.6f %.6f %.6f\n", item.velocity.x, item.velocity.y, item.velocity.z);
	}
}
bool EntityInspector::ExportData(std::string filePath) {
	if (this->data.empty()) {
		return false;
	}

	std::ofstream file(filePath, std::ios::out | std::ios::trunc);
	if (!file.good()) {
		file.close();
		return false;
	}

	file << SAR_INSPECTION_EXPORT_HEADER << std::endl;

	auto current = 1;
	for (const auto &item : data) {
		file << current++
							<< "," << item.session
							<< "," << std::fixed << std::setprecision(6)
							<< item.origin.x << "," << item.origin.y << "," << item.origin.z
							<< "," << item.angles.x << "," << item.angles.y << "," << item.angles.z
							<< "," << item.velocity.x << "," << item.velocity.y << "," << item.velocity.z
							<< "," << item.flags
							<< "," << item.eFlags
							<< "," << item.maxSpeed
							<< "," << item.gravity
							<< "," << item.viewOffset.x << "," << item.viewOffset.y << "," << item.viewOffset.z
							<< std::endl;
	}

	file.close();
	return true;
}

// Commands

CON_COMMAND(sar_inspection_start, "sar_inspection_start - starts recording entity data\n") {
	inspector->Start();
	console->Print("Started recording data at tick %i!\n", session->GetTick());
}
CON_COMMAND(sar_inspection_stop, "sar_inspection_stop - stops recording entity data\n") {
	inspector->Stop();
	console->Print("Stopped recording data at tick %i!\n", session->GetTick());
}
CON_COMMAND(sar_inspection_print, "sar_inspection_print - prints recorded entity data\n") {
	inspector->PrintData();
}
CON_COMMAND(sar_inspection_export, "sar_inspection_export <file_name> - saves recorded entity data to a csv file\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_inspection_export.ThisPtr()->m_pszHelpString);
	}

	auto filePath = std::string(engine->GetGameDirectory()) + std::string("/") + std::string(args[1]);
	if (filePath.substr(filePath.length() - 4, 4) != ".csv")
		filePath += ".csv";

	if (inspector->ExportData(filePath)) {
		console->Print("Exported data!\n");
	} else {
		console->Warning("Failed to export data!\n");
	}
}
CON_COMMAND(sar_inspection_index, "sar_inspection_index - sets entity index for inspection\n") {
	if (args.ArgC() != 2) {
		return console->Print("Current index: %i\n", inspector->entityIndex);
	}

	auto index = std::atoi(args[1]);
	if (index < 0) {
		return console->Print("Index cannot be negative!\n");
	}

	if (index >= Offsets::NUM_ENT_ENTRIES) {
		return console->Print("Index cannot be higher than maximum allowed entities!\n");
	}

	inspector->entityIndex = index;
}

// HUD

HUD_ELEMENT2(inspection, "0", "Draws entity inspection data.\n", HudType_InGame | HudType_Paused) {
	ctx->DrawElement(inspector->IsRunning() ? "inspection (recording)" : "inspection");

	auto info = entityList->GetEntityInfoByIndex(inspector->entityIndex);
	if (info && info->m_pEntity) {
		ctx->DrawElement("name: %s", server->GetEntityName(info->m_pEntity));
		ctx->DrawElement("class: %s", server->GetEntityClassName(info->m_pEntity));
	} else {
		ctx->DrawElement("name: -");
		ctx->DrawElement("class: -");
	}

	auto data = inspector->GetData();
	ctx->DrawElement("pos: %.3f %.3f %.3f", data.origin.x, data.origin.y, data.origin.z);
	ctx->DrawElement("off: %.3f %.3f %.3f", data.viewOffset.x, data.viewOffset.y, data.viewOffset.z);
	ctx->DrawElement("ang: %.3f %.3f %.3f", data.angles.x, data.angles.y, data.angles.z);
	ctx->DrawElement("vel: %.3f %.3f %.3f", data.velocity.x, data.velocity.y, data.velocity.z);
	ctx->DrawElement("flags: %i", data.flags);
	ctx->DrawElement("eflags: %i", data.eFlags);
	ctx->DrawElement("maxspeed: %.3f", data.maxSpeed);
	ctx->DrawElement("gravity: %.3f", data.gravity);
}
