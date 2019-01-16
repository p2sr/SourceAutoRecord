#include "TasTools.hpp"

#include <cmath>
#include <cstring>

#include "Features/OffsetFinder.hpp"

#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "SAR.hpp"
#include "Utils.hpp"

TasTools* tasTools;

TasTools::TasTools()
    : propName("m_hGroundEntity")
    , propType(PropType::Handle)
    , acceleration({ 0, 0, 0 })
    , prevVelocity({ 0, 0, 0 })
    , prevTick(0)
{
    if (sar.game->version & (SourceGame_Portal | SourceGame_Portal2Engine)) {
        std::strncpy(this->className, "CPortal_Player", sizeof(this->className));
    } else if (sar.game->version & SourceGame_HalfLife2) {
        std::strncpy(this->className, "CHL2_Player", sizeof(this->className));
    } else {
        std::strncpy(this->className, "CBasePlayer", sizeof(this->className));
    }

    offsetFinder->ServerSide(this->className, this->propName, &this->propOffset);

    this->hasLoaded = true;
}
void TasTools::AimAtPoint(void* player, float x, float y, float z)
{
    Vector target = { y, x, z };
    Vector campos = server->GetAbsOrigin(player) + server->GetViewOffset(player);
    campos = { campos.y, campos.x, campos.z };

    // Axis in the game, need to know it to fix up:
    //              : G - D ; Av - Ar ; H - B
    // Rotation Axis:   x        z        y
    // Translation  :   y        x        z

    float xdis = target.x - campos.x;
    float ydis = target.z - campos.z;
    float zdis = target.y - campos.y;
    float xzdis = sqrtf(xdis * xdis + zdis * zdis);

    QAngle angles = { RAD2DEG(-atan2f(ydis, xzdis)), RAD2DEG(-(atan2f(-xdis, zdis))), 0 };

    engine->SetAngles(angles);
}
void* TasTools::GetPlayerInfo()
{
    auto player = server->GetPlayer();
    return (player) ? reinterpret_cast<void*>((uintptr_t)player + this->propOffset) : nullptr;
}
Vector TasTools::GetVelocityAngles(void* player)
{
    auto velocityAngles = server->GetLocalVelocity(player);
    if (velocityAngles.Length() == 0) {
        return { 0, 0, 0 };
    }

    Math::VectorNormalize(velocityAngles);

    float yaw = atan2f(velocityAngles.y, velocityAngles.x);
    float pitch = atan2f(velocityAngles.z, sqrtf(velocityAngles.y * velocityAngles.y + velocityAngles.x * velocityAngles.x));

    return { RAD2DEG(yaw), RAD2DEG(pitch), 0 };
}
Vector TasTools::GetAcceleration(void* player)
{
    auto curTick = engine->GetSessionTick();
    if (this->prevTick != curTick) {
        auto curVelocity = server->GetLocalVelocity(player);

        // z used to represent the combined x/y acceleration axis value
        this->acceleration.z = curVelocity.Length2D() - prevVelocity.Length2D();
        this->acceleration.x = std::abs(curVelocity.x) - std::abs(this->prevVelocity.x);
        this->acceleration.y = std::abs(curVelocity.y) - std::abs(this->prevVelocity.y);

        this->prevVelocity = curVelocity;
        this->prevTick = curTick;
    }

    return this->acceleration;
}

// Commands

CON_COMMAND(sar_tas_aim_at_point, "sar_tas_aim_at_point <x> <y> <z> : Aims at point {x, y, z} specified.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot use sar_tas_aim_at_point without sv_cheats set to 1.\n");
    }

    if (args.ArgC() != 4) {
        return console->Print("sar_tas_aim_at_point <x> <y> <z> : Aims at point {x, y, z} specified.\n");
    }

    auto player = server->GetPlayer();
    if (player) {
        tasTools->AimAtPoint(player, static_cast<float>(std::atof(args[1])), static_cast<float>(std::atof(args[2])), static_cast<float>(std::atof(args[3])));
    }
}
CON_COMMAND(sar_tas_set_prop, "sar_tas_set_prop <prop_name> : Sets value for sar_hud_player_info.\n")
{
    if (args.ArgC() < 2) {
        return console->Print("sar_tas_set_prop <prop_name> : Sets value for sar_hud_player_info.\n");
    }

    auto offset = 0;
    offsetFinder->ServerSide(tasTools->className, args[1], &offset);

    if (!offset) {
        console->Print("Unknown prop of %s!\n", tasTools->className);
    } else {
        std::strncpy(tasTools->propName, args[1], sizeof(tasTools->propName));
        tasTools->propOffset = offset;

        if (Utils::StartsWith(tasTools->propName, "m_b")) {
            tasTools->propType = PropType::Boolean;
        } else if (Utils::StartsWith(tasTools->propName, "m_f")) {
            tasTools->propType = PropType::Float;
        } else if (Utils::StartsWith(tasTools->propName, "m_vec") || Utils::StartsWith(tasTools->propName, "m_ang") || Utils::StartsWith(tasTools->propName, "m_q")) {
            tasTools->propType = PropType::Vector;
        } else if (Utils::StartsWith(tasTools->propName, "m_h") || Utils::StartsWith(tasTools->propName, "m_p")) {
            tasTools->propType = PropType::Handle;
        } else if (Utils::StartsWith(tasTools->propName, "m_sz") || Utils::StartsWith(tasTools->propName, "m_isz")) {
            tasTools->propType = PropType::String;
        } else if (Utils::StartsWith(tasTools->propName, "m_ch")) {
            tasTools->propType = PropType::Char;
        } else {
            tasTools->propType = PropType::Integer;
        }
    }

    console->Print("Current prop: %s::%s\n", tasTools->className, tasTools->propName);
}
CON_COMMAND(sar_tas_addang, "sar_tas_addang <x> <y> [z] : Adds {x, y, z} degrees to {x, y, z} view axis.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot use sar_tas_addang without sv_cheats set to 1.\n");
    }

    if (args.ArgC() < 3) {
        return console->Print("Missing arguments : sar_tas_addang <x> <y> [z].\n");
    }

    auto angles = engine->GetAngles();

    angles.x += static_cast<float>(std::atof(args[1]));
    angles.y += static_cast<float>(std::atof(args[2])); // Player orientation

    if (args.ArgC() == 4)
        angles.z += static_cast<float>(std::atof(args[3]));

    engine->SetAngles(angles);
}
CON_COMMAND(sar_tas_setang, "sar_tas_setang <x> <y> [z] : Sets {x, y, z} degrees to view axis.\n")
{
    if (!sv_cheats.GetBool()) {
        return console->Print("Cannot use sar_tas_setang without sv_cheats set to 1.\n");
    }

    if (args.ArgC() < 3) {
        return console->Print("Missing arguments : sar_tas_setang <x> <y> [z].\n");
    }

    // Fix the bug when z is not set
    if (args.ArgC() == 3) {
        engine->SetAngles(QAngle{ static_cast<float>(std::atof(args[1])), static_cast<float>(std::atof(args[2])), 0.0f });
    } else {
        engine->SetAngles(QAngle{ static_cast<float>(std::atof(args[1])), static_cast<float>(std::atof(args[2])), static_cast<float>(std::atof(args[3])) });
    }
}
