#include "ClassDumper.hpp"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <string>

#ifdef _WIN32
#include <functional>
#endif

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Utils/SDK.hpp"

#include "SAR.hpp"

ClassDumper* classDumper;

ClassDumper::ClassDumper()
    : sendPropSize(sar.game->Is(SourceGame_Portal2Engine) ? sizeof(SendProp2) : sizeof(SendProp))
    , serverClassesFile("server_classes.txt")
    , clientClassesFile("client_classes.txt")
{
    this->hasLoaded = true;
}
void ClassDumper::Dump(bool dumpServer)
{
    auto source = (dumpServer) ? &this->serverClassesFile : &this->clientClassesFile;

    std::ofstream file(*source, std::ios::out | std::ios::trunc);
    if (!file.good()) {
        console->Warning("Failed to create file!\n");
        return file.close();
    }

    if (dumpServer) {
        for (auto sclass = server->GetAllServerClasses(); sclass; sclass = sclass->m_pNext) {
            file << sclass->m_pNetworkName << std::endl;
            auto level = 1;
            this->DumpSendTable(file, sclass->m_pTable, level);
        }
    } else {
        for (auto cclass = client->GetAllClasses(); cclass; cclass = cclass->m_pNext) {
            file << cclass->m_pNetworkName << std::endl;
            auto level = 1;
            this->DumpRecvTable(file, cclass->m_pRecvTable, level);
        }
    }

    console->Print("Created %s file.\n", source->c_str());
    file.close();
}
void ClassDumper::DumpSendTable(std::ofstream& file, SendTable* table, int& level)
{
    file << std::setw(level * 4) << "";
    file << table->m_pNetTableName << std::endl;

    for (auto i = 0; i < table->m_nProps; ++i) {
        auto prop = *reinterpret_cast<SendProp*>((uintptr_t)table->m_pProps + this->sendPropSize * i);

        auto name = prop.m_pVarName;
        auto offset = prop.m_Offset;
        auto type = prop.m_Type;
        auto nextTable = prop.m_pDataTable;

        if (sar.game->Is(SourceGame_Portal2Engine)) {
            auto temp = *reinterpret_cast<SendProp2*>(&prop);
            name = temp.m_pVarName;
            offset = temp.m_Offset;
            type = temp.m_Type;
            nextTable = temp.m_pDataTable;
        }

        file << std::setw(level * 4) << "";
        file << name << " -> " << (int16_t)offset << std::endl;

        if (type != SendPropType::DPT_DataTable) {
            continue;
        }

        ++level;
        this->DumpSendTable(file, nextTable, level);
    }
    --level;
}
void ClassDumper::DumpRecvTable(std::ofstream& file, RecvTable* table, int& level)
{
    file << std::setw(level * 4) << "";
    file << table->m_pNetTableName << std::endl;

    for (auto i = 0; i < table->m_nProps; ++i) {
        auto prop = table->m_pProps[i];

        auto name = prop.m_pVarName;
        auto offset = prop.m_Offset;
        auto type = prop.m_RecvType;
        auto nextTable = prop.m_pDataTable;

        file << std::setw(level * 4) << "";
        file << name << " -> " << (int16_t)offset << std::endl;

        if (type != SendPropType::DPT_DataTable) {
            continue;
        }

        ++level;
        this->DumpRecvTable(file, nextTable, level);
    }
    --level;
}

// Commands

CON_COMMAND(sar_dump_server_classes, "Dumps all server classes to a file.\n")
{
    classDumper->Dump();
}
CON_COMMAND(sar_dump_client_classes, "Dumps all client classes to a file.\n")
{
    classDumper->Dump(false);
}
CON_COMMAND(sar_list_server_classes, "Lists all server classes.\n")
{
    for (auto sclass = server->GetAllServerClasses(); sclass; sclass = sclass->m_pNext) {
        console->Print("%s\n", sclass->m_pNetworkName);
    }
}
CON_COMMAND(sar_list_client_classes, "Lists all client classes.\n")
{
    for (auto cclass = client->GetAllClasses(); cclass; cclass = cclass->m_pNext) {
        console->Print("%s\n", cclass->m_pNetworkName);
    }
}
CON_COMMAND(sar_find_server_class, "Finds specific server class tables and props with their offset.\n"
                                   "Usage: sar_find_serverclass <class_name>\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_find_server_class.ThisPtr()->m_pszHelpString);
    }

    std::function<void(SendTable * table, int& level)> DumpTable;
    DumpTable = [&DumpTable](SendTable* table, int& level) {
        console->Print("%*s%s\n", level * 4, "", table->m_pNetTableName);
        for (auto i = 0; i < table->m_nProps; ++i) {
            auto prop = *reinterpret_cast<SendProp*>((uintptr_t)table->m_pProps + classDumper->sendPropSize * i);

            auto name = prop.m_pVarName;
            auto offset = prop.m_Offset;
            auto type = prop.m_Type;
            auto nextTable = prop.m_pDataTable;

            if (sar.game->Is(SourceGame_Portal2Engine)) {
                auto temp = *reinterpret_cast<SendProp2*>(&prop);
                name = temp.m_pVarName;
                offset = temp.m_Offset;
                type = temp.m_Type;
                nextTable = temp.m_pDataTable;
            }

            console->Msg("%*s%s -> %d\n", level * 4, "", name, (int16_t)offset);

            if (type != SendPropType::DPT_DataTable) {
                continue;
            }

            ++level;
            DumpTable(nextTable, level);
        }
        --level;
    };

    for (auto sclass = server->GetAllServerClasses(); sclass; sclass = sclass->m_pNext) {
        if (!std::strcmp(args[1], sclass->m_pNetworkName)) {
            console->Print("%s\n", sclass->m_pNetworkName);
            auto level = 1;
            DumpTable(sclass->m_pTable, level);
            break;
        }
    }
}
CON_COMMAND(sar_find_client_class, "Finds specific client class tables and props with their offset.\n"
                                   "Usage: sar_find_clientclass <class_name>\n")
{
    if (args.ArgC() != 2) {
        return console->Print(sar_find_client_class.ThisPtr()->m_pszHelpString);
    }

    std::function<void(RecvTable * table, int& level)> DumpTable;
    DumpTable = [&DumpTable](RecvTable* table, int& level) {
        console->Print("%*s%s\n", level * 4, "", table->m_pNetTableName);
        for (auto i = 0; i < table->m_nProps; ++i) {
            auto prop = table->m_pProps[i];

            auto name = prop.m_pVarName;
            auto offset = prop.m_Offset;
            auto type = prop.m_RecvType;
            auto nextTable = prop.m_pDataTable;

            console->Msg("%*s%s -> %d\n", level * 4, "", name, (int16_t)offset);

            if (type != SendPropType::DPT_DataTable) {
                continue;
            }

            ++level;
            DumpTable(nextTable, level);
        }
        --level;
    };

    for (auto cclass = client->GetAllClasses(); cclass; cclass = cclass->m_pNext) {
        if (!std::strcmp(args[1], cclass->m_pNetworkName)) {
            console->Print("%s\n", cclass->m_pNetworkName);
            auto level = 1;
            DumpTable(cclass->m_pRecvTable, level);
            break;
        }
    }
}
