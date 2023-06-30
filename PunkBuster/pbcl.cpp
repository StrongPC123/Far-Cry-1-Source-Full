// Copyright (C) 2001-2003 Even Balance, Inc.
//
//
// pbcl.cpp
//
// EVEN BALANCE - T.RAY
//



#define _cplusplus



#include "pbmd5.h"

#if !defined(NOT_USE_PUNKBUSTER_SDK)

#define PbSdk_DEFINED
#include "pbsdk.h"



#ifdef __WITH_PB__



//
// PbClGameCommand
//
extern void PBpakNames ( char *buf ) ;
char * __cdecl PbClGameCommand ( char *Cmd , char *Result )
{
	char *arg1 = Result ;
	while ( *arg1 == ' ' ) ++arg1 ;
	while ( *arg1 && *arg1 != ' ' ) ++arg1 ;
	while ( *arg1 == ' ' ) ++arg1 ;

	if ( !stricmp ( Cmd , "set_cl_punkbuster" ) ) pbsdk->pb_SetClPunkBuster ( Result ) ;
	else if ( !stricmp ( Cmd , "pakNames" ) ) 
		PBpakNames ( Result ) ;//note: Result must be 1025+ bytes for this one
	else if ( !stricmp ( Cmd , "Cvar_Set" ) ) pbsdk->pb_CvarSet ( Result , arg1 ) ;
	else if ( !stricmp ( Cmd , "Cmd_Exec" ) ) pbsdk->pb_ExecCmd ( Result ) ;
	return NULL ;
}



//extern int PbSearchBindings ( char *subtext , int iStart ) ;								//cl_keys.c
//
// PbClGameQuery
//
// assumes Data points to buffer at least as large as PB_Q_MAXRESULTLEN+1
//
char * __cdecl PbClGameQuery ( int Qtype , char *Data )
{
	if ( Data == NULL ) return NULL ;

	Data[PB_Q_MAXRESULTLEN] = 0 ;

	char *arg2 = Data , *name , *string , *resetString ;
	int i , n , flags ;

	while ( *arg2 && *arg2 != ' ' ) ++arg2 ;
	while ( *arg2 == ' ' ) ++arg2 ;
	switch ( Qtype ) {
	case PB_Q_CVAR: strncpy ( Data , pbsdk->pb_GetCvarValue ( Data ) , PB_Q_MAXRESULTLEN ) ; break ;
	case PB_Q_SINFO: strncpy ( Data , pbsdk->pb_GetKeyValue ( pbsdk->pb_GetServerInfo() , Data ) , PB_Q_MAXRESULTLEN ) ; break ;
	case PB_Q_SADDR: strncpy ( Data , pbsdk->pb_GetServerAddr() , PB_Q_MAXRESULTLEN ) ; break ;
	case PB_Q_SEARCHBINDINGS:
		n = pbsdk->pb_GetMaxKeys() ;
		for ( i = atoi ( Data ) ; i < n ; i++ ) 
			if ( stristr ( pbsdk->pb_GetKeyBinding ( i ) , arg2 ) != NULL ) {
				itoa ( i , Data , 10 ) ;
				return NULL ;
			}
		itoa ( -1 , Data , 10 ) ;
		break ;
	case PB_Q_GETBINDING:
		strncpy ( Data , pbsdk->pb_GetKeyBinding ( atoi ( Data ) ) , PB_Q_MAXRESULTLEN ) ; break ;
	case PB_Q_KEYNAME:
		strncpy ( Data , pbsdk->pb_GetKeyName ( atoi ( Data ) ) , PB_Q_MAXRESULTLEN ) ; break ;
	case PB_Q_SEARCHCVARS:
		if ( ! (*Data) ) break ;
		while ( pbsdk->pb_CvarWalk ( &name , &string , &flags , &resetString ) ) {
			if ( name == NULL || string == NULL ) continue ;
			if ( !(*name) || !(*string) ) continue ;
			if ( stristr ( string , Data ) != NULL ) {
				strncpy ( Data , name , PB_Q_MAXRESULTLEN ) ;
				return NULL ;
			}
		}
		*Data = 0 ;
		break ;
	case PB_Q_CVARVALID: return pbsdk->pb_CvarValidate ( Data ) ;
	case PB_Q_CVARFLAGS:				//note: this query type returns NULL when done only, cvar name otherwise
		if ( pbsdk->pb_CvarWalk ( &name , &string , &flags , &resetString ) == 0 ) return NULL ;
		if ( name == NULL ) name = "" ;
		itoa ( flags , Data , 10 ) ;
		return name ;
	case PB_Q_CVARDEFAULTS:				//note: this query type returns NULL when done only, cvar name otherwise
		if ( pbsdk->pb_CvarWalk ( &name , &string , &flags , &resetString ) == 0 ) return NULL ;
		if ( name == NULL || string == NULL || resetString == NULL ) name = string = resetString = "" ;
		if ( !strcmp ( string , resetString ) ) *Data = 0 ;
		else {
			strncpy ( Data + 1 , resetString , PB_Q_MAXRESULTLEN - 2 ) ;
			Data[0] = '"' ;
			strcat ( Data , "\"" ) ;
		}
		return name ;
	case PB_Q_EXEINSTANCE: 
#ifdef __PBWIN32__
		return (char *) pbsdk->exeInstance ;
#else
		strncpy ( Data , (char *) pbsdk->exeInstance , PB_Q_MAXRESULTLEN ) ;
		return NULL ;
#endif
	case PB_Q_DLLHANDLE: return (char *) pbsdk->pb_DllHandle ( Data ) ;
	default: *Data = 0 ; break ;
	}
	return NULL ;
}

//
// PbClGameMsg
//
char * __cdecl PbClGameMsg ( char *Msg , int Type )
{
	if ( !isPBmultiplayerMode() ) return NULL ;

	Type;//reserved
	pbsdk->pb_Outf ( "%s: %s\n" , pbsdk->pbcl.m_msgPrefix , Msg ) ;
	return NULL ;
}



//
// PbClSendToServer
//
char * __cdecl PbClSendToServer ( int DataLen , char *Data )
{
	pbsdk->pb_SendClPacket ( DataLen , Data ) ;
	return NULL ;
}



//
// PbClSendToAddrPort
//
char * __cdecl PbClSendToAddrPort ( char *addr , unsigned short port , int DataLen , char *Data )
{
	pbsdk->pb_SendUdpPacket ( addr , port , DataLen , Data , 1 ) ;//1 means coming from client
	return NULL ;
}



//
// Function wrappers used to call C++ functions from C
//   these are declared in pbcommon.h

extern "C" {

void __cdecl PbClAddEvent ( int event , int datalen , char *data )
{
	if ( pbsdk == NULL ) return ;
	pbsdk->pbcl.AddPbEvent ( event , datalen , data , 0 ) ;
}

int __cdecl PbTrapPreExecCmd ( char *cmdtext )//return 0 if game should continue exec'ing the command, 1 if not
{
	if ( !isPBmultiplayerMode() ) return 0 ;
	if ( pbsdk->pbcl.m_TrapPreExecCmd == NULL ) return 0 ;
	return pbsdk->pbcl.m_TrapPreExecCmd ( &pbsdk->pbcl , cmdtext ) ;
}

void __cdecl PbClientTrapConsole ( char *msg , int msglen )
{
	if ( !isPBmultiplayerMode() ) return ;
	if ( pbsdk->pbcl.m_TrapConsole == NULL ) return ;
	pbsdk->pbcl.m_TrapConsole ( &pbsdk->pbcl , msg , msglen ) ;
}

void __cdecl PbClientInitialize ( void *exeInst )
{
	if ( pbsdk == NULL ) return ;

	pbsdk->exeInstance = exeInst ;
	pbsdk->pbcl.pbsvptr = &pbsdk->pbsv ;
	pbsdk->pbcl.initialize() ;

	pbsdk->pb_getBasePath ( pbsdk->pbcl.m_basepath , PB_Q_MAXRESULTLEN ) ;
	pbsdk->pb_getHomePath ( pbsdk->pbcl.m_homepath , PB_Q_MAXRESULTLEN ) ;

	PbClAddEvent ( PB_EV_CONFIG , 0 , "" ) ;
	if ( pbsdk->pbcl.m_ClInstance == NULL ) pbsdk->pb_SetClPunkBuster ( "0" ) ;
}

void __cdecl PbClientProcessEvents ( void )
{
	if ( !isPBmultiplayerMode() ) return ;
	pbsdk->pbcl.ProcessPbEvents() ;
}

void __cdecl PbClientForceProcess ( void )
{
	if ( !isPBmultiplayerMode() ) return ;
	pbsdk->pbcl.ProcessPbEvents ( -1 ) ;
}

//added for Enemy Territory - PB knows max pktlen is 1024
void __cdecl PbClientConnecting ( int status , char *pkt , int *pktlen )
{
	if ( !isPBmultiplayerMode() ) return ;
	if ( pbsdk->pbcl.m_ClientConnect == NULL ) return ;
	pbsdk->pbcl.m_ClientConnect ( &pbsdk->pbcl , status , pkt , pktlen ) ;
}

void __cdecl PbClientCompleteCommand ( char *buf , int buflen )
{
	if ( !isPBmultiplayerMode() ) return ;
	pbsdk->pbcl.AddPbEvent ( PB_EV_CMDCOMPL , buflen , buf ) ;
}

void __cdecl md5Digest2text ( MD5_CTX *m , char *textbuf )//assumes textbuf is 33+ chars
{
	sprintf ( textbuf , "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
		(int) m->digest[0] , (int) m->digest[1] , (int) m->digest[2] , (int) m->digest[3] ,
		(int) m->digest[4] , (int) m->digest[5] , (int) m->digest[6] , (int) m->digest[7] ,
		(int) m->digest[8] , (int) m->digest[9] , (int) m->digest[10] , (int) m->digest[11] ,
		(int) m->digest[12] , (int) m->digest[13] , (int) m->digest[14] , (int) m->digest[15] ) ;
}

char * __cdecl PbSetGuid ( char *nums , int len )//updated for ET
{
	if ( !isPBmultiplayerMode() ) return "" ;
	MD5_CTX m ;
	MD5Init ( &m , 11961507 ) ;
	MD5Update ( &m , (unsigned char *) nums , len ) ;
	MD5Final ( &m ) ;
	md5Digest2text ( &m , pbsdk->pbcl.m_guid ) ;
	MD5Init ( &m , 334422 ) ;
	MD5Update ( &m , (unsigned char *) pbsdk->pbcl.m_guid , strlen ( pbsdk->pbcl.m_guid ) ) ;
	MD5Final ( &m ) ;
	md5Digest2text ( &m , pbsdk->pbcl.m_guid ) ;
	return pbsdk->pbcl.m_guid ;
}

int __cdecl isPbClEnabled ( void )
{
	if ( !isPBmultiplayerMode() ) return 0 ;
	return (int) pbsdk->pbcl.AddPbEvent ( PB_EV_ISENABLED , 0 , NULL ) ;
}

int __cdecl getPbGuidAge ( void )
{
	if ( !isPbClEnabled() ) return -2 ;
	return (int) pbsdk->pbcl.AddPbEvent ( PB_EV_GUIDAGE , 0 , NULL ) ;//returns -1 if bad/missing cdkey
}

void __cdecl EnablePbCl ( void )
{
	if ( !isPBmultiplayerMode() ) return ;
	pbsdk->pbcl.AddPbEvent ( PB_EV_ENABLE , 0 , NULL ) ;
}

void __cdecl DisablePbCl ( void )
{
	if ( !isPBmultiplayerMode() ) return ;
	pbsdk->pbcl.AddPbEvent ( PB_EV_DISABLE , 0 , NULL ) ;
}

} //extern "C"

#endif //#ifdef __WITHPB__
#endif // NOT_USE_PUNKBUSTER_SDK