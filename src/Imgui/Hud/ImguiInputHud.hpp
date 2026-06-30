#pragma once

#include "ImguiHudPanel.hpp"

class ImguiInputHud : public ImguiHudPanel {
public:
  ImguiInputHud() {
    mSnapState.anchorX = Edge_Left;
    mSnapState.anchorY = Edge_Bottom;
  }

  void DrawContent() override;

  bool ShouldDraw() override;

  const char* GetName() const { return "Input Hud"; }
  const char* GetHandle() const { return "ihud"; }
};
