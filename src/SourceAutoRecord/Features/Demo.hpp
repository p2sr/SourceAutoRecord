#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Modules/Console.hpp"

class Demo {
public:
    char demoFileStamp[8];
    int32_t demoProtocol;
    int32_t networkProtocol;
    char serverName[260];
    char clientName[260];
    char mapName[260];
    char gameDirectory[260];
    float playbackTime;
    int32_t playbackTicks;
    int32_t playbackFrames;
    int32_t signOnLength;
    std::vector<int32_t> messageTicks;

public:
    int32_t LastTick()
    {
        return (messageTicks.size() > 0 ) ? messageTicks.back() : playbackTicks;
    }
    float IntervalPerTick()
    {
        return playbackTime / playbackTicks;
    }
    float Tickrate()
    {
        return playbackTicks / playbackTime;
    }
};

// Basic demo parser to handle Portal and Portal 2 demos
class DemoParser {
public:
    bool headerOnly = false;
    int outputMode = 0;
    bool hasAlignmentByte = true;
    int maxSplitScreenClients = 2;

    void Adjust(Demo* demo)
    {
        float ipt = demo->IntervalPerTick();
        demo->playbackTicks = demo->LastTick();
        demo->playbackTime = ipt * demo->playbackTicks;
    }
    bool Parse(std::string filePath, Demo* demo)
    {
        try {
            if (filePath.substr(filePath.length() - 4, 4) != ".dem")
                filePath += ".dem";

            Console::DevMsg("Trying to parse \"%s\"...\n", filePath.c_str());

            std::ifstream file(filePath, std::ios::in | std::ios::binary);
            if (!file.good())
                return false;

            file.read(demo->demoFileStamp, sizeof(demo->demoFileStamp));
            file.read((char*)&demo->demoProtocol, sizeof(demo->demoProtocol));
            file.read((char*)&demo->networkProtocol, sizeof(demo->networkProtocol));
            file.read(demo->serverName, sizeof(demo->serverName));
            file.read(demo->clientName, sizeof(demo->clientName));
            file.read(demo->mapName, sizeof(demo->mapName));
            file.read(demo->gameDirectory, sizeof(demo->gameDirectory));
            file.read((char*)&demo->playbackTime, sizeof(demo->playbackTime));
            file.read((char*)&demo->playbackTicks, sizeof(demo->playbackTicks));
            file.read((char*)&demo->playbackFrames, sizeof(demo->playbackFrames));
            file.read((char*)&demo->signOnLength, sizeof(demo->signOnLength));

            if (!headerOnly) {
                if (demo->demoProtocol != 4) {
                    hasAlignmentByte = false;
                    maxSplitScreenClients = 1;
                }

                while (!file.eof() && !file.bad()) {
                    unsigned char cmd;
                    int32_t tick;

                    file.read((char*)&cmd, sizeof(cmd));
                    if (cmd == 0x07) // Stop
                        break;

                    file.read((char*)&tick, sizeof(tick));
                    if (tick >= 0)
                        demo->messageTicks.push_back(tick);
                    if (hasAlignmentByte)
                        file.ignore(1);

                    switch (cmd) {
                    case 0x01: // SignOn
                    case 0x02: // Packet
                    {
                        if (outputMode == 2) {
                            for (int i = 0; i < maxSplitScreenClients; i++) {
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
                        } else {
                            file.ignore((maxSplitScreenClients * 76) + 4 + 4);
                        }

                        int32_t length;
                        file.read((char*)&length, sizeof(length));
                        file.ignore(length);
                        break;
                    }
                    case 0x03: // SyncTick
                        continue;
                    case 0x04: // ConsoleCmd:
                    {
                        int32_t length;
                        file.read((char*)&length, sizeof(length));
                        if (outputMode >= 1) {
                            std::string cmd(length, ' ');
                            file.read(&cmd[0], length);
                            Console::Msg("[%i] %s\n", tick, cmd.c_str());
                        } else {
                            file.ignore(length);
                        }
                        break;
                    }
                    case 0x05: // UserCmd:
                    {
                        int32_t cmd;
                        int32_t length;
                        file.read((char*)&cmd, sizeof(cmd));
                        file.read((char*)&length, sizeof(length));
                        file.ignore(length);
                        break;
                    }
                    case 0x06: // DataTables:
                    {
                        int32_t length;
                        file.read((char*)&length, sizeof(length));
                        file.ignore(length);
                        break;
                    }
                    case 0x08: // CustomData or StringTables
                    {
                        if (demo->demoProtocol == 4) {
                            int32_t unk;
                            int32_t length;
                            file.read((char*)&unk, sizeof(unk));
                            file.read((char*)&length, sizeof(length));
                            file.ignore(length);
                        } else {
                            int32_t length;
                            file.read((char*)&length, sizeof(length));
                            file.ignore(length);
                        }
                        break;
                    }
                    case 0x09: // StringTables
                    {
                        if (demo->demoProtocol != 4)
                            return false;
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
        } catch (const std::exception& ex) {
            Console::Warning("SAR: Error occurred when trying to parse the demo file.\nIf you think this is an issue, report it at: https://github.com/NeKzor/SourceAutoRecord/issues\n%s\n", std::string(ex.what()));
            return false;
        }
        return true;
    }
};