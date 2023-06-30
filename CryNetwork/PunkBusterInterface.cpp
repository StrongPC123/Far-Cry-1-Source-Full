////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PunkBusterInterface.cpp
//  Version:     v1.00
//  Created:     1/3/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: Interface to the PunkBuster from CryEngine.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#if !defined(NOT_USE_PUNKBUSTER_SDK)

#include "PunkBusterInterface.h"
#define PbSdk_DEFINED
#include "../PunkBuster/pbsdk.h"
#include "../PunkBuster/pbmd5.h"

#include "Network.h"
#include "IRenderer.h"
#include "ServerSlot.h"
#include "Server.h"
#include "Client.h"
#include "ClientLocal.h"
#include "ICryPak.h"

static const char RegistryKeyName[] = "SOFTWARE\\Crytek\\FarCry";
static const char CDKEYREGKEY[] = "CDKey";

void PBcomputeHash ( char *idhash , CServerSlot *pSlot ) ;//defined in this source file after class definitions


//////////////////////////////////////////////////////////////////////////
CPunkBusterInterface::CPunkBusterInterface( CNetwork *pNetwork )
{
	m_bSinglePlayer = false ;
	m_bClInitialized = false;
	m_bSvInitialized = false;
	m_pNetwork = pNetwork;
	m_pSystem = pNetwork->GetSystem();
	m_pClient = 0;
	m_pClientLocal = 0;
	m_pServer = 0;
	sys_punkbuster_loaded = 0;

	cl_punkbuster = m_pSystem->GetIConsole()->CreateVariable( "cl_punkbuster","0",VF_DUMPTODISK,"Enables PunkBuster for client, 1=Enabled,0=Disabled" );
	sv_punkbuster = m_pSystem->GetIConsole()->CreateVariable( "sv_punkbuster","0",VF_DUMPTODISK,"Enables PunkBuster for server, 1=Enabled,0=Disabled" );
	fs_homepath = m_pSystem->GetIConsole()->CreateVariable( "fs_homepath","",VF_DUMPTODISK,"Specifies alternate PunkBuster 'home' (if non-empty)" );

	// Start recieving OnBeforeVarChange events.
	m_pSystem->GetIConsole()->AddConsoleVarSink(this);
	m_pSystem->GetIConsole()->AddOutputPrintSink(this);
}

//////////////////////////////////////////////////////////////////////////
CPunkBusterInterface::~CPunkBusterInterface()
{
	if ( sys_punkbuster_loaded ) {
		SAFE_RELEASE(sys_punkbuster_loaded);
		sys_punkbuster_loaded = 0 ;
	}
	m_pSystem->GetIConsole()->RemoveOutputPrintSink(this);
	m_pSystem->GetIConsole()->RemoveConsoleVarSink(this);
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::Init( bool bClient , bool isLocalServer )
{
	if ( bClient == false ) {
		m_bSinglePlayer = !isLocalServer ;
		if ( m_bSinglePlayer == true ) {
			ShutDown ( false ) ;
			return ;
		}
	} else if ( m_bSinglePlayer ) return ;

	if ( !sys_punkbuster_loaded ) {
		sys_punkbuster_loaded = m_pSystem->GetIConsole()->CreateVariable( "sys_punkbuster_loaded","1",NULL);
	}

	if ( bClient ) {
		if ( m_bClInitialized ) return ;
		PbClientInitialize ( this ) ;
		m_bClInitialized = true;
	} else {
		if ( m_bSvInitialized ) return ;
		PbServerInitialize() ;
		m_bSvInitialized = true;
	}
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::SetServer( CServer *pServer )
{
	m_pServer = pServer;
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::SetClient( CClient *pClient )
{
	m_pClient = pClient;
	m_pClientLocal = 0;
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::SetClientLocal( CClientLocal *pClient )
{
	m_pClientLocal = pClient;
	m_pClient = 0;
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::LockCVars()
{
	cl_punkbuster->SetFlags( cl_punkbuster->GetFlags() | VF_READONLY );
	sv_punkbuster->SetFlags( sv_punkbuster->GetFlags() | VF_READONLY );
	fs_homepath->SetFlags( fs_homepath->GetFlags() | VF_READONLY );
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::UnlockCVars()
{
	cl_punkbuster->SetFlags( cl_punkbuster->GetFlags() & ~VF_READONLY );
	sv_punkbuster->SetFlags( sv_punkbuster->GetFlags() & ~VF_READONLY );
	fs_homepath->SetFlags( fs_homepath->GetFlags() & ~VF_READONLY );
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::ShutDown( bool bClient )
{
//	m_bInitialized = false;
	if ( sys_punkbuster_loaded ) {
		SAFE_RELEASE(sys_punkbuster_loaded);
		sys_punkbuster_loaded = 0 ;
	}

	if (bClient)
	{
		m_pClientLocal = 0;
		m_pClient = 0;
	}
	else
	{
		m_pServer = 0;
	}

	UnlockCVars();
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::Update( bool bClient )
{
	if ( !sys_punkbuster_loaded ) return ;//no PB processing when PB has been shut down or is not yet initialized

	if ( bClient) {
		if (!m_bClInitialized) return;
		PbClientProcessEvents() ;
	} else {
		if (!m_bSvInitialized) return;
		PbServerProcessEvents() ;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CPunkBusterInterface::OnBeforeVarChange( ICVar *pVar,const char *sNewValue )
{
//	if (!m_bInitialized)
//		return true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::Print( const char *inszText )
{
	PbCaptureConsoleOutput ( (char *) inszText , strlen ( inszText ) ) ;
}

//////////////////////////////////////////////////////////////////////////
bool CPunkBusterInterface::LoadCDKey( string &sCDKey )
{
	if (ReadStringFromRegistry("Ubi.com", CDKEYREGKEY, sCDKey))
	{
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CPunkBusterInterface::ReadStringFromRegistry(const string &szKeyName, const string &szValueName, string &szValue)
{
#ifdef WIN32
	HKEY hKey;

	string szREGKeyName = string(RegistryKeyName) + string("\\") + szKeyName;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, szREGKeyName.c_str(), &hKey) != ERROR_SUCCESS)
	{
		return false;
	}
	DWORD dwSize = 0;
	if (RegQueryValueEx(hKey, szValueName.c_str(), 0, 0, 0, &dwSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return false;
	}
	szValue.resize(dwSize);
	RegQueryValueEx(hKey, szValueName.c_str(), 0, 0, (LPBYTE)szValue.c_str(), &dwSize);
	RegCloseKey(hKey);

	return true;
#else
	return false;
#endif
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::OnAddClient( CIPAddress &clientIP )
{
	// Must have server by this point.
//	assert( m_pServer );
	if ( !m_pServer ) return ;//happens in single-player mode

	//at this point in the process, just add the client address and idhash, see PBgetClientInfo() later in this source file
	char addr[32] ;
	strncpy ( addr , clientIP.GetAsString(true) , 31 ) ;
	addr[31] = 0 ;
	if ( clientIP.IsLocalHost() ) strcpy ( addr , "localhost" ) ;

//	if ( !strcmp ( addr , "127.0.0.1:0" ) ) strcpy ( addr , "localhost" ) ;

	char idhash[PB_GUIDLEN+1] = "" ;
	CServerSlot *pSlot = m_pServer->GetPacketOwner( clientIP );
	if (pSlot)
	{
		PBcomputeHash ( idhash , pSlot ) ;
	} else {
		strcpy ( idhash , "" ) ;
	}
	PbSvAddClient ( addr , "" , idhash ) ;
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::OnDisconnectClient( CIPAddress &clientIP )
{
	char addr[32] ;
	strncpy ( addr , clientIP.GetAsString(true) , 31 ) ;
	addr[31] = 0 ;
	if ( !strcmp ( addr , "127.0.0.1:0" ) ) strcpy ( addr , "localhost" ) ;

	PbSvRemoveClient ( addr ) ;
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::CheaterFound( CIPAddress &clientIP,int type,const char *sMsg )
{
	if (m_pServer && m_pServer->GetSecuritySink())
	{
		m_pServer->GetSecuritySink()->CheaterFound( clientIP.GetAsUINT(),type,sMsg );
	}
}

bool CPunkBusterInterface::CheckPBPacket(CStream &stmPacket,CIPAddress &ip)
{
	int len = stmPacket.GetSize() >> 3 ;//convert bits to bytes
	if ( len >= 7) // Must be at least 7 bytes int.
	{
		BYTE *pBuf = stmPacket.GetPtr();
		if ( !memcmp ( pBuf , "\xff\xff\xff\xffPB_" , 7 ) ) {
//if not ded
			if ( pBuf[7] != 'S' && pBuf[7] != '2' && pBuf[7] != 'G' && pBuf[7] != 'I'
					&& pBuf[7] != 'Y' && pBuf[7] != 'B' && pBuf[7] != 'L' )
				 PbClAddEvent ( PB_EV_PACKET , len - 4 , (char *) pBuf + 4 ) ;
//
			if ( pBuf[7] != 'C' && pBuf[7] != '1' && pBuf[7] != 'J' )
				PbSvAddEvent ( PB_EV_PACKET , -1 , len - 4 , (char *) pBuf + 4 ) ;
			return false ;//yes pb packet
		}
	}
	return true;//not a pb packet
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::ValidateClient( CServerSlot *pSlot )
{
	//this function is called as client is trying to connect before level load
	//if PB is enabled at this server, it checks to see if the PB client is enabled for the player
	//   and checks for any ban issues with the id/hash
	unsigned int nFlags = pSlot->GetClientFlags();
	char idhash[PB_GUIDLEN+1] = "" , addr[32] = "" ;
	strncpy ( addr , pSlot->GetIP().GetAsString(true) , 31 ) ;
	addr[31] = 0 ;
	PBcomputeHash ( idhash , pSlot ) ;
	char *res = PbAuthClient ( addr , nFlags & CLIENT_FLAGS_PUNK_BUSTER , idhash ) ;
	if ( res != NULL ) pSlot->Disconnect ( res ) ;
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::OnCCPPunkBusterMsg( CIPAddress &ipAddress,CStream &stm )
{
	if ( m_pClient || m_pClientLocal )
	{
		BYTE *pByte = stm.GetPtr() ;
		PbClAddEvent ( PB_EV_PACKET , stm.GetSize() >> 3 , (char *) pByte ) ;//convert bits to bytes
	} 
	if ( m_pServer )
	{
		int i = 0 ;
		char *addr = ipAddress.GetAsString(true);
		for ( i = 0 ; i < PB_MAX_CLIENTS ; i++ ) if ( !strcmp ( addr , pbsdk->pbsv.m_client[i].pbc.ip ) ) break ;
		if ( i < PB_MAX_CLIENTS ) {
			BYTE *pByte = stm.GetPtr() ;
			PbSvAddEvent ( PB_EV_PACKET , i , stm.GetSize() >> 3 , (char *) pByte ) ;//convert bits to bytes
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::SendMsgToClient( CIPAddress &clientIP,CStream &stm )
{
	if (m_pServer)
	{
		CServerSlot *pSlot = m_pServer->GetPacketOwner(clientIP);
		if (pSlot)
		{
			pSlot->SendPunkBusterMsg(stm);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPunkBusterInterface::SendMsgToServer( CStream &stm )
{
	if (m_pClient)//don't check for local here because those packets are handled internally by PB
	{
		m_pClient->SendPunkBusterMsg( stm );
	}
}

//below added by TR as "glue" functions
//
// PBoutgame
//
void PBoutgame ( char *text , int hudAlso )
{
	if ( pbsdk == NULL ) return ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return ;

	char *te = text , hold = 0 ;
	for(;;) {								//loop in case of multiple lines (to simulate linefeeds)
		char *cp = strstr ( te , "\n" ) ;
		if ( cp != NULL ) {					//linefeed found so hold end-char and terminate
			hold = *cp ;
			*cp = 0 ;
		} else hold = 0 ;					//signifies end of line

		int wrapat = 89 ;
		if ( *te ) {						//ignore empty lines
			char *wrp = te ;
			while ( *wrp ) {
				char hold = 0 ;
				if ( strlen ( wrp ) > wrapat ) {
					hold = wrp[wrapat] ;
					wrp[wrapat] = 0 ;
				}
				pip->m_pSystem->GetIConsole()->PrintLine ( wrp ) ;	//output to console
				if ( !hold ) break ;
				wrp[wrapat] = hold ;
				wrp += wrapat ;
			}

			if ( hudAlso ) {									//optionally output to screen
				if ( !pip->m_pSystem->IsDedicated() ) {
					char buf[101] ;
					char finalbuf[201] ;
					
					strncpy( buf,te,sizeof(buf)-1 );
					buf[sizeof(buf)-1] = 0;
					for (int i = 0; i < strlen(buf); i++)
					{
						if (buf[i] == '\\')
							buf[i] = '/';
						if (buf[i] == '"')
							buf[i] = '\'';
					}
					_snprintf ( finalbuf , sizeof(finalbuf)-1 , "#if (Hud) then Hud:AddMessage( \"%s\" ); end" , buf ) ;
					pip->m_pSystem->GetIConsole()->ExecuteString( finalbuf,true,true );
				}
			}
		}

		if ( !hold ) break ;				//if end of line, then break out of loop
		*cp = hold ;
		te = cp + 1 ;
	}
}

//
// PBsendPktToServer
//
void PBsendPktToServer ( int datalen , char *data )
{
	if ( pbsdk == NULL ) return ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return ;

	CStream stm;
	stm.SetBits( (BYTE *) data , 0 , datalen * 8 );
	stm.SetSize( datalen * 8 ) ;
	pip->SendMsgToServer ( stm ) ;
}

//
// Sys_PBSendUdpPacket
//
void Sys_PBSendUdpPacket ( char *addr , unsigned short port , int datalen , char *data , int isFromClient )
{
	if ( pbsdk == NULL ) return ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return ;

	CStream stm;
	stm.SetBits( (BYTE *) data , 0 , datalen * 8 );
	stm.SetSize( datalen * 8 ) ;
	CIPAddress cip ( port , addr ) ;

	if ( isFromClient ) {	//from client
		if ( pip->m_pClient != NULL ) pip->m_pClient->SendTo ( cip , stm ) ;//no need to check for local due to PB's internal handling
	} else {							//from server
		if ( pip->m_pServer != NULL ) pip->m_pServer->Send ( stm , cip ) ;
	}
}

//
// PBsendPktToClient
//
void PBsendPktToClient ( int datalen , char *data , char *addr )
{
	if ( pbsdk == NULL ) return ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return ;

	CStream stm;
	stm.SetBits( (BYTE *) data , 0 , datalen * 8 );
	stm.SetSize( datalen * 8 ) ;
	char ip[32] ;
	unsigned short port = 0 ;
	strncpy ( ip , addr , 31 ) ;
	ip[31] = 0 ;
	char *cp = strstr ( ip , ":" ) ;
	if ( cp != NULL ) {
		*cp = 0 ;
		port = atoi ( cp + 1 ) ;
	}
	CIPAddress cip ( port , ip ) ;
	pip->SendMsgToClient ( cip , stm ) ;
}

//
// PBisLocalServer
//
int PBisLocalServer ( void )
{
	if ( pbsdk == NULL ) return 1 ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return 1 ;

	if ( pip->m_pClient == NULL ) return 1 ;
	CIPAddress ca = pip->m_pClient->GetServerIP() ;
	return ca.IsLocalHost() ;
}

//
// PBgetHomePath
//
void PBgetHomePath ( char *path , int maxlen )
{
	*path = 0 ;
	if ( pbsdk == NULL ) return ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return ;

	strncpy ( path , pip->fs_homepath->GetString() , maxlen - 1 ) ;
	path[maxlen-1] = 0 ;
}

//
// PBgetClientInfo
//
int PBgetClientInfo ( stPb_Sv_Client *c )
{
	if ( pbsdk == NULL ) return 0 ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return 0 ;

	if ( pip->m_pServer == NULL ) return 0 ;

	char ip[32] ;
	unsigned short port = 0 ;
	strncpy ( ip , c->ip , 31 ) ;
	ip[31] = 0 ;
	char *cp = strstr ( ip , ":" ) ;
	if ( cp != NULL ) {
		*cp = 0 ;
		port = atoi ( cp + 1 ) ;
	}
	CIPAddress clientIP ( port , ip ) ;

	IServerSecuritySink::SSlotInfo playerInfo;
	memset ( &playerInfo , 0 , sizeof ( playerInfo ) ) ;
	pip->m_pServer->GetSecuritySink()->GetSlotInfo( clientIP.GetAsUINT(),playerInfo , 1 );
	if ( *playerInfo.playerName == 0 ) return 0 ;		//player not set up yet

	strncpy ( c->name , playerInfo.playerName , PB_NAMELEN ) ;
	c->name[PB_NAMELEN] = 0 ;

	if ( *c->guid == 0 ) {													//if the guid is empty, compute it from the hash
		char idhash[PB_GUIDLEN+1] = "" ;
		CServerSlot *pSlot = pip->m_pServer->GetPacketOwner( clientIP );
		if (pSlot) {
			PBcomputeHash ( idhash , pSlot ) ;
			strcpy ( c->guid , idhash ) ;
		}
	}

	return 1 ;
}

//
// PBgetStats
//
int PBgetStats ( int index , char *Data )
{
	if ( pbsdk == NULL ) return 0 ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return 0 ;

	if ( pip->m_pServer == NULL ) return 0 ;

	char ip[32] ;
	unsigned short port = 0 ;
	strncpy ( ip , pbsdk->pbsv.m_client[index].pbc.ip , 31 ) ;
	ip[31] = 0 ;
	char *cp = strstr ( ip , ":" ) ;
	if ( cp != NULL ) {
		*cp = 0 ;
		port = atoi ( cp + 1 ) ;
	}
	CIPAddress clientIP ( port , ip ) ;

	IServerSecuritySink::SSlotInfo playerInfo;
	memset ( &playerInfo , 0 , sizeof ( playerInfo ) ) ;
	pip->m_pServer->GetSecuritySink()->GetSlotInfo( clientIP.GetAsUINT(),playerInfo , 0 ) ;
	if ( *playerInfo.playerName == 0 ) return 0 ;//player not set up yet

	sprintf ( Data , "score=%d deaths=%d" , playerInfo.score , playerInfo.deaths ) ;

	return 1 ;
}

//
// PBcomputeHash
//
void PBcomputeHash ( char *guid , CServerSlot *pSlot )
{
		MD5_CTX mc ;
		if (pSlot->m_pbGlobalID)
		{
			MD5Init ( &mc ) ;
			MD5Update ( &mc , pSlot->m_pbGlobalID , pSlot->m_uiGlobalIDSize );
			MD5Final ( &mc ) ;
		}
		else
		{
			memset(mc.digest,0,sizeof(mc.digest));
		}
		_snprintf ( guid , PB_GUIDLEN , "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x" ,
			mc.digest[0] , mc.digest[1] , mc.digest[2] , mc.digest[3] , mc.digest[4] , mc.digest[5] , mc.digest[6] ,
			mc.digest[7] , mc.digest[8] , mc.digest[9] , mc.digest[10] , mc.digest[11] , mc.digest[12] , mc.digest[13] ,
			mc.digest[14] , mc.digest[15] ) ;
}

//
// PBcvar_VariableString
//
char *PBcvar_VariableString ( const char *var_name )
{
	if ( pbsdk == NULL ) return "ERROR: NULL POINTER" ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return "ERROR: NULL POINTER" ;

	if ( pip->m_pSystem == NULL ) return "ERROR: NULL POINTER" ;

	ICVar *cv = pip->m_pSystem->GetIConsole()->GetCVar(var_name);
	if ( cv == NULL ) return "" ;

	return cv->GetString() ; 
}

//
// PBcvar_Set
//
void PBcvar_Set ( const char *cvar , const char *value )
{
	if ( pbsdk == NULL ) return ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return ;

	if ( pip->m_pSystem == NULL ) return ;

	ICVar *cv = pip->m_pSystem->GetIConsole()->GetCVar(cvar);
	if ( cv == NULL ) return ;

	cv->ForceSet ( value ) ;
}

//
// PBcmd_execString
//
void PBcmd_execString ( const char *text )
{
	if ( pbsdk == NULL ) return ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return ;

	if ( pip->m_pSystem == NULL ) return ;
	pip->m_pSystem->GetIConsole()->ExecuteString( text,false,false);
}

//
// PBdropClient
//
void PBdropClient ( int clientIndex , char *reason )
{
	if ( pbsdk == NULL ) return ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return ;

	if ( pip->m_pServer == NULL ) return ;

	char ip[32] ;
	unsigned short port = 0 ;
	strncpy ( ip , pbsdk->pbsv.m_client[clientIndex].pbc.ip , 31 ) ;
	ip[31] = 0 ;
	char *cp = strstr ( ip , ":" ) ;
	if ( cp != NULL ) {
		*cp = 0 ;
		port = atoi ( cp + 1 ) ;
	}
	CIPAddress clientIP ( port , ip ) ;

	CServerSlot *pSlot = pip->m_pServer->GetPacketOwner( clientIP );
	if (pSlot) pSlot->Disconnect ( reason ) ;	
}

//
// PBgameVer
//
char *PBgameVer ( void )
{
	if ( pbsdk == NULL ) return "" ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return "" ;

	//must return pointer to full game version info string
	static char buf[64] = "" ;
	pip->m_pSystem->GetFileVersion().ToString ( buf ) ;
	return buf ;
}

//
// isPBmultiplayerMode
//
int isPBmultiplayerMode ( void )
{
	if ( pbsdk == NULL ) return 0 ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return 0 ;

	if ( pip->m_pNetwork == NULL ) return 0 ;
	if ( pip->m_pServer == NULL && pip->m_pClient == NULL && pip->m_pClientLocal == NULL ) return 0 ;
	return 1 ;
}

//
// PBserverIp
//
char *PBserverIp ( int bServer )
{
	if ( pbsdk == NULL ) return "bot" ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return "bot" ;

	static CIPAddress cip ;

	if ( !bServer ) {
		if ( pip->m_pClient == NULL ) {
			if ( pip->m_pClientLocal == NULL ) return "bot" ;
			return "localhost" ;
		}
		cip = pip->m_pClient->GetServerIP() ;
	} else {
		if ( pip->m_pServer == NULL || pip->m_pNetwork == NULL ) return "???" ;
		cip.m_Address.ADDR = pip->m_pNetwork->GetLocalIP();
		cip.m_Address.sin_port = htons ( pip->m_pServer->GetServerPort() ) ;
	}
	return cip.GetAsString ( true ) ;
}

//
// PBserverHostname
//
char *PBserverHostname ( void )
{
	if ( pbsdk == NULL ) return "" ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return "" ;
	if ( pip->m_pServer == NULL ) return "" ;
	CIPAddress cip ;
	return (char *) pip->m_pServer->GetHostName() ;
}

//
// PBkeyValue
//
const char *PBkeyValue ( char *notused , char *key )
{
	notused;
	if ( pbsdk == NULL ) return "" ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return "" ;
	if ( pip->m_pSystem == NULL ) return "" ;

	const char *newkey = key ;
	ICVar *cv ;

	if ( !stricmp ( key , "gamename" ) ) {
#ifdef GAME_IS_FARCRY
			newkey = GetIXGame( pip->m_pSystem->GetIGame() )->IsMODLoaded() ;
#else
			newkey = NULL;
#endif
		if ( newkey == NULL ) return "FarCry" ;
		else return newkey ;
	} else if ( !stricmp ( key , "mapname" ) ) {
		newkey = "g_levelName" ;
	}/*	else if ( !stricmp ( s , "sv_hostname" ) ) {
	}*/

	cv = pip->m_pSystem->GetIConsole()->GetCVar( newkey ) ;
	if ( cv == NULL ) return "" ;

	return cv->GetString() ; 
}

//
// PBqueryGL
//
char *PBqueryGL ( int type )
{
	if ( pbsdk == NULL ) return "" ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return "" ;

//	pip->m_pSystem->GetIRenderer()->GetColorBpp() ;
	switch ( type ) {
	case PB_GL_READPIXELS: //(char *) qglReadPixels ;//function pointer
		return (char *) pip->m_pSystem->GetIRenderer()->EF_Query(EFQ_glReadPixels);
	case PB_GL_WIDTH: 
		return (char *) pip->m_pSystem->GetIRenderer()->GetWidth() ;
	case PB_GL_HEIGHT: 
		return (char *) pip->m_pSystem->GetIRenderer()->GetHeight() ;
	case PB_GL_RGB: return NULL ;//(char *) GL_RGB ;
	case PB_GL_UB: return NULL ;//(char *) GL_UNSIGNED_BYTE ;
	case PB_GL_D3DDEV: 
		return (char *) pip->m_pSystem->GetIRenderer()->EF_Query(EFQ_D3DDevice);
	}
	return NULL ;
}

class CCVarsDump : public ICVarDumpSink
{
public:
	CCVarsDump()
	{	
		m_vars.resize(0);
	}
	void OnElementFound(ICVar *pCVar)
	{
		m_vars.push_back(pCVar);
	}
	ICVar** GetVars() { return &m_vars[0]; }
	int NumVars() { return m_vars.size(); }
private:
	static std::vector<ICVar*> m_vars;
};
std::vector<ICVar*> CCVarsDump::m_vars;

//
// PBcvarWalk
//
//this is a callback function called from the PB dlls
int PBcvarWalk ( char **name , char **string , int *flags , char **resetString )
{
	if ( pbsdk == NULL ) return 0 ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return 0 ;

	static int i = 0 , n = 0 ;
	static ICVar **pVars = NULL ;

	if ( i == 0 || pVars == NULL ) {
		CCVarsDump dump;
		pip->m_pSystem->GetIConsole()->DumpCVars( &dump );
		pVars = dump.GetVars();
		if ( pVars == NULL ) return 0 ;
		i = 0 ;
		n = dump.NumVars() ;
	}

	if ( i >= n || pVars == NULL ) {//end of enumeration
		i = 0 ;
		n = 0 ;
		pVars = NULL ;
		return 0 ;
	}

	*name = (char *) pVars[i]->GetName() ;
	*string = (char *) pVars[i]->GetString() ;
	*flags = pVars[i]->GetFlags() ;
	*resetString = "" ;
	++i ;
	return 1;//keep going
}

class CKeyBindsDump : public IKeyBindDumpSink
{
public:
	CKeyBindsDump()
	{	
		m_keys.resize(0);
	}
	void OnKeyBindFound( const char *sBind,const char *sCommand )
	{
		m_keys.push_back(sBind);
	}
	const char** GetKeys() { return &m_keys[0]; }
	int NumKeys() { return m_keys.size(); }
private:
	static std::vector<const char*> m_keys;
};
std::vector<const char*> CKeyBindsDump::m_keys;

//
// PBbindStuff
//
int PBbindStuff ( int type , const char **data )
{
	if ( pbsdk == NULL ) return 0 ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return 0 ;

	static int i = 0 , n = 0 ;
	static const char **pKeys = NULL ;

	if ( type == 1 || n == 0 ) {
		n = 0 ;
		if ( i == 0 || pKeys == NULL ) {
			CKeyBindsDump dump;
			pip->m_pSystem->GetIConsole()->DumpKeyBinds( &dump );
			pKeys = dump.GetKeys();
			if ( pKeys == NULL ) return 0 ;
			i = 0 ;
			n = dump.NumKeys() ;
		}
	}

	if ( type == 1 ) {//return # of keys
		return n ;
	} else if ( type == 2 ) {//return keyname in *data
		int key = atoi ( *data ) ;
		*data = "" ;
		if ( pKeys == NULL ) return 0 ;
		if ( key < n ) *data = pKeys[key] ;
	} else {//return data for key bind name
		int key = atoi ( *data ) ;
		*data = "" ;
		if ( pKeys == NULL ) return 0 ;
		*data = pip->m_pSystem->GetIConsole()->FindKeyBind(pKeys[key]);
	}
	return 0 ;
}

//
// PBpakNames
//
void PBpakNames ( char *buf )//assumes buf is 1025+ in size (bytes)
{
	strcpy ( buf , "*ERROR*" ) ;
	if ( pbsdk == NULL ) return ;
	CPunkBusterInterface *pip = (CPunkBusterInterface *) pbsdk->pbinterface ;
	if ( pip == NULL ) return ;
	if ( pip->m_pSystem == NULL ) return ;

	*buf = 0 ;
	ICryPak::PakInfo* pPakInfo = pip->m_pSystem->GetIPak()->GetPakInfo();		//get loaded pak names
	int i ;
	char priorRoot[513] = "" ;
	for ( i = 0 ; i < pPakInfo->numOpenPaks ; i++ ) {
		int slb = strlen ( buf ) ;
		if ( stricmp ( priorRoot , pPakInfo->arrPaks[i].szBindRoot ) ) {			//new bind root ?
			strncpy ( priorRoot , pPakInfo->arrPaks[i].szBindRoot , 512 ) ;			//yes add to list with a * prefix
			priorRoot[512] = 0 ;
			if ( strlen ( priorRoot ) + slb < 1020 ) _snprintf ( buf + slb , 1025 - slb , "*\"%s\" " , priorRoot ) ;
			slb = strlen ( buf ) ;
		}
		const char *cp = pPakInfo->arrPaks[i].szFilePath ;
		if ( strlen ( cp ) > strlen ( priorRoot ) ) cp += strlen ( priorRoot ) ;//this should always happen
		if ( strlen ( cp ) + slb > 1020 ) continue ;													//skip it, too many already
		_snprintf ( buf + slb , 1025 - slb , "\"%s\" " , cp ) ;								//append pak file name from point of prior bind root
	}
}


#endif // NOT_USE_PUNKBUSTER_SDK