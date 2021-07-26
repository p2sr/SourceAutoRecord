#include "ClassDumper.hpp"

#include "Modules/Client.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "SAR.hpp"
#include "Utils/SDK.hpp"

#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <string>

ClassDumper *classDumper;

ClassDumper::ClassDumper()
	: serverClassesFile("server_classes.json")
	, clientClassesFile("client_classes.json") {
	this->hasLoaded = true;
}
void ClassDumper::Dump(bool dumpServer) {
	auto source = (dumpServer) ? &this->serverClassesFile : &this->clientClassesFile;

	std::ofstream file(*source, std::ios::out | std::ios::trunc);
	if (!file.good()) {
		console->Warning("Failed to create file!\n");
		return file.close();
	}

	file << "{\"data\":[";

	if (dumpServer) {
		for (auto sclass = server->GetAllServerClasses(); sclass; sclass = sclass->m_pNext) {
			file << "{\"class\":\"" << sclass->m_pNetworkName << "\",\"table\":";
			this->DumpSendTable(file, sclass->m_pTable);
			file << "},";
		}
	} else {
		for (auto cclass = client->GetAllClasses(); cclass; cclass = cclass->m_pNext) {
			file << "{\"class\":\"" << cclass->m_pNetworkName << "\",\"table\":";
			this->DumpRecvTable(file, cclass->m_pRecvTable);
			file << "},";
		}
	}

	file.seekp(-1, SEEK_DIR_CUR);
	file << "]}";
	file.close();

	console->Print("Created %s file.\n", source->c_str());
}
void ClassDumper::DumpSendTable(std::ofstream &file, SendTable *table) {
	file << "{\"name\":\"" << table->m_pNetTableName << "\",\"props\":[";

	for (auto i = 0; i < table->m_nProps; ++i) {
		auto prop = *reinterpret_cast<SendProp *>((uintptr_t)table->m_pProps + sizeof(SendProp) * i);

		auto name = prop.m_pVarName;
		auto offset = prop.m_Offset;
		auto type = prop.m_Type;
		auto nextTable = prop.m_pDataTable;

		auto sanitized = std::string("");
		auto c = name;
		while (*c != '\0') {
			if (*c == '"') {
				sanitized += "\\";
			}
			sanitized += *c;
			++c;
		}

		file << "{\"name\":\"" << sanitized.c_str() << "\",\"offset\":" << (int16_t)offset;

		if (type != SendPropType::DPT_DataTable) {
			file << ",\"type\":" << type << "},";
			continue;
		}

		file << ",\"table\":";

		this->DumpSendTable(file, nextTable);

		file << "},";
	}

	if (table->m_nProps != 0) {
		file.seekp(-1, SEEK_DIR_CUR);
	}
	file << "]}";
}
void ClassDumper::DumpRecvTable(std::ofstream &file, RecvTable *table) {
	file << "{\"name\":\"" << table->m_pNetTableName << "\",\"props\":[";

	for (auto i = 0; i < table->m_nProps; ++i) {
		auto prop = table->m_pProps[i];

		auto name = prop.m_pVarName;
		auto offset = prop.m_Offset;
		auto type = prop.m_RecvType;
		auto nextTable = prop.m_pDataTable;

		auto sanitized = std::string("");
		auto c = name;
		while (*c != '\0') {
			if (*c == '"') {
				sanitized += "\\";
			}
			sanitized += *c;
			++c;
		}

		file << "{\"name\":\"" << sanitized.c_str() << "\",\"offset\":" << (int16_t)offset;

		if (type != SendPropType::DPT_DataTable) {
			file << ",\"type\":" << type << "},";
			continue;
		}

		file << ",\"table\":";

		this->DumpRecvTable(file, nextTable);

		file << "},";
	}

	if (table->m_nProps != 0) {
		file.seekp(-1, SEEK_DIR_CUR);
	}
	file << "]}";
}

// Commands

CON_COMMAND(sar_dump_server_classes, "sar_dump_server_classes - dumps all server classes to a file\n") {
	classDumper->Dump();
}
CON_COMMAND(sar_dump_client_classes, "sar_dump_client_classes - dumps all client classes to a file\n") {
	classDumper->Dump(false);
}
CON_COMMAND(sar_list_server_classes, "sar_list_server_classes - lists all server classes\n") {
	for (auto sclass = server->GetAllServerClasses(); sclass; sclass = sclass->m_pNext) {
		console->Print("%s\n", sclass->m_pNetworkName);
	}
}
CON_COMMAND(sar_list_client_classes, "sar_list_client_classes - lists all client classes\n") {
	for (auto cclass = client->GetAllClasses(); cclass; cclass = cclass->m_pNext) {
		console->Print("%s\n", cclass->m_pNetworkName);
	}
}
CON_COMMAND(sar_find_server_class, "sar_find_server_class <class_name> - finds specific server class tables and props with their offset\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_find_server_class.ThisPtr()->m_pszHelpString);
	}

	std::function<void(SendTable * table, int &level)> DumpTable;
	DumpTable = [&DumpTable](SendTable *table, int &level) {
		console->Print("%*s%s\n", level * 4, "", table->m_pNetTableName);
		for (auto i = 0; i < table->m_nProps; ++i) {
			auto prop = *reinterpret_cast<SendProp *>((uintptr_t)table->m_pProps + sizeof(SendProp) * i);

			auto name = prop.m_pVarName;
			auto offset = prop.m_Offset;
			auto type = prop.m_Type;
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

	for (auto sclass = server->GetAllServerClasses(); sclass; sclass = sclass->m_pNext) {
		if (!std::strcmp(args[1], sclass->m_pNetworkName)) {
			console->Print("%s\n", sclass->m_pNetworkName);
			auto level = 1;
			DumpTable(sclass->m_pTable, level);
			break;
		}
	}
}
CON_COMMAND(sar_find_client_class, "sar_find_clientclass <class_name> - finds specific client class tables and props with their offset\n") {
	if (args.ArgC() != 2) {
		return console->Print(sar_find_client_class.ThisPtr()->m_pszHelpString);
	}

	std::function<void(RecvTable * table, int &level)> DumpTable;
	DumpTable = [&DumpTable](RecvTable *table, int &level) {
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
