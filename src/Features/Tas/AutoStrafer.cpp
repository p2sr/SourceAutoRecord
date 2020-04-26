#include "AutoStrafer.hpp"

#include <cmath>
#include <cstring>

#include "Features/OffsetFinder.hpp"
#include "Features/Tas/TasTools.hpp"

#include "Modules/Client.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"

#include "Command.hpp"
#include "Offsets.hpp"
#include "Utils.hpp"
#include "Variable.hpp"

AutoStrafer* autoStrafer;

AutoStrafer::AutoStrafer()
    : in_autostrafe()
    , states()
{
    for (auto i = 0; i < Offsets::MAX_SPLITSCREEN_PLAYERS; ++i) {
        this->states.push_back(new StrafeState());
    }

    this->hasLoaded = true;
}
AutoStrafer::~AutoStrafer()
{
    for (auto& state : this->states) {
        delete state;
    }
    this->states.clear();
}
void AutoStrafer::Strafe(void* pPlayer, CMoveData* pMove)
{
    auto slot = server->GetSplitScreenPlayerSlot(pPlayer);
    auto strafe = this->states[slot];

    if (pMove->m_nButtons & IN_AUTOSTRAFE && !(pMove->m_nOldButtons & IN_AUTOSTRAFE)) {
        strafe->type = StrafingType::Straight;
    } else if (pMove->m_nOldButtons & IN_AUTOSTRAFE && !(pMove->m_nButtons & IN_AUTOSTRAFE)) {
        strafe->type = StrafingType::None;
    }

    if (strafe->type == StrafingType::None) {
        return;
    }

    float velAngle = tasTools->GetVelocityAngles(pPlayer).x;
    float angle = velAngle + this->GetStrafeAngle(strafe, pPlayer, pMove);

    // whishdir set based on current angle ("controller" inputs)
    if (strafe->vecType == VecStrafeType::Normal || strafe->vecType == VecStrafeType::Visual) {
        // make vectorial strafing look like AD strafing
        if (strafe->vecType == VecStrafeType::Visual) {
            QAngle newAngle = { 0, velAngle, 0 };
            pMove->m_vecViewAngles = newAngle;
            pMove->m_vecAbsViewAngles = newAngle;

            engine->SetAngles(slot, newAngle);
        }

        angle = DEG2RAD(angle - pMove->m_vecAbsViewAngles.y);
        pMove->m_flForwardMove = cosf(angle) * cl_forwardspeed.GetFloat();
        pMove->m_flSideMove = -sinf(angle) * cl_sidespeed.GetFloat();
    } else {
        // Angle set based on current wishdir
        float lookangle = RAD2DEG(atan2f(pMove->m_flSideMove, pMove->m_flForwardMove));

        QAngle newAngle = { 0, angle + lookangle, 0 };
        pMove->m_vecViewAngles = newAngle;
        pMove->m_vecAbsViewAngles = newAngle;
        engine->SetAngles(slot, newAngle);
    }

    if (strafe->type == StrafingType::Straight) {
        strafe->direction *= -1;
    }
}
float AutoStrafer::GetStrafeAngle(const StrafeState* strafe, void* pPlayer, const CMoveData* pMove)
{
    float tau = server->gpGlobals->frametime; // A time for one frame to pass

    // Getting player's friction
    float playerFriction = *reinterpret_cast<float*>((uintptr_t)pPlayer + Offsets::m_flFriction);
    float friction = sv_friction.GetFloat() * playerFriction * 1;

    // Creating lambda(v) - velocity after ground friction
    Vector velocity = pMove->m_vecVelocity;
    Vector lambda = velocity;

    bool pressedJump = ((pMove->m_nButtons & IN_JUMP) > 0 && (pMove->m_nOldButtons & IN_JUMP) == 0);
    bool grounded = pMove->m_vecVelocity.z == 0 && !pressedJump;
    if (grounded) {
        if (velocity.Length2D() >= sv_stopspeed.GetFloat()) {
            lambda = lambda * (1.0f - tau * friction);
        } else if (velocity.Length2D() >= std::fmax(0.1, tau * sv_stopspeed.GetFloat() * friction)) {
            Vector normalizedVelocity = velocity;
            Math::VectorNormalize(normalizedVelocity);
            lambda = lambda + (normalizedVelocity * (tau * sv_stopspeed.GetFloat() * friction)) * -1; // lambda -= v * tau * stop * friction
        } else {
            lambda = lambda * 0;
        }
    }

    // Getting M
    float maxSpeed = *reinterpret_cast<float*>((uintptr_t)pPlayer + Offsets::m_flMaxspeed);

    float forwardMove = pMove->m_flForwardMove;
    float sideMove = pMove->m_flSideMove;

    auto isCrouched = *reinterpret_cast<bool*>((uintptr_t)pPlayer + Offsets::m_bDucked);
    float duckMultiplier = (grounded && isCrouched) ? 1.0f / 3.0f : 1.0f;

    float stateLen = sqrt(forwardMove * forwardMove + sideMove * sideMove);
    float F = forwardMove * maxSpeed * duckMultiplier / stateLen;
    float S = sideMove * maxSpeed * duckMultiplier / stateLen;

    float M = std::fminf(maxSpeed, sqrt(F * F + S * S));

    // Getting other stuff
    float A = (grounded) ? sv_accelerate.GetFloat() : sv_airaccelerate.GetFloat() / 2;
    float L = (grounded) ? M : std::fminf(60, M);

    // Getting the most optimal angle
    float cosTheta;
    if (strafe->type == StrafingType::Turning && !grounded) {
        cosTheta = (playerFriction * tau * M * A) / (2 * velocity.Length2D());
    } else if (strafe->type == StrafingType::Turning && grounded) {
        cosTheta = std::cos(velocity.Length2D() * velocity.Length2D() - std::pow(playerFriction * tau * M * A, 2) - lambda.Length2D() * lambda.Length2D()) / (2 * playerFriction * tau * M * A * lambda.Length2D());
    } else {
        cosTheta = (L - playerFriction * tau * M * A) / lambda.Length2D();
    }

    if (cosTheta < 0)
        cosTheta = M_PI_F / 2;
    if (cosTheta > 1)
        cosTheta = 0;

    float theta;
    if (strafe->type == StrafingType::Turning) {
        theta = (M_PI_F - acosf(cosTheta)) * ((strafe->direction > 0) ? -1 : 1);
    } else {
        theta = acosf(cosTheta) * ((strafe->direction > 0) ? -1 : 1);
    }

    return RAD2DEG(theta);
}

// Commands

void IN_AutoStrafeDown(const CCommand& args) { client->KeyDown(&autoStrafer->in_autostrafe, (args.ArgC() > 1) ? args[1] : nullptr); }
void IN_AutoStrafeUp(const CCommand& args) { client->KeyUp(&autoStrafer->in_autostrafe, (args.ArgC() > 1) ? args[1] : nullptr); }

Command startautostrafe("+autostrafe", IN_AutoStrafeDown, "Auto-strafe button.\n");
Command endautostrafe("-autostrafe", IN_AutoStrafeUp, "Auto-strafe button.\n");

CON_COMMAND(sar_tas_strafe, "sar_tas_strafe <type> <direction> : Automatic strafing.\n"
                            "Type: 0 = off, 1 = straight, 2 = turning and keeping velocity, 3 = turning with velocity gain.\n"
                            "Direction: -1 = left, 1 = right.\n")
{
    IGNORE_DEMO_PLAYER();

    auto type = StrafingType::Straight;
    auto direction = -1;

    if (args.ArgC() == 2) {
        type = static_cast<StrafingType>(std::atoi(args[1]));
    } else if (args.ArgC() == 3) {
        type = static_cast<StrafingType>(std::atoi(args[1]));
        direction = std::atoi(args[2]);
    } else {
        return console->Print(sar_tas_strafe.ThisPtr()->m_pszHelpString);
    }

    auto nSlot = GET_SLOT();
    autoStrafer->states[nSlot]->type = type;
    autoStrafer->states[nSlot]->direction = direction;
}
CON_COMMAND(sar_tas_strafe_vectorial, "sar_tas_strafe_vectorial <type>: Change type of vectorial strafing.\n"
                                      "0 = Auto-strafer calculates perfect viewangle,\n"
                                      "1 = Auto-strafer calculates perfect forward-side movement,\n"
                                      "2 = Auto-strafer calculates perfect forward-side movement, "
                                      "while setting the viewangle toward current velocity, to make strafing visually visible.\n")
{
    IGNORE_DEMO_PLAYER();

    auto type = VecStrafeType::Disabled;

    if (args.ArgC() == 2) {
        type = static_cast<VecStrafeType>(std::atoi(args[1]));
    } else {
        return console->Print(sar_tas_strafe_vectorial.ThisPtr()->m_pszHelpString);
    }

    auto nSlot = GET_SLOT();
    autoStrafer->states[nSlot]->vecType = type;
}
