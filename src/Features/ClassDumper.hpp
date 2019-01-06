#pragma once
#include "Feature.hpp"

#include <fstream>
#include <string>

#include "Utils/SDK.hpp"

#include "Command.hpp"

class ClassDumper : public Feature {
public:
    int sendPropSize;

private:
    std::string serverClassesFile;
    std::string clientClassesFile;

public:
    ClassDumper();
    void DumpServerClasses();
    void DumpClientClasses();

private:
    void DumpSendTable(std::ofstream& file, SendTable* table, int& level);
    void DumpRecvTable(std::ofstream& file, RecvTable* table, int& level);
};

extern ClassDumper* classDumper;

extern Command sar_dump_server_classes;
extern Command sar_dump_client_classes;
extern Command sar_list_server_classes;
extern Command sar_list_client_classes;
extern Command sar_find_server_class;
extern Command sar_find_client_class;
