#include "TimescaleDetect.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Console.hpp"
#include "Modules/Client.hpp"
#include "Features/Speedrun/SpeedrunTimer.hpp"

#include <chrono>

// 10% difference will trigger cheat warnings
#define THRESHOLD 0.1

// Timescale will be measured over 30 ticks
#define TICKS_MEASURE 30

// The measured timescale tends to act a bit weirdly shortly after
// spawning - we ignore discrepancies for the first 60 ticks
#define TICKS_IGNORE 60

TimescaleDetect* timescaleDetect;

TimescaleDetect::TimescaleDetect()
{
    this->hasLoaded = true;
    this->startTick = -1;
}

void TimescaleDetect::Update()
{
    int tick = engine->GetTick();

    if (tick - this->spawnTick < TICKS_IGNORE) {
        this->startTick = -1;
        return;
    }

    if (this->startTick == -1 || tick < this->startTick) {
        this->startTick = tick;
        this->startTickTime = std::chrono::system_clock::now();
        return;
    }

    if (tick >= this->startTick + TICKS_MEASURE) {
        auto now = std::chrono::system_clock::now();

        int tickDelta = tick - this->startTick;

        std::chrono::duration<float> deltaDur = now - this->startTickTime;
        float delta = deltaDur.count();
        float expected = *engine->interval_per_tick * tickDelta;

        float timescale = expected / delta;

        char msg[128];
        msg[0] = 0;

        if (timescale > 1 + THRESHOLD) {
            snprintf(msg, sizeof msg, "Potential timescale increase to %.2f", timescale);
        } else if (timescale < 1 - THRESHOLD) {
            snprintf(msg, sizeof msg, "Potential timescale decrease to %.2f", timescale);
        }

        if (msg[0]) {
            speedrun->StatusReportHidden(msg);
#if 0
            client->Chat(TextColor::ORANGE, msg);
#endif
        }

        this->startTick = tick;
        this->startTickTime = now;
    }
}

void TimescaleDetect::Cancel()
{
    this->startTick = -1;
}

void TimescaleDetect::Spawn()
{
    this->startTick = -1;
    this->spawnTick = engine->GetTick();
}
