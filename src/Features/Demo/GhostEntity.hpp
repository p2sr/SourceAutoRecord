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
    sf::Uint32 ID;
    std::string name;
    DataGhost data;
    std::string currentMap;
    bool sameMap;

    std::string modelName;
    void* entity;

    DataGhost oldPos;
    DataGhost newPos;
    std::chrono::time_point<std::chrono::steady_clock> lastUpdate;
    long long loopTime;

    static int ghost_type;

public:
    GhostEntity(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map);
    ~GhostEntity();

    void Spawn();
    void DeleteGhost();
    void SetData(Vector pos, QAngle ang);
    void SetupGhost(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map);
    void Display();
    void Lerp(float time);
    void DeleteGhostModel(const bool newEntity);
};

extern Variable ghost_height;
extern Variable ghost_transparency;
extern Variable ghost_text_offset;
extern Variable ghost_show_advancement;
extern Command ghost_prop_model;
extern Command ghost_type;
