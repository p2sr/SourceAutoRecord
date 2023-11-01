#pragma once
#include "Utils/json11.hpp"

#include <optional>
#include <string>
#include <map>

namespace AutoSubmitMod {
	void LoadApiKey(bool output_nonexist);
	void FinishRun(float final_time, const char *demopath, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb);
	std::optional<std::string> GetMapId(std::string map_name);
	std::optional<int> GetCurrentPbScore(std::string &map_id);
	json11::Json::object GetMapJson(std::string &map_id);
};  // namespace AutoSubmitMod
