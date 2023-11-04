#pragma once
#include "../TasTool.hpp"

struct DuckToolParams : public TasToolParams {
	DuckToolParams()
		: TasToolParams() {
	}
	DuckToolParams(bool enabled, int time)
		: TasToolParams(enabled)
		, time(time) {
	}

	int time = 0;
};

class DuckTool : public TasToolWithParams<DuckToolParams> {
public:
	DuckTool(int slot)
		: TasToolWithParams("duck", slot) {
	}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &bulk, const TasPlayerInfo &pInfo);

private:
	int elapsedTicks;
};

extern DuckTool duckTool[2];
