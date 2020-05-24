#include "Features/Demo/GhostEntity.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Features/Hud/Hud.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"

Variable ghost_type("ghost_type", "0", "lol");
Variable ghost_height("ghost_height", "16", -256, "Height of the ghost.\n");
Variable ghost_transparency("ghost_transparency", "255", 0, 256, "Transparency of the ghost.\n");
Variable ghost_text_offset("ghost_text_offset", "20", -1024, "Offset of the name over the ghost.\n");

GhostEntity::GhostEntity(sf::Uint32& ID, std::string& name, DataGhost& data, std::string& current_map)
    : ID(ID)
    , name(name)
    , data(data)
    , currentMap(current_map)
    , modelName("models/props/food_can/food_can_open.mdl")
{
}

GhostEntity::~GhostEntity()
{
    this->DeleteGhost();
}

void GhostEntity::Spawn()
{
    if (!ghost_type.GetBool()) {
        return;
    }

    this->entity = server->CreateEntityByName("prop_dynamic_override");
    if (this->entity == nullptr) {
        console->Warning("CreateEntityByName() failed !\n");
        return;
    }

    server->SetKeyValueChar(this->entity, "model", this->modelName.c_str());
    std::string ghostName = "ghost_" + this->name;
    server->SetKeyValueChar(this->entity, "targetname", ghostName.c_str());

    if (ghost_transparency.GetInt() < 255) {
        server->SetKeyValueChar(this->entity, "rendermode", "1");
        server->SetKeyValueFloat(this->entity, "renderamt", ghost_transparency.GetFloat());
    } else {
        server->SetKeyValueChar(this->entity, "rendermode", "0");
    }

    server->DispatchSpawn(this->entity);
}

void GhostEntity::DeleteGhost()
{
    delete (this->entity);
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
    if (!ghost_type.GetBool()) {
        Vector p1 = this->data.position;
        Vector p2 = this->data.position;
        Vector p3 = this->data.position;

        p2.x += 10;
        p3.x += 5;
        p3.z += 10;

        engine->AddTriangleOverlay(p1, p2, p3, 255, 0, 0, 255, false, 0);
        engine->AddTriangleOverlay(p3, p2, p1, 255, 0, 0, 255, false, 0);
    } else {
        if (this->entity != nullptr) {
            server->SetKeyValueVector(this->entity, "origin", this->data.position);
            server->SetKeyValueVector(this->entity, "angles", Vector{ this->data.view_angle.x, this->data.view_angle.y, this->data.view_angle.z });
        }
    }
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

HUD_ELEMENT(ghost_show_name, "1", "Display the name of the ghost over it.\n", HudType_InGame)
{
    networkManager.DrawNames(ctx);
}
