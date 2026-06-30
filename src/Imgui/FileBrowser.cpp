#include "FileBrowser.hpp"

#include <imgui.h>

#include "Modules/Console.hpp"

#include <algorithm>
#include <vector>

void FileBrowser::Show(const FileBrowserOptions& options) {
  if (!sOpen) {
    sOpen = true;
    sFinished = false;
    sOptions = options;
    sCurrentPath = options.startPath;
    sSelection.clear();
  }
}

bool FileBrowser::HasResults() {
  return sFinished;
}

std::vector<std::filesystem::path> FileBrowser::GetResults() {
  // Can only get results once
  sOpen = false;
  sFinished = false;
  return sSelection;
}

bool FileBrowser::IsSelected(const std::filesystem::path& path) {
  return std::find(
    sSelection.begin(),
    sSelection.end(),
    path
  ) != sSelection.end();
}

void FileBrowser::Render() {
  if (sOpen) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 windowSize{600.0f, 400.0f};

    ImVec2 windowPos{
      viewport->Pos.x + (viewport->Size.x - windowSize.x) * 0.5f,
      viewport->Pos.y + (viewport->Size.y - windowSize.y) * 0.5f,
    };

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);

    ImGui::Begin(sOptions.title.c_str());

    // Header
    ImGui::TextUnformatted(sCurrentPath.string().c_str());

    if (sCurrentPath.has_parent_path()) {
      if (ImGui::Button("..")) {
        sCurrentPath = sCurrentPath.parent_path();
      }
    }

    ImGui::Separator();

    try {
      std::vector<std::filesystem::directory_entry> entries;
      for (const auto& entry : std::filesystem::directory_iterator(sCurrentPath)) {
        entries.push_back(entry);
      }

      std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        if (a.is_directory() != b.is_directory()) {
          return a.is_directory();
        }

        return a.path().filename().string() < b.path().filename().string();
      });

      for (const auto& entry : entries) {
        const auto filename = entry.path().filename().string();

        bool selected = IsSelected(entry.path());

        if (ImGui::Selectable(filename.c_str(), selected)) {

        }
      }
    } catch (...) {
      ImGui::TextUnformatted("Failed to read directory");
    }

    ImGui::Separator();

    if (ImGui::Button("Ok")) {
      sFinished = true;
    }

    ImGui::End();
  }
}

