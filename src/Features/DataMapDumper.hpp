#pragma once
#include "Command.hpp"
#include "Feature.hpp"

#include <string>
#include <vector>

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

extern DataMapDumper *dataMapDumper;

extern Command sar_dump_server_datamap;
extern Command sar_dump_client_datamap;
