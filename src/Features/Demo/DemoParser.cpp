#include "DemoParser.hpp"

#include "Command.hpp"
#include "Demo.hpp"
#include "Features/Demo/DemoGhostPlayer.hpp"
#include "Features/Hud/Hud.hpp"
#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Variable.hpp"

#include <fstream>
#include <sstream>
#include <string>

Variable sar_time_demo_dev("sar_time_demo_dev", "0", 0,
                           "Printing mode when using sar_time_demo.\n"
                           "0 = Default,\n"
                           "1 = Console commands,\n"
                           "2 = Console commands & packets.\n");

DemoParser::DemoParser()
	: headerOnly(false)
	, outputMode()
	, hasAlignmentByte(true)
	, maxSplitScreenClients(2) {
}
void DemoParser::Adjust(Demo *demo) {
	auto ipt = demo->IntervalPerTick();
	demo->playbackTicks = demo->LastTick();
	demo->playbackTime = ipt * demo->playbackTicks;
}
bool DemoParser::Parse(std::string filePath, Demo *demo, bool ghostRequest, std::map<int, DataGhost> *datas) {
	bool gotFirstPositivePacket = false;
	bool gotSync = false;
	try {
		if (filePath.substr(filePath.length() - 4, 4) != ".dem")
			filePath += ".dem";

		console->DevMsg("Trying to parse \"%s\"...\n", filePath.c_str());

		std::ifstream file(filePath, std::ios::in | std::ios::binary);
		if (!file.good())
			return false;

		file.read(demo->demoFileStamp, sizeof(demo->demoFileStamp));
		file.read((char *)&demo->demoProtocol, sizeof(demo->demoProtocol));
		file.read((char *)&demo->networkProtocol, sizeof(demo->networkProtocol));
		file.read(demo->serverName, sizeof(demo->serverName));
		file.read(demo->clientName, sizeof(demo->clientName));
		file.read(demo->mapName, sizeof(demo->mapName));
		file.read(demo->gameDirectory, sizeof(demo->gameDirectory));
		file.read((char *)&demo->playbackTime, sizeof(demo->playbackTime));
		file.read((char *)&demo->playbackTicks, sizeof(demo->playbackTicks));
		file.read((char *)&demo->playbackFrames, sizeof(demo->playbackFrames));
		file.read((char *)&demo->signOnLength, sizeof(demo->signOnLength));

		demo->segmentTicks = -1;

		if (!headerOnly) {
			//Ghosts
			bool waitForNext = false;
			int lastTick = 0;

			if (demo->demoProtocol != 4) {
				this->hasAlignmentByte = false;
				this->maxSplitScreenClients = 1;
			}

			while (!file.eof() && !file.bad()) {
				unsigned char cmd;
				int32_t tick;

				file.read((char *)&cmd, sizeof(cmd));
				if (cmd == 0x07)  // Stop
					break;

				file.read((char *)&tick, sizeof(tick));

				// Save positive ticks to keep adjustments simple
				if (tick >= 0)
					demo->messageTicks.push_back(tick);

				if (this->hasAlignmentByte)
					file.ignore(1);

				switch (cmd) {
				case 0x01:  // SignOn
				case 0x02:  // Packet
				{
					if (tick > 0 && gotSync && !gotFirstPositivePacket) {
						demo->firstPositivePacketTick = tick;
						gotFirstPositivePacket = true;
					}

					if (outputMode == 2 || ghostRequest == true) {
						for (auto i = 0; i < this->maxSplitScreenClients; ++i) {
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
							file.read((char *)&flags, sizeof(flags));
							file.read((char *)&vo_x, sizeof(vo_x));
							file.read((char *)&vo_y, sizeof(vo_y));
							file.read((char *)&vo_z, sizeof(vo_z));
							file.read((char *)&va_x, sizeof(va_x));
							file.read((char *)&va_y, sizeof(va_y));
							file.read((char *)&va_z, sizeof(va_z));
							file.read((char *)&lva_x, sizeof(lva_x));
							file.read((char *)&lva_y, sizeof(lva_y));
							file.read((char *)&lva_z, sizeof(lva_z));
							file.read((char *)&vo2_x, sizeof(vo2_x));
							file.read((char *)&vo2_y, sizeof(vo2_y));
							file.read((char *)&vo2_z, sizeof(vo2_z));
							file.read((char *)&va2_x, sizeof(va2_x));
							file.read((char *)&va2_y, sizeof(va2_y));
							file.read((char *)&va2_z, sizeof(va2_z));
							file.read((char *)&lva2_x, sizeof(lva2_x));
							file.read((char *)&lva2_y, sizeof(lva2_y));
							file.read((char *)&lva2_z, sizeof(lva2_z));

							if (ghostRequest) {
								if (tick == 0) {
									waitForNext = true;
								}

								if (tick > 0 && waitForNext && lastTick != tick) {
									lastTick = tick;
									(*datas)[tick] = DataGhost{{vo_x, vo_y, vo_z}, {va_x, va_y, va_z}};
								}
							} else {
								console->Msg(
									"[%i] flags: %i | "
									"view origin: %.3f/%.3f/%.3f | "
									"view angles: %.3f/%.3f/%.3f | "
									"local view angles: %.3f/%.3f/%.3f\n",
									tick,
									flags,
									vo_x,
									vo_y,
									vo_z,
									va_x,
									va_y,
									va_z,
									lva_x,
									lva_y,
									lva_z);
							}
						}
						int32_t in_seq, out_seq;
						file.read((char *)&in_seq, sizeof(in_seq));
						file.read((char *)&out_seq, sizeof(out_seq));
					} else {
						file.ignore((this->maxSplitScreenClients * 76) + 4 + 4);
					}

					int32_t length;
					file.read((char *)&length, sizeof(length));
					file.ignore(length);
					break;
				}
				case 0x03:  // SyncTick
					gotSync = true;
					continue;
				case 0x04:  // ConsoleCmd
				{
					int32_t length;
					file.read((char *)&length, sizeof(length));
					std::string cmd(length, ' ');
					file.read(&cmd[0], length);
					if (!ghostRequest && outputMode >= 1) {
						console->Msg("[%i] %s\n", tick, cmd.c_str());
					}

					if (cmd.find("__END__") != std::string::npos) {
						console->ColorMsg(Color(0, 255, 0, 255), "Segment length -> %d ticks: %.3fs\n", tick, tick / 60.f);
						demo->segmentTicks = tick;
					}
					break;
				}
				case 0x05:  // UserCmd
				{
					int32_t cmd;
					int32_t length;
					file.read((char *)&cmd, sizeof(cmd));
					file.read((char *)&length, sizeof(length));
					file.ignore(length);
					break;
				}
				case 0x06:  // DataTables
				{
					int32_t length;
					file.read((char *)&length, sizeof(length));
					file.ignore(length);
					break;
				}
				case 0x08:  // CustomData or StringTables
				{
					if (demo->demoProtocol == 4) {
						int32_t unk;
						int32_t length;
						file.read((char *)&unk, sizeof(unk));
						file.read((char *)&length, sizeof(length));
						file.ignore(length);
					} else {
						int32_t length;
						file.read((char *)&length, sizeof(length));
						file.ignore(length);
					}
					break;
				}
				case 0x09:  // StringTables
				{
					if (demo->demoProtocol != 4)
						return false;
					int32_t length;
					file.read((char *)&length, sizeof(length));
					file.ignore(length);
					break;
				}
				default:
					return false;
				}
			}
		}
		file.close();
	} catch (const std::exception &ex) {
		console->Warning(
			"SAR: Error occurred when trying to parse the demo file.\n"
			"If you think this is an issue, report it at: https://github.com/NeKzor/SourceAutoRecord/issues\n"
			"%s\n",
			std::string(ex.what()));
		return false;
	}
	return true;
}

// Commands

CON_COMMAND_AUTOCOMPLETEFILE(sar_time_demo, "sar_time_demo <demo_name> - parses a demo and prints some information about it\n", 0, 0, dem) {
	if (args.ArgC() != 2) {
		return console->Print(sar_time_demo.ThisPtr()->m_pszHelpString);
	}

	std::string name;
	if (args[1][0] == '\0') {
		if (engine->demoplayer->DemoName[0] != '\0') {
			name = std::string(engine->demoplayer->DemoName);
		} else {
			return console->Print("No demo was recorded or played back!\n");
		}
	} else {
		name = std::string(args[1]);
	}

	DemoParser parser;
	parser.outputMode = sar_time_demo_dev.GetInt();

	Demo demo;
	auto dir = std::string(engine->GetGameDirectory()) + std::string("/") + name;
	if (parser.Parse(dir, &demo)) {
		parser.Adjust(&demo);
		console->Print("Demo:     %s\n", name.c_str());
		console->Print("Client:   %s\n", demo.clientName);
		console->Print("Map:      %s\n", demo.mapName);
		console->Print("Ticks:    %i\n", demo.playbackTicks);
		console->Print("Time:     %.3f\n", demo.playbackTime);
		console->Print("Tickrate: %.3f\n", demo.Tickrate());
	} else {
		console->Print("Could not parse \"%s\"!\n", name.c_str());
	}
}
CON_COMMAND_AUTOCOMPLETEFILE(sar_time_demos, "sar_time_demos <demo_name> [demo_name2]... - parses multiple demos and prints the total sum of them\n", 0, 0, dem) {
	if (args.ArgC() <= 1) {
		return console->Print(sar_time_demos.ThisPtr()->m_pszHelpString);
	}

	auto totalTicks = 0;
	auto totalTime = 0.f;
	auto printTotal = false;

	DemoParser parser;
	parser.outputMode = sar_time_demo_dev.GetInt();

	auto name = std::string();
	auto dir = std::string(engine->GetGameDirectory()) + std::string("/");
	for (auto i = 1; i < args.ArgC(); ++i) {
		name = std::string(args[i]);

		Demo demo;
		if (parser.Parse(dir + name, &demo)) {
			parser.Adjust(&demo);
			console->Print("Demo:     %s\n", name.c_str());
			console->Print("Client:   %s\n", demo.clientName);
			console->Print("Map:      %s\n", demo.mapName);
			console->Print("Ticks:    %i\n", demo.playbackTicks);
			console->Print("Time:     %.3f\n", demo.playbackTime);
			console->Print("Tickrate: %.3f\n", demo.Tickrate());
			console->Print("---------------\n");
			totalTicks += demo.playbackTicks;
			totalTime += demo.playbackTime;
			printTotal = true;
		} else {
			console->Print("Could not parse \"%s\"!\n", name.c_str());
		}
	}

	if (printTotal) {
		console->Print("Total Ticks: %i\n", totalTicks);
		console->Print("Total Time: %.3f\n", totalTime);
	}
}

// HUD

HUD_ELEMENT_MODE2(demo, "0", 0, 2, "Draws name, tick and time of current demo.\n", HudType_InGame | HudType_Paused | HudType_LoadingScreen) {
	int tick;
	float time;
	const char *demoName;

	if (!*engine->m_bLoadgame && *engine->demorecorder->m_bRecording && !engine->demorecorder->currentDemo.empty()) {
		tick = engine->demorecorder->GetTick();
		time = engine->ToTime(tick);
		demoName = engine->demorecorder->currentDemo.c_str();
	} else if (!*engine->m_bLoadgame && engine->demoplayer->IsPlaying()) {
		tick = engine->demoplayer->GetTick();
		time = engine->ToTime(tick);
		demoName = engine->demoplayer->DemoName;
	} else {
		ctx->DrawElement("demo: -");
		return;
	}

	if (mode == 1) {
		ctx->DrawElement("demo: %s %i (%.3f)", demoName, tick, time);
	} else { // mode == 2
		const char *name = demoName;

		while (*name) ++name; // Find end of path

		// Go back until slash
		while (name >= demoName && *name != '/' && *name != '\\') {
			--name;
		}

		++name;

		ctx->DrawElement("demo: %s %i", name, tick);
	}
}
