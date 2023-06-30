// LogFile.cpp: implementation of the CLogFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogFile.h"

#include "ProcessInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define EDITOR_LOG_FILE "Editor.log"

// Static member variables
HWND CLogFile::m_hWndListBox = 0;
HWND CLogFile::m_hWndEditBox = 0;
IConsole* CLogFile::m_console = 0;
bool CLogFile::m_bFileOpened = false;
bool CLogFile::m_bShowMemUsage = false;
ICVar	*CLogFile::m_pLogVerbosity = 0;
char CLogFile::m_szLogFilename[_MAX_PATH];

#define MAX_LOGBUFFER_SIZE 16384

//////////////////////////////////////////////////////////////////////////
void Error( const char *format,... )
{
	va_list	ArgList;
	char		szBuffer[MAX_LOGBUFFER_SIZE];

	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	CString str = "####-ERROR-####: ";
	str += szBuffer;

	CLogFile::WriteLine( str );
	AfxMessageBox( szBuffer,MB_OK|MB_ICONERROR|MB_APPLMODAL );
}

//////////////////////////////////////////////////////////////////////////
void Warning( const char *format,... )
{
	va_list	ArgList;
	char		szBuffer[MAX_LOGBUFFER_SIZE];

	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	CLogFile::WriteLine( szBuffer );
	AfxMessageBox( szBuffer,MB_OK|MB_ICONWARNING|MB_APPLMODAL );
}

//////////////////////////////////////////////////////////////////////////
void Log( const char *format,... )
{
	va_list	ArgList;
	char		szBuffer[MAX_LOGBUFFER_SIZE];

	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	CLogFile::WriteLine(szBuffer);
}


/*
//////////////////////////////////////////////////////////////////////////
void Log( const char *format,... )
{
	va_list arg;
	va_start(arg, szFormat);
	LogV (eError, szFormat, arg);
	va_end(arg);
}
*/

//////////////////////////////////////////////////////////////////////////
inline void RemoveEndLine( char *str )
{
	int len = strlen(str);
	if (len > 0 && str[len-1] == '\n')
	{
		str[len-1] = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogFile::CLogFile()
{
	if (!m_bFileOpened)
		OpenFile();
}

CLogFile::~CLogFile()
{

}

//////////////////////////////////////////////////////////////////////////
const char*	CLogFile::GetFileName()
{
	return GetLogFileName();
}


//////////////////////////////////////////////////////////////////////////
void CLogFile::EnableVerbosity( bool bEnable )
{
	ISystem *pSystem = GetIEditor()->GetSystem();
	if (!pSystem)
		return;

	m_console = pSystem->GetIConsole();

	if (bEnable)
	{
		if (!m_pLogVerbosity)
		{
			if (pSystem->GetIConsole())
				m_pLogVerbosity = pSystem->GetIConsole()->CreateVariable("log_Verbosity","3",VF_DUMPTODISK);
		}
	}
	else
	{
		if (pSystem->GetIConsole())
			pSystem->GetIConsole()->UnregisterVariable("log_Verbosity",true);
		m_pLogVerbosity = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CLogFile::SetVerbosity( int verbosity )
{
	EnableVerbosity(true);
	if (m_pLogVerbosity)
		m_pLogVerbosity->Set(verbosity);
}

//////////////////////////////////////////////////////////////////////////
int CLogFile::GetVerbosityLevel()
{
	if (m_pLogVerbosity)
		return (m_pLogVerbosity->GetIVal());
	return (0);
}

//////////////////////////////////////////////////////////////////////////
void CLogFile::OpenFile()
{
	if (!m_bFileOpened)
	{
		char szMasterCDPath[_MAX_PATH];
		GetCurrentDirectory( sizeof(szMasterCDPath),szMasterCDPath );
		CString masterCDFolder = Path::AddBackslash(szMasterCDPath);

		static char szLogFileName[_MAX_PATH];
		// Get Current Dir.
		strcpy( szLogFileName,masterCDFolder );
		// Add the logfile name
		strcat(szLogFileName, EDITOR_LOG_FILE );
		strcpy( m_szLogFilename,szLogFileName );

		DeleteFile( "error.log" );
		m_bFileOpened = true;

		// Clear log file.
		FILE *fp = fopen(GetLogFileName(),"wt");
		if (fp)
		{
			fclose(fp);
		}

		WriteLogHeader();
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
/*
void CLogFile::Log(int dwFlags,const char *command,...)
{
}
*/

void CLogFile::Log( const char* szFormat, ... )
{
	va_list arg;
	va_start(arg, szFormat);
	LogV (eMessage, szFormat, arg);
	va_end(arg);
}

void CLogFile::LogWarning( const char* szFormat, ... )
{
	va_list arg;
	va_start(arg, szFormat);
	LogV (eWarning, szFormat, arg);
	va_end(arg);
}

void CLogFile::LogError( const char* szFormat, ... )
{
	va_list arg;
	va_start(arg, szFormat);
	LogV (eError, szFormat, arg);
	va_end(arg);
}


//////////////////////////////////////////////////////////////////////////
void CLogFile::LogV(const ELogType ineType, const char* format, va_list arglist )
{
	if (!format)
		return;

	bool bOnlyFile = false;
	const char* szFormat = CheckAgainstVerbosity(format,bOnlyFile);
	if (!szFormat)
	{
		return;
	}

	char		buf[MAX_LOGBUFFER_SIZE];
	
	_vsnprintf(buf, sizeof(buf), szFormat, arglist);
	
	RemoveEndLine(buf);

	char str[MAX_LOGBUFFER_SIZE];
	_snprintf( str,sizeof(str),"<Engine> %s",buf );
	LogLine( str,bOnlyFile );
}

//////////////////////////////////////////////////////////////////////////
void CLogFile::LogPlus(const char *format,...)
{
	bool bOnlyFile = false;
	const char* szFormat = CheckAgainstVerbosity(format,bOnlyFile);
	if (!szFormat)
		return;

	va_list		arglist;
	char		buf[MAX_LOGBUFFER_SIZE];

	va_start(arglist, format);
	vsprintf(buf, szFormat, arglist);
	va_end(arglist);	

	LogString( buf,bOnlyFile );
}

//log to console only
//////////////////////////////////////////////////////////////////////
void CLogFile::LogToConsole(const char *format,...)
{
	bool bOnlyFile = false;
	const char* szFormat = CheckAgainstVerbosity(format,bOnlyFile);
	if (!szFormat)
		return;

	va_list		arglist;
	char		buf[MAX_LOGBUFFER_SIZE];

	va_start(arglist, format);
	vsprintf(buf, szFormat, arglist);
	va_end(arglist);	

	RemoveEndLine(buf);
	
  char str[MAX_LOGBUFFER_SIZE];
	sprintf( str,"<Engine> %s",buf );
	LogLine( str,bOnlyFile );
}

//useless function that add a string to the previous line in console
//////////////////////////////////////////////////////////////////////
void CLogFile::LogToConsolePlus(const char *format,...)
{
	bool bOnlyFile = false;
	const char* szFormat = CheckAgainstVerbosity(format,bOnlyFile);
	if (!szFormat)
		return;

	va_list		arglist;
	char		buf[MAX_LOGBUFFER_SIZE];

	va_start(arglist, format);
	vsprintf(buf, szFormat, arglist);
	va_end(arglist);	
	
	LogString( buf,bOnlyFile );
}

//same as above but to a file
//////////////////////////////////////////////////////////////////////
void CLogFile::LogToFilePlus(const char *format,...)
{
	va_list		arglist;
	char		buf[MAX_LOGBUFFER_SIZE];

	va_start(arglist, format);
	vsprintf(buf, format, arglist);
	va_end(arglist);	

	LogString( buf,true );
}

//log to the file specified in setfilename
//////////////////////////////////////////////////////////////////////
void CLogFile::LogToFile(const char *szFormatIn,...)
{
	bool bOnlyFile = true;
	const char* szFormat = CheckAgainstVerbosity(szFormatIn, bOnlyFile);
	if (!szFormat)
		return;
	
	va_list		arglist;
	char		buf[MAX_LOGBUFFER_SIZE];

	va_start(arglist, szFormatIn);
	strcpy(buf,"<Engine> ");
	unsigned nPrefixLen = strlen(buf);
	unsigned nBufSizeRemains = sizeof(buf) - nPrefixLen - 2;
	_vsnprintf(buf + nPrefixLen, nBufSizeRemains, szFormat, arglist);
	buf[sizeof(buf)-1] = '\0';
	va_end(arglist);

	RemoveEndLine(buf);
	
	LogLine( buf,true );
}

void CLogFile::UpdateLoadingScreen(const char *format,...)
{
	bool bOnlyFile = false;
	const char* szFormat = CheckAgainstVerbosity(format,bOnlyFile);
	if (!szFormat)
		return;

	va_list		arglist;
	char		buf[MAX_LOGBUFFER_SIZE];

	va_start(arglist, format);
	vsprintf(buf, szFormat, arglist);
	va_end(arglist);

	RemoveEndLine(buf);
	
  char str[MAX_LOGBUFFER_SIZE];
	sprintf( str,"<Engine> %s",buf );
	LogLine( str,bOnlyFile );
}

void CLogFile::UpdateLoadingScreenPlus(const char *format,...)
{
	bool bOnlyFile = false;
	const char* szFormat = CheckAgainstVerbosity(format,bOnlyFile);
	if (!szFormat)
		return;

	va_list		arglist;
	char		buf[MAX_LOGBUFFER_SIZE];

	va_start(arglist, format);
	vsprintf(buf, szFormat, arglist);
	va_end(arglist);	
	
	LogString( buf,bOnlyFile );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CLogFile::LogString(const char * pszString,bool onlyFile)
{
	if (!m_bFileOpened)
		OpenFile();

	CString str = pszString;
	if (m_bShowMemUsage)
	{
		str = CString("(") + GetMemUsage() + ")" + str;
	}

	////////////////////////////////////////////////////////////////////////
	// Write a string to the logfile
	////////////////////////////////////////////////////////////////////////

	FILE *hFile = NULL;
	PSTR pszText = NULL;

	// Open the file
	hFile = fopen(GetLogFileName(), "a");

	if (!hFile)
	{
#if defined(LINUX)
		printf(("Critical Error: Can't open / access log file !\n");
#endif
		return;
	}

	// Write the string to the file and close it
	fprintf(hFile, "%s", (const char*)str );
	fclose(hFile);

	if (onlyFile)
		return;

	// If we have a listbox attached, also output the string to this listbox
	if (m_hWndListBox)
	{
	
		// Add the string to the listbox
		SendMessage(m_hWndListBox, LB_ADDSTRING, 0, (LPARAM) (const char*)str );

		// Make sure the recently added string is visible
		SendMessage(m_hWndListBox, LB_SETTOPINDEX, 
			SendMessage(m_hWndListBox, LB_GETCOUNT, 0, 0) - 1, 0);
	}

	bool bNewLine = str[0] == '\n';

	if (m_console)
	{
		if (bNewLine)
		{
			m_console->PrintLine(str);
		}
		else
		{
			m_console->PrintLinePlus(str);
		}
	}

	if (m_hWndEditBox)
	{
		if (bNewLine)
		{
			//str = CString("\r\n") + str.TrimLeft();
			//str = CString("\r\n") + str;
			str = CString("\r") + str;
		}
		const char *szStr = str;
		SendMessage( m_hWndEditBox,EM_REPLACESEL, FALSE, (LPARAM)szStr );
	}
}

void CLogFile::LogLine(const char * pszString,bool bOnlyFile)
{
	////////////////////////////////////////////////////////////////////////
	// Write a line to the logfile
	////////////////////////////////////////////////////////////////////////
	CString str = pszString;
	LogString( CString("\n")+str,bOnlyFile );
}

PSTR CLogFile::GetLogFileName()
{
	// Return the path
	return m_szLogFilename;
}

void CLogFile::FormatLine(PSTR format,...)
{
	////////////////////////////////////////////////////////////////////////
	// Write a line with printf style formatting
	////////////////////////////////////////////////////////////////////////

	va_list		ArgList;
	char		szBuffer[MAX_LOGBUFFER_SIZE];

	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	LogLine(szBuffer);
}

void CLogFile::WriteLogHeader()
{
	////////////////////////////////////////////////////////////////////////
	// Write the header for a new editor session (Editor version, windows
	// version etc.)
	////////////////////////////////////////////////////////////////////////

	// Header
	LogLine("----------------------------=== Cry Editor Log File ===----------------------------");

	Version fileVersion;
	Version productVersion;

	char exe[_MAX_PATH];
	DWORD dwHandle;
	UINT len;

	char ver[MAX_LOGBUFFER_SIZE];

//	GetModuleFileName( NULL, exe, _MAX_PATH );
	strcpy(exe,"CrySystem.dll");			// we want to version from the system dll (FarCry.exe we cannot change because of CopyProtection)

	int verSize = GetFileVersionInfoSize( exe,&dwHandle );
	if (verSize > 0)
	{
		GetFileVersionInfo( exe,dwHandle,MAX_LOGBUFFER_SIZE,ver );
		VS_FIXEDFILEINFO *vinfo;
		VerQueryValue( ver,"\\",(void**)&vinfo,&len );

		fileVersion.v[0] = vinfo->dwFileVersionLS & 0xFFFF;
		fileVersion.v[1] = vinfo->dwFileVersionLS >> 16;
		fileVersion.v[2] = vinfo->dwFileVersionMS & 0xFFFF;
		fileVersion.v[3] = vinfo->dwFileVersionMS >> 16;

		productVersion.v[0] = vinfo->dwProductVersionLS & 0xFFFF;
		productVersion.v[1] = vinfo->dwProductVersionLS >> 16;
		productVersion.v[2] = vinfo->dwProductVersionMS & 0xFFFF;
		productVersion.v[3] = vinfo->dwProductVersionMS >> 16;
	}

	//! Get time.
	time_t ltime;
	time( &ltime );
	tm *today = localtime( &ltime );

	char s[MAX_LOGBUFFER_SIZE];
	//! Use strftime to build a customized time string.
	//strftime( timebuf,128,"Logged at %A, %B %d,%Y\n\n", today );
	strftime( s,128,"Log Started at %#c", today );
	LogLine( s );
	sprintf( s,"FileVersion: %d.%d.%d.%d",fileVersion.v[3],fileVersion.v[2],fileVersion.v[1],fileVersion.v[0] );
	LogLine( s );
	sprintf( s,"ProductVersion: %d.%d.%d.%d",productVersion.v[3],productVersion.v[2],productVersion.v[1],productVersion.v[0] );
	LogLine( s );
	// Write system configuration
	AboutSystem();
	LogLine(" ");
}

void CLogFile::AboutSystem()
{
	//////////////////////////////////////////////////////////////////////
	// Write the system informations to the log
	//////////////////////////////////////////////////////////////////////

	char szBuffer[MAX_LOGBUFFER_SIZE];
	char szProfileBuffer[128];
	char szLanguageBuffer[64];
	//char szCPUModel[64];
	char *pChar = 0;
	MEMORYSTATUS MemoryStatus;
	DEVMODE DisplayConfig;
	OSVERSIONINFO OSVerInfo;
	OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	//////////////////////////////////////////////////////////////////////
	// Display editor and Windows version
	//////////////////////////////////////////////////////////////////////

	// Get system language
	GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SENGLANGUAGE, 
		szLanguageBuffer, sizeof(szLanguageBuffer));

	// Format and send OS information line
	sprintf(szBuffer, "Current Language: %s ", szLanguageBuffer);
	LogLine(szBuffer);

	// Format and send OS version line
	CString str = "Windows ";
	GetVersionEx(&OSVerInfo);
	if (OSVerInfo.dwMajorVersion == 4)
	{
		if (OSVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			if (OSVerInfo.dwMinorVersion > 0)
				// Windows 98
				str += "98";
			else
				// Windows 95
				str += "95";
		}
		else
			// Windows NT
			str += "NT";
	}
	else if (OSVerInfo.dwMajorVersion == 5)
	{
		if (OSVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
			// Windows Millenium
			str += "ME";
		else
		{
			if (OSVerInfo.dwMinorVersion > 0)
				// Windows XP
				str += "XP";
			else
				// Windows 2000
				str += "2000";
		}
	}
	sprintf(szBuffer, " %d.%d", OSVerInfo.dwMajorVersion, OSVerInfo.dwMinorVersion);
	str += szBuffer;

	//////////////////////////////////////////////////////////////////////
	// Show Windows directory
	//////////////////////////////////////////////////////////////////////

	str += " (";
	GetWindowsDirectory(szBuffer, sizeof(szBuffer));
	str += szBuffer;
	str += ")";
	LogLine(str);
	
	//////////////////////////////////////////////////////////////////////
	// Send system time & date
	//////////////////////////////////////////////////////////////////////

	str = "Local time is ";
	_strtime(szBuffer);
	str += szBuffer;
	str += " ";
	_strdate(szBuffer);
	str += szBuffer;
	sprintf(szBuffer, ", system running for %d minutes", GetTickCount() / 60000);
	str += szBuffer;
	LogLine(str);
	
	//////////////////////////////////////////////////////////////////////
	// Send system CPU informations
	//////////////////////////////////////////////////////////////////////

	/*
	GetCPUModel(szCPUModel);
#ifdef _DEBUG
	sprintf(szBuffer, "System CPU is an %s running at %d Mhz", 
		szCPUModel, GetCPUSpeed());
#else
	sprintf(szBuffer, "System CPU is an %s, frequency only measured in debug versions", 
		szCPUModel);
#endif
	LogLine(szBuffer);
	*/

	//////////////////////////////////////////////////////////////////////
	// Send system memory status
	//////////////////////////////////////////////////////////////////////

	GlobalMemoryStatus(&MemoryStatus);
	sprintf(szBuffer, "%dMB phys. memory installed, %dMB paging available", 
		MemoryStatus.dwTotalPhys / 1048576 + 1, 
		MemoryStatus.dwAvailPageFile / 1048576);
	LogLine(szBuffer);

	//////////////////////////////////////////////////////////////////////
	// Send display settings
	//////////////////////////////////////////////////////////////////////

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DisplayConfig);
	GetPrivateProfileString("boot.description", "display.drv", 
		"(Unknown graphics card)", szProfileBuffer, sizeof(szProfileBuffer), 
		"system.ini");
	sprintf(szBuffer, "Current display mode is %dx%dx%d, %s",
		DisplayConfig.dmPelsWidth, DisplayConfig.dmPelsHeight,
		DisplayConfig.dmBitsPerPel, szProfileBuffer);
	LogLine(szBuffer);

	//////////////////////////////////////////////////////////////////////
	// Send input device configuration
	//////////////////////////////////////////////////////////////////////

	str = "";
	// Detect the keyboard type
	switch (GetKeyboardType(0))
	{
		case 1:
			str = "IBM PC/XT (83-key)";
			break;
		case 2:
			str = "ICO (102-key)";
			break;
		case 3:
			str = "IBM PC/AT (84-key)";
			break;
		case 4:
			str = "IBM enhanced (101/102-key)";
			break;
		case 5:
			str = "Nokia 1050";
			break;
		case 6:
			str = "Nokia 9140";
			break;
		case 7:
			str = "Japanese";
			break;
		default:
			str = "Unknown";
			break;
	}

	// Any mouse attached ?
	if (!GetSystemMetrics(SM_MOUSEPRESENT))
		LogLine( str + " keyboard and no mouse installed");
	else
	{
		sprintf(szBuffer, " keyboard and %i+ button mouse installed", 
			GetSystemMetrics(SM_CMOUSEBUTTONS));
		LogLine( str + szBuffer);
	}

	LogLine("--------------------------------------------------------------------------------");
}

unsigned int CLogFile::GetCPUSpeed()
{
	//////////////////////////////////////////////////////////////////////
	// Return system CPU speed in Mhz (Code by Frank Blaha)
	//////////////////////////////////////////////////////////////////////

	/*
	Basic goal is use high precision performance counter.
	Reason is described in VC HELP as follow:

	Each time the specified interval (or time-out value) 
	for a timer elapses, the system notifies the window associated 
	with the timer. 

	Because the accuracy of a timer depends on the system clock 
	rate and how often the application retrieves messages from 
	the message queue, the time-out value is only approximate. 
	If you need a timer with higher precision, use the high-resolution 
	timer. For more information, see High-Resolution Timer. 
	If you need to be notified when a timer elapses, 
	use the waitable timers. 
	For more information on these, see Waitable Timer Objects. 

	About RDSTC instruction:

	Beginning with the Pentium« processor, Intel processors allow the 
	programmer to access a time-stamp counter. 
	The time-stamp counter keeps an accurate count of every cycle 
	that occurs on the processor. The Intel time-stamp counter is a 
	64-bit MSR (model specific register) that is incremented every 
	clock cycle. 
	On reset, the time-stamp counter is set to zero.

	To access this counter, programmers can use the 
	RDTSC (read time-stamp counter) instruction. 
	This instruction loads the high-order 32 bits of 
	the register into EDX, and the low-order 32 bits into EAX.

	Remember that the time-stamp counter measures "cycles" and not "time". 
	For example, two hundred million cycles on a 200 MHz processor is equivalent
	to one second of real time, while the same number of cycles on a 400 MHz
	processor is only one-half second of real time. Thus, comparing cycle counts
	only makes sense on processors of the same speed. To compare processors of
	different speeds, the cycle counts should be converted into time units, where: 

	# seconds = # cycles / frequency

	Note: frequency is given in Hz, where: 1,000,000 Hz = 1 MHz

	*/

	/*
	LARGE_INTEGER   ulFreq,			// No. ticks/s ( frequency). 
									// This is exactly 1 second interval
					ulTicks,		// Curent value of ticks 
					ulValue,		// for calculation, how many tick is system
									// (depend on instaled HW) able    
					ulStartCounter, // Start No. of processor counter
					ulEAX_EDX,		// We need 64 bits value and it is stored in EAX, EDX registry
					ulResult;		// Calculate result of "measuring"

	// Function retrieves the frequency of the high-resolution performance counter (HW not CPU)   
	// It is number of ticks per second       
	QueryPerformanceFrequency(&ulFreq); 	   

	// Current value of the performance counter        
    QueryPerformanceCounter(&ulTicks);   

	// Calculate one sec interval 
	// ONE SEC interval  = start nuber of the ticks + # of ticks/s
	// Loop (do..while statement bellow) until actual # of ticks this number is <= then 1 sec

	ulValue.QuadPart = ulTicks.QuadPart + ulFreq.QuadPart;    

	// (read time-stamp counter) instruction.    
	// This asm instruction loads the high-order 32 bits of the register into EDX, and the low-order 32 bits into EAX.      

	__asm RDTSC     

	// Load 64-bits counter from registry to LARGE_INTEGER variable (take a look to HELP)
	__asm mov ulEAX_EDX.LowPart, EAX         
	__asm mov ulEAX_EDX.HighPart, EDX       

	// Starting number of processor ticks    

	ulStartCounter.QuadPart = ulEAX_EDX.QuadPart;                 

	// Loop for 1 sec and  check ticks        
	// this is descibed bellow
	do
	{	         
		// Just read actual HW counter
		QueryPerformanceCounter(&ulTicks); 
	} while (ulTicks.QuadPart <= ulValue.QuadPart);

	// Get actual number of processor ticks      
	__asm RDTSC       

	__asm mov ulEAX_EDX.LowPart, EAX        
	__asm mov ulEAX_EDX.HighPart,EDX       

	// Calculate result from current processor ticks count
	ulResult.QuadPart = ulEAX_EDX.QuadPart - ulStartCounter.QuadPart;     

	// Return the value
	return (unsigned int) ulResult.QuadPart / 1000000;
	*/
	return 0;
}

void CLogFile::GetCPUModel(char szType[])
{
	/*
	//////////////////////////////////////////////////////////////////////
	// Get the CPU Model
	//////////////////////////////////////////////////////////////////////

	int CPUfamily,InstCache,DataCache,L2Cache;
	char VendorID[16];
	bool SupportCMOVs,Support3DNow,Support3DNowExt,SupportMMX,SupportMMXext,SupportSSE;

	#ifndef CPUID
	#define CPUID __asm _emit 0x0F __asm _emit 0xA2
	#endif
	#ifndef RDTSC
	#define RDTSC __asm _emit 0x0F __asm _emit 0x31
	#endif

	__asm {
		PUSHFD
		POP		EAX
		MOV		EBX, EAX
		XOR		EAX, 00200000h
		PUSH	EAX
		POPFD
		PUSHFD
		POP		EAX
		CMP		EAX, EBX
		JZ		ExitCpuTest

			XOR		EAX, EAX
			CPUID

			MOV		DWORD PTR [VendorID],		EBX
			MOV		DWORD PTR [VendorID + 4],	EDX
			MOV		DWORD PTR [VendorID + 8],	ECX
			MOV		DWORD PTR [VendorID + 12],	0

			MOV		EAX, 1
			CPUID
			TEST	EDX, 0x00008000
			SETNZ	AL
			MOV		SupportCMOVs, AL
			TEST	EDX, 0x00800000
			SETNZ	AL
			MOV		SupportMMX, AL
	
			TEST	EDX, 0x02000000
			SETNZ	AL
			MOV		SupportSSE, AL

			SHR		EAX, 8
			AND		EAX, 0x0000000F
			MOV		CPUfamily, EAX
	
			MOV		Support3DNow, 0
			MOV		EAX, 80000000h
			CPUID
			CMP		EAX, 80000000h
			JBE		NoExtendedFunction
				MOV		EAX, 80000001h
				CPUID
				TEST	EDX, 80000000h
				SETNZ	AL
				MOV		Support3DNow, AL

				TEST	EDX, 40000000h
				SETNZ	AL
				MOV		Support3DNowExt, AL

				TEST	EDX, 0x00400000
				SETNZ	AL
				MOV		SupportMMXext, AL

				MOV		EAX, 80000005h
				CPUID
				SHR		ECX, 24
				MOV		DataCache, ECX
				SHR		EDX, 24
				MOV		InstCache, EDX
				
				MOV		EAX, 80000006h
				CPUID
				SHR		ECX, 16
				MOV		L2Cache, ECX

				
			JMP		ExitCpuTest

			NoExtendedFunction:
			MOV		EAX, 2
			CPUID

			MOV		ESI, 4
			TestCache:
				CMP		DL, 0x40
				JNA		NotL2
					MOV		CL, DL
					SUB		CL, 0x40
					SETZ	CH
					DEC		CH
					AND		CL, CH
					MOV		EBX, 64
					SHL		EBX, CL
					MOV		L2Cache, EBX
				NotL2:
				CMP		DL, 0x06
				JNE		Next1
					MOV		InstCache, 8
				Next1:
				CMP		DL, 0x08
				JNE		Next2
					MOV		InstCache, 16
				Next2:
				CMP		DL, 0x0A
				JNE		Next3
					MOV		DataCache, 8
				Next3:
				CMP		DL, 0x0C
				JNE		Next4
					MOV		DataCache, 16
				Next4:
				SHR		EDX, 8
				DEC		ESI
			JNZ	TestCache

		ExitCpuTest:
	}

	// Build the CPU ID string
	sprintf(szType, "%s (%s%s%s)", VendorID, 
		(SupportMMX || SupportMMXext) ? "MMX, " : "No MMX, ",
		(Support3DNow || Support3DNowExt) ? "3DNow!, " : "No 3DNow!, ",
		SupportSSE ? "SSE" : "No SSE");
		*/
}

//////////////////////////////////////////////////////////////////////////
CString CLogFile::GetMemUsage()
{
	ProcessMemInfo mi;
	CProcessInfo::QueryMemInfo( mi );
	int MB = 1024*1024;

	CString str;
	str.Format( "Memory=%dMb, Pagefile=%dMb",mi.WorkingSet/MB,mi.PagefileUsage/MB );
	//FormatLine( "PeakWorkingSet=%dMb, PeakPagefileUsage=%dMb",pc.PeakWorkingSetSize/MB,pc.PeakPagefileUsage/MB );
	//FormatLine( "PagedPoolUsage=%d",pc.QuotaPagedPoolUsage/MB );
	//FormatLine( "NonPagedPoolUsage=%d",pc.QuotaNonPagedPoolUsage/MB );
	
	return str;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Checks the verbosity of the message and returns NULL if the message must NOT be
// logged, or the pointer to the part of the message that should be logged
// NOTE:
//    Normally, this is either the pText pointer itself, or the pText+1, meaning
//    the first verbosity character may be cut off)
//    This is done in order to avoid modification of const char*, which may cause GPF
//    sometimes, or kill the verbosity qualifier in the text that's gonna be passed next time.
const char* CLogFile::CheckAgainstVerbosity(const char * pText,bool &bOnlyFile )
{
	// the max verbosity (most detailed level)
	const unsigned char nMaxVerbosity = 8;

	if (!pText)
		return 0;
	
	// the verbosity of unqualified strings not really needed
//	const unsigned char nDefaultVerbosity = nMaxVerbosity;
	
	// the current verbosity of the log
	unsigned int nLogVerbosity = m_pLogVerbosity ? m_pLogVerbosity->GetIVal() : nMaxVerbosity;

	int textVerbosity = CheckVerbosity(pText);
	if ((unsigned char)pText[0] > nMaxVerbosity) 
	{
	 if (nLogVerbosity >= nMaxVerbosity)			
				return pText;															
		else
		{
			bOnlyFile = true;
			return pText;
		}
	}
	else
	{
		if (nLogVerbosity >= (unsigned char)pText[0])
			// if the text is an empty string, a NULL will be returned
			return pText[0] ? pText + 1 : NULL;
		else
		{
			bOnlyFile = true;
			return pText[0] ? pText + 1 : NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int CLogFile::CheckVerbosity( const char * pText )
{
	// the max verbosity (most detailed level)
	const unsigned char nMaxVerbosity = 5;

	// set message verbosity for error and warning messages
	char sBuff[256]; 
	strncpy(sBuff,pText,255);			// todo: optimize - copy if only neccessary to use strlwr because strstr is case sensitive
	sBuff[255]=0; 
	strlwr(sBuff);
	if(strstr(sBuff,"error"))
		return 1;
	if(strstr(sBuff,"warning"))
		return 3;
	
	int textVerbosity = (unsigned char)pText[0];
	if (textVerbosity > nMaxVerbosity) 
	{
		return nMaxVerbosity;
	}
	else
		return textVerbosity;
}

//////////////////////////////////////////////////////////////////////////
void CLogFile::WriteLine( const char * pszString )
{
	LogLine( pszString );
}

//////////////////////////////////////////////////////////////////////////
void CLogFile::WriteString( const char * pszString )
{
	LogString( pszString );
}