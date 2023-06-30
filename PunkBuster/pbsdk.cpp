//
// pbsdk.cpp
//
// PunkBuster / Game Integration SDK
//
// © Copyright 2003-2004 Even Balance, Inc. All Rights Reserved.
//
// This Software Development Kit (SDK) is proprietary and confidential. It may not be used,
// transferred, displayed or otherwise distributed in any manner except by express written 
// consent of Even Balance, Inc.
//
// created MAR 27 2003 by T.Ray @ Even Balance
// last modified MAR 05 2004 by T.Ray @ Even Balance
//


#include "../CryNetwork/StdAfx.h"

#if !defined(NOT_USE_PUNKBUSTER_SDK)
#include "../CryNetwork/PunkBusterInterface.h"



#define DEFINE_PbSdk	/* this define is used once in all game modules (usually where the main() function is defined) */
#include "pbsdk.h"



#ifdef __WITH_PB__
//
// PBsdk_getBasePath
//
extern "C" char *PBsdk_getBasePath ( char *path , int maxlen )
{
	if ( *path == 0 ) {	//only populate if empty
		getcwd ( path , maxlen ) ;
	}

	return path ;
}
#endif



#ifdef __WITH_PB__
extern void PBgetHomePath ( char *path , int maxlen ) ;
//
// PBsdk_getHomePath
//
extern "C" char *PBsdk_getHomePath ( char *path , int maxlen )
{
	if ( *path == 0 ) {	//only populate if empty
		PBgetHomePath ( path , maxlen ) ;
		if ( *path == 0 ) getcwd ( path , maxlen ) ;
	}

	return path ;
}
#endif


#ifdef __WITH_PB__
//
// PBsdk_Out
//
void PBoutgame ( char *text , int hudAlso ) ;//defined in PunkBusterInterface.cpp
extern void Com_Printf( const char *msg, ... ) ;
void Com_Printf( const char *msg, ... ) { }
//
extern "C" void PBsdk_Out ( char *msg )
{
	if ( pbsdk->pbinterface == NULL ) return ;

	char *cp = msg ;
	int hudAlso = 1 ;
	if ( !strnicmp ( msg , "[skipnotify]" , 12 ) ) {
		hudAlso = 0 ;
		cp += 12 ;
	}
	PBoutgame ( msg , hudAlso  ) ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_SendUdpPacket
//
extern void Sys_PBSendUdpPacket ( char *addr , unsigned short port , int datalen , char *data , int isFromClient ) ;
//
extern "C" void PBsdk_SendUdpPacket ( char *addr , unsigned short port , int datalen , char *data , int isFromClient )
{
	Sys_PBSendUdpPacket ( addr , port , datalen , data , isFromClient ) ;
}
#endif



extern void PBsendPktToServer ( int datalen , char *data ) ;
extern int PBisLocalServer ( void ) ;
#ifdef __WITH_PB__
//
// PBsdk_SendClPacket
//
extern "C" void PBsdk_SendClPacket ( int datalen , char *data )
{
	if ( pbsdk == NULL ) return ;
	if ( PBisLocalServer () ) {
		int i ;
		for ( i = 0 ; i < PB_MAX_CLIENTS ; i++ ) if ( !stricmp ( "localhost" , pbsdk->pbsv.m_client[i].pbc.ip ) ) break ;
		if ( i < PB_MAX_CLIENTS ) PbSvAddEvent ( PB_EV_PACKET , i , datalen , data ) ;
	} else {
		PBsendPktToServer ( datalen , data ) ;
	}
}
#endif



extern void PBsendPktToClient ( int datalen , char *data , char *addr ) ;
/*SDK-sendsvpacket
*/
#ifdef __WITH_PB__
//
// PBsdk_SendSvPacket
//
extern "C" void PBsdk_SendSvPacket ( int datalen , char *data , int index )
{
	if ( pbsdk == NULL ) return ;
	char *addr = pbsdk->pbsv.m_client[index].pbc.ip ;
	if ( *addr == 0 ) return ;
	if ( !stricmp ( addr , "localhost" ) ) {
		PbClAddEvent ( PB_EV_PACKET , datalen , data ) ;
	} else {
		PBsendPktToClient ( datalen , data , addr ) ;
	}
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_CvarSet
//
extern void PBcvar_Set ( const char *cvar , const char *value ) ;		//sample forward declaration
//
extern "C" void PBsdk_CvarSet ( const char *varName , const char *value )
{
	PBcvar_Set ( varName , value ) ;									//sample function call
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_SetClPunkBuster
//
extern "C" void PBsdk_SetClPunkBuster ( char *value )
{
	PBsdk_CvarSet ( "cl_punkbuster" , value ) ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_SetSvPunkBuster
//
//
extern "C" void PBsdk_SetSvPunkBuster ( char *value )
{
	PBsdk_CvarSet ( "sv_punkbuster" , value ) ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_ExecCmd
//
extern void	PBcmd_execString ( const char *text ) ;				//sample forward declaration
//
extern "C" void PBsdk_ExecCmd ( const char *cmd )
{
	PBcmd_execString ( cmd ) ;										//sample function call
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_CvarValidate
//
extern char *PbCvarValidate ( char *buf ) ;							//sample forward declaration
char *PbCvarValidate ( char *buf ){*buf=0; return buf;}				//empty function - remove
//
extern "C" char *PBsdk_CvarValidate ( char *buf )
{
	return PbCvarValidate ( buf ) ;									//sample function call
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_CvarWalk
//
extern int PBcvarWalk ( char **name , char **string , int *flags , char **resetString ) ;	//sample forward declaration
//
extern "C" int PBsdk_CvarWalk ( char **name , char **string , int *flags , char **resetString )
{
	return PBcvarWalk ( name , string , flags , resetString ) ;								//sample function call
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_GetKeyName
//
extern int PBbindStuff ( int type , const char **data ) ;
char *Key_KeynumToString( int keynum ) {return "";}
//
extern "C" char *PBsdk_GetKeyName ( int keynum )
{
	static const char *data ;
	char buf[50] ;
	itoa ( keynum , buf , 10 ) ;
	data = buf ;
	PBbindStuff ( 2 , &data ) ;
	return (char *) data ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_GetKeyBinding
//
extern "C" char *PBsdk_GetKeyBinding ( int keynum )
{
	static const char *data ;
	char buf[50] ;
	itoa ( keynum , buf , 10 ) ;
	data = buf ;
	PBbindStuff ( 3 , &data ) ;
	return (char *) data ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_GetMaxKeys
//
int PbMaxKeys ( void ) {return 0;}
//
extern "C" int PBsdk_GetMaxKeys ( void )
{
	return PBbindStuff ( 1 , NULL ) ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_GetServerAddr
//
char *PBserverIp ( int bClient = false ) ;
extern "C" char *PBsdk_GetServerAddr ( void )
{
	return PBserverIp() ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_GetKeyValue
//
const char *PBkeyValue ( char *notused , char *key ) ;
extern "C" char *PBsdk_GetKeyValue ( char *s , char *k )
{
	return (char *) PBkeyValue ( s , k ) ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_GetServerInfo
//
extern char *PB_Q_Serverinfo ( void ) ;
char *PB_Q_Serverinfo ( void ){ return "" ;}
//
extern "C" char *PBsdk_GetServerInfo ( void )
{
	return PB_Q_Serverinfo() ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_GetCvarValue
//
extern char *PBcvar_VariableString( const char *var_name ) ;
extern char *PBgameVer ( void ) ;
extern char *PBserverHostname ( void ) ;
//
extern "C" char *PBsdk_GetCvarValue ( char *var_name )
{
	//special cases
	if ( !stricmp ( var_name , "version" ) ) return ( strcpy ( var_name , PBgameVer() ) ) ;
	if ( !stricmp ( var_name , "name" ) ) {
		char *cp = PBcvar_VariableString ( "p_name" ) ;
		if ( *cp == 0 ) cp = "Jack Carver" ;
		return cp ;
	}
	if ( !stricmp ( var_name , "sv_hostname" ) ) {
		strcpy ( var_name , "sv_name" ) ;
		char *cp = PBcvar_VariableString ( var_name ) ;
		if ( *cp == 0 ) cp = PBserverHostname() ;
		return cp ;
	}
	if ( !stricmp ( var_name , "server" ) ) return PBserverIp ( true ) ;

	//redirects
	if ( !stricmp ( var_name , "mapname" ) ) strcpy ( var_name , "g_levelName" ) ;

	return PBcvar_VariableString ( var_name ) ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_GlQuery
//
extern char *PBqueryGL ( int type ) ;
extern "C" char *PBsdk_GlQuery ( int queryType )
{
	return PBqueryGL ( queryType ) ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_DropClient
//
extern void PBdropClient ( int clientIndex , char *reason ) ;
//
extern "C" void PBsdk_DropClient ( int clientIndex , char *reason )
{
	PBdropClient ( clientIndex , reason ) ;
}
#endif



#ifdef __WITH_PB__
extern int PBgetClientInfo ( stPb_Sv_Client *c ) ;
//
// PBsdk_GetClientInfo
//
extern "C" int PBsdk_GetClientInfo ( int svsIndex , stPb_Sv_Client *c )
{
	memset ( c , 0 , sizeof ( *c ) ) ;															//clear structure
	if ( svsIndex < 0 || svsIndex >= PB_MAX_CLIENTS ) return 0 ;		//return on invalid index
	if ( *pbsdk->pbsv.m_client[svsIndex].pbc.ip ) {									//player in this slot?
		strcpy ( c->ip , pbsdk->pbsv.m_client[svsIndex].pbc.ip ) ;		//populate ip/guid with prior values
		strcpy ( c->guid , pbsdk->pbsv.m_client[svsIndex].pbc.guid ) ;
		int gci = PBgetClientInfo ( c ) ;															//populate name field
		if ( gci ) {
			strcpy ( pbsdk->pbsv.m_client[svsIndex].pbc.name , c->name ) ;//name can change so we update our internal array structure each frame
			strcpy ( pbsdk->pbsv.m_client[svsIndex].pbc.guid , c->guid ) ;//guid is persistent and stored in pbc.guid
		} else *c->ip = 0 ;																							//player not set up yet (i.e. GetSlotInfo returned false)
	}
	return 1 ;
}
#endif



#ifdef __WITH_PB__
//
// PBsdk_GetClientStats
//
extern int PBgetStats ( int svsIndex , char *Data ) ;
//
extern "C" int PBsdk_GetClientStats ( int index , char *data )
{
	if ( index < 0 || index >= PB_MAX_CLIENTS ) return 0 ;		//return on invalid index
	if ( *pbsdk->pbsv.m_client[index].pbc.ip == 0 ) return 0 ;	//no client at this index
	return PBgetStats ( index , data ) ;
}
#endif



//
// PbSvAddClient
//
//NOTE: This function adds the player to PB's internal array used to track reliable slot numbers
int PbSvAddClient ( char *addr , char *name , char *guid )
{
	if ( pbsdk == NULL || *addr == 0 ) return 0 ;

	int i , j = -1 ;
	for ( i = 0 ; i < PB_MAX_CLIENTS ; i++ ) {
		if ( !stricmp ( addr , pbsdk->pbsv.m_client[i].pbc.ip ) ) {
			char buf[1025] ;
			sprintf ( buf , "ERROR: Game is reporting duplicate player IP:Port %s as new connection" , addr ) ;
			PBoutgame ( buf , 1 ) ;
			return 0 ;
		}
		if ( *pbsdk->pbsv.m_client[i].pbc.ip == 0 && j < 0 ) j = i ;
	}
	if ( j < 0 ) return 0 ;//all slots full

	strncpy ( pbsdk->pbsv.m_client[j].pbc.ip , addr , PB_NAMELEN ) ;
	pbsdk->pbsv.m_client[j].pbc.ip[PB_NAMELEN] = 0 ;
	strncpy ( pbsdk->pbsv.m_client[j].pbc.name , name , PB_NAMELEN ) ;
	pbsdk->pbsv.m_client[j].pbc.name[PB_NAMELEN] = 0 ;
	strncpy ( pbsdk->pbsv.m_client[j].pbc.guid , guid , PB_GUIDLEN ) ;
	pbsdk->pbsv.m_client[j].pbc.guid[PB_GUIDLEN] = 0 ;
	return j + 1 ;//return PB slot # (1 to max)
}



//
// PbSvRemoveClient
//
//NOTE: This function removes the player from PB's internal array - see PBsdk_DropClient below for kicks
int PbSvRemoveClient ( char *addr )
{
	if ( pbsdk == NULL ) return 0 ;
	int i ;
	for ( i = 0 ; i < PB_MAX_CLIENTS ; i++ ) {
		if ( strcmp ( pbsdk->pbsv.m_client[i].pbc.ip , addr ) ) continue ;
		memset ( &pbsdk->pbsv.m_client[i].pbc , 0 , sizeof ( pbsdk->pbsv.m_client[i].pbc ) ) ;
		return i + 1 ;//return PB slot # (1 to max)
	}
	return 0 ;//return 0 means client pointer not found in array (should never happen)
}



#ifdef __WITH_PB__
//
// PBsdk_SetPointers
//
extern "C" void PBsdk_SetPointers ( void *pbinterface )
{
	static int tries = 0 ;

	if ( pbsdk != NULL ) return ;	//already accomplished

	if ( tries >= 3 ) return ;		//after 3 tries, give up

	++tries ;

	if ( pbinterface == NULL ) return ;

	pbsdk = &PbSdkInstance ;
//	PBsdk_getPbSdkPointer ( "module_filename_goes_here" , 0 ) ;/*note use non-zero for Flag if desired*/

	if ( pbsdk == NULL ) return ;	//failed to get pointer to struct instance

	pbsdk->pbinterface = pbinterface ;

	//uncomment lines from the following section that are to be defined in "this" game module
	pbsdk->m_CvarSet = PBsdk_CvarSet ;
//	pbsdk->m_CvarValidate = PBsdk_CvarValidate ;
	pbsdk->m_CvarWalk = PBsdk_CvarWalk ;
	pbsdk->m_DropClient = PBsdk_DropClient ;
	pbsdk->m_ExecCmd = PBsdk_ExecCmd ;
	pbsdk->m_getBasePath = PBsdk_getBasePath ;
	pbsdk->m_GetClientInfo = PBsdk_GetClientInfo ;
	pbsdk->m_GetClientStats = PBsdk_GetClientStats ;
	pbsdk->m_GetCvarValue = PBsdk_GetCvarValue ;
	pbsdk->m_getHomePath = PBsdk_getHomePath ;
	pbsdk->m_GetKeyBinding = PBsdk_GetKeyBinding ;
	pbsdk->m_GetKeyName = PBsdk_GetKeyName ;
	pbsdk->m_GetKeyValue = PBsdk_GetKeyValue ;
//	pbsdk->m_GetMaxClients = PBsdk_GetMaxClients ;
	pbsdk->m_GetMaxKeys = PBsdk_GetMaxKeys ;
	pbsdk->m_GetServerAddr = PBsdk_GetServerAddr ;
//	pbsdk->m_GetServerInfo = PBsdk_GetServerInfo ;
	pbsdk->m_GlQuery = PBsdk_GlQuery ;
	pbsdk->m_Out = PBsdk_Out ;
	pbsdk->m_SendClPacket = PBsdk_SendClPacket ;
	pbsdk->m_SendSvPacket = PBsdk_SendSvPacket ;
	pbsdk->m_SendUdpPacket = PBsdk_SendUdpPacket ;
	pbsdk->m_SetClPunkBuster = PBsdk_SetClPunkBuster ;
	pbsdk->m_SetSvPunkBuster = PBsdk_SetSvPunkBuster ;
}
#endif
#endif // NOT_USE_PUNKBUSTER_SDK