#include "DemoGhostPlayer.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"

#include "Utils.hpp"

#include "Features/Demo/Demo.hpp"
#include "Features/Demo/DemoParser.hpp"
#include "NetworkGhostPlayer.hpp"

#include <filesystem>
#include <fstream>


Variable ghost_sync("ghost_sync", "0", "When loading a new level, pauses the game until other players load it.\n");

DemoGhostPlayer demoGhostPlayer;

DemoGhostPlayer::DemoGhostPlayer()
    : isPlaying(false)
    , isFullGame(false)
{
}

void DemoGhostPlayer::SpawnAllGhosts()
{
    for (auto& ghost : this->ghostPool) {
        ghost.Spawn();
    }

    this->isPlaying = true;
}

void DemoGhostPlayer::StartAllGhost()
{
    for (auto& ghost : this->ghostPool) {
        ghost.ChangeLevel(engine->m_szLevelName);
        ghost.Reset();
        ghost.Spawn();
    }

    this->isPlaying = true;
}

void DemoGhostPlayer::ResetAllGhosts()
{
    this->isPlaying = false;
    for (auto& ghost : this->ghostPool) {
        if (this->IsFullGame()) {
            ghost.ChangeLevel(engine->m_szLevelName);
            ghost.Reset();
        } else {
            ghost.Reset();
        }
    }
}

void DemoGhostPlayer::PauseAllGhosts()
{
    this->isPlaying = false;
}

void DemoGhostPlayer::ResumeAllGhosts()
{
    this->isPlaying = true;
}

void DemoGhostPlayer::DeleteAllGhosts()
{
    for (int i = 0; i < this->ghostPool.size(); ++i) {
        this->ghostPool[i].DeleteGhost();
    }
    this->ghostPool.clear();
    this->isPlaying = false;
}

void DemoGhostPlayer::DeleteGhostsByID(const unsigned int ID)
{
    for (int i = 0; i < this->ghostPool.size(); ++i) {
        if (this->ghostPool[i].ID == ID) {
            this->ghostPool[i].DeleteGhost();
            this->ghostPool.erase(this->ghostPool.begin() + i);
            return;
        }
    }
}

void DemoGhostPlayer::KillAllGhosts(const bool newEntity)
{
    for (auto& ghost : this->ghostPool) {
        ghost.KillGhost(newEntity);
        ghost.DeleteGhost();
    }
}

void DemoGhostPlayer::UpdateGhostsPosition()
{
    for (auto& ghost : this->ghostPool) {
        if (!ghost.hasFinished) {
            if (ghost_sync.GetBool() && !ghost.sameMap) {
                return;
            }
            ghost.UpdateDemoGhost();
        }
    }
}

void DemoGhostPlayer::UpdateGhostsSameMap()
{
    for (auto& ghost : this->ghostPool) {
        ghost.sameMap = engine->m_szLevelName == ghost.currentMap;
        ghost.isAhead = engine->GetMapIndex(ghost.currentMap) > engine->GetMapIndex(engine->m_szLevelName);
    }
}

void DemoGhostPlayer::UpdateGhostsModel(const std::string model)
{
    if (GhostEntity::ghost_type) {
        for (auto& ghost : this->ghostPool) {
            ghost.modelName = model;
            ghost.KillGhost(true);
            ghost.Spawn();
        }
    }
}

void DemoGhostPlayer::Sync()
{
    for (auto& ghost : this->ghostPool) {
        if (!ghost.sameMap && !ghost.isAhead) { //isAhead prevents the ghost from restarting if the player load a save after the ghost has finished a chamber
            ghost.ChangeLevel(engine->m_szLevelName);
            ghost.Reset();
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

bool DemoGhostPlayer::SetupGhostFromDemo(const std::string& demo_path, const unsigned int ghost_ID, bool fullGame)
{
    DemoParser parser;
    Demo demo;
    std::vector<DataGhost> datas;

    if (parser.Parse(demo_path, &demo, true, &datas)) {
        parser.Adjust(&demo);

        DemoDatas demoDatas{ datas, demo };
        if (demoGhostPlayer.GetGhostByID(ghost_ID) == nullptr) {
            DemoGhostEntity ghost = { ghost_ID, demo.clientName, DataGhost{ { 0, 0, 0 }, { 0, 0, 0 } }, demo.mapName };
            if (fullGame) {
                DemoDatas demoDatas{ datas, demo };
                ghost.AddLevelDatas(demoDatas);
            } else {
                ghost.SetFirstLevelDatas(demoDatas);
                ghost.lastLevel = demo.mapName;
            }
            ghost.totalTicks = demo.playbackTicks;
            demoGhostPlayer.AddGhost(ghost);
        } else {
            auto ghost = demoGhostPlayer.GetGhostByID(ghost_ID);
            if (fullGame) {
                ghost->AddLevelDatas(demoDatas);
                ghost->lastLevel = demo.mapName;
                ghost->totalTicks += demo.playbackTicks;
            } else {
                ghost->name = demo.clientName;
                ghost->currentMap = demo.mapName;
                ghost->totalTicks = demo.playbackTicks;
                ghost->lastLevel = demo.mapName;
                ghost->SetFirstLevelDatas(demoDatas);
            }
        }
        return true;
    }
    return false;
}

void DemoGhostPlayer::AddGhost(DemoGhostEntity& ghost)
{
    this->ghostPool.push_back(ghost);
}

bool DemoGhostPlayer::IsPlaying()
{
    return this->isPlaying;
}

bool DemoGhostPlayer::IsFullGame()
{
    return this->isFullGame;
}

void DemoGhostPlayer::PrintRecap()
{
    auto current = 1;
    auto total = this->ghostPool.size();

    console->Print("Recap of all ghosts :\n");

    for (auto& ghost : this->ghostPool) {
        console->Msg("    [%i of %i] %s : %s -> %s in %s\n", current++, total, ghost.name.c_str(), "sp_a1_intro1", ghost.lastLevel.c_str(), SpeedrunTimer::Format(ghost.totalTicks * speedrun->GetIntervalPerTick()).c_str());
    }
}

void DemoGhostPlayer::DrawNames(HudContext* ctx)
{
    auto player = client->GetPlayer(GET_SLOT() + 1);
    if (player) {
        //auto pos = client->GetAbsOrigin(player);
        for (int i = 0; i < this->ghostPool.size(); ++i) {
            if (this->ghostPool[i].sameMap && !this->ghostPool[i].hasFinished) {
                Vector screenPos;
                engine->PointToScreen(this->ghostPool[i].data.position, screenPos);
                ctx->DrawElementOnScreen(i, screenPos.x, screenPos.y - ghost_text_offset.GetInt() - ghost_height.GetInt(), this->ghostPool[i].name.c_str());
            }
        }
    }
}

CON_COMMAND_AUTOCOMPLETEFILE(ghost_set_demo, "ghost_set_demo <demo> [ID]. Ghost will use this demo. If ID is specified, will create or modify the ID-�me ghost.\n", 0, 0, dem)
{
    if (args.ArgC() < 2) {
        return console->Print(ghost_set_demo.ThisPtr()->m_pszHelpString);
    }

    unsigned int ID = args.ArgC() > 2 ? std::atoi(args[2]) : 0;
    if (demoGhostPlayer.SetupGhostFromDemo(engine->GetGameDirectory() + std::string("/") + args[1], ID, false)) {
        console->Print("Ghost sucessfully created ! Final time of the ghost : %s\n", SpeedrunTimer::Format(demoGhostPlayer.GetGhostByID(ID)->GetTotalTime()).c_str());
    } else {
        console->Print("Could not parse \"%s\"!\n", engine->GetGameDirectory() + std::string("/") + args[1]);
    }

    demoGhostPlayer.UpdateGhostsSameMap();
    demoGhostPlayer.isFullGame = false;
}

CON_COMMAND_AUTOCOMPLETEFILE(ghost_set_demos, "ghost_set_demos <first_demo> [ID]. Ghost will setup a speedrun with first_demo, first_demo_2, etc.\n"
                                              "If ID is specified, will create or modify the ID-th ghost.\n",
    0, 0, dem)
{
    if (args.ArgC() < 2) {
        return console->Print(ghost_set_demo.ThisPtr()->m_pszHelpString);
    }

    unsigned int ID = args.ArgC() > 2 ? std::atoi(args[2]) : 0;

    auto dir = engine->GetGameDirectory() + std::string("/") + args[1];
    int counter = 2;

    bool ok = std::filesystem::exists(dir + ".dem");
    if (!ok || !demoGhostPlayer.SetupGhostFromDemo(dir, ID, true)) {
        return console->Print("Could not parse \"%s\"!\n", engine->GetGameDirectory() + std::string("/") + args[1]);
    }
    while (ok) {
        auto tmp_dir = dir + "_" + std::to_string(counter) + ".dem";
        ok = std::filesystem::exists(tmp_dir);
        if (ok && !demoGhostPlayer.SetupGhostFromDemo(tmp_dir, ID, true)) {
            return console->Print("Could not parse \"%s\"!\n", tmp_dir.c_str());
        }
        ++counter;
    }

    console->Print("Ghost sucessfully created ! Final time of the ghost : %s\n", SpeedrunTimer::Format(demoGhostPlayer.GetGhostByID(ID)->GetTotalTime()).c_str());

    demoGhostPlayer.UpdateGhostsSameMap();
    demoGhostPlayer.isFullGame = true;
}

CON_COMMAND(ghost_delete_by_ID, "ghost_delete_by_ID <ID>. Delete the ghost selected.\n")
{
    if (args.ArgC() < 2) {
        return console->Print(ghost_delete_by_ID.ThisPtr()->m_pszHelpString);
    }

    demoGhostPlayer.DeleteGhostsByID(std::atoi(args[1]));
    console->Print("Ghost %d has been deleted !\n", std::atoi(args[1]));
}

CON_COMMAND(ghost_delete_all, "Delete all ghosts.\n")
{
    demoGhostPlayer.DeleteAllGhosts();
    console->Print("All ghosts have been deleted !\n");
}

CON_COMMAND(ghost_recap, "Recap all ghosts setup.\n")
{
    demoGhostPlayer.PrintRecap();
}

CON_COMMAND(ghost_start, "Start ghosts")
{
    if (engine->m_szLevelName[0] == '\0') {
        return console->Print("Can't start ghosts in menu.\n");
    }

    demoGhostPlayer.StartAllGhost();
    console->Print("All ghosts have started.\n");
}

CON_COMMAND(ghost_stop, "Reset ghosts.\n")
{
    demoGhostPlayer.ResetAllGhosts();
    console->Print("All ghost have been reset.\n");
}

CON_COMMAND(ghost_offset, "ghost_offset <offset> <ID>. Delay the ghost start by <offset> frames.\n")
{
    if (args.ArgC() < 2) {
        return console->Print(ghost_offset.ThisPtr()->m_pszHelpString);
    }

    unsigned int ID = args.ArgC() > 2 ? std::atoi(args[2]) : 0;

    auto ghost = demoGhostPlayer.GetGhostByID(ID);
    if (ghost) {
        ghost->offset = std::atoi(args[1]);
        console->Print("Final time of ghost %d : %s\n", ID, SpeedrunTimer::Format(demoGhostPlayer.GetGhostByID(ID)->GetTotalTime()).c_str());
    } else {
        return console->Print("No ghost with that ID\n");
    }
}
