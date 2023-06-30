#include "stdafx.h"
#include "DedicatedServer.h"				// InitDedicatedServer

#if defined(LINUX)
	#include <unistd.h>
	#include "CryLibrary.h"
	#include <signal.h>
	#include <execinfo.h>
#endif

#if defined(LINUX)
	extern int MainCON( const char *szCmdLine, const bool cIsDaemon);
#else
	extern int MainCON( const char *szCmdLine );
#endif

#if defined(LINUX)

bool g_OnQuit;//do not perform a stack trace on quit

#if defined(LINUX32)
	const char *gpBinDir = "bin32linux";
#else
	const char *gpBinDir = "bin64linux";
#endif

volatile sig_atomic_t fatal_error_in_progress = 0;

void UnRegisterExceptionHandler(std::map<int,sighandler_t>& rSignalMap)
{
	for(std::map<int,sighandler_t>::const_iterator iter = rSignalMap.begin(); iter != rSignalMap.end(); ++iter)
		signal(iter->first, iter->second);
	rSignalMap.clear();
}

void PrintTrace (int signum)
{
	if (fatal_error_in_progress)
		return;
	fatal_error_in_progress = 1;
	if(g_OnQuit || IsOnQuit())
	{
		abort();//do not perform a stack trace on quit
		return;	
	}

	FILE *p = ::fopen("./log_LinuxSV.txt", "a+");
	if(p)fprintf(p, "\n");

	printf("\n");
	switch(signum)
	{
	case SIGINT:
		printf("Received signal SIGINT: 'program interrupt', received INTR character (normally C-c)\n");
		fprintf(p, "Received signal SIGINT: 'program interrupt', received INTR character (normally C-c)\n");
			break;
	case SIGHUP:
		printf("Received signal SIGHUP: user's terminal is disconnected\n");
		if(p)fprintf(p, "Received signal SIGHUP: user's terminal is disconnected\n");
		break;
	case SIGTERM:
		printf("Received signal SIGTERM: cause program termination\n");
		if(p)fprintf(p, "Received signal SIGTERM: cause program termination\n");
		break;
	case SIGQUIT:
		printf("Received signal SIGQUIT: received QUIT character (normally C-\) character\n");
		if(p)fprintf(p, "Received signal SIGQUIT: received QUIT character (normally C-\) character\n");
		break;
	case SIGKILL:
		printf("Received signal SIGKILL: cause immediate program termination\n");
		if(p)fprintf(p, "Received signal SIGKILL: cause immediate program termination\n");
		break;
	case SIGILL:
		printf("Received signal SIGILL: 'illegal instruction', program is trying to execute garbage or a privileged instruction\n");
		if(p)fprintf(p, "Received signal SIGILL: 'illegal instruction', program is trying to execute garbage or a privileged instruction\n");
		break;
	case SIGBUS:
		printf("Received signal SIGBUS: invalid pointer is dereferenced\n");
		if(p)fprintf(p, "Received signal SIGBUS: invalid pointer is dereferenced\n");
		break;
	case SIGSEGV:
		printf("Received signal SIGSEGV: program tries to read or write outside the memory that is allocated for it\n");
		if(p)fprintf(p, "Received signal SIGSEGV: program tries to read or write outside the memory that is allocated for it\n");
		break;
	case SIGABRT:
		printf("Received signal SIGABRT: error detected by the program itself and reported by calling 'abort'\n");
		if(p)fprintf(p, "Received signal SIGABRT: error detected by the program itself and reported by calling 'abort'\n");
		break;
	}

	void *array[10];
	size_t size			= backtrace (array, 10);
	char **strings	= backtrace_symbols (array, size);
	printf("\n");
	printf ("Stack-Trace: obtained %d stack frames.\n", size);
	if(p)fprintf(p, "Stack-Trace: obtained %d stack frames.\n", size);

	for (size_t i = 0; i < size; i++)
	{
		printf ("%s\n", strings[i]);
		//try to log as well
		if(p)fprintf(p, "%s\n", strings[i]);
	}
	if(p)fclose(p);p=NULL;
	free (strings);  
	printf("\n");

	abort(); 
}

void RegisterExceptionHandler(std::map<int,sighandler_t>& rSignalMap) 
{
	sighandler_t t;
	rSignalMap.clear();
	t = signal (SIGINT, PrintTrace);
	rSignalMap.insert(std::pair<int,sighandler_t>(SIGINT,t));
	t = signal (SIGHUP, PrintTrace);
	rSignalMap.insert(std::pair<int,sighandler_t>(SIGHUP,t));
	t = signal (SIGTERM, PrintTrace);
	rSignalMap.insert(std::pair<int,sighandler_t>(SIGTERM,t));
	t = signal (SIGQUIT, PrintTrace);
	rSignalMap.insert(std::pair<int,sighandler_t>(SIGQUIT,t));
	t = signal (SIGKILL, PrintTrace);
	rSignalMap.insert(std::pair<int,sighandler_t>(SIGKILL,t));
	t = signal (SIGILL, PrintTrace);
	rSignalMap.insert(std::pair<int,sighandler_t>(SIGILL,t));
	t = signal (SIGBUS, PrintTrace);
	rSignalMap.insert(std::pair<int,sighandler_t>(SIGBUS,t));
	t = signal (SIGSEGV, PrintTrace);
	rSignalMap.insert(std::pair<int,sighandler_t>(SIGSEGV,t));
	t = signal (SIGABRT, PrintTrace);
	rSignalMap.insert(std::pair<int,sighandler_t>(SIGABRT,t));
}

static void RetrievePaths(std::vector<string>& rDirectories, const char* cpPathContents)
{
	rDirectories.clear();
	//make sure it end with /
	string path(cpPathContents);
	size_t loc = 0;
	while(true)
	{
		loc = path.find(":",0);
		if(loc == string::npos)
			break;
		string tmp = path.substr(0, loc);
		if(loc != string::npos)
			path = path.substr(loc+1, (path.size()-1 - loc));
		if(tmp.c_str()[tmp.size()-1] != '/')
			tmp += "/";
		rDirectories.push_back(tmp);
	}
}

static void SetMasterCDFolder(const char* pExecutableName) 
{ 
	char szExeFileName[_MAX_PATH];
	// Get the path of the executable
	GetCurrentDirectory(sizeof(szExeFileName), szExeFileName);
	string currentDir(szExeFileName);		currentDir += '/';//temp storage	
	string DllPath = szExeFileName;
	//check whether this is a correct path
	DllPath += "/"; 
	SetModulePath(DllPath.c_str());
 
	string masterCDPath(GetModulePath());
	masterCDPath += "..";
	SetCurrentDirectory( masterCDPath.c_str() );
	//get name without ..
	GetCurrentDirectory(sizeof(szExeFileName), szExeFileName);
	SetFileAttributes(szExeFileName, 0x777);//set full access to current directory
	string t(szExeFileName);
	if(t.size() > 0 && (t.c_str()[t.size()-1] != '/'))
		t += "/";
	//test with directory name bin??linux
	string s("./"); 
	s += gpBinDir;
	DIR *pTest = opendir(s.c_str());
	if(pTest == NULL) 
	{
		//resolve symlink
		string execFilename(pExecutableName);
		string str(currentDir);
		str += execFilename;
		string realExecName;
		const char* pRealName = canonicalize_file_name(str.c_str());
		if(!pRealName)
		{
			//now try the executable name itself
			const char* pRealName2 = canonicalize_file_name(pExecutableName);
			if(!pRealName2)
			{		
				//okay last attempt, check all $PATH to find the executable in
				const char *cpPATH = getenv("PATH");
				if(cpPATH)
				{
					//parse it
					std::vector<string> directories;
					RetrievePaths(directories, cpPATH);
					//iterate all entries
					bool found = false;
					for(std::vector<string>::iterator iter = directories.begin(); iter != directories.end(); ++iter)
					{
						string tmp(*iter);
						tmp += pExecutableName;
						const char* pRealNameTest = canonicalize_file_name(tmp.c_str());
						if(pRealNameTest)	
						{
							realExecName = pRealNameTest;
							found = true;
							break;
						}
					}
					if(!found)
					{
						printf("could not find valid MasterCD folder, could not resolve directory where it is located\n");
						exit(1);
					}
				}
				else
				{
					printf("could not find valid MasterCD folder, could not resolve directory where it is located\n");
					exit(1);
				}
			}
			else
			{
				realExecName = pRealName2;
			}
		}
		else
		{
			realExecName = pRealName;
		}
		//OK, link has been resolved, now get directory
		string::size_type loc = realExecName.find(gpBinDir);
		if(loc == string::npos)
		{
			printf("could not find valid binary(binxxlinux) folder in the executable path, could not resolve directory where it is located\n");
			exit(1);
		}
		//ok now compose mastercd folder 
		char pNewMasterCDPath[256];
		memset(pNewMasterCDPath, '\0',256);
		memcpy(pNewMasterCDPath, realExecName.c_str(), loc);
		pNewMasterCDPath[loc + 1] = '\0';//terminate string
		SetCurrentDirectory(pNewMasterCDPath);
		DIR *pNewTest = opendir(s.c_str());
		if(pNewTest)
		{
			//adapt module path
			string s(pNewMasterCDPath);
			if(s.c_str()[s.size()-1] != '/')
				s += "/";
			s += gpBinDir;
			if(s.c_str()[s.size()-1] != '/')
				s += "/";
			SetModulePath(s.c_str());
			printf("setting MasterCD folder to:  %s\n",pNewMasterCDPath);
			SetFileAttributes(pNewMasterCDPath, 0x777);//set full access to current directory
			closedir(pNewTest);
		}
		else
		{
			printf("could not find valid MasterCD folder\n");
			exit(1);
		}
	}
	else  
	{
		printf("setting MasterCD folder to:  %s\n",t.c_str());
		closedir(pTest);
	}
	SetFileAttributes(".", 0x777);//set full access to current directory
}

const int CopyFile(const string& crSource, const string& crDest)
{
	FILE *fs = ::fopen(crSource.c_str(), "rb");
	int success = 0 ;
	if (fs != NULL) 
	{
		FILE *ft = ::fopen(crDest.c_str(), "wb");
		if ( ft != NULL ) 
		{
			fseek(fs , 0, SEEK_END);
			const int siz = ftell(fs);
			if ( siz > 0 ) 
			{
				char *buf = new char[siz];
				if(buf != NULL)
				{
					fseek(fs, 0, SEEK_SET);
					int rb = fread(buf, 1, siz, fs);
					int wb = fwrite(buf, 1, rb, ft);
					delete buf;	buf = NULL;
					if(wb == siz) 
						success = 1; 
				}
			}
			fclose ( ft ) ;
		}
		fclose ( fs ) ;
	}
	return success;
}

void CopyPunkBusterFiles()
{
	//copies punkbuster libraries if needed
	const string sourceDir(GetModulePath());
	char szExeFileName[_MAX_PATH];
	GetCurrentDirectory(sizeof(szExeFileName), szExeFileName);
	string destDir(szExeFileName);	if(destDir.c_str()[destDir.size()-1] != '/') destDir += "/"; 
	string linuxDestDir(destDir);	linuxDestDir += "pb";
	DIR *dirp = ::opendir(linuxDestDir.c_str());
	//create symlink ./pb/ to ./PB/
	if(dirp == NULL)  
	{
		string dirTest(destDir);
		dirTest += "PB";
		DIR *d = ::opendir(dirTest.c_str());
		if(d)
		{
			destDir += "PB";
			closedir(d);
			SetFileAttributes("./PB/", 0x777);//set full access to current directory
		}
		else
		{
			string dirTest(destDir);
			dirTest += "Pb";
			DIR *d1 = ::opendir(dirTest.c_str());
			if(d1)
			{
				destDir += "Pb";
				closedir(d1);
				SetFileAttributes("./Pb/", 0x777);//set full access to current directory
			}
			else
			{
				string dirTest(destDir);
				dirTest += "Pb";
				DIR *d2 = ::opendir(dirTest.c_str());
				if(d2)
				{
					destDir += "pB";
					closedir(d2);
					SetFileAttributes("./pB/", 0x777);//set full access to current directory
				}
			}
		}

		//create symlink
		if(symlink(destDir.c_str(), linuxDestDir.c_str()) == -1)
		{
			printf("Cannot create symlink to Punkbuster directory %s: ",linuxDestDir.c_str());
			int error = errno;
			switch(error)
			{
			case EACCES:
				printf("The process does not have search permission for a directory component of the file name\n");
				break;
			case ENAMETOOLONG :
				printf("Either the total length of a file name is greater than PATH_MAX or an individual file name component has a length greater than NAME_MAX\n");
				break;
			case ENOENT:
				printf("File referenced as a directory component in the file name doesn't exist\n");
				break;
			case ENOTDIR:
				printf("A file that is referenced as a directory component in the file name exists, but it isn't a directory\n");
				break;
			case EEXIST:
				printf("There is already an existing file named %s\n",linuxDestDir.c_str()); 
				break;
			case EROFS: 
				printf("The file would exist on a read-only file system\n");
				break;
			case ENOSPC:
					printf("The directory or file system cannot be extended to make the new link\n");
				break;
			case EIO:
					printf("A hardware error occurred while reading or writing data on the disk\n");
				break;
			}
		}
		dirp = ::opendir(linuxDestDir.c_str());
		if(dirp == NULL)
		{
			printf("Cannot read symlink to Punkbuster directory: %s , Punkbuster won't work\n. Please create a symlink 'pb' pointing to 'PB' in the mastercd folder manually to make it work. Check access rights too.\n",linuxDestDir.c_str());
			return;
		}
	}
	else
	{
		destDir = linuxDestDir;
		destDir += '/';
	}
	if(dirp)
	{
		SetFileAttributes(linuxDestDir.c_str(), 0x777);//set full access to current directory
		closedir(dirp);
	}
	destDir += "/";
	string s(sourceDir); 
	s += "pbag.so";
	string d(destDir);
	d += "pbag.so";
	FILE *p = ::fopen(d.c_str(), "r");
	if(p == NULL)
	{
		 if(!CopyFile(s.c_str(), d.c_str()))
			 printf("Failed to copy Punkbuster files to working folder: from %s to %s\n", s.c_str(), d.c_str());
	}
	else
		fclose(p);
	s = sourceDir; 
	s += "pbcl.so";
	d = destDir;
	d += "pbcl.so";
	p = ::fopen(d.c_str(), "r");
	if(p == NULL)
	{
		if(!CopyFile(s.c_str(), d.c_str()))
			printf("Failed to copy Punkbuster files to working folder: from %s to %s\n", s.c_str(), d.c_str());
	}
	else
		fclose(p);
	s = sourceDir; 
	s += "pbsv.so";
	d = destDir;
	d += "pbsv.so";
	p = ::fopen(d.c_str(), "r");
	if(p == NULL)
	{
		if(!CopyFile(s.c_str(), d.c_str()))
			printf("Failed to copy Punkbuster files to working folder: from %s to %s\n", s.c_str(), d.c_str());
	}
	else
		fclose(p);
}
 
int main(int argc, char **argv) 
{
	//change the file mode mask
	umask(0);
	std::map<int,sighandler_t> signalMap;
	string cmdLine; 
	g_OnQuit = false;
	bool bIsDaemon = false;
	for(int i=1; i<argc; ++i)
	{
		const string argument(argv[i]);
		if(argument.find("-daemon") != string::npos)
		{
			bIsDaemon = true;
			continue;
		}
		cmdLine += string("\"")+argument+string("\"");
		cmdLine += " "; 
	}
	if(bIsDaemon)
	{
		//our process ID and Session ID 
		pid_t pid, sid;
		//fork off the parent process 
		pid = fork();
		if (pid < 0) 
			exit(EXIT_FAILURE);
		//if we got a good PID, then we can exit the parent process
		if (pid > 0) 
			exit(EXIT_SUCCESS);
		//open any logs here
		//create a new SID for the child process
		sid = setsid();
		if (sid < 0) 
			//log the failure
			exit(EXIT_FAILURE);
		//close out the standard file descriptors
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}
	RegisterExceptionHandler(signalMap);

	assert(argc > 0);
	SetMasterCDFolder(argv[0]);

	//copy punkbuster libraries if needed
#ifndef NOT_USE_PUNKBUSTER_SDK
	CopyPunkBusterFiles();
#endif

	const int ret = MainCON(cmdLine.c_str(), bIsDaemon);
	UnRegisterExceptionHandler(signalMap);

	return ret;
}




  
#else //LINUX
//#ifndef NONWIN32TESTCOMPILE

#ifdef WIN32
#include <windows.h>
#include <ShellApi.h>
#include <process.h>
#endif

#include <vector>
#include <queue>
#include <list>
#include <map>
#include <set>
#include <string>
#include <algorithm>


#include <platform.h>


#include "Cry_Math.h"
#include <Cry_Camera.h>

#include <IRenderer.h>
#include <ILog.h>
#include <ISystem.h>
#include <ITimer.h>
#include <IGame.h>
#include <IConsole.h>								// IOutputPrintSink
#include <IInput.h>

#include <crtdbg.h>
#include "resource.h"								// IDI_ICON


int MainWin32( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow );


//! Sink for ParseArguments()
struct ICmdlineArgumentSink
{
	//! used for early command e.g. "-DEVMODE", "-IP:123.22.23.1", "-MOD:CS"
	//! or for console command e.g. "map mp_airstrip", "name test"
	//! \param inszArgument must not be 0
	virtual void ReturnArgument( const char *inszArgument )=0;
};

void ParseArguments( const char *inszCommandLine, ICmdlineArgumentSink *pEarlyCommands, ICmdlineArgumentSink *pConsoleCommands )
{
	assert(inszCommandLine);

//	int iArgNo=0;
	char *src=(char *)inszCommandLine;
	char Arg[1024];

	while(*src)
	{
		char *dst=Arg;

		while(*src<=' ' && *src!=0)
			src++;		// jump over whitespace

		if(*src=='\"')
		{
			src++;

			while(*src!='\"' && *src!=0)
				*dst++=*src++;

			if(*src=='\"')
				src++;
		}
		else
		{
			while(*src!=' ' && *src!=0)
				*dst++=*src++;
		}

		*dst=0;

		if(*Arg!=0)
		{
//			if(iArgNo!=0)		// ignore .exe name
			{
				if(Arg[0]=='-')
				{
					if(pEarlyCommands)
						pEarlyCommands->ReturnArgument(&Arg[1]);
				}
				else
				{
					if(pConsoleCommands)
						pConsoleCommands->ReturnArgument(Arg);
				}
			}

//			iArgNo++;
		}
	}
}

class CDetectEarlyCommand :public ICmdlineArgumentSink
{
public:
	//! constructor
	CDetectEarlyCommand()
	{
		m_bActivateCON=false;
	}

	virtual void ReturnArgument( const char *inszArgument )
	{
		if(strcmp("CON",inszArgument)==0)
			m_bActivateCON=true;
	}

	bool					m_bActivateCON;			//!< true=simple stdio application, false=Win32 application
};


int APIENTRY _tWinMain( HINSTANCE hInstance,
												HINSTANCE hPrevInstance,
												LPTSTR    lpCmdLine,
												int       nCmdShow )
{
	CDetectEarlyCommand EarlyCommands;

	ParseArguments(lpCmdLine,&EarlyCommands,0);

	if(EarlyCommands.m_bActivateCON)
		return MainCON(lpCmdLine);
	else
	{
		FreeConsole();
		return MainWin32(hInstance,hPrevInstance,lpCmdLine,nCmdShow);
	}
}



//////////////////////////////////////////////////////////////////////////
// Call to set correct root folder when running from Bin32
//////////////////////////////////////////////////////////////////////////
static void SetMasterCDFolder()
{
	char szExeFileName[_MAX_PATH];
	// Get the path of the executable
	GetModuleFileName( GetModuleHandle(NULL), szExeFileName, sizeof(szExeFileName));

	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath( szExeFileName, drive, dir, fname, ext );
	_makepath( path_buffer, drive,dir,NULL,NULL );
	strcat( path_buffer,".." );
	SetCurrentDirectory( path_buffer );
}

//-------------------------------------------------------------------------------------------------
// tray icon stuff

#define							WM_TRAYICON		(WM_USER+5)
#define							MENUID_SHOW		(0x1001)
#define							MENUID_QUIT		(0x1002)

NOTIFYICONDATA			g_NotifyIconData = {0};
HMENU								g_hTrayMenu = 0;

void AddTrayIcon();
void RemoveTrayIcon();
void ShowTrayMenu(int x, int y);
void HideFromTaskBar();
void ShowOnTaskBar();
//-------------------------------------------------------------------------------------------------

TCHAR								g_szWindowClass[]=_T("FarCry_WinSV");		//!< the main window class name
HWND								g_WndMain=NULL;													//!<
HWND								g_WndIn=NULL;														//!<
HWND								g_WndOut=NULL;													//!<

WNDPROC								g_pfOldWindowProcOfWndIn( 0 );

const unsigned long					g_dwInnerWidth=800;
const unsigned long					g_dwInnerHeight=600;
const unsigned long					g_dwEditLineHeight=16;
const unsigned long					g_dwLineHeight=16;



class COutputPrintSink :public IOutputPrintSink
{
public:

	//! constructor
	COutputPrintSink(): m_dwFirstLine(0) {};
	//! destructor
	virtual ~COutputPrintSink() {};

	unsigned int m_dwFirstLine;						//!< used to be able to scroll the window with the Page up/down keys 

	// interface IOutputPrintSink ------------------------

	virtual void Print( const char *inszText )
	{
		Redraw();
	}

	void Redraw()
	{
		InvalidateRect(g_WndOut,0,false);
		UpdateWindow(g_WndOut);
	}

	// ---------------------------------------------------

	void DrawHDC( HDC inHdc )
	{
		RECT rect;

		GetClientRect(g_WndOut,&rect);
		FillRect(inHdc, &rect,(HBRUSH)GetStockObject(BLACK_BRUSH));

		HFONT hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);

		SelectObject(inHdc, hFont);
		SetBkColor(inHdc, 0);								// black
		SetTextColor(inHdc, 0xaaaaaa);			// BGR

		TEXTMETRIC metric;
		GetTextMetrics(inHdc, &metric);

		if(!GetISystem())
		{
			const char *szInitString = "Initializing...";
			const int szInitStringLen = (const int)strlen(szInitString);

			unsigned long x = ((rect.right - rect.left) - szInitStringLen * metric.tmMaxCharWidth) >> 1;
			unsigned long y = ((rect.bottom - rect.top) - metric.tmHeight) >> 1;

			TextOut(inHdc, x, y, szInitString, szInitStringLen);

			m_dwFirstLine = 0;

			return;
		}

		IConsole *pConsole = GetISystem()->GetIConsole();
		assert(pConsole);

		unsigned long dwLineNo=m_dwFirstLine;

		char szBuffer[256];
		unsigned long y = g_dwInnerHeight - g_dwEditLineHeight-2;

		while(pConsole->GetLineNo(dwLineNo++, szBuffer, 256))
		{
			y -= metric.tmHeight;

			const char *szStripped = Strip(szBuffer);

			TextOut(inHdc, 4, y, szStripped, (int)strlen(szStripped));

			if(y < 0)
				break;
		}
	}
};



COutputPrintSink g_Output;



bool OnWinInKeyDown( unsigned int uiChar, unsigned int uiRepCnt, unsigned int uiFlags )
{
	IConsole* pConsole( ( 0 != GetISystem() ) ? GetISystem()->GetIConsole() : 0 );

	assert( GetISystem() );
	assert( pConsole );

	if( 0 != pConsole )
	{
		if( VK_TAB != uiChar )
		{
			pConsole->ResetAutoCompletion();
		}

		switch( uiChar )
		{
		case VK_RETURN:
			{
				char szInput[ 512] ;
				GetWindowText( g_WndIn, szInput, 511 );
				SetWindowText( g_WndIn, "" );
				UpdateWindow( g_WndIn );

				pConsole->ExecuteString(szInput);
				pConsole->AddCommandToHistory(szInput);
				return true;
			}
		case VK_TAB:
			{
				char szInput[ 512 ];
				GetWindowText( g_WndIn, szInput, 511 );
				// don't use SendMessage( g_WndIn, WM_KEYDOWN, VK_END, 0 ) here as it will reset auto completion!
				SendMessage( g_WndIn, EM_SETSEL, 0, -1 );
				SendMessage( g_WndIn, EM_REPLACESEL, (WPARAM) TRUE, (LPARAM) pConsole->ProcessCompletion( szInput ) + 1 );
				SendMessage( g_WndIn, EM_SETSEL, -1, -1 );
				return( true );
			}
		case VK_PRIOR:
			{
				if( g_Output.m_dwFirstLine + 1 < (unsigned long) pConsole->GetLineCount() )
				{
					++g_Output.m_dwFirstLine;
					g_Output.Redraw();
				}
				return( true );
			}
		case VK_NEXT:
			{
				if( 0 < g_Output.m_dwFirstLine )
				{
					--g_Output.m_dwFirstLine;
					g_Output.Redraw();
				}
				return( true );
			}
		case VK_UP:
			{
				const char *szHistoryLine=pConsole->GetHistoryElement(true);		// true=UP

				if(szHistoryLine)
				{
					SetWindowText(g_WndIn,szHistoryLine);
					SendMessage( g_WndIn, WM_KEYDOWN, VK_END, 0 ); // set the cursor to the end
				}

				return true;
			}

		case VK_DOWN:
			{
				const char *szHistoryLine=pConsole->GetHistoryElement(false);		// false=DOWN

				if(szHistoryLine)
				{
					SetWindowText(g_WndIn,szHistoryLine);
					SendMessage( g_WndIn, WM_KEYDOWN, VK_END, 0 ); // set the cursor to the end
				}

				return true;
			}

		default:
			{
				return( false );
			}
		}
	}
	return( false );
}

bool
OnWinInChar( unsigned int uiChar, unsigned int uiRepCnt, unsigned int uiFlags )
{
	switch( uiChar )
	{
	case VK_TAB:
	case VK_RETURN:
		{
			return( true );
		}
	default:
		{
			break;
		}
	}
	return( false );
}

LRESULT CALLBACK
WndInWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
	case WM_KEYDOWN:
		{
			if( false != OnWinInKeyDown( (unsigned int) wParam, (unsigned int) LOWORD( lParam ), (unsigned int) HIWORD( lParam ) ) )
			{
				return( 0 );
			}
		}
	case WM_CHAR:
		{
			if( false != OnWinInChar( (unsigned int) wParam, (unsigned int) LOWORD( lParam ), (unsigned int) HIWORD( lParam ) ) )
			{
				return( 0 );
			}
		}
	}
	return( CallWindowProc( g_pfOldWindowProcOfWndIn, hWnd, message, wParam, lParam ) );
}

LRESULT CALLBACK
WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case MENUID_SHOW:
				ShowWindow(g_WndMain, SW_RESTORE);
				SetForegroundWindow(g_WndMain);
				SetActiveWindow(g_WndMain);
				break;
			case MENUID_QUIT:
				PostQuitMessage(0);
				break;
			}
		}
		return 0;

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED)
			{
				AddTrayIcon();
				HideFromTaskBar();
			}
			else if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
			{
				RemoveTrayIcon();
				ShowOnTaskBar();
			}
		}
		break;
	case WM_TRAYICON:
		{
			if (lParam == WM_RBUTTONUP)
			{
				POINT pt;

				GetCursorPos(&pt);
				ShowTrayMenu(pt.x, pt.y);
			}
			else if (lParam == WM_LBUTTONDBLCLK)
			{
				ShowWindow(g_WndMain, SW_RESTORE);
				SetForegroundWindow(g_WndMain);
				SetActiveWindow(g_WndMain);
			}
		}
		return 0;

	case WM_SETFOCUS:
		{
			SetFocus(g_WndIn); // only out input window can have focus
		}
		return 0;

	case WM_PAINT:
		{
			if(hWnd == g_WndOut)
			{
				hdc = BeginPaint(hWnd, &ps);

				g_Output.DrawHDC(hdc);

				EndPaint(hWnd, &ps);

				return 0;
			}
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style					= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc		= (WNDPROC)WndProc;
	wcex.cbClsExtra			= 0;
	wcex.cbWndExtra			= 0;
	wcex.hInstance			= hInstance;
	wcex.hIcon					= LoadIcon(hInstance, (LPCTSTR)IDI_ICON);
	wcex.hCursor				= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= CreateSolidBrush(0);	// black (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName		= 0;
	wcex.lpszClassName	= g_szWindowClass;
	wcex.hIconSm				= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_ICON);

	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
#ifdef WIN32
	g_WndMain = CreateWindow(g_szWindowClass, "Far Cry dedicated server", WS_OVERLAPPED|WS_CAPTION|WS_MINIMIZEBOX|WS_SYSMENU,
		CW_USEDEFAULT, 0,
		g_dwInnerWidth+2*GetSystemMetrics(SM_CXFIXEDFRAME),
		g_dwInnerHeight+2*GetSystemMetrics(SM_CYFIXEDFRAME)+GetSystemMetrics(SM_CYCAPTION),
		NULL, NULL, hInstance, NULL);

	if (!g_WndMain)
		return FALSE;

//	g_WndOut = CreateWindow("EDIT", "",WS_CHILD|WS_VISIBLE|WS_DISABLED,
//		0,0, g_dwInnerWidth, g_dwInnerHeight-g_dwEditLineHeight, g_WndMain, NULL, hInstance, NULL);
	g_WndOut = CreateWindow(g_szWindowClass, "",WS_CHILD|WS_VISIBLE|WS_DISABLED,
		0,0, g_dwInnerWidth, g_dwInnerHeight-g_dwEditLineHeight, g_WndMain, HMENU(), hInstance, NULL);

	g_WndIn = CreateWindow("EDIT", "",WS_CHILD|WS_VISIBLE,
		0,g_dwInnerHeight-g_dwEditLineHeight, g_dwInnerWidth, g_dwEditLineHeight, g_WndMain, HMENU(), hInstance, NULL);

	// subclass input window of console
	g_pfOldWindowProcOfWndIn = reinterpret_cast<WNDPROC>( SetWindowLongPtr( g_WndIn, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>( WndInWindowProc ) ) );

	SendMessage( g_WndIn, WM_SETFONT, (WPARAM) GetStockObject( ANSI_FIXED_FONT ), 1 );
	SetFocus( g_WndIn );

	ShowWindow(g_WndMain, nCmdShow);
	UpdateWindow( g_WndMain );
#endif
	return TRUE;
}







int MainWin32( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
	SetMasterCDFolder();
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

	// initialize the system
	bool bRelaunch=false;
	do
	{
		//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);

		DeInitDedicatedServer();

		Shell_NotifyIcon(NIM_DELETE, &g_NotifyIconData);

		// parse and execute the early CmdLine arguments
		InitDedicatedServer_System(lpCmdLine);

		ISystem *pSysten = GetISystem();

		if(!pSysten)
			return FALSE;					// if init failed

		IConsole *pConsole = pSysten->GetIConsole();
		assert(pConsole);

		pConsole->AddOutputPrintSink(&g_Output);

		PrintDedicatedServerStatus();

		SetForegroundWindow( g_WndMain );

		InitDedicatedServer_Game(lpCmdLine);

		IGame *pGame = GetISystem()->GetIGame();


		PrintWelcomeMessage();

		while( pGame->Run( bRelaunch ) )
		{
#ifdef WIN32
			// Main message loop:
			MSG msg;
			while( 0 != PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
#endif

			SleepIfNeeded();

#ifdef WIN32
			if( WM_QUIT == msg.message )
			{
				break;
			}
#endif // #ifdef WIN32
		}

//#ifdef WIN32
//		::FreeLibrary(g_hSystemHandle);
//#endif
		DeInitDedicatedServer();

	} while(bRelaunch);

	PrintGoodbyeMessage();

#ifdef WIN32
	RemoveTrayIcon();
#endif

	return true;
}

#ifdef WIN32

void AddTrayIcon()
{
	RemoveTrayIcon(); // make sure we have only one icon at a time

	// add a tray icon
	memset(&g_NotifyIconData, 0, sizeof(NOTIFYICONDATA));

	g_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	g_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	g_NotifyIconData.hWnd = g_WndMain;
	g_NotifyIconData.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON));
	g_NotifyIconData.uCallbackMessage = WM_TRAYICON;
	strncpy(g_NotifyIconData.szTip, "Far Cry Dedicated Server", 64);

	Shell_NotifyIcon(NIM_ADD, &g_NotifyIconData);

	g_hTrayMenu = CreatePopupMenu();

	AppendMenu(g_hTrayMenu, MF_STRING, MENUID_SHOW, "Show Console");
	AppendMenu(g_hTrayMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(g_hTrayMenu, MF_STRING, MENUID_QUIT, "Quit");
}

void RemoveTrayIcon()
{
	Shell_NotifyIcon(NIM_DELETE, &g_NotifyIconData);

	DestroyMenu(g_hTrayMenu);
}

void HideFromTaskBar()
{
	ShowWindow(g_WndMain, SW_HIDE);
	SetWindowLong(g_WndMain, GWL_EXSTYLE, GetWindowLong(g_WndMain, GWL_EXSTYLE) & ~WS_EX_APPWINDOW);
}

void ShowOnTaskBar()
{
	SetWindowLong(g_WndMain, GWL_EXSTYLE, GetWindowLong(g_WndMain, GWL_EXSTYLE) | WS_EX_APPWINDOW);
	ShowWindow(g_WndMain, SW_SHOW);
}

void ShowTrayMenu(int x, int y)
{
	TrackPopupMenu(g_hTrayMenu, TPM_LEFTALIGN, x, y, 0, g_WndMain, 0);
}
#endif // WIN32
#endif //LINUX
//#endif // NONWIN32TESTCOMPILE
