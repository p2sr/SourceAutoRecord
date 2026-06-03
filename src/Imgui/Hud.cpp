#include "Hud.hpp"

#include "ImguiWindow.hpp"
#include "Modules/Engine.hpp"

#include <imgui.h>

#include <string>
#include "Modules/VGui.hpp"

static bool background = false;
static int position = 0;

void GetConVars() {
  background = sar_hud_bg.GetBool();
  // position = sar_hud_position
}

void UpdateConVar(const std::string& convar, int val) {
  std::string cmd{convar + " " + std::to_string(val)};
  engine->ExecuteCommand(cmd.c_str());
}

void AddHud() {
  imguiWindows.push_back({
    ImguiWindowCategory::Hud,
    "Hud",
    "hud",
    false,
    {-1, -1},
    []() {
      GetConVars();

      ImGui::SeparatorText("Hud Settings");
      if (ImGui::Checkbox("Background", &background)) {
        UpdateConVar("sar_hud_bg", background);
      }

      {
        ImGui::SeparatorText("Position");
        const char* items[] = {
          "Default",
          "Player Position",
          "Camera (shoot) position",
        };
      }
    }
  });
}

