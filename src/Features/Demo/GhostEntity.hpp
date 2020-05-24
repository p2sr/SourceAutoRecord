#pragma once
#include "SFML/Network.hpp"

#include "Utils/SDK.hpp"
#include "Variable.hpp"

#include <chrono>


struct DataGhost {
    Vector position;
    QAngle view_angle;
};

class GhostEntity {

private:
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

public:

    GhostEntity(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map);
    ~GhostEntity();

    void Spawn();
    void DeleteGhost();
    void SetData(Vector pos, QAngle ang);
    void SetupGhost(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map);
    void Update();
    void Lerp(float time);
};

extern Variable ghost_type;
extern Variable ghost_height;
extern Variable ghost_transparency;
extern Variable ghost_text_offset;