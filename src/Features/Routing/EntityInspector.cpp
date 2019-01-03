#include "EntityInspector.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Command.hpp"
#include "Variable.hpp"

Variable sar_inspection_save_every_tick("sar_inspection_save_every_tick", "0",
    "Saves inspection data even when session ticks do not increment.\n");

EntityInspector* inspector;

EntityInspector::EntityInspector()
    : entityIndex(1)
    , offset()
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
    auto session = engine->GetSessionTick();

    auto entity = server->GetEntityInfoByIndex(this->entityIndex);
    if (entity->m_pEntity != nullptr) {
        this->latest = InspectionItem{
            session,
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
        if (session != this->lastSession || sar_inspection_save_every_tick.GetBool()) {
            this->data.push_back(this->latest);
            this->lastSession = session;
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
    console->Print("Started recording data at tick %i!\n", engine->GetSessionTick());
}
CON_COMMAND(sar_inspection_stop, "Stops recording entity data.\n")
{
    inspector->Stop();
    console->Print("Stopped recording data at tick %i!\n", engine->GetSessionTick());
}
CON_COMMAND(sar_inspection_print, "Prints recorded entity data.\n")
{
    inspector->PrintData();
}
CON_COMMAND(sar_inspection_export, "Stops recording entity data.\n")
{
    if (args.ArgC() != 2) {
        return console->Print("sar_inspection_export <file_name> : Saves recorded entitiy data to a csv file.\n");
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

    inspector->entityIndex = std::atoi(args[1]);
}
CON_COMMAND(sar_inspection_offset, "Sets member offset for inspection.\n")
{
    if (args.ArgC() != 2) {
        return console->Print("Current offset: %i\n", inspector->offset);
    }

    inspector->offset = std::atoi(args[1]);
}
CON_COMMAND(sar_list_ents, "Lists entities.\n")
{
    console->Print("[index] address | m_iClassName | m_iName\n");

    auto pages = Offsets::NUM_ENT_ENTRIES / 512;

    auto page = (args.ArgC() == 2) ? std::atoi(args[1]) : 1;
    page = std::max(page, 1);
    page = std::min(page, pages);

    auto first = (page - 1) * 512;
    auto last = page * 512;

    for (int index = first; index < Offsets::NUM_ENT_ENTRIES; ++index) {
        if (index == last) {
            break;
        }

        auto info = server->GetEntityInfoByIndex(index);
        if (info->m_pEntity == nullptr) {
            continue;
        }

        console->Print("[%i] %p", index, info->m_pEntity);
        console->Msg(" | %s | %s\n", server->GetEntityClassName(info->m_pEntity), server->GetEntityName(info->m_pEntity));
    }
    console->Msg("[page %i of %i]\n", page, pages);
}
CON_COMMAND(sar_find_ent, "Finds entity in the entity list by name.\n")
{
    if (args.ArgC() != 2) {
        return console->Print("sar_find_ent <m_iName> : Finds entity in the entity list by name.\n");
    }

    console->Msg("Results for %s\n", args[1]);
    for (int index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
        auto info = server->GetEntityInfoByIndex(index);
        if (info->m_pEntity == nullptr) {
            continue;
        }

        auto name = server->GetEntityName(info->m_pEntity);
        if (!name || std::strcmp(name, args[1]) != 0) {
            continue;
        }

        console->Print("[%i] %p", index, info->m_pEntity);
        console->Msg(" -> %s\n", server->GetEntityClassName(info->m_pEntity));
        break;
    }
}
CON_COMMAND(sar_find_ents, "Finds entities in the entity list by class name.\n")
{
    if (args.ArgC() != 2) {
        return console->Print("sar_find_ents <m_iClassName> : Finds entities in the entity list by class name.\n");
    }

    console->Msg("Results for %s\n", args[1]);
    for (int index = 0; index < Offsets::NUM_ENT_ENTRIES; ++index) {
        auto info = server->GetEntityInfoByIndex(index);
        if (info->m_pEntity == nullptr) {
            continue;
        }

        auto cname = server->GetEntityClassName(info->m_pEntity);
        if (!cname || std::strcmp(cname, args[1]) != 0) {
            continue;
        }

        console->Print("[%i] %p", index, info->m_pEntity);
        console->Msg(" -> %s\n", server->GetEntityName(info->m_pEntity));
    }
}
