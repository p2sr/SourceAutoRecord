#include "DataMapDumper.hpp"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <string>

#ifdef _WIN32
#include <functional>
#endif

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Server.hpp"

#include "Utils/SDK.hpp"

#include "SAR.hpp"

DataMapDumper* dataMapDumper;

DataMapDumper::DataMapDumper()
    : serverDataMapFile("server_datamap.txt")
    , clientDataMapFile("client_datamap.txt")
    , serverResult()
    , clientResult()
{
    this->hasLoaded = true;
}
void DataMapDumper::DumpServer()
{
    std::ofstream file(this->serverDataMapFile, std::ios::out | std::ios::trunc);

    this->InternalDump(file);

    console->Print("Created %s file.\n", this->serverDataMapFile.c_str());
    file.close();
}
void DataMapDumper::DumpClient()
{
    std::ofstream file(this->clientDataMapFile, std::ios::out | std::ios::trunc);
    if (!file.good()) {
        console->Warning("Failed to create file!\n");
        return file.close();
    }

    this->InternalDump(file, false);

    console->Print("Created %s file.\n", this->clientDataMapFile.c_str());
    file.close();
}
static void DumpThat(std::ofstream& file, datamap_t* map, int& level)
{
    file << std::setw(level * 4) << "";
    file << map->dataClassName << std::endl;
    while (map) {
        for (auto i = 0; i < map->dataNumFields; ++i) {
            //auto field = &map->dataDesc[i];
            auto field = reinterpret_cast<typedescription_t*>(*(uintptr_t*)map + i * 52);
            file << std::setw((level + 1) * 4) << "";
            file << ((field->fieldName) ? field->fieldName : "unk") << " -> " << field->fieldOffset[0] << std::endl;
            if (field->fieldType == FIELD_EMBEDDED) {
                ++level;
                DumpThat(file, field->td, level);
            }
        }
        map = map->baseMap;
    }
    --level;
}
void DataMapDumper::InternalDump(std::ofstream& file, bool dumpServer)
{
    if (!file.good()) {
        console->Warning("Failed to create file!\n");
        return file.close();
    }

    std::function<void(datamap_t* map, int& level)> DumpMap;
    DumpMap = [&DumpMap, &file](datamap_t* map, int& level) {
        file << std::setw(level * 4) << "";
        file << map->dataClassName << std::endl;
        while (map) {
            for (auto i = 0; i < map->dataNumFields; ++i) {
                auto field = &map->dataDesc[i];
                file << std::setw((level + 1) * 4) << "";
                file << ((field->fieldName) ? field->fieldName : "unk") << " -> " << field->fieldOffset[0] << std::endl;
                if (field->fieldType == FIELD_EMBEDDED) {
                    ++level;
                    DumpMap(field->td, level);
                }
            }
            map = map->baseMap;
        }
        --level;
    };
    std::function<void(datamap_t2* map, int& level)> DumpMap2;
    DumpMap2 = [&DumpMap2, &file](datamap_t2* map, int& level) {
        file << std::setw(level * 4) << "";
        file << map->dataClassName << std::endl;
        while (map) {
            for (auto i = 0; i < map->dataNumFields; ++i) {
                auto field = &map->dataDesc[i];
                file << std::setw((level + 1) * 4) << "";
                file << ((field->fieldName) ? field->fieldName : "unk") << " -> " << field->fieldOffset << std::endl;
                if (field->fieldType == FIELD_EMBEDDED) {
                    ++level;
                    DumpMap2(field->td, level);
                }
            }
            map = map->baseMap;
        }
        --level;
    };

    auto result = (dumpServer) ? &this->serverResult : &this->clientResult;
    if (result->empty()) {
        auto hl2 = sar.game->Is(SourceGame_HalfLife2Engine);
        *result = Memory::MultiScan((dumpServer) ? server->Name() : client->Name(), (hl2) ? DATAMAP_HL2_PATTERN : DATAMAP_P2_PATTERN);
        for (auto const& addr : *result) {
            auto num = Memory::Deref<int>(addr + (hl2 ? DATAMAP_HL2_NUM_OFFSET : DATAMAP_P2_NUM_OFFSET));
            if (num > 0) {
                auto ptr1 = Memory::Deref<void*>(addr + (hl2 ? DATAMAP_HL2_OFFSET1 : DATAMAP_P2_OFFSET1));
                auto ptr2 = Memory::Deref<void*>(addr + (hl2 ? DATAMAP_HL2_OFFSET2 : DATAMAP_P2_OFFSET2));
                if (ptr1 == ptr2) {
                    auto level = 0;
                    if (hl2) {
                        DumpMap(reinterpret_cast<datamap_t*>(ptr1), level);
                    } else {
                        DumpMap2(reinterpret_cast<datamap_t2*>(ptr1), level);
                    }
                }
            }
        }
    }
}

// Commands

CON_COMMAND(sar_dump_server_datamap, "Dumps server datamap to a file.\n")
{
    dataMapDumper->DumpServer();
}
CON_COMMAND(sar_dump_client_datamap, "Dumps client datmap to a file.\n")
{
    dataMapDumper->DumpClient();
}
