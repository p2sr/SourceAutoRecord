#pragma once
#include "Feature.hpp"

#include <fstream>
#include <string>

#include "Command.hpp"

#include "Utils/SDK.hpp"

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

extern Command sar_dump_serverclasses;
extern Command sar_list_serverclasses;
extern Command sar_find_serverclass;
extern Command sar_dump_clientclasses;
extern Command sar_list_clientclasses;
extern Command sar_find_clientclass;
