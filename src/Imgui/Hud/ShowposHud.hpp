#pragma once

#include "ImguiHudPanel.hpp"

class ShowposHud : public ImguiHudPanel {
public:
  ShowposHud() {
    mSnapState.anchorX = Edge_Left;
    mSnapState.anchorY = Edge_Top;
  }

  void DrawContent() override;
  void ContextMenu() override;

  bool ShouldDraw() override;

  const char* GetName() const { return "Showpos"; }
  const char* GetHandle() const { return "showpos"; }
};
