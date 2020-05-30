#include "StatsCounter.hpp"

#include "Command.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"

#include "Modules/Engine.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

void StatsCounter::Reset()
{
}

bool StatsCounter::LoadFromFile(std::string CMPath, std::string fullGamePath)
{
    std::string CMfilepath = std::string(engine->GetGameDirectory()) + std::string("/Stats/") + CMPath;
    if (CMfilepath.substr(CMfilepath.length() - 4, 4) != ".csv")
        CMfilepath += ".csv";

    std::ifstream CMfile(CMfilepath, std::ios::in);
    if (!CMfile.good()) {
        CMfile.close();
        return false;
    }

    /*std::fstream FullGamefile(fullGamePath, std::ios::in);
    if (!FullGamefile.good()) {
        FullGamefile.close();
        return false;
    }*/


    //CM

    std::vector<std::string> CMstats, FullGameStats;
    std::string CMheader, FullGameHeader, line, data, tmp;

    if (CMfile >> tmp) {
        std::getline(CMfile, CMheader);
    }

    if (CMheader != SAR_CMCOUNTER_EXPORT_HEADER) {
        return false;
    }

    while (CMfile >> tmp) {
        CMstats.clear();
        std::getline(CMfile, line);
        std::stringstream s(line);

        while (std::getline(s, data, CSV_SEPARATOR)) {
            CMstats.push_back(data);
        }

        this->CMMapsStats[CMstats[1]].retries = std::stoi(CMstats[2]);
        this->CMMapsStats[CMstats[1]].framesSpent = SpeedrunTimer::UnFormat(CMstats[3]);
    }

    //Full Game

   /* if (FullGameHeader != SAR_FULLGAMECOUNTER_EXPORT_HEADER) {
        return false;
    }

    while (FullGamefile >> tmp) {
        FullGameStats.clear();
        std::getline(FullGamefile, line);
        std::stringstream s(line);

        while (std::getline(s, data, CSV_SEPARATOR)) {
            CMstats.push_back(data);
        }

        this->CMMapsStats[FullGameStats[1]].retries = std::stoi(FullGameStats[2]);

        std::tm t;
        std::istringstream ss(FullGameStats[3]);
        ss >> std::get_time(&t, "%H:%M.%S");
        this->fullGameMapsStats[FullGameStats[1]].framesSpent = (t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec) * 60;
    }*/

    return true;
}

bool StatsCounter::ExportToFile(std::string filePath)
{
    if (this->CMMapsStats.empty() || this->fullGameMapsStats.empty()) {
        return false;
    }

    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.good()) {
        file.close();
        return false;
    }

    file << SAR_CMCOUNTER_EXPORT_HEADER << std::endl;

    auto current = 1;
    for (const auto& item : this->CMMapsStats) {
        file << current++
             << CSV_SEPARATOR << item.first
             << CSV_SEPARATOR << item.second.retries
             << CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(item.second.framesSpent * speedrun->GetIntervalPerTick()).c_str()
             << std::endl;
    }

    file.close();
    return true;
}
