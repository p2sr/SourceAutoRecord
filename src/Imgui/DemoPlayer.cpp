#include "DemoPlayer.hpp"

#include "Modules/Console.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Server.hpp"
#include "Features/Camera.hpp"

#include <imgui.h>

#include <string>

#include "FileBrowser.hpp"

static float playbackSpeed = 1.0f;

static char input[256] = "";

static bool g_keysDown[512] = {};
static bool g_keysPressed[512] = {};
static bool g_keysReleased[512] = {};

static float maxSpeed = 10.0f;

float ValueToT(float v) {
  if (v <= 1.0f)
    return 0.5f * (v / 1.0f);
  else
    return 0.5f + 0.5f * ((v - 1.0f) / (maxSpeed - 1.0f));
}
float TToValue(float t) {
  if (t <= 0.5f)
    return (t / 0.5f) * 1.0f;
  else
    return 1.0f + ((t - 0.5f) / 0.5f) * (maxSpeed - 1.0f);
}

void AddDemoPlayer() {
  imguiWindows.push_back({
    ImguiWindowCategory::Sar,
    "Demo Player",
    "demoplayer",
    false,
    {350, -1},
    []() {
      static bool follow = true;
      // First line
      if (ImGui::Button("Load...")) {
        follow = true;
        FileBrowser::Show({});
      }

      if (FileBrowser::HasResults()) {
        auto files = FileBrowser::GetResults();
        for (auto& f : files) {
          console->Print("%s", f.string().c_str());
        }
      }
      
      float avail = ImGui::GetContentRegionAvail().x;

      ImGui::SameLine();

      const char* stopLabel = "Stop";
      ImVec2 rightSize = ImGui::CalcTextSize(stopLabel);
      rightSize.x += ImGui::GetStyle().FramePadding.x;
      
      ImGui::SetCursorPosX((avail - rightSize.x));

      if (ImGui::Button(stopLabel)) {
        engine->ExecuteCommand("disconnect");
      }

      EngineDemoPlayer* demoPlayer = engine->demoplayer;

      bool isPlaying = demoPlayer->IsPlaying();

      if (demoPlayer) {
        if (demoPlayer->DemoName) {
          ImGui::Text("%s", demoPlayer->DemoName);
        }
      }

      // Second line
      int tick = 0;
      int totalTicks = 0;
      float time = 0.0f;
      float totalTime = 0.0f;

      if (demoPlayer) {
        tick = demoPlayer->GetTick();
        totalTicks = demoPlayer->demoPlaybackTicks;
        time = engine->ToTime(tick);
        totalTime = engine->ToTime(totalTicks);
      }

      if (isPlaying) {
        ImGui::Text("%.3f/%.3f", time, totalTime);
      } else {
        ImGui::Text("0.000/0.000");
      }

      ImGui::SameLine();
      avail = ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemWidth(avail);

      float t = ValueToT(playbackSpeed);

      bool changed = ImGui::SliderFloat("##Speed", &t, 0.0f, 1.0f, "", ImGuiSliderFlags_None);

      if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        ImGui::SetKeyboardFocusHere(-1);
      }

      if (changed) {
        playbackSpeed = TToValue(t);
      }

      float snapTarget = 1.0f;
      float snapRange = 0.05f;

      if (ImGui::IsItemActive()) {
        if (fabs(playbackSpeed - snapTarget) <= snapRange) {
          playbackSpeed = snapTarget;
        }
        // console->Print("New playback speed: %.3f\n", playbackSpeed);
        std::string cmd{"demo_timescale " + std::to_string(playbackSpeed)};
        // console->Print("command: %s\n", cmd.c_str());
        engine->ExecuteCommand(cmd.c_str());
      }

      char overlay[64];

      snprintf(overlay, sizeof(overlay), "%.0f%%", playbackSpeed * 100.0f);

      ImVec2 min = ImGui::GetItemRectMin();
      ImVec2 max = ImGui::GetItemRectMax();

      ImVec2 textSize = ImGui::CalcTextSize(overlay);

      // center inside slider
      ImVec2 pos(
        min.x + (max.x - min.x - textSize.x) * 0.5f,
        min.y + (max.y - min.y - textSize.y) * 0.5f
      );

      ImDrawList* drawList = ImGui::GetWindowDrawList();
      drawList->AddText(pos, ImGui::GetColorU32(ImGuiCol_Text), overlay);

      // ImGui::TextUnformatted(overlay);

      // Timeline
      static float scrub = 0.0f;
      static int pendingTick = -1;

      float progress = (totalTicks > 0)
        ? (float)tick / (float)totalTicks
        : 0.0f;
      
      if (follow) {
        scrub = progress;
      }

      avail = ImGui::GetContentRegionAvail().x;
      ImGui::SetNextItemWidth(
        avail - ImGui::CalcTextSize("Goto").x - ImGui::GetStyle().FramePadding.x
      );
      if (ImGui::SliderFloat("##Timeline", &scrub, 0.0f, 1.0f, "")) {
        follow = false;
      }

      snprintf(overlay, sizeof(overlay), "%.3f", scrub * totalTime);

      min = ImGui::GetItemRectMin();
      max = ImGui::GetItemRectMax();

      textSize = ImGui::CalcTextSize(overlay);

      // center inside slider
      pos = ImVec2(
        min.x + (max.x - min.x - textSize.x) * 0.5f,
        min.y + (max.y - min.y - textSize.y) * 0.5f
      );

      drawList->AddText(pos, ImGui::GetColorU32(ImGuiCol_Text), overlay);

      int seekTick = (int)(scrub * totalTicks);

      ImGui::SameLine();

      if (ImGui::Button("Goto")) {
        std::string cmd{"demo_gototick " + std::to_string(seekTick)};
        engine->ExecuteCommand(cmd.c_str());
        follow = true;
      }

      if (ImGui::Button("<")) {
        std::string cmd{"demo_gototick " + std::to_string(tick - 1)};
        engine->ExecuteCommand(cmd.c_str());
      }

      ImGui::SameLine();

      char* playerButtonLabel = "Play";
      bool isPaused = false;
      if (demoPlayer) {
        if (demoPlayer->IsPaused()) {
          isPaused = true;
          playerButtonLabel = "Resume";
        } else {
          isPaused = false;
          playerButtonLabel = "Pause";
        }
      }
      if (ImGui::Button(playerButtonLabel)) {
        if (isPaused) {
          engine->ExecuteCommand("demo_resume");
        } else {
          engine->ExecuteCommand("demo_pause");
        }
      }

      ImGui::SameLine();

      if (ImGui::Button(">")) {
        std::string cmd{"demo_gototick " + std::to_string(tick + 1)};
        engine->ExecuteCommand(cmd.c_str());
      }

      if (demoPlayer) {
        if (isPlaying) {
          ImGui::Text("Ticks: %d/%d", tick, totalTicks);
        } else {
          ImGui::Text("Ticks: 0/0");
        }
      }

      bool camControlActive = sar_cam_control.GetInt();
      if (ImGui::Checkbox("Cam Control", &camControlActive)) {
        if (camControlActive) {
          engine->ExecuteCommand("sar_cam_control 1");
          engine->ExecuteCommand("sar_cam_drive 1");
        } else {
          // Defaults (why?)
          engine->ExecuteCommand("sar_cam_control 0");
          engine->ExecuteCommand("sar_cam_drive 2");
        }
      }

      bool svAltTicks = sv_alternateticks.GetBool();
      if (ImGui::Checkbox("Alternate Ticks", &svAltTicks)) {
        if (svAltTicks) {
          engine->ExecuteCommand("sv_alternateticks 1");
        } else {
          engine->ExecuteCommand("sv_alternateticks 0");
        }
      }

      // ImGui::Text("%.3f", demo_timescale.GetFloat());

      bool removeBroken = sar_demo_remove_broken.GetBool();
      if (ImGui::Checkbox("Remove Broken", &removeBroken)) {
        if (removeBroken) {
          engine->ExecuteCommand("sv_alternateticks 1");
          engine->ExecuteCommand("sar_demo_remove_broken 1");
        } else {
          engine->ExecuteCommand("sar_demo_remove_broken 0");
        }
      }
    }
  });
}

// void DrawDemoPlayer() {
//   ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
//   ImGui::Begin("Demo Playback");
//
//   ImGui::End();
// }

