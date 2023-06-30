////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   ILog.h
//  Version:     v1.00
//  Created:     24.1.2002 by Sergiy
//  Compilers:   Visual Studio.NET
//  Description: ILog interface.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _RESOURCE_COMPILER_IRCLOG_HDR_
#define _RESOURCE_COMPILER_IRCLOG_HDR_

#include <stdarg.h>
#include "..\CryCommon\ILog.h"		// FIXME: won't compile otherwise?

// This interface is used to log events inside the convertors,
// including information and warning messages
struct IRCLog: public ILog
{

	// interface IRCLog ---------------------------------------------

	//! is used by LogV, you only have to implement this
	virtual void LogLine ( const ELogType ineType, const char* szText) = 0;

	// interface ILog -----------------------------------------------

	// use only these three methods (\n is auto splitting in lines)

	// 
	virtual void Log( const char* szFormat, ...);

	// 
	virtual void LogWarning( const char* szFormat, ...);

	// 
	virtual void LogError( const char* szFormat, ...);
	
	// --------------------------------------------------------------

	//! split in lines (empty lines if given) and print with Log function
	virtual void LogV( const ELogType ineType, const char* szFormat, va_list args )
	{
		char str[16*1024],*p=str;

		vsprintf(str,szFormat, args);

		bool bRun=true;

		while(bRun)
		{
			char *start=p;

			// search for end marker
			while(*p!=0 && *p!=10)
			{
				if(*p<32)*p=' ';  // remove nonprintable characters

				p++;
			}

			if(*p==0)
				bRun=false;

			*p=0;

			LogLine(ineType,start);

			p++;	// jump over end marker
		}
	}



	/*
	void Info (const char* szFormat, ...);
	virtual void Info (const char* szFormat, va_list args) {Log(szFormat, args);}
	void Warning (const char* szFormat, ...);
	virtual void Warning (const char* szFormat, va_list args) {Log(szFormat, args);}
	void Error (const char* szFormat, ...);
	virtual void Error (const char* szFormat, va_list args) {Log(szFormat, args);}
	*/
	void ThrowError(const char* szFormat, ...); // print message and exit CStatCFGCompiler::Process function, todo: Use same system for all converters
	void LogPlus(const char* szFormat, ...); // print message on the prev line (maybe not needed)

	virtual void Release() {}
	/* 
	void	Log(int dwFlags,const char *szCommand,...)
	{
		va_list arg;
		va_start(arg, szCommand);
		LogV (eMessage, szCommand, arg);
		va_end(arg);
	}
	*/

	//set the file used to log to disk
	virtual void SetFileName (const char *command = NULL) {}
	virtual const char*	GetFileName() {return "stdout";}
	virtual void	LogToFile(const char *szCommand,...)
	{
		va_list arg;
		va_start(arg, szCommand);
		LogV (eMessage, szCommand, arg);
		va_end(arg);
	}

	virtual void	LogToFilePlus(const char *szCommand,...)
	{
		va_list arg;
		va_start(arg, szCommand);
		LogV (eMessage, szCommand, arg);
		va_end(arg);
	}

	//log to console only
	virtual void	LogToConsole(const char *szCommand,...)
	{
		va_list arg;
		va_start(arg, szCommand);
		LogV (eMessage, szCommand, arg);
		va_end(arg);
	}

	//
	virtual void	LogToConsolePlus(const char *szCommand,...) {}

	//
	virtual void	UpdateLoadingScreen(const char *szCommand,...) {}

	//
	virtual void	UpdateLoadingScreenPlus(const char *szCommand,...) {}

	//
	virtual void	EnableVerbosity( bool bEnable ) {}

	//
	virtual void	SetVerbosity( int verbosity ) {}
	virtual int GetVerbosityLevel () {return 5;}
};

inline void IRCLog::Log(const char* szFormat, ...)
{
	va_list arg;
	va_start(arg, szFormat);
	LogV (eMessage, szFormat, arg);
	va_end(arg);
}


inline void IRCLog::LogWarning(const char* szFormat, ...)
{
	va_list arg;
	va_start(arg, szFormat);
	LogV (eWarning, szFormat, arg);
	va_end(arg);
}


inline void IRCLog::LogError(const char* szFormat, ...)
{
	va_list arg;
	va_start(arg, szFormat);
	LogV (eError, szFormat, arg);
	va_end(arg);
}


inline void IRCLog::LogPlus(const char* szFormat, ...)
{
	va_list arg;
	va_start(arg, szFormat);
	LogV (eMessage, szFormat, arg);
	va_end(arg);
}

inline void IRCLog::ThrowError(const char* szFormat, ...)
{
	va_list arg;
	va_start(arg, szFormat);
	LogV (eError, szFormat, arg);
	va_end(arg);
//  Beep(1000,1000);
//	exit(0);
	throw "ThrowError";
}

#endif // _RESOURCE_COMPILER_IRCLOG_HDR_