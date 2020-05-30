#pragma once
#include <map>
#include <string>

#define FR


#ifdef FR
#define CSV_SEPARATOR ';'
#else
#define CSV_SEPARATOR ','
#endif

#ifdef FR
#define SAR_CMCOUNTER_EXPORT_HEADER "Index;MapName;Retries;TimeSpent"
#define SAR_FULLGAMECOUNTER_EXPORT_HEADER "Index;MapName;Retries;TimeSpent;CompletedRuns;AvgResetTime;TotalTimeSP"
#else
#define SAR_CMCOUNTER_EXPORT_HEADER "Index,MapName,Retries,TimeSpent"
#define SAR_FULLGAMECOUNTER_EXPORT_HEADER "Index,MapName,Retries,TimeSpent,CompletedRuns,AvgResetTime,TotalTimeSP"
#endif


struct CMStats{
    int retries;
    int framesSpent;
};

struct FullGameStats{
    int retries;
    int framesSpent;
};


class StatsCounter {
public:
    
    std::map<std::string, CMStats> CMMapsStats;
    std::map<std::string, FullGameStats> fullGameMapsStats;

    int completedRuns;
    int avgResetTime;
    int totalTimeSP;
    //int totalTimeCoop; //When SAR will support Coop


public:
    void Reset();

    bool LoadFromFile(std::string filePath, std::string fullGamePath);
    bool ExportToFile(std::string filePath);
};
