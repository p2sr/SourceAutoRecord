#include "PerformanceHud.hpp"

#include "Event.hpp"
#include "Modules/Engine.hpp"
#include "Modules/Surface.hpp"
#include "Variable.hpp"

#include <cfloat>

#define PERFORMANCE_HUD_BUCKETS 20

Variable sar_performance_hud("sar_performance_hud", "0", "Enables the performance HUD.\n1 = normal,\n2 = stats only.\n");
Variable sar_performance_hud_duration("sar_performance_hud_duration", "60", 1, "How long (in frames) to measure performance for.\n");
Variable sar_performance_hud_duration_vgui("sar_performance_hud_duration_vgui", "60", 1, "How long (in frames) to measure performance for each individual VGui element.\n");
Variable sar_performance_hud_sort_vgui("sar_performance_hud_sort_vgui", "1", "Whether to sort the VGui elements from slowest to fastest.\n");

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
	if (this->times_totalframe_offTick.size() > 0) {
		for (auto &frametime : this->times_totalframe_offTick) {
			mean_offTick += frametime;
			if (frametime < min_offTick) min_offTick = frametime;
			if (frametime > max_offTick) max_offTick = frametime;
		}
		mean_offTick /= this->times_totalframe_offTick.size();
		surface->DrawTxt(font, x, y, {255, 255, 255}, "frametime (off tick): min %.3fms, mean %.3fms, max %.3fms", min_offTick * 1000, mean_offTick * 1000, max_offTick * 1000);
		y += lineHeight;
	}

	float min_onTick = FLT_MAX;
	float max_onTick = FLT_MIN;
	float mean_onTick = 0;
	if (this->times_totalframe_onTick.size() > 0) {
		for (auto &frametime : this->times_totalframe_onTick) {
			mean_onTick += frametime;
			if (frametime < min_onTick) min_onTick = frametime;
			if (frametime > max_onTick) max_onTick = frametime;
		}
		mean_onTick /= this->times_totalframe_onTick.size();
		surface->DrawTxt(font, x, y, {255, 255, 255}, "frametime (on tick):  min %.3fms, mean %.3fms, max %.3fms", min_onTick * 1000, mean_onTick * 1000, max_onTick * 1000);
		y += lineHeight;
	} else {
		min_onTick = min_offTick;
		max_onTick = max_offTick;
		mean_onTick = mean_offTick;
	}

	auto drawTimeType = [&](auto type, const char *name) {
		if (type.size() > 0) {
			float min = FLT_MAX;
			float max = FLT_MIN;
			float mean = 0;
			for (auto &frametime : type) {
				mean += frametime;
				if (frametime < min) min = frametime;
				if (frametime > max) max = frametime;
			}
			mean /= type.size();
			surface->DrawTxt(font, x, y, {255, 255, 255}, "%s: min %.3fms, mean %.3fms, max %.3fms", name, min * 1000, mean * 1000, max * 1000);
			y += lineHeight;
		}
	};

	drawTimeType(this->times_render,   "render   ");
	drawTimeType(this->times_vgui,     "vgui     ");
	drawTimeType(this->times_preTick,  "pre tick ");
	drawTimeType(this->times_postTick, "post tick");

	if (sar_performance_hud.GetInt() == 1) {
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
		for (auto &frametime : this->times_totalframe_offTick) {
			int bucket = (int)((frametime - min_onTick) * buckets / (max_onTick - min_onTick));
			if (bucket < 0) bucket = 0;
			if (bucket >= buckets) bucket = buckets - 1;
			buckets_offTick[bucket]++;
		}
		for (auto &frametime : this->times_totalframe_onTick) {
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
			if (this->times_totalframe_onTick.size() != 0) {
				int height_onTick = buckets_onTick[i] * 100 / this->times_totalframe_onTick.size();
				surface->DrawRect({0, 255, 0, 255}, left, y, right, y + 2 + height_onTick);
			}
			if (this->times_totalframe_offTick.size() != 0) {
				int height_offTick = buckets_offTick[i] * 100 / this->times_totalframe_offTick.size();
				surface->DrawRect({255, 0, 0, 255}, left, y, right, y + 2 + height_offTick);
			}
		}
		y += 100;
	}

	std::unordered_map<std::string, float> times_vgui_elements;
	if (this->times_vgui_elements.size() > 0) {
		surface->DrawTxt(font, x, y, {255, 255, 255}, "vgui elements:");
		y += lineHeight;
		for (auto &elem : this->times_vgui_elements) {
			// mean
			float mean = 0;
			for (auto &frametime : elem.second) {
				mean += frametime;
			}
			mean /= elem.second.size();
			times_vgui_elements[elem.first] = mean;
		}

		std::vector<std::pair<std::string, float>> sorted_times_vgui_elements(times_vgui_elements.begin(), times_vgui_elements.end());
		if (sar_performance_hud_sort_vgui.GetBool())
			std::sort(sorted_times_vgui_elements.begin(), sorted_times_vgui_elements.end(), [](auto &a, auto &b) { return a.second > b.second; });

		for (auto &elem : sorted_times_vgui_elements) {
			surface->DrawTxt(font, x, y, {255, 255, 255}, "%s: %.3fms", elem.first.c_str(), elem.second * 1000);
			y += lineHeight;
		}
	}
}

void PerformanceHud::OnFrame(float frametime) {
	if (!sar_performance_hud.GetBool()) {
		if (this->times_totalframe_offTick.size() > 0) {
			this->times_totalframe_offTick.clear();
		}
		return;
	}

	if (this->accum_ticks > 0) {
		// TODO: in sp this will be render + 2x tick (accum_ticks = 2)
		// in mp it's render + tick. maybe account for that?
		this->times_totalframe_onTick.push_back(frametime);
		while (this->times_totalframe_onTick.size() > (unsigned)sar_performance_hud_duration.GetInt()) {
			this->times_totalframe_onTick.erase(this->times_totalframe_onTick.begin());
		}
		this->accum_ticks = 0;
	} else {
		this->times_totalframe_offTick.push_back(frametime);
		while (this->times_totalframe_offTick.size() > (unsigned)sar_performance_hud_duration.GetInt()) {
			this->times_totalframe_offTick.erase(this->times_totalframe_offTick.begin());
		}
	}
}

void PerformanceHud::AddMetric(std::vector<float> &type, float time) {
	if (!sar_performance_hud.GetBool()) {
		if (type.size() > 0) {
			type.clear();
		}
		return;
	}
	type.push_back(time);
	while (type.size() > (unsigned)sar_performance_hud_duration.GetInt()) {
		type.erase(type.begin());
	}
}

void PerformanceHud::AddVGuiMetric(std::string type, float time) {
	if (!sar_performance_hud.GetBool()) {
		if (this->times_vgui_elements.size() > 0) {
			this->times_vgui_elements.clear();
		}
		return;
	}
	this->times_vgui_elements[type].push_back(time);
	while (this->times_vgui_elements[type].size() > (unsigned)sar_performance_hud_duration_vgui.GetInt()) {
		this->times_vgui_elements[type].erase(this->times_vgui_elements[type].begin());
	}
}

static std::chrono::high_resolution_clock::time_point preTick;
static std::chrono::high_resolution_clock::time_point postTick;

ON_EVENT_P(PRE_TICK, 2147483647) {
	if (!sar_performance_hud.GetBool()) {
		performanceHud->accum_ticks = 0;
		return;
	}
	performanceHud->accum_ticks++;
	preTick = NOW();
}

ON_EVENT_P(PRE_TICK, -2147483647) {
	float frametime = std::chrono::duration_cast<std::chrono::microseconds>(NOW() - preTick).count() / 1000000.0f;
	performanceHud->AddMetric(performanceHud->times_preTick, frametime);
}


ON_EVENT_P(POST_TICK, 2147483647) {
	postTick = NOW();
}

ON_EVENT_P(POST_TICK, -2147483647) {
	float frametime = std::chrono::duration_cast<std::chrono::microseconds>(NOW() - postTick).count() / 1000000.0f;
	performanceHud->AddMetric(performanceHud->times_postTick, frametime);
}


CON_COMMAND(sar_performance_hud_clear, "Clears the performance HUD data.\n") {
	performanceHud->times_totalframe_offTick.clear();
	performanceHud->times_totalframe_onTick.clear();
	performanceHud->times_render.clear();
	performanceHud->times_vgui.clear();
	performanceHud->times_preTick.clear();
	performanceHud->times_postTick.clear();
	performanceHud->times_vgui_elements.clear();
}

PerformanceHud *performanceHud = new PerformanceHud();
