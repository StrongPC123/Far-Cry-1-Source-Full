//////////////////////////////////////////////////////////////////////////
// A simple profiler useful for collecting multiple call times per frame
// and displaying their different average statistics.
// For usage, see the bottom of the file
//////////////////////////////////////////////////////////////////////////


#ifndef _SIMPLE_FRAME_PROFILER_HDR_
#define _SIMPLE_FRAME_PROFILER_HDR_

// OBSOLETE
#if 0
#define ENABLE_FRAME_PROFILER 0


class CProfilerTimer
{
public:
	static void init(); // called once
	static void getTicks(__int64* nTime);
	static __int64 getTicks() {__int64 nTime; getTicks(&nTime); return nTime;}
	static float ticksToSeconds (__int64 nTime);
	static float ticksToMilliseconds (__int64 nTime);
protected:
	static __int64 g_nTicksPerSecond;
	static double g_fSecondsPerTick;
	static double g_fMilliSecondsPerTick;

	// CPU speed, in Herz
	static unsigned g_nCPUHerz;
};

extern CProfilerTimer g_ProfilerTimer;

template <typename TValue, int g_nCount>
class CProfilerTimerHistory
{
public:
	CProfilerTimerHistory():
		m_nTimerHistoryNext (0),
		m_nTimerHistoryCount (0)
	{
	}

	void add(float fTimer)
	{
		m_fTimerHistory[m_nTimerHistoryNext] = fTimer;
		m_nTimerHistoryNext = (m_nTimerHistoryNext+g_nCount-1)%g_nCount;
		if (m_nTimerHistoryCount < g_nCount)
			++m_nTimerHistoryCount;
	}
	// cleans up the timer history 
	void clear()
	{
		m_nTimerHistoryNext = 0;
		m_nTimerHistoryCount = 0;
	}
	TValue getLast()
	{
		if (m_nTimerHistoryCount)
			return m_fTimerHistory[(m_nTimerHistoryNext+1)%g_nCount];
		else
			return 0;
	}
	// calculates average time for at most the given number of frames (less if so many unavailable)
	float getAve (int nCount = g_nCount)
	{
		if (m_nTimerHistoryCount)
		{
			float fSum = 0;
			if (nCount > m_nTimerHistoryCount)
				nCount = m_nTimerHistoryCount;
			for (int i = 1; i <= nCount; ++i)
			{
				fSum += m_fTimerHistory[(m_nTimerHistoryNext+i)%SIZEOF_ARRAY(m_fTimerHistory)];
			}
			return fSum / nCount;
		}
		else
			return 0;
	}
	// calculates average time for at most the given number of frames (less if so many unavailable),
	// multiplied by the Poisson function
	float getAvePoisson (int nCount, float fMultiplier)
	{
		if (m_nTimerHistoryCount)
		{
			float fSum = 0, fCurrMult = 1, fSumWeight = 0;
			if (nCount > m_nTimerHistoryCount)
				nCount = m_nTimerHistoryCount;
			for (int i = 1; i <= nCount; ++i)
			{
				fSum += m_fTimerHistory[(m_nTimerHistoryNext+i)%g_nCount] * fCurrMult;
				fSumWeight += fCurrMult;
				fCurrMult *= fMultiplier;
			}
			return fSum / fSumWeight;
		}
		else
			return 0;
	}
	// calculates max time for at most the given number of frames (less if so many unavailable)
	float getMax(int nCount=g_nCount)
	{
		if (m_nTimerHistoryCount)
		{
			float fMax;
			if (nCount > m_nTimerHistoryCount)
				nCount = m_nTimerHistoryCount;
			for (int i = 1; i <= nCount; ++i)
			{
				float fCur = m_fTimerHistory[(m_nTimerHistoryNext+i)%SIZEOF_ARRAY(m_fTimerHistory)];
				if (i == 1 || fCur > fMax)
					fMax = fCur;
			}
			return fMax;
		}
		else
			return 0;
	}
	// calculates min time for at most the given number of frames (less if so many unavailable)
	float getMin(int nCount = g_nCount)
	{
		if (m_nTimerHistoryCount)
		{
			float fMin;
			if (nCount > m_nTimerHistoryCount)
				nCount = m_nTimerHistoryCount;
			for (int i = 1; i <= nCount; ++i)
			{
				float fCur = m_fTimerHistory[(m_nTimerHistoryNext+i)%SIZEOF_ARRAY(m_fTimerHistory)];
				if (i == 1 || fCur < fMin)
					fMin = fCur;
			}
			return fMin;
		}
		else
			return 0;
	}
protected:
	// the timer values for the last frames
	TValue m_fTimerHistory[g_nCount];
	// the current pointer in the timer history, decreases
	int m_nTimerHistoryNext;
	// the currently collected samples in the timer history
	int m_nTimerHistoryCount;
	// adds the entry to the timer history (current timer value)
};

// this is the accumulator - static object that will hold the profile info
class CSimpleFrameProfilerInfo: public CryAnimationBase
{
public:
	CSimpleFrameProfilerInfo (const char* szName);

	void startInterval();
	void endInterval();

	void startDelay();
	void endDelay();

	void flush();
protected:
	void drawHeaderLabel ();
	void drawStatistics (float fRow, float* fColor, const char* szLabel, CProfilerTimerHistory<float,64>& rProfiler);
	void drawLabel(float fRow, float* fColor, const char* szText);

protected:

	__int64 m_nTickTimer; // accumulates the time spent on this frame
	unsigned m_nCounter; // accumulates the number of times the interval has been measured during the last frame
	float m_fCounter; // accumulates the 
	int m_nFrame;
	const char* m_szName;
	int m_nIndex; // the index of this object
	
	CProfilerTimerHistory<float,64> m_HistTime, m_HistCount;
	static int g_nCount; // count of such objects ever created
};





// this is a simple class facilitating profiling within one frame:
// during the frame, it collects the profile info, and when the frame
// changes, it displays the result and flushes it to start over again
// It doesn't take the recursiveness into account
class CSimpleFrameProfiler
{
public:
	CSimpleFrameProfiler (CSimpleFrameProfilerInfo* pInfo):
		m_pInfo (pInfo)
	{
		if (pInfo)
			pInfo->startInterval();
	}
	~CSimpleFrameProfiler ()
	{
		if (m_pInfo)
			m_pInfo->endInterval();
	}

protected:
	CSimpleFrameProfilerInfo* m_pInfo;
};

// this is a class facilitating profiling within one frame, taking recursiveness
// into account:
// during the frame, it collects the profile info, and when the frame
// changes, it displays the result and flushes it to start over again
// If one block is nested into another, it delays profiling of the parent block
class CRecursiveFrameProfiler
{
	// the depth of the profilers' stack
	enum {nStackDepth = 32};
public:
	CRecursiveFrameProfiler (CSimpleFrameProfilerInfo* pInfo):
		m_pInfo (pInfo)
	{
		if (pInfo)
		{
			pInfo->startInterval();
			if (g_nStackTop > 0)
				g_arrStack[g_nStackTop-1]->startDelay();
			g_arrStack[g_nStackTop] = this;
			++g_nStackTop;
			assert (g_nStackTop < nStackDepth);
		}
	}
	~CRecursiveFrameProfiler ()
	{
		if (m_pInfo)
		{
			m_pInfo->endInterval();
			--g_nStackTop;
			if (g_nStackTop > 0)
				g_arrStack[g_nStackTop-1]->endDelay();
			assert (g_nStackTop >= 0);
		}
	}

protected:
	void startDelay()
	{
		// this is only called for the profilers on the stack; only the profilers
		// with non-NULL profiler info are placed on the stack
		assert (m_pInfo);
		m_pInfo->startDelay();
	}

	void endDelay()
	{
		// this is only called for the profilers on the stack; only the profilers
		// with non-NULL profiler info are placed on the stack
		assert (m_pInfo);
		m_pInfo->endDelay();
	}
protected:

	CSimpleFrameProfilerInfo* m_pInfo;
	// the profilers' stack
	static CRecursiveFrameProfiler* g_arrStack[nStackDepth];
	// the profiler stack pointer (the top of the stack, the stack grows up)
	static int g_nStackTop;
};

// USAGE: declare in someplace:
// static DECLARE_FRAME_PROFILER(a123,"my name");
// in all the profiled blocks, put the following:
// PROFILE_FRAME(a123, true);

// set #if 0 here if you don't want profiling to be compiled in the code
#if ENABLE_FRAME_PROFILER
#include "CVars.h"
#define DECLARE_FRAME_PROFILER(id,name) extern CSimpleFrameProfilerInfo __##id##_frame_profiler
#define DEFINE_FRAME_PROFILER(id,name) CSimpleFrameProfilerInfo __##id##_frame_profiler(name)
#define PROFILE_FRAME_SELF(id) CRecursiveFrameProfiler __##id##_auto_frame_profile_locker(CryAnimationBase::GetCVars()->ca_Profile()?&__##id##_frame_profiler:NULL)
#define PROFILE_FRAME_TOTAL(id) CSimpleFrameProfiler __##id##_auto_frame_profile_locker(CryAnimationBase::GetCVars()->ca_Profile()?&__##id##_frame_profiler:NULL)
#define PROFILE_FRAME(id) PROFILE_FRAME_SELF(id)
// flushes the given profiler: makes sure it's called this frame (to draw)
#define FLUSH_PROFILER(id) do{if (CryAnimationBase::GetCVars()->ca_Profile())__##id##_frame_profiler.flush();}while(false)
#else
#define DECLARE_FRAME_PROFILER(id,name)
#define DEFINE_FRAME_PROFILER(id,name)
#define PROFILE_FRAME_SELF(id)
#define PROFILE_FRAME_TOTAL(id)
#define PROFILE_FRAME(id)
// flushes the given profiler: makes sure it's called this frame (to draw)
#define FLUSH_PROFILER(id)
#endif
#endif
#endif