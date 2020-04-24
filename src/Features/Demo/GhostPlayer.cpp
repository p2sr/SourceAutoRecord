#include "Features/Demo/GhostPlayer.hpp"
#include "Command.hpp"
#include "Features/Demo/DemoParser.hpp"
#include "Features/Demo/NetworkGhostPlayer.hpp"
#include "Modules/Server.hpp"

GhostPlayer* ghostPlayer;

GhostPlayer::GhostPlayer()
    : ghost()
    , enabled(false)
    , isNetworking(false)
    , defaultGhostType(1)
{
    this->hasLoaded = true;
}

bool GhostPlayer::IsReady()
{
    for (auto it : this->ghost) {
        if (!it->IsReady()) {
            return false;
        }
    }
    if (!this->enabled || this->ghost.empty()) {
        return false;
    }

    return true;
}

void GhostPlayer::Run()
{
    for (auto it : this->ghost) {
        it->Think();
    }
}

void GhostPlayer::StopAll()
{
    for (auto it : this->ghost) {
        it->Stop();
    }
    this->ghost.clear();
}

void GhostPlayer::StopByID(unsigned int& ID)
{
    this->GetGhostFromID(ID)->Stop();
}

void GhostPlayer::ResetGhost()
{
    for (auto it : this->ghost) {
        it->Reset();
    }
}

void GhostPlayer::ResetCoord()
{
    for (auto it : this->ghost) {
        it->positionList.clear();
        it->angleList.clear();
    }
}

void GhostPlayer::SetPosAng(unsigned int& ID, Vector position, Vector angle)
{
    this->GetGhostFromID(ID)->SetPosAng(position, angle);
}

GhostEntity* GhostPlayer::GetFirstGhost()
{
    return this->ghost[0];
}

GhostEntity* GhostPlayer::GetGhostFromID(unsigned int& ID)
{
    for (auto it : this->ghost) {
        if (it->ID == ID) {
            return it;
        }
    }
    return nullptr;
}

void GhostPlayer::AddGhost(GhostEntity* ghost)
{
    this->ghost.push_back(ghost);
}

int GhostPlayer::GetStartTick()
{
    return this->GetFirstGhost()->startTick;
}

void GhostPlayer::SetStartTick(int startTick)
{
    this->GetFirstGhost()->startTick = startTick;
}

void GhostPlayer::SetCoordList(std::vector<Vector> pos, std::vector<Vector> ang)
{
    this->GetFirstGhost()->positionList = pos;
    this->GetFirstGhost()->angleList = ang;
}

bool GhostPlayer::IsNetworking()
{
    return this->isNetworking;
}

// Commands

CON_COMMAND_AUTOCOMPLETEFILE(sar_ghost_set_demo, "Set the demo in order to build the ghost.\n", 0, 0, dem)
{
    if (args.ArgC() <= 1) {
        if (ghostPlayer->enabled) {
            ghostPlayer->enabled = false;
            ghostPlayer->StopAll();
            return;
        } else {
            return console->Print(sar_ghost_set_demo.ThisPtr()->m_pszHelpString);
        }
    }
    if (networkGhostPlayer->IsConnected()) {
        return console->Warning("Can't play ghost with demos when connected to a server !\n");
    }

    std::string name;
    if (args[1][0] == '\0') {
        ghostPlayer->enabled = false;
        ghostPlayer->StopAll();
    } else {
        if (!ghostPlayer->enabled) {
            ghostPlayer->AddGhost(new GhostEntity(ghostPlayer->defaultGhostType));
            ghostPlayer->enabled = true;
        }
        name = std::string(args[1]);
    }

    DemoParser parser;
    parser.outputMode = 3;
    Demo demo;

    auto dir = std::string(engine->GetGameDirectory()) + std::string("/") + name;
    if (parser.Parse(dir, &demo)) {
        parser.Adjust(&demo);
        ghostPlayer->GetFirstGhost()->SetCMTime(demo.playbackTime);
        ghostPlayer->GetFirstGhost()->demo = demo;
        ghostPlayer->GetFirstGhost()->name = demo.clientName;
        console->Print("Ghost sucessfully created ! Final time of the ghost : %f\n", demo.playbackTime);
    } else {
        console->Print("Could not parse \"%s\"!\n", name.c_str());
    }
}

CON_COMMAND_COMPLETION(sar_ghost_set_prop_model, "Set the prop model. Example : models/props/metal_box.mdl\n",
    ({ "models/props/metal_box.mdl", "models/player/chell/player.mdl", "models/player/eggbot/eggbot.mdl", "models/player/ballbot/ballbot.mdl", "models/props/radio_reference.mdl",
        "models/props/food_can/food_can_open.mdl", "models/npcs/turret/turret.mdl", "models/npcs/bird/bird.mdl" }))
{
    if (args.ArgC() <= 1) {
        return console->Print(sar_ghost_set_prop_model.ThisPtr()->m_pszHelpString);
    }

    networkGhostPlayer->modelName = args[1];

    if (ghostPlayer->ghost.empty()) {
        return;
    }

    auto ghost = ghostPlayer->GetFirstGhost();
    ghost->ChangeModel(args[1]);
    if (ghost->ghostType == 2) {
        ghost->KillGhost(true);
        ghost->Spawn(true, ghost->currentPos, ghost->ghostType);
    }
}

CON_COMMAND(sar_ghost_time_offset, "In seconds. Start the ghost with a delay. Can be negative or positive.\n")
{
    if (args.ArgC() <= 1) {
        return console->Print(sar_ghost_time_offset.ThisPtr()->m_pszHelpString);
    }
    if (ghostPlayer->ghost.empty()) {
        return console->Print("sar_ghost_enable must be enabled before setting the demo.\n");
    }

    float delay = static_cast<float>(std::atof(args[1]));
    GhostEntity* ghost = ghostPlayer->GetFirstGhost();

    if (delay < 0) { //Asking for faster ghost
        if (delay * 60 > ghost->positionList.size()) { //Too fast
            console->Print("Time offset is too low.\n");
            return;
        } else { //Ok
            ghost->SetStartDelay(-delay * 60); //Seconds -> ticks ; TODO : Check if this works properly in Coop
            console->Print("Final time of the ghost : %f\n", ghost->demo.playbackTime + delay);
        }
    } else if (delay > 0) { //Asking for slower ghost
        ghost->SetStartDelay(0);
        ghost->SetCMTime(ghost->demo.playbackTime + delay);
        console->Print("Final time of the ghost : %f\n", ghost->demo.playbackTime + delay);
    } else if (delay == 0) {
        ghost->SetStartDelay(0);
        ghost->SetCMTime(ghost->demo.playbackTime);
        console->Print("Final time of the ghost : %f\n", ghost->demo.playbackTime);
    }
}
