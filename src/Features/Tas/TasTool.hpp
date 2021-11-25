#pragma once

#include "TasPlayer.hpp"
#include <list>

struct TasToolParams {
	bool enabled = false;
	TasToolParams() {}
	TasToolParams(bool enabled)
		: enabled(enabled) {}
};

struct TasFramebulk;
struct TasPlayerInfo;


class TasTool {
protected:
	const char *name;
	std::shared_ptr<TasToolParams> params = nullptr;
	bool updated = false;
	int slot;

public:
	TasTool(const char *name, int slot);
	~TasTool();

	const char *GetName();

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>) = 0;
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo) = 0;
	virtual void Reset();

	void SetParams(std::shared_ptr<TasToolParams> params);
	std::shared_ptr<TasToolParams> GetCurrentParams();

public:
	static std::list<TasTool *> &GetList(int slot);
};


class TasToolCommand {
public:
	TasTool *tool;
	std::shared_ptr<TasToolParams> params;

	TasToolCommand(TasTool *tool, std::shared_ptr<TasToolParams> params)
		: tool(tool)
		, params(params) {}

	~TasToolCommand() {}
};
