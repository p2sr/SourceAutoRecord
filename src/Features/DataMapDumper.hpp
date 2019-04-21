#pragma once
#include "Feature.hpp"

#include <string>
#include <vector>

#include "Utils/Memory.hpp"

#include "Command.hpp"

PATTERN(DATAMAP_PATTERN1, "B8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? ", 11, 1);
PATTERN(DATAMAP_PATTERN2, "C7 05 ? ? ? ? ? ? ? ? B8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? ", 6, 11);
PATTERN(DATAMAP_PATTERN3, "B8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 89 E5 5D C7 05 ? ? ? ? ? ? ? ? ", 11, 1);

PATTERNS(DATAMAP_PATTERNS, &DATAMAP_PATTERN1, &DATAMAP_PATTERN2, &DATAMAP_PATTERN3);

class DataMapDumper : public Feature {
private:
    std::string serverDataMapFile;
    std::string clientDataMapFile;
    std::vector<std::vector<uintptr_t>> serverResult;
    std::vector<std::vector<uintptr_t>> clientResult;

public:
    DataMapDumper();
    void Dump(bool dumpServer = true);
};

extern DataMapDumper* dataMapDumper;

extern Command sar_dump_server_datamap;
extern Command sar_dump_client_datamap;
