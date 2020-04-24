#pragma once
#include <vector>

#include "Features/Feature.hpp"

#include "Command.hpp"
#include "Features/Demo/Demo.hpp"
#include "Utils/SDK.hpp"
#include "Variable.hpp"

#include <chrono>
#include <vector>

struct DataGhost {
    QAngle position;
    QAngle view_angle;
};

class GhostEntity {

private:
    int tickCount;
    float startDelay;
    std::chrono::steady_clock clock;

public:
    std::vector<Vector> positionList;
    std::vector<Vector> angleList;
    unsigned int ID;
    int ghostType;
    std::string name;
    std::string currentMap;
    bool sameMap;
    Demo demo;
    int startTick;
    void* ghost_entity;
    int CMTime;
    char modelName[64];
    bool isPlaying;
    bool hasFinished;

    DataGhost oldPos;
    DataGhost newPos;
    Vector currentPos;
    std::chrono::time_point<std::chrono::steady_clock> lastUpdate;
    long long loopTime;

public:
    GhostEntity(int ghostType);
    void Reset();
    void Stop();
    void KillGhost(bool newEntity);
    GhostEntity* Spawn(bool instantPlay, Vector position, int ghostType);
    void ChangeType(int newType);

    bool IsReady();
    void SetCMTime(float);
    void Think();
    int GetTickCount();
    int GetStartDelay();
    void SetStartDelay(int);
    
	void ChangeModel(std::string modelName);
    void SetPosAng(const Vector&, const Vector&);
    void Lerp(DataGhost& oldPosition, DataGhost& targetPosition, float time);
};


extern Variable sar_ghost_height;
extern Variable sar_ghost_transparency;
extern Variable sar_ghost_name_offset;
extern Command sar_ghost_type;
/*extern Variable sar_hud_ghost_show_crouched;*/