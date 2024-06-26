#pragma once

#include <optional>
#include <string>

namespace AutoSubmit {
	extern bool g_cheated;
	extern std::string g_partner_name;

	void LoadApiKey(bool output_nonexist);
	void FinishRun(float final_time, const char *demopath, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb);
};
