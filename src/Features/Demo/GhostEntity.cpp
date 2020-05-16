#include "Features/Demo/GhostEntity.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

GhostEntity::GhostEntity(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map)
{
    this->ID = ID;
    this->name = name;
    this->data = data;
    this->currentMap = current_map;
    this->update = 0;
}

void GhostEntity::Spawn()
{
}

void GhostEntity::DeleteGhost()
{
}

void GhostEntity::SetData(Vector pos, QAngle ang)
{
    this->data.position = pos;
    this->data.view_angle = ang;
}

void GhostEntity::Update()
{
    Vector p1 = this->data.position;
    Vector p2 = this->data.position;
    Vector p3 = this->data.position;

    p2.x += 10;
    p3.x += 5;
    p3.z += 10;

    engine->AddTriangleOverlay(p1, p2, p3, 255, 0, 0, 255, false, server->gpGlobals->frametime);
    engine->AddTriangleOverlay(p3, p2, p1, 255, 0, 0, 255, false, server->gpGlobals->frametime);
}

void GhostEntity::SetupGhost(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map)
{
    this->ID = ID;
    this->name = name;
    this->data = data;
    this->currentMap = current_map;
}