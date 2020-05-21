#include "Features/Demo/GhostEntity.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

GhostEntity::GhostEntity(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map)
{
    this->ID = ID;
    this->name = name;
    this->data = data;
    this->currentMap = current_map;
}

void GhostEntity::Spawn()
{
}

void GhostEntity::DeleteGhost()
{
}

void GhostEntity::SetData(Vector pos, QAngle ang)
{
    this->oldPos = this->newPos;
    this->newPos = { pos, ang };

    auto now = NOW();
    this->loopTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->lastUpdate).count();
    this->lastUpdate = now;
}

void GhostEntity::SetupGhost(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map)
{
    this->ID = ID;
    this->name = name;
    this->data = data;
    this->currentMap = current_map;
}

void GhostEntity::Update()
{
    Vector p1 = this->data.position;
    Vector p2 = this->data.position;
    Vector p3 = this->data.position;

    p2.x += 10;
    p3.x += 5;
    p3.z += 10;

    engine->AddTriangleOverlay(p1, p2, p3, 255, 0, 0, 255, false, 0);
    engine->AddTriangleOverlay(p3, p2, p1, 255, 0, 0, 255, false, 0);
}

void GhostEntity::Lerp(float time)
{
    this->data.position.x = (1 - time) * this->oldPos.position.x + time * this->newPos.position.x;
    this->data.position.y = (1 - time) * this->oldPos.position.y + time * this->newPos.position.y;
    this->data.position.z = (1 - time) * this->oldPos.position.z + time * this->newPos.position.z;

    this->data.view_angle.z = (1 - time) * this->oldPos.view_angle.z + time * this->newPos.view_angle.z;
    this->data.view_angle.z = (1 - time) * this->oldPos.view_angle.z + time * this->newPos.view_angle.z;
    this->data.view_angle.z = 0;

    this->Update();
}
