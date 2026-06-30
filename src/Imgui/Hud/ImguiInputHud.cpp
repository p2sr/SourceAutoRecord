#include "ImguiInputHud.hpp"

#include "Modules/Engine.hpp"

#include <imgui.h>

bool ImguiInputHud::ShouldDraw() {
  // Only show in game
  if (!engine->hoststate->m_activeGame && !engine->demoplayer->IsPlaying()) {
    return false;
  } else {
    return true;
  }
}

void ImguiInputHud::DrawContent() {
  ImGui::Text("This is the input hud");
}
