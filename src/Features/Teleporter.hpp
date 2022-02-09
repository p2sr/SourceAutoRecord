#pragma once
#include "Feature.hpp"
#include "Utils/SDK.hpp"

#include <vector>
#include <optional>

struct TeleportLocation {
	bool isSet = false;
	Vector origin;
	QAngle angles;
	Vector velocity;
	struct {
		bool isSet = false;
		unsigned char linkage;
		Vector pos;
		QAngle ang;
	} portals[2];
};

class Teleporter : public Feature {
private:
	std::vector<TeleportLocation> locations;

public:
	Teleporter();

	TeleportLocation &GetLocation(int slot);

	void SaveLocal(int slot, QAngle ang);
	void TeleportLocal(int slot, bool portals);

	void Save(int slot);
	void Teleport(int slot, bool portals);
};

extern Teleporter *teleporter;
