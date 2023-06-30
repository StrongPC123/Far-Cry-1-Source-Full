//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
// 
//	File: System.h	
// 
//	History:
//	-Jan 31,2001:Originally Created by Marco Corbetta
//	-: modified by all
//
//////////////////////////////////////////////////////////////////////

#ifndef SYSTEM_H
#define SYSTEM_H

#if _MSC_VER > 1000
# pragma once
#endif

#include <ISystem.h>
#include <ISound.h>
#include <IRenderer.h>
#include <IPhysics.h>
#include "Timer.h"
#include <CryVersion.h>

#include "FrameProfileSystem.h"
#include "StreamEngine.h"
#include "MTSafeAllocator.h"
#include "CPUDetect.h"
#include "PakVars.h"

#include "DownloadManager.h"

#if defined(LINUX)
	#include "CryLibrary.h"
#endif

#ifdef WIN32
#include <Tlhelp32.h>
#endif	

#ifdef WIN32
typedef HMODULE WIN_HMODULE;
#else
typedef void* WIN_HMODULE;
#endif

//forward declarations
class CScriptSink;
class CLUADbg;
struct IMusicSystem;
struct SDefaultValidator;
struct IDataProbe;

#define PHSYICS_OBJECT_ENTITY 0

typedef void (__cdecl *VTuneFunction)(void);
extern VTuneFunction VTResume;
extern VTuneFunction VTPause;

/*
===========================================
The System interface Class
===========================================
*/
class CXConsole;
//////////////////////////////////////////////////////////////////////
//!	ISystem implementation
class CSystem : 
	public ISystem
{
public:
	
	CSystem();
	~CSystem();
	bool IsDedicated(){return m_bDedicatedServer;}
	///////////////////////////////////////////////////////////////////////////
	//! @name ISystem implementation
	//@{ 
	virtual bool Init( const SSystemInitParams &params );
	virtual void Release();
	// Release all resources.
	void	ShutDown(bool bRelaunch);

	virtual bool CreateGame( const SGameInitParams &params );
	virtual bool Update( int updateFlags=0, int nPauseMode=0);
	virtual void UpdateScriptSink();

	//! Begin rendering frame.
	void	RenderBegin();
	//! Render subsystems.
	void	Render();
	//! End rendering frame and swap back buffer.
	void	RenderEnd();

	//! Update screen during loading.
	void UpdateLoadingScreen();

	//! Renders the statistics; this is called from RenderEnd, but if the 
	//! Host application (Editor) doesn't employ the Render cycle in ISystem,
	//! it may call this method to render the essencial statistics
	void RenderStatistics ();

	//! dumps the memory usage statistics to the log
	void DumpMemoryUsageStatistics();

	void Relaunch( bool bRelaunch );
	bool IsRelaunch() const { return m_bRelaunch; };
	void Quit();
	bool IsQuitting();
	void SetAffinity();
  const char *GetUserName();
	
	IGame						*GetIGame(){ return m_pGame; }
	INetwork				*GetINetwork(){ return m_pNetwork; }
	IRenderer				*GetIRenderer(){ return CSystem::m_pRenderer; }
	IInput					*GetIInput(){ return m_pIInput; }
	ITimer					*GetITimer(){ return &m_Time; }
	ICryPak					*GetIPak(){ return m_pIPak; }
	IConsole				*GetIConsole();
	IScriptSystem		*GetIScriptSystem(){ return m_pScriptSystem; }
	I3DEngine				*GetI3DEngine(){ return m_pI3DEngine; }
	ICryCharManager *GetIAnimationSystem() {return m_pICryCharManager;}
	ISoundSystem		*GetISoundSystem(){ return m_pISound; }
	IMusicSystem		*GetIMusicSystem(){ return m_pIMusic; }
  IPhysicalWorld	*GetIPhysicalWorld(){ return m_pIPhysicalWorld;}
	IMovieSystem		*GetIMovieSystem() { return m_pIMovieSystem; };
	IAISystem				*GetAISystem(){ return m_pAISystem;}
	IMemoryManager	*GetIMemoryManager(){ return m_pMemoryManager;}
	IEntitySystem		*GetIEntitySystem(){ return m_pEntitySystem;}
	ICryFont				*GetICryFont(){ return m_pICryFont; }
	ILog						*GetILog(){ return m_pLog; }
	IStreamEngine   *GetStreamEngine() {return m_pStreamEngine;}
	CLUADbg					*GetLuaDebugger() { return m_pLuaDebugger; }
	IValidator			*GetIValidator() { return m_pValidator; };
	IFrameProfileSystem* GetIProfileSystem() { return &m_FrameProfileSystem; }
	const char			*GetGameMOD() { if (m_szGameMOD[0]) return (m_szGameMOD);return (NULL); }

	XDOM::IXMLDOMDocument *CreateXMLDocument();

	//////////////////////////////////////////////////////////////////////////
	virtual XmlNodeRef CreateXmlNode( const char *sNodeName="" );
	virtual XmlNodeRef LoadXmlFile( const char *sFilename );
	virtual XmlNodeRef LoadXmlFromString( const char *sXmlString );
	//////////////////////////////////////////////////////////////////////////

	void SetViewCamera(class CCamera &Camera){ m_ViewCamera = Camera; }
	CCamera& GetViewCamera() { return m_ViewCamera; }

  virtual int GetCPUFlags()
  {
    int Flags = 0;
    if (!m_pCpu)
      return Flags;
    if (m_pCpu->hasMMX())
      Flags |= CPUF_MMX;
    if (m_pCpu->hasSSE())
      Flags |= CPUF_SSE;
    if (m_pCpu->hasSSE2())
      Flags |= CPUF_SSE;
    if (m_pCpu->has3DNow())
      Flags |= CPUF_3DNOW;

    return Flags;
  }
  virtual double GetSecondsPerCycle()
  {
    if (!m_pCpu)
      return 0;
    else
      return m_pCpu->m_Cpu[0].m_SecondsPerCycle;
  }

	void CreateEntityScriptBinding(IEntity *pEntity);

	void IgnoreUpdates( bool bIgnore ) { m_bIgnoreUpdates = bIgnore; };
	void SetGCFrequency( const float fRate );

	void SetIProcess(IProcess *process);
	IProcess* GetIProcess(){ return m_pProcess; }

	bool IsTestMode() const { return m_bTestMode; }
	//@}

	IRenderer	*CreateRenderer(bool fullscreen, void* hinst, void* hWndAttach = 0);

	virtual void Error( const char *format,... );
	// Validator Warning.
	void Warning( EValidatorModule module,EValidatorSeverity severity,int flags,const char *file,const char *format,... );
	bool CheckLogVerbosity( int verbosity );
		
	virtual void DebugStats(bool checkpoint, bool leaks);
	void DumpWinHeaps();
	// this enumeration describes the purpose for which the statistics is gathered.
	// if it's gathered to be dumped, then some different rules may be applied
	enum MemStatsPurposeEnum {nMSP_ForDisplay, nMSP_ForDump};
	void GetExeSizes (ICrySizer* pSizer, MemStatsPurposeEnum nPurpose = nMSP_ForDisplay);
	
	// tries to log the call stack . for DEBUG purposes only
	void LogCallStack();

	struct DumpHeap32Stats
	{
		DumpHeap32Stats():dwFree(0),dwMoveable(0),dwFixed(0),dwUnknown(0)
		{}
		void operator += (const DumpHeap32Stats& right)
		{
			dwFree += right.dwFree;
			dwMoveable += right.dwMoveable;
			dwFixed += right.dwFixed;
			dwUnknown += right.dwUnknown;
		}
		DWORD dwFree;
		DWORD dwMoveable;
		DWORD dwFixed;
		DWORD dwUnknown;
	};
#if defined(_XBOX) || defined(LINUX)
	//ASH: HEAPLIST32 doesn't exist on xbox.
	//void DumpHeap32 (const HEAPLIST32& hl, DumpHeap32Stats& stats);
#else // _XBOX
	void DumpHeap32 (const HEAPLIST32& hl, DumpHeap32Stats& stats);
#endif // _XBOX
	virtual int DumpMMStats(bool log);

	//! Return pointer to user defined callback.
	ISystemUserCallback* GetUserCallback() const { return m_pUserCallback; };

	//! refreshes the m_pMemStats if necessary; creates it if it's not created
	void TickMemStats(MemStatsPurposeEnum nPurpose = nMSP_ForDisplay);
	void SaveConfiguration();
	ESystemConfigSpec GetConfigSpec();

private:
	//! @name Initialization routines
	//@{ 
	bool InitNetwork();
	bool InitInput(WIN_HINSTANCE hinst,WIN_HWND hwnd);
	bool InitConsole();
	bool InitRenderer(WIN_HINSTANCE hinst,WIN_HWND hwnd,const char *szCmdLine);
	bool InitSound(WIN_HWND hwnd);
	bool InitPhysics();
	bool InitFont();
	bool InitFlash();
	bool InitAISystem();
	bool InitScriptSystem();
	bool InitFileSystem();
	bool InitStreamEngine();
	bool Init3DEngine();
	bool InitAnimationSystem();
	bool InitMovieSystem();
	bool InitEntitySystem(WIN_HINSTANCE hInstance, WIN_HWND hWnd);
	bool InitScriptBindings();
	bool OpenRenderLibrary(int type);
#if !defined(LINUX)
	int AutoDetectRenderer(char *Vendor, char *Device);
#endif
  bool OpenRenderLibrary(const char *t_rend);
  bool CloseRenderLibrary();
	bool ShutDownScriptBindings();
	//@}
	void Strange();
	bool ParseSystemConfig(string &sFileName);

	//////////////////////////////////////////////////////////////////////////
	// Helper functions.
	//////////////////////////////////////////////////////////////////////////
	void CreateRendererVars();
	void CreateSystemVars();
	void RenderStats();
	void RenderMemStats();
	// collects the whole memory statistics into the given sizer object
	void CollectMemStats (class CrySizerImpl* pSizer, MemStatsPurposeEnum nPurpose = nMSP_ForDisplay);
	WIN_HMODULE LoadDLL( const char *dllName, bool bQuitIfNotFound=true);
#if defined(LINUX)
	void FreeLib(HMODULE hLibModule);
#else
	void FreeLib(IN OUT HMODULE hLibModule);
#endif
	void QueryVersionInfo();
	void LogVersion();
	void SetDevMode( bool bEnable );
	void InitScriptDebugger();
	
public:

	// interface ISystem -------------------------------------------

	virtual void ShowDebugger(const char *pszSourceFile, int iLine, const char *pszReason);
	virtual bool GetSSFileInfo( const char *inszFileName, char *outszInfo, const DWORD indwBufferSize );
	virtual IDataProbe* GetIDataProbe() { return m_pDataProbe; };
	virtual void SetForceNonDevMode( const bool bValue );
	virtual bool GetForceNonDevMode() const;
	virtual bool WasInDevMode() const { return m_bWasInDevMode; };
	virtual bool IsDevMode() const { return m_bInDevMode && !GetForceNonDevMode(); }

	// -------------------------------------------------------------

	//! attaches the given variable to the given container;
	//! recreates the variable if necessary
	ICVar* attachVariable (const char* szVarName, int* pContainer, const char *szComment,int dwFlags=0 );

private: // ------------------------------------------------------

	CTimer								m_Time;								//!<
	CCamera								m_ViewCamera;					//!<
	CXConsole *						m_pConsole;						//!<
	bool									m_bQuit;							//!< if is true the system is quitting
	bool									m_bRelaunch;					//!< relaunching the app or not (true beforerelaunch)
	bool									m_bRelaunched;				//!< Application was started with the -RELAUNCH option (true after relaunch)
	bool									m_bTestMode;					//!< If running in testing mode.
	bool									m_bEditor;						//!< If running in Editor.
	bool									m_bDedicatedServer;		//!< If running as Dedicated server.
	bool									m_bIgnoreUpdates;			//!< When set to true will ignore Update and Render calls,
	IValidator *					m_pValidator;					//!< Pointer to validator interface.
	bool									m_bForceNonDevMode;		//!< true when running on a cheat protected server or a client that is connected to it (not used in singlplayer)
	bool									m_bWasInDevMode;			//!< Set to true if was in dev mode.
	bool									m_bInDevMode;					//!< Set to true if was in dev mode.
	SDefaultValidator *		m_pDefaultValidator;	//!<
	int										m_nStrangeRatio;			//!<

	//! Input system
	//! @see CRenderer
	IRenderer		*m_pRenderer;

  //! CPU features
  CCpuFeatures *m_pCpu;

	//! DLLs handles.
	struct SDllHandles
	{
		WIN_HMODULE hRenderer;
		WIN_HMODULE hInput;
    WIN_HMODULE hFlash;
		WIN_HMODULE hSound;
		WIN_HMODULE hEntitySystem;
		WIN_HMODULE hNetwork;
		WIN_HMODULE hAI;
		WIN_HMODULE	hMovie;
		WIN_HMODULE	hPhysics;
		WIN_HMODULE	hFont;
		WIN_HMODULE hScript;
		WIN_HMODULE h3DEngine;
		WIN_HMODULE hAnimation;
		WIN_HMODULE hIndoor;
		WIN_HMODULE hGame;
	};
	SDllHandles m_dll;

	//! Input system
	//! @see CInput
	IInput *m_pIInput;

	//! Log interface.
	ILog* m_pLog;

	//! THe streaming engine
	CStreamEngine* m_pStreamEngine;
	
	//! current active process
	IProcess *m_pProcess;

	IMemoryManager *m_pMemoryManager;

	//! Pack file system.
	CCryPak*	m_pIPak;

	//! Flash Module
	//IFlash *m_pFlashManager;

	//! Sound System
	//! @see CSoundSystem
	ISoundSystem*	m_pISound;

	//! Music System
	//! @see CMusicSystem
	IMusicSystem*	m_pIMusic;

	//! Entity System
	//! @see CEntitySystem
	IEntitySystem * m_pEntitySystem;

	//! Network Module
	//! @see CNetwork
	INetwork *m_pNetwork;

	//! AI System
	//! @see CAISystem
	IAISystem *m_pAISystem;

	//! Physics System
	IPhysicalWorld* m_pIPhysicalWorld;

	//! Movie System.
	IMovieSystem* m_pIMovieSystem;
	
	//! Font System
	//! @see CCryFont
	ICryFont*	m_pICryFont;
	
	//! Script System
	//! @see CScriptSystem
	IScriptSystem *m_pScriptSystem;
	//[Timur] CREATEDOMDOCUMENT_FNCPTR m_CreateDOMDocument;
	
	//! 3D Engine
	//! @see C3DEngine
	I3DEngine *m_pI3DEngine;
	ICryCharManager* m_pICryCharManager;

	//! The default font
	IFFont*	m_pIFont;

	//! game path folder
	char	m_szGameMOD[MAX_PATH];

	//! to hold the values stored in system.cfg
	//! because editor uses it's own values,
	//! and then saves them to file, overwriting the user's resolution.
	int m_iHeight;
	int m_iWidth;
	int m_iColorBits;
	
	// System console variables.
	//////////////////////////////////////////////////////////////////////////
	ICVar *m_cvAIUpdate;
	ICVar *m_rWidth;
	ICVar *m_rHeight;
	ICVar *m_rColorBits;
	ICVar *m_rDepthBits;
	ICVar *m_rStencilBits;
	ICVar *m_rFullscreen;
	ICVar *m_rDriver;
	ICVar *m_rDisplayInfo;
	ICVar *m_sysNoUpdate;
	ICVar *i_direct_input;
	ICVar *sys_script_debugger;
	ICVar *m_cvEntitySuppressionLevel;
	ICVar *m_pCVarQuit;
	ICVar *m_cvMemStats;
	ICVar *m_cvMemStatsThreshold;
	ICVar *m_cvMemStatsMaxDepth;
	ICVar *m_sysWarnings;										//!< might be 0, "sys_warnings" - Treat warning as errors.
	ICVar *m_cvSSInfo;											//!< might be 0, "sys_SSInfo" 0/1 - get file sourcesafe info
	ICVar *m_sys_profile;
	ICVar *m_sys_profile_graph;
	ICVar *m_sys_profile_graphScale;
	ICVar *m_sys_profile_pagefaultsgraph;
	ICVar *m_sys_profile_filter;
	ICVar *m_sys_profile_network;
	ICVar *m_sys_profile_peak;
	ICVar *m_sys_profile_memory;
	ICVar *m_sys_spec;
	ICVar *m_sys_skiponlowspec;
	ICVar *m_sys_firstlaunch;
	ICVar *m_sys_StreamCallbackTimeBudget;
	ICVar *m_sys_StreamCompressionMask;			//!< bitmask, lossy compression, useful for network comunication, should be 0 for load/save

	string	m_sSavedRDriver;								//!< to restore the driver when quitting the dedicated server

	ICVar* m_cvPakPriority;
	ICVar* m_cvPakReadSlice;
	ICVar* m_cvPakLogMissingFiles;
	// the contents of the pak priority file
	PakVars m_PakVar;

	//////////////////////////////////////////////////////////////////////////

	CScriptSink					*m_pScriptSink;

	//! User define callback for system events.
	ISystemUserCallback *m_pUserCallback;

	WIN_HWND		m_hWnd;
	WIN_HINSTANCE	m_hInst;

	// this is the memory statistics that is retained in memory between frames
	// in which it's not gathered
	class CrySizerStats* m_pMemStats;
	class CrySizerImpl* m_pSizer;

	struct CScriptBindings* m_pScriptBindings;
	
	CFrameProfileSystem m_FrameProfileSystem;
	int m_profile_old;

	//int m_nCurrentLogVerbosity;

	SFileVersion m_fileVersion;
	SFileVersion m_productVersion;
	IDataProbe *m_pDataProbe;
public:
	//! Pointer to Game Interface,
	IGame								*m_pGame;

	//! Pointer to the download manager
	CDownloadManager		*m_pDownloadManager;

	CLUADbg *m_pLuaDebugger;

#ifdef USE_FRAME_PROFILER
	void SetFrameProfiler(bool on, bool display, char *prefix) { m_FrameProfileSystem.SetProfiling(on, display, prefix, this); };
	void StartProfilerSection( CFrameProfilerSection *pProfileSection );
	void EndProfilerSection( CFrameProfilerSection *pProfileSection );
#else
	void SetFrameProfiler(bool on, bool display, char *prefix) {};
	void StartProfilerSection( CFrameProfilerSection *pProfileSection ) {};
	void EndProfilerSection( CFrameProfilerSection *pProfileSection ) {};
#endif

	//////////////////////////////////////////////////////////////////////////
	// VTune.
	virtual void VTuneResume();
	virtual void VTunePause();
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// File version.
	//////////////////////////////////////////////////////////////////////////
	virtual const SFileVersion& GetFileVersion();
	virtual const SFileVersion& GetProductVersion();

	bool WriteCompressedFile(char *filename, void *data, unsigned int bitlen);
	unsigned int ReadCompressedFile(char *filename, void *data, unsigned int maxbitlen);
	unsigned int GetCompressedFileSize(char *filename);
	void InitVTuneProfiler();

	void OpenBasicPaks();
	void OpenLanguagePak( const char *sLanguage );

	void	Deltree(const char *szFolder, bool bRecurse);
	void	LoadConfiguration(const string &sFilename);

protected: // -------------------------------------------------------------

	// this heap is used for small allocations - the queues
	CMTSafeHeap m_SmallHeap;
	// this heap is used for big allocations - the chunks to be loaded
	CMTSafeHeap m_BigHeap;

	friend struct SDefaultValidator;
};

#endif // SYSTEM_H