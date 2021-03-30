#include "TimescaleDetect.hpp"

#include "Modules/Engine.hpp"
#include "Modules/EngineDemoRecorder.hpp"
#include "Modules/Console.hpp"
#include "Modules/Client.hpp"

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

        if (timescale > 1 + THRESHOLD || timescale < 1 - THRESHOLD) {
            char data[5];
            data[0] = 0x01;
            // THIS IS REALLY BAD AND HACKY but whatever it'll work
            *(float*)(data+1) = timescale;
            engine->demorecorder->RecordData(data, sizeof data);
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
