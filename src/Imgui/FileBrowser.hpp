#pragma once

#include <vector>
#include <filesystem>

struct FileBrowserOptions {
  std::string title = "File Browser";
  bool multiSelect = false;
  bool selectDirectories = false;
  std::filesystem::path startPath = std::filesystem::current_path();
};

class FileBrowser {
public:
  static void Show(const FileBrowserOptions& options);

  static bool HasResults();
  static std::vector<std::filesystem::path> GetResults();

  static void Render();

private:
  static bool IsSelected(const std::filesystem::path& path);

private:
  inline static bool sOpen = false;
  inline static bool sFinished = false;
  
  inline static FileBrowserOptions sOptions;
  inline static std::filesystem::path sCurrentPath;
  inline static std::vector<std::filesystem::path> sSelection;
};

