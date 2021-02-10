#pragma once
#include "SFML/Network.hpp"

#include "Command.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

#include <chrono>

struct DataGhost {
    Vector position;
    QAngle view_angle;
};

class GhostEntity {

private:
    Vector p1;
    Vector p2;
    Vector p3;

public:
    unsigned int ID;
    std::string name;
    DataGhost data;
    std::string currentMap;
    bool sameMap;
    bool isAhead;
    float lastTransparency;

    std::string modelName;
    void* prop_entity;

    DataGhost oldPos;
    DataGhost newPos;
    std::chrono::time_point<std::chrono::steady_clock> lastUpdate;
    long long loopTime;

    static int ghost_type;

    bool isDestroyed; // used by NetworkGhostPlayer for sync reasons

public:
    GhostEntity(unsigned int& ID, std::string& name, DataGhost& data, std::string& current_map);
    ~GhostEntity();

    void Spawn();
    void DeleteGhost();
    void SetData(Vector pos, QAngle ang, bool network = false);
    void SetupGhost(unsigned int& ID, std::string& name, DataGhost& data, std::string& current_map);
    void Display();
    void Lerp(float time);
};

extern Variable ghost_height;
extern Variable ghost_transparency;
extern Variable ghost_text_offset;
extern Variable ghost_show_advancement;
extern Command ghost_prop_model;
extern Command ghost_type;
