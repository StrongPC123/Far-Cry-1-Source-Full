// Copyright (C) 2001-2003 Even Balance, Inc.
//
//
// pbsv.cpp
//
// EVEN BALANCE - T.RAY
//


#define _cplusplus



#define PbSdk_DEFINED
#include "pbsdk.h"

#if !defined(NOT_USE_PUNKBUSTER_SDK)

#ifdef __WITH_PB__



//
// Functions and wrappers 
//   these are declared in pbcommon.h

extern "C" { 

void __cdecl PbSvAddEvent ( int event , int clientIndex , int datalen , char *data )
{
	if ( pbsdk == NULL ) return ;
	pbsdk->pbsv.AddPbEvent ( event , clientIndex , datalen , data , 0 ) ;
}

void __cdecl PbServerInitialize ( void )
{
	if ( pbsdk == NULL ) return ;
	pbsdk->pbsv.initialize() ;

	pbsdk->pb_getBasePath ( pbsdk->pbsv.m_basepath , PB_Q_MAXRESULTLEN ) ;
	pbsdk->pb_getHomePath ( pbsdk->pbsv.m_homepath , PB_Q_MAXRESULTLEN ) ;

	PbSvAddEvent ( PB_EV_CONFIG , -1 , 0 , "" ) ;
	if ( pbsdk->pbsv.m_AddPbEvent == NULL ) pbsdk->pb_SetSvPunkBuster ( "0" ) ;
}

void __cdecl PbServerProcessEvents ( void )
{
	if ( !isPBmultiplayerMode() ) return ;
	pbsdk->pbsv.ProcessPbEvents() ;
}

void __cdecl PbServerForceProcess ( void )
{
	if ( !isPBmultiplayerMode() ) return ;
	pbsdk->pbsv.ProcessPbEvents ( -1 ) ;
}

void __cdecl PbServerCompleteCommand ( char *buf , int buflen )
{
	if ( !isPBmultiplayerMode() ) return ;
	pbsdk->pbsv.AddPbEvent ( PB_EV_CMDCOMPL , -1 , buflen , buf ) ;
}

void __cdecl PbPassConnectString ( char *fromAddr , char *connectString )
{
	if ( !isPBmultiplayerMode() ) return ;
	if ( pbsdk->pbsv.m_PassConnectString == NULL ) return ;//means PB not installed/enabled
	pbsdk->pbsv.m_PassConnectString ( &pbsdk->pbsv , fromAddr , connectString ) ;
}

char * __cdecl PbAuthClient ( char *fromAddr , int cl_pb , char *cl_guid )
{
	if ( !isPBmultiplayerMode() ) return NULL ;
	if ( pbsdk->pbsv.m_AuthClient == NULL ) return NULL ;//means PB not installed/enabled
	return pbsdk->pbsv.m_AuthClient ( &pbsdk->pbsv , fromAddr , cl_pb , cl_guid ) ;
}

int __cdecl isPbSvEnabled ( void )
{
	if ( !isPBmultiplayerMode() ) return 0 ;
	return (int) pbsdk->pbsv.AddPbEvent ( PB_EV_ISENABLED , -1 , 0 , NULL ) ;
}

void __cdecl EnablePbSv ( void )
{
	if ( !isPBmultiplayerMode() ) return ;
	pbsdk->pbsv.AddPbEvent ( PB_EV_ENABLE , -1 , 0 , NULL ) ;
}

void __cdecl DisablePbSv ( void )
{
	if ( !isPBmultiplayerMode() ) return ;
	pbsdk->pbsv.AddPbEvent ( PB_EV_DISABLE , -1 , 0 , NULL ) ;
}

} //extern "C"

extern void __cdecl PbClientTrapConsole ( char *msg , int msglen ) ;//in pbcl.cpp
void __cdecl PbCaptureConsoleOutput ( char *msg , int msglen )
{
	if ( !isPBmultiplayerMode() ) return ;

	if ( pbsdk->pbcl.m_ReloadClient == 0 ) PbClientTrapConsole ( msg , msglen ) ;

	if ( pbsdk->pbsv.m_ReloadServer == 0 ) if ( pbsdk->pbsv.m_TrapConsole != NULL ) pbsdk->pbsv.m_TrapConsole ( &pbsdk->pbsv , msg , msglen ) ;

	if ( *msg == '/' || *msg == '\\' ) ++msg , --msglen ;
	if ( !strnicmp ( msg , "pb_" , 3 ) ) {
		if ( !strnicmp ( msg + 3 , "sv_" , 3 ) ) {
			if ( pbsdk->pbsv.m_TrapConsole != NULL ) PbSvAddEvent ( PB_EV_CMD , -1 , msglen , msg ) ;
		} else {
			if ( pbsdk->pbcl.m_ReloadClient == 0 ) PbClAddEvent ( PB_EV_CMD , msglen , msg ) ;
		}
	}

	if ( pbsdk->ConsoleCaptureBuf == NULL ) return ;
	int sl = strlen ( pbsdk->ConsoleCaptureBuf ) ;
	if ( sl + (int) strlen ( msg ) + 1 >= pbsdk->ConsoleCaptureBufLen ) return ;
	strcpy ( pbsdk->ConsoleCaptureBuf + sl , msg ) ;
	strcat ( pbsdk->ConsoleCaptureBuf , "\n" ) ;
}


//
// PbSvGameCommand
//
extern void PBpakNames ( char *buf ) ;
char * __cdecl PbSvGameCommand ( char *Cmd , char *Result )
{
	if ( !isPBmultiplayerMode() ) return NULL ;
	if ( !stricmp ( Cmd , "set_sv_punkbuster" ) ) pbsdk->pb_SetSvPunkBuster ( Result ) ;
	else if ( !stricmp ( Cmd , "pakNames" ) ) PBpakNames ( Result ) ;//note: Result must be 1025+ bytes for this one
	else if ( !stricmp ( Cmd , "ConCapBufLen" ) ) pbsdk->ConsoleCaptureBufLen = (int) Result ;
	else if ( !stricmp ( Cmd , "ConCapBuf" ) ) pbsdk->ConsoleCaptureBuf = Result ;
	else if ( !stricmp ( Cmd , "Cmd_Exec" ) ) {
		int pb = !strnicmp ( Result , "pb_" , 3 ) ;
		pbsdk->pb_ExecCmd ( Result ) ;
		if ( pb ) PbServerForceProcess() ;
	} else {
		char *arg1 = Result ;
		while ( *arg1 == ' ' ) ++arg1 ;
		while ( *arg1 && *arg1 != ' ' ) ++arg1 ;
		char *endResult = arg1 ;
		while ( *arg1 == ' ' ) ++arg1 ;

		if ( !stricmp ( Cmd , "DropClient" ) ) pbsdk->pb_DropClient ( atoi ( Result ) , arg1 ) ;
		else if ( !stricmp ( Cmd , "Cvar_Set" ) ) {
			char hold = *endResult ;
			*endResult = 0 ;
			pbsdk->pb_CvarSet ( Result , arg1 ) ;
			*endResult = hold ;
		}
	}
	
	return NULL ;
}



//
// PbSvGameQuery
//
// assumes Data buffer is appropriately size/allocated for Qtype call
char * __cdecl PbSvGameQuery ( int Qtype , char *Data )
{
	Data[PB_Q_MAXRESULTLEN] = 0 ;

	int i ;
	switch ( Qtype ) {
	case PB_Q_MAXCLIENTS: itoa ( PB_MAX_CLIENTS , Data , 10 ) ; break ;
	case PB_Q_CLIENT:
		i = atoi ( Data ) ;
		if ( !pbsdk->pb_GetClientInfo ( i , (stPb_Sv_Client *) Data ) ) return PbsQueryFail ;
		break ;
	case PB_Q_CVAR: strncpy ( Data , pbsdk->pb_GetCvarValue ( Data ) , PB_Q_MAXRESULTLEN ) ; break ;
	case PB_Q_STATS:
		i = atoi ( Data ) ;
		if ( !pbsdk->pb_GetClientStats ( i , Data ) ) return PbsQueryFail ;
		break ;
	}
	return NULL ;
}



//
// PbSvGameMsg
//
char * __cdecl PbSvGameMsg ( char *Msg , int Type )
{
	if ( !isPBmultiplayerMode() ) return NULL ;

	Type;//reserved
	pbsdk->pb_Outf ( "%s: %s\n" , pbsdk->pbsv.m_msgPrefix , Msg ) ;
	return NULL ;
}



//
// PbSvSendToClient
//
char * __cdecl PbSvSendToClient ( int DataLen , char *Data , int clientIndex )
{
	pbsdk->pb_SendSvPacket ( DataLen , Data , clientIndex ) ;
	return NULL ;
}



//
// PbSvSendToAddrPort
//
char * __cdecl PbSvSendToAddrPort ( char *addr , unsigned short port , int DataLen , char *Data )
{
	pbsdk->pb_SendUdpPacket ( addr , port , DataLen , Data , 0 ) ; //convert bits to bytes
	return NULL ;
}




#endif //#ifdef __WITHPB__
#endif // NOT_USE_PUNKBUSTER_SDK