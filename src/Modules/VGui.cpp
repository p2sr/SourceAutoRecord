#include "VGui.hpp"

#include "Features/EntityList.hpp"
#include "Features/Hud/InputHud.hpp"
#include "Features/Hud/InspectionHud.hpp"
#include "Features/Hud/SpeedrunHud.hpp"
#include "Features/Routing/EntityInspector.hpp"
#include "Features/Routing/Tracer.hpp"
#include "Features/Session.hpp"
#include "Features/Stats/Stats.hpp"
#include "Features/StepCounter.hpp"
#include "Features/Summary.hpp"
#include "Features/Tas/TasTools.hpp"
#include "Features/Timer/PauseTimer.hpp"
#include "Features/Timer/Timer.hpp"
#include "Features/Timer/TimerAverage.hpp"
#include "Features/Timer/TimerCheckPoints.hpp"

#include "Client.hpp"
#include "Console.hpp"
#include "Engine.hpp"
#include "Scheme.hpp"
#include "Server.hpp"
#include "Surface.hpp"

#include "Game.hpp"
#include "Interface.hpp"
#include "Offsets.hpp"
#include "SAR.hpp"
#include "Utils.hpp"

REDECL(VGui::Paint);

// CEngineVGui::Paint
DETOUR(VGui::Paint, int mode)
{
    surface->StartDrawing(surface->matsurface->ThisPtr());

    auto slot = GET_SLOT();

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
        auto player = server->GetPlayer();
        if (player) {
            auto pos = server->GetAbsOrigin(player);
            DrawElement("pos: %.3f %.3f %.3f", pos.x, pos.y, pos.z);
        } else {
            DrawElement("pos: -");
        }
    }
    if (sar_hud_angles.GetBool()) {
        auto ang = engine->GetAngles();
        if (sar_hud_angles.GetInt() == 1) {
            DrawElement("ang: %.3f %.3f", ang.x, ang.y);
        } else {
            DrawElement("ang: %.3f %.3f %.3f", ang.x, ang.y, ang.z);
        }
    }
    if (sar_hud_velocity.GetBool()) {
        auto player = server->GetPlayer();
        if (player) {
            auto vel = (sar_hud_velocity.GetInt() == 1)
                ? server->GetLocalVelocity(player).Length()
                : server->GetLocalVelocity(player).Length2D();
            DrawElement("vel: %.3f", vel);
        } else {
            DrawElement("vel: -");
        }
    }
    // Session
    if (sar_hud_session.GetBool()) {
        auto tick = (session->isRunning) ? engine->GetSessionTick() : 0;
        DrawElement("session: %i (%.3f)", tick, engine->ToTime(tick));
    }
    if (sar_hud_last_session.GetBool()) {
        DrawElement("last session: %i (%.3f)", session->lastSession, engine->ToTime(session->lastSession));
    }
    if (sar_hud_sum.GetBool()) {
        if (summary->isRunning && sar_sum_during_session.GetBool()) {
            auto tick = (session->isRunning) ? engine->GetSessionTick() : 0;
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
    if (sar_hud_pause_timer.GetBool()) {
        auto tick = pauseTimer->GetTotal();
        auto time = engine->ToTime(tick);
        DrawElement("pause: %i (%.3f)", tick, time);
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
        auto stat = stats->Get(slot);
        DrawElement("jumps: %i", stat->jumps->total);
    }
    if (sar_hud_portals.isRegistered && sar_hud_portals.GetBool()) {
        auto player = server->GetPlayer();
        if (player) {
            DrawElement("portals: %i", server->GetPortals(player));
        } else {
            DrawElement("portals: -");
        }
        DrawElement("aaa: %i", slot);
    }
    if (sar_hud_steps.GetBool()) {
        auto stat = stats->Get(slot);
        DrawElement("steps: %i", stat->steps->total);
    }
    if (sar_hud_jump.GetBool()) {
        auto stat = stats->Get(slot);
        DrawElement("jump: %.3f", stat->jumps->distance);
    }
    if (sar_hud_jump_peak.GetBool()) {
        auto stat = stats->Get(slot);
        DrawElement("jump peak: %.3f", stat->jumps->distance);
    }
    if (sar_hud_velocity_peak.GetBool()) {
        auto stat = stats->Get(slot);
        DrawElement("vel peak: %.3f", stat->velocity->peak);
    }
    // Routing
    if (sar_hud_trace.GetBool()) {
        auto result = tracer->GetTraceResult(slot);
        auto xyz = tracer->CalculateDifferences(result);
        auto length = (sar_hud_trace.GetInt() == 1)
            ? tracer->CalculateLength(result, TracerLengthType::VEC3)
            : tracer->CalculateLength(result, TracerLengthType::VEC2);
        DrawElement("trace: %.3f (%.3f/%.3f/%.3f)", length, std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz));
    }
    if (sar_hud_frame.GetBool()) {
        DrawElement("frame: %i", session->currentFrame);
    }
    if (sar_hud_last_frame.GetBool()) {
        DrawElement("last frame: %i", session->lastFrame);
    }
    if (sar_hud_inspection.GetBool()) {
        DrawElement(inspector->IsRunning() ? "inspection (recording)" : "inspection");

        auto info = entityList->GetEntityInfoByIndex(inspector->entityIndex);
        if (info && info->m_pEntity) {
            DrawElement("name: %s", server->GetEntityName(info->m_pEntity));
            DrawElement("class: %s", server->GetEntityClassName(info->m_pEntity));
        } else {
            DrawElement("name: -");
            DrawElement("class: -");
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
    // Tas tools
    if (sar_hud_velocity_angle.GetBool()) {
        auto player = server->GetPlayer();
        if (player) {
            auto velocityAngles = tasTools->GetVelocityAngles(player);
            DrawElement("vel ang: %.3f %.3f", velocityAngles.x, velocityAngles.y);
        } else {
            DrawElement("vel ang: -");
        }
    }
    if (sar_hud_acceleration.GetBool()) {
        auto player = server->GetPlayer();
        if (player) {
            auto acceleration = tasTools->GetAcceleration(player);
            if (sar_hud_acceleration.GetInt() == 1) {
                DrawElement("accel: %.3f %.3f", acceleration.x, acceleration.y);
            } else {
                DrawElement("accel: %.3f", acceleration.z);
            }
        } else {
            DrawElement("accel: -");
        }
    }
    if (sar_hud_player_info.GetBool()) {
        auto info = tasTools->GetPlayerInfo();
        if (info) {
            if (tasTools->propType == PropType::Boolean) {
                DrawElement("%s::%s: %s", tasTools->className, tasTools->propName, *reinterpret_cast<bool*>(info) ? "true" : "false");
            } else if (tasTools->propType == PropType::Float) {
                DrawElement("%s::%s: %.3f", tasTools->className, tasTools->propName, *reinterpret_cast<float*>(info));
            } else if (tasTools->propType == PropType::Vector) {
                auto vec = *reinterpret_cast<Vector*>(info);
                DrawElement("%s::%s: %.3f %.3f %.3f", tasTools->className, tasTools->propName, vec.x, vec.y, vec.z);
            } else if (tasTools->propType == PropType::Handle) {
                DrawElement("%s::%s: %p", tasTools->className, tasTools->propName, *reinterpret_cast<void**>(info));
            } else if (tasTools->propType == PropType::String) {
                DrawElement("%s::%s: %s", tasTools->className, tasTools->propName, *reinterpret_cast<char**>(info));
            } else if (tasTools->propType == PropType::Char) {
                DrawElement("%s::%s: %c", tasTools->className, tasTools->propName, *reinterpret_cast<char*>(info));
            } else {
                DrawElement("%s::%s: %i", tasTools->className, tasTools->propName, *reinterpret_cast<int*>(info));
            }
        } else {
            DrawElement("%s::%s: -", tasTools->className, tasTools->propName);
        }
    }

    surface->FinishDrawing();

    // Draw other HUDs
    if (slot == 0) {
        for (auto const& hud : vgui->huds) {
            hud->Draw();
        }
    } else if (slot == 1) {
        for (auto const& hud : vgui->huds2) {
            hud->Draw();
        }
    }

    return VGui::Paint(thisptr, mode);
}

bool VGui::Init()
{
    this->enginevgui = Interface::Create(this->Name(), "VEngineVGui0");
    if (this->enginevgui) {
        this->enginevgui->Hook(VGui::Paint_Hook, VGui::Paint, Offsets::Paint);
    }

    this->huds.push_back(inputHud = new InputHud());
    this->huds2.push_back(inputHud2 = new InputHud());
    this->huds.push_back(inspectionHud = new InspectionHud());

    if (sar.game->version & (SourceGame_Portal2Game | SourceGame_Portal)) {
        this->huds.push_back(speedrunHud = new SpeedrunHud());
    }

    if (sar.game->version & SourceGame_HalfLife2Engine) {
        this->respectClShowPos = false;
    }

    return this->hasLoaded = this->enginevgui;
}
void VGui::Shutdown()
{
    Interface::Delete(this->enginevgui);

    for (auto const& hud : this->huds) {
        delete hud;
    }
    this->huds.clear();
}

VGui* vgui;
