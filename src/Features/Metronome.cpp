#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Variable.hpp"
#include "Utils.hpp"

#include <thread>

Variable sar_metronome("sar_metronome", "0", "Enable metronome.\n");
Variable sar_metronome_beats("sar_metronome_beats", "4", 1, 16, "Set the number of beats per bar for the metronome.\n");
Variable sar_metronome_bpm("sar_metronome_bpm", "60", 1, 1000, "Set the beats per minute for the metronome.\n");
Variable sar_metronome_sound("sar_metronome_sound", "ui/ui_coop_hud_focus_01", "Set the sound to play for the metronome.\n");
Variable sar_metronome_sound_bar("sar_metronome_sound_bar", "ui/ui_coop_hud_unfocus_01", "Set the sound to play for the metronome bar.\n");
Variable sar_metronome_volume("sar_metronome_volume", "1", 0, 1, "Set the volume for the metronome.\n");

static std::thread metronomeThread;
static int metronomeBeat = 0;

static void bpmTick() {
    auto interval = std::chrono::milliseconds(1000);
    if (sar_metronome.GetBool()) {
        auto isBar = metronomeBeat == 0;
        metronomeBeat = (metronomeBeat + 1) % sar_metronome_beats.GetInt();
        auto sound = isBar ? sar_metronome_sound_bar.GetString() : sar_metronome_sound.GetString();
        engine->ExecuteCommand(Utils::ssprintf("playvol \"%s\" %f", sound, sar_metronome_volume.GetFloat()).c_str());
        if (sar_metronome_bpm.GetFloat() > 0) {
            interval = std::chrono::milliseconds((int)(60000 / sar_metronome_bpm.GetFloat()));
        }
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
