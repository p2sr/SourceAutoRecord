#include "StatsCounter.hpp"

#include "Command.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

Variable sar_statcounter_filePath("sar_statcounter_filePath", "Stats/phunkpaiDWPS.csv", "Path to the statcounter .csv file.\n", 0);

void StatsCounter::Reset() {
	this->CMMapsStats.clear();
	this->fullGameMapsStats.clear();
	this->completedRuns = 0;
	this->avgResetTime = 0;
	this->nbReset = 0;
	this->portalCount = 0;
	this->totalTimeSP = 0;
	this->totalTimeInGame = 0;
}

void StatsCounter::IncrementReset(const float time) {
	auto tmp = this->avgResetTime * this->nbReset + time;
	++this->nbReset;  //Avoid undefined behavior
	this->avgResetTime += tmp / this->nbReset;
}

void StatsCounter::IncrementRunFinished(const float time) {
	++this->completedRuns;
}

void StatsCounter::Init() {
	if (!this->LoadFromFile(sar_statcounter_filePath.GetString())) {
		console->Print("Couldn't open the file. Are you sure the file is here? : \"%s\".\n", sar_statcounter_filePath.GetString());
		return;
	}
	console->Print("Datas has been successfully loaded.\n");
}

bool StatsCounter::LoadFromFile(const std::string &path) {
	std::string filePath = std::string(engine->GetGameDirectory()) + std::string("/") + path;
	if (filePath.substr(filePath.length() - 4, 4) != ".csv")
		filePath += ".csv";

	std::ifstream file(filePath, std::ios::in);
	if (!file.good()) {
		file.close();
		return false;
	}

	std::vector<std::string> stats;
	std::string header, line, word, tmp;

	std::getline(file, header);  //Skip "sep=,"
	std::getline(file, header);

	if (header != SAR_CM_COUNTER_EXPORT_HEADER) {
		return false;
	}

	while (std::getline(file, line)) {
		if (line == SAR_FULLGAME_COUNTER_EXPORT_HEADER) {
			break;
		}

		stats.clear();
		std::stringstream s(line);

		while (std::getline(s, word, CSV_SEPARATOR)) {
			if (!word.empty())
				stats.push_back(word);
		}

		this->CMMapsStats[stats[0]].retries = std::stoi(stats[1]);
		this->CMMapsStats[stats[0]].secondSpent = SpeedrunTimer::UnFormat(stats[2]);
		this->fullGameMapsStats[stats[0]].retries = std::stoi(stats[3]);
		this->fullGameMapsStats[stats[0]].secondSpent = SpeedrunTimer::UnFormat(stats[4]);
	}

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
			this->totalTimeSP = SpeedrunTimer::UnFormat(stats[3]);
			this->portalCount = std::stoi(stats[4]);
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
	std::string filePath = std::string(engine->GetGameDirectory()) + std::string("/") + path;
	if (filePath.substr(filePath.length() - 4, 4) != ".csv")
		filePath += ".csv";

	std::ofstream file(filePath, std::ios::out | std::ios::trunc);
	if (!file.good()) {
		file.close();
		return false;
	}

	file << MICROSOFT_PLEASE_FIX_YOUR_SOFTWARE_SMHMYHEAD << std::endl;

	file << SAR_CM_COUNTER_EXPORT_HEADER << std::endl;

	for (auto &map : Game::mapNames) {
		file << map
							<< CSV_SEPARATOR << this->CMMapsStats[map].retries
							<< CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(this->CMMapsStats[map].secondSpent).c_str()
							<< CSV_SEPARATOR << this->fullGameMapsStats[map].retries
							<< CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(this->fullGameMapsStats[map].secondSpent).c_str()
							<< std::endl;
	}

	file << SAR_FULLGAME_COUNTER_EXPORT_HEADER << std::endl;

	file << this->completedRuns
						<< CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(this->avgResetTime).c_str()
						<< CSV_SEPARATOR << this->nbReset
						<< CSV_SEPARATOR << SpeedrunTimer::SimpleFormat(this->totalTimeSP).c_str()
						<< CSV_SEPARATOR << this->portalCount
						<< std::endl;

	file << SAR_TOTAL_COUNTER_EXPORT_HEADER << std::endl;

	file << SpeedrunTimer::SimpleFormat(this->totalTimeInGame);

	file.close();
	return true;
}

void StatsCounter::Print() {
	auto current = 1;
	auto total = Game::mapNames.size();
	for (auto &map : Game::mapNames) {
		console->Print("[%i of %i] Map %s\n", current++, total, map.c_str());
		console->Msg("    -> CM retries: %d\n", this->CMMapsStats[map].retries);
		console->Msg("    -> CM time spent: %s\n", SpeedrunTimer::Format(this->CMMapsStats[map].secondSpent).c_str());
		console->Msg("    -> Full Game reloads: %d\n", this->fullGameMapsStats[map].retries);
		console->Msg("    -> Full Game time spent: %s\n", SpeedrunTimer::Format(this->fullGameMapsStats[map].secondSpent).c_str());
	}

	console->Print("\nOther Full Game stats:\n");
	console->Msg("    -> Completed runs: %d\n", this->completedRuns);
	console->Msg("    -> Average Reset Time: %s\n", SpeedrunTimer::Format(this->avgResetTime).c_str());
	console->Msg("    -> Reset count: %d\n", this->nbReset);
	console->Msg("    -> Total Time SP: %s\n", SpeedrunTimer::Format(this->totalTimeSP).c_str());
	console->Msg("    -> Total Portal count: %d\n", this->portalCount);
	console->Msg("    -> Total Time In-Game: %s\n", SpeedrunTimer::Format(this->totalTimeInGame).c_str());
}

void StatsCounter::RecordDatas(const int tick) {
	if (sv_bonus_challenge.GetBool()) {
		this->CMMapsStats[engine->GetCurrentMapName()].secondSpent += engine->ToTime(tick);
		this->CMMapsStats[engine->GetCurrentMapName()].retries++;
	} else {
		this->fullGameMapsStats[engine->GetCurrentMapName()].secondSpent += engine->ToTime(tick);
		this->fullGameMapsStats[engine->GetCurrentMapName()].retries++;
		this->totalTimeSP += engine->ToTime(tick);
	}

	this->totalTimeInGame += engine->ToTime(tick);

	auto nSlot = GET_SLOT();
	auto player = server->GetPlayer(nSlot + 1);
	if (player) {
		this->portalCount += server->GetPortals(player);
	}
}
