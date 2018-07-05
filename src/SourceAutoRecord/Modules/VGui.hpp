#pragma once
#include "Console.hpp"
#include "Engine.hpp"
#include "Scheme.hpp"
#include "Server.hpp"
#include "Surface.hpp"

#include "Features/Routing.hpp"
#include "Features/Session.hpp"
#include "Features/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Timer.hpp"
#include "Features/TimerAverage.hpp"
#include "Features/TimerCheckPoints.hpp"

#include "Cheats.hpp"
#include "Game.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace VGui {

VMT enginevgui;

using _Paint = int(__func*)(void* thisptr, int mode);

bool RespectClShowPos = true;
int FontIndexOffset = 0;

// CEngineVGui::Paint
DETOUR(Paint, int mode)
{
    Surface::StartDrawing(Surface::matsurface->GetThisPtr());

    int elements = 0;
    int xPadding = Cheats::sar_hud_default_padding_x.GetInt();
    int yPadding = Cheats::sar_hud_default_padding_y.GetInt();
    int spacing = Cheats::sar_hud_default_spacing.GetInt();

    auto font = Scheme::GetDefaultFont() + (int)Cheats::sar_hud_default_font_index.GetFloat() - VGui::FontIndexOffset;
    auto fontSize = Surface::GetFontHeight(font);

    int r, g, b, a;
    sscanf_s(Cheats::sar_hud_default_font_color.GetString(), "%i%i%i%i", &r, &g, &b, &a);
    Color textColor(r, g, b, a);

    if (RespectClShowPos && Cheats::cl_showpos.GetBool()) {
        elements += 6;
        yPadding += spacing;
    }

    // cl_showpos replacement
    if (Cheats::sar_hud_text.GetString()[0] != '\0') {
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, (char*)Cheats::sar_hud_text.GetString());
        elements++;
    }
    if (Cheats::sar_hud_position.GetBool()) {
        auto abs = Client::GetAbsOrigin();

        char position[64];
        snprintf(position, sizeof(position), "pos: %.3f %.3f %.3f", abs.x, abs.y, abs.z);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, position);
        elements++;
    }
    if (Cheats::sar_hud_angles.GetBool()) {
        auto va = Engine::GetAngles();

        char angles[64];
        snprintf(angles, sizeof(angles), "ang: %.3f %.3f", va.x, va.y);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, angles);
        elements++;
    }
    if (Cheats::sar_hud_velocity.GetBool()) {
        auto vel = (Cheats::sar_hud_velocity.GetInt() == 1)
            ? Client::GetLocalVelocity().Length()
            : Client::GetLocalVelocity().Length2D();

        char velocity[64];
        snprintf(velocity, sizeof(velocity), "vel: %.3f", vel);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, velocity);
        elements++;
    }
    // Session
    if (Cheats::sar_hud_session.GetBool()) {
        int tick = (Engine::IsInGame) ? Engine::GetTick() : 0;
        float time = Engine::GetTime(tick);

        char session[64];
        snprintf(session, sizeof(session), "session: %i (%.3f)", tick, time);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, session);
        elements++;
    }
    if (Cheats::sar_hud_last_session.GetBool()) {
        char session[64];
        snprintf(session, sizeof(session), "last session: %i (%.3f)", Session::LastSession, Engine::GetTime(Session::LastSession));
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, session);
        elements++;
    }
    if (Cheats::sar_hud_sum.GetBool()) {
        char sum[64];
        if (Summary::IsRunning && Cheats::sar_sum_during_session.GetBool()) {
            int tick = (Engine::IsInGame) ? Engine::GetTick() : 0;
            float time = Engine::GetTime(tick);
            snprintf(sum, sizeof(sum), "sum: %i (%.3f)", Summary::TotalTicks + tick, Engine::GetTime(Summary::TotalTicks) + time);
        } else {
            snprintf(sum, sizeof(sum), "sum: %i (%.3f)", Summary::TotalTicks, Engine::GetTime(Summary::TotalTicks));
        }
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, sum);
        elements++;
    }
    // Timer
    if (Cheats::sar_hud_timer.GetBool()) {
        int tick = (!Timer::IsPaused) ? Timer::GetTick(*Engine::tickcount) : Timer::TotalTicks;
        float time = Engine::GetTime(tick);

        char timer[64];
        snprintf(timer, sizeof(timer), "timer: %i (%.3f)", tick, time);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, timer);
        elements++;
    }
    if (Cheats::sar_hud_avg.GetBool()) {
        char avg[64];
        snprintf(avg, sizeof(avg), "avg: %i (%.3f)", Timer::Average::AverageTicks, Timer::Average::AverageTime);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, avg);
        elements++;
    }
    if (Cheats::sar_hud_cps.GetBool()) {
        char cps[64];
        snprintf(cps, sizeof(cps), "last cp: %i (%.3f)", Timer::CheckPoints::LatestTick, Timer::CheckPoints::LatestTime);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, cps);
        elements++;
    }
    // Demo
    if (Cheats::sar_hud_demo.GetBool()) {
        char demo[64];
        if (!*Engine::m_bLoadgame && *Engine::DemoRecorder::m_bRecording && !Engine::DemoRecorder::CurrentDemo.empty()) {
            int tick = Engine::DemoRecorder::GetTick();
            float time = Engine::GetTime(tick);
            snprintf(demo, sizeof(demo), "demo: %s %i (%.3f)", Engine::DemoRecorder::CurrentDemo.c_str(), tick, time);
        } else if (!*Engine::m_bLoadgame && Engine::DemoPlayer::IsPlaying()) {
            int tick = Engine::DemoPlayer::GetTick();
            float time = Engine::GetTime(tick);
            snprintf(demo, sizeof(demo), "demo: %s %i (%.3f)", Engine::DemoPlayer::DemoName, tick, time);
        } else {
            snprintf(demo, sizeof(demo), "demo: -");
        }
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, demo);
        elements++;
    }
    // Stats
    if (Cheats::sar_hud_jumps.GetBool()) {
        char jumps[64];
        snprintf(jumps, sizeof(jumps), "jumps: %i", Stats::Jumps::Total);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, jumps);
        elements++;
    }
    if (Cheats::sar_hud_portals.GetBool()) {
        auto iNumPortalsPlaced = Server::GetPortals();
        char portals[64];
        snprintf(portals, sizeof(portals), "portals: %i", iNumPortalsPlaced);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, portals);
        elements++;
    }
    if (Cheats::sar_hud_steps.GetBool()) {
        char steps[64];
        snprintf(steps, sizeof(steps), "steps: %i", Stats::Steps::Total);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, steps);
        elements++;
    }
    if (Cheats::sar_hud_jump.GetBool()) {
        auto latest = Stats::Jumps::Distance;
        char distance[64];
        snprintf(distance, sizeof(distance), "jump: %.3f", latest);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, distance);
        elements++;
    }
    if (Cheats::sar_hud_jump_peak.GetBool()) {
        auto peak = Stats::Jumps::Distance;
        char distance[64];
        snprintf(distance, sizeof(distance), "jump peak: %.3f", peak);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, distance);
        elements++;
    }
    if (Cheats::sar_hud_velocity_peak.GetBool()) {
        auto peak = Stats::Velocity::Peak;
        char velocity[64];
        snprintf(velocity, sizeof(velocity), "vel peak: %.3f", peak);
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, velocity);
        elements++;
    }
    // Routing
    if (Cheats::sar_hud_trace.GetBool()) {
        auto xyz = Routing::Tracer::GetDifferences();
        auto result = Routing::Tracer::GetResult();
        char trace[64];
        snprintf(trace, sizeof(trace), "trace: %.3f (%.3f/%.3f/%.3f)", result, std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz));
        Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, trace);
        elements++;
    }

    Surface::FinishDrawing();
    return Original::Paint(thisptr, mode);
}

void Hook()
{
    if (SAR::NewVMT(Interfaces::IEngineVGui, enginevgui)) {
        HOOK(enginevgui, Paint);

        if (Game::IsHalfLife2Engine()) {
            RespectClShowPos = false;
        } else if (Game::Version == Game::Portal2) {
#ifdef _WIN32
            FontIndexOffset = 0;
#else
            FontIndexOffset = 1;
#endif
        }
    }
}
void Unhook()
{
    UNHOOK(enginevgui, Paint);
    SAR::DeleteVMT(enginevgui);
}
}