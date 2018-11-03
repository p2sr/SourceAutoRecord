#include "VGui.hpp"

#include "Client.hpp"
#include "Console.hpp"
#include "Engine.hpp"
#include "Scheme.hpp"
#include "Server.hpp"
#include "Surface.hpp"

#include "Features/Hud/InputHud.hpp"
#include "Features/Hud/SpeedrunHud.hpp"
#include "Features/Routing/EntityInspector.hpp"
#include "Features/Routing/Tracer.hpp"
#include "Features/Session.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Summary.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/Timer/TimerAverage.hpp"
#include "Features/Timer/TimerCheckPoints.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"

REDECL(VGui::Paint);

// CEngineVGui::Paint
DETOUR(VGui::Paint, int mode)
{
    surface->StartDrawing(surface->matsurface->ThisPtr());

    auto elements = 0;
    auto xPadding = sar_hud_default_padding_x.GetInt();
    auto yPadding = sar_hud_default_padding_y.GetInt();
    auto spacing = sar_hud_default_spacing.GetInt();

    auto font = scheme->GetDefaultFont() + sar_hud_default_font_index.GetInt();
    auto fontSize = surface->GetFontHeight(font);

    int r, g, b, a;
    sscanf(sar_hud_default_font_color.GetString(), "%i%i%i%i", &r, &g, &b, &a);
    Color textColor(r, g, b, a);

    if (vgui->respectClShowPos && cl_showpos.GetBool()) {
        elements += 4;
        yPadding += spacing;
    }

    auto DrawElement = [font, xPadding, yPadding, fontSize, spacing, textColor, &elements](const char* fmt, ...) {
        va_list argptr;
        va_start(argptr, fmt);
        char data[1024];
        vsnprintf(data, sizeof(data), fmt, argptr);
        va_end(argptr);

        surface->DrawTxt(font,
            xPadding,
            yPadding + elements * (fontSize + spacing),
            textColor,
            data);

        ++elements;
    };

    // cl_showpos replacement
    if (sar_hud_text.GetString()[0] != '\0') {
        DrawElement((char*)sar_hud_text.GetString());
    }
    if (sar_hud_position.GetBool()) {
        auto abs = client->GetAbsOrigin();
        DrawElement("pos: %.3f %.3f %.3f", abs.x, abs.y, abs.z);
    }
    if (sar_hud_angles.GetBool()) {
        auto va = engine->GetAngles();
        if (sar_hud_angles.GetInt() == 1) {
            DrawElement("ang: %.3f %.3f", va.x, va.y);
        } else {
            DrawElement("ang: %.3f %.3f %.3f", va.x, va.y, va.z);
        }
    }
    if (sar_hud_velocity.GetBool()) {
        auto vel = (sar_hud_velocity.GetInt() == 1)
            ? client->GetLocalVelocity().Length()
            : client->GetLocalVelocity().Length2D();
        DrawElement("vel: %.3f", vel);
    }
    // Session
    if (sar_hud_session.GetBool()) {
        auto tick = (session->isInSession) ? engine->GetSessionTick() : 0;
        auto time = engine->ToTime(tick);
        DrawElement("session: %i (%.3f)", tick, time);
    }
    if (sar_hud_last_session.GetBool()) {
        DrawElement("last session: %i (%.3f)", session->lastSession, engine->ToTime(session->lastSession));
    }
    if (sar_hud_sum.GetBool()) {
        if (summary->isRunning && sar_sum_during_session.GetBool()) {
            auto tick = (session->isInSession) ? engine->GetSessionTick() : 0;
            auto time = engine->ToTime(tick);
            DrawElement("sum: %i (%.3f)", summary->totalTicks + tick, engine->ToTime(summary->totalTicks) + time);
        } else {
            DrawElement("sum: %i (%.3f)", summary->totalTicks, engine->ToTime(summary->totalTicks));
        }
    }
    // Timer
    if (sar_hud_timer.GetBool()) {
        auto tick = (!timer->isPaused) ? timer->GetTick(*engine->tickcount) : timer->totalTicks;
        auto time = engine->ToTime(tick);
        DrawElement("timer: %i (%.3f)", tick, time);
    }
    if (sar_hud_avg.GetBool()) {
        DrawElement("avg: %i (%.3f)", timer->avg->averageTicks, timer->avg->averageTime);
    }
    if (sar_hud_cps.GetBool()) {
        DrawElement("last cp: %i (%.3f)", timer->cps->latestTick, timer->cps->latestTime);
    }
    // Demo
    if (sar_hud_demo.GetBool()) {
        if (!*engine->m_bLoadgame && *engine->demorecorder->m_bRecording && !engine->demorecorder->currentDemo.empty()) {
            auto tick = engine->demorecorder->GetTick();
            auto time = engine->ToTime(tick);
            DrawElement("demo: %s %i (%.3f)", engine->demorecorder->currentDemo.c_str(), tick, time);
        } else if (!*engine->m_bLoadgame && engine->demoplayer->IsPlaying()) {
            auto tick = engine->demoplayer->GetTick();
            auto time = engine->ToTime(tick);
            DrawElement("demo: %s %i (%.3f)", engine->demoplayer->DemoName, tick, time);
        } else {
            DrawElement("demo: -");
        }
    }
    // Stats
    if (sar_hud_jumps.GetBool()) {
        DrawElement("jumps: %i", stats->jumps->total);
    }
    if (sar.game->version & (SourceGame_Portal2 | SourceGame_Portal) && sar_hud_portals.GetBool()) {
        DrawElement("portals: %i", server->GetPortals());
    }
    if (sar_hud_steps.GetBool()) {
        DrawElement("steps: %i", stats->steps->total);
    }
    if (sar_hud_jump.GetBool()) {
        DrawElement("jump: %.3f", stats->jumps->distance);
    }
    if (sar_hud_jump_peak.GetBool()) {
        DrawElement("jump peak: %.3f", stats->jumps->distance);
    }
    if (sar_hud_velocity_peak.GetBool()) {
        DrawElement("vel peak: %.3f", stats->velocity->peak);
    }
    // Routing
    if (sar_hud_trace.GetBool()) {
        auto xyz = tracer->GetDifferences();
        auto result = (sar_hud_trace.GetInt() == 1)
            ? tracer->GetResult(TracerResultType::VEC3)
            : tracer->GetResult(TracerResultType::VEC2);
        DrawElement("trace: %.3f (%.3f/%.3f/%.3f)", result, std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz));
    }
    if (sar_hud_frame.GetBool()) {
        DrawElement("frame: %i", session->currentFrame);
    }
    if (sar_hud_last_frame.GetBool()) {
        DrawElement("last frame: %i", session->lastFrame);
    }
    if (sar_hud_inspection.GetBool()) {
        DrawElement(inspector->IsRunning() ? "inspection (recording)" : "inspection");

        auto info = server->GetEntityInfoByIndex(inspector->entityIndex);
        if (info != nullptr) {
            DrawElement("name: %s", server->GetEntityName(info->m_pEntity));
            DrawElement("class: %s", server->GetEntityClassName(info->m_pEntity));
        }

        auto data = inspector->GetData();
        DrawElement("pos: %.3f %.3f %.3f", data.origin.x, data.origin.y, data.origin.z);
        DrawElement("off: %.3f %.3f %.3f", data.viewOffset.x, data.viewOffset.y, data.viewOffset.z);
        DrawElement("ang: %.3f %.3f %.3f", data.angles.x, data.angles.y, data.angles.z);
        DrawElement("vel: %.3f %.3f %.3f", data.velocity.x, data.velocity.y, data.velocity.z);
        DrawElement("flags: %i", data.flags);
        DrawElement("eflags: %i", data.eFlags);
        DrawElement("maxspeed: %.3f", data.maxSpeed);
        DrawElement("gravity: %.3f", data.gravity);
    }

    surface->FinishDrawing();

    // Draw other HUDs
    vgui->inputHud->Draw();
    vgui->speedrunHud->Draw();
#ifndef _WIN32
    vgui->inspectionHud->Draw();
#endif

    return VGui::Paint(thisptr, mode);
}

bool VGui::Init()
{
    this->inputHud = new InputHud();
    this->speedrunHud = new SpeedrunHud();
    this->inspectionHud = new InspectionHud();

    this->enginevgui = Interface::Create(this->Name(), "VEngineVGui0");
    if (this->enginevgui) {
        this->enginevgui->Hook(VGui::Paint_Hook, VGui::Paint, Offsets::Paint);
    }

    if (sar.game->version & SourceGame_HalfLife2Engine) {
        this->respectClShowPos = false;
    }

    return this->hasLoaded = this->inputHud && this->speedrunHud && this->inspectionHud && this->enginevgui;
}
void VGui::Shutdown()
{
    Interface::Delete(this->enginevgui);

    SAFE_DELETE(this->inputHud)
    SAFE_DELETE(this->speedrunHud)
    SAFE_DELETE(this->inspectionHud)
}

VGui* vgui;
