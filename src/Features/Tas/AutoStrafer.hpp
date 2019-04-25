#pragma once
#include <vector>

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

enum class VecStrafeType {
    Disabled,
    Normal,
    Visual
};
struct StrafeState {
    int direction = 1;
    StrafingType type = StrafingType::None;
    VecStrafeType vecType = VecStrafeType::Visual;
};

class AutoStrafer : public Feature {
public:
    kbutton_t in_autostrafe;
    std::vector<StrafeState*> states;

public:
    AutoStrafer();
    ~AutoStrafer();

    void Strafe(void* pPlayer, CMoveData* pMove);

private:
    float GetStrafeAngle(const StrafeState* strafe, void* pPlayer, const CMoveData* pMove);
};

extern AutoStrafer* autoStrafer;

extern Command sar_tas_strafe;
extern Command startautostrafe;
extern Command endautostrafe;
