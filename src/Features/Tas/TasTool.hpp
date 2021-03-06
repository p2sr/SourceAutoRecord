#pragma once

#include "TasPlayer.hpp"
#include <list>

// A bunch of stuff accidentally used sin/cos directly, which led to
// inconsistent scripts between Windows and Linux since Windows has
// overloads for some functions. These helper macros dispatch to
// sinf/cosf on newer script versions for platform consistency
#define absOld(x) ((tasPlayer->scriptVersion >= 3) ? fabsf(x) : abs(x)) // we noticed this one sooner lol
#define sinOld(x) ((tasPlayer->scriptVersion >= 4) ? sinf(x) : sin(x))
#define cosOld(x) ((tasPlayer->scriptVersion >= 4) ? cosf(x) : cos(x))
#define atan2Old(x, y) ((tasPlayer->scriptVersion >= 4) ? atan2f(x, y) : atan2(x, y))
#define powOld(x, y) ((tasPlayer->scriptVersion >= 4) ? powf(x, y) : pow(x, y))

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
	static std::vector<std::string> priorityList;
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
