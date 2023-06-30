// Copyright (C) 2001-2003 Even Balance, Inc.
//
//
// pbsv.h
//
// EVEN BALANCE - T.RAY
//
// The only file in the game project that needs to include this file is pbsv.cpp
//



#ifndef __PBSV_H__
#define __PBSV_H__



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pbcommon.h"

#if !defined(NOT_USE_PUNKBUSTER_SDK)

#ifdef __PBWIN32__
#include <windows.h>
#endif
#ifdef __PBLINUX__
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#ifdef __PBMAC__
#include "../macosx/dlfcn.h"
#endif



#ifdef __WITH_PB__



#ifdef _cplusplus



#if !defined(_WIN32) && !defined(__TRMACH__)
#define _cdecl
#define stricmp strcasecmp
#define strnicmp strncasecmp
#ifndef __TRLTOA__
#define __TRLTOA__
//
// ltoa
//
// assumes buffer a is large enough to hold ascii representation of i
//
inline char *ltoa ( int i , char *a , int radix )
{
	if ( a == NULL ) return NULL ;
	strcpy ( a , "0" ) ;
	if ( i && radix > 1 && radix < 37 ) {
		char buf[35] ;
		unsigned int u = i , p = 34 ;
		buf[p] = 0 ;
		if ( i < 0 && radix == 10 ) u = -i ;
		while ( u ) {
			unsigned int d = u % radix ;
			buf[--p] = d < 10 ? '0' + d : 'a' + d - 10 ;
			u /= radix ;
		}
		if ( i < 0 && radix == 10 ) buf[--p] = '-' ;
		strcpy ( a , buf + p ) ;
	}
	return a ;
}
#define itoa ltoa
#endif
#endif //#ifndef _WIN32



#ifndef __PBDLL__
extern char *sa ( void *cp , int ticklimit ) ;
extern char *sb ( void *cp , int event , int clientIndex , int datalen , char *data , int retry ) ;
#endif

#define PB_MAX_CLIENTS 64

//
// Class/Struct Definitions
//
struct stPbSvClSlot {
	stPb_Sv_Client pbc ;
} ;

struct stPbSv {
	unsigned int m_svId ;
	void *m_Md5 ;
	void *m_SvInstance , *m_ClInstance , *m_AgInstance ;
	char m_msgPrefix[PB_MISCLEN+1] , m_basepath[PB_Q_MAXRESULTLEN+4] , m_homepath[PB_Q_MAXRESULTLEN+4] , m_cwd[PB_Q_MAXRESULTLEN+4] ;
	int m_ReloadServer ;

	//func ptrs
	tdPbGameCommand m_GameCommand ;//game
	tdPbGameQuery m_GameQuery ;//game
	tdPbGameMsg m_GameMsg ;//game
	tdPbSendToClient m_SendToClient ;//game
	tdPbAddSvEvent m_AddPbEvent ;//dll
	tdPbProcessPbEvents m_ProcessPbEvents ;//dll
	tdPbSendToAddrPort m_SendToAddrPort ;//game
	tdPbPassConnectString m_PassConnectString ;//dll
	tdPbAuthClient m_AuthClient ;//dll
	tdPbTrapConsole m_TrapConsole ;//dll

	void *m_Agent ;

	stPbSvClSlot m_client[PB_MAX_CLIENTS] ;
 
	inline void copyIfNotExists ( char *fn , char *basepath ) {
		char fromFn[PB_Q_MAXRESULTLEN*2+1] , toFn[PB_Q_MAXRESULTLEN*2+1] ;
		strcpy ( toFn , m_cwd ) ;
		strcat ( toFn , fn ) ;
		FILE *f = fopen ( toFn , "rb" ) ;
		if ( f != NULL ) fclose ( f ) ;
		else {
			strcpy ( fromFn , basepath ) ;
			strcat ( fromFn , fn ) ;
			PbCopyFile ( fromFn , toFn ) ;
		}
	}

	inline char *getBasePath ( char *path ) {
		strncpy ( path , m_basepath , PB_Q_MAXRESULTLEN ) ;
		path[PB_Q_MAXRESULTLEN] = 0 ;
		if ( !(*path) ) getcwd ( path , PB_Q_MAXRESULTLEN - 4 ) ;
		if ( *path && path[strlen(path)-1] != *pbDIRSEP ) strcat ( path , pbDIRSEP ) ;
		strcat ( path , "pb" pbDIRSEP ) ;
		return path ;
	}

	inline char *getHomePath ( void ) {
		strncpy ( m_cwd , m_homepath , PB_Q_MAXRESULTLEN ) ;
		m_cwd[PB_Q_MAXRESULTLEN] = 0 ;
		if ( !(*m_cwd) ) getcwd ( m_cwd , PB_Q_MAXRESULTLEN - 4 ) ;
		if ( *m_cwd && m_cwd[strlen(m_cwd)-1] != *pbDIRSEP ) strcat ( m_cwd , pbDIRSEP ) ;
		strcat ( m_cwd , "pb" pbDIRSEP ) ;
		return m_cwd ;
	}

	inline char *makefn ( char *buf , char *fn ) {	//assumes buf is large enough to hold cwd + fn + overhead
		if ( !(*m_cwd) ) {
			getHomePath() ;

			char basepath[PB_Q_MAXRESULTLEN+4] ;
			getBasePath ( basepath ) ;
			if ( stricmp ( basepath , m_cwd ) && *basepath && *m_cwd ) {
#ifdef __PBWIN32__
				mkdir ( m_cwd ) ;
#endif
#ifdef __PBLINUX__
				mkdir ( m_cwd , 511 ) ;
#endif
#ifdef __PBMAC__
				if ( m_cwd[strlen(m_cwd)-1] == *pbDIRSEP ) m_cwd[strlen(m_cwd)-1] = 0 ;
				mkdir ( m_cwd , 511 ) ;
				strcat ( m_cwd , pbDIRSEP ) ;
#endif
				copyIfNotExists ( "pbsv" pbDLLEXT , basepath ) ;
				copyIfNotExists ( "pbcl" pbDLLEXT , basepath ) ;
				copyIfNotExists ( "pbag" pbDLLEXT , basepath ) ;
			}
		}
		strcpy ( buf , m_cwd ) ;
		strcat ( buf , fn ) ;
		return buf ;
	}

	inline void UnloadClientDll ( void ) {
#ifdef __PBWIN32__
		if ( m_ClInstance != NULL ) FreeLibrary ( (HMODULE) m_ClInstance ) ;
#endif
#ifdef __PBLINUX__
		if ( m_ClInstance != NULL ) dlclose ( m_ClInstance ) ;
#endif
#ifdef __PBMAC__
		if ( m_ClInstance != NULL ) dlclose ( m_ClInstance ) ;
#endif
		m_ClInstance = NULL ;
	}

	inline char *LoadClientDll ( void ) {
		if ( m_ClInstance != NULL ) return NULL ;
		UnloadClientDll() ;

		//check for replacement (updated) dll files and rename if necessary

		char fn[PB_Q_MAXRESULTLEN*2+1] , extrafn[PB_Q_MAXRESULTLEN*2+1] ;

		FILE *f = fopen ( makefn ( fn , "pbclsnew" pbDLLEXT ) , "rb" ) ;
		if ( f != NULL ) {
			fclose ( f ) ;
			setRW ( makefn ( fn , "pbclsold" pbDLLEXT ) ) ;
			remove ( makefn ( fn , "pbclsold" pbDLLEXT ) ) ;
			rename ( makefn ( fn , "pbcls" pbDLLEXT ) , makefn ( extrafn , "pbclsold" pbDLLEXT ) ) ;
			setRW ( makefn ( fn , "pbcls" pbDLLEXT ) ) ;
			remove ( makefn ( fn , "pbcls" pbDLLEXT ) ) ;
			rename ( makefn ( fn , "pbclsnew" pbDLLEXT ) , makefn ( extrafn , "pbcls" pbDLLEXT ) ) ;
		}
		makefn ( fn , "pbcls" pbDLLEXT ) ;
#ifdef __PBWIN32__
		m_ClInstance = LoadLibraryA ( fn ) ;
#endif
#ifdef __PBLINUX__
		m_ClInstance = ::dlopen ( fn , RTLD_LAZY ) ;
#endif
#ifdef __PBMAC__
		m_ClInstance = dlopen ( fn , RTLD_LAZY ) ;
#endif
		if ( m_ClInstance != NULL ) return NULL ;
		return PbsSvDllLoadFail ;
	}

	inline void UnloadAgentDll ( void ) {
		m_Agent = NULL ;
#ifdef __PBWIN32__
		if ( m_AgInstance != NULL ) FreeLibrary ( (HMODULE) m_AgInstance ) ;
#endif
#ifdef __PBLINUX__
		if ( m_AgInstance != NULL ) dlclose ( m_AgInstance ) ;
#endif
#ifdef __PBMAC__
		if ( m_AgInstance != NULL ) dlclose ( m_AgInstance ) ;
#endif
		m_AgInstance = NULL ;
	}

	inline char *LoadAgentDll ( void ) {
		if ( m_AgInstance != NULL ) return NULL ;
		UnloadAgentDll() ;

		//check for replacement (updated) dll files and rename if necessary

		char fn[PB_Q_MAXRESULTLEN*2+1] , extrafn[PB_Q_MAXRESULTLEN*2+1] ;

		FILE *f = fopen ( makefn ( fn , "pbagsnew" pbDLLEXT ) , "rb" ) ;
		if ( f != NULL ) {
			fclose ( f ) ;
			setRW ( makefn ( fn , "pbagsold" pbDLLEXT ) ) ;
			remove ( makefn ( fn , "pbagsold" pbDLLEXT ) ) ;
			rename ( makefn ( fn , "pbags" pbDLLEXT ) , makefn ( extrafn , "pbagsold" pbDLLEXT ) ) ;
			setRW ( makefn ( fn , "pbags" pbDLLEXT ) ) ;
			remove ( makefn ( fn , "pbags" pbDLLEXT ) ) ;
			rename ( makefn ( fn , "pbagsnew" pbDLLEXT ) , makefn ( extrafn , "pbags" pbDLLEXT ) ) ;
		}
		makefn ( fn , "pbags" pbDLLEXT ) ;
#ifdef __PBWIN32__
		m_AgInstance = LoadLibraryA ( fn ) ;
		if ( m_AgInstance != NULL ) {
			m_Agent = GetProcAddress ( (HMODULE) m_AgInstance , "a" ) ;
			if ( m_Agent != NULL ) return NULL ;
			UnloadAgentDll() ;
		}
#endif
#ifdef __PBLINUX__
		m_AgInstance = ::dlopen ( fn , RTLD_LAZY ) ;
		if ( m_AgInstance != NULL ) {
			m_Agent = dlsym ( m_AgInstance , "a" ) ;
			if ( m_Agent != NULL ) return NULL ;
			UnloadAgentDll() ;
		}
#endif
#ifdef __PBMAC__
		m_AgInstance = dlopen ( fn , RTLD_LAZY ) ;
		if ( m_AgInstance != NULL ) {
			m_Agent = dlsym ( m_AgInstance , "_a" ) ;
			if ( m_Agent != NULL ) return NULL ;
			UnloadAgentDll() ;
		}
#endif
		return PbsSvDllLoadFail ;
	}

#ifndef __PBDLL__ //the following functions are not needed by the PB DLLs

	inline void uninitialize ( void ) {//also initializes game-side func ptrs
		m_GameCommand = NULL ;
		m_GameQuery = NULL ;
		m_GameMsg = NULL ;
		m_SendToClient = NULL ;
		m_SendToAddrPort = NULL ;
		m_PassConnectString = NULL ;
		m_AuthClient = NULL ;
		m_TrapConsole = NULL ;
		m_Md5 = NULL ;
	}

	inline void initialize ( void ) {
		uninitialize() ;
		m_GameCommand = PbSvGameCommand ;
		m_GameQuery = PbSvGameQuery ;
		m_GameMsg = PbSvGameMsg ;
		m_SendToClient = PbSvSendToClient ;
		m_SendToAddrPort = PbSvSendToAddrPort ;
	}

	inline void UnloadServerDll ( void ) {
		m_ProcessPbEvents = NULL ;
		m_AddPbEvent = NULL ;

		m_PassConnectString = NULL ;
		m_AuthClient = NULL ;
		m_TrapConsole = NULL ;
		m_Md5 = NULL ;

#ifdef __PBWIN32__
		if ( m_SvInstance != NULL ) FreeLibrary ( (HMODULE) m_SvInstance ) ;
#endif
#ifdef __PBLINUX__
		if ( m_SvInstance != NULL ) dlclose ( m_SvInstance ) ;
#endif
#ifdef __PBMAC__
		if ( m_SvInstance != NULL ) dlclose ( m_SvInstance ) ;
#endif
		m_SvInstance = NULL ;
	}

	inline char *LoadServerDll ( void ) {
		if ( m_SvInstance != NULL ) return NULL ;
		UnloadServerDll() ;

		char fn[PB_Q_MAXRESULTLEN*2+1] , extrafn[PB_Q_MAXRESULTLEN*2+1] ;

		//check for replacement (updated) dll file and rename if necessary
		//load PB server dll and retrieve exported function pointers
		FILE *f = fopen ( makefn ( fn , "pbsvnew" pbDLLEXT ) , "rb" ) ;
		if ( f != NULL ) {
			fclose ( f ) ;
			setRW ( makefn ( fn , "pbsvold" pbDLLEXT ) ) ;
			remove ( makefn ( fn , "pbsvold" pbDLLEXT ) ) ;
			rename ( makefn ( fn , "pbsv" pbDLLEXT ) , makefn ( extrafn , "pbsvold" pbDLLEXT ) ) ;
			setRW ( makefn ( fn , "pbsv" pbDLLEXT ) ) ;
			remove ( makefn ( fn , "pbsv" pbDLLEXT ) ) ;
			rename ( makefn ( fn , "pbsvnew" pbDLLEXT ) , makefn ( extrafn , "pbsv" pbDLLEXT ) ) ;
		}
#ifdef __PBWIN32__
		m_SvInstance = LoadLibraryA ( makefn ( fn , "pbsv" pbDLLEXT ) ) ;
		if ( m_SvInstance == NULL ) return PbsSvDllLoadFail ;
		m_ProcessPbEvents = (tdPbProcessPbEvents) GetProcAddress ( (HINSTANCE) m_SvInstance , "sa" ) ;
		m_AddPbEvent = 	(tdPbAddSvEvent) GetProcAddress ( (HINSTANCE) m_SvInstance , "sb" ) ;
#endif
#ifdef __PBLINUX__
		m_SvInstance = ::dlopen ( makefn ( fn , "pbsv" pbDLLEXT ) , RTLD_LAZY ) ;
		if ( m_SvInstance == NULL ) return PbsSvDllLoadFail ;
		m_ProcessPbEvents = (tdPbProcessPbEvents) dlsym ( m_SvInstance , "sa" ) ;
		m_AddPbEvent = 	(tdPbAddSvEvent) dlsym ( m_SvInstance , "sb" ) ;
#endif
#ifdef __PBMAC__
		m_SvInstance = dlopen ( makefn ( fn , "pbsv" pbDLLEXT ) , RTLD_LAZY ) ;
		if ( m_SvInstance == NULL ) return PbsSvDllLoadFail ;
		m_ProcessPbEvents = (tdPbProcessPbEvents) dlsym ( m_SvInstance , "_sa" ) ;
		m_AddPbEvent = 	(tdPbAddSvEvent) dlsym ( m_SvInstance , "_sb" ) ;
#endif

		if ( m_ProcessPbEvents == NULL || m_AddPbEvent == NULL ) {
			UnloadServerDll() ;
			return PbsSvDllProcFail ;
		}

		m_ReloadServer = 0 ;

		return NULL ;
	}

	inline char *AddPbEvent ( int type , int clientIndex , int datalen , char *data , int retry = 0 ) {
		if ( m_GameCommand == NULL ) return NULL ;//not considered an error, this signifies that PB is disabled

		if ( m_ReloadServer || m_SvInstance == NULL ) {
			if ( m_SvInstance != NULL ) {
				UnloadServerDll() ;
				return NULL ;
			}

			char *res = LoadServerDll() ;
			if ( res != NULL ) {
				if ( type == PB_EV_ISENABLED ) return (char *) 0 ;
				return res ;
			}
		}

		return m_AddPbEvent ( this , type , clientIndex , datalen , data , retry ) ;
	}

	inline char *ProcessPbEvents ( int TickLimit = 0 ) {
		if ( m_GameCommand == NULL ) return NULL ;//not considered an error, this signifies that PB is disabled
		if ( m_SvInstance == NULL ) {
			if ( m_ReloadServer ) AddPbEvent ( PB_EV_CONFIG , -1 , 0 , "" ) ;
			return NULL ;//no events have been successfully added so nothing to process
		}

		if ( m_ReloadServer ) {
			UnloadServerDll() ;
			return NULL ;
		}

		char *ret = m_ProcessPbEvents ( this , TickLimit ) ;

		if ( m_ReloadServer ) {
			UnloadServerDll() ;
			return NULL ;
		}

		return ret ;
	}

	inline void init ( void ) {
		m_svId = PB_SV_ID ;
		strcpy ( m_msgPrefix , "PunkBuster Server" ) ;
		m_SvInstance = NULL ;
		m_ReloadServer = 1 ;
		uninitialize() ;
	}

	inline stPbSv() {
		init() ;
	}

	inline ~stPbSv() {
		UnloadServerDll() ;
		UnloadClientDll() ;
		UnloadAgentDll() ;
	}

#endif //#ifndef __PBDLL__

} ;



#endif //#ifdef _cplusplus
#endif //#ifdef __WITHPB__
#endif //#ifndef __PBSV_H__
#endif // NOT_USE_PUNKBUSTER_SDK
