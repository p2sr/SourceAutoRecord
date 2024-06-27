#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

#define DEFAULT_POS_EPSILON 0.5
#define DEFAULT_ANG_EPSILON 0.2
#define DEFAULT_VEL_EPSILON 1.0

struct CheckToolParams : public TasToolParams {
	CheckToolParams()
		: TasToolParams()
		, posepsilon(DEFAULT_POS_EPSILON)
		, angepsilon(DEFAULT_ANG_EPSILON)
		, velepsilon(DEFAULT_VEL_EPSILON)
	{}

	CheckToolParams(std::optional<Vector> pos, std::optional<QAngle> ang, std::optional<float> vel, std::optional<std::string> veldir, std::optional<std::string> holding, float posepsilon, float angepsilon, float velepsilon)
		: TasToolParams(true), pos(pos), ang(ang), vel(vel), veldir(veldir), holding(holding), posepsilon(posepsilon), angepsilon(angepsilon), velepsilon(velepsilon)
	{}

	std::optional<Vector> pos;
	std::optional<QAngle> ang; // roll ignored
	std::optional<float> vel;
	std::optional<std::string> veldir;
	std::optional<std::string> holding;
	float posepsilon;
	float angepsilon;
	float velepsilon;
};

class CheckTool : public TasToolWithParams<CheckToolParams> {
public:
	CheckTool(int slot)
		: TasToolWithParams("check", slot)
	{}

	virtual std::shared_ptr<TasToolParams> ParseParams(std::vector<std::string>);
	virtual void Apply(TasFramebulk &fb, const TasPlayerInfo &info);
};

extern CheckTool tasCheckTool[2];
extern int g_tas_check_replays;
