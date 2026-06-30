#include "ToastPos.hpp"
#include "Modules/Engine.hpp"

#include <imgui.h>
#include <math.h>

enum class Direction {
  Up,
  Down,
  Left,
  Right,
  UpLeft,
  UpRight,
  DownLeft,
  DownRight
};

float degreesToRadians(float degrees) {
  return degrees * M_PI / 180;
}

void DrawArrow(ImDrawList* dl, ImVec2 center, float size, ImU32 col, Direction dir) {
  ImVec2 tip, left, right;
  float angle = 0.f;

  switch(dir) {
    case Direction::Right: angle = 0.f; break;
    case Direction::UpRight: angle = 45.f; break;
    case Direction::Up: angle = 90.f; break;
    case Direction::UpLeft: angle = 135.f; break;
    case Direction::Left: angle = 180.f; break;
    case Direction::DownLeft: angle = 225.f; break;
    case Direction::Down: angle = 270.f; break;
    case Direction::DownRight: angle = 315.f; break;
    default: angle = 0.f; break;
  }

  // angle = 0.f;
  float k = 0.75f;

  tip = {
    center.x + std::cos(degreesToRadians(angle)) * size * k,
    center.y - std::sin(degreesToRadians(angle)) * size * k
  };
  left = {
    center.x + std::cos(degreesToRadians(angle + 120)) * size,
    center.y - std::sin(degreesToRadians(angle + 120)) * size
  };
  right = {
    center.x + std::cos(degreesToRadians(angle + 240)) * size,
    center.y - std::sin(degreesToRadians(angle + 240)) * size
  };

  dl->AddTriangleFilled(tip, left, right, col);
}

void DrawToastPos() {
  ImGui::Begin("Toasts Position");

  const char* labels[3][3] = {
    {"##up_left", "↑", "↗"},
    {"↙", "↓", "↘"}
  };

  int size = 40;
  int i = 0;
  for (int y = 0; y < 2; y++) {
    for (int x = 0; x < 3; x++) {
      Direction arrowDir;
      if (ImGui::Button(labels[y][x], ImVec2(size, size))) {
        switch (i) {
          // Top
          case 0: engine->ExecuteCommand("sar_toast_setpos top left"); break;
          case 1: engine->ExecuteCommand("sar_toast_setpos top center"); break;
          case 2: engine->ExecuteCommand("sar_toast_setpos top right"); break;
          // Bottom
          case 3: engine->ExecuteCommand("sar_toast_setpos bottom left"); break;
          case 4: engine->ExecuteCommand("sar_toast_setpos bottom center"); break;
          case 5: engine->ExecuteCommand("sar_toast_setpos bottom right"); break;
        }
      }

      switch (i) {
          // Top
          case 0: arrowDir = Direction::UpLeft; break;
          case 1: arrowDir = Direction::Up; break;
          case 2: arrowDir = Direction::UpRight; break;
          // Bottom
          case 3: arrowDir = Direction::DownLeft; break;
          case 4: arrowDir = Direction::Down; break;
          case 5: arrowDir = Direction::DownRight; break;
      }

      ImDrawList* dl = ImGui::GetWindowDrawList();
      ImVec2 p0 = ImGui::GetItemRectMin();
      ImVec2 p1 = ImGui::GetItemRectMax();
      ImVec2 c((p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f);
      DrawArrow(dl, c, 10.0f, IM_COL32_WHITE, arrowDir);

      if (x < 2) {
        ImGui::SameLine();
      }

      i++;
    }
  }

  ImGui::End();
}

