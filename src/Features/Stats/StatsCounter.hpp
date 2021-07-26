#pragma once
#include "Command.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

#include <array>
#include <map>
#include <string>

#define SAR_CM_COUNTER_EXPORT_HEADER "Map Name,CM Retries,CM Time Spent,FullGame Reloads,FullGame Time Spent"
#define SAR_FULLGAME_COUNTER_EXPORT_HEADER "Completed Runs,Avg Reset Time,Nb Reset,Total Time SP,Portal Count"
#define SAR_TOTAL_COUNTER_EXPORT_HEADER "Total Time In-Game"

struct CMStats {
	int retries;
	float secondSpent;
};

struct FullGameStats {
	int retries;
	float secondSpent;
};

class StatsCounter {
private:
	std::map<std::string, CMStats> CMMapsStats;
	std::map<std::string, FullGameStats> fullGameMapsStats;

	int completedRuns;
	float avgResetTime;
	int nbReset;
	float totalTimeSP;
	//int totalTimeCoop; //When SAR will support Coop
	int portalCount;

	float totalTimeInGame;

public:
public:
	void Reset();
	void IncrementReset(const float time);
	void IncrementRunFinished(const float time);

	void Init();
	bool LoadFromFile(const std::string &path);
	bool ExportToFile(const std::string &path);
	void Print();
	void RecordDatas(const int tick);
};

extern Variable sar_statcounter_filePath;
