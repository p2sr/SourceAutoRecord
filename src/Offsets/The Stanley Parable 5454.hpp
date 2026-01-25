#include "Offsets/Beginners Guide 6167.hpp"

// CInputSystem
OFFSET_WINDOWS(GetRawMouseAccumulators, 50)

// Others
OFFSET_LINUX(VideoMode_Create, 104)
OFFSET_LINUX(StartDrawing, 0) // these are why the plugin crashes on linux
OFFSET_LINUX(FinishDrawing, 0)

// clang-format off

// Renderer
SIGSCAN_DEFAULT(SND_RecordBuffer, "55 8B EC 80 3D ? ? ? ? 00 53 56 57",
                                  "55 89 E5 57 56 53 83 EC 3C 65 A1 ? ? ? ? 89 45 ? 31 C0 E8 ? ? ? ? 84 C0")


// Client
SIGSCAN_WINDOWS(MatrixBuildRotationAboutAxis, "55 8B EC 51 F3 0F 10 45 ? 0F 5A C0 F2 0F 59 05 ? ? ? ? 66 0F 5A C0 F3 0F 11 45 ? E8 ? ? ? ? F3 0F 11 45 ? F3 0F 10 45 ? E8 ? ? ? ? 8B 45 ? F3 0F 10 08")
SIGSCAN_WINDOWS(DrawTranslucentRenderables, "55 8B EC 81 EC 80 00 00 00 53 56 8B F1 8B 0D")
SIGSCAN_DEFAULT(DrawOpaqueRenderables, "55 8B EC 83 EC 5C 83 7D ? 00 A1",
                                       "55 89 E5 57 56 53 81 EC 9C 00 00 00 8B 45 ? 8B 5D ? 89 85")

// clang-format on
