#pragma once
#include "Features/Tas/TasPlayer.hpp"
#include "Features/Tas/TasTool.hpp"

#define DEFAULT_POS_EPSILON 0.5
#define DEFAULT_ANG_EPSILON 0.2

struct CheckToolParams : public TasToolParams {
	CheckToolParams()
		: TasToolParams()
		, posepsilon(DEFAULT_POS_EPSILON)
		, angepsilon(DEFAULT_ANG_EPSILON)
	{}

	CheckToolParams(std::optional<Vector> pos, std::optional<QAngle> ang, float posepsilon, float angepsilon)
		: TasToolParams(true), pos(pos), ang(ang), posepsilon(posepsilon), angepsilon(angepsilon)
	{}

	std::optional<Vector> pos;
	std::optional<QAngle> ang; // roll ignored
	float posepsilon;
	float angepsilon;
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
