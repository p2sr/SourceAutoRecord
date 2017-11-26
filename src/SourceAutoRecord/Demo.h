#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Modules/Console.h"

#define DEMO_HEADER_ID "HL2DEMO"
#define DEMO_PROTOCOL 4
#define	MAX_OSPATH 260	
#define MAX_SPLITSCREEN_CLIENTS 2

std::string exeFilePath;

enum DemoMessageType
{
	SignOn = 1,
	Packet,
	SyncTick,
	ConsoleCmd,
	UserCmd,
	DataTables,
	Stop,
	CustomData,
	StringTables
};

class Demo {
public:
	char demoFileStamp[8];
	__int32 demoProtocol;
	__int32 networkProtocol;
	char serverName[MAX_OSPATH];
	char clientName[MAX_OSPATH];
	char mapName[MAX_OSPATH];
	char gameDirectory[MAX_OSPATH];
	float playbackTime;
	__int32 playbackTicks;
	__int32 playbackFrames;
	__int32 signOnLength;
private:
	std::vector<__int32> messageTicks;

public:
	__int32 Demo::GetLastTick() {
		return messageTicks.back();
	}
	bool Demo::Parse(std::string demoName, bool headerOnly = true) {
		try {
			std::ifstream file(exeFilePath + "\\portal2\\" + demoName + ".dem", std::ios::in | std::ios::binary);
			if (!file.good()) return false;

			file.read(demoFileStamp, sizeof(demoFileStamp));
			if (strcmp(demoFileStamp, DEMO_HEADER_ID)) return false;
			file.read((char*)&demoProtocol, sizeof(demoProtocol));
			if (demoProtocol != DEMO_PROTOCOL) return false;
			file.read((char*)&networkProtocol, sizeof(networkProtocol));
			file.read(serverName, sizeof(serverName));
			file.read(clientName, sizeof(clientName));
			file.read(mapName, sizeof(mapName));
			file.read(gameDirectory, sizeof(gameDirectory));
			file.read((char*)&playbackTime, sizeof(playbackTime));
			file.read((char*)&playbackTicks, sizeof(playbackTicks));
			file.read((char*)&playbackFrames, sizeof(playbackFrames));
			file.read((char*)&signOnLength, sizeof(signOnLength));

			if (!headerOnly) {
				while (!file.eof() && !file.bad()) {
					byte cmd;
					__int32 tick;
					byte tag;

					file.read((char*)&cmd, sizeof(cmd));
					auto type = (DemoMessageType)cmd;
					if (type == DemoMessageType::Stop)
						break;

					file.read((char*)&tick, sizeof(tick));
					if (tick >= 0) messageTicks.push_back(tick);
					file.read((char*)&tag, sizeof(tag));

					switch (type) {
					case DemoMessageType::SignOn:
					case DemoMessageType::Packet:
						{
							file.ignore((MAX_SPLITSCREEN_CLIENTS * 76) + 4 + 4);
							__int32 length;
							file.read((char*)&length, sizeof(length));
							file.ignore(length);
							break;
						}
					case DemoMessageType::SyncTick:
						continue;
					case DemoMessageType::ConsoleCmd:
						{
							__int32 length;
							file.read((char*)&length, sizeof(length));
							file.ignore(length);
							break;
						}
					case DemoMessageType::UserCmd:
						{
							__int32 cmd;
							__int32 length;
							file.read((char*)&cmd, sizeof(cmd));
							file.read((char*)&length, sizeof(length));
							file.ignore(length);
							break;
						}
					case DemoMessageType::DataTables:
						{
							__int32 length;
							file.read((char*)&length, sizeof(length));
							file.ignore(length);
							break;
						}
					case DemoMessageType::CustomData:
						{
							__int32 idk;
							__int32 length;
							file.read((char*)&idk, sizeof(idk));
							file.read((char*)&length, sizeof(length));
							file.ignore(length);
							break;
						}
					case DemoMessageType::StringTables:
						{
							__int32 length;
							file.read((char*)&length, sizeof(length));
							file.ignore(length);
							break;
						}
					default:
						return false;
					}
				}
			}
			file.close();
		}
		catch (const std::exception& ex) {
			Console::Warning("SAR: Error occurred when trying to parse the demo file.\nReport this to NeKz: https://github.com/NeKzor/Portal2AutoRecord/issues\n%s", std::string(ex.what()));
			return false;
		}
		return true;
	}
};

extern Demo dem;