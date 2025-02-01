#include "Offsets/Beginners Guide 6167.hpp"

// CInputSystem
OFFSET_WINDOWS(GetRawMouseAccumulators, 50)

// clang-format off

// Renderer
SIGSCAN_WINDOWS(SND_RecordBuffer, "55 8B EC 80 3D ? ? ? ? 00 53 56 57")


// Client
SIGSCAN_WINDOWS(MatrixBuildRotationAboutAxis, "55 8B EC 51 F3 0F 10 45 ? 0F 5A C0 F2 0F 59 05 ? ? ? ? 66 0F 5A C0 F3 0F 11 45 ? E8 ? ? ? ? F3 0F 11 45 ? F3 0F 10 45 ? E8 ? ? ? ? 8B 45 ? F3 0F 10 08")
SIGSCAN_WINDOWS(DrawTranslucentRenderables, "55 8B EC 81 EC 80 00 00 00 53 56 8B F1 8B 0D")
SIGSCAN_WINDOWS(DrawOpaqueRenderables, "55 8B EC 83 EC 5C 83 7D ? 00 A1")

// clang-format on
