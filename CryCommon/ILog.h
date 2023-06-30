
#ifndef _ILOG_H_
#define _ILOG_H_

#include "IMiniLog.h"
#if defined(LINUX)
	#include "platform.h"
#endif

//////////////////////////////////////////////////////////////////////
#define LOG_TO_FILE					(1L<<8)
#define LOG_TO_CONSOLE			(2L<<8)
#define LOG_TO_FILE_PLUS		(4L<<8)	//
#define LOG_TO_CONSOLE_PLUS	(8L<<8)	//
#define LOG_TO_SCREEN				(16L<<8)
#define LOG_TO_SCREEN_PLUS	(32L<<8) //

#define LOG_TO_FILE_AND_CONSOLE	(LOG_TO_FILE | LOG_TO_CONSOLE)


//verbosity levels
//////////////////////////////////////////////////////////////////////
// log verbosity levels are from 1 to 5 (default is 5, 1 for messages with "ERROR", 3 for messages with "WARNING")
// global verbosity levels are from 0 to 5

// LEVEL 1 - Critical messages. Only events that are critical for the proper operation of the game need to be logged at this level. This includes features - event that are critical for a feature to work MUST be logged at this level.
// LEVEL 2 - Artifact filter - This level is for events that will not affect the basic operation of the game, but might introduce artifacts (visual or in the way the features work).
// LEVEL 3 - Startup / Shutdown logs - Basically information when systems start and when they shutdown as well as some diagnostic information if you want.
// LEVEL 4 - Events that are used for debugging purposes (like entity spawned there and there, which sector of the building we are loading etc)
// LEVEL 5 - Everything else - including stuff like (x=40 y=30 z=30 five times a frame :) 

// This means that clean verbosity 2 should guarantee a top notch run of the game.

// the console variable called log_Verbosity defines the verbosity
// level.
// With this option, the level of verbosity of the output is controlled.
// This option can be given multiple times to set the verbosity level to that value. 
// The default global verbosity level is 0, in which no log messages will be displayed.
//
// Usage:
//
// e.g. System:Log("\001 This is log verbosity 1") for error
// e.g. System:Log("\002 This is log verbosity 2")
// e.g. System:Log("\003 This is log verbosity 3") for warning
// e.g. System:Log("\004 This is log verbosity 4")
// e.g. System:Log("\005 This is log verbosity 5")
//
// e.g. System:Log("\002 This is log verbosity 2")      
//      with global_verbosity_level 1 this is not printed but with global_verbosity_level 2 or higher it is

//////////////////////////////////////////////////////////////////////
struct ILog: public IMiniLog
{
	virtual void Release() = 0;

	//@param dwFlags		must be a combination of priority and flags, i.e. (12 | LOG_TO_FILE)
	//@param szCommand	string to ouput to the log
//	virtual void	Log(int dwFlags,const char *szCommand,...)=0;

	//set the file used to log to disk
	virtual void	SetFileName(const char *command = NULL) = 0;

	//
	virtual const char*	GetFileName() = 0;

	//all the following functions will be removed are here just to be able to compile the project ---------------------------

	//will log the text both to file and console
	virtual void	Log(const char *szCommand,...)=0;

	virtual void	LogWarning(const char *szCommand,...)=0;

	virtual void	LogError(const char *szCommand,...)=0;

	//will log the text both to the end of file and console
	virtual void	LogPlus(const char *command,...) = 0;	

	//log to the file specified in setfilename
  virtual void	LogToFile(const char *command,...) = 0;	

	//
	virtual void	LogToFilePlus(const char *command,...) = 0;

	//log to console only
	virtual void	LogToConsole(const char *command,...) = 0;

	//
	virtual void	LogToConsolePlus(const char *command,...) = 0;

	//
	virtual void	UpdateLoadingScreen(const char *command,...) = 0;	

	//
 	virtual void	UpdateLoadingScreenPlus(const char *command,...) = 0;

	//
	virtual void	EnableVerbosity( bool bEnable ) = 0;

	//
	virtual void	SetVerbosity( int verbosity ) = 0;

	virtual int		GetVerbosityLevel()=0;
};


//////////////////////////////////////////////////////////////////////
//#include <stdio.h>
//#include <stdlib.h>

#ifdef PS2
#include "File.h"
#endif

#ifdef _XBOX
inline void _ConvertNameForXBox(char *dst, const char *src)
{
  //! On XBox d:\ represents current working directory (C:\MasterCD)
  //! only back slash (\) can be used
  strcpy(dst, "d:\\");
  if (src[0]=='.' && (src[1]=='\\' || src[1]=='/'))
    strcat(dst, &src[2]);
  else
    strcat(dst, src);
  int len = strlen(dst);
  for (int n=0; dst[n]; n++)
  {
    if ( dst[n] == '/' )
      dst[n] = '\\';
    if (n > 8 && n+3 < len && dst[n] == '\\' && dst[n+1] == '.' && dst[n+2] == '.')
    {
      int m = n+3;
      n--;
      while (dst[n] != '\\')
      {
        n--;
        if (!n)
          break;
      }
      if (n)
      {
        memmove(&dst[n], &dst[m], len-m+1);
        len -= m-n;
        n--;
      }
    }
  }
}
#endif

//! Everybody should use fxopen instead of fopen
//! so it will work both on PC and XBox
inline FILE * fxopen(const char *file, const char *mode)
{
  //SetFileAttributes(file,FILE_ATTRIBUTE_ARCHIVE);
//	FILE *pFile = fopen("C:/MasterCD/usedfiles.txt","a");
//	if (pFile)
//	{
//		fprintf(pFile,"%s\n",file);
//		fclose(pFile);
//	}

#ifdef _XBOX
  char name[256];
  _ConvertNameForXBox(name, file);
  return fopen(name, mode);
#else
#if defined(LINUX)
	return fopen_nocase(file, mode);
#else
  return fopen(file, mode);
#endif //LINUX
#endif
}

#endif //_ILOG_H_
