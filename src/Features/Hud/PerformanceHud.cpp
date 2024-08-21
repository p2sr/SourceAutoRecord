#include "PerformanceHud.hpp"

#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Surface.hpp"
#include "Variable.hpp"

#define PERFORMANCE_HUD_BUCKETS 20

Variable sar_performance_hud("sar_performance_hud", "0", "Enables the performance HUD.\n1 = normal,\n2 = stats only.\n");
Variable sar_performance_hud_duration("sar_performance_hud_duration", "60", 1, "How long (in frames) to measure performance for.\n");

Variable sar_performance_hud_x("sar_performance_hud_x", "20", "X position of the performance HUD.\n");
Variable sar_performance_hud_y("sar_performance_hud_y", "300", "Y position of the performance HUD.\n");
Variable sar_performance_hud_font_index("sar_performance_hud_font_index", "6", "Font index of the performance HUD.\n");

bool PerformanceHud::ShouldDraw() {
	return sar_performance_hud.GetBool();
}

void PerformanceHud::Paint(int slot) {
	int x = sar_performance_hud_x.GetInt();
	int y = sar_performance_hud_y.GetInt();
	int font = sar_performance_hud_font_index.GetInt();
	int lineHeight = surface->GetFontHeight(font);

	float min_offTick = FLT_MAX;
	float max_offTick = FLT_MIN;
	float mean_offTick = 0;
	if (this->frametimes_offTick.size() > 0) {
		for (auto &frametime : this->frametimes_offTick) {
			mean_offTick += frametime;
			if (frametime < min_offTick) min_offTick = frametime;
			if (frametime > max_offTick) max_offTick = frametime;
		}
		mean_offTick /= this->frametimes_offTick.size();
		surface->DrawTxt(font, x, y, {255, 255, 255}, "frametime (off tick): min %.3fms, mean %.3fms, max %.3fms", min_offTick * 1000, mean_offTick * 1000, max_offTick * 1000);
	}

	float min_onTick = FLT_MAX;
	float max_onTick = FLT_MIN;
	float mean_onTick = 0;
	if (this->frametimes_onTick.size() > 0) {
		for (auto &frametime : this->frametimes_onTick) {
			mean_onTick += frametime;
			if (frametime < min_onTick) min_onTick = frametime;
			if (frametime > max_onTick) max_onTick = frametime;
		}
		mean_onTick /= this->frametimes_onTick.size();
		surface->DrawTxt(font, x, y + lineHeight, {255, 255, 255}, "frametime (on tick):  min %.3fms, mean %.3fms, max %.3fms", min_onTick * 1000, mean_onTick * 1000, max_onTick * 1000);
	} else {
		min_onTick = min_offTick;
		max_onTick = max_offTick;
		mean_onTick = mean_offTick;
	}

	if (sar_performance_hud.GetInt() > 1) return;

	int buckets = PERFORMANCE_HUD_BUCKETS;

	// some stupid statistics to get a rough cutoff for outliers
	// makes graph more readable
	float faux_iqr_onTick = (std::min)(mean_onTick - min_onTick, max_onTick - mean_onTick) * 0.5f;
	float faux_iqr_offTick = (std::min)(mean_offTick - min_offTick, max_onTick - mean_offTick) * 0.5f;
	float faux_iqr = (std::max)(faux_iqr_onTick, faux_iqr_offTick);
	max_onTick = (std::max)(mean_offTick, mean_onTick) + faux_iqr * 3.0f;
	min_onTick = (std::min)(0.0f, (std::min)(mean_offTick, mean_onTick) - faux_iqr * 3.0f);


	std::vector<int> buckets_offTick(buckets);
	std::vector<int> buckets_onTick(buckets);
	for (auto &frametime : this->frametimes_offTick) {
		int bucket = (int)((frametime - min_onTick) * buckets / (max_onTick - min_onTick));
		if (bucket < 0) bucket = 0;
		if (bucket >= buckets) bucket = buckets - 1;
		buckets_offTick[bucket]++;
	}
	for (auto &frametime : this->frametimes_onTick) {
		int bucket = (int)((frametime - min_onTick) * buckets / (max_onTick - min_onTick));
		if (bucket < 0) bucket = 0;
		if (bucket >= buckets) bucket = buckets - 1;
		buckets_onTick[bucket]++;
	}

	// draw buckets below text
	int width = surface->GetFontLength(font, "frametime (on tick):  min 33.333ms");  // about half the length of the text
	for (int i = 0; i < buckets; i++) {
		int left = x + i * width / buckets;
		int right = x + (i + 1) * width / buckets;
		if (this->frametimes_onTick.size() != 0) {
			int height_onTick = buckets_onTick[i] * 100 / this->frametimes_onTick.size();
			surface->DrawRect({0, 255, 0, 255}, left, y + 2 * lineHeight, right, y + 2 * lineHeight + 2 + height_onTick);
		}
		if (this->frametimes_offTick.size() != 0) {
			int height_offTick = buckets_offTick[i] * 100 / this->frametimes_offTick.size();
			surface->DrawRect({255, 0, 0, 255}, left, y + 2 * lineHeight, right, y + 2 * lineHeight + 2 + height_offTick);
		}
	}
}

void PerformanceHud::OnFrame(float frametime) {
	if (!sar_performance_hud.GetBool()) {
		if (this->frametimes_offTick.size() > 0) {
			this->frametimes_offTick.clear();
		}
		if (this->frametimes_onTick.size() > 0) {
			this->frametimes_onTick.clear();
		}
		return;
	}

	if (this->accum_ticks > 0) {
		// TODO: in sp this will be render + 2x tick (accum_ticks = 2)
		// in mp it's render + tick. maybe account for that?
		this->frametimes_onTick.push_back(frametime);
		while (this->frametimes_onTick.size() > (unsigned)sar_performance_hud_duration.GetInt()) {
			this->frametimes_onTick.erase(this->frametimes_onTick.begin());
		}
		this->accum_ticks = 0;
	} else {
		this->frametimes_offTick.push_back(frametime);
		while (this->frametimes_offTick.size() > (unsigned)sar_performance_hud_duration.GetInt()) {
			this->frametimes_offTick.erase(this->frametimes_offTick.begin());
		}
	}
}

ON_EVENT(PRE_TICK) {
	if (!sar_performance_hud.GetBool()) {
		performanceHud.accum_ticks = 0;
		return;
	}
	performanceHud.accum_ticks++;
}

CON_COMMAND(sar_performance_hud_clear, "Clears the performance HUD data.\n") {
	performanceHud.frametimes_offTick.clear();
	performanceHud.frametimes_onTick.clear();
}

PerformanceHud performanceHud;
