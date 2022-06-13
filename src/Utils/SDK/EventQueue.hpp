#pragma once


struct CEventAction {
	const char *m_iTarget;       // 0
	const char *m_iTargetInput;  // 4
	const char *m_iParameter;    // 8
	float m_flDelay;             // 12
	int m_nTimesToFire;          // 16
	int m_iIDStamp;              // 20
	CEventAction *m_pNext;       // 24
};

struct EventQueuePrioritizedEvent_t {
	float m_flFireTime;                     // 0
	char *m_iTarget;                        // 4
	char *m_iTargetInput;                   // 8
	int m_pActivator;                       // 12
	int m_pCaller;                          // 16
	int m_iOutputID;                        // 20
	int m_pEntTarget;                       // 24
	char m_VariantValue[20];                // 28
	EventQueuePrioritizedEvent_t *m_pNext;  // 48
	EventQueuePrioritizedEvent_t *m_pPrev;  // 52
};

struct CEventQueue {
	EventQueuePrioritizedEvent_t m_Events;  // 0
	int m_iListCount;                       // 56
};
