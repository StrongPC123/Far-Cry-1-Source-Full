
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Log.h
//
//	History:
//	-Feb 2,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef LOG_H
#define LOG_H

#if _MSC_VER > 1000
# pragma once
#endif

#include <ILog.h>

//////////////////////////////////////////////////////////////////////
#define MAX_TEMP_LENGTH_SIZE	2048
#define MAX_FILENAME_SIZE			256
 



//////////////////////////////////////////////////////////////////////
class CLog:
public ILog
{
public:

	CLog( ISystem *pSystem );
	~CLog();

	// interface ILog -------------------------------------------------

	virtual void Release() { delete this; };
	virtual void SetFileName(const char *command);		
	virtual const char*	GetFileName();
//	void	Log(int dwFlags,const char *szCommand,...){}
	virtual void Log(const char *command,...);
	virtual void LogWarning(const char *command,...);
	virtual void LogError(const char *command,...);
	virtual void LogPlus(const char *command,...);
	virtual void LogToFile	(const char *command,...);
  virtual void LogToFilePlus(const char *command,...);
	virtual void LogToConsole(const char *command,...);
	virtual void LogToConsolePlus(const char *command,...);
	virtual void UpdateLoadingScreen(const char *command,...);
  virtual void UpdateLoadingScreenPlus(const char *command,...);
	virtual void EnableVerbosity( bool bEnable );
	virtual void SetVerbosity( int verbosity );
	virtual	int	 GetVerbosityLevel();

private:
	
	virtual void LogV( const ELogType ineType, const char* szFormat, va_list args );
	void LogStringToFile( const char* szString,bool bAdd=false );
	void LogStringToConsole( const char* szString,bool bAdd=false );
	void Done();

	//will format the message into m_szTemp
	void	FormatMessage(const char *szCommand,...);

	ISystem	*m_pSystem;
	char	m_szTemp[MAX_TEMP_LENGTH_SIZE];
	char	m_szFilename[MAX_FILENAME_SIZE];
		
	ICVar			*m_pLogWarningsOnly;
	ICVar			*m_pLogIncludeTime;
	
	ICVar			*m_pLogColoredText;	
	IConsole	*m_pConsole;	
public:
	// checks the verbosity of the message and returns NULL if the message must NOT be
	// logged, or the pointer to the part of the message that should be logged
	const char* CheckAgainstVerbosity(const char * pText, bool &logtofile, bool &logtoconsole);
	//! Returns verbosity level of this string.
	//int CheckVerbosity( const char * pText );

	ICVar			*m_pLogVerbosity;
	ICVar			*m_pLogFileVerbosity;
};


#endif