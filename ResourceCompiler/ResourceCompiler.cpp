// ResourceCompiler.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <time.h>
#include <DbgHelp.h>
#include <io.h>
#include "ResourceCompiler.h"
#include "CmdLine.h"
#include "Config.h"
#include "CfgFile.h"
#include "FileUtil.h"
#include "IConvertor.h"
#include "FileUtil.h"

#include "CryChunkedFile.h"
#include "CgfUtils.h"
#include <StringUtils.h>

//! Section in rc.ini file for common settings.
#define COMMON_SECTION "Common"

static const char *RC_FILENAME_LOG=					"rc_log.log";
static const char *RC_FILENAME_WARNINGS=		"rc_log_warnings.log";
static const char *RC_FILENAME_ERRORS=			"rc_log_errors.log";
static const char *RC_FILENAME_FILEDEP=			"rc_stats_filedependencies.log";
static const char *RC_FILENAME_MATDEP=			"rc_stats_materialdependencies.log";

//////////////////////////////////////////////////////////////////////////
// Globals.
//////////////////////////////////////////////////////////////////////////

// Determines whether a path to a file system object such as a file or directory is valid

BOOL RCPathFileExists (const char* szPath)
{
	DWORD dwAttr = GetFileAttributes (szPath);
	return (dwAttr != 0xFFFFFFFF);
}


//////////////////////////////////////////////////////////////////////////
// ResourceCompiler implementation.
//////////////////////////////////////////////////////////////////////////
ResourceCompiler::ResourceCompiler()
{
	m_config = 0;
	m_pIPhysicalWorld = 0;

	m_hLogFile=0;
	m_hErrorLogFile=0;
	m_hWarningLogFile=0;
	m_bWarningHeaderLine=false;
	m_bErrorHeaderLine=false;
	m_bStatistics = false;
	m_bQuiet = false;
}

ResourceCompiler::~ResourceCompiler()
{
	if (m_pIPhysicalWorld)
	{
		m_pIPhysicalWorld->Release();
		m_pIPhysicalWorld = NULL;
	}

	// close files if open
	if(m_hLogFile)fclose(m_hLogFile);m_hLogFile=0;
	if(m_hErrorLogFile)fclose(m_hErrorLogFile);m_hErrorLogFile=0;
	if(m_hWarningLogFile)fclose(m_hWarningLogFile);m_hWarningLogFile=0;
}

//////////////////////////////////////////////////////////////////////////
void ResourceCompiler::RegisterConvertor( IConvertor *conv )
{
	m_extensionManager.RegisterConvertor( conv, this );
}

//////////////////////////////////////////////////////////////////////////
FILE*	ResourceCompiler::OpenFile( const char *filename,const char *mode )
{
	FILE *file = fopen(filename,mode);
	// check if read only.
	return file;
}

IRCLog *ResourceCompiler::GetIRCLog()
{
	return(this);
}

// returns the file unix time - the latest of modification and creation times
DWORD ResourceCompiler::GetFileUnixTimeMax (const char* filename)
{
	FILETIME ftWrite, ftCreate;
	if (GetFileTime(filename, &ftWrite, &ftCreate))
	{
		return max (FileUtil::FiletimeToUnixTime(ftWrite),FileUtil::FiletimeToUnixTime(ftCreate));
	}
	else
		return 0;
}

// returns the file unix time - the earliest of modification and creation times
DWORD ResourceCompiler::GetFileUnixTimeMin (const char* filename)
{
	FILETIME ftWrite, ftCreate;
	if (GetFileTime(filename, &ftWrite, &ftCreate))
	{
		return min (FileUtil::FiletimeToUnixTime(ftWrite),FileUtil::FiletimeToUnixTime(ftCreate));
	}
	else
		return 0;
}


//////////////////////////////////////////////////////////////////////////
bool ResourceCompiler::GetFileTime( const char *filename,FILETIME *ftimeModify, FILETIME*ftimeCreate )
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	hFind = FindFirstFile( filename,&FindFileData );
  if (hFind == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	FindClose(hFind);
	if (ftimeCreate)
	{
		ftimeCreate->dwLowDateTime = FindFileData.ftCreationTime.dwLowDateTime;
		ftimeCreate->dwHighDateTime = FindFileData.ftCreationTime.dwHighDateTime;
	}
	if (ftimeModify)
	{
		ftimeModify->dwLowDateTime = FindFileData.ftLastWriteTime.dwLowDateTime;
		ftimeModify->dwHighDateTime = FindFileData.ftLastWriteTime.dwHighDateTime;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
const char* ResourceCompiler::GetSectionName( Platform platform ) const
{
	switch (platform)
	{
	case PLATFORM_PC:				return "PC";
	case PLATFORM_XBOX:			return "XBOX";
	case PLATFORM_PS2:			return "PS2";
	case PLATFORM_GAMECUBE:	return "GAMECUBE";
	default:
		// unknown platform.
		MessageBoxError( _T("Section name requested for unknown platform") );
		assert(0);
	}
	return "";
}

void ResourceCompiler::RemoveOutputFiles()
{
	DeleteFile(RC_FILENAME_LOG);
	DeleteFile(RC_FILENAME_WARNINGS);
	DeleteFile(RC_FILENAME_ERRORS);
	DeleteFile(RC_FILENAME_FILEDEP);
	DeleteFile(RC_FILENAME_MATDEP);
}


//////////////////////////////////////////////////////////////////////////
// Returns true if successfully converted at least one file
bool ResourceCompiler::Compile( Platform platform,IConfig *config,const char *filespec )
{
	RemoveOutputFiles();		// to remove old files for less confusion

	if (m_MainConfig.HasKey("statistics"))
		m_bStatistics = true;
	else
		m_bStatistics = false;

	m_bQuiet = config->HasKey("quiet");

	if(!config->HasKey("logfiles"))
	{
		m_hLogFile=fopen(RC_FILENAME_LOG,"wb");
	}
	{
		m_hWarningLogFile=fopen(RC_FILENAME_WARNINGS,"wb");
		m_hErrorLogFile=fopen(RC_FILENAME_ERRORS,"wb");
	}

	m_config = config;
	m_platform = platform;

	DWORD dwFileSpecAttr = GetFileAttributes (filespec);

	std::vector<CString> arrFiles; // files to convert, with relative paths

	bool bRecursive = config->GetAs<bool>("recursive", true);
	
	m_presets = new CfgFile();
	CString presetcfg;

	if(!config->Get("presetcfg", presetcfg))
	{
		Log("No preset configuration defined (e.g. presetcfg=rc_presets_pc.ini)");

// we should uncoment this soon (MM 05/28/2002)
//		Log("    exiting...");return false;     // it's better to have resource not working without that info
	}
	else if(!m_presets->Load(presetcfg))
	{
		Log("Failed to read preset configuration %s, exiting...", presetcfg.GetString());
		return false;
	};
	
	CString path = Path::GetPath(filespec);
	if (dwFileSpecAttr == 0xFFFFFFFF)
	{
		// there's no such file; so, this is probably a mask:
		// path\*.mask
		
		// Scan all files matching filespec.
		FileUtil::ScanDirectory( path,Path::GetFile(filespec),arrFiles, bRecursive);
	}
	else
	if (dwFileSpecAttr & FILE_ATTRIBUTE_DIRECTORY)
	{
		path = Path::AddBackslash(filespec);
		
		// it's a directory; the mask can be found via /file=... option
		FileUtil::ScanDirectory(path, config->GetAs<CString>("file", "*.*"), arrFiles, bRecursive);
	}
	else
		arrFiles.push_back(Path::GetFile(filespec));

	if (arrFiles.empty())
	{
		LogError( "The system cannot find the file specified, 0 file(s) converted" );
		return false;
	}
	
	// determine the target output path (may be a different directory structure)
	// if none is specified, the target is the same as the source, as before.
	CString targetroot;
	if (!config->Get( "targetroot", targetroot ))
	{
		targetroot = path;
	};	
	targetroot = Path::AddBackslash(targetroot);

	// these are the files that couldn't be converted
	std::vector<CString> arrNonConvertedFiles;

	// the number of files that were successfully converted
	unsigned numFilesConverted = 0;

	int nTimer = GetTickCount();

	size_t i, iSize=arrFiles.size();

	for (i = 0; i < iSize; i++)
	{
		// show progress
		{
			int iPercentage=(100*i)/(iSize);
			char str[0x100];

			_snprintf(str, sizeof(str),"Progress: %3d%% %s",iPercentage,arrFiles[i]);

			SetConsoleTitle(str);
		}

		CString strFileName = path + arrFiles[i];
		if (CompileFile( strFileName.GetString(),targetroot.GetString(), Path::GetPath(CString(targetroot+arrFiles[i])).GetString()))
			++numFilesConverted;
		else
			arrNonConvertedFiles.push_back(strFileName);
	}

	nTimer = GetTickCount() - nTimer;
	char szTimeMsg[128] ;
	szTimeMsg[0] = '\0';
	if (nTimer > 500)
		sprintf (szTimeMsg, " in %.1f sec", nTimer/1000.0f);

	if (arrNonConvertedFiles.empty())
		Log ("%d file%s converted%s.", arrFiles.size(), arrFiles.size()>1?"s":"",szTimeMsg);
	else
	{
		Log("");
		Log("");
		Log ( "%d of %d file%s converted%s. Couldn't convert the following files:", numFilesConverted, arrFiles.size(), arrFiles.size() > 1 ? "s":"", szTimeMsg);
		Log("");
		for (i = 0; i < arrNonConvertedFiles.size(); ++i)
			Log ( "   %s", arrNonConvertedFiles[i]);
		Log("");
	}

	delete m_presets;

	return numFilesConverted > 0;
}

void ResourceCompiler::EnsureDirectoriesPresent(const char *path)
{
	DWORD dwFileSpecAttr = GetFileAttributes (path);
	if (dwFileSpecAttr == 0xFFFFFFFF && *path)
	{
		EnsureDirectoriesPresent(Path::GetPath(Path::RemoveBackslash(path)).GetString());
		Log("Creating directory %s (%s)", path, _mkdir(path) ? "failed" : "ok");
	};
};


// makes the relative path out of any
CString NormalizePath(const char* szPath)
{
	char szCurDir[0x800]="";
	GetCurrentDirectory(sizeof(szCurDir),szCurDir);
	strcat(szCurDir, "/");

	char szFullPath[0x800];
	if (!_fullpath(szFullPath, szPath, sizeof(szFullPath)))
		strcpy (szFullPath, szPath);


	char* p, *q;
	CString sRes = szPath;
	for (p = szCurDir, q = szFullPath; *p && *q; ++p, ++q)
	{
		if (tolower(*p)==tolower(*q))
			continue;

    if ((*p=='/'||*p=='\\')&&(*q=='/'||*q=='\\'))
			continue;

		return sRes;
	}

	if (*p)
		return szPath;

	return q; // return whatever has left after truncating the leading path
}

//////////////////////////////////////////////////////////////////////////
bool ResourceCompiler::CompileFile( const char *filename, const char *outroot, const char *outpath )
{
	CmdLine cmdLine;

	if (!RCPathFileExists(filename))
		return false;

	// get file extension.
	CString ext = Path::GetExt(filename);

	// get key for special copy/ignore options to certain extensions
	CString extkey = "ext_";
	extkey += ext;
	CString extcommand;
	m_config->Get(extkey.GetString(), extcommand);
	
	if(extcommand=="ignore")
	{
		Log("Ignoring %s", filename);
		return true;
	};
		
	if(extcommand=="copy")
	{
		CString dest = outpath;
		dest += Path::GetFile(filename);
		if(dest!=filename)
		{
			// TODO: can compare filestamps of source and destination to avoid copy, but maybe overkill
			Log("Copying %s to %s", filename, dest.GetString());
			CopyFile(filename, dest.GetString(), false); // overwrites any existing file, same as all converters
		};
		return true;
	};

	// find convertor matching platform and extension.
	IConvertor *conv = m_extensionManager.FindConvertor( m_platform,ext.GetString() );
	if (!conv)
	{
		// no convertor for this file.
		return false;
	}

	CString sourcePath = Path::GetPath(filename);

	CfgFile CfgFile;				// file specifig config file

	CString defFile = Path::ReplaceExtension(filename,DEF_FILE_EXTENSION);

	CfgFile.SetFileName(defFile);

	Config localConfig;

	// Check if definition file exist for specified filename
	if (RCPathFileExists(defFile.GetString()))
		if (CfgFile.Load( defFile ))
		{
			CfgFile.SetConfig( COMMON_SECTION,localConfig.GetInternalRepresentation() );
			CfgFile.SetConfig( GetSectionName(m_platform),localConfig.GetInternalRepresentation() );
		}

	localConfig.Merge(&m_MainConfig);

	// Setup conversion context.
	ConvertContext cc;

	cc.config								= &localConfig;
	cc.platform							= m_platform;
	cc.pRC									= this;
	cc.sourceFile						= Path::GetFile(filename);
	cc.sourceFolder         = Path::GetPath(filename);
	if (!m_config->Get("MasterFolder", cc.masterFolder))
		cc.masterFolder         = "";
	if (!cc.masterFolder.IsEmpty() && cc.masterFolder.Right(1)!="/" && cc.masterFolder.Right(1)!="\\")
		cc.masterFolder += '\\';

	cc.outputFile						= "";
	cc.outputFolder					= outpath;
	cc.pLog									= this;
	cc.pFileSpecificConfig	= &CfgFile;
	cc.presets              = m_presets;
	cc.bQuiet								= m_bQuiet;

	cc.sourceFolder = NormalizePath(cc.sourceFolder.GetString());
	cc.outputFolder = NormalizePath(cc.outputFolder.GetString());

	// Check if output file is valid (have same timestamp as input file).
	conv->GetOutputFile( cc );
	CString outputFileName = cc.outputFile;
	CString outputFile = cc.getOutputPath();
	//[Timur] outputFile, is Only filename. CString outputFile = outputFileName;

	//cc.sourceFile = filename;
	//cc.sourceFolder = "";

	const char *sOutFile = cc.outputFile.GetString();
	const char *sOutDir = cc.outputFolder.GetString();

	EnsureDirectoriesPresent(CString(cc.masterFolder+cc.outputFolder).GetString());
	
	// Compare time stamp of output file.
	if (!m_config->HasKey("refresh"))
	{
		unsigned nTimeSrc = GetFileUnixTimeMax( filename );
		unsigned nTimeTgt = GetFileUnixTimeMin( cc.getOutputPath().GetString() );
		unsigned nFilterTimestamp = conv->GetTimestamp();

		if (nTimeSrc < nTimeTgt && nFilterTimestamp < nTimeTgt)
		{
			// both Source and Filter code are older than target,
			// thus the target is up to date
			Log("Skipping %s: compiled file is up to date", filename);
			return true;	// wouter: was false
		}												
	}

	Log("");
	Log("-------------------------------------------------------");
	Log("Compiling %s", filename);
	Log("");

	//[Timur] 
	/*
//	IConfig *config = m_config;

	cc.config								= &localConfig;
	cc.platform							= m_platform;
	cc.pRC									= this;
	//cc.sourceFile						= filename;
	//cc.outputFile						= outputFile;
	//cc.outputFolder					= outroot;
	cc.pLog									= this;
	cc.pFileSpecificConfig	= &CfgFile;
	cc.presets              = m_presets;
*/

	OutputDebugString("Current file: '");
	OutputDebugString(filename);
	OutputDebugString("' ... ");

	// file name changed - print new header for warnings and errors
	SetHeaderLine(filename);

	//convert GCF into CCG
	bool bRet=conv->Process( cc );

	OutputDebugString("processed\n");

	if(!bRet)
		LogError("failed to convert file");

	// Release cloned config.
//	if (config != m_config)
//		config->Release();

	return bRet;
}


void ResourceCompiler::SetHeaderLine( const char *inszLine )
{
	m_bWarningHeaderLine=false;
	m_bErrorHeaderLine=false;
	m_sHeaderLine=inszLine;
}


void ResourceCompiler::LogLine( const ELogType ineType, const char* szText )
{
	if (m_bQuiet)
	{
		if(m_hLogFile)
		{
			fprintf(m_hLogFile,"%s\n",szText);
		}
		return;
	}

	switch(ineType)
	{
		case eMessage:
			printf ("  ");							// to make it aligned with E: and W:
			break;
		case eWarning:
			printf ("W: ");							// for Warning
			if(m_hWarningLogFile)
			{
				if(!m_bWarningHeaderLine)
				{
					fprintf(m_hWarningLogFile,"\r\n-----------------------------------------------------------------\r\n\r\n");
					fprintf(m_hWarningLogFile,"%s\r\n",m_sHeaderLine.c_str());
					m_bWarningHeaderLine=true;
				}

				fprintf(m_hWarningLogFile,"  %s\r\n",szText);
				fflush(m_hWarningLogFile);
			}
			break;
		case eError:
			printf ("E: ");							// for Error
			if(m_hErrorLogFile)
			{
				if(!m_bErrorHeaderLine)
				{
					fprintf(m_hErrorLogFile,"\r\n-----------------------------------------------------------------\r\n\r\n");
					fprintf(m_hErrorLogFile,"%s\r\n",m_sHeaderLine.c_str());
					m_bErrorHeaderLine=true;
				}

				fprintf(m_hErrorLogFile,"  %s\r\n",szText);
				fflush(m_hErrorLogFile);
			}
			break;

		default:assert(0);
	}

	if(m_hLogFile)
	{
		fprintf(m_hLogFile,"%s\r\n",szText);
		//fflush(m_hLogFile); // No need to flush really, we cannot reboot here.
	}

	printf("%s\n",szText);
}



//! Load and parse the Crytek Chunked File into the universal (very big) structure
//! The caller should then call Release on the structure to free the mem
//! @param filename Full filename including path to the file
CryChunkedFile* ResourceCompiler::LoadCryChunkedFile (const char* szFileName)
{
	CChunkFileReader_AutoPtr pReader = new CChunkFileReader ();
	if (!pReader->open (szFileName))
		return NULL;

	try
	{
		return new CryChunkedFile(pReader);
	}
	catch (CryChunkedFile::Error& e)
	{
		LogError("%s", e.strDesc.c_str());
		return NULL;
	}
	catch (...)
	{
		LogError("UNEXPECTED ERROR while trying to load Cry Chunked File \"%s\"", szFileName);
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Print error message.
void MessageBoxError( const char *format,... )
{
	va_list	ArgList;
	char		szBuffer[1024];

	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	CString str = "####-ERROR-####: ";
	str += szBuffer;

//	printf( "%s\n",str );
	MessageBox( NULL,szBuffer,_T("Error"),MB_OK|MB_ICONERROR );
}

/*
//////////////////////////////////////////////////////////////////////////
//! Log message.
void Log( const char *format,... )
{
	va_list	ArgList;
	char		szBuffer[1024];

	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	printf( "%s\n",szBuffer );
}
*/

//////////////////////////////////////////////////////////////////////////
void ResourceCompiler::show_help()
{
	Log( "Usage: RC filespec /p=<Platform> [/Key1=Value1] [/Key2=Value2] etc..." );
	Log( "" );
	Log( "  /p\tSpecifies target compilation platform." );
	Log( "    \tValid platforms: PC,XBOX,PS2,GC" );
	Log( "  /recursive");
}

//////////////////////////////////////////////////////////////////////////
static Platform GetPlatformFromName( const char *sPlatform )
{
	// Platform name to enum mapping.
	struct {
		const char *name;
		Platform platform;
	} platformNames[] =
	{
		{ "PC",PLATFORM_PC },
		{ "XBOX",PLATFORM_XBOX },
		{ "PS2",PLATFORM_PS2 },
		{ "GC",PLATFORM_GAMECUBE },
		{ "GameCube",PLATFORM_GAMECUBE },
	};
	for (int i = 0; i < sizeof(platformNames)/sizeof(platformNames[0]); i++)
	{
		if (stricmp(platformNames[i].name,sPlatform) == 0)
			return platformNames[i].platform;
	}
	return PLATFORM_UNKNOWN;
}



//////////////////////////////////////////////////////////////////////////
void RegisterConvertors( IResourceCompiler *rc )
{
	IRCLog *log=rc->GetIRCLog();					assert(log);

	string strDir;
	{
		char szRCPath[1000];
		if (GetModuleFileName (NULL, szRCPath, sizeof(szRCPath)))
			strDir = CryStringUtils::GetParentDirectory<string>(szRCPath) + "\\";
	}
	__finddata64_t fd;
	int hSearch = _findfirst64 ((strDir +  "ResourceCompiler*.dll").c_str(), &fd);
	if (hSearch != -1)

	do {
		HMODULE hPlugin = LoadLibrary ((strDir+fd.name).c_str());
		if (!hPlugin)
		{
			log->Log ("Error: Couldn't load plug-in module %s", fd.name);
			continue;
		}
		
		FnRegisterConvertors fnRegister = hPlugin?(FnRegisterConvertors)GetProcAddress(hPlugin, "RegisterConvertors"):NULL;
		if (!fnRegister)
		{
			log->Log ("Error: plug-in module %s doesn't have RegisterConvertors function", fd.name);
			continue;
		}

		time_t nTime = GetTimestampForLoadedLibrary (hPlugin);
		char* szTime = "unknown";
		if (nTime)
		{
			szTime = asctime(localtime(&nTime));
			szTime[strlen(szTime)-1] = '\0';
		}
//		Info ("timestamp %s", szTime);
		log->Log("");
		log->Log("  Loading \"%s\"", fd.name);

		fnRegister (rc);
	}
	while(_findnext64(hSearch, &fd) != -1);

	_findclose(hSearch);
	log->Log("");
}

void TestMatEntityNameTokenizer ()
{
	char szMatName[1024];
	while (gets (szMatName))
	{
		CMatEntityNameTokenizer mt;
		mt.tokenize(szMatName);
		printf ("Your string: \"%s\"\n", szMatName);
		printf ("name:     \"%s\"\n", mt.szName);
		printf ("template: \"%s\"\n", mt.szTemplate);
		printf ("phys mtl: \"%s\"\n", mt.szPhysMtl);
		printf ("sortValue:    %d\n\n", mt.nSortValue);
	}
}


//////////////////////////////////////////////////////////////////////////
int __cdecl main(int argc, char **argv, char **envp)
{
	/*
	int tmpDbgFlag;
	tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	// Clear the upper 16 bits and OR in the desired freqency
	tmpDbgFlag = (tmpDbgFlag & 0x0000FFFF) | (32768 << 16);
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpDbgFlag);

	// Check heap every 
	//_CrtSetBreakAlloc(2031);
	*/


	ResourceCompiler rc;
	CmdLine cmdLine;

	rc.Log("ResourceCompiler V 1.02 (Crytek)");
#ifdef _WIN64
	rc.Log("64-bit edition");
#endif

	rc.Log("================");
	rc.Log("");

//	string test = "asd" + string("asdf") + "asdf" + string("df");
//	CString test2 = "asd" + CString("asdf") + "asdf" + CString("df");

	bool bConfigFileLoaded = false;
	// Load main config.
	CfgFile cfgFile;
	if (cfgFile.Load(RC_INI_FILE))
	{
		bConfigFileLoaded = true;
		cfgFile.SetConfig( COMMON_SECTION,&rc.m_MainConfig );
	}

	// Parse command line.
	cmdLine.Parse( argc,argv,&rc.m_MainConfig );

	if (cmdLine.m_bHelp)
	{
		rc.show_help();
		exit(0);
	}

	if (cmdLine.m_fileSpec.IsEmpty())
	{
		rc.Log( "The syntax of the command is incorrect, file not specified." );
		rc.show_help();
		return 1;
	}

	CString platformStr;
	if (!rc.m_MainConfig.Get( "p",platformStr ))
	{
		// Platform switch not specified.
		rc.Log("Platform not specified, defaulting to PC.");
		rc.Log("Use /p=<Platform> switch to specify platform.");
		rc.Log("");
		platformStr = "PC";
		rc.m_MainConfig.Set("p",platformStr.GetString());
	}
	
	// Detect platform.
	Platform platform = GetPlatformFromName(platformStr.GetString());
	if (platform == PLATFORM_UNKNOWN)
	{
		rc.Log( "Unknown platform %s specified",(const char*)platformStr.GetString() );
		return 1;
	}


	rc.GetHWnd();
	//rc.InitPhysics();

	rc.Log("Registering sub compilers (ResourceCompiler*.dll)");


	RegisterConvertors( &rc );
	if (bConfigFileLoaded)
	{
		// Load configuration from per platform section. 
		cfgFile.SetConfig( rc.GetSectionName(platform),&rc.m_MainConfig );
	}
	
	rc.Compile( platform,&rc.m_MainConfig,cmdLine.m_fileSpec.GetString() );

	rc.PostBuild();		// e.g. print material dependencies

  if(rc.m_MainConfig.HasKey("wait"))
  {
		rc.Log("");		
		rc.Log("                                              <RETURN>  (/wait was specified)");			// right aligned on 80 char screen
		getchar();
	};

	return 0;
}

//! Returns the main application window
HWND ResourceCompiler::GetHWnd()
{
	HMODULE hKernel32 = LoadLibrary ("kernel32.dll");
	HWND hResult = GetDesktopWindow();
	if (hKernel32)
	{
		//typedef WINBASEAPI  HWND APIENTRY (*FnGetConsoleWindow )(VOID);
		typedef HWND (APIENTRY*FnGetConsoleWindow )(VOID);
		FnGetConsoleWindow GetConsoleWindow = (FnGetConsoleWindow)GetProcAddress (hKernel32, "GetConsoleWindow");
		if (GetConsoleWindow)
		{
			hResult = GetConsoleWindow();
		}
		FreeLibrary (hKernel32);
	}
	return hResult;
}

HWND ResourceCompiler::GetEmptyWindow()
{
	if (!m_hEmptyWindow)
	{
		const char szClassName[] = "DirectXWnd";
		WNDCLASS wc;
		memset (&wc, 0, sizeof(wc));
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = DefWindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance  = (HINSTANCE)GetModuleHandle (NULL);
		wc.lpszClassName = szClassName;

		ATOM atomWndClass = RegisterClass (&wc);
		m_hEmptyWindow = CreateWindow (szClassName, "DirectXEmpty", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,CW_USEDEFAULT,256,256,NULL, NULL, wc.hInstance, 0);
	}
	return m_hEmptyWindow;
}

// ------------------------------------------------------
void ResourceCompiler::InitPhysics()
{
	HMODULE hPhysics = LoadLibrary("CryPhysics.dll");
	if (!hPhysics)
	{
		LogError("Cannot load physics dll");
		return;
	}
	IPhysicalWorld *(*pfnCreatePhysicalWorld)(ILog *pLog) = (IPhysicalWorld*(*)(ILog*))GetProcAddress(hPhysics,"CreatePhysicalWorld");
	if(!pfnCreatePhysicalWorld)
	{
		LogError("Cannot find procedure entry CreatePhysicalWorld in CryPhysics.dll");
		FreeLibrary(hPhysics);
		return;
	}

	m_pIPhysicalWorld = pfnCreatePhysicalWorld(this);
	if (!m_pIPhysicalWorld)
	{
		LogError("Cannot create physical world");
		FreeLibrary(hPhysics);
		return;
	}

	Log ("Physical World created");
}

// ------------------------------------------------------
IPhysicalWorld* ResourceCompiler::GetPhysicalWorld()
{
	if (!m_pIPhysicalWorld)
		InitPhysics();
	return m_pIPhysicalWorld;
}


// ------------------------------------------------------
void ResourceCompiler::AddDependencyMaterial( const char *inszSrcFilename, const char *inszMatName, const char *inszScriptName )
{
	if (!m_bStatistics)
		return;
	CMatDep dep;

	dep.m_sMatName=inszMatName;
	dep.m_sScriptName=inszScriptName;

	m_MaterialDependencies.insert( CMatDepPair(dep,inszSrcFilename) );

	Log("  DepMat: <%s> <%s>",inszMatName,inszScriptName);
}


// ------------------------------------------------------
void ResourceCompiler::AddDependencyFile( const char *inszSrcFilename, const char *inszPathFileName )
{
	if (!m_bStatistics)
		return;
	m_FileDependencies.insert( CFileDepPair(inszPathFileName,inszSrcFilename) );

	Log("  DepFile: <%s>",inszPathFileName);
}


// ------------------------------------------------------
void ResourceCompiler::ShowFileDependencies()
{
	const char *szFileName=RC_FILENAME_FILEDEP;

	FILE *out=fopen(szFileName,"wb");

	if(!out)
	{
		LogError("unable to open %s - file it not updated",szFileName);
		return;
	}

	Log("writing %s (counted %d) ...",szFileName,m_FileDependencies.size());
	Log("");

	CFileDepMap::iterator it;

	string sLastDep="";		// for a nice printout
	bool bFirst=true;

	for(it=m_FileDependencies.begin(); it!=m_FileDependencies.end(); ++it)
	{
		const string &rsDepFile = it->first;
		const string &rsSrcFile = it->second;

		if(bFirst || rsDepFile!=sLastDep)
		{
			fprintf(out,"\r\n");
			fprintf(out,"'%s'\r\n",rsDepFile.c_str());
			sLastDep=rsDepFile;
			bFirst=false;
		}

		fprintf(out,"    used by: '%s'\r\n",rsSrcFile.c_str());
	}

	fprintf(out,"\r\n");
	fprintf(out,"------------------------------------------------------------------------------\r\n");
	fprintf(out,"\r\n");
	fprintf(out,"all used files:\r\n");
	fprintf(out,"\r\n");

	bFirst=true;
	for(it=m_FileDependencies.begin(); it!=m_FileDependencies.end(); ++it)
	{
		const string &rsDepFile = it->first;
		const string &rsSrcFile = it->second;

		if(bFirst || rsDepFile!=sLastDep)
		{
			fprintf(out,"  '%s'\r\n",rsDepFile.c_str());
			sLastDep=rsDepFile;
			bFirst=false;
		}
	}

	fclose(out);
}

// ------------------------------------------------------
void ResourceCompiler::ShowMaterialDependencies()
{
	const char *szFileName=RC_FILENAME_MATDEP;

	FILE *out=fopen(szFileName,"wb");

	if(!out)
	{
		LogError("unable to open %s - file it not updated",szFileName);
		return;
	}

	Log("writing %s (counted %d) ...",szFileName,m_MaterialDependencies.size());
	Log("");

	CMatDepMap::iterator it;

	CMatDep LastDep;		// for a nice printout
	bool bFirst=true;

	// max info
	for(it=m_MaterialDependencies.begin(); it!=m_MaterialDependencies.end(); ++it)
	{
		const CMatDep &rsMatDep = it->first;
		const string &rsSrcFile = it->second;

		if(bFirst || !(rsMatDep==LastDep))
		{
			fprintf(out,"\r\n");
			fprintf(out,"scriptmaterial='%s' materialname='%s'\r\n",rsMatDep.m_sScriptName.c_str(),rsMatDep.m_sMatName.c_str());
			LastDep=rsMatDep; 
			bFirst=false;
		}

		fprintf(out,"    used by: '%s'\r\n",rsSrcFile.c_str());
	}

	fprintf(out,"\r\n");
	fprintf(out,"------------------------------------------------------------------------------\r\n");
	fprintf(out,"\r\n");
	fprintf(out,"all used scriptmaterials:\r\n");
	fprintf(out,"\r\n");

	// only the used scripsmaterials
	bFirst=true;
	for(it=m_MaterialDependencies.begin(); it!=m_MaterialDependencies.end(); ++it)
	{
		const CMatDep &rsMatDep = it->first;
		const string &rsSrcFile = it->second;

		if(bFirst || !(rsMatDep.m_sScriptName==LastDep.m_sScriptName))
		{
			fprintf(out,"  '%s'\r\n",rsMatDep.m_sScriptName.c_str());
			LastDep=rsMatDep; 
			bFirst=false;
		}
	}

	fclose(out);
}


// ------------------------------------------------------
void ResourceCompiler::PostBuild()
{
	if (m_bStatistics)
	{
		ShowFileDependencies();
		ShowMaterialDependencies();
	}
}

