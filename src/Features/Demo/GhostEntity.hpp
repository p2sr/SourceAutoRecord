#pragma once
#include "Command.hpp"
#include "Features/Hud/Hud.hpp"
#include "SFML/Network.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"
#include <Features/Demo/GhostRenderer.hpp>

#include <chrono>
#include <optional>

#define GHOST_TOAST_TAG "ghost"

struct DataGhost {
	Vector position;
	QAngle view_angle;
	float view_offset;
	bool grounded;
};

enum class GhostType {
	CIRCLE = 0,
	PYRAMID = 1,
	PYRAMID_PGUN = 2,
	MODEL = 3,
	BENDY = 4
};

class GhostEntity {
public:
	unsigned int ID;
	bool network;
	std::string name;
	DataGhost data;
	std::string currentMap;
	bool sameMap;
	bool isAhead;
	float lastOpacity;
	std::optional<Color> color;

	std::string modelName;
	void *prop_entity;

	GhostRenderer renderer;

	DataGhost oldPos;
	DataGhost newPos;
	float lastUpdate;
	float loopTime = 0.0f;
	Vector velocity;

	static GhostType ghost_type;
	static std::string defaultModelName;
	static Color set_color;
	bool spectator;
	bool isDestroyed;  // used by NetworkGhostPlayer for sync reasons

	static void KillAllGhosts();

public:
	GhostEntity(unsigned int &ID, std::string &name, DataGhost &data, std::string &current_map, bool network);
	~GhostEntity();

	void Spawn();
	void DeleteGhost();
	void SetData(DataGhost data, bool network = false);
	void SetupGhost(unsigned int &ID, std::string &name, DataGhost &data, std::string &current_map);
	void Display();
	void Lerp();
	Color GetColor();
	void DrawName();

	static bool followNetwork;
	static int followId;
	static GhostEntity *GetFollowTarget();
	static void FollowPov(CViewSetup *view);
	static void StopFollowing();
	static void StartFollowing(GhostEntity *ghost);
	bool IsBeingFollowed();
};

extern Variable ghost_height;
extern Variable ghost_opacity;
extern Variable ghost_text_offset;
extern Variable ghost_show_advancement;
extern Command ghost_prop_model;
extern Command ghost_type;
