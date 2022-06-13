#pragma once

#include "Math.hpp"

#define SIGNONSTATE_NONE 0
#define SIGNONSTATE_CHALLENGE 1
#define SIGNONSTATE_CONNECTED 2
#define SIGNONSTATE_NEW 3
#define SIGNONSTATE_PRESPAWN 4
#define SIGNONSTATE_SPAWN 5
#define SIGNONSTATE_FULL 6
#define SIGNONSTATE_CHANGELEVEL 7


typedef enum {
	HS_NEW_GAME = 0,
	HS_LOAD_GAME = 1,
	HS_CHANGE_LEVEL_SP = 2,
	HS_CHANGE_LEVEL_MP = 3,
	HS_RUN = 4,
	HS_GAME_SHUTDOWN = 5,
	HS_SHUTDOWN = 6,
	HS_RESTART = 7,
} HOSTSTATES;

struct CHostState {
	int m_currentState;            // 0
	int m_nextState;               // 4
	Vector m_vecLocation;          // 8, 12, 16
	QAngle m_angLocation;          // 20, 24, 28
	char m_levelName[256];         // 32
	char m_landmarkName[256];      // 288
	char m_saveName[256];          // 544
	float m_flShortFrameTime;      // 800
	bool m_activeGame;             // 804
	bool m_bRememberLocation;      // 805
	bool m_bBackgroundLevel;       // 806
	bool m_bWaitingForConnection;  // 807
};
