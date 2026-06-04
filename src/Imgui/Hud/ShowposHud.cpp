#include "ShowposHud.hpp"

#include "Modules/VGui.hpp"
#include "Modules/Server.hpp"
#include "Modules/Engine.hpp"

#include "Features/Hud/Hud.hpp"

#include <imgui.h>

static inline int getPrecision(bool velocity = false) {
	int p = velocity ? sar_hud_velocity_precision.GetInt() : sar_hud_precision.GetInt();
	if (p < 0) p = 0;
	if (!sv_cheats.GetBool()) {
		const int max = velocity ? 2 : 6;
		if (p > max) p = max;
	}
	return p;
}

bool ShowposHud::ShouldDraw() {
  // Only show in game
  if (!engine->hoststate->m_activeGame && !engine->demoplayer->IsPlaying()) {
    return false;
  } else {
    return true;
  }
  return false;
}

void ShowposHud::ContextMenu() {
  if (ImGui::Button("Settings \xef\x85\x88")) {
    engine->ExecuteCommand("sar_imgui_toggle_window hudsettings");
  }
}

void ShowposHud::DrawContent() {
  ImVec2 pos = ImGui::GetWindowPos();
  ImVec2 size = ImGui::GetWindowSize();

  // Background
  if (sar_hud_bg.GetBool()) {
    ImVec2 p0 = pos;
    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);
    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddRectFilled(p0, p1, IM_COL32(0, 0, 0, 200));
  }

  // Position
  auto player = client->GetPlayer(engine->GetLocalPlayerIndex());
  if (player && sar_hud_position.GetInt()) {
    auto pos = client->GetAbsOrigin(player);
		if (sar_hud_position.GetInt() >= 2) {
			pos = pos + client->GetViewOffset(player) + client->GetPortalLocal(player).m_vEyeOffset;
		}
    int p = getPrecision();
    ImGui::Text("pos: %.*f %.*f %.*f", p, pos.x, p, pos.y, p, pos.z);
  } else {
    if (g_imguiEditMode && g_drawImgui && !engine->hoststate->m_activeGame) {
      ImGui::Text("pos: -");
    }
  }

  // Angles
  if (player && sar_hud_angles.GetInt()) {
    auto ang = engine->GetAngles(engine->IsOrange() ? 0 : engine->GetLocalPlayerIndex() - 1);
    int p = getPrecision();
    int mode = sar_hud_angles.GetInt();
    if (mode == 1) {
      ImGui::Text("ang: %.*f %.*f", p, ang.x, p, ang.y);
    } else if (mode == 2) {
      ImGui::Text("ang: %.*f %.*f %.*f", p, ang.x, p, ang.y, p, ang.z);
    } else if (mode == 3) {
      ImGui::Text("ang: %.*f", p, ang.x);
    } else {
      ImGui::Text("ang: %.*f", p, ang.y);
    }
  } else {
    if (g_imguiEditMode && g_drawImgui && !engine->hoststate->m_activeGame) {
      ImGui::Text("ang: -");
    }
  }

  if (player && sar_hud_velocity.GetInt()) {
		int p = getPrecision(true);
		auto vel = client->GetLocalVelocity(player);
		switch (sar_hud_velocity.GetInt()) {
		case 1:
			ImGui::Text("vel: %.*f %.*f %.*f", p, vel.x, p, vel.y, p, vel.z);
			break;
		case 2:
			ImGui::Text("vel: %.*f", p, vel.Length2D());
			break;
		case 3:
			ImGui::Text("vel: %.*f %.*f", p, vel.Length2D(), p, vel.z);
			break;
		case 4:
			ImGui::Text("vel: %.*f", p, vel.Length());
			break;
		case 5:
			ImGui::Text("vel: %.*f %.*f (%.*f) %.*f", p, vel.x, p, vel.y, p, vel.Length2D(), p, vel.z);
			break;
		}
  } else {
    if (g_imguiEditMode && g_drawImgui && !engine->hoststate->m_activeGame) {
      ImGui::Text("vel: -");
    }
  }
  
}
