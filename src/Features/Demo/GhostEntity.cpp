#include "Features/Demo/GhostEntity.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Features/Demo/DemoGhostPlayer.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Features/Hud/Hud.hpp"

#ifdef _WIN32
#define PLAT_CALL(fn, ...) fn(__VA_ARGS__)
#else
#define PLAT_CALL(fn, ...) fn(nullptr, __VA_ARGS__)
#endif

int GhostEntity::ghost_type = 0;

Variable ghost_height("ghost_height", "16", -256, "Height of the ghosts.\n");
Variable ghost_transparency("ghost_transparency", "255", 0, 256, "Transparency of the ghosts.\n");
Variable ghost_text_offset("ghost_text_offset", "20", -1024, "Offset of the name over the ghosts.\n");
Variable ghost_show_advancement("ghost_show_advancement", "1", "Show the advancement of the ghosts.\n");

GhostEntity::GhostEntity(unsigned int& ID, std::string& name, DataGhost& data, std::string& current_map)
    : ID(ID)
    , name(name)
    , data(data)
    , currentMap(current_map)
    , modelName("models/props/food_can/food_can_open.mdl")
    , prop_entity(nullptr)
    , isDestroyed(false)
{
}

GhostEntity::~GhostEntity()
{
    this->DeleteGhost();
}

void GhostEntity::Spawn()
{
    if (!GhostEntity::ghost_type) {
        return;
    }

    this->prop_entity = PLAT_CALL(server->CreateEntityByName, "prop_dynamic_override");
    if (this->prop_entity == nullptr) {
        console->Warning("CreateEntityByName() failed !\n");
        return;
    }

    PLAT_CALL(server->SetKeyValueChar, this->prop_entity, "model", this->modelName.c_str());
    std::string ghostName = "ghost_" + this->name;
    PLAT_CALL(server->SetKeyValueChar, this->prop_entity, "targetname", ghostName.c_str());

    if (ghost_transparency.GetInt() < 255) {
        PLAT_CALL(server->SetKeyValueChar, this->prop_entity, "rendermode", "1");
        PLAT_CALL(server->SetKeyValueFloat, this->prop_entity, "renderamt", ghost_transparency.GetFloat());
    } else {
        PLAT_CALL(server->SetKeyValueChar, this->prop_entity, "rendermode", "0");
    }

    PLAT_CALL(server->DispatchSpawn, this->prop_entity);
}

void GhostEntity::DeleteGhost()
{
    if (this->prop_entity != nullptr) {
        PLAT_CALL(server->SetKeyValueChar, this->prop_entity, "targetname", "_ghost_destroy");
        engine->ExecuteCommand("ent_fire _ghost_destroy kill");
        this->prop_entity = nullptr;
    }
}

void GhostEntity::SetData(Vector pos, QAngle ang, bool network)
{
    this->oldPos = this->newPos;
    this->newPos = { pos, ang };

    auto now = NOW_STEADY();
    long long newLoopTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->lastUpdate).count();
    if (network) {
        // Loop time could do strange things due to network latency etc.
        // Try to smooth it using a biased average of the new time with
        // the old one
        if (this->loopTime = 0) {
            this->loopTime = newLoopTime;
        } else {
            this->loopTime = (2 * this->loopTime + 1 * newLoopTime) / 3;
        }
    } else {
        this->loopTime = newLoopTime;
    }
    this->lastUpdate = now;
}

void GhostEntity::SetupGhost(unsigned int& ID, std::string& name, DataGhost& data, std::string& current_map)
{
    this->ID = ID;
    this->name = name;
    this->data = data;
    this->currentMap = current_map;
}

void GhostEntity::Display()
{
    if (!GhostEntity::ghost_type) {
        this->data.position.z += ghost_height.GetInt();

        this->p1 = this->data.position;
        this->p2 = this->data.position;
        this->p3 = this->data.position;

        p2.x += 10;
        p3.x += 5;
        p3.z += 10;

        PLAT_CALL(engine->AddTriangleOverlay, p1, p2, p3, 255, 0, 0, 255, false, 0);
        PLAT_CALL(engine->AddTriangleOverlay, p3, p2, p1, 255, 0, 0, 255, false, 0);
    } else {
        if (this->prop_entity != nullptr) {
            this->data.position.z += ghost_height.GetInt();
            PLAT_CALL(server->SetKeyValueVector, this->prop_entity, "origin", this->data.position);
            PLAT_CALL(server->SetKeyValueVector, this->prop_entity, "angles", Vector{ this->data.view_angle.x, this->data.view_angle.y, this->data.view_angle.z });
        }
    }
}

void GhostEntity::Lerp(float time)
{
    if (time > 1) time = 1;
    if (time < 0) time = 0;

    this->data.position.x = (1 - time) * this->oldPos.position.x + time * this->newPos.position.x;
    this->data.position.y = (1 - time) * this->oldPos.position.y + time * this->newPos.position.y;
    this->data.position.z = (1 - time) * this->oldPos.position.z + time * this->newPos.position.z;

    this->data.view_angle.x = (1 - time) * this->oldPos.view_angle.x + time * this->newPos.view_angle.x;
    this->data.view_angle.y = (1 - time) * this->oldPos.view_angle.y + time * this->newPos.view_angle.y;
    this->data.view_angle.z = 0;

    this->Display();
}

HUD_ELEMENT2(ghost_show_name, "1", "Display the name of the ghost over it.\n", HudType_InGame)
{
    if (networkManager.isConnected)
        networkManager.DrawNames(ctx);

    if (demoGhostPlayer.IsPlaying())
        demoGhostPlayer.DrawNames(ctx);
}

CON_COMMAND_COMPLETION(ghost_prop_model, "Set the prop model. Example : models/props/metal_box.mdl\n",
    ({ "models/props/metal_box.mdl", "models/player/chell/player.mdl", "models/player/eggbot/eggbot.mdl", "models/player/ballbot/ballbot.mdl", "models/props/radio_reference.mdl",
        "models/props/food_can/food_can_open.mdl", "models/npcs/turret/turret.mdl", "models/npcs/bird/bird.mdl" }))
{
    if (args.ArgC() <= 1) {
        return console->Print(ghost_prop_model.ThisPtr()->m_pszHelpString);
    }

    networkManager.UpdateModel(args[1]);

    demoGhostPlayer.UpdateGhostsModel(args[1]);
}

CON_COMMAND(ghost_type, "ghost_type <0/1>:\n"
                        "0: Ghost not recorded in demos.\n"
                        "1: Ghost using props model but recorded in demos (NOT RECOMMANDED !).\n")
{
    if (args.ArgC() < 2) {
        return console->Print(ghost_type.ThisPtr()->m_pszHelpString);
    }

    if (GhostEntity::ghost_type == 0 && std::atoi(args[1]) == 1) {
        GhostEntity::ghost_type = 1;
        demoGhostPlayer.SpawnAllGhosts();
        if (networkManager.isConnected) {
            networkManager.SpawnAllGhosts();
        }
    } else if (GhostEntity::ghost_type == 1 && std::atoi(args[1]) == 0) {
        GhostEntity::ghost_type = 0;
        demoGhostPlayer.DeleteAllGhosts();
        if (networkManager.isConnected) {
            networkManager.DeleteAllGhosts();
        }
    }
}
