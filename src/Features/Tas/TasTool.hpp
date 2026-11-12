#pragma once

#include <list>
#include <memory>
#include <string>
#include <vector>

// glibc's sinf etc. sometimes return a slightly different value than
// MSVC's, which computes them in double. Since version 10 the tools use
// Math::Sin etc. (float in/out, computed in double) on every platform.
// Older scripts keep the platform's own functions
#define tasSinf(x) (TAS_SCRIPT_VERSION_AT_LEAST(10) ? Math::Sin(x) : sinf(x))
#define tasCosf(x) (TAS_SCRIPT_VERSION_AT_LEAST(10) ? Math::Cos(x) : cosf(x))
#define tasAtan2f(x, y) (TAS_SCRIPT_VERSION_AT_LEAST(10) ? Math::Atan2(x, y) : atan2f(x, y))
#define tasAcosf(x) (TAS_SCRIPT_VERSION_AT_LEAST(10) ? Math::Acos(x) : acosf(x))
#define tasAsinf(x) (TAS_SCRIPT_VERSION_AT_LEAST(10) ? Math::Asin(x) : asinf(x))
#define tasPowf(x, y) (TAS_SCRIPT_VERSION_AT_LEAST(10) ? Math::Pow(x, y) : powf(x, y))

// A bunch of stuff accidentally used sin/cos directly, which led to
// inconsistent scripts between Windows and Linux since Windows has
// overloads for some functions. These helper macros dispatch to
// sinf/cosf on newer script versions for platform consistency
#define absOld(x) (TAS_SCRIPT_VERSION_AT_LEAST(3) ? fabsf(x) : abs(x))  // we noticed this one sooner lol
#define sinOld(x) (TAS_SCRIPT_VERSION_AT_LEAST(4) ? tasSinf(x) : sin(x))
#define cosOld(x) (TAS_SCRIPT_VERSION_AT_LEAST(4) ? tasCosf(x) : cos(x))
#define atan2Old(x, y) (TAS_SCRIPT_VERSION_AT_LEAST(4) ? tasAtan2f(x, y) : atan2(x, y))
#define powOld(x, y) (TAS_SCRIPT_VERSION_AT_LEAST(4) ? tasPowf(x, y) : pow(x, y))

struct TasToolParams {
	bool enabled = false;
	TasToolParams() {}
	TasToolParams(bool enabled)
		: enabled(enabled) {}
};

struct TasFramebulk;
struct TasPlayerInfo;

enum TasToolProcessingType {
	PRE_PROCESSING,
	POST_PROCESSING,
};

enum TasToolBulkType {
	NONE = 0,
	MOVEMENT = 1 << 1,
	VIEWANGLES = 1 << 2,
	BUTTONS = 1 << 3,
	COMMANDS = 1 << 4,
	META = 1 << 5,

	ALL_TYPES_MASK = MOVEMENT | VIEWANGLES | BUTTONS | COMMANDS | META,
};

class TasTool {
protected:
	const char *name;
	TasToolProcessingType processingType;
	TasToolBulkType bulkType;
	std::shared_ptr<TasToolParams> paramsPtr = nullptr;
	bool updated = false;
	int slot;

public:
	TasTool(const char *name, TasToolProcessingType processingType, TasToolBulkType bulkType, int slot);
	~TasTool();

	const char *GetName() const { return name; }
	inline int GetSlot() const { return slot; }
	bool CanProcess(TasToolProcessingType type) const { return this->processingType == type; }
	TasToolBulkType GetBulkType() const { return this->bulkType; }

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
	TasToolWithParams(const char *name, TasToolProcessingType processingType, TasToolBulkType bulkType, int slot)
		: TasTool(name, processingType, bulkType, slot) {};

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
