#pragma once
#include "SFML/Network.hpp"
#include "Utils/SDK.hpp"

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

public:
    GhostEntity(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map);

    void Spawn();
    void DeleteGhost();
    void Update();
    void SetData(Vector pos, QAngle ang);
    void SetupGhost(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map);
    float update;
};
