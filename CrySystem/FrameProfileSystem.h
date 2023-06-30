////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   frameprofilesystem.h
//  Version:     v1.00
//  Created:     24/6/2003 by Timur,Sergey,Wouter.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __frameprofilesystem_h__
#define __frameprofilesystem_h__
#pragma once

#include "platform.h"
#include "FrameProfiler.h"
#if !defined (LINUX)
#	include <Psapi.h>
#endif

#ifdef USE_FRAME_PROFILER

//////////////////////////////////////////////////////////////////////////
// Frame Profile Timer, provides precise timer for frame profiler.
//////////////////////////////////////////////////////////////////////////
class CFrameProfilerTimer
{
public:
	static void Init(); // called once
	static void GetTicks( int64* nTime);
	static int64 GetTicks() { int64 nTime; GetTicks(&nTime); return nTime; }
	static float TicksToSeconds( int64 nTime );
	static float TicksToMilliseconds( int64 nTime );
protected:
	static int64 g_nTicksPerSecond;
	static double g_fSecondsPerTick;
	static double g_fMilliSecondsPerTick;

	// CPU speed, in Herz
	static unsigned g_nCPUHerz;
};

//////////////////////////////////////////////////////////////////////////
//! the system which does the gathering of stats
class CFrameProfileSystem : public IFrameProfileSystem
{
public:
	int m_nCurSample;
	
	char *m_pPrefix;
	bool m_bEnabled;
	//! True when collection must be paused.
	bool m_bCollectionPaused;
	
	//! If set profiling data will be collected.
	bool m_bCollect;
	//! If set profiling data will be displayed.
	bool m_bDisplay;
	//! True if network profiling is enabled.
	bool m_bNetworkProfiling;
	//! If set memory info by modules will be displayed.
	bool m_bDisplayMemoryInfo;
	//! Put memory info also in the log.
	bool m_bLogMemoryInfo;

	ISystem *m_pSystem;
	IRenderer *m_pRenderer;

	struct SPeakRecord
	{
		CFrameProfiler *pProfiler;
		float peakValue;
		float avarageValue;
		float variance;
		int pageFaults; // Number of page faults at this frame.
		int count;  // Number of times called for peak.
		float when; // when it added.
	};
	struct SProfilerDisplayInfo
	{
		float x,y; // Position where this profiler rendered.
		int averageCount;
		int level; // child level.
		CFrameProfiler *pProfiler;
	};
	struct SSubSystemInfo
	{
		const char *name;
		float selfTime;
	};

	EDisplayQuantity m_displayQuantity;

	int m_smoothFrame;
	int m_smoothMaxFrames;

	//! When profiling frame started.
	int64 m_frameStartTime;
	//! Total time of profiling.
	int64 m_totalProfileTime;
	//! Frame time from the last frame.
	int64 m_frameTime;
	//! Frame time not accounted by registered profilers.
	int64 m_frameLostTime;
	//! Frame profiler.
	CFrameProfilerSection *m_pCurrentProfileSection;
	CCustomProfilerSection *m_pCurrentCustomSection;

	typedef std::vector<CFrameProfiler*> Profilers;
	//! Array of all registered profilers.
	Profilers m_profilers;
	//! Network profilers, they are not in regular list.
	Profilers m_netTrafficProfilers;
	//! Currently active profilers array.
	Profilers *m_pProfilers;

	float m_peakTolerance;
	//! List of several latest peaks.
	std::vector<SPeakRecord> m_peaks;
	std::vector<SProfilerDisplayInfo> m_displayedProfilers;
	bool m_bDisplayedProfilersValid;
	EProfiledSubsystem m_subsystemFilter;
	bool m_bSubsystemFilterEnabled;

	//////////////////////////////////////////////////////////////////////////
	//! Smooth frame time in milliseconds.
	CFrameProfilerSamplesHistory<float,32> m_frameTimeHistory;
	CFrameProfilerSamplesHistory<float,32> m_frameTimeLostHistory;

	//////////////////////////////////////////////////////////////////////////
	// Graphs.
	//////////////////////////////////////////////////////////////////////////
	bool m_bDrawGraph;
	std::vector<unsigned char> m_timeGraph;
	std::vector<unsigned char> m_timeGraph2;
	int m_timeGraphCurrentPos;
	CFrameProfiler* m_pGraphProfiler;

	//////////////////////////////////////////////////////////////////////////
	// Histograms.
	//////////////////////////////////////////////////////////////////////////
	bool m_bEnableHistograms;
	int m_histogramsCurrPos;
	int m_histogramsMaxPos;
	int m_histogramsHeight;
	float m_histogramScale;

	//////////////////////////////////////////////////////////////////////////
	// Selection/Render.
	//////////////////////////////////////////////////////////////////////////
	int m_selectedRow,m_selectedCol;
	float ROW_SIZE,COL_SIZE;
	float m_baseY;
	float m_mouseX,m_mouseY;

#ifdef WIN32
	HMODULE hPsapiModule;
	typedef BOOL (WINAPI *FUNC_GetProcessMemoryInfo)( HANDLE,PPROCESS_MEMORY_COUNTERS,DWORD );
	FUNC_GetProcessMemoryInfo pfGetProcessMemoryInfo;
	bool m_bNoPsapiDll;
#endif
	int m_nPagesFaultsLastFrame;
	int m_nPagesFaultsPerSec;
	int64 m_nLastPageFaultCount;
	bool m_bPageFaultsGraph;

	//////////////////////////////////////////////////////////////////////////
	// Subsystems.
	//////////////////////////////////////////////////////////////////////////
	SSubSystemInfo m_subsystems[PROFILE_LAST_SUBSYSTEM];
	
	CFrameProfilerOfflineHistory m_frameTimeOfflineHistory;

	//////////////////////////////////////////////////////////////////////////
	// Peak callbacks.
	//////////////////////////////////////////////////////////////////////////
	std::vector<IFrameProfilePeakCallback*> m_peakCallbacks;

public:
	//////////////////////////////////////////////////////////////////////////
	// Methods.
	//////////////////////////////////////////////////////////////////////////
	CFrameProfileSystem();
	~CFrameProfileSystem();
	void Init( ISystem *pSystem );
	void Done();

	void SetProfiling(bool on, bool display, char *prefix, ISystem *pSystem);

	//////////////////////////////////////////////////////////////////////////
	// IFrameProfileSystem interface implementation.
	//////////////////////////////////////////////////////////////////////////
	//! Reset all profiling data.
	void Reset();
	//! Add new frame profiler.
	//! Profile System will not delete those pointers, client must take care of memory managment issues.
	void AddFrameProfiler( CFrameProfiler *pProfiler );
	//! Must be called at the start of the frame.
	void StartFrame();
	//! Must be called at the end of the frame.
	void EndFrame();
	//! Get number of registered frame profilers.
	int GetProfilerCount() const { return (int)m_profilers.size(); };
	//! Get frame profiler at specified index.
	//! @param index must be 0 <= index < GetProfileCount() 
	CFrameProfiler* GetProfiler( int index ) const;

	//////////////////////////////////////////////////////////////////////////
	// Adds a value to profiler.
	virtual void StartCustomSection( CCustomProfilerSection *pSection );
	virtual void EndCustomSection( CCustomProfilerSection *pSection );

	//////////////////////////////////////////////////////////////////////////
	// Peak callbacks.
	//////////////////////////////////////////////////////////////////////////
	virtual void AddPeaksListener( IFrameProfilePeakCallback *pPeakCallback );
	virtual void RemovePeaksListener( IFrameProfilePeakCallback *pPeakCallback );
	
	//////////////////////////////////////////////////////////////////////////
	//! Starts profiling a new section.
	void StartProfilerSection( CFrameProfilerSection *pSection );
	//! Ends profiling a section.
	void EndProfilerSection( CFrameProfilerSection *pSection );

	//! Enable/Diable profile samples gathering.
	void Enable( bool bCollect,bool bDisplay );
	void EnableMemoryProfile( bool bEnable );
	void SetSubsystemFilter( bool bFilterSubsystem,EProfiledSubsystem subsystem );
	void EnableHistograms( bool bEnableHistograms );
	bool IsEnabled() const { return m_bEnabled; };
	bool IsProfiling() const { return m_bCollect; }
	void SetDisplayQuantity( EDisplayQuantity quantity );
	void AddPeak( SPeakRecord &peak );
	void SetHistogramScale( float fScale ) { m_histogramScale = fScale; }
	void SetDrawGraph( bool bDrawGraph ) { m_bDrawGraph = bDrawGraph; };
	void SetNetworkProfiler( bool bNet ) { m_bNetworkProfiling = bNet; };
	void SetPeakTolerance( float fPeakTimeMillis ) { m_peakTolerance = fPeakTimeMillis; }
	void SetPageFaultsGraph( bool bEnabled ) { m_bPageFaultsGraph = bEnabled; };

	void SetSubsystemFilter( const char *sFilterName );
	void UpdateOfflineHistory( CFrameProfiler *pProfiler );

	//////////////////////////////////////////////////////////////////////////
	// Rendering.
	//////////////////////////////////////////////////////////////////////////
	void Render();
	void RenderMemoryInfo();
	void RenderProfiler( CFrameProfiler *pProfiler,int level,float col,float row,bool bExtended,bool bSelected );
	void RenderProfilerHeader( float col,float row,bool bExtended );
	void RenderProfilers( float col,float row,bool bExtended );
	void RenderPeaks();
	void RenderSubSystems( float col,float row );
	void RenderHistograms();
	void CalcDisplayedProfilers();
	void DrawGraph();
	void DrawLabel( float raw,float column, float* fColor,float glow,const char* szText,float fScale=1.0f);
	void DrawRect( float x1,float y1,float x2,float y2,float *fColor );
	CFrameProfiler* GetSelectedProfiler();
	// Recursively add frame profiler and childs to displayed list.
	void AddDisplayedProfiler( CFrameProfiler *pProfiler,int level );

	//////////////////////////////////////////////////////////////////////////
	float TranslateToDisplayValue( int64 val );
};

#else

// Dummy Frame profile Timer interface.
class CFrameProfilerTimer
{
public:
	
	static float TicksToSeconds( int64 nTime ){return 0.0f;}
};

// Dummy Frame profile system interface.
struct CFrameProfileSystem : public IFrameProfileSystem
{
	//! Reset all profiling data.
	virtual void Reset() {};
	//! Add new frame profiler.
	//! Profile System will not delete those pointers, client must take care of memory managment issues.
	virtual void AddFrameProfiler( class CFrameProfiler *pProfiler ) {};
	//! Must be called at the start of the frame.
	virtual void StartFrame() {};
	//! Must be called at the end of the frame.
	virtual void EndFrame() {};

	//! Here the new methods needed to enable profiling to go off.
	virtual int GetProfilerCount() const {return 0;}

	virtual CFrameProfiler* GetProfiler( int index ) const {return NULL;}

	virtual void Enable( bool bCollect,bool bDisplay ){}

	virtual void SetSubsystemFilter( bool bFilterSubsystem,EProfiledSubsystem subsystem ){}
	virtual void SetSubsystemFilter( const char *sFilterName ){}

	virtual bool IsEnabled() const {return 0;}
	
	virtual bool IsProfiling() const {return 0;}

	virtual void SetDisplayQuantity( EDisplayQuantity quantity ){}

	virtual void StartCustomSection( CCustomProfilerSection *pSection ){}
	virtual void EndCustomSection( CCustomProfilerSection *pSection ){}

	virtual void AddPeaksListener( IFrameProfilePeakCallback *pPeakCallback ){}
	virtual void RemovePeaksListener( IFrameProfilePeakCallback *pPeakCallback ){}

	void Init( ISystem *pSystem ){}
	void Done(){}
	void Render(){}

	void SetHistogramScale( float fScale ){}
	void SetDrawGraph( bool bDrawGraph ){}
	void SetNetworkProfiler( bool bNet ){}
	void SetPeakTolerance( float fPeakTimeMillis ){}
	void SetPageFaultsGraph( bool bEnabled ){}

	void EnableMemoryProfile( bool bEnable ){}
};

#endif // USE_FRAME_PROFILER

#endif // __frameprofilesystem_h__
