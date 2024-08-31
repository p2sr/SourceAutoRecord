#pragma once

#include <list>
#include <memory>
#include <vector>
#include <string>

// A bunch of stuff accidentally used sin/cos directly, which led to
// inconsistent scripts between Windows and Linux since Windows has
// overloads for some functions. These helper macros dispatch to
// sinf/cosf on newer script versions for platform consistency
#define absOld(x) (TAS_SCRIPT_VERSION_AT_LEAST(3) ? fabsf(x) : abs(x))  // we noticed this one sooner lol
#define sinOld(x) (TAS_SCRIPT_VERSION_AT_LEAST(4) ? sinf(x) : sin(x))
#define cosOld(x) (TAS_SCRIPT_VERSION_AT_LEAST(4) ? cosf(x) : cos(x))
#define atan2Old(x, y) (TAS_SCRIPT_VERSION_AT_LEAST(4) ? atan2f(x, y) : atan2(x, y))
#define powOld(x, y) (TAS_SCRIPT_VERSION_AT_LEAST(4) ? powf(x, y) : pow(x, y))

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
	std::shared_ptr<TasToolParams> paramsPtr = nullptr;
	bool updated = false;
	int slot;
public:
	TasTool(const char *name, int slot);
	~TasTool();

	const char *GetName();

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>) = 0;
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &pInfo) = 0;
	virtual void Reset() = 0;
	virtual void SetParams(std::shared_ptr<TasToolParams> params);
public:
	static std::list<TasTool *> &GetList(int slot);
	static TasTool *GetInstanceByName(int slot, std::string name);
	static std::vector<std::string> priorityList;
};


template <typename Params>
class TasToolWithParams : public TasTool {
protected:
	Params params;
public:
	TasToolWithParams(const char *name, int slot)
		: TasTool(name, slot){};

	virtual void Reset() {
		this->paramsPtr = std::make_shared<Params>();
		this->params = *std::static_pointer_cast<Params>(this->paramsPtr);
	}
	virtual void SetParams(std::shared_ptr<TasToolParams> params) {
		TasTool::SetParams(params);
		this->params = *std::static_pointer_cast<Params>(params);
	}

	inline Params GetCurrentParams() const { return params; }
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
