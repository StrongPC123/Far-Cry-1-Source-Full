
#include "stdafx.h"
#include "System.h"
#include <time.h>
//#include "ini_vars.h"

#include <INetwork.h>
#include <I3DEngine.h>
#include <IAISystem.h>
#include <IRenderer.h>
#include <CryMemoryManager.h>
#include <ICryPak.h>
#include <IMovieSystem.h>
#include <IEntitySystem.h>
#include <IInput.h>
#include <ILog.h>
#include <ISound.h>
#include <IScriptSystem.h>
#include <IGame.h>

#ifndef _XBOX
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
#else
#include <xtl.h>
#endif

#include "SourceSafeHelper.h"				// _GetSSFileInfo

#ifdef WIN32
#include "DebugCallStack.h"
#endif

#ifdef WIN32
#include "luadebugger/luadbginterface.h"
#include "luadebugger/LuaDbg.h"
#endif

#include "XConsole.h"

#include "CrySizerStats.h"
#include "CrySizerImpl.h"

// this is the list of modules that can be loaded into the game process
// Each array element contains 2 strings: the name of the module (case-insensitive)
// and the name of the group the module belongs to
//////////////////////////////////////////////////////////////////////////
const char g_szGroupCore[] = "CryENGINE";
const char g_szGroupGame[] = "CryGAME";
const char g_szGroupSupport[] = "System";
const char g_szGroupWindows[] = "System";
const char* g_szModuleGroups[][2] = {
	{"FarCry.exe", g_szGroupGame},
	{"FarCry_WinSV.exe", g_szGroupGame},
	{"Editor.exe", g_szGroupGame},
	{"CrySystem.dll", g_szGroupCore},
	{"CryScriptSystem.dll", g_szGroupGame},
	{"CryNetwork.dll", g_szGroupGame},
	{"CryPhysics.dll", g_szGroupCore},
	{"CryMovie.dll", g_szGroupCore},
	{"CryInput.dll", g_szGroupCore},
	{"CrySoundSystem.dll", g_szGroupCore},
#ifdef WIN64
	{"crysound64.dll", g_szGroupCore},
#else
	{"crysound.dll", g_szGroupCore},
#endif
	{"CryFont.dll", g_szGroupCore},
	{"CryAISystem.dll", g_szGroupGame},
	{"CryEntitySystem.dll", g_szGroupCore},
	{"Cry3DEngine.dll", g_szGroupCore},
	{"CryGame.dll", g_szGroupGame},
	{"CryAnimation.dll", g_szGroupCore},
	{"XRenderD3D9.dll", g_szGroupCore},
	{"XRenderOGL.dll", g_szGroupCore},
	{"XRenderNULL.dll", g_szGroupCore}
};


//////////////////////////////////////////////////////////////////////////
void CSystem::SetAffinity()
{
	// the following code is only for Windows
#ifdef WIN32
	// set the process affinity
	ICVar* pcvAffinityMask = GetIConsole()->GetCVar("sys_affinity");
	if (!pcvAffinityMask)
		pcvAffinityMask = GetIConsole()->CreateVariable("sys_affinity","0", 0);

	if (pcvAffinityMask)
	{
		unsigned nAffinity = pcvAffinityMask->GetIVal();
		if (nAffinity)
		{
			typedef BOOL (WINAPI *FnSetProcessAffinityMask)(IN HANDLE hProcess,IN DWORD_PTR dwProcessAffinityMask);
			HMODULE hKernel = CryLoadLibrary ("kernel32.dll");
			if (hKernel)
			{
				FnSetProcessAffinityMask SetProcessAffinityMask = (FnSetProcessAffinityMask)GetProcAddress(hKernel, "SetProcessAffinityMask");
				if (SetProcessAffinityMask && !SetProcessAffinityMask(GetCurrentProcess(), nAffinity))
					GetILog()->LogToFile ("\003Error: Cannot set affinity mask %d, error code %d", nAffinity, GetLastError());
				FreeLibrary (hKernel);
			}
		}
	}
#endif
}

//! dumps the memory usage statistics to the log
//////////////////////////////////////////////////////////////////////////
void CSystem::DumpMemoryUsageStatistics()
{
	TickMemStats(nMSP_ForDump);
	/*
	CrySizerImpl Sizer;
	CrySizerStats MemStats;

	MemStats.startTimer(0,GetITimer());
	CollectMemStats (&Sizer);
	MemStats.stopTimer(0,GetITimer());

	MemStats.startTimer(1,GetITimer());
	CrySizerStatsBuilder builder (&Sizer);
	builder.build (&MemStats);
	MemStats.stopTimer(1,GetITimer());

	MemStats.startTimer(2,GetITimer());
	Sizer.clear();
	MemStats.stopTimer(2,GetITimer());

	*/
	
	CrySizerStatsRenderer StatsRenderer (this, m_pMemStats, 10, 0);
	StatsRenderer.dump();
	
	// since we've recalculated this mem stats for dumping, we'll want to calculate it anew the next time it's rendered
	SAFE_DELETE(m_pMemStats);
}

// collects the whole memory statistics into the given sizer object
//////////////////////////////////////////////////////////////////////////
void CSystem::CollectMemStats (CrySizerImpl* pSizer, MemStatsPurposeEnum nPurpose)
{
#ifdef WIN32
	{
		SIZER_COMPONENT_NAME(pSizer, "Code");
		GetExeSizes (pSizer, nPurpose);
	}
#endif

	{
		SIZER_COMPONENT_NAME(pSizer, "VFS");
		if (m_pStreamEngine)
		{
			SIZER_COMPONENT_NAME(pSizer, "Stream Engine");
			m_pStreamEngine->GetMemoryStatistics(pSizer);
		}
		if (m_pIPak)
		{
			SIZER_COMPONENT_NAME(pSizer, "CryPak");
			m_pIPak->GetMemoryStatistics(pSizer);
		}
	}

	if (m_pI3DEngine)
	{
		SIZER_COMPONENT_NAME(pSizer, "3DEngine");
		m_pI3DEngine->GetMemoryUsage (pSizer);
	}

	if (m_pICryCharManager)
	{
		SIZER_COMPONENT_NAME(pSizer, "Animation");
		m_pICryCharManager->GetMemoryUsage(pSizer);
	}

	if (m_pIPhysicalWorld)
	{
		SIZER_COMPONENT_NAME(pSizer, "Physics");
		m_pIPhysicalWorld->GetMemoryStatistics (pSizer);
	}
	
	assert (m_pRenderer);
	{
		SIZER_COMPONENT_NAME(pSizer, "Renderer");
		m_pRenderer->GetMemoryUsage (pSizer);
	}

	if (m_pICryFont)
	{
		SIZER_COMPONENT_NAME(pSizer, "Fonts");
		m_pICryFont->GetMemoryUsage(pSizer);
	}

	if (m_pConsole)
	{
		SIZER_COMPONENT_NAME (pSizer, "Console");
		m_pConsole->GetMemoryUsage (pSizer);
	}

	if (m_pISound)
	{
		SIZER_COMPONENT_NAME(pSizer, "Sound");
		m_pISound->GetMemoryUsage(pSizer);
	}

	if (m_pIMusic)
	{
		SIZER_COMPONENT_NAME(pSizer, "Music");
		m_pIMusic->GetMemoryUsage(pSizer);
	}

	if (m_pScriptSystem)
	{
		SIZER_COMPONENT_NAME(pSizer, "Script");
		m_pScriptSystem->GetMemoryStatistics(pSizer);
	}

	if (m_pAISystem)
	{
		SIZER_COMPONENT_NAME(pSizer, "AI");
		m_pAISystem->GetMemoryStatistics (pSizer);
	}

	if (m_pGame)
	{
		SIZER_COMPONENT_NAME(pSizer, "Game");
		m_pGame->GetMemoryStatistics (pSizer);
	}

	if (m_pNetwork)
	{
		SIZER_COMPONENT_NAME(pSizer, "Network");
		m_pNetwork->GetMemoryStatistics(pSizer);
	}

	if (m_pEntitySystem)
	{
		SIZER_COMPONENT_NAME(pSizer, "Entities");
		m_pEntitySystem->GetMemoryStatistics(pSizer);
	}
 
	pSizer->end();
}

//////////////////////////////////////////////////////////////////////////
const char *CSystem::GetUserName()
{
	static char						szNameBuffer[1024];
	memset(szNameBuffer, 0, 1024);

	DWORD dwSize = 1024;

	::GetUserName(szNameBuffer, &dwSize);

	return szNameBuffer;
}

// refreshes the m_pMemStats if necessary; creates it if it's not created
//////////////////////////////////////////////////////////////////////////
void CSystem::TickMemStats(MemStatsPurposeEnum nPurpose)
{
	// gather the statistics, if required
	// if there's  no object, or if it's time to recalculate, or if it's for dump, then recalculate it
	if (!m_pMemStats || (m_pRenderer->GetFrameID()%m_cvMemStats->GetIVal())==0 || nPurpose == nMSP_ForDump)
	{
		if (!m_pMemStats)
		{
			if (m_cvMemStats->GetIVal() < 4 && m_cvMemStats->GetIVal())
				GetILog()->LogToConsole ("memstats is too small (%d). Performnce impact can be significant. Please set to a greater value.",m_cvMemStats->GetIVal());
			m_pMemStats = new CrySizerStats();
		}

		if (!m_pSizer)
			m_pSizer = new CrySizerImpl();

		m_pMemStats->startTimer(0,GetITimer());
		CollectMemStats (m_pSizer,nPurpose);
		m_pMemStats->stopTimer(0,GetITimer());

		m_pMemStats->startTimer(1,GetITimer());
		CrySizerStatsBuilder builder (m_pSizer);
		builder.build (m_pMemStats);
		m_pMemStats->stopTimer(1,GetITimer());

		m_pMemStats->startTimer(2,GetITimer());
		m_pSizer->clear();
		m_pMemStats->stopTimer(2,GetITimer());
	}
	else
		m_pMemStats->incAgeFrames();
}

//#define __HASXP

// these 2 functions are duplicated in System.cpp in editor
//////////////////////////////////////////////////////////////////////////
#if !defined(LINUX)
extern int CryStats(char *buf);
#endif
int CSystem::DumpMMStats(bool log)
{
#if defined(LINUX)
	return 0;
#else
	if(log)
	{
		char buf[1024];
		int n = CryStats(buf);
		GetILog()->Log(buf);
		return n;
	}
	else
	{
		return CryStats(NULL);
	};
#endif
};   

//////////////////////////////////////////////////////////////////////////
struct CryDbgModule
{
	HANDLE heap;
	WIN_HMODULE handle;
	string name;
	DWORD dwSize;
};

//////////////////////////////////////////////////////////////////////////
void CSystem::DebugStats(bool checkpoint, bool leaks)
{
#ifdef WIN32
	std::vector<CryDbgModule> dbgmodules;

/*
	{
		{ NULL, (WIN_HMODULE)GetModuleHandle("CrySystem.dll"), "SYSTEM"},
#ifdef _WIN32		
		{ NULL, (WIN_HMODULE)GetModuleHandle("Editor.exe"), "EDITOR"},
		{ NULL, (WIN_HMODULE)GetModuleHandle("FarCry.exe"), "FARCRY"},
#endif
		{ NULL, m_dll.hNetwork,      "NETWORK" },
		{ NULL, m_dll.hGame,         "GAME" },
		{ NULL, m_dll.hAI,           "AI" },
		{ NULL, m_dll.hEntitySystem, "ENTITY" },
		{ NULL, m_dll.hRenderer,     "RENDERER" },
		{ NULL, m_dll.hInput,        "INPUT" },
		{ NULL, m_dll.hSound,        "SOUND" },
		{ NULL, m_dll.hPhysics,      "PHYSICS" },
		{ NULL, m_dll.hFont,         "FONT" },
		{ NULL, m_dll.hScript,       "SCRIPT" },
		{ NULL, m_dll.h3DEngine,     "3DENGINE" },
		//{ NULL, (WIN_HMODULE)GetModuleHandle("CrySystem.dll"), "SYSTEM"},
		{ NULL, m_dll.hAnimation,    "ANIMATION" },
#ifdef WIN64
		{ NULL, LoadDLL("crysound64.dll"),        "FMOD" }     // temp!
#else
		{ NULL, LoadDLL("crysound.dll"),        "FMOD" }     // temp!
#endif
		// missing: OPENGL
	}; 
*/

	//////////////////////////////////////////////////////////////////////////
	// Use windows Performance Monitoring API to enumerate all modules of current process.
	//////////////////////////////////////////////////////////////////////////
	HANDLE hSnapshot;
	hSnapshot = CreateToolhelp32Snapshot (TH32CS_SNAPMODULE, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 me;
		memset (&me, 0, sizeof(me));
		me.dwSize = sizeof(me);

		if (Module32First (hSnapshot, &me))
		{
			// the sizes of each module group
			do
			{
				CryDbgModule module;
				module.handle = me.hModule;
				module.name = me.szModule;
				module.dwSize = me.modBaseSize;
				dbgmodules.push_back( module );
			}
			while(Module32Next (hSnapshot, &me));
		}
		CloseHandle (hSnapshot);
	}
	//////////////////////////////////////////////////////////////////////////

	
	ILog *log = GetILog();
	int totalal = 0, totalbl = 0, nolib = 0;

#ifdef _DEBUG
	int extrastats[10];
#endif
	
	int totalUsedInModules = 0;
	int countedMemoryModules = 0;
	for(int i = 0; i < (int)(dbgmodules.size()); i++)
	{
		if(!dbgmodules[i].handle)
		{
			CryLogAlways( "WARNING: <CrySystem> CSystem::DebugStats: NULL handle for %s", dbgmodules[i].name.c_str() );
			nolib++;
			continue; 
		};

		typedef int (*PFN_MODULEMEMORY)();
		PFN_MODULEMEMORY fpCryModuleGetAllocatedMemory = (PFN_MODULEMEMORY)::GetProcAddress(dbgmodules[i].handle, "CryModuleGetAllocatedMemory");
		if (fpCryModuleGetAllocatedMemory)
		{
			int allocatedMemory = fpCryModuleGetAllocatedMemory();
			totalUsedInModules += allocatedMemory;
			countedMemoryModules++;
			CryLogAlways("%8d K used in Module %s: ",allocatedMemory/1024,dbgmodules[i].name.c_str() );
		}
		
#ifdef _DEBUG
		typedef void (*PFNUSAGESUMMARY)(ILog *log, const char *, int *);
		typedef void (*PFNCHECKPOINT)();
		PFNUSAGESUMMARY fpu = (PFNUSAGESUMMARY)::GetProcAddress(dbgmodules[i].handle, "UsageSummary");
		PFNCHECKPOINT fpc = (PFNCHECKPOINT)::GetProcAddress(dbgmodules[i].handle, "CheckPoint");
		if(fpu && fpc)
		{
			if(checkpoint) fpc();
			else
			{
				extrastats[2] = (int)leaks;
				fpu(log, dbgmodules[i].name.c_str(), extrastats);
				totalal += extrastats[0];
				totalbl += extrastats[1];
			};

		}
		else
		{
			CryLogAlways( "WARNING: <CrySystem> CSystem::DebugStats: could not retrieve function from DLL %s", dbgmodules[i].name.c_str());
			nolib++;
		};
#endif

		typedef HANDLE(*PFNGETDLLHEAP)();
		PFNGETDLLHEAP fpg = (PFNGETDLLHEAP)::GetProcAddress(dbgmodules[i].handle, "GetDLLHeap");
		if(fpg)
		{
			dbgmodules[i].heap = fpg();
		};
	};

	CryLogAlways("-------------------------------------------------------" );
	CryLogAlways("%8d K Total Memory Allocated in %d Modules",totalUsedInModules/1024,countedMemoryModules );
#ifdef _DEBUG
	CryLogAlways("$8GRAND TOTAL: %d k, %d blocks (%d dlls not included)", totalal/1024, totalbl, nolib);
	CryLogAlways("estimated debugalloc overhead: between %d k and %d k", totalbl*36/1024, totalbl*72/1024);
#endif

	//////////////////////////////////////////////////////////////////////////
	// Get HeapQueryInformation pointer if on windows XP.
	//////////////////////////////////////////////////////////////////////////
	typedef BOOL (WINAPI *FUNC_HeapQueryInformation)( HANDLE,HEAP_INFORMATION_CLASS,PVOID,SIZE_T,PSIZE_T );
	FUNC_HeapQueryInformation pFnHeapQueryInformation = NULL;
	HMODULE hKernelInstance = CryLoadLibrary(_T("Kernel32.dll"));
	if (hKernelInstance)
	{
		pFnHeapQueryInformation = (FUNC_HeapQueryInformation)(::GetProcAddress(hKernelInstance,"HeapQueryInformation" ));
	}
	//////////////////////////////////////////////////////////////////////////

	const int MAXHANDLES = 100;
	HANDLE handles[MAXHANDLES];
	int realnumh = GetProcessHeaps(MAXHANDLES, handles);
	char hinfo[1024];
	PROCESS_HEAP_ENTRY phe;
	CryLogAlways("$6--------------------- dump of windows heaps ---------------------");
	int nTotalC = 0, nTotalCP = 0, nTotalUC = 0, nTotalUCP = 0, totalo = 0;
	for(int i = 0; i<realnumh; i++)
	{
		HANDLE hHeap = handles[i];
		HeapCompact(hHeap, 0);
		hinfo[0] = 0;
		if (pFnHeapQueryInformation)
		{
			pFnHeapQueryInformation(hHeap, HeapCompatibilityInformation, hinfo, 1024, NULL);
		}
		else
		{
			for(int m = 0; m < (int)(dbgmodules.size()); m++)
			{
				if(dbgmodules[m].heap==handles[i]) strcpy(hinfo, dbgmodules[m].name.c_str());
			}
		}
		phe.lpData = NULL;
		int nCommitted = 0, nUncommitted = 0, nOverhead = 0;
		int nCommittedPieces = 0, nUncommittedPieces = 0;
		int nPrevRegionIndex = -1;
		while(HeapWalk(hHeap, &phe))
		{	
			if (phe.wFlags & PROCESS_HEAP_REGION)
			{
				assert (++nPrevRegionIndex == phe.iRegionIndex);
				nCommitted += phe.Region.dwCommittedSize;
				nUncommitted +=  phe.Region.dwUnCommittedSize;
				assert (phe.cbData == 0 || (phe.wFlags & PROCESS_HEAP_ENTRY_BUSY));
			}
			else
				if (phe.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE)
					nUncommittedPieces += phe.cbData;
				else
					//if (phe.wFlags & PROCESS_HEAP_ENTRY_BUSY)
					nCommittedPieces += phe.cbData;


			{
				/*
				MEMORY_BASIC_INFORMATION mbi;
				if (VirtualQuery(phe.lpData, &mbi,sizeof(mbi)) == sizeof(mbi))
				{
				if (mbi.State == MEM_COMMIT)
				nCommittedPieces += phe.cbData;//mbi.RegionSize;
				//else
				//	nUncommitted += mbi.RegionSize;
				}
				else
				nCommittedPieces += phe.cbData;
				*/
			}

			nOverhead += phe.cbOverhead;
		};
		int nCommittedMin = min(nCommitted, nCommittedPieces);
		int nCommittedMax = max(nCommitted, nCommittedPieces);
		CryLogAlways("* heap %8x: %6d (or ~%6d) K in use, %6d..%6d K uncommitted, %6d K overhead (%s)\n",
			handles[i], nCommittedPieces/1024, nCommitted/1024, nUncommittedPieces/1024, nUncommitted/1024, nOverhead/1024, hinfo);

		nTotalC += nCommitted;
		nTotalCP += nCommittedPieces;
		nTotalUC += nUncommitted;
		nTotalUCP += nUncommittedPieces;
		totalo += nOverhead;
	};
	CryLogAlways("$6----------------- total in heaps: %d megs committed (win stats shows ~%d) (%d..%d uncommitted, %d k overhead) ---------------------", nTotalCP/1024/1024, nTotalC/1024/1024, nTotalUCP/1024/1024, nTotalUC/1024/1024, totalo/1024);

#endif //WIN32
};

#ifdef WIN32
//////////////////////////////////////////////////////////////////////////
void CSystem::DumpHeap32 (const HEAPLIST32& hl, DumpHeap32Stats&stats)
{
	HEAPENTRY32 he;
	memset (&he,0, sizeof(he));
	he.dwSize = sizeof(he);

	if (Heap32First (&he, hl.th32ProcessID, hl.th32HeapID))
	{
		DumpHeap32Stats heap;
		do {
			if (he.dwFlags & LF32_FREE)
				heap.dwFree += he.dwBlockSize;
			else
			if (he.dwFlags & LF32_MOVEABLE)
				heap.dwMoveable += he.dwBlockSize;
			else
			if (he.dwFlags & LF32_FIXED)
			{
				heap.dwFixed += he.dwBlockSize;
			}
			else
				heap.dwUnknown += he.dwBlockSize;
		} while(Heap32Next (&he));

		CryLogAlways ("%08X  %6d %6d %6d (%d)", hl.th32HeapID, heap.dwFixed/0x400, heap.dwFree/0x400, heap.dwMoveable/0x400, heap.dwUnknown/0x400);
		stats += heap;
	}
	else
		CryLogAlways ("%08X  empty or invalid");
}

//////////////////////////////////////////////////////////////////////////
class CStringOrder
{
public:
	bool operator () (const char*szLeft, const char* szRight)const {return stricmp(szLeft, szRight) < 0;}
};
typedef std::map<const char*,unsigned,CStringOrder> StringToSizeMap;
void AddSize (StringToSizeMap& mapSS, const char* szString, unsigned nSize)
{
	StringToSizeMap::iterator it = mapSS.find (szString);
	if (it == mapSS.end())
		mapSS.insert (StringToSizeMap::value_type(szString, nSize));
	else
		it->second += nSize;
}

//////////////////////////////////////////////////////////////////////////
const char* GetModuleGroup (const char* szString)
{
	for (unsigned i = 0; i < sizeof(g_szModuleGroups)/sizeof(g_szModuleGroups[0]); ++i)
		if (stricmp(szString, g_szModuleGroups[i][0]) == 0)
			return g_szModuleGroups[i][1];
	return "Other";
}

//////////////////////////////////////////////////////////////////////////
void CSystem::GetExeSizes (ICrySizer* pSizer, MemStatsPurposeEnum nPurpose)
{
	HANDLE hSnapshot;
	hSnapshot = CreateToolhelp32Snapshot (TH32CS_SNAPMODULE, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		CryLogAlways ("Cannot get the module snapshot, error code %d", GetLastError());
		return;
	}

	DWORD dwProcessID = GetCurrentProcessId();

	MODULEENTRY32 me;
	memset (&me, 0, sizeof(me));
	me.dwSize = sizeof(me);

	if (Module32First (hSnapshot, &me))
	{
		// the sizes of each module group
		StringToSizeMap mapGroupSize; 
		DWORD dwTotalModuleSize = 0;
		do
		{
			dwProcessID = me.th32ProcessID;
			const char* szGroup = GetModuleGroup (me.szModule);
			SIZER_COMPONENT_NAME(pSizer, szGroup);
			if (nPurpose == nMSP_ForDump)
			{
				SIZER_COMPONENT_NAME(pSizer, me.szModule);
				pSizer->AddObject(me.modBaseAddr, me.modBaseSize);
			}
			else
				pSizer->AddObject(me.modBaseAddr, me.modBaseSize);
		}
		while(Module32Next (hSnapshot, &me));
	}
	else
		CryLogAlways ("No modules to dump");

	CloseHandle (hSnapshot);
}

#endif

//////////////////////////////////////////////////////////////////////////
void CSystem::DumpWinHeaps()
{
#ifdef WIN32
	//
	// Retrieve modules and log them; remember the process id

	HANDLE hSnapshot;
	hSnapshot = CreateToolhelp32Snapshot (TH32CS_SNAPMODULE, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		CryLogAlways ("Cannot get the module snapshot, error code %d", GetLastError());
		return;
	}

	DWORD dwProcessID = GetCurrentProcessId();

	MODULEENTRY32 me;
	memset (&me, 0, sizeof(me));
	me.dwSize = sizeof(me);

	if (Module32First (hSnapshot, &me))
	{
		// the sizes of each module group
		StringToSizeMap mapGroupSize; 
		DWORD dwTotalModuleSize = 0;
		CryLogAlways ("base        size  module");
		do
		{
			dwProcessID = me.th32ProcessID;
			const char* szGroup = GetModuleGroup (me.szModule);
			CryLogAlways ("%08X %8X  %25s   - %s", me.modBaseAddr, me.modBaseSize, me.szModule, stricmp(szGroup,"Other")?szGroup:"");
			dwTotalModuleSize += me.modBaseSize;
			AddSize (mapGroupSize, szGroup, me.modBaseSize);
		}
		while(Module32Next (hSnapshot, &me));

		CryLogAlways ("------------------------------------");
		for (StringToSizeMap::iterator it = mapGroupSize.begin(); it != mapGroupSize.end(); ++it)
			CryLogAlways ("         %6.3f Mbytes  - %s", double(it->second)/0x100000, it->first);
		CryLogAlways ("------------------------------------");
		CryLogAlways ("         %6.3f Mbytes  - TOTAL", double(dwTotalModuleSize)/0x100000);
		CryLogAlways ("------------------------------------");
	}
	else
		CryLogAlways ("No modules to dump");

	CloseHandle (hSnapshot);

	//
	// Retrieve the heaps and dump each of them with a special function

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		CryLogAlways ("Cannot get the heap LIST snapshot, error code %d", GetLastError());
		return;
	}

	HEAPLIST32 hl;
	memset (&hl, 0, sizeof(hl));
	hl.dwSize = sizeof(hl);

	CryLogAlways ("__Heap__   fixed   free   move (unknown)");
	if (Heap32ListFirst (hSnapshot, &hl))
	{
		DumpHeap32Stats stats;
		do {
			DumpHeap32 (hl, stats);
		} while(Heap32ListNext (hSnapshot,&hl));

		CryLogAlways ("-------------------------------------------------");
		CryLogAlways ("$6          %6.3f %6.3f %6.3f (%.3f) Mbytes", double(stats.dwFixed)/0x100000, double(stats.dwFree)/0x100000, double(stats.dwMoveable)/0x100000, double(stats.dwUnknown)/0x100000);
		CryLogAlways ("-------------------------------------------------");
	}
	else
		CryLogAlways ("No heaps to dump");

	CloseHandle(hSnapshot);
#endif
}

// Make system error message string
//////////////////////////////////////////////////////////////////////////
//! \return pointer to the null terminated error string or 0
static const char* GetLastSystemErrorMessage()
{
#ifdef WIN32
  DWORD dwError = GetLastError();

	static char szBuffer[512]; // function will return pointer to this buffer

  if(dwError)
  {
//#ifdef _XBOX

    LPVOID lpMsgBuf=0;
    
		if(FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL))
		{
	    strncpy(szBuffer, (char*)lpMsgBuf, sizeof(szBuffer));
		  LocalFree(lpMsgBuf);
		}
		else return 0;

//#else

	  //sprintf(szBuffer, "Win32 ERROR: %i", dwError);
    //OutputDebugString(szBuffer);

//#endif

    return szBuffer;
  }
#else
	return 0;

#endif //WIN32

  return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CSystem::GetSSFileInfo( const char *inszFileName, char *outszInfo, const DWORD indwBufferSize )
{
	if(!(m_cvSSInfo && m_cvSSInfo->GetIVal()!=0))
	{		
		// SSInfo is deavtivated
		strcpy(outszInfo,"SourceSafe-Info is deactivated (sys_SSInfo=0)");
		assert(indwBufferSize>strlen(outszInfo)+1);
		return(true);
	}

	for(int iDatabase=0;iDatabase<=1;iDatabase++)		// search in all databases - if neccessry 
	{
		const char *szDatabase=0;
		const char *szRoot=0;
		
		// (hard coded) 
		switch(iDatabase)
		{
			case 0:																												// artist sourcesafe file (first try this)
				szDatabase="\\\\server2\\XIsle\\ArtworkVSS\\srcsafe.ini";
				szRoot="$/MasterCD";
				break;

			case 1:																												// programmer sourcesafe file
				szDatabase="\\\\server1\\vss\\srcsafe.ini";
				szRoot="$/MasterCD_Programmers";		
				break;
		}

		assert(szDatabase);
		assert(szRoot);

		const int iSize=256;

		char name[iSize];
		char comment[iSize];
		char date[iSize];

		// get mastercd directory (like in editor) - can be cleaned up
		char szMasterCD[_MAX_PATH];

		GetCurrentDirectory( _MAX_PATH,szMasterCD );

		if(::_GetSSFileInfo(	szDatabase,													// sourcesafe file
													szRoot,															// project sourcesafe path 
													szMasterCD,													// project folder path
													inszFileName,
													name,comment,date,iSize))
		{
			if(_snprintf(outszInfo,indwBufferSize,"SourceSafe-Info: name='%s' comment='%s' date='%s'",name,comment,date)<0)
			{
				*outszInfo=0;
				return false;		// buffer size exceeded
			}

			return true;
		}

	}	//  search in all databases


	return false;					// _GetSSFileInfo failed
}

//////////////////////////////////////////////////////////////////////////
void CSystem::Error( const char *format,... )
{
	// format message
	va_list	ArgList;
	char szBuffer[MAX_WARNING_LENGTH];
	const char *sPrefix = "\001CRITICAL ERROR: ";
	strcpy( szBuffer,sPrefix );
	va_start(ArgList, format);
	_vsnprintf(szBuffer+strlen(sPrefix), MAX_WARNING_LENGTH-strlen(sPrefix), format, ArgList);
	va_end(ArgList);

	// get system error message before any attempt to write into log
  const char * szSysErrorMessage = GetLastSystemErrorMessage();

	// write both messages into log
	if (m_pLog)
		m_pLog->Log( szBuffer );

	if (szSysErrorMessage && m_pLog)
		m_pLog->Log( "<CrySystem> Last System Error: %s",szSysErrorMessage );

	bool bHandled = false;
	if (GetUserCallback())
		bHandled = GetUserCallback()->OnError( szBuffer );

	// remove verbosity tag since it is not supported by ::MessageBox
	strcpy(szBuffer,szBuffer+1);

#ifdef WIN32
	if (!bHandled)
		::MessageBox( NULL,szBuffer,"CryEngine Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL );
	// Dump callstack.
	DebugCallStack::instance()->LogCallstack();
#endif
#ifndef PS2
  ::OutputDebugString(szBuffer);
#endif	//PS2

	// try to shutdown renderer (if we crash here - error message will already stay in the log)
	if(m_pRenderer)
		m_pRenderer->ShutDown();

	// app can not continue
#ifdef _DEBUG
#if defined(WIN32) && !defined(WIN64)
	DEBUG_BREAK;
#endif
#else
	exit(1);
#endif
}

// tries to log the call stack . for DEBUG purposes only
//////////////////////////////////////////////////////////////////////////
void CSystem::LogCallStack()
{
#if !defined(LINUX)
	DebugCallStack::instance()->LogCallstack();
#endif
}

