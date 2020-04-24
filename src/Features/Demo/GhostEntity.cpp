#include "Features/Demo/GhostEntity.hpp"
#include "Features/Demo/GhostPlayer.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"

#include "Features/Demo/Demo.hpp"
#include "Features/Demo/DemoParser.hpp"
#include "Features/Hud/Hud.hpp"
#include "Features/Session.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/EngineDemoPlayer.hpp"
#include "Modules/Server.hpp"

Variable sar_ghost_height("sar_ghost_height", "16", -256, "Height of the ghost.\n");
Variable sar_ghost_transparency("sar_ghost_transparency", "255", 0, 256, "Transparency of the ghost.\n");
Variable sar_ghost_name_offset("sar_ghost_text_offset", "20", -1024, "Offset of the name over the ghost.\n");
Variable sar_hud_ghost_show_crouched("sar_hud_ghost_show_crouched", "0", "Display the crouched state of the ghost\n");*/

GhostEntity::GhostEntity(int ghostType)
    : positionList()
    , angleList()
    , ID()
    , ghostType(ghostType)
    , name("demo")
    , currentMap()
    , sameMap(false)
    , ghost_entity(nullptr)
    , startTick()
    , CMTime(0)
    , modelName("models/props/food_can/food_can_open.mdl")
    , isPlaying(false)
    , hasFinished(false)
    , tickCount(0)
    , startDelay(0)
    , demo()
    , newPos({ { 1, 1, 1 }, { 1, 1, 1 } })
    , oldPos({ { 1, 1, 1 }, { 1, 1, 1 } })
{
}

void GhostEntity::Reset()
{
    this->ghost_entity = nullptr;
    this->isPlaying = false;
    this->tickCount = GetStartDelay();
    if (this->ghostType == 1) {
        engine->ClearAllOverlays();
    } else {
        this->KillGhost(false);
    }
}

void GhostEntity::Stop()
{
    this->Reset();
    delete this->ghost_entity;
}

void GhostEntity::KillGhost(bool newEntity)
{
    //Bad way but didn't find any function that simply delete the entity lol
    if (newEntity) {
        server->SetKeyValueChar(this->ghost_entity, "targetname", "ghost_destroy");
        engine->ExecuteCommand("ent_fire ghost_destroy kill");
    } else {
        engine->ExecuteCommand(std::string("ent_fire ghost_" + this->name + " kill").c_str());
    }
}

GhostEntity* GhostEntity::Spawn(bool instantPlay, Vector position, int ghostType)
{
    if (ghostType == 1) { //Ghost drawn with debug triangles

        this->ghost_entity = new GhostEntity(ghostPlayer->defaultGhostType);
        this->SetPosAng(position, Vector{ 0, 0, 0 });
        this->isPlaying = instantPlay;
        if (this->ghost_entity != nullptr) {
            this->lastUpdate = this->clock.now();
            return this;
        }
    } else if (ghostType == 2) { //Ghost drawn with in-game props
        this->ghost_entity = server->CreateEntityByName("prop_dynamic_override");
        server->SetKeyValueChar(this->ghost_entity, "model", this->modelName);
        std::string ghostName = "ghost_" + this->name;
        server->SetKeyValueChar(this->ghost_entity, "targetname", ghostName.c_str());

        this->SetPosAng(position, Vector{ 0, 0, 0 });

        if (sar_ghost_transparency.GetFloat() <= 254) {
            server->SetKeyValueChar(this->ghost_entity, "rendermode", "1");
            server->SetKeyValueFloat(this->ghost_entity, "renderamt", sar_ghost_transparency.GetFloat());
        } else {
            server->SetKeyValueChar(this->ghost_entity, "rendermode", "0");
        }

        server->DispatchSpawn(this->ghost_entity);
        this->isPlaying = instantPlay;

        if (this->ghost_entity != nullptr) {
            this->lastUpdate = this->clock.now();
            return this;
        }
    }

    return nullptr;
}

bool GhostEntity::IsReady()
{
    if (ghostPlayer->enabled && this->CMTime != 0) { //No need to check positionList anymore, cause CMTime is > 0 if PositionList > 0
        return true;
    }
    return false;
}

void GhostEntity::SetCMTime(float playbackTime)
{
    float time = (playbackTime)*60;
    this->CMTime = std::round(time);
}

void GhostEntity::Think()
{
    auto tick = session->GetTick();
    if (this->ghost_entity == nullptr && !this->hasFinished && ((engine->GetMaxClients() == 1 && tick >= (this->startTick + (this->CMTime - this->demo.playbackTicks))) || (engine->GetMaxClients() > 1 && tick >= this->startTick))) {
        auto pos = this->positionList[(this->tickCount)];
        this->Spawn(true, pos, this->ghostType);
    }

    if (this->isPlaying) {
        Vector position = this->positionList[(this->tickCount)];
        position.z += sar_ghost_height.GetFloat();
        this->SetPosAng(position, this->angleList[(this->tickCount)]);

        if (!engine->demoplayer->IsPlaying()) {
            if (engine->GetMaxClients() == 1) {
                if (tick % 2 == 0) {
                    ++this->tickCount;
                }
            } else {
                ++this->tickCount;
            }
        } else {
            if (engine->GetMaxClients() == 1) {
                this->tickCount = tick / 2 - (this->startTick + (this->CMTime - this->demo.playbackTicks));
                if (this->tickCount < 0) {
                    this->tickCount = 0;
                }
            } else {
                this->tickCount = tick - (this->startTick + (this->CMTime - this->demo.playbackTicks));
                if (this->tickCount < 0) {
                    this->tickCount = 0;
                }
            }
        }

        if (this->tickCount == this->positionList.size()) {
            console->Print("Ghost has finished.\n");
            this->hasFinished = true;
            this->Reset();
        }
    }
}

int GhostEntity::GetTickCount()
{
    return this->tickCount;
}

int GhostEntity::GetStartDelay()
{
    return this->startDelay;
}

void GhostEntity::SetStartDelay(int delay)
{
    this->startDelay = delay;
}

void GhostEntity::ChangeModel(std::string modelName)
{
    std::copy(modelName.begin(), modelName.end(), this->modelName);
    this->modelName[sizeof(this->modelName) - 1] = '\0';
}

void GhostEntity::ChangeType(int newType)
{
    if (this->ghostType == 1 && newType == 2) {
        this->ChangeModel(this->modelName);
        this->ghostType = newType;
        this->Spawn(true, this->currentPos, newType);
    } else if (this->ghostType == 2 && newType == 1) {
        this->KillGhost(false);
        this->ghostType = newType;
        this->Spawn(true, this->currentPos, newType);
    }
}

void GhostEntity::SetPosAng(const Vector& pos, const Vector& ang)
{
    if (this->ghostType == 1) {
        Vector p1 = pos;
        Vector p2 = pos;
        p2.x += 10;
        Vector p3 = pos;
        p3.x += 5;
        p3.z += 10;

        engine->AddTriangleOverlay(p1, p2, p3, 254, 0, 0, sar_ghost_transparency.GetInt(), false, 0);
        engine->AddTriangleOverlay(p3, p2, p1, 254, 0, 0, sar_ghost_transparency.GetInt(), false, 0);
    } else if (this->ghostType == 2) {
        if (this->ghost_entity != nullptr) {
            server->SetKeyValueVector(this->ghost_entity, "origin", pos);
            server->SetKeyValueVector(this->ghost_entity, "angles", ang);
        }
    }

    this->currentPos = pos;
}

void GhostEntity::Lerp(DataGhost& oldPosition, DataGhost& targetPosition, float time)
{
    if (time > 1) {
        return;
    }

    Vector newPos;
    newPos.x = (1 - time) * oldPosition.position.x + time * targetPosition.position.x;
    newPos.y = (1 - time) * oldPosition.position.y + time * targetPosition.position.y;
    newPos.z = (1 - time) * oldPosition.position.z + time * targetPosition.position.z;

    Vector newAngle;
    newAngle.x = (1 - time) * oldPosition.view_angle.x + time * targetPosition.view_angle.x;
    newAngle.y = (1 - time) * oldPosition.view_angle.y + time * targetPosition.view_angle.y;
    newAngle.z = 0;

    this->SetPosAng(newPos, newAngle);
}

CON_COMMAND(sar_ghost_type, "Type of the ghost :\n"
                            "1 = Ghost drawn manually. Aren't recorded in demos (but still can be drawn in them with SAR)\n"
                            "2 = Ghost using in-game model. WARNING : Those remain permanently in demos\n")
{
    if (args.ArgC() <= 1) {
        console->Print(sar_ghost_tickrate.ThisPtr()->m_pszHelpString);
        return;
    }

    int arg = std::atoi(args[1]);
    ghostPlayer->defaultGhostType = arg;
    networkGhostPlayer->defaultGhostType = arg;

    if (!networkGhostPlayer->ghostPool.empty()) {
        for (auto& ghost : networkGhostPlayer->ghostPool) {
            ghost->ChangeType(arg);
        }
    } else if ((!ghostPlayer->ghost.empty() && ghostPlayer->GetFirstGhost()->isPlaying) || (engine->demoplayer->IsPlaying() && !ghostPlayer->ghost.empty())) { //Demo ghost or Playing demo with ghost
        auto ghost = ghostPlayer->GetFirstGhost();
        ghost->ChangeType(arg);
    }
}

HUD_ELEMENT(ghost_show_name, "1", "Display the name of the ghost over it.\n", HudType_InGame)
{
    auto slot = GET_SLOT();
    auto player = client->GetPlayer(slot + 1);
    if (player) {
        if (!networkGhostPlayer->ghostPool.empty()) {
            auto pos = client->GetAbsOrigin(player);
            for (int i = 0; i < networkGhostPlayer->ghostPool.size(); ++i) {
                if (networkGhostPlayer->ghostPool[i]->sameMap) {
                    Vector screenPos;
                    engine->PointToScreen(networkGhostPlayer->ghostPool[i]->currentPos, screenPos);
                    ctx->DrawElementOnScreen(i, screenPos.x, screenPos.y - sar_ghost_name_offset.GetInt() - sar_ghost_height.GetInt(), networkGhostPlayer->ghostPool[i]->name.c_str());
                }
            }
        } else if (!ghostPlayer->ghost.empty() && ghostPlayer->GetFirstGhost()->isPlaying) { //Demo ghost
            auto pos = client->GetAbsOrigin(player);
            auto ghost = ghostPlayer->GetFirstGhost();
            Vector screenPos;
            engine->PointToScreen(ghost->currentPos, screenPos);
            ctx->DrawElementOnScreen(0, screenPos.x, screenPos.y - sar_ghost_name_offset.GetInt() - sar_ghost_height.GetInt(), ghost->name.c_str());
        } else if (engine->demoplayer->IsPlaying() && !ghostPlayer->ghost.empty()) { //Playing demo with ghost
            auto pos = client->GetAbsOrigin(player);
            auto ghost = ghostPlayer->GetFirstGhost();
            int tick = ghost->GetTickCount();
            if (tick < 0) {
                tick = 0;
            }
            Vector screenPos;
            engine->PointToScreen(ghost->positionList[tick], screenPos);
            ctx->DrawElementOnScreen(0, screenPos.x, screenPos.y - sar_ghost_name_offset.GetInt() - sar_ghost_height.GetInt(), ghost->name.c_str());
        }
    }
}

HUD_ELEMENT(ghost_show_distance, "0", "Display the distance from the ghost over it.\n", HudType_InGame | HudType_Paused)
{
    auto slot = GET_SLOT();
    auto player = client->GetPlayer(slot + 1);
    auto pos = client->GetAbsOrigin(player);
    if (player) {
        if (!networkGhostPlayer->ghostPool.empty()) {
            auto pos = client->GetAbsOrigin(player);
            for (int i = 0; i < networkGhostPlayer->ghostPool.size(); ++i) {
                if (networkGhostPlayer->ghostPool[i]->sameMap) {
                    Vector screenPos;
                    engine->PointToScreen(networkGhostPlayer->ghostPool[i]->currentPos, screenPos);
                    ctx->DrawElementOnScreen(i, screenPos.x, screenPos.y - sar_ghost_name_offset.GetInt() - sar_ghost_height.GetInt(), "Dist: %.3f", Math::Distance(pos, networkGhostPlayer->ghostPool[i]->currentPos));
                }
            }
        } else if (!ghostPlayer->ghost.empty() && ghostPlayer->GetFirstGhost()->isPlaying) { //Demo ghost
            auto ghost = ghostPlayer->GetFirstGhost();
            Vector screenPos;
            engine->PointToScreen(ghost->currentPos, screenPos);
            ctx->DrawElementOnScreen(0, screenPos.x, screenPos.y - sar_ghost_name_offset.GetInt() - sar_ghost_height.GetInt(), "Dist: %.3f", Math::Distance(pos, ghost->currentPos));
        } else if (engine->demoplayer->IsPlaying() && !ghostPlayer->ghost.empty()) { //Playing demo with ghost
            auto pos = client->GetAbsOrigin(player);
            auto ghost = ghostPlayer->GetFirstGhost();
            int tick = ghost->GetTickCount();
            if (tick < 0) {
                tick = 0;
            }
            Vector screenPos;
            engine->PointToScreen(ghost->positionList[tick], screenPos);
            ctx->DrawElementOnScreen(0, screenPos.x, screenPos.y - sar_ghost_name_offset.GetInt() - sar_ghost_height.GetInt(), "Dist: %.3f", Math::Distance(pos, ghost->currentPos));
        }
    }
}
