#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Modules/Console.hpp"

#define DEMO_HEADER_ID "HL2DEMO"
#define DEMO_PROTOCOL 4
#define	MAX_OSPATH 260	
#define MAX_SPLITSCREEN_CLIENTS 2

enum DemoMessageType {
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

class Demo
{
public:
	char demoFileStamp[8];
	int32_t demoProtocol;
	int32_t networkProtocol;
	char serverName[MAX_OSPATH];
	char clientName[MAX_OSPATH];
	char mapName[MAX_OSPATH];
	char gameDirectory[MAX_OSPATH];
	float playbackTime;
	int32_t playbackTicks;
	int32_t playbackFrames;
	int32_t signOnLength;
private:
	std::vector<int32_t> messageTicks;

public:
	int32_t GetLastTick() {
		return messageTicks.back();
	}
	float IntervalPerTick() {
		return playbackTime / playbackTicks;
	}
	void Fix() {
		float ipt = IntervalPerTick();
		playbackTicks = GetLastTick();
		playbackTime = ipt * playbackTicks;
	}
	bool Parse(std::string filePath, int outputMode = 0, bool headerOnly = false) {
		try {
			if (filePath.substr(filePath.length() - 4, 4) != ".dem") filePath += ".dem";

			Console::DevMsg("Trying to parse \"%s\"...\n", filePath.c_str());

			std::ifstream file(filePath, std::ios::in | std::ios::binary);
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
					unsigned char cmd;
					int32_t tick;
					unsigned char tag;

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
							if (outputMode == 2) {
								for (size_t i = 0; i < MAX_SPLITSCREEN_CLIENTS; i++)
								{
									if (i >= 1) {
										file.ignore(76);
										continue;
									}
									int32_t flags;
									float vo_x, vo_y, vo_z;
									float va_x, va_y, va_z;
									float lva_x, lva_y, lva_z;
									float vo2_x, vo2_y, vo2_z;
									float va2_x, va2_y, va2_z;
									float lva2_x, lva2_y, lva2_z;
									file.read((char*)&flags, sizeof(flags));
									file.read((char*)&vo_x, sizeof(vo_x));
									file.read((char*)&vo_y, sizeof(vo_y));
									file.read((char*)&vo_z, sizeof(vo_z));
									file.read((char*)&va_x, sizeof(va_x));
									file.read((char*)&va_y, sizeof(va_y));
									file.read((char*)&va_z, sizeof(va_z));
									file.read((char*)&lva_x, sizeof(lva_z));
									file.read((char*)&lva_y, sizeof(lva_y));
									file.read((char*)&lva_z, sizeof(lva_z));
									file.read((char*)&vo2_x, sizeof(vo2_x));
									file.read((char*)&vo2_y, sizeof(vo2_y));
									file.read((char*)&vo2_z, sizeof(vo2_z));
									file.read((char*)&va2_x, sizeof(va2_x));
									file.read((char*)&va2_y, sizeof(va2_y));
									file.read((char*)&va2_z, sizeof(va2_z));
									file.read((char*)&lva2_x, sizeof(lva2_z));
									file.read((char*)&lva2_y, sizeof(lva2_y));
									file.read((char*)&lva2_z, sizeof(lva2_z));
									Console::Msg("[%i] flags: %i | view origin: %.3f/%.3f/%.3f | view angles: %.3f/%.3f/%.3f | local view angles: %.3f/%.3f/%.3f\n", tick, flags, vo_x, vo_y, vo_z, va_x, va_y, va_z, lva_x, lva_y, lva_z);
								}
								int32_t in_seq, out_seq;
								file.read((char*)&in_seq, sizeof(in_seq));
								file.read((char*)&out_seq, sizeof(out_seq));
							}
							else {
								file.ignore((MAX_SPLITSCREEN_CLIENTS * 76) + 4 + 4);
							}

							int32_t length;
							file.read((char*)&length, sizeof(length));
							file.ignore(length);
							break;
						}
					case DemoMessageType::SyncTick:
						continue;
					case DemoMessageType::ConsoleCmd:
						{
							int32_t length;
							file.read((char*)&length, sizeof(length));
							if (outputMode >= 1) {
								std::string cmd(length, ' ');
								file.read(&cmd[0], length);
								Console::Msg("[%i] %s\n", tick, cmd.c_str());
							}
							else {
								file.ignore(length);
							}
							break;
						}
					case DemoMessageType::UserCmd:
						{
							int32_t cmd;
							int32_t length;
							file.read((char*)&cmd, sizeof(cmd));
							file.read((char*)&length, sizeof(length));
							file.ignore(length);
							break;
						}
					case DemoMessageType::DataTables:
						{
							int32_t length;
							file.read((char*)&length, sizeof(length));
							file.ignore(length);
							break;
						}
					case DemoMessageType::CustomData:
						{
							int32_t idk;
							int32_t length;
							file.read((char*)&idk, sizeof(idk));
							file.read((char*)&length, sizeof(length));
							file.ignore(length);
							break;
						}
					case DemoMessageType::StringTables:
						{
							int32_t length;
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
			Console::Warning("SAR: Error occurred when trying to parse the demo file.\nIf you think this is an issue, report it at: https://github.com/NeKzor/SourceAutoRecord/issues\n%s\n", std::string(ex.what()));
			return false;
		}
		return true;
	}
};