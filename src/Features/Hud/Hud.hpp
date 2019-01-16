#pragma once
#include "Utils/SDK.hpp"

#include "Variable.hpp"

class Hud {
protected:
    Color GetColor(const char* source);

public:
    virtual ~Hud() = default;
    virtual bool GetCurrentSize(int& xSize, int& ySize) = 0;
    virtual void Draw() = 0;
};

extern Variable sar_hud_text;
extern Variable sar_hud_position;
extern Variable sar_hud_angles;
extern Variable sar_hud_velocity;
extern Variable sar_hud_session;
extern Variable sar_hud_last_session;
extern Variable sar_hud_sum;
extern Variable sar_hud_timer;
extern Variable sar_hud_pause_timer;
extern Variable sar_hud_avg;
extern Variable sar_hud_cps;
extern Variable sar_hud_demo;
extern Variable sar_hud_jumps;
extern Variable sar_hud_portals;
extern Variable sar_hud_steps;
extern Variable sar_hud_jump;
extern Variable sar_hud_jump_peak;
extern Variable sar_hud_trace;
extern Variable sar_hud_frame;
extern Variable sar_hud_last_frame;
extern Variable sar_hud_inspection;
extern Variable sar_hud_velocity_peak;
extern Variable sar_hud_velocity_angle;
extern Variable sar_hud_acceleration;
extern Variable sar_hud_player_info;
extern Variable sar_hud_default_spacing;
extern Variable sar_hud_default_padding_x;
extern Variable sar_hud_default_padding_y;
extern Variable sar_hud_default_font_index;
extern Variable sar_hud_default_font_color;
