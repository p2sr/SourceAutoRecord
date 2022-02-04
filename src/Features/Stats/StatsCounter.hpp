#pragma once
#include "Command.hpp"
#include "Features.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

#include <array>
#include <map>
#include <string>

#define SAR_MAP_COUNTER_EXPORT_HEADER "Map Name,CM Retries,CM Total Time,FG Reloads,FG Total Time"
#define SAR_CM_COUNTER_EXPORT_HEADER "Nb CM Reset,Total Time CM"
#define SAR_FULLGAME_COUNTER_EXPORT_HEADER "Completed Runs,Avg Reset Time,Nb Reset,Total Time SP,Total Time Coop,Portal Count"
#define SAR_TOTAL_COUNTER_EXPORT_HEADER "Total Time In-Game"

struct MapStats {
	bool coop;
	int CMretries;
	float CMTotalTime;
	int FullGameRetries;
	float FullGameTotalTime;
	float secondsSpent;
};

class StatsCounter : public Feature {
private:
	std::map<std::string, MapStats> mapStats;

	int completedRuns;
	float avgResetTime;
	int nbReset;
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

extern StatsCounter *statsCounter;

extern Variable sar_statcounter_filePath;
