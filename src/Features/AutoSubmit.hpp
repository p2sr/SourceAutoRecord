#pragma once

#include <optional>
#include <string>

extern std::string g_partner_name;

namespace AutoSubmit {
	void FinishRun(float final_time, const char *demopath, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb);
};
