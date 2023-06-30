// LogFile.h: interface for the CLogFile class.
//
//////////////////////////////////////////////////////////////////////
 
#if !defined(AFX_LOGFILE_H__9809D818_0A64_4187_A2EB_C3878F61C229__INCLUDED_)
#define AFX_LOGFILE_H__9809D818_0A64_4187_A2EB_C3878F61C229__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ILog.h"
#include <IConsole.h>

//struct IConsole;
//struct ICVar;

//////////////////////////////////////////////////////////////////////////
// Global log functions.
//////////////////////////////////////////////////////////////////////////
//! Displays error message.
extern void Error( const char *format,... );
//! Log to console and file.
extern void Log( const char *format,... );
//! Display Warning dialog.
extern void Warning( const char *format,... );

/*!
 *	CLogFile implements ILog interface.
 */
class CLogFile : public ILog
{
public:
	static void FormatLine(PSTR pszMessage, ...);
	CLogFile();
	virtual ~CLogFile();

	void Release() { delete this; };

	// interface ILog -------------------------------------------------

	static void WriteLogHeader();
	static PSTR GetLogFileName();
	static void AttachListBox(HWND hWndListBox) { m_hWndListBox = hWndListBox; };
	static void AttachEditBox(HWND hWndEditBox) { m_hWndEditBox = hWndEditBox; };

	static void SetConsole( IConsole *c ) { m_console = c;  if (c) {m_pLogVerbosity = c->CreateVariable("log_Verbosity","5",VF_DUMPTODISK);}; };

	//! Write to log spanpshot of current process memory usage.
	static CString GetMemUsage();

	static void WriteString(const char * pszString);
	static void WriteLine(const char * pszLine);

	//////////////////////////////////////////////////////////////////////////
	// Implement ILog interface.
	//////////////////////////////////////////////////////////////////////////
//	virtual void	Log(int dwFlags,const char *command,...);

	virtual void	LogWarning(const char *command,...);
	virtual void	LogError(const char *command,...);
	virtual void	Log(const char *command,...);	
	virtual void	LogPlus(const char *command,...);
  virtual void	LogToFile(const char *command,...);
  virtual void	LogToFilePlus(const char *command,...);
	virtual void	LogToConsole(const char *command,...);
	virtual void	LogToConsolePlus(const char *command,...);
///	virtual void	Exit(const char *command,...);
  virtual void	UpdateLoadingScreen(const char *command,...);
  virtual void	UpdateLoadingScreenPlus(const char *command,...);
	virtual void	SetFileName(const char *command = NULL) {};
	const char*	GetFileName();

	virtual void	EnableVerbosity( bool bEnable );
	virtual void	SetVerbosity( int verbosity );
	virtual int GetVerbosityLevel();

	// checks the verbosity of the message and returns NULL if the message must NOT be
	// logged, or the pointer to the part of the message that should be logged
	static const char* CheckAgainstVerbosity(const char * pText,bool &bOnlyFile );
private:

	virtual void LogV( const IMiniLog::ELogType ineType, const char* szFormat, va_list args );

	static void LogString(const char * pszString,bool onlyFile=false);
	static void LogLine(const char * pszLine,bool onlyFile=false);
	static int CheckVerbosity( const char * pText );
	static void AboutSystem();
	static unsigned int GetCPUSpeed();
	static void GetCPUModel(char szType[]);

	static void OpenFile();

	// Attached control(s)
	static HWND m_hWndListBox;
	static HWND m_hWndEditBox;
	static IConsole* m_console;
	static bool m_bFileOpened;
	static bool m_bShowMemUsage;
	static ICVar	*m_pLogVerbosity;
	static char m_szLogFilename[_MAX_PATH];
};

#endif // !defined(AFX_LOGFILE_H__9809D818_0A64_4187_A2EB_C3878F61C229__INCLUDED_)
