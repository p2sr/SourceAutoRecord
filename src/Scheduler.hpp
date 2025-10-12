#include <functional>

namespace Scheduler {
	void InServerTicks(int ticks, std::function<void()> fn);
	void InHostTicks(int ticks, std::function<void()> fn);
	void OnMainThread(std::function<void()> fn);
}
