#pragma once
#include <cstdint>
#include <vector>

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
	int32_t firstPositivePacketTick;
	int32_t segmentTicks;

public:
	int32_t LastTick();
	float IntervalPerTick();
	float Tickrate();
};
