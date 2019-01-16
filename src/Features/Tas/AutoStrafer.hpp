#pragma once
#include "Features/Feature.hpp"

#include "Utils/SDK.hpp"

#include "Command.hpp"
#include "Variable.hpp"

#define IN_AUTOSTRAFE (1 << 30)

enum class StrafingType {
    None,
    Straight,
    Turning
};

struct StrafeState {
    int direction = 1;
    StrafingType type = StrafingType::None;
};

class AutoStrafer : public Feature {
public:
    kbutton_t in_autostrafe;
    StrafeState states[MAX_SPLITSCREEN_PLAYERS];

public:
    AutoStrafer();
    void Strafe(void* pPlayer, CMoveData* pMove);

private:
    float GetStrafeAngle(const StrafeState* strafe, void* pPlayer, const CMoveData* pMove);
};

extern AutoStrafer* autoStrafer;

extern Command sar_tas_strafe;
extern Command startautostrafe;
extern Command endautostrafe;

extern Variable sar_tas_strafe_vectorial;
