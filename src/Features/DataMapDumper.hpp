#pragma once
#include "Feature.hpp"

#include <fstream>
#include <string>
#include <vector>

#include "Utils/SDK.hpp"

#include "Command.hpp"

#ifdef _WIN32
// \xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\xB8\x00\x00\x00\x00 xx????????xx????????x????
#define DATAMAP_P2_PATTERN "C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? B8 ? ? ? ?"
#define DATAMAP_P2_NUM_OFFSET 6
#define DATAMAP_P2_OFFSET1 12
#define DATAMAP_P2_OFFSET2 21
#define DATAMAP_HL2_PATTERN "B8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ?"
#define DATAMAP_HL2_NUM_OFFSET 11
#define DATAMAP_HL2_OFFSET1 1
#define DATAMAP_HL2_OFFSET2 17
#else
#define DATAMAP_P2_PATTERN "C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? B8 ? ? ? ?"
#define DATAMAP_HL2_PATTERN "B8 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ?"
#define DATAMAP_P2_NUM_OFFSET 6
#define DATAMAP_P2_OFFSET1 12
#define DATAMAP_P2_OFFSET2 21
#define DATAMAP_HL2_NUM_OFFSET 11
#define DATAMAP_HL2_OFFSET1 1
#define DATAMAP_HL2_OFFSET2 17
#endif

class DataMapDumper : public Feature {
private:
    std::string serverDataMapFile;
    std::string clientDataMapFile;
    std::vector<uintptr_t> serverResult;
    std::vector<uintptr_t> clientResult;

public:
    DataMapDumper();
    void DumpServer();
    void DumpClient();

private:
    void InternalDump(std::ofstream& file, bool dumpServer = true);
};

extern DataMapDumper* dataMapDumper;

extern Command sar_dump_server_datamap;
extern Command sar_dump_client_datamap;
