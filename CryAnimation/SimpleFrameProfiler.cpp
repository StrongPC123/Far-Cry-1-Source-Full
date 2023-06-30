#if 0 /// obsolete

#include "stdafx.h"
#include "SimpleFrameProfiler.h"
#include "CVars.h"

static float g_fColorWhite[4] = {1,1,1,1};
static float g_fColorHeader[4] = {0.5f,0.9f,1,0.75f};
static float g_fColorSecondary[4] = {1,1,1,0.6f};

CRecursiveFrameProfiler* CRecursiveFrameProfiler::g_arrStack[CRecursiveFrameProfiler::nStackDepth];
int CRecursiveFrameProfiler::g_nStackTop = 0;

CProfilerTimer g_ProfilerTimer;

CSimpleFrameProfilerInfo::CSimpleFrameProfilerInfo (const char* szName):
	m_szName (szName),
	m_nTickTimer (0)
{
	m_nIndex = g_nCount * 2;
	++g_nCount;
}

int CSimpleFrameProfilerInfo::g_nCount = 0;

// this is called when the monitored interval starts
void CSimpleFrameProfilerInfo::startInterval()
{
	flush();
	m_nTickTimer -= g_ProfilerTimer.getTicks();
	++m_nCounter;
}

void CSimpleFrameProfilerInfo::flush()
{
	int nCurrentFrame = GetRenderer()->GetFrameID();
	if (nCurrentFrame != m_nFrame)
	{

		// draw the header
		if (m_nIndex == 0)
			drawHeaderLabel();

		drawStatistics (float(m_nIndex), g_fColorWhite, m_szName, m_HistTime);
		drawStatistics (float(m_nIndex+0.9f), g_fColorSecondary, "# of calls", m_HistCount);

		// clean up statistics if the frames didn't go in sequence,
		// or add to statistics if they did
		if (nCurrentFrame - m_nFrame > 16)
		{
			m_HistTime.clear();
			m_HistCount.clear();
		}
		else
		{
			m_HistTime.add(g_ProfilerTimer.ticksToMilliseconds(m_nTickTimer));
			m_HistCount.add((float)m_nCounter);
		}

		m_nTickTimer = 0;
		m_nFrame = nCurrentFrame;
		m_nCounter = 0;
	}
}


void CSimpleFrameProfilerInfo::endInterval()
{
	m_nTickTimer += g_ProfilerTimer.getTicks();
}

void CSimpleFrameProfilerInfo::startDelay()
{
	m_nTickTimer += g_ProfilerTimer.getTicks();
}
void CSimpleFrameProfilerInfo::endDelay()
{
	m_nTickTimer -= g_ProfilerTimer.getTicks();
}


void CSimpleFrameProfilerInfo::drawLabel (float fRow, float* fColor, const char* szText)
{
	GetRenderer()->Draw2dLabel (1, fRow*16+100, 1.4f, fColor, false, "%s", szText);
}

void CSimpleFrameProfilerInfo::drawHeaderLabel()
{
	drawLabel (-1.25f, g_fColorHeader, "                          max   ave   ave8  poi16 now  min");
}

void CSimpleFrameProfilerInfo::drawStatistics (float fRow, float* fColor, const char* szLabel, CProfilerTimerHistory<float,64>& rProfiler)
{
	// draw the profiler statistics gathered on the previous frame
	char szBuf[128];
	sprintf (szBuf, "%22s  %6.2f%6.2f%6.2f%6.2f%6.2f%6.2f",
		szLabel,
		rProfiler.getMax (),
		rProfiler.getAve (),
		rProfiler.getAve (8),
		rProfiler.getAvePoisson (16,0.85f),
		rProfiler.getLast(),
		rProfiler.getMin()
	);
	drawLabel (fRow, fColor, szBuf);
}




__int64 CProfilerTimer::g_nTicksPerSecond = 1000000000;
double CProfilerTimer::g_fSecondsPerTick = 1e-9;
double CProfilerTimer::g_fMilliSecondsPerTick = 1e-6;
unsigned CProfilerTimer::g_nCPUHerz = 1000000000;
void CProfilerTimer::init() // called once
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



void CProfilerTimer::getTicks(__int64* pnTime)
{
//QueryPerformanceCounter((LARGE_INTEGER*)&nTime);
#if defined(WIN64)
	// AMD64 compiler intrinsic
	*pnTime = __rdtsc();
#elif defined(_CPU_X86)
	__asm {
		mov ebx, pnTime
		rdtsc
		mov [ebx], eax
		mov [ebx+4], edx
	}
#elif defined(GAMECUBE)
	//I know it looks strange, but this are the cycles!
	pnTime = (s64*)(OSGetTime()*12);
#else
#error // plese define this for your target platform
#endif
}



float CProfilerTimer::ticksToSeconds (__int64 nTime)
{
	return float(g_fSecondsPerTick * nTime);
}

float CProfilerTimer::ticksToMilliseconds (__int64 nTime)
{
	return float(g_fMilliSecondsPerTick * nTime);
}

#endif  // 0
