#pragma once

#include <optional>
#include <string>

namespace AutoSubmitMod {
	void LoadApiKey(bool output_nonexist);
	void FinishRun(float final_time, const char *demopath, std::optional<std::string> rename_if_pb, std::optional<std::string> replay_append_if_pb);
};  // namespace AutoSubmitMod
