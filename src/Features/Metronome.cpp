#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Variable.hpp"
#include "Utils.hpp"

#include <thread>

Variable sar_metronome("sar_metronome", "0", "Enable metronome.\n");
Variable sar_metronome_bpm("sar_metronome_bpm", "60", 1, 1000, "Set the beats per minute for the metronome.\n");
Variable sar_metronome_sound("sar_metronome_sound", "ui/ui_coop_hud_focus_01", "Set the sound to play for the metronome.\n");

static std::thread metronomeThread;

static void bpmTick() {
    if (sar_metronome.GetBool()) {
        engine->ExecuteCommand(Utils::ssprintf("playvol \"%s\" 1", sar_metronome_sound.GetString()).c_str());
    }
    auto interval = std::chrono::milliseconds(1000);
    if (sar_metronome_bpm.GetFloat() > 0) {
        interval = std::chrono::milliseconds((int)(60000 / sar_metronome_bpm.GetFloat()));
    }
    std::this_thread::sleep_for(interval);
    bpmTick();
}

ON_INIT {
    metronomeThread = std::thread(bpmTick);
}

ON_EVENT(SAR_UNLOAD) {
    if (metronomeThread.joinable()) metronomeThread.detach();
}
