#pragma once

template <class T, class I = int>
struct CUtlMemory {
	T *m_pMemory;
	int m_nAllocationCount;
	int m_nGrowSize;
};

template <class T, class A = CUtlMemory<T>>
struct CUtlVector {
	A m_Memory;
	int m_Size;
	T *m_pElements;

	void Append(const T &val) {
		if (this->m_Size == this->m_Memory.m_nAllocationCount) {
			int grow = this->m_Memory.m_nGrowSize;
			if (grow == 0)
				grow = 1;
			this->m_Memory.m_nAllocationCount += grow;
			this->m_Memory.m_pMemory = static_cast<T *>(realloc(this->m_Memory.m_pMemory, sizeof(T) * this->m_Memory.m_nAllocationCount));
			this->m_pElements = this->m_Memory.m_pMemory;
		}
		this->m_Memory.m_pMemory[this->m_Size] = val;
		this->m_Size++;
	}

	void Clear() {
		if (this->m_Memory.m_pMemory) {
			free(this->m_Memory.m_pMemory);
			this->m_Memory.m_pMemory = 0;
		}
		this->m_Size = 0;
		this->m_Memory.m_nAllocationCount = 0;
	}
};
