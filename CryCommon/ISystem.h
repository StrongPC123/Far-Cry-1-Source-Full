#ifndef _CRY_SYSTEM_H_
#define _CRY_SYSTEM_H_

#ifdef WIN32
	#ifdef CRYSYSTEM_EXPORTS
		#define CRYSYSTEM_API __declspec(dllexport)
	#else
		#define CRYSYSTEM_API __declspec(dllimport)
	#endif
#else
	#define CRYSYSTEM_API
#endif

#include "platform.h" // Needed for LARGE_INTEGER (for consoles).

////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////////////////////////
#include <IXMLDOM.h>
#include <IXml.h>
#include <IValidator.h>
#include <ILog.h>

struct ISystem;
struct ILog;
struct IEntitySystem;
struct IEntity;
struct ICryPak;
struct IKeyboard;
struct IMouse;
struct IConsole;
struct IInput;
struct IRenderer;
struct IConsole;
struct IProcess;
struct I3DEngine;
struct ITimer;
struct IGame;
struct IScriptSystem;
struct IAISystem;
struct IFlash;
struct INetwork;
struct ICryFont;
struct IMovieSystem;
class IPhysicalWorld;
struct IMemoryManager;
struct ISoundSystem;
struct IMusicSystem;
struct XDOM::IXMLDOMDocument;
struct IFrameProfileSystem;
struct FrameProfiler;
struct IStreamEngine;
struct ICryCharManager;
struct SFileVersion;
struct IDataProbe;

class CFrameProfilerSection;

#define DEFAULT_GAME_PATH	"FarCry"
#define DATA_FOLDER "FCData"

#define PROC_MENU		1
#define PROC_3DENGINE	2

//ID for script userdata typing (maybe they should be moved into the game.dll)
#define USER_DATA_SOUND			1
#define USER_DATA_TEXTURE		2
#define USER_DATA_OBJECT		3
#define USER_DATA_LIGHT			4
#define USER_DATA_BONEHANDLER	5
#define USER_DATA_POINTER		6

enum ESystemUpdateFlags
{
	ESYSUPDATE_IGNORE_AI			= 0x0001,
	ESYSUPDATE_IGNORE_PHYSICS = 0x0002,
	// Special update mode for editor.
	ESYSUPDATE_EDITOR					=	0x0004,
	ESYSUPDATE_MULTIPLAYER		= 0x0008
};

enum ESystemConfigSpec
{
	CONFIG_LOW_SPEC = 0,
	CONFIG_MEDIUM_SPEC = 1,
	CONFIG_HIGH_SPEC = 2,
	CONFIG_VERYHIGH_SPEC = 3,
};

// User defined callback, which can be passed to ISystem.
struct ISystemUserCallback
{
	/** Signals to User that engine error occured.
			@return true to Halt execution or false to ignore this error.
	*/
	virtual bool OnError( const char *szErrorString ) = 0;
	/** If working in Editor environment notify user that engine want to Save current document.
			This happens if critical error have occured and engine gives a user way to save data and not lose it
			due to crash.
	*/
	virtual void OnSaveDocument() = 0;
	
	/** Notify user that system wants to switch out of current process.
			(For ex. Called when pressing ESC in game mode to go to Menu).
	*/
	virtual void OnProcessSwitch() = 0;
};


// Structure passed to Init method of ISystem interface.
struct SSystemInitParams
{
	void *hInstance;											//
	void *hWnd;														//
	char szSystemCmdLine[512];						// command line, used to execute the early commands e.g. -DEVMODE "g_gametype ASSAULT"
	ISystemUserCallback *pUserCallback;		//
	ILog *pLog;														// You can specify your own ILog to be used by System.
	IValidator *pValidator;								// You can specify different validator object to use by System.
	const char* sLogFileName;							// File name to use for log.
	bool bEditor;													// When runing in Editor mode.
	bool bPreview;												// When runing in Preview mode (Minimal initialization).
	bool bTestMode;												// When runing in Automated testing mode.
	bool bDedicatedServer;								// When runing a dedicated server.
	ISystem *pSystem;											// Pointer to existing ISystem interface, it will be reused if not NULL.
//	char szLocalIP[256];									// local IP address (needed if we have several servers on one machine)
#if defined(LINUX)
	void (*pCheckFunc)(void*);							// authentication function (must be set).
#else
	void *pCheckFunc;											// authentication function (must be set).
#endif

	// Initialization defaults.
	SSystemInitParams()
	{
		hInstance = 0;
		hWnd = 0;
		memset(szSystemCmdLine,0,sizeof(szSystemCmdLine));
		pLog = 0;
		pValidator = 0;
		pUserCallback = 0;
		sLogFileName = 0;
		bEditor = false;
		bPreview = false;
		bTestMode = false;
		bDedicatedServer = false;
		pSystem = 0;
		pCheckFunc = 0;
//		memset(szLocalIP,0,256);
	}
};


// Structure passed to CreateGame method of ISystem interface.
struct SGameInitParams
{
	const char *	sGameDLL;							// Name of Game DLL. (Win32 Only)
	IGame *				pGame;								// Pointer to already created game interface.
	bool					bDedicatedServer;			// When runing a dedicated server.
	char					szGameCmdLine[256];		// command line, used to execute the console commands after game creation e.g. -DEVMODE "g_gametype ASSAULT"
	
	SGameInitParams()
	{
		sGameDLL = NULL;
		pGame = NULL;		
		bDedicatedServer = false;
		memset(szGameCmdLine,0,256);
	}
};


////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
// Main Engine Interface
// initialize and dispatch all engine's subsystems 
struct ISystem
{ 
	// Loads GameDLL and creates game instance.
	virtual bool CreateGame( const SGameInitParams &params ) = 0;
	
	// Release ISystem.
	virtual void Release() = 0;

	// Update all subsystems (including the ScriptSink() )
	// @param flags one or more flags from ESystemUpdateFlags sructure.
	// @param boolean to set when in pause or cutscene mode in order to avoid
	// certain subsystem updates 0=menu/pause, 1=cutscene mode
	virtual bool Update( int updateFlags=0,int nPauseMode=0) = 0;

	// update _time, _frametime (useful after loading level to apply the time value)
	virtual void UpdateScriptSink()=0;

	// Begin rendering frame.
	virtual void	RenderBegin() = 0;
	// Render subsystems.
	virtual void	Render() = 0;
	// End rendering frame and swap back buffer.
	virtual void	RenderEnd() = 0;

	// Renders the statistics; this is called from RenderEnd, but if the 
	// Host application (Editor) doesn't employ the Render cycle in ISystem,
	// it may call this method to render the essencial statistics
	virtual void RenderStatistics () = 0;

	// Retrieve the name of the user currently logged in to the computer
	virtual const char *GetUserName() = 0;

  // Gets current supported CPU features flags. (CPUF_SSE, CPUF_SSE2, CPUF_3DNOW, CPUF_MMX)
  virtual int GetCPUFlags() = 0;

  // Get seconds per processor tick
  virtual double GetSecondsPerCycle() = 0;

  // dumps the memory usage statistics to the log
	virtual void DumpMemoryUsageStatistics() = 0;

	// Quit the appliacation
	virtual void	Quit() = 0;
	// Tells the system if it is relaunching or not
	virtual void	Relaunch(bool bRelaunch) = 0;
	// return true if the application is in the shutdown phase
	virtual bool	IsQuitting() = 0;

	// Display error message.
	// Logs it to console and file and error message box.
	// Then terminates execution.
	virtual void Error( const char *sFormat,... ) = 0;
	
	//DOC-IGNORE-BEGIN
	//[Timur] DEPRECATED! Use Validator Warning instead.
	// Display warning message.
	// Logs it to console and file and display a warning message box.
	// Not terminates execution.
	//__declspec(deprecated) virtual void Warning( const char *sFormat,... ) = 0;
	//DOC-IGNORE-END

	// Report warning to current Validator object.
	// Not terminates execution.
	virtual void Warning( EValidatorModule module,EValidatorSeverity severity,int flags,const char *file,const char *format,... ) = 0;
	// Compare specified verbosity level to the one currently set.
	virtual bool CheckLogVerbosity( int verbosity ) = 0;

	// returns true if this is dedicated server application
	virtual bool IsDedicated() {return false;}

	// return the related subsystem interface
	virtual IGame						*GetIGame() = 0;
	virtual INetwork				*GetINetwork() = 0;
	virtual IRenderer				*GetIRenderer() = 0;
	virtual IInput					*GetIInput() = 0;
	virtual ITimer					*GetITimer() = 0;
	virtual IConsole				*GetIConsole() = 0;
	virtual IScriptSystem		*GetIScriptSystem() = 0;
	virtual I3DEngine				*GetI3DEngine() = 0;
	virtual ISoundSystem		*GetISoundSystem() = 0;
	virtual IMusicSystem		*GetIMusicSystem() = 0;
  virtual IPhysicalWorld	*GetIPhysicalWorld() = 0;
	virtual IMovieSystem		*GetIMovieSystem() = 0;
	virtual IAISystem				*GetAISystem() = 0;
	virtual IMemoryManager	*GetIMemoryManager() = 0;
	virtual IEntitySystem		*GetIEntitySystem() = 0;
	virtual ICryFont				*GetICryFont()	= 0;
	virtual ICryPak				  *GetIPak()	= 0;
	virtual ILog						*GetILog() = 0;
	virtual IStreamEngine   *GetStreamEngine() = 0;
	virtual ICryCharManager *GetIAnimationSystem() = 0;
	virtual IValidator			*GetIValidator() = 0;
	virtual IFrameProfileSystem* GetIProfileSystem() = 0;	

	//virtual	const char			*GetGamePath()=0;

	virtual void DebugStats(bool checkpoint, bool leaks) = 0;
	virtual void DumpWinHeaps() = 0;
	virtual int DumpMMStats(bool log) = 0;

	//////////////////////////////////////////////////////////////////////////
	// @param bValue set to true when running on a cheat protected server or a client that is connected to it (not used in singlplayer)
	virtual void SetForceNonDevMode( const bool bValue )=0;
	// @return is true when running on a cheat protected server or a client that is connected to it (not used in singlplayer)
	virtual bool GetForceNonDevMode() const=0;
	virtual bool WasInDevMode() const=0;
	virtual bool IsDevMode() const=0;
	//////////////////////////////////////////////////////////////////////////

	virtual XDOM::IXMLDOMDocument *CreateXMLDocument() = 0;

	//////////////////////////////////////////////////////////////////////////
	// IXmlNode interface.
	//////////////////////////////////////////////////////////////////////////
	
	// Creates new xml node.
	virtual XmlNodeRef CreateXmlNode( const char *sNodeName="" ) = 0;
	// Load xml file, return 0 if load failed.
	virtual XmlNodeRef LoadXmlFile( const char *sFilename ) = 0;
	// Load xml from string, return 0 if load failed.
	virtual XmlNodeRef LoadXmlFromString( const char *sXmlString ) = 0;

	virtual void SetViewCamera(class CCamera &Camera) = 0;
	virtual CCamera& GetViewCamera() = 0;

	virtual void CreateEntityScriptBinding(IEntity *pEntity)=0;
	// When ignore update sets to true, system will ignore and updates and render calls.
	virtual void IgnoreUpdates( bool bIgnore ) = 0;

	// Set rate of Garbage Collection for script system.
	// @param fRate in seconds
	virtual void SetGCFrequency( const float fRate ) = 0;

	/* Set the active process
		@param process a pointer to a class that implement the IProcess interface
	*/
	virtual void SetIProcess(IProcess *process) = 0;
	/* Get the active process
		@return a pointer to the current active process
	*/
	virtual IProcess* GetIProcess() = 0;

#if defined (WIN32) || defined (PS2)
	virtual IRenderer* CreateRenderer(bool fullscreen, void* hinst, void* hWndAttach = 0) = 0;
#endif	

	// Returns true if system running in Test mode.
	virtual bool IsTestMode() const = 0;
 
	virtual void ShowDebugger(const char *pszSourceFile, int iLine, const char *pszReason) = 0;
	
	//////////////////////////////////////////////////////////////////////////
	// Frame profiler functions
	virtual void SetFrameProfiler(bool on, bool display, char *prefix) = 0;

	// Starts section profiling.
	virtual void StartProfilerSection( CFrameProfilerSection *pProfileSection ) = 0;
	// Stops section profiling.
	virtual void EndProfilerSection( CFrameProfilerSection *pProfileSection ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// VTune Profiling interface.
	// Resume vtune data collection.
	virtual void VTuneResume() = 0;
	// Pauses vtune data collection.
	virtual void VTunePause() = 0;
	//////////////////////////////////////////////////////////////////////////

	virtual void Deltree(const char *szFolder, bool bRecurse) = 0;

	//////////////////////////////////////////////////////////////////////////
	// File version.
	//////////////////////////////////////////////////////////////////////////
	
	virtual const SFileVersion& GetFileVersion() = 0;
	virtual const SFileVersion& GetProductVersion() = 0;
	
	// Compressed file read & write
	virtual bool WriteCompressedFile(char *filename, void *data, unsigned int bitlen) = 0;
	virtual unsigned int ReadCompressedFile(char *filename, void *data, unsigned int maxbitlen) = 0;
	virtual unsigned int GetCompressedFileSize(char *filename)=0;

	// Sample:  char str[256];	bool bRet=GetSSFileInfo("C:\\mastercd\\materials\\compound_indoor.xml",str,256);
	// get info about the last SourceSafe action for a specifed file (Name,Comment,Date)
	// @param inszFileName inszFileName!=0, e.g. "c:\\mastercd\\AssMan\\AssManShellExt\\AssManMenu.cpp"
	// @param outszInfo outszInfo!=0, [0..indwBufferSize-1]
	// @param indwBufferSize >0
	// @return true=success, false otherwise (output parameter is set to empty strings)
	virtual bool GetSSFileInfo( const char *inszFileName, char *outszInfo, const DWORD indwBufferSize )=0;

	// Retrieve IDataProbe interface.
	virtual IDataProbe* GetIDataProbe() = 0;
	//////////////////////////////////////////////////////////////////////////
	// Configuration.
	//////////////////////////////////////////////////////////////////////////
	// Saves system configuration.
	virtual void SaveConfiguration() = 0;
	// Loads system configuration
	virtual void LoadConfiguration(const string &sFilename)=0;

	// Get current configuration specification.
	virtual ESystemConfigSpec GetConfigSpec() = 0;
};

//////////////////////////////////////////////////////////////////////////
// CrySystem DLL Exports.
//////////////////////////////////////////////////////////////////////////
typedef ISystem* (*PFNCREATESYSTEMINTERFACE)( SSystemInitParams &initParams );

// Get the system interface (must be defined locally in each module)
extern ISystem *GetISystem();

// interface of the DLL
extern "C" 
{
	CRYSYSTEM_API ISystem* CreateSystemInterface( SSystemInitParams &initParams );
}

//////////////////////////////////////////////////////////////////////////
// Display error message.
// Logs it to console and file and error message box.
// Then terminates execution.
inline void CryError( const char *format,... )
{ 
	if (!GetISystem())
		return;

	va_list	ArgList;
	char szBuffer[MAX_WARNING_LENGTH];
	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	GetISystem()->Error( "%s",szBuffer );
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Display warning message.
// Logs it to console and file and display a warning message box.
// Not terminates execution.
inline void CryWarning( EValidatorModule module,EValidatorSeverity severity,const char *format,... )
{
	if (!GetISystem() || !format)
		return;
	va_list	ArgList;
	char		szBuffer[MAX_WARNING_LENGTH];
	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);
	GetISystem()->Warning( module,severity,0,0,szBuffer );
}
//////////////////////////////////////////////////////////////////////////
// Simple log of data with low verbosity.
inline void CryLog( const char *format,... )
{
	if (GetISystem() && GetISystem()->CheckLogVerbosity(8))
	{
		va_list args;
		va_start(args,format);
		GetISystem()->GetILog()->LogV( ILog::eMessage,format,args );
		va_end(args);
	}
}

//////////////////////////////////////////////////////////////////////////
// Very rarely used log comment.
inline void CryLogComment( const char *format,... )
{
	if (GetISystem() && GetISystem()->CheckLogVerbosity(9))
	{
		va_list args;
		va_start(args,format);
		GetISystem()->GetILog()->LogV( ILog::eMessage,format,args );
		va_end(args);
	}
}

//////////////////////////////////////////////////////////////////////////
// Logs important data that must be printed regardless verbosity.
inline void CryLogAlways( const char *format,... )
{
	if (GetISystem())
	{
		va_list args;
		va_start(args,format);
		GetISystem()->GetILog()->LogV( ILog::eAlways,format,args );
		va_end(args);
	}
}

//////////////////////////////////////////////////////////////////////////
// Additional headers.
//////////////////////////////////////////////////////////////////////////
#include <FrameProfiler.h>

#endif //_CRY_SYSTEM_H_
