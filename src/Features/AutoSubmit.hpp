#pragma once
#include "Utils/json11.hpp"

#include <map>
#include <optional>
#include <string>

// NOTE: this actually has nothing to do with the game's PortalLeaderboardItem_t
struct PortalLeaderboardItem_t {
	char name[32];
	char autorender[16];
	uint8_t *avatarTex;
	int32_t rank;
	int32_t score;
};

namespace AutoSubmit {
	extern bool g_cheated;
	extern std::string g_partner_name;

	void LoadApiKey(bool output_nonexist);
	void FinishRun(float final_time, const char *demopath, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb);
	std::optional<std::string> GetMapId(std::string map_name);
	void Search(std::string map);
	json11::Json::object GetTopScores(std::string &map_id);
	bool IsQuerying();
	const std::vector<PortalLeaderboardItem_t> &GetTimes();
};
