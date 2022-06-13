#pragma once

#include <cstdint>

class bf_read {
public:
	uint32_t ReadUnsigned(int nbits) {
		if (m_nBitsAvail >= nbits) {
			unsigned val = m_nInBufWord & ((1 << nbits) - 1);
			m_nBitsAvail -= nbits;
			m_nInBufWord >>= nbits;
			if (m_nBitsAvail == 0) {
				m_nBitsAvail = 32;
				m_nInBufWord = *(m_pDataIn++);
			}
			return val;
		}

		int rem = nbits - m_nBitsAvail;
		uint32_t hi = m_nInBufWord << rem;
		m_nBitsAvail = 32;
		m_nInBufWord = *(m_pDataIn++);
		uint32_t lo = m_nInBufWord & ((1 << rem) - 1);
		return (hi | lo) & ((1 << nbits) - 1);
	}
	const char *m_pDebugName;
	bool m_bOverflow;
	int m_nDataBits;
	size_t m_nDataBytes;
	uint32_t m_nInBufWord;
	int m_nBitsAvail;
	const uint32_t *m_pDataIn;
	const uint32_t *m_pBufferEnd;
	const uint32_t *m_pData;
};
