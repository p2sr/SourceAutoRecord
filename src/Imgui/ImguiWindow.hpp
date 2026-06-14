#pragma once

#include <vector>
#include <imgui.h>

enum class ImguiWindowCategory {
  Sar,
  Hud
};

struct ImguiWindow {
  ImguiWindowCategory category = ImguiWindowCategory::Sar;
  const char* name;
  const char* internalName;
  bool open = false;

  ImVec2 windowSize = {-1, -1};

  void (*render)();
};

extern std::vector<ImguiWindow> imguiWindows;

