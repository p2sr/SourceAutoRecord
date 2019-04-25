#include "OffsetFinder.hpp"

#include <cstring>

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Server.hpp"

#include "Command.hpp"
#include "SAR.hpp"

OffsetFinder* offsetFinder;

OffsetFinder::OffsetFinder()
{
    this->hasLoaded = true;
}
void OffsetFinder::ServerSide(const char* className, const char* propName, int* offset)
{
    if (server->GetAllServerClasses) {
        for (auto curClass = server->GetAllServerClasses(); curClass; curClass = curClass->m_pNext) {
            if (!std::strcmp(curClass->m_pNetworkName, className)) {
                auto result = this->Find(curClass->m_pTable, propName);
                if (result != 0) {
                    console->DevMsg("Found %s::%s at %i (server-side)\n", className, propName, result);
                    if (offset)
                        *offset = result;
                }
                break;
            }
        }
    }

    if (offset && *offset == 0) {
        console->DevWarning("Failed to find offset for: %s::%s (server-side)\n", className, propName);
    }
}
void OffsetFinder::ClientSide(const char* className, const char* propName, int* offset)
{
    if (client->GetAllClasses) {
        for (auto curClass = client->GetAllClasses(); curClass; curClass = curClass->m_pNext) {
            if (!std::strcmp(curClass->m_pNetworkName, className)) {
                auto result = this->Find(curClass->m_pRecvTable, propName);
                if (result != 0) {
                    console->DevMsg("Found %s::%s at %i (client-side)\n", className, propName, result);
                    if (offset)
                        *offset = result;
                }
                break;
            }
        }
    }

    if (offset && *offset == 0) {
        console->DevWarning("Failed to find offset for: %s::%s (client-side)\n", className, propName);
    }
}
int16_t OffsetFinder::Find(SendTable* table, const char* propName)
{
    auto size = sar.game->Is(SourceGame_Portal2Engine) ? sizeof(SendProp2) : sizeof(SendProp);

    for (auto i = 0; i < table->m_nProps; ++i) {
        auto prop = *reinterpret_cast<SendProp*>((uintptr_t)table->m_pProps + size * i);

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

        if (!std::strcmp(name, propName)) {
            return offset;
        }

        if (type != SendPropType::DPT_DataTable) {
            continue;
        }

        if (auto nextOffset = this->Find(nextTable, propName)) {
            return offset + nextOffset;
        }
    }

    return 0;
}
int16_t OffsetFinder::Find(RecvTable* table, const char* propName)
{
    for (auto i = 0; i < table->m_nProps; ++i) {
        auto prop = table->m_pProps[i];

        auto name = prop.m_pVarName;
        auto offset = prop.m_Offset;
        auto type = prop.m_RecvType;
        auto nextTable = prop.m_pDataTable;

        if (!std::strcmp(name, propName)) {
            return offset;
        }

        if (type != SendPropType::DPT_DataTable) {
            continue;
        }

        if (auto nextOffset = this->Find(nextTable, propName)) {
            return offset + nextOffset;
        }
    }

    return 0;
}

// Commands

CON_COMMAND(sar_find_server_offset, "Finds prop offset in specified server class.\n"
                                    "Usage: sar_find_server_offset <class_name> <prop_name>\n")
{
    if (args.ArgC() != 3) {
        return console->Print(sar_find_server_offset.ThisPtr()->m_pszHelpString);
    }

    auto offset = 0;
    offsetFinder->ServerSide(args[1], args[2], &offset);
    console->Print("%s::%s = %d\n", args[1], args[2], offset);
}
CON_COMMAND(sar_find_client_offset, "Finds prop offset in specified client class.\n"
                                    "Usage: sar_find_client_offset <class_name> <prop_name>\n")
{
    if (args.ArgC() != 3) {
        return console->Print(sar_find_client_offset.ThisPtr()->m_pszHelpString);
    }

    auto offset = 0;
    offsetFinder->ClientSide(args[1], args[2], &offset);
    console->Print("%s::%s = %d\n", args[1], args[2], offset);
}
