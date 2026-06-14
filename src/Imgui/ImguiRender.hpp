#pragma once

#include <imgui.h>

// I don't like this, but whatever
extern bool g_drawImgui;

extern void InitImgui();
extern void DrawImgui();
extern void ImGui_RenderDrawData_Source(ImDrawData* drawData);
