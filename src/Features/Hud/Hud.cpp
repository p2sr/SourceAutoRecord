#include "Hud.hpp"

#include <cstdio>

#include "Variable.hpp"

Color Hud::GetColor(const char* source)
{
    int r, g, b, a;
    std::sscanf(source, "%i%i%i%i", &r, &g, &b, &a);
    return Color(r, g, b, a);
}

Variable sar_hud_text("sar_hud_text", "", "Draws specified text when not empty.\n", 0);
Variable sar_hud_position("sar_hud_position", "0", 0, "Draws absolute position of the client.\n");
Variable sar_hud_angles("sar_hud_angles", "0", 0, "Draws absolute view angles of the client.\n"
                                                  "0 = default,\n"
                                                  "1 = xy,\n"
                                                  "2 = xyz.\n");
Variable sar_hud_velocity("sar_hud_velocity", "0", 0, "Draws velocity of the client.\n"
                                                      "0 = default,\n"
                                                      "1 = x/y/z,\n"
                                                      "2 = x/y\n");
Variable sar_hud_session("sar_hud_session", "0", "Draws current session tick.\n");
Variable sar_hud_last_session("sar_hud_last_session", "0", "Draws value of latest completed session.\n");
Variable sar_hud_sum("sar_hud_sum", "0", "Draws summary value of sessions.\n");
Variable sar_hud_timer("sar_hud_timer", "0", "Draws current value of timer.\n");
Variable sar_hud_avg("sar_hud_avg", "0", "Draws calculated average of timer.\n");
Variable sar_hud_cps("sar_hud_cps", "0", "Draws latest checkpoint of timer.\n");
Variable sar_hud_demo("sar_hud_demo", "0", "Draws name, tick and time of current demo.\n");
Variable sar_hud_jumps("sar_hud_jumps", "0", "Draws total jump count.\n");
Variable sar_hud_portals("sar_hud_portals", "0", "Draws total portal count.\n");
Variable sar_hud_steps("sar_hud_steps", "0", "Draws total step count.\n");
Variable sar_hud_jump("sar_hud_jump", "0", "Draws current jump distance.\n");
Variable sar_hud_jump_peak("sar_hud_jump_peak", "0", "Draws longest jump distance.\n");
Variable sar_hud_trace("sar_hud_trace", "0", 0, "Draws distance values of tracer. "
                                                "0 = default,\n"
                                                "1 = vec3,\n"
                                                "2 = vec2.\n");
Variable sar_hud_frame("sar_hud_frame", "0", "Draws current frame count.\n");
Variable sar_hud_last_frame("sar_hud_last_frame", "0", "Draws last saved frame value.\n");
Variable sar_hud_inspection("sar_hud_inspection", "0", "Draws entity inspection data.\n");
Variable sar_hud_velocity_peak("sar_hud_velocity_peak", "0", "Draws last saved velocity peak.\n");
Variable sar_hud_default_spacing("sar_hud_default_spacing", "4", 0, "Spacing between elements of HUD.\n");
Variable sar_hud_default_padding_x("sar_hud_default_padding_x", "2", 0, "X padding of HUD.\n");
Variable sar_hud_default_padding_y("sar_hud_default_padding_y", "2", 0, "Y padding of HUD.\n");
Variable sar_hud_default_font_index("sar_hud_default_font_index", "0", 0, "Font index of HUD.\n");
Variable sar_hud_default_font_color("sar_hud_default_font_color", "255 255 255 255", "RGBA font color of HUD.\n", 0);
