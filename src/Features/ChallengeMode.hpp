#pragma once
#include "Features/Feature.hpp"
#include "Game.hpp"
#include "Utils/SDK.hpp"

#include <string>
#include <vector>

class ChallengeMode : public Feature {
public:
	static std::vector<ChallengeNodeData_t> customNodes;

public:
	void CreateNode();
	void LoadNodes(SourceGameVersion version);
};

extern ChallengeMode *cm;
