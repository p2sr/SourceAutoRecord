#include "DataMapDumper.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Server.hpp"
#include "SAR.hpp"
#include "Utils/Memory.hpp"
#include "Utils/SDK.hpp"

#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <string>

#ifdef _WIN32
PATTERN(DATAMAP_PATTERN1, "C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? B8 ? ? ? ? ", 6, 12);
PATTERN(DATAMAP_PATTERN2, "C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C3", 6, 12);
PATTERNS(DATAMAP_PATTERNS, &DATAMAP_PATTERN1, &DATAMAP_PATTERN2);
#else
PATTERN(DATAMAP_PATTERN1, "B8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? ", 11, 1);
PATTERN(DATAMAP_PATTERN2, "C7 05 ? ? ? ? ? ? ? ? B8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? ", 6, 11);
PATTERN(DATAMAP_PATTERN3, "B8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 89 E5 5D C7 05 ? ? ? ? ? ? ? ? ", 11, 1);
PATTERNS(DATAMAP_PATTERNS, &DATAMAP_PATTERN1, &DATAMAP_PATTERN2, &DATAMAP_PATTERN3);
#endif

DataMapDumper *dataMapDumper;

DataMapDumper::DataMapDumper()
	: serverDataMapFile("server_datamap.json")
	, clientDataMapFile("client_datamap.json")
	, serverResult()
	, clientResult() {
	this->hasLoaded = true;
}
void DataMapDumper::Dump(bool dumpServer) {
	auto source = (dumpServer) ? &this->serverDataMapFile : &this->clientDataMapFile;

	std::ofstream file(*source, std::ios::out | std::ios::trunc);
	if (!file.good()) {
		console->Warning("Failed to create file!\n");
		return file.close();
	}

	file << "{\"data\":[";

	std::function<void(datamap_t * map)> DumpMap;
	DumpMap = [&DumpMap, &file](datamap_t *map) {
		file << "{\"type\":\"" << map->dataClassName << "\",\"fields\":[";
		while (map) {
			for (auto i = 0; i < map->dataNumFields; ++i) {
				auto field = &map->dataDesc[i];

				file << "{";

				if (field->fieldName) {
					file << "\"name\":\"" << field->fieldName << "\",";
				}

				file << "\"offset\":" << field->fieldOffset;

				if (field->externalName) {
					file << ",\"external\":\"" << field->externalName << "\"";
				}

				if (field->fieldType == FIELD_EMBEDDED) {
					file << ",\"type\":";
					DumpMap(field->td);
				} else {
					file << ",\"type\":" << field->fieldType;
				}

				file << "},";
			}
			map = map->baseMap;
		}
		file.seekp(-1, SEEK_DIR_CUR);
		file << "]}";
	};

	auto results = (dumpServer) ? &this->serverResult : &this->clientResult;
	if (results->empty()) {
		auto moduleName = (dumpServer) ? server->Name() : client->Name();

		*results = Memory::MultiScan(moduleName, &DATAMAP_PATTERNS);
		for (auto const &result : *results) {
			auto num = Memory::Deref<int>(result[0]);
			if (num > 0 && num < 1000) {
				auto ptr = Memory::Deref<void *>(result[1]);
				DumpMap(reinterpret_cast<datamap_t *>(ptr));
				file << ",";
			}
		}
	}

	file.seekp(-1, SEEK_DIR_CUR);
	file << "]}";
	file.close();

	console->Print("Created %s file.\n", source->c_str());
}

// Commands

CON_COMMAND(sar_dump_server_datamap, "sar_dump_server_datamap - dumps server datamap to a file\n") {
	dataMapDumper->Dump();
}
CON_COMMAND(sar_dump_client_datamap, "sar_dump_client_datamap - dumps client datmap to a file\n") {
	dataMapDumper->Dump(false);
}
