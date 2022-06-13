#pragma once

#include "EntityEdict.hpp"

enum MapLoadType_t {
	MapLoad_NewGame = 0,
	MapLoad_LoadGame = 1,
	MapLoad_Transition = 2,
	MapLoad_Background = 3
};


struct CGlobalVarsBase {
	float realtime;                 // 0
	int framecount;                 // 4
	float absoluteframetime;        // 8
	float curtime;                  // 12
	float frametime;                // 16
	int maxClients;                 // 20
	int tickcount;                  // 24
	float interval_per_tick;        // 28
	float interpolation_amount;     // 32
	int simTicksThisFrame;          // 36
	int network_protocol;           // 40
	void *pSaveData;                // 44
	bool m_bClient;                 // 48
	int nTimestampNetworkingBase;   // 52
	int nTimestampRandomizeWindow;  // 56
};

struct CGlobalVars : CGlobalVarsBase {
	char *mapname;            // 60
	int mapversion;           // 64
	char *startspot;          // 68
	MapLoadType_t eLoadType;  // 72
	char bMapLoadFailed;      // 76
	char deathmatch;          // 77
	char coop;                // 78
	char teamplay;            // 79
	int maxEntities;          // 80
	int serverCount;          // 84
	edict_t *pEdicts;         // 88
};
