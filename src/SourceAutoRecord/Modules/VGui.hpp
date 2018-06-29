#pragma once
#include "vmthook/vmthook.h"

#include "Console.hpp"
#include "Scheme.hpp"
#include "Server.hpp"
#include "Surface.hpp"
#include "Vars.hpp"

#include "Features/Routing.hpp"
#include "Features/Session.hpp"
#include "Features/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Timer.hpp"
#include "Features/TimerAverage.hpp"
#include "Features/TimerCheckPoints.hpp"

#include "Game.hpp"
#include "Interfaces.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

namespace VGui {

using _Paint = int(__cdecl*)(void* thisptr, int mode);

std::unique_ptr<VMTHook> enginevgui;

bool RespectClShowPos = true;
int FontIndexOffset = 0;

namespace Original {
    _Paint Paint;
}

namespace Detour {
    int __cdecl Paint(void* thisptr, int mode)
    {
        Surface::StartDrawing(Surface::matsurface->GetThisPtr());

        int elements = 0;
        int xPadding = sar_hud_default_padding_x.GetInt();
        int yPadding = sar_hud_default_padding_y.GetInt();
        int spacing = sar_hud_default_spacing.GetInt();

        auto font = Scheme::GetDefaultFont() + sar_hud_default_font_index.GetFloat() - FontIndexOffset;
        auto fontSize = Surface::GetFontHeight(font);

        int r, g, b, a;
        sscanf(sar_hud_default_font_color.GetString(), "%i%i%i%i", &r, &g, &b, &a);
        Color textColor(r, g, b, a);

        if (RespectClShowPos && cl_showpos.GetBool()) {
            elements += 4;
            yPadding += spacing;
        }

        // cl_showpos replacement
        if (sar_hud_text.GetString()[0] != '\0') {
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, (char*)sar_hud_text.GetString());
            elements++;
        }
        if (sar_hud_position.GetBool()) {
            auto abs = Client::GetAbsOrigin();

            char position[64];
            snprintf(position, sizeof(position), "pos: %.3f %.3f %.3f", abs.x, abs.y, abs.z);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, position);
            elements++;
        }
        if (sar_hud_angles.GetBool()) {
            auto va = Engine::GetAngles();

            char angles[64];
            snprintf(angles, sizeof(angles), "ang: %.3f %.3f", va.x, va.y);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, angles);
            elements++;
        }
        if (sar_hud_velocity.GetBool()) {
            auto vel = (sar_hud_velocity.GetInt() == 1)
                ? Client::GetLocalVelocity().Length()
                : Client::GetLocalVelocity().Length2D();

            char velocity[64];
            snprintf(velocity, sizeof(velocity), "vel: %.3f", vel);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, velocity);
            elements++;
        }
        // Session
        if (sar_hud_session.GetBool()) {
            int tick = (Engine::IsInGame) ? Engine::GetTick() : 0;
            float time = Engine::GetTime(tick);

            char session[64];
            snprintf(session, sizeof(session), "session: %i (%.3f)", tick, time);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, session);
            elements++;
        }
        if (sar_hud_last_session.GetBool()) {
            char session[64];
            snprintf(session, sizeof(session), "last session: %i (%.3f)", Session::LastSession, Engine::GetTime(Session::LastSession));
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, session);
            elements++;
        }
        if (sar_hud_sum.GetBool()) {
            char sum[64];
            if (Summary::IsRunning && sar_sum_during_session.GetBool()) {
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
        if (sar_hud_timer.GetBool()) {
            int tick = (!Timer::IsPaused) ? Timer::GetTick(*Vars::tickcount) : Timer::TotalTicks;
            float time = Engine::GetTime(tick);

            char timer[64];
            snprintf(timer, sizeof(timer), "timer: %i (%.3f)", tick, time);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, timer);
            elements++;
        }
        if (sar_hud_avg.GetBool()) {
            char avg[64];
            snprintf(avg, sizeof(avg), "avg: %i (%.3f)", Timer::Average::AverageTicks, Timer::Average::AverageTime);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, avg);
            elements++;
        }
        if (sar_hud_cps.GetBool()) {
            char cps[64];
            snprintf(cps, sizeof(cps), "last cp: %i (%.3f)", Timer::CheckPoints::LatestTick, Timer::CheckPoints::LatestTime);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, cps);
            elements++;
        }
        // Demo
        if (sar_hud_demo.GetBool()) {
            char demo[64];
            if (!*Vars::m_bLoadgame && *DemoRecorder::m_bRecording && !DemoRecorder::CurrentDemo.empty()) {
                int tick = DemoRecorder::GetTick();
                float time = Engine::GetTime(tick);
                snprintf(demo, sizeof(demo), "demo: %s %i (%.3f)", DemoRecorder::CurrentDemo.c_str(), tick, time);
            } else if (!*Vars::m_bLoadgame && DemoPlayer::IsPlaying()) {
                int tick = DemoPlayer::GetTick();
                float time = Engine::GetTime(tick);
                snprintf(demo, sizeof(demo), "demo: %s %i (%.3f)", DemoPlayer::DemoName, tick, time);
            } else {
                snprintf(demo, sizeof(demo), "demo: -");
            }
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, demo);
            elements++;
        }
        // Stats
        if (sar_hud_jumps.GetBool()) {
            char jumps[64];
            snprintf(jumps, sizeof(jumps), "jumps: %i", Stats::TotalJumps);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, jumps);
            elements++;
        }
        if (sar_hud_portals.GetBool()) {
            auto iNumPortalsPlaced = Server::GetPortals();
            char portals[64];
            snprintf(portals, sizeof(portals), "portals: %i", iNumPortalsPlaced);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, portals);
            elements++;
        }
        if (sar_hud_steps.GetBool()) {
            char steps[64];
            snprintf(steps, sizeof(steps), "steps: %i", Stats::TotalSteps);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, steps);
            elements++;
        }
        if (sar_hud_distance.GetBool()) {
            auto jumpDistance = Stats::JumpDistance::LastDistance;
            char distance[64];
            snprintf(distance, sizeof(distance), "distance: %.3f", jumpDistance);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, distance);
            elements++;
        }
        // Routing
        if (sar_hud_trace.GetBool()) {
            auto xyz = Routing::Tracer::GetDifferences();
            auto result = Routing::Tracer::GetResult();
            char trace[64];
            snprintf(trace, sizeof(trace), "trace: %.3f (%.3f/%.3f/%.3f)", result, std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz));
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, trace);
            elements++;
        }
        if (sar_hud_velocity_peak.GetBool()) {
            auto peak = Routing::Velocity::Peak;
            char velocity[64];
            snprintf(velocity, sizeof(velocity), "peak: %.3f", peak);
            Surface::Draw(font, xPadding, yPadding + elements * (fontSize + spacing), textColor, velocity);
            elements++;
        }

        Surface::FinishDrawing();
        return Original::Paint(thisptr, mode);
    }
}

void Hook()
{
    if (Interfaces::IEngineVGui) {
        enginevgui = std::make_unique<VMTHook>(Interfaces::IEngineVGui);
        enginevgui->HookFunction((void*)Detour::Paint, Offsets::Paint);
        Original::Paint = enginevgui->GetOriginalFunction<_Paint>(Offsets::Paint);

        if (Game::Version == Game::Portal) {
            RespectClShowPos = false;
        } else if (Game::Version == Game::Portal2) {
            FontIndexOffset = 1;
        }
    }
}
}