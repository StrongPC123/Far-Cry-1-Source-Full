	
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Log.cpp
//  Description:Log related functions
//
//	History:
//	-Feb 2,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "log.h"

//this should not be included here
#include <IConsole.h>
#include <ISystem.h>
#include <IStreamEngine.h>
#include "System.h"

#ifdef _WIN32
#include <time.h>
#endif


//#define RETURN return
#define RETURN


//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CLog::CLog( ISystem *pSystem )
{
	memset(m_szFilename,0,MAX_FILENAME_SIZE);	
	memset(m_szTemp,0,MAX_TEMP_LENGTH_SIZE);
	m_pSystem=pSystem;
	m_pLogVerbosity = 0;
	m_pLogFileVerbosity = 0;
	m_pLogIncludeTime = 0;
}

//////////////////////////////////////////////////////////////////////
CLog::~CLog()
{
	Done();
}

void CLog::Done()
{
	//# These console vars are released when script system shutdown.
	m_pLogVerbosity = 0;
	m_pLogFileVerbosity = 0;
	m_pLogWarningsOnly = 0;
	m_pLogColoredText = 0;
	m_pLogIncludeTime = 0;
}

//////////////////////////////////////////////////////////////////////////
void CLog::EnableVerbosity( bool bEnable )
{
	RETURN;

	if (bEnable)
	{
		if (!m_pLogVerbosity)
		{
			if (m_pSystem->GetIConsole())
			{
#if defined(DEBUG) || (defined(LINUX) && !defined(NDEBUG))
				m_pLogVerbosity = m_pSystem->GetIConsole()->CreateVariable("log_Verbosity","5",VF_DUMPTODISK);
#else
				m_pLogVerbosity = m_pSystem->GetIConsole()->CreateVariable("log_Verbosity","3",VF_DUMPTODISK);
#endif
				m_pLogFileVerbosity = m_pSystem->GetIConsole()->CreateVariable("log_FileVerbosity","3",VF_DUMPTODISK);
			}
		}
	}
	else
	{
		m_pLogIncludeTime = 0; // otherwise may not work in debug mode.
		if (m_pSystem->GetIConsole())
		{
			m_pSystem->GetIConsole()->UnregisterVariable("log_Verbosity",true);
			m_pSystem->GetIConsole()->UnregisterVariable("log_FileVerbosity",true);
		}
		m_pLogVerbosity = 0;
		m_pLogFileVerbosity = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CLog::SetVerbosity( int verbosity )
{
	RETURN;
	EnableVerbosity(true);
	if (m_pLogVerbosity)
		m_pLogVerbosity->Set(verbosity);
}

//////////////////////////////////////////////////////////////////////////
void CLog::LogWarning(const char *szFormat,...)
{
	va_list	ArgList;
	char		szBuffer[MAX_WARNING_LENGTH];
	va_start(ArgList, szFormat);
	vsprintf(szBuffer, szFormat, ArgList);
	va_end(ArgList);

	m_pSystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,NULL,"%s",szBuffer );
}

//////////////////////////////////////////////////////////////////////////
void CLog::LogError(const char *szFormat,...)
{
	va_list	ArgList;
	char		szBuffer[MAX_WARNING_LENGTH];
	va_start(ArgList, szFormat);
	vsprintf(szBuffer, szFormat, ArgList);
	va_end(ArgList);

	m_pSystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_ERROR,0,NULL,"%s",szBuffer );
}

//////////////////////////////////////////////////////////////////////////
void CLog::Log(const char *szFormat,...)
{
	if (m_pLogVerbosity && !m_pLogVerbosity->GetIVal())
	{
		if (m_pLogFileVerbosity && !m_pLogFileVerbosity->GetIVal())
		{
			return;
		}
	}

	RETURN;
	va_list arg;
	va_start(arg, szFormat);
	LogV (eMessage, szFormat, arg);
	va_end(arg);
}

//will log the text both to file and console
//////////////////////////////////////////////////////////////////////
void CLog::LogV( const ELogType type, const char* szFormat, va_list args )
{
	RETURN;
	if (!szFormat)
		return;

	bool bfile = false, bconsole = false;
	const char* szCommand = szFormat;
	if (type != eAlways && type != eWarningAlways && type != eErrorAlways  && type != eInput)
	{
		szCommand = CheckAgainstVerbosity(szFormat, bfile, bconsole);
		if (!bfile && !bconsole)
			return;
	}
	else
	{
		bfile = true;
		if (type == eInput)
		{
			const unsigned char nMaxVerbosity = 8;
			int nLogFileVerbosity = m_pLogFileVerbosity ? m_pLogFileVerbosity->GetIVal() : nMaxVerbosity;
			if (nLogFileVerbosity == 0)
				bfile = 0;
		}
		bconsole = true; // console always true.
	}
	
	char szBuffer[MAX_WARNING_LENGTH+32];
	char *szString = szBuffer;

	switch(type)
	{
	case eWarningAlways:
		strcpy( szString,"$6" ); // yellow color.
		szString += 2;
		break;
	case eErrorAlways:
		strcpy( szString,"$4" ); // red color.
		szString += 2;
		break;
	case eWarning:
		strcpy( szString,"[WARNING]" );
		szString += strlen("[WARNING]");
		break;

	case eError:
		strcpy( szString,"[ERROR]" );
		szString += strlen("[ERROR]");
		break;
	}
	
	_vsnprintf( szString, sizeof(szBuffer)-32, szCommand, args );
	szBuffer[sizeof(szBuffer)-8]=0;

	if (bfile)
		LogStringToFile( szString );
	if (bconsole)
		LogStringToConsole( szString );	

  // in case of error - update screen and make a sound to wake up artists (in testing)
  /*if(strstr(szCommand,"rror"))
  { 
		IConsole *console=m_pSystem->GetIConsole();
		IRenderer *renderer=m_pSystem->GetIRenderer();

		if(console)
		if(renderer)
		{
			console->Update();
			renderer->BeginFrame();		
			console->Draw();	
			renderer->Update();
		}


    ///MessageBeep(MB_ICONHAND);

////    Sleep(500);
  }	*/
}

//will log the text both to the end of file and console
//////////////////////////////////////////////////////////////////////
void CLog::LogPlus(const char *szFormat,...)
{
	if (m_pLogVerbosity && !m_pLogVerbosity->GetIVal())
	{
		if (m_pLogFileVerbosity && !m_pLogFileVerbosity->GetIVal())
		{
			return;
		}
	}

	RETURN;
	if (!szFormat)
		return;

	bool bfile = false, bconsole = false;
	const char* szCommand = CheckAgainstVerbosity(szFormat, bfile, bconsole);
	if (!bfile && !bconsole)
		return;

	va_list		arglist;	

	va_start(arglist, szFormat);
	_vsnprintf(m_szTemp, sizeof(m_szTemp), szCommand, arglist);
	m_szTemp[sizeof(m_szTemp)-8]=0;
	va_end(arglist);	

	if (bfile)
		LogToFilePlus(m_szTemp);		
	if (bconsole)
		LogToConsolePlus(m_szTemp);	
}

//log to console only
//////////////////////////////////////////////////////////////////////
void CLog::LogStringToConsole( const char *szString,bool bAdd )
{
	if (!szString || !szString[0])
		return;

	if (!m_pSystem)
		return;
	IConsole *console = m_pSystem->GetIConsole();
	if (!console)
		return;

	char szTemp[MAX_WARNING_LENGTH];
	strncpy( szTemp,szString,sizeof(szTemp)-32 ); // leave space for additional text data.
	szTemp[sizeof(szTemp)-32] = 0;

	size_t len = strlen(szTemp);
	const char * mptr=szTemp+len-1;
	if (*mptr!='\n') 
		strcat(szTemp,"\n");

	size_t nLen = strlen(szTemp);

	assert(nLen<sizeof(szTemp));

	//check if Tony wanna output only wanrning or error messages
	if (strstr(szTemp,"rror") || strstr(szTemp,"ERROR"))
	{ // make error message red
		memmove(szTemp+2,szTemp,nLen+1);
		nLen+=2;
		szTemp[0] = '$';
		szTemp[1] = '4';
	}
	else 
	{
		if (strstr(szTemp,"arning") || strstr(szTemp,"WARNING"))
		{ // make error message blue
			memmove(szTemp+2,szTemp,nLen+1);
			nLen+=2;
			szTemp[0] = '$';
			szTemp[1] = '6';
		}
	}

	if (bAdd)
		console->PrintLinePlus(szTemp);	
	else
		console->PrintLine(szTemp);
}

//log to console only
//////////////////////////////////////////////////////////////////////
void CLog::LogToConsole(const char *szFormat,...)
{
	if (m_pLogVerbosity && !m_pLogVerbosity->GetIVal())
	{
		if (m_pLogFileVerbosity && !m_pLogFileVerbosity->GetIVal())
		{
			return;
		}
	}

	RETURN;
	if (!szFormat)
		return;

	bool bfile = false, bconsole = false;
	const char* szCommand = CheckAgainstVerbosity(szFormat, bfile, bconsole);
	if (!bconsole)
		return;

	va_list		arglist;	

	char szBuffer[MAX_WARNING_LENGTH];
	va_start(arglist, szFormat);
	_vsnprintf(szBuffer, sizeof(szBuffer), szCommand, arglist);
	szBuffer[sizeof(szBuffer)-8]=0;
	va_end(arglist);

	LogStringToConsole( szBuffer );
}

//////////////////////////////////////////////////////////////////////
void CLog::LogToConsolePlus(const char *szFormat,...)
{
	if (m_pLogVerbosity && !m_pLogVerbosity->GetIVal())
	{
		if (m_pLogFileVerbosity && !m_pLogFileVerbosity->GetIVal())
		{
			return;
		}
	}

	RETURN;
	if (!szFormat)
		return;

	bool bfile = false, bconsole = false;
	const char* szCommand = CheckAgainstVerbosity(szFormat, bfile, bconsole);
	if (!bconsole)
		return;

	va_list		arglist;
	
	va_start(arglist, szFormat);
	_vsnprintf(m_szTemp, sizeof(m_szTemp), szCommand, arglist);
	m_szTemp[sizeof(m_szTemp)-8]=0;
	va_end(arglist);	

	if (!m_pSystem)
		return;

	LogStringToConsole( m_szTemp,true );
}


//////////////////////////////////////////////////////////////////////
void CLog::LogStringToFile( const char *szString,bool bAdd )
{
	if (!szString || !szString[0])
		return;

	if (!m_pSystem)
		return;
	IConsole * console = m_pSystem->GetIConsole();

	char szTemp[MAX_TEMP_LENGTH_SIZE];
	strncpy( szTemp,szString,sizeof(szTemp)-32 ); // leave space for additional text data.
	szTemp[sizeof(szTemp)-32] = 0;

	size_t len = strlen(szTemp);
	const char * mptr=szTemp+len-1;
	if (*mptr!='\n') 
		strcat(szTemp,"\n");

	if (szTemp[0] == '$')
		strcpy(szTemp, szTemp+2);

#ifdef _WIN32
	if (!m_pLogIncludeTime)
	{
		// put time into begin of the string if requested by cvar
		m_pLogIncludeTime = (console && m_pSystem->GetIScriptSystem()) ? console->CreateVariable("log_IncludeTime", "0", 0,
			"Toggles timestamping of log entries.\n"
			"Usage: log_IncludeTime [0/1]\n"
			"Default is 0 (off). Set to 1 to include time in log items.") : 0;
	}
	if (m_pLogIncludeTime && m_pLogIncludeTime->GetIVal())
	{
		memmove( szTemp+8, szTemp, strlen(szTemp)+1 );
		time_t ltime;
		time( &ltime );
		struct tm *today = localtime( &ltime );
		memset(szTemp, ' ', 8);
		strftime( szTemp, 8, "<%M:%S> ", today );
	}
#endif

	if (bAdd)
	{
		FILE *fp=fxopen(m_szFilename,"r+t");
		if (fp)
		{
			int p1 = ftell(fp);
			fseek(fp,0,SEEK_END);
			p1 = ftell(fp);
			fseek(fp,-2,SEEK_CUR);
			p1 = ftell(fp);

			fputs(szTemp,fp);		
			fclose(fp);
		}
	}
	else
	{
		if(FILE * fp = fxopen(m_szFilename,"at"))
		{
			fputs(szTemp,fp);
			fclose(fp);
		}  
	}
}

//same as above but to a file
//////////////////////////////////////////////////////////////////////
void CLog::LogToFilePlus(const char *szFormat,...)
{
	if (m_pLogVerbosity && !m_pLogVerbosity->GetIVal())
	{
		if (m_pLogFileVerbosity && !m_pLogFileVerbosity->GetIVal())
		{
			return;
		}
	}

	RETURN;
	if (!m_szFilename[0] || !szFormat) 
		return;

	bool bfile = false, bconsole = false;
	const char* szCommand = CheckAgainstVerbosity(szFormat, bfile, bconsole);
	if (!bfile)
		return;

	va_list		arglist;	
	va_start(arglist, szFormat);
	_vsnprintf(m_szTemp, sizeof(m_szTemp), szCommand, arglist);
	m_szTemp[sizeof(m_szTemp)-8]=0;
	va_end(arglist);	

	LogStringToFile( m_szTemp,true );
}

//log to the file specified in setfilename
//////////////////////////////////////////////////////////////////////
void CLog::LogToFile(const char *szFormat,...)
{
	if (m_pLogVerbosity && !m_pLogVerbosity->GetIVal())
	{
		if (m_pLogFileVerbosity && !m_pLogFileVerbosity->GetIVal())
		{
			return;
		}
	}

	RETURN;
	if (!m_szFilename[0] || !szFormat) 
		return;	 

	bool bfile = false, bconsole = false;
	const char* szCommand = CheckAgainstVerbosity(szFormat, bfile, bconsole);
	if (!bfile)
		return;

	va_list		arglist;  	
	va_start(arglist, szFormat);
	_vsnprintf(m_szTemp, sizeof(m_szTemp), szCommand, arglist);
	m_szTemp[sizeof(m_szTemp)-16]=0;
	va_end(arglist);	

	LogStringToFile( m_szTemp );
}

//set the file used to log to disk
//////////////////////////////////////////////////////////////////////
void CLog::SetFileName(const char *command)
{
	RETURN;
	if (!command) 
    return;

	strcpy(m_szFilename,command); 

#ifndef _XBOX
		FILE *fp=fxopen(m_szFilename,"wt");
    if (fp)
		  fclose(fp);
#endif
}

//////////////////////////////////////////////////////////////////////////
const char* CLog::GetFileName()
{
	return m_szFilename;
}

//////////////////////////////////////////////////////////////////////
void CLog::UpdateLoadingScreen(const char *szFormat,...)
{
	if ((!m_pLogVerbosity) || (m_pLogVerbosity && m_pLogVerbosity->GetIVal()) || ((!m_pLogFileVerbosity) || (m_pLogFileVerbosity && m_pLogFileVerbosity->GetIVal())))
	{
		RETURN;
		if (szFormat) 
		{
			bool bfile = false, bconsole = false;
			CheckAgainstVerbosity(szFormat, bfile, bconsole);

			if (bconsole || bfile)
			{
				va_list args;
				va_start(args, szFormat);
				_vsnprintf(m_szTemp, sizeof(m_szTemp), szFormat, args);
				m_szTemp[sizeof(m_szTemp)-8]=0;
				va_end(args);

				if (bconsole)
					LogToConsole(m_szTemp);
				if (bfile)
					LogToFile(m_szTemp);
				if (bconsole)
				{
					((CSystem*)m_pSystem)->UpdateLoadingScreen();
				}
			}
		}
	}
	// Take this oportunity to update streaming engine.
	GetISystem()->GetStreamEngine()->Update();
}

//////////////////////////////////////////////////////////////////////
void CLog::UpdateLoadingScreenPlus(const char *szFormat,...)
{
	if ((!m_pLogVerbosity) || (m_pLogVerbosity && m_pLogVerbosity->GetIVal()) || ((!m_pLogFileVerbosity) || (m_pLogFileVerbosity && m_pLogFileVerbosity->GetIVal())))
	{
		RETURN; 
		if (szFormat) 
		{
			bool bfile = false, bconsole = false;
			CheckAgainstVerbosity(szFormat, bfile, bconsole);

			va_list args;
			va_start(args, szFormat);
			_vsnprintf(m_szTemp, sizeof(m_szTemp), szFormat, args);
			m_szTemp[sizeof(m_szTemp)-8]=0;
			va_end(args);

			if (bconsole)
				LogToConsolePlus(m_szTemp);
			if (bfile)
				LogToFilePlus(m_szTemp);
			
			if (bconsole)
			{
				((CSystem*)m_pSystem)->UpdateLoadingScreen();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int	CLog::GetVerbosityLevel()
{
	if (m_pLogVerbosity)
		return (m_pLogVerbosity->GetIVal());

	return (0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Checks the verbosity of the message and returns NULL if the message must NOT be
// logged, or the pointer to the part of the message that should be logged
// NOTE:
//    Normally, this is either the pText pointer itself, or the pText+1, meaning
//    the first verbosity character may be cut off)
//    This is done in order to avoid modification of const char*, which may cause GPF
//    sometimes, or kill the verbosity qualifier in the text that's gonna be passed next time.
const char* CLog::CheckAgainstVerbosity(const char * pText, bool &logtofile, bool &logtoconsole)
{
	RETURN;
	// the max verbosity (most detailed level)
	const unsigned char nMaxVerbosity = 8;
	
	// the current verbosity of the log
	int nLogVerbosity = m_pLogVerbosity ? m_pLogVerbosity->GetIVal() : nMaxVerbosity;
	int nLogFileVerbosity = m_pLogFileVerbosity ? m_pLogFileVerbosity->GetIVal() : nMaxVerbosity;

	nLogFileVerbosity=max(nLogFileVerbosity,nLogVerbosity);		// file verbosity depends on usual log_verbosity as well

	const char *pStartText;
	int textVerbosity;

	// Empty string
	if (pText[0] == '\0')
	{
		logtoconsole = false;
		logtofile = false;
		return 0;
	}

	if((unsigned char)pText[0]>=' ')
	{
		// verbosity is not defined in the text
		pStartText=pText;
		textVerbosity=nMaxVerbosity;
	}
	else
	{
		// verbosity is defined in the text
		pStartText=pText+1;
		textVerbosity=(unsigned char)pText[0];
	}

	logtoconsole = (nLogVerbosity >= textVerbosity);
	logtofile = (nLogFileVerbosity >= textVerbosity);

	return pStartText;
}

/*
//////////////////////////////////////////////////////////////////////////
int CLog::CheckVerbosity( const char * pText )
{	
	RETURN;
	// the max verbosity (most detailed level)
	const unsigned char nMaxVerbosity = 5;

	// set message verbosity for error and warning messages	
	char sBuff[256]; 
	strncpy(sBuff,pText,255);			
	sBuff[255]=0; 
	strlwr(sBuff);
	if(strstr(pText,"error"))
		return 1;
	if(strstr(pText,"warning"))
		return 3;	
	
	int textVerbosity = (unsigned char)pText[0];
	if (textVerbosity > nMaxVerbosity) 
	{
		return nMaxVerbosity;
	}
	else
		return textVerbosity;	
}
*/