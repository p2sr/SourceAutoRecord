#pragma once

namespace Timer
{
	namespace Average
	{
		struct AverageItem {
			int Ticks;
			float Time;
			char* Map;
		};

		bool IsEnabled;

		std::vector<AverageItem> Items;

		int AverageTicks;
		float AverageTime;

		void Start()
		{
			Items.clear();
			AverageTicks = 0;
			AverageTime = 0;
			IsEnabled = true;
		}
		void Add(int ticks, float time, char* map)
		{
			Items.push_back(AverageItem{
				ticks,
				time,
				map
			});
			int count = Items.size();
			AverageTicks += ticks / count;
			AverageTime += time / count;
		}
	}
}