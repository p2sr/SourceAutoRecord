#pragma once

#define NUM_HISTOGRAM_BUCKETS 31

enum HistogramEntryState_t {
	HESTATE_INITIAL = 0,
	HESTATE_FIRST_QUERY_IN_FLIGHT,
	HESTATE_QUERY_IN_FLIGHT,
	HESTATE_QUERY_DONE,
};

struct CHistogramBucket {
	HistogramEntryState_t m_state;
	int m_hOcclusionQueryHandle;
	int m_nFrameQueued;
	int m_nPixels;
	int m_nPixelsInRange;
	float m_flMinLuminance, m_flMaxLuminance;
	float m_flScreenMinX, m_flScreenMinY, m_flScreenMaxX, m_flScreenMaxY;
};

struct CTonemapSystem {
	CHistogramBucket m_histogramBucketArray[NUM_HISTOGRAM_BUCKETS];
	int m_nCurrentQueryFrame;
	int m_nCurrentAlgorithm;

	float m_flTargetTonemapScale;
	float m_flCurrentTonemapScale;

	int m_nNumMovingAverageValid;
	float m_movingAverageTonemapScale[10];

	bool m_bOverrideTonemapScaleEnabled;
	float m_flOverrideTonemapScale;
};
