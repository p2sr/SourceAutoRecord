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
void DataMapDumper::Dump(bool dumpServer)
{
    auto source = (dumpServer) ? &this->serverDataMapFile : &this->clientDataMapFile;

    std::ofstream file(*source, std::ios::out | std::ios::trunc);
    if (!file.good()) {
        console->Warning("Failed to create file!\n");
        return file.close();
    }

    std::function<void(datamap_t * map, int& level)> DumpMap;
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

    auto results = (dumpServer) ? &this->serverResult : &this->clientResult;
    if (results->empty()) {
        auto hl2 = sar.game->Is(SourceGame_HalfLife2Engine);
        auto moduleName = (dumpServer) ? server->Name() : client->Name();

        *results = Memory::MultiScan(moduleName, &DATAMAP_PATTERNS);
        for (auto const& result : *results) {
            auto num = Memory::Deref<int>(result[0]);
            if (num > 0) {
                auto ptr = Memory::Deref<void*>(result[1]);
                auto level = 0;
                if (hl2) {
                    DumpMap(reinterpret_cast<datamap_t*>(ptr), level);
                } else {
                    DumpMap2(reinterpret_cast<datamap_t2*>(ptr), level);
                }
            }
        }
    }

    console->Print("Created %s file.\n", source->c_str());
    file.close();
}

// Commands

CON_COMMAND(sar_dump_server_datamap, "Dumps server datamap to a file.\n")
{
    dataMapDumper->Dump();
}
CON_COMMAND(sar_dump_client_datamap, "Dumps client datmap to a file.\n")
{
    dataMapDumper->Dump(false);
}
