#include "StatsCounter.hpp"

#include "Command.hpp"
#include "Event.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Engine.hpp"
#include "Modules/FileSystem.hpp"
#include "Modules/Server.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

Variable sar_statcounter_filePath("sar_statcounter_filePath", "Stats/phunkpaiDWPS.csv", "Path to the statcounter .csv file.\n", 0);

StatsCounter *statsCounter;

void StatsCounter::Reset() {
	this->mapStats.clear();
	this->completedRuns = 0;
	this->avgResetTime = 0;
	this->nbReset = 0;
	this->portalCount = 0;
	this->totalTimeInGame = 0;
}

void StatsCounter::IncrementReset(const float time) {
	if (sv_bonus_challenge.GetBool()) return;

	auto tmp = this->avgResetTime * this->nbReset + time;
	++this->nbReset;  //Avoid undefined behavior
	this->avgResetTime += tmp / this->nbReset;
}

void StatsCounter::IncrementRunFinished(const float time) {
	++this->completedRuns;
}

void StatsCounter::Init() {
	if (this->LoadFromFile(sar_statcounter_filePath.GetString())) {
		console->Print("Data has been successfully loaded.\n");
	}
}

ON_EVENT(CONFIG_EXEC) {
	statsCounter->Init();
}

bool StatsCounter::LoadFromFile(const std::string &path) {
	auto filePath = std::string(path);
	if (!Utils::EndsWith(filePath, ".csv")) filePath = filePath + ".csv";
	filePath = fileSystem->FindFileSomewhere(filePath).value_or(filePath);

	std::ifstream file(filePath, std::ios::in);
	if (!file.good()) {
		file.close();
		return false;
	}

	std::vector<std::string> stats;
	std::string header, line, word, tmp;

	std::getline(file, header);  //Skip "sep=,"
	std::getline(file, header);

	if (header != SAR_MAP_COUNTER_EXPORT_HEADER) {
		return false;
	}

	while (std::getline(file, line)) {
		if (line == SAR_CM_COUNTER_EXPORT_HEADER) {
			break;
		}

		stats.clear();
		std::stringstream s(line);

		while (std::getline(s, word, CSV_SEPARATOR)) {
			if (!word.empty())
				stats.push_back(word);
		}

		auto &mapStat = this->mapStats[stats[0]];

		mapStat.CMretries = std::stoi(stats[1]);
		mapStat.CMTotalTime = SpeedrunTimer::UnFormat(stats[2]);
		mapStat.FullGameRetries = std::stoi(stats[3]);
		mapStat.FullGameTotalTime = SpeedrunTimer::UnFormat(stats[4]);
		mapStat.secondsSpent = mapStat.CMTotalTime + mapStat.FullGameTotalTime;
		mapStat.coop = stats[0].rfind("mp_coop", 0) == 0 ? true : false;
	}


	//Don't read CM
	std::getline(file, line);
	std::getline(file, line);


	if (line == SAR_FULLGAME_COUNTER_EXPORT_HEADER) {
		if (std::getline(file, line)) {
			stats.clear();
			std::stringstream s(line);

			while (std::getline(s, word, CSV_SEPARATOR)) {
				if (!word.empty())
					stats.push_back(word);
			}

			this->completedRuns = std::stoi(stats[0]);
			this->avgResetTime = SpeedrunTimer::UnFormat(stats[1]);
			this->nbReset = std::stoi(stats[2]);
			this->portalCount = std::stoi(stats[5]);
		}
	}

	std::getline(file, header);
	if (header != SAR_TOTAL_COUNTER_EXPORT_HEADER) {
		return false;
	}
	if (std::getline(file, line)) {
		stats.clear();
		std::stringstream s(line);

		while (std::getline(s, word, CSV_SEPARATOR)) {
			if (!word.empty())
				stats.push_back(word);
		}

		this->totalTimeInGame = SpeedrunTimer::UnFormat(stats[0]);
	}

	return true;
}

bool StatsCounter::ExportToFile(const std::string &path) {
	std::string filePath = path;
	if (!Utils::EndsWith(filePath, ".csv")) filePath = filePath + ".csv";

	auto filepath = fileSystem->FindFileSomewhere(filePath).value_or(filePath);
	std::ofstream file(filepath, std::ios::out | std::ios::trunc);
	if (!file.good()) {
		file.close();
		return false;
	}

	file << MICROSOFT_PLEASE_FIX_YOUR_SOFTWARE_SMHMYHEAD << std::endl;
	file << SAR_MAP_COUNTER_EXPORT_HEADER << std::endl;

	for(auto &map : this->mapStats) {
		file << map.first
							<< CSV_SEPARATOR << map.second.CMretries
							<< CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(map.second.CMTotalTime).c_str()
							<< CSV_SEPARATOR << map.second.FullGameRetries
							<< CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(map.second.FullGameTotalTime).c_str()
							<< std::endl;
	}

	auto CMRetries = std::accumulate(std::begin(this->mapStats), std::end(this->mapStats), 0, [](int retries, auto &map) { return retries + map.second.CMretries; });
	auto CMTime = std::accumulate(std::begin(this->mapStats), std::end(this->mapStats), 0.f, [](float time, auto &map) { return time + map.second.CMTotalTime; });

	file << SAR_CM_COUNTER_EXPORT_HEADER << std::endl;

	file << CMRetries << CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(CMTime) << std::endl;

	auto totalTimeSP = std::accumulate(std::begin(this->mapStats), std::end(this->mapStats), 0.f, [](float time, auto &map) {
		if (!map.second.coop) return time + map.second.FullGameTotalTime + map.second.CMTotalTime;
		else return time;
	});
	auto totalTimeCoop = std::accumulate(std::begin(this->mapStats), std::end(this->mapStats), 0.f, [](float time, auto &map) {
		if (map.second.coop) return time + map.second.FullGameTotalTime + map.second.CMTotalTime;
		else return time;
	});

	file << SAR_FULLGAME_COUNTER_EXPORT_HEADER << std::endl;

	file << this->completedRuns
						<< CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(this->avgResetTime).c_str()
						<< CSV_SEPARATOR << this->nbReset
						<< CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(totalTimeSP).c_str()
						<< CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(totalTimeCoop).c_str()
						<< CSV_SEPARATOR << this->portalCount
						<< std::endl;

	file << SAR_TOTAL_COUNTER_EXPORT_HEADER << std::endl;

	file << SpeedrunTimer::SimpleFormat(this->totalTimeInGame);

	file.close();
	return true;
}

void StatsCounter::Print() {

	int current = 1;
	int total = this->mapStats.size();
	auto CMRetries = std::accumulate(std::begin(this->mapStats), std::end(this->mapStats), 0, [](int retries, auto &map) { return retries + map.second.CMretries; });
	auto CMTime = std::accumulate(std::begin(this->mapStats), std::end(this->mapStats), 0.f, [](float time, auto &map) { return time + map.second.CMTotalTime; });

	for(auto &map : this->mapStats){
		console->Print("[%i of %i] Map %s\n", current++, total, map.first.c_str());
		console->Msg("    -> CM retries: %d\n", map.second.CMretries);
		console->Msg("    -> CM time spent: %s\n", SpeedrunTimer::Format(map.second.CMTotalTime).c_str());
		console->Msg("    -> Full Game reloads: %d\n", map.second.FullGameRetries);
		console->Msg("    -> Full Game time spent: %s\n", SpeedrunTimer::Format(map.second.FullGameTotalTime).c_str());
	}

	auto totalTimeSP = std::accumulate(std::begin(this->mapStats), std::end(this->mapStats), 0.f, [](float time, auto &map) {
		if (!map.second.coop) return time + map.second.FullGameTotalTime + map.second.CMTotalTime;
		else return time;
	});
	auto totalTimeCoop = std::accumulate(std::begin(this->mapStats), std::end(this->mapStats), 0.f, [](float time, auto &map) {
		if (map.second.coop) return time + map.second.FullGameTotalTime + map.second.CMTotalTime;
		else return time;
	});

	console->Print("\nOther Full Game stats:\n");
	console->Msg("    -> Completed runs: %d\n", this->completedRuns);
	console->Msg("    -> Average Reset Time: %s\n", SpeedrunTimer::Format(this->avgResetTime).c_str());
	console->Msg("    -> Reset count: %d\n", this->nbReset);
	console->Msg("    -> Total Time SP: %s\n", SpeedrunTimer::Format(totalTimeSP).c_str());
	console->Msg("    -> Total Time Coop: %s\n", SpeedrunTimer::Format(totalTimeCoop).c_str());
	console->Msg("    -> Total Portal count: %d\n", this->portalCount);
	console->Msg("    -> Total Time In-Game: %s\n", SpeedrunTimer::Format(this->totalTimeInGame).c_str());
	console->Msg("    -> Total CM retries: %d\n", CMRetries);
	console->Msg("    -> Total CM time spent: %s\n", SpeedrunTimer::Format(CMTime).c_str());
}

void StatsCounter::RecordData(const int tick) {
	
	auto currentMap = engine->GetCurrentMapName();
	if (currentMap.empty()) return;

	auto &mapStats = this->mapStats[currentMap];

	if (sv_bonus_challenge.GetBool()) {
		mapStats.CMretries++;
		mapStats.CMTotalTime += engine->ToTime(tick);

	} else {
		mapStats.FullGameRetries++;
		mapStats.FullGameTotalTime += engine->ToTime(tick);
	}

	mapStats.coop = engine->IsCoop();

	this->totalTimeInGame += engine->ToTime(tick);

	auto nSlot = GET_SLOT();
	auto player = server->GetPlayer(nSlot + 1);
	if (player) {
		this->portalCount += server->GetPortals(player);
	}
}
