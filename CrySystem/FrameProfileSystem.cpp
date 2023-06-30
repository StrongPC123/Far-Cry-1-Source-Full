////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   FrameProfileSystem.cpp
//  Version:     v1.00
//  Created:     24/6/2003 by Timur,Sergey,Wouter.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FrameProfileSystem.h"
#include <ILog.h>
#include <IRenderer.h>
#include <IInput.h>
#include <StlUtils.h>

#if defined(LINUX)
#include "platform.h"
#endif

bool g_bProfilerEnabled = false;

#ifdef USE_FRAME_PROFILER

#define MAX_SMOOTH_FRAMES 40
#define MAX_PEAK_PROFILERS 20
//! Peak tolerance in milliseconds.
#define PEAK_TOLERANCE 10.0f

//////////////////////////////////////////////////////////////////////////
// CFrameProfilerTimer static variable.
//////////////////////////////////////////////////////////////////////////
int64 CFrameProfilerTimer::g_nTicksPerSecond = 1000000000;
double CFrameProfilerTimer::g_fSecondsPerTick = 1e-9;
double CFrameProfilerTimer::g_fMilliSecondsPerTick = 1e-6;
unsigned CFrameProfilerTimer::g_nCPUHerz = 1000000000;

//////////////////////////////////////////////////////////////////////////
// CFrameProfilerTimer implementation.
//////////////////////////////////////////////////////////////////////////
void CFrameProfilerTimer::Init() // called once
{
#ifdef WIN32
	QueryPerformanceFrequency ((LARGE_INTEGER*)&g_nTicksPerSecond);
#endif //WIN32

#ifdef GAMECUBE
	g_nTicksPerSecond = OS_CORE_CLOCK; //its a simple define on GC: 486.000.000
	g_nCPUHerz = OS_CORE_CLOCK;
#endif 

	g_fSecondsPerTick = 1.0 / (double)g_nTicksPerSecond;
	g_fMilliSecondsPerTick = 1000.0 / (double)g_nTicksPerSecond;

#ifdef WIN32
	HKEY hKey;
	DWORD dwSize = sizeof(g_nCPUHerz);
	if (ERROR_SUCCESS == RegOpenKeyEx (HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey)
		&&ERROR_SUCCESS == RegQueryValueEx (hKey, "~MHz", NULL, NULL, (LPBYTE)&g_nCPUHerz, &dwSize))
	{
		g_nCPUHerz *= 1000000;
		g_fSecondsPerTick = 1.0/(double)g_nCPUHerz;
		g_fMilliSecondsPerTick = 1000.0/(double)g_nCPUHerz;
	}
	else
		g_nCPUHerz = 1000000000;
#endif //WIN32

#ifdef _XBOX
	//@FIXME: Hack for XBOX
	g_nCPUHerz = 800*1000*1000; // 800 Mhz
	g_fSecondsPerTick = 1.0/(double)g_nCPUHerz;
	g_fMilliSecondsPerTick = 1000.0/(double)g_nCPUHerz;
#endif //XBOX
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfilerTimer::GetTicks(int64* pnTime)
{
#if defined(LINUX)
	*pnTime = ::GetTicks();
#else

#ifdef WIN64
	*pnTime = __rdtsc();
#elif defined(_CPU_X86)
	__asm {
		mov ebx, pnTime
			rdtsc
			mov [ebx], eax
			mov [ebx+4], edx
	}
#elif defined(GAMECUBE)
	//#error Please provide a precise value or zero for pnTime
	//I know it looks strange, but this are the cycles!
	pnTime = (s64*)(OSGetTime()*12); 
#elif defined(WIN32)
	QueryPerformanceCounter((LARGE_INTEGER*)pnTime);
#endif
#endif
}

//////////////////////////////////////////////////////////////////////////
float CFrameProfilerTimer::TicksToSeconds (int64 nTime)
{
	return float(g_fSecondsPerTick * nTime);
}

//////////////////////////////////////////////////////////////////////////
float CFrameProfilerTimer::TicksToMilliseconds (int64 nTime)
{
	return float(g_fMilliSecondsPerTick * nTime);
}

//////////////////////////////////////////////////////////////////////////
// FrameProfilerSystem Implementation.
//////////////////////////////////////////////////////////////////////////

// these functions will let you use both string or const char* in the maps TraceMap and SampleMap just by switching
inline const char* toCStr(const char* p){	return p;}
inline const char* toCStr (const string& str){	return str.c_str();}

//////////////////////////////////////////////////////////////////////////
CFrameProfileSystem::CFrameProfileSystem() 
: m_nCurSample(-1)
{
#ifdef WIN32
	hPsapiModule = NULL;
	pfGetProcessMemoryInfo = NULL;
	m_bNoPsapiDll = false;
#endif

	// Allocate space for 256 profilers.
	m_profilers.reserve( 256 );
	m_pCurrentProfileSection = 0;
	m_pCurrentCustomSection = 0;
	m_bEnabled = false;
	m_totalProfileTime = 0;
	m_frameStartTime = 0;
	m_frameTime = 0;
	m_frameLostTime = 0;
	m_pRenderer = 0;
	m_displayQuantity = SELF_TIME;

	m_bCollect = false;
	m_bDisplay = false;
	m_bDisplayMemoryInfo = false;
	m_bLogMemoryInfo = false;

	m_smoothFrame = 0;
	m_smoothMaxFrames = MAX_SMOOTH_FRAMES;
	m_peakTolerance = PEAK_TOLERANCE;

	m_pGraphProfiler = 0;
	m_timeGraphCurrentPos = 0;
	m_bCollectionPaused = false;
	m_bDrawGraph = false;

	m_selectedRow = -1;
	m_selectedCol = -1;

	m_bEnableHistograms = false;
	m_histogramsMaxPos = 200;
	m_histogramsHeight = 16;
	m_histogramsCurrPos = 0;

	m_bSubsystemFilterEnabled = false;
	m_subsystemFilter = PROFILE_RENDERER;
	m_histogramScale = 100;

	m_bDisplayedProfilersValid = false;
	m_bNetworkProfiling = false;
	//m_pProfilers = &m_netTrafficProfilers;
	m_pProfilers = &m_profilers;

	m_nLastPageFaultCount = 0;
	m_nPagesFaultsLastFrame = 0;
	m_bPageFaultsGraph = false;
	m_nPagesFaultsPerSec = 0;

	//////////////////////////////////////////////////////////////////////////
	// Initialize subsystems list.
	memset( m_subsystems,0,sizeof(m_subsystems) );
	m_subsystems[PROFILE_RENDERER].name = "Renderer";
	m_subsystems[PROFILE_3DENGINE].name = "3DEngine";
	m_subsystems[PROFILE_ANIMATION].name = "Animation";
	m_subsystems[PROFILE_AI].name = "AI";
	m_subsystems[PROFILE_ENTITY].name = "Entity";
	m_subsystems[PROFILE_PHYSICS].name = "Physics";
	m_subsystems[PROFILE_SOUND].name = "Sound";
	m_subsystems[PROFILE_GAME].name = "Game";
	m_subsystems[PROFILE_EDITOR].name = "Editor";
	m_subsystems[PROFILE_NETWORK].name = "Network";
	m_subsystems[PROFILE_SYSTEM].name = "System";
};

//////////////////////////////////////////////////////////////////////////
CFrameProfileSystem::~CFrameProfileSystem()
{
	// Delete graphs for all frame profilers.
#ifdef WIN32
	if (hPsapiModule)
		::FreeLibrary( hPsapiModule );
#endif
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::Init( ISystem *pSystem )
{
	m_pSystem = pSystem;

	CFrameProfilerTimer::Init();
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::Done()
{
	for (int i = 0; i < (int)m_profilers.size(); i++)
	{
		SAFE_DELETE( m_profilers[i]->m_pGraph );
		SAFE_DELETE( m_profilers[i]->m_pOfflineHistory );
	}
	for (int i = 0; i < (int)m_netTrafficProfilers.size(); i++)
	{
		SAFE_DELETE( m_netTrafficProfilers[i]->m_pGraph );
		SAFE_DELETE( m_netTrafficProfilers[i]->m_pOfflineHistory );
	}
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::SetProfiling(bool on, bool display, char *prefix, ISystem *pSystem)
{
	Enable( on,display );
	m_pPrefix = prefix;
	if(on && m_nCurSample<0)
	{
		m_nCurSample = 0;
		pSystem->GetILog()->Log("\001Profiling data started (%s), prefix = \"%s\"", display ? "display only" : "tracing", prefix);

		m_frameTimeOfflineHistory.m_selfTime.reserve(1000);
		m_frameTimeOfflineHistory.m_count.reserve(1000);
	}
	else if(!on && m_nCurSample>=0)
	{
		pSystem->GetILog()->Log("\001Profiling data finished");
		{
#ifdef WIN32
			// find the "frameprofileXX" filename for the file
			char outfilename[32] = "frameprofile.dat";
			// while there is such file already
			for (int i = 0; (GetFileAttributes (outfilename) != INVALID_FILE_ATTRIBUTES) && i < 1000; ++i)
				sprintf (outfilename, "frameprofile%02d.dat", i);

			FILE *f = fopen(outfilename, "wb");
			if(!f)
			{
				pSystem->GetILog()->Log("\001Could not write profiling data to file!");
			}
			else
			{
				int i;
				// Find out how many profilers was active.
				int numProf = 0;

				for (i = 0; i < (int)m_pProfilers->size(); i++)
				{
					CFrameProfiler *pProfiler = (*m_pProfilers)[i];
					if (pProfiler->m_pOfflineHistory)
						numProf++;
				}

				fwrite("FPROFDAT", 8, 1, f);                // magic header, for what its worth
				int version = 2;                            // bump this if any of the format below changes
				fwrite(&version, sizeof(int), 1, f); 

				int numSamples = m_nCurSample;
				fwrite(&numSamples, sizeof(int), 1, f);   // number of samples per group (everything little endian)
				int mapsize = numProf+1; // Plus 1 global.
				fwrite(&mapsize, sizeof(int), 1, f);

				// Write global profiler.
				fwrite( "__frametime",strlen("__frametime")+1,1, f);
				int len = (int)m_frameTimeOfflineHistory.m_selfTime.size();
				assert( len == numSamples );
				for(i = 0; i<len; i++)
				{
					fwrite( &m_frameTimeOfflineHistory.m_selfTime[i], 1, sizeof(int),   f);
					fwrite( &m_frameTimeOfflineHistory.m_count[i], 1, sizeof(short), f);
				};

				// Write other profilers.
				for (i = 0; i < (int)m_pProfilers->size(); i++)
				{
					CFrameProfiler *pProfiler = (*m_pProfilers)[i];
					if (!pProfiler->m_pOfflineHistory)
						continue;

					const char *name = pProfiler->m_name;
					//int slen = strlen(name)+1;
					fwrite(name, strlen(name)+1,1,f);
					
					len = (int)pProfiler->m_pOfflineHistory->m_selfTime.size();
					assert( len == numSamples );
					for(int i = 0; i<len; i++)
					{
						fwrite( &pProfiler->m_pOfflineHistory->m_selfTime[i], 1, sizeof(int),   f);
						fwrite( &pProfiler->m_pOfflineHistory->m_count[i], 1, sizeof(short), f);
					};

					// Delete offline data, from profiler.
					SAFE_DELETE( pProfiler->m_pOfflineHistory );
				};
				fclose(f);
				pSystem->GetILog()->Log("\001Profiling data saved to file '%s'",outfilename);
			};
#endif
		};
		m_frameTimeOfflineHistory.m_selfTime.clear();
		m_frameTimeOfflineHistory.m_count.clear();
		m_nCurSample = -1;
	};
};

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::Enable( bool bCollect,bool bDisplay )
{
	if (m_bEnabled != bCollect)
	{
		Reset();
	}
	m_bEnabled = bCollect;
	m_bDisplay = bDisplay;
	m_bDisplayedProfilersValid = false;
}

void CFrameProfileSystem::EnableHistograms( bool bEnableHistograms )
{
	if (m_bEnableHistograms != bEnableHistograms)
	{
		
	}
	m_bEnableHistograms = bEnableHistograms;
	m_bDisplayedProfilersValid = false;
}

//////////////////////////////////////////////////////////////////////////
CFrameProfiler* CFrameProfileSystem::GetProfiler( int index ) const
{
	assert( index >= 0 && index < (int)m_profilers.size() );
	return m_profilers[index];
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::Reset()
{
	m_pCurrentProfileSection = 0;
	m_pCurrentCustomSection = 0;
	m_totalProfileTime = 0;
	m_frameStartTime = 0;
	m_frameTime = 0;
	m_frameLostTime = 0;
	m_smoothFrame = 0;
	m_bCollectionPaused = false;
	
	int i;
	// Iterate over all profilers update thier history and reset them.
	for (i = 0; i < (int)m_profilers.size(); i++)
	{
		CFrameProfiler *pProfiler = m_profilers[i];
		// Reset profiler.
		pProfiler->m_totalTimeHistory.Clear();
		pProfiler->m_selfTimeHistory.Clear();
		pProfiler->m_countHistory.Clear();
		pProfiler->m_sumTotalTime = 0;
		pProfiler->m_sumSelfTime = 0;
		pProfiler->m_totalTime = 0;
		pProfiler->m_selfTime = 0;
		pProfiler->m_count = 0;
		pProfiler->m_displayedValue = 0;
		pProfiler->m_displayedCurrentValue = 0;
		pProfiler->m_variance = 0;
	}
	// Iterate over all profilers update thier history and reset them.
	for (i = 0; i < (int)m_netTrafficProfilers.size(); i++)
	{
		CFrameProfiler *pProfiler = m_netTrafficProfilers[i];
		// Reset profiler.
		pProfiler->m_totalTimeHistory.Clear();
		pProfiler->m_selfTimeHistory.Clear();
		pProfiler->m_countHistory.Clear();
		pProfiler->m_sumTotalTime = 0;
		pProfiler->m_sumSelfTime = 0;
		pProfiler->m_totalTime = 0;
		pProfiler->m_selfTime = 0;
		pProfiler->m_count = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::AddFrameProfiler( CFrameProfiler *pProfiler )
{
	if (pProfiler->m_subsystem == PROFILE_NETWORK_TRAFFIC)
	{
		m_netTrafficProfilers.push_back( pProfiler );
	}
	else
	{
		m_profilers.push_back( pProfiler );
	}
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::StartProfilerSection( CFrameProfilerSection *pSection )
{
	if (!m_bCollect)
		return;

	pSection->m_excludeTime = 0;
	pSection->m_pParent = m_pCurrentProfileSection;
	m_pCurrentProfileSection = pSection;
	CFrameProfilerTimer::GetTicks( &pSection->m_startTime );
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::EndProfilerSection( CFrameProfilerSection *pSection )
{
	if (!m_bCollect)
		return;

	int64 endTime;
	CFrameProfilerTimer::GetTicks( &endTime );
	int64 totalTime = endTime - pSection->m_startTime;
	int64 selfTime = totalTime - pSection->m_excludeTime;

	CFrameProfiler *pProfiler = pSection->m_pFrameProfiler;
	pProfiler->m_count++;
	pProfiler->m_selfTime += selfTime;
	pProfiler->m_totalTime += totalTime;

	m_pCurrentProfileSection = pSection->m_pParent;
	if (m_pCurrentProfileSection)
	{
		// If we have parent, add this counter total time to parent exclude time.
		m_pCurrentProfileSection->m_pFrameProfiler->m_bHaveChildren = true;
		m_pCurrentProfileSection->m_excludeTime += totalTime;
		pProfiler->m_pParent = m_pCurrentProfileSection->m_pFrameProfiler;
	}
	else
		pProfiler->m_pParent = 0;
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::StartCustomSection( CCustomProfilerSection *pSection )
{
	if (!m_bNetworkProfiling)
		return;

	pSection->m_excludeValue = 0;
	pSection->m_pParent = m_pCurrentCustomSection;
	m_pCurrentCustomSection = pSection;
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::EndCustomSection( CCustomProfilerSection *pSection )
{
	if (!m_bNetworkProfiling || m_bCollectionPaused)
		return;

	int total = *pSection->m_pValue;
	int self = total - pSection->m_excludeValue;

	CFrameProfiler *pProfiler = pSection->m_pFrameProfiler;
	pProfiler->m_count++;
	pProfiler->m_selfTime += self;
	pProfiler->m_totalTime += total;

	m_pCurrentCustomSection = pSection->m_pParent;
	if (m_pCurrentCustomSection)
	{
		// If we have parent, add this counter total time to parent exclude time.
		m_pCurrentCustomSection->m_pFrameProfiler->m_bHaveChildren = true;
		m_pCurrentCustomSection->m_excludeValue += total;
		pProfiler->m_pParent = m_pCurrentCustomSection->m_pFrameProfiler;
	}
	else
		pProfiler->m_pParent = 0;
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::StartFrame()
{
	m_bCollect = m_bEnabled && !m_bCollectionPaused;
	
	if (m_bCollect)
	{
		m_pCurrentProfileSection = 0;
		m_pCurrentCustomSection = 0;
		CFrameProfilerTimer::GetTicks(&m_frameStartTime);
	}
	g_bProfilerEnabled = m_bCollect;
	/*
	if (m_displayQuantity == SUBSYSTEM_INFO)
	{
		for (int i = 0; i < PROFILE_LAST_SUBSYSTEM; i++)
		{
			//m_subsystems[i].selfTime = 0;
		}
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
float CFrameProfileSystem::TranslateToDisplayValue( int64 val )
{
	if (m_bNetworkProfiling)
		return (float)val;
	else
		return CFrameProfilerTimer::TicksToMilliseconds(val);
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::EndFrame()
{
	if (!m_bEnabled && !m_bNetworkProfiling)
		return;

#ifdef WIN32

	bool bPaused = false;

	if (GetISystem()->GetIInput())
	{
		bPaused = (GetISystem()->GetIInput()->GetKeyState(XKEY_SCROLLLOCK) & 1);
	}
	// Will pause or resume collection.
	if (bPaused != m_bCollectionPaused)
	{
		if (bPaused)
		{
			// Must be paused.
			m_pSystem->GetIInput()->SetMouseExclusive( false );
		}
		else
		{
			// Must be resumed.
			m_pSystem->GetIInput()->SetMouseExclusive( true );
		}
	}
	if (m_bCollectionPaused != bPaused)
	{
		m_bDisplayedProfilersValid = false;
	}
	m_bCollectionPaused = bPaused;
#endif

	if (m_bCollectionPaused || (!m_bCollect && !m_bNetworkProfiling))
		return;

	FUNCTION_PROFILER( m_pSystem,PROFILE_SYSTEM );

	int64 endTime;
	CFrameProfilerTimer::GetTicks(&endTime);
	m_frameTime = endTime - m_frameStartTime;
	m_totalProfileTime += m_frameTime;


	//////////////////////////////////////////////////////////////////////////
	// Lets see how many page faults we got.
	//////////////////////////////////////////////////////////////////////////
#if defined(WIN32) && !defined(WIN64)

	// PSAPI is not supported on window9x
	// so, don't use it
	if (!m_bNoPsapiDll)
	{
		// Load psapi dll.
		if (!pfGetProcessMemoryInfo)
		{
			hPsapiModule = ::LoadLibrary( "psapi.dll" );
			if (hPsapiModule)
			{
				pfGetProcessMemoryInfo = (FUNC_GetProcessMemoryInfo)(::GetProcAddress(hPsapiModule,"GetProcessMemoryInfo" ));
			}
			else
				m_bNoPsapiDll = true;
		}
		if (pfGetProcessMemoryInfo)
		{
			PROCESS_MEMORY_COUNTERS pc;
			pfGetProcessMemoryInfo( GetCurrentProcess(),&pc,sizeof(pc) );
			m_nPagesFaultsLastFrame = pc.PageFaultCount - m_nLastPageFaultCount;
			m_nLastPageFaultCount = pc.PageFaultCount;
			static float fLastPFTime = 0;
			static int nPFCounter = 0;
			nPFCounter += m_nPagesFaultsLastFrame;
			float fCurr = CFrameProfilerTimer::TicksToMilliseconds(endTime);
			if ((fCurr - fLastPFTime) >= 1000)
			{
				fLastPFTime = fCurr;
				m_nPagesFaultsPerSec = nPFCounter;
				nPFCounter = 0;
			}
		}
	}
#endif
	//////////////////////////////////////////////////////////////////////////


	int64 selfAccountedTime = 0;

	//float times_to_reach_90_percent = 0.8f;
	float times_to_reach_90_percent = 0.8f;
	if (m_displayQuantity == PEAK_TIME || m_displayQuantity == COUNT_INFO+1)
	{
		//m_smoothMaxFrames = 60*5;
		times_to_reach_90_percent = 1.0f;
	}
	float dt = CFrameProfilerTimer::TicksToSeconds(m_frameTime);
	float smoothFactor = (float)pow( 0.1, dt/double(times_to_reach_90_percent) );

	m_frameTimeHistory.Add( CFrameProfilerTimer::TicksToMilliseconds(m_frameTime) );
	m_frameTimeLostHistory.Add( CFrameProfilerTimer::TicksToMilliseconds(m_frameLostTime) );

	// Iterate over all profilers update thier history and reset them.
	for (int i = 0; i < (int)m_pProfilers->size(); i++)
	{
		CFrameProfiler *pProfiler = (*m_pProfilers)[i];

		// Skip this profiler if its filtered out.
		if (m_bSubsystemFilterEnabled && pProfiler->m_subsystem != m_subsystemFilter)
			continue;
    
		selfAccountedTime += pProfiler->m_selfTime;
		pProfiler->m_sumTotalTime += pProfiler->m_totalTime;
		pProfiler->m_sumSelfTime += pProfiler->m_selfTime;

		bool bEnablePeaks = true;
		float aveValue;
		float currentValue;
		float variance;
		switch (m_displayQuantity)
		{
		case SELF_TIME:
		case PEAK_TIME:
		case COUNT_INFO:
			currentValue = TranslateToDisplayValue(pProfiler->m_selfTime);
			aveValue = pProfiler->m_selfTimeHistory.GetAverage();
			variance = (currentValue - aveValue) * (currentValue - aveValue);
			break;
		case TOTAL_TIME:
			currentValue = TranslateToDisplayValue(pProfiler->m_totalTime);
			aveValue = pProfiler->m_totalTimeHistory.GetAverage();
			variance = (currentValue - aveValue) * (currentValue - aveValue);
			break;
		case SELF_TIME_EXTENDED:
			currentValue = TranslateToDisplayValue(pProfiler->m_selfTime);
			aveValue = pProfiler->m_selfTimeHistory.GetAverage();
			variance = (currentValue - aveValue) * (currentValue - aveValue);
			bEnablePeaks = false;
			break;
		case TOTAL_TIME_EXTENDED:
			currentValue = TranslateToDisplayValue(pProfiler->m_totalTime);
			aveValue = pProfiler->m_totalTimeHistory.GetAverage();
			variance = (currentValue - aveValue) * (currentValue - aveValue);
			bEnablePeaks = false;
			break;
		case SUBSYSTEM_INFO:
			currentValue = (float)pProfiler->m_count;
			aveValue = pProfiler->m_selfTimeHistory.GetAverage();
			variance = (currentValue - aveValue) * (currentValue - aveValue);
			if (pProfiler->m_subsystem < PROFILE_LAST_SUBSYSTEM)
				m_subsystems[pProfiler->m_subsystem].selfTime += aveValue;
			break;
		case COUNT_INFO+1:
			// Standart Deviation.
			aveValue = pProfiler->m_selfTimeHistory.GetStdDeviation();
			aveValue *= 100.0f;
			currentValue = aveValue;
			variance = 0;
			break;
		};

		//////////////////////////////////////////////////////////////////////////
		// Records Peaks.
		if (bEnablePeaks)
		{
			float prevValue = pProfiler->m_selfTimeHistory.GetLast();
			float peakValue = TranslateToDisplayValue(pProfiler->m_selfTime);
			if ((peakValue-prevValue) > m_peakTolerance)
			{
				SPeakRecord peak;
				peak.pProfiler = pProfiler;
				peak.peakValue = peakValue;
				peak.avarageValue = pProfiler->m_selfTimeHistory.GetAverage();
				peak.count = pProfiler->m_count;
				peak.pageFaults = m_nPagesFaultsLastFrame;
				peak.when = CFrameProfilerTimer::TicksToSeconds(m_totalProfileTime);
				AddPeak( peak );

				// Call peak callbacks.
				if (!m_peakCallbacks.empty())
				{
					for (int i = 0; i < (int)m_peakCallbacks.size(); i++)
					{
						m_peakCallbacks[i]->OnFrameProfilerPeak( pProfiler,peakValue );
					}
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////

		pProfiler->m_totalTimeHistory.Add( TranslateToDisplayValue(pProfiler->m_totalTime) );
		pProfiler->m_selfTimeHistory.Add( TranslateToDisplayValue(pProfiler->m_selfTime) );
		pProfiler->m_countHistory.Add( pProfiler->m_count );
		
		pProfiler->m_displayedCurrentValue = aveValue;
		//if (m_smoothFrame < m_smoothMaxFrames)
		{
			pProfiler->m_displayedValue = pProfiler->m_displayedValue*smoothFactor + aveValue*(1.0f - smoothFactor);
			pProfiler->m_variance = pProfiler->m_variance*smoothFactor + variance*(1.0f - smoothFactor);
		}
		//else
		{
			//pProfiler->m_displayedValue = value;
			//pProfiler->m_variance = variance;
		}

		if (m_bEnableHistograms)
		{
			if (!pProfiler->m_pGraph)
			{
				// Create graph.
				pProfiler->m_pGraph = new CFrameProfilerGraph;
			}
			// Update values in histogram graph.
			if (m_histogramsMaxPos != pProfiler->m_pGraph->m_data.size())
			{
				pProfiler->m_pGraph->m_width = m_histogramsMaxPos;
				pProfiler->m_pGraph->m_height = m_histogramsHeight;
				pProfiler->m_pGraph->m_data.resize( m_histogramsMaxPos );
			}
			float millis;
			if (m_displayQuantity == TOTAL_TIME || m_displayQuantity == TOTAL_TIME_EXTENDED)
				millis = m_histogramScale * pProfiler->m_totalTimeHistory.GetLast();
			else
				millis = m_histogramScale * pProfiler->m_selfTimeHistory.GetLast();
			if (millis < 0) millis = 0;
			if (millis > 255) millis = 255;
			pProfiler->m_pGraph->m_data[m_histogramsCurrPos] = 255-FtoI(millis); // must use ftoi.
		}

		if (m_nCurSample >= 0)
		{
			UpdateOfflineHistory( pProfiler );
		}

		// Reset profiler.
		pProfiler->m_totalTime = 0;
		pProfiler->m_selfTime = 0;
		pProfiler->m_count = 0;
	}

	if (m_smoothFrame >= m_smoothMaxFrames)
	{
		m_smoothFrame = 0;
	}
	else
	{
		m_smoothFrame++;
	}

	m_frameLostTime = m_frameTime - selfAccountedTime;

	if (m_nCurSample >= 0)
	{
		// Keep offline global time history.
		m_frameTimeOfflineHistory.m_selfTime.push_back( FtoI(CFrameProfilerTimer::TicksToMilliseconds(m_frameTime)*1000) );
		m_frameTimeOfflineHistory.m_count.push_back(1);
		m_nCurSample++;
	}
	//AdvanceFrame( m_pSystem );
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::UpdateOfflineHistory( CFrameProfiler *pProfiler )
{
	if (!pProfiler->m_pOfflineHistory)
	{
		pProfiler->m_pOfflineHistory = new CFrameProfilerOfflineHistory;
		pProfiler->m_pOfflineHistory->m_count.reserve( 1000+m_nCurSample*2 );
		pProfiler->m_pOfflineHistory->m_selfTime.reserve( 1000+m_nCurSample*2 );
	}
	int prevCont = (int)pProfiler->m_pOfflineHistory->m_selfTime.size();
	int newCount = m_nCurSample+1;
	pProfiler->m_pOfflineHistory->m_selfTime.resize( newCount );
	pProfiler->m_pOfflineHistory->m_count.resize( newCount );

	unsigned int micros = FtoI(CFrameProfilerTimer::TicksToMilliseconds(pProfiler->m_selfTime)*1000);
	unsigned short count = pProfiler->m_count;
	for (int i = prevCont; i < newCount; i++)
	{
		pProfiler->m_pOfflineHistory->m_selfTime[i] = micros;
		pProfiler->m_pOfflineHistory->m_count[i] = count;
	}
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::AddPeak( SPeakRecord &peak )
{
	// Add peak.
	if (m_peaks.size() > MAX_PEAK_PROFILERS)
		m_peaks.pop_back();

	if(m_pSystem->IsDedicated())
		m_pSystem->GetILog()->Log("Peak: name:'%s' val:%.2f avg:%.2f cnt:%d",peak.pProfiler->m_name,peak.peakValue,peak.avarageValue,peak.count);

	/*
	// Check to see if this function is already a peak.
	for (int i = 0; i < (int)m_peaks.size(); i++)
	{
		if (m_peaks[i].pProfiler == peak.pProfiler)
		{
			m_peaks.erase( m_peaks.begin()+i );
			break;
		}
	}
	*/
	m_peaks.insert( m_peaks.begin(),peak );
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::SetDisplayQuantity( EDisplayQuantity quantity )
{
	m_displayQuantity = quantity;
	m_bDisplayedProfilersValid = false;
	if (m_displayQuantity == SELF_TIME_EXTENDED || m_displayQuantity == TOTAL_TIME_EXTENDED)
		EnableHistograms(true);
	else
		EnableHistograms(false);
};

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::SetSubsystemFilter( bool bFilterSubsystem,EProfiledSubsystem subsystem )
{
	m_bSubsystemFilterEnabled = bFilterSubsystem;
	m_subsystemFilter = subsystem;
	m_bDisplayedProfilersValid = false;
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::SetSubsystemFilter( const char *szFilterName )
{
	bool bFound = false;
	for (int i = 0; i < PROFILE_LAST_SUBSYSTEM; i++)
	{
		if (!m_subsystems[i].name)
			continue;
		if (stricmp(m_subsystems[i].name,szFilterName) == 0)
		{
			SetSubsystemFilter( true,(EProfiledSubsystem)i );
			bFound = true;
			break;
		}
	}
	if (!bFound)
		SetSubsystemFilter( false,PROFILE_ANY );
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::AddPeaksListener( IFrameProfilePeakCallback *pPeakCallback )
{
	// Only add one time.
	stl::push_back_unique( m_peakCallbacks,pPeakCallback );
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::RemovePeaksListener( IFrameProfilePeakCallback *pPeakCallback )
{
	stl::find_and_erase( m_peakCallbacks,pPeakCallback );
}

//////////////////////////////////////////////////////////////////////////
void CFrameProfileSystem::EnableMemoryProfile( bool bEnable )
{
	if (bEnable != m_bDisplayMemoryInfo)
		m_bLogMemoryInfo = true;
	m_bDisplayMemoryInfo = bEnable;
}

#endif