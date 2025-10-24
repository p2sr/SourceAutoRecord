#pragma once
#include "Utils/json11.hpp"

#include <optional>
#include <string>

namespace AutoSubmit {
	extern bool g_cheated;
	extern int g_paused;
	extern std::string g_partner_name;

	void LoadApiKey(bool output_nonexist);
	void FinishRun(float final_time, const char *demopath, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb);
	std::optional<std::string> GetMapId(std::string map_name);
	void Search(std::string map);
	json11::Json::array GetTopScores(std::string &map_id);
	bool IsQuerying();
	const std::vector<json11::Json>& GetTimes();
};
