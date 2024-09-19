#pragma once

#include <vector>

template <class T>
class FixedQueue {
private:
	std::vector<T> queue;
	int begin, end;
	inline int BoundIndex(int trueIndex, int side = 0) const { //1 right, -1 left, 0 both
		if (side >= 0 && trueIndex >= queue.size()) {
			trueIndex -= queue.size();
		}
		if (side <= 0 && trueIndex < 0) {
			trueIndex += queue.size();
		}
		return trueIndex;
	}

public:
	size_t size;
	FixedQueue(size_t maxSize)
		: queue(maxSize)
		, begin(0)
		, end(0)
		, size(0) {
	}
	FixedQueue(size_t maxSize, T defaultValue)
		: queue(maxSize, defaultValue)
		, begin(0)
		, end(0)
		, size(maxSize) {
	}
	inline T First() const {
		return queue[begin];
	}
	inline T Last() const {
		return queue[BoundIndex(end - 1, -1)];
	}
	inline void Push(T value) {
		queue[end] = value;
		end = BoundIndex(end + 1, 1);
		if (size == queue.size()) {
			begin = BoundIndex(begin + 1, 1);
		} else {
			size++;
		}
	}
	inline void Clear() {
		begin = 0;
		end = 0;
		size = 0;
	}
	inline void setAll(T value) {
		begin = 0;
		end = 0;
		size = queue.size();
		for (auto i = 0; i < size; i++) {
			queue[i] = value;
		}
	}
	inline T operator[](int index) const {
		return queue[BoundIndex(index + begin, 1)];
	}
	inline T &operator[](int index) {
		return queue[BoundIndex(index + begin, 1)];
	}
};
