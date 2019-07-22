#include "EntityInspector.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "Features/EntityList.hpp"
#include "Features/Session.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Command.hpp"
#include "Variable.hpp"

Variable sar_inspection_save_every_tick("sar_inspection_save_every_tick", "0",
    "Saves inspection data even when session tick does not increment.\n");

EntityInspector* inspector;

EntityInspector::EntityInspector()
    : entityIndex(1)
    , isRunning(false)
    , lastSession(0)
    , latest()
    , data()
{
    this->hasLoaded = true;
}
void EntityInspector::Start()
{
    this->data.clear();
    this->isRunning = true;
}
void EntityInspector::Record()
{
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
            server->GetViewOffset(entity->m_pEntity)
        };
    }

    if (this->isRunning) {
        if (sessionTick != this->lastSession || sar_inspection_save_every_tick.GetBool()) {
            this->data.push_back(this->latest);
            this->lastSession = sessionTick;
        }
    }
}
void EntityInspector::Stop()
{
    this->isRunning = false;
}
bool EntityInspector::IsRunning()
{
    return this->isRunning;
}
InspectionItem EntityInspector::GetData()
{
    return this->latest;
}
void EntityInspector::PrintData()
{
    auto current = 1;
    auto total = data.size();
    for (const auto& item : data) {
        console->Print("[%i of %i] Tick %i\n", current++, total, item.session);
        console->Msg("    -> pos: %.6f %.6f %.6f\n", item.origin.x, item.origin.y, item.origin.z);
        console->Msg("    -> ang: %.6f %.6f %.6f\n", item.angles.x, item.angles.y, item.angles.z);
        console->Msg("    -> vel: %.6f %.6f %.6f\n", item.velocity.x, item.velocity.y, item.velocity.z);
    }
}
bool EntityInspector::ExportData(std::string filePath)
{
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
    for (const auto& item : data) {
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

CON_COMMAND(sar_inspection_start, "Starts recording entity data.\n")
{
    inspector->Start();
    console->Print("Started recording data at tick %i!\n", session->GetTick());
}
CON_COMMAND(sar_inspection_stop, "Stops recording entity data.\n")
{
    inspector->Stop();
    console->Print("Stopped recording data at tick %i!\n", session->GetTick());
}
CON_COMMAND(sar_inspection_print, "Prints recorded entity data.\n")
{
    inspector->PrintData();
}
CON_COMMAND(sar_inspection_export, "Saves recorded entity data to a csv file.\n"
                                   "Usage: sar_inspection_export <file_name>\n")
{
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
CON_COMMAND(sar_inspection_index, "Sets entity index for inspection.\n")
{
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
