#include "DemoGhostPlayer.hpp"

#include "Modules/Engine.hpp"

#include "Features/Demo/Demo.hpp"
#include "Features/Demo/DemoParser.hpp"

#include <fstream>

DemoGhostPlayer demoGhostPlayer;

DemoGhostPlayer::DemoGhostPlayer()
{
}

void DemoGhostPlayer::SpawnAllGhosts()
{
    for (auto& ghost : this->ghostPool) {
        ghost.tickCount = 0;
        ghost.Spawn();
    }

    this->isPlaying = true;

    console->Print("Ghost : %d\n", this->isPlaying);
}

void DemoGhostPlayer::PauseAllGhosts()
{
    this->isPlaying = false;
}

void DemoGhostPlayer::DeleteAllGhosts()
{
    for (auto& ghost : this->ghostPool) {
        ghost.DeleteGhost();
    }
}

void DemoGhostPlayer::UpdateGhostsPosition()
{
    for (auto& ghost : this->ghostPool) {
        /*auto time = std::chrono::duration_cast<std::chrono::milliseconds>(NOW() - ghost.lastUpdate).count();
        ghost.Lerp(((float)time / (ghost.loopTime)));*/

        console->Print("%d < %d\n", ghost.tickCount, ghost.nbDemoTicks);

        if (ghost.tickCount < ghost.nbDemoTicks) {
            auto data = ghost.datas[ghost.tickCount++];
            ghost.SetData(data.position, data.view_angle);
            ghost.Update();
        }
    }
}

DemoGhostEntity* DemoGhostPlayer::GetGhostByID(int ID)
{
    for (auto& ghost : this->ghostPool) {
        if (ghost.ID == ID) {
            return &ghost;
        }
    }

    return nullptr;
}

void DemoGhostPlayer::SetDemoTime(float time)
{
}

void DemoGhostPlayer::AddGhost(DemoGhostEntity& ghost)
{
    this->ghostPool.push_back(ghost);
}

bool DemoGhostPlayer::IsPlaying()
{
    return this->isPlaying;
}

CON_COMMAND_AUTOCOMPLETEFILE(ghost_set_demo, "ghost_set_demo <demo> [ID]. Ghost will use this demo. If ID is specified, will create or modify the ID-ème ghost.\n", 0, 0, dem)
{
    if (args.ArgC() < 2) {
        return console->Print(ghost_set_demo.ThisPtr()->m_pszHelpString);
    }

    sf::Uint32 ID = args.ArgC() > 2 ? std::atoi(args[2]) : 0;
    /*if (demoGhostPlayer.GetGhostByID(ID) == nullptr) {
        GhostEntity ghost(ID, ;
        if (ghost == nullptr) {
            ghost->ID = ID;
            ghost.
        }
    }*/

    DemoParser parser;
    Demo demo;
    std::vector<DataGhost> datas;

    auto dir = engine->GetGameDirectory() + std::string("/") + args[1];
    if (parser.Parse(dir, &demo, true, &datas)) {
        parser.Adjust(&demo);

        std::string name = demo.clientName;
        std::string current_map = demo.mapName;

        if (demoGhostPlayer.GetGhostByID(ID) == nullptr) {
            DemoGhostEntity ghost = { ID, name, DataGhost{ { 0, 0, 0 }, { 0, 0, 0 } }, current_map, datas, datas.size()};
            demoGhostPlayer.AddGhost(ghost);
        } else {
            auto ghost = demoGhostPlayer.GetGhostByID(ID);
            ghost->name = name;
            ghost->currentMap = current_map;
            ghost->SetCoordList(datas);
        }

        return console->Print("Ghost sucessfully created ! Final time of the ghost : %s\n", SpeedrunTimer::Format(demo.playbackTime).c_str());
    }

    console->Print("Could not parse \"%s\"!\n", dir.c_str());
}

CON_COMMAND(ghost_start, "Start ghosts")
{
    demoGhostPlayer.SpawnAllGhosts();
}

CON_COMMAND(ghost_stop, "Stop ghosts.\n")
{
    demoGhostPlayer.PauseAllGhosts();
}