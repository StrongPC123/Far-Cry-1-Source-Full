////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   frameprofiler.h
//  Version:     v1.00
//  Created:     24/6/2003 by Timur,Sergey,Wouter.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __frameprofiler_h__
#define __frameprofiler_h__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define USE_FRAME_PROFILER      // comment this define to remove most profiler related code in the engine

enum EProfiledSubsystem
{
	PROFILE_ANY,
	PROFILE_RENDERER,
	PROFILE_3DENGINE,
	PROFILE_AI,
	PROFILE_ANIMATION,
	PROFILE_MOVIE,
	PROFILE_ENTITY,
	PROFILE_FONT,
	PROFILE_NETWORK,
	PROFILE_PHYSICS,
	PROFILE_SCRIPT,
	PROFILE_SOUND,
	PROFILE_EDITOR,
	PROFILE_SYSTEM,
	PROFILE_GAME,

	//////////////////////////////////////////////////////////////////////////
	// This enumes used for Network traffic profilers.
	// They record bytes transferred instead of time.
	//////////////////////////////////////////////////////////////////////////
	PROFILE_NETWORK_TRAFFIC,

	PROFILE_LAST_SUBSYSTEM // Must always be last.
};

#include "platform.h"
#include "ISystem.h"

class CFrameProfiler;
class CFrameProfilerSection;
class CCustomProfilerSection;

//////////////////////////////////////////////////////////////////////////
/*! This callback will be called for Profiling Peaks.
 */
struct IFrameProfilePeakCallback
{
	//! Called when peak is detected for this profiler.
	//! @param fPeakTime peak time in milliseconds.
	virtual void OnFrameProfilerPeak( CFrameProfiler *pProfiler,float fPeakTime ) = 0;
};

//////////////////////////////////////////////////////////////////////////
//! IFrameProfileSystem interface.
//! the system which does the gathering of stats
struct IFrameProfileSystem
{
	enum EDisplayQuantity
	{
		SELF_TIME,
		TOTAL_TIME,
		SELF_TIME_EXTENDED,
		TOTAL_TIME_EXTENDED,
		PEAK_TIME,
		SUBSYSTEM_INFO,
		COUNT_INFO,
	};

	//! Reset all profiling data.
	virtual void Reset() = 0;
	//! Add new frame profiler.
	virtual void AddFrameProfiler( CFrameProfiler *pProfiler ) = 0;
	//! Must be called at the start of the frame.
	virtual void StartFrame() = 0;
	//! Must be called at the end of the frame.
	virtual void EndFrame() = 0;

	//! Get number of registered frame profilers.
	virtual int GetProfilerCount() const = 0;
	//! Get frame profiler at specified index.
	//! @param index must be 0 <= index < GetProfileCount() 
	virtual CFrameProfiler* GetProfiler( int index ) const = 0;

	virtual void Enable( bool bCollect,bool bDisplay ) = 0;
	virtual void SetSubsystemFilter( bool bFilterSubsystem,EProfiledSubsystem subsystem ) = 0;
	//! True if profiler is turned off (even if collection is paused).
	virtual bool IsEnabled() const = 0;
	//! True if profiler must collect profiling data.
	virtual bool IsProfiling() const = 0;
	virtual void SetDisplayQuantity( EDisplayQuantity quantity ) = 0;

	// For custom frame profilers.
	virtual void StartCustomSection( CCustomProfilerSection *pSection ) = 0;
	virtual void EndCustomSection( CCustomProfilerSection *pSection ) = 0;

	//! Register peak listener callback to be called when peak value is greater then this.
	virtual void AddPeaksListener( IFrameProfilePeakCallback *pPeakCallback ) = 0;
	virtual void RemovePeaksListener( IFrameProfilePeakCallback *pPeakCallback ) = 0;
};


//////////////////////////////////////////////////////////////////////////
//! CFrameProfilerSamplesHistory provides information on history of sample values
//! for profiler counters.
//////////////////////////////////////////////////////////////////////////
template <class T, int TCount>
class CFrameProfilerSamplesHistory
{
public:
	CFrameProfilerSamplesHistory() :	m_nHistoryNext (0),m_nHistoryCount(0) {}

	//! Add a new sample to history.
	void Add( T sample )
	{
		m_history[m_nHistoryNext] = sample;
		m_nHistoryNext = (m_nHistoryNext+TCount-1) % TCount;
		if (m_nHistoryCount < TCount)
			++m_nHistoryCount;
	}
	//! cleans up the data history 
	void Clear()
	{
		m_nHistoryNext = 0;
		m_nHistoryCount = 0;
	}
	//! Get last sample value.
	T GetLast()
	{
		if (m_nHistoryCount)
			return m_history[(m_nHistoryNext+1)%TCount];
		else
			return 0;
	}
	//! calculates average sample value for at most the given number of frames (less if so many unavailable)
	T GetAverage( int nCount = TCount )
	{
		if (m_nHistoryCount)
		{
			T fSum = 0;
			if (nCount > m_nHistoryCount)
				nCount = m_nHistoryCount;
			for (int i = 1; i <= nCount; ++i)
			{
				fSum += m_history[(m_nHistoryNext+i) % (sizeof(m_history)/sizeof((m_history)[0]))];
			}
			return fSum / nCount;
		}
		else
			return 0;
	}
	//! calculates average sample value for at most the given number of frames (less if so many unavailable),
	//! multiplied by the Poisson function
	T GetAveragePoisson(int nCount, float fMultiplier)
	{
		if (m_nHistoryCount)
		{
			float fSum = 0, fCurrMult = 1, fSumWeight = 0;
			if (nCount > m_nHistoryCount)
				nCount = m_nHistoryCount;
			for (int i = 1; i <= nCount; ++i)
			{
				fSum += m_history[(m_nHistoryNext+i)%TCount] * fCurrMult;
				fSumWeight += fCurrMult;
				fCurrMult *= fMultiplier;
			}
			return (T)(fSum / fSumWeight);
		}
		else
			return 0;
	}
	//! calculates Standart deviation of values.
	//! stdev = Sqrt( Sum((X-Xave)^2)/(n-1) )
	T GetStdDeviation( int nCount = TCount )
	{
		if (m_nHistoryCount)
		{
			T fAve = GetAverage(nCount);
			T fVal = 0;
			T fSumVariance = 0;
			if (nCount > m_nHistoryCount)
				nCount = m_nHistoryCount;
			for (int i = 1; i <= nCount; ++i)
			{
				fVal = m_history[(m_nHistoryNext+i) % (sizeof(m_history)/sizeof((m_history)[0]))];
				fSumVariance = (fVal - fAve)*(fVal - fAve); // (X-Xave)^2
			}
			return sqrtf( fSumVariance/(nCount-1) );
		}
		else
			return 0;
	}
	//! calculates max sample value for at most the given number of frames (less if so many unavailable)
	T GetMax(int nCount=TCount)
	{
		if (m_nHistoryCount)
		{
			T fMax;
			if (nCount > m_nHistoryCount)
				nCount = m_nHistoryCount;
			for (int i = 1; i <= nCount; ++i)
			{
				T fCur = m_history[(m_nHistoryNext+i) % (sizeof(m_history)/sizeof((m_history)[0]))];
				if (i == 1 || fCur > fMax)
					fMax = fCur;
			}
			return fMax;
		}
		else
			return 0;
	}
	//! calculates min sample value for at most the given number of frames (less if so many unavailable)
	T GetMin(int nCount = TCount)
	{
		if (m_nHistoryCount)
		{
			T fMin;
			if (nCount > m_nHistoryCount)
				nCount = m_nHistoryCount;
			for (int i = 1; i <= nCount; ++i)
			{
				T fCur = m_history[(m_nHistoryNext+i) % (sizeof(m_history)/sizeof((m_history)[0]))];
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
	T m_history[TCount];
	// the current pointer in the timer history, decreases
	int m_nHistoryNext;
	// the currently collected samples in the timer history
	int m_nHistoryCount;
	// adds the entry to the timer history (current timer value)
};

//////////////////////////////////////////////////////////////////////////
class CFrameProfilerGraph
{
public:
	int m_x;
	int m_y;
	int m_width;
	int m_height;
	std::vector<unsigned char> m_data;
};

//////////////////////////////////////////////////////////////////////////
class CFrameProfilerOfflineHistory
{
public:
	//! Self time in microseconds.
	std::vector<unsigned int> m_selfTime;
	//! Number of calls.
	std::vector<unsigned short> m_count;
};

//////////////////////////////////////////////////////////////////////////
//! CFrameProfiler is a single profiler counter with unique name and data.
//! Multiple Sections can be executed for this profiler, they all will be merged in this class.
//! CFrameProfileSection must reference pointer to instance of this counter, to collect the sampling data.
//!
class CFrameProfiler
{
public:
	ISystem *m_pISystem;
	const char *m_name;

	//! Total time spent in this counter including time of child profilers in current frame.
	int64 m_totalTime;
	//! Self frame time spent only in this counter (But includes recursive calls to same counter) in current frame.
	int64 m_selfTime;
	//! How many times this profiler counter was executed.
	int m_count;
	//! Total time spent in this counter during all profiling period.
	int64 m_sumTotalTime;
	//! Total self time spent in this counter during all profiling period.
	int64 m_sumSelfTime;
	//! Displayed quantity (interpolated or avarage).
	float m_displayedValue;
	//! Displayed quantity (current frame value).
	float m_displayedCurrentValue;
	//! How variant this value.
	float m_variance;

	//! Current parent profiler in last frame.
	CFrameProfiler *m_pParent;
	//! Expended or collapsed displaying state.
	bool m_bExpended;
	bool m_bHaveChildren;

	EProfiledSubsystem m_subsystem;

	CFrameProfilerSamplesHistory<float,64> m_totalTimeHistory;
	CFrameProfilerSamplesHistory<float,64> m_selfTimeHistory;
	CFrameProfilerSamplesHistory<int,64> m_countHistory;

	//! Graph data for this frame profiler.
	
	//! Graph associated with this profiler.
	CFrameProfilerGraph* m_pGraph;
	CFrameProfilerOfflineHistory *m_pOfflineHistory;

	CFrameProfiler( ISystem *pSystem,const char *sCollectorName,EProfiledSubsystem subsystem=PROFILE_ANY )
	{
		m_pParent = 0;
		m_pGraph = 0;
		m_bExpended = false;
		m_bHaveChildren = false;
		m_pOfflineHistory = 0;
		m_subsystem = subsystem;
		m_totalTime = m_selfTime = 0;
		m_sumTotalTime = m_sumSelfTime = 0;
		m_count = 0;
		m_pISystem = pSystem;
		m_name = sCollectorName;
		m_pISystem->GetIProfileSystem()->AddFrameProfiler( this );
	}
};

//////////////////////////////////////////////////////////////////////////
//! CFrameProfilerSection is an auto class placed where code block need to be profiled.
//! Every time this object is constructed and destruted the time between constructor
//! and destructur is merged into the referenced CFrameProfiler instance.
//!
class CFrameProfilerSection
{
public:
	int64 m_startTime;
	int64 m_excludeTime;
	CFrameProfiler *m_pFrameProfiler;
	CFrameProfilerSection *m_pParent;

	__forceinline CFrameProfilerSection( CFrameProfiler *profiler )
	{
		m_pFrameProfiler = profiler;
		if (profiler)
			m_pFrameProfiler->m_pISystem->StartProfilerSection( this );
	}
	__forceinline ~CFrameProfilerSection()
	{
		if (m_pFrameProfiler)
			m_pFrameProfiler->m_pISystem->EndProfilerSection( this );
	}
};


//////////////////////////////////////////////////////////////////////////
//! CCustomProfilerSection is an auto class placed where any custom data need to be profiled.
//! Works similary to CFrameProfilerSection, but records any custom data, instead of elapsed time.
//!
class CCustomProfilerSection
{
public:
	int *m_pValue;
	int m_excludeValue;
	CFrameProfiler *m_pFrameProfiler;
	CCustomProfilerSection *m_pParent;

	//! pValue pointer must remain valid until after calling destructor of this custom profiler section.
	__forceinline CCustomProfilerSection( CFrameProfiler *profiler,int *pValue )
	{
		m_pValue = pValue;
		m_pFrameProfiler = profiler;
		if (profiler)
			m_pFrameProfiler->m_pISystem->GetIProfileSystem()->StartCustomSection( this );
	}
	__forceinline ~CCustomProfilerSection()
	{
		if (m_pFrameProfiler)
			m_pFrameProfiler->m_pISystem->GetIProfileSystem()->EndCustomSection( this );
	}
};

//USE_FRAME_PROFILER
#if defined(USE_FRAME_PROFILER) && (!defined(_RELEASE) || defined(WIN64))

//////////////////////////////////////////////////////////////////////////
//! Place this macro when you need to profile a function.
//!
//! void CTest::Func() {
//!   FUNCTION_PROFILER( GetISystem() );
//!   // function body will be profiled.
//! }
#define FUNCTION_PROFILER( pISystem,subsystem ) \
	static CFrameProfiler staticFrameProfiler( pISystem,__FUNCTION__,subsystem ); \
	CFrameProfilerSection frameProfilerSection( &staticFrameProfiler );

#define FUNCTION_PROFILER_FAST( pISystem,subsystem,bProfileEnabled ) \
	static CFrameProfiler staticFrameProfiler( pISystem,__FUNCTION__,subsystem ); \
	CFrameProfilerSection frameProfilerSection( (bProfileEnabled)?&staticFrameProfiler:NULL );

//////////////////////////////////////////////////////////////////////////
//! Place this macro when you need to profile any code block.
//! {
//!		... some code ...
//!   {
//!			FRAME_PROFILER( GetISystem(),"MyCode" );
//!     ... more code ... // This code will be profiled with counter named "MyCode"
//!		}
//! }
#define FRAME_PROFILER( szProfilerName,pISystem,subsystem ) \
	static CFrameProfiler staticFrameProfiler( pISystem,szProfilerName,subsystem ); \
	CFrameProfilerSection frameProfilerSection( &staticFrameProfiler );

//! Faster version of FRAME_PROFILE macro, also accept a pointer to boolean variable which turn on/off profiler.
#define FRAME_PROFILER_FAST( szProfilerName,pISystem,subsystem,bProfileEnabled ) \
	static CFrameProfiler staticFrameProfiler( pISystem,szProfilerName,subsystem ); \
	CFrameProfilerSection frameProfilerSection( (bProfileEnabled)?&staticFrameProfiler:NULL );

#else //#if !defined(_RELEASE) || defined(WIN64)

#define FUNCTION_PROFILER( pISystem,subsystem )
#define FUNCTION_PROFILER_FAST( pISystem,subsystem,bProfileEnabled )
#define FRAME_PROFILER( szProfilerName,pISystem,subsystem )
#define FRAME_PROFILER_FAST( szProfilerName,pISystem,subsystem,bProfileEnabled )
;

#endif //USE_FRAME_PROFILER

#endif // __frameprofiler_h__
