#include "stdafx.h"

#ifndef NOT_USE_UBICOM_SDK


#include "NewUbisoftClient.h"
#include "CommonDefines.h"						// UBI.com common defines
#include "IConsole.h"									// ICVar
#include "IRenderer.h"								// IRenderer

#if defined(LINUX)
	#include "CryLibrary.h"
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fstream>
#endif	

#if defined(WIN32) || defined(WIN64)
	#include "windows.h"
	#include "Wininet.h"
#endif

#include "ScriptObjectNewUbisoftClient.h"		// CScriptObjectNewUbisoftClient
#if !defined(LINUX)
#include <assert.h>
#endif


#include "GSCDKeyInterface.h"
#include "InitSockets.h"

#if !defined(LINUX)
	static const char GSINIFILE[10] = ".\\gs.ini";
	static const char GSINIFILETMP[10] = ".\\gs.tmp";
#else
	static const char GSINIFILE[10] = "gs.ini";
	static const char GSINIFILETMP[10] = "gs.tmp";
#endif

static const char RegistryKeyName[] = "SOFTWARE\\Crytek\\FarCry";



// src and trg can be the same pointer (in place encryption)
// len must be in bytes and must be multiple of 8 byts (64bits).
// key is 128bit:  int key[4] = {n1,n2,n3,n4};
// void encipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k )
#define TEA_ENCODE( src,trg,len,key ) {\
	register unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; \
	register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; \
	while (nlen--) {\
	register unsigned int y=v[0],z=v[1],n=32,sum=0; \
	while(n-->0) { sum += delta; y += (z << 4)+a ^ z+sum ^ (z >> 5)+b; z += (y << 4)+c ^ y+sum ^ (y >> 5)+d; } \
	w[0]=y; w[1]=z; v+=2,w+=2; }}

// src and trg can be the same pointer (in place decryption)
// len must be in bytes and must be multiple of 8 byts (64bits).
// key is 128bit: int key[4] = {n1,n2,n3,n4};
// void decipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k)
#define TEA_DECODE( src,trg,len,key ) {\
	register unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; \
	register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; \
	while (nlen--) { \
	register unsigned int y=v[0],z=v[1],sum=0xC6EF3720,n=32; \
	while(n-->0) { z -= (y << 4)+c ^ y+sum ^ (y >> 5)+d; y -= (z << 4)+a ^ z+sum ^ (z >> 5)+b; sum -= delta; } \
	w[0]=y; w[1]=z; v+=2,w+=2; }}



NewUbisoftClient::NewUbisoftClient( const char *szLocalIPAddress ):m_strUsername(""), m_iJoinedLobbyID(0), m_iJoinedRoomID(0),
		m_bDownloadedGSini(false), m_eServerState(NoUbiServer), m_eClientState(NoUbiClient), m_hCDKey(0),
		m_pCDKeyServer(NULL), m_bCheckCDKeys(false), m_dwNextServerAbsTime(0),
		m_dwNextClientAbsTime(0), m_dwAccountCreateTime(0), m_bDisconnecting(0), sv_authport(0), sv_regserver_port(0),
		m_pLog(0), m_pSystem(0), m_usGamePort(0)
{
	assert(szLocalIPAddress);

	m_pScriptObject=0;

	GSbool bRet;

	if(strcmp(szLocalIPAddress,"0.0.0.0")!=0)
	{
		static char szLocalIP[256];

		strcpy(szLocalIP,(char *)szLocalIPAddress);				// make a copy (to compensate UBI sdk error)
		bRet=InitializeSockets((GSchar *)szLocalIP);
	}
	else bRet=InitializeSockets();


/*
	if(!bRet)
	{
	// log error
	}
*/

	m_bSavePassword = false;
}

#if defined(LINUX)
static void	ResolveServerAndPath( const char* szURL, string& strServer, string& strPath )
{
	const char c_szHTTPSig[] =  "http://";
	const size_t c_HTTPSigLength( sizeof( c_szHTTPSig ) - 1 ); // substract zero termination

	string strTemp( szURL );
	if( string::npos == strTemp.find( c_szHTTPSig ) )
	{
		string::size_type posFirstSlash( strTemp.find( "/" ) );
		if(posFirstSlash == string::npos)
		{
			printf("Cannot resolve server path URL for downloading gs.ini\n");
			return;
		}
		strServer = strTemp.substr( 0, posFirstSlash );
		strPath = strTemp.substr( posFirstSlash, strTemp.size() -  posFirstSlash );
	}
	else
	{
		string::size_type posFirstSlash( strTemp.find( "/", c_HTTPSigLength ) );
		if(posFirstSlash == string::npos)
		{
			printf("Cannot resolve server path URL for downloading gs.ini\n");
			return;
		}
		strServer = strTemp.substr( c_HTTPSigLength, posFirstSlash - c_HTTPSigLength );
		strPath = strTemp.substr( posFirstSlash , strTemp.size() -  posFirstSlash  );
	}
}



static bool SendReceive( SOCKET& conSocket, const char* szRequestFmt, const string& strServer, const string& strPath, string& strData )
{
	// send request
	std::vector< char > request;
	request.reserve( strServer.size() + strPath.size() + strlen( szRequestFmt ) + 1 );
	sprintf( &request[ 0 ], szRequestFmt, strPath.c_str(), strServer.c_str() );

	if( SOCKET_ERROR == send( conSocket, &request[ 0 ], (int) strlen( &request[ 0 ] ) + 1, 0 ) )
	{
		return( false );
	}

	// receive data
	strData.clear();
	while( true )
	{
		const int c_bufferSize( 128 );
		char buffer[ c_bufferSize + 1 ];
		buffer[ 0 ] = 0;

		int retval( recv( conSocket, buffer, c_bufferSize, 0 ) );
		if( SOCKET_ERROR == retval )
		{
			return( false );
		}

		if( 0 < retval )
		{
			buffer[ retval ] = 0;
			strData += buffer;
		}
		else
		{
			break;
		}
	}

	return( true );
}



bool GetTextFromURL( const char* szURL, string& strText  )
{
	// determine server and path from URL
	string strServer;
	string strPath;

	ResolveServerAndPath( szURL, strServer, strPath );
	//  create host structure
	hostent* pHost( 0 );
	if( false != isalpha( strServer[ 0 ] ) )
	{
		// resolve host name
		pHost = gethostbyname( strServer.c_str() );
	}
	else
	{
		// convert string to ip address number
		unsigned int addr( inet_addr( strServer.c_str() ) );
		pHost = gethostbyaddr( (char*) &addr, 4, AF_INET );
	}

	if( 0 == pHost )
	{
		return( false );
	}

	// create socket
	sockaddr_in server;
	memset( &server, 0, sizeof( server ) );
	memcpy( &(server.sin_addr), pHost->h_addr, pHost->h_length );
	server.sin_family = pHost->h_addrtype;
	server.sin_port = htons( 80 );

	SOCKET conSocket( socket( AF_INET, SOCK_STREAM, 0 ) );
	if( 0 > conSocket )
	{
		return( false );
	}

	// connect
	if( SOCKET_ERROR == connect( conSocket, (sockaddr*) &server, sizeof( server ) ) )
	{
		return( false );
	}

	// send request
	//const char pcRequest[] = "GET /gsinit.php?user=%25s&dp=%25s HTTP/1.0\nUser-Agent: Wget/1.9\nHost: gsconnect.ubisoft.com\nAccept: */*\nConnection: Keep-Alive\n\n";
	//const char pcRequest[] = "GET /gsinit.php?user=%s&dp=%s HTTP/1.0\nHost: gsconnect.ubisoft.com\n\n";
	const char c_szRequestFmt[] = "GET %s HTTP/1.0\nHost: %s\n\n";
	if( false != SendReceive( conSocket, c_szRequestFmt, strServer, strPath, strText ) )
	{
		const char c_szHTTPHeaderEnd[ ] = "\r\n\r\n\r\n";
		const size_t c_szHTTPHeaderEndLength( sizeof( c_szHTTPHeaderEnd ) - 1 ); // substract zero termination

		// remove http header
		string::size_type pos( strText.find( c_szHTTPHeaderEnd ) );
		if( string::npos != pos )
		{
			strText = strText.substr( pos + c_szHTTPHeaderEndLength, strText.size() - pos - c_szHTTPHeaderEndLength );
		}
	}
	else
	{
		return( false );
	}

	// close socket and return
	closesocket( conSocket );

	return( true );
}
#endif


NewUbisoftClient::~NewUbisoftClient()
{
	Server_RemoveAllPlayers();

#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	if (m_eClientState != NoUbiClient)
		clMSClientClass::Uninitialize();
#endif // EXCLUDE_UBICOM_CLIENT_SDK

	if (m_pCDKeyServer)
		delete m_pCDKeyServer;

	GSCDKey_Uninitialize();
	UninitializeSockets();
}

bool NewUbisoftClient::WriteStringToRegistry(const string &szKeyName, const string &szValueName, const string &szValue)
{
#if !defined(LINUX)
	HKEY hKey;

	string szREGKeyName = string(RegistryKeyName) + string("\\") + szKeyName;

	if (RegOpenKey(HKEY_LOCAL_MACHINE, szREGKeyName.c_str(), &hKey) != ERROR_SUCCESS)
	{
		if (RegCreateKey(HKEY_LOCAL_MACHINE, szREGKeyName.c_str(), &hKey) != ERROR_SUCCESS)
		{
			return false;
		}
	}

	if (RegSetValueEx(hKey, szValueName.c_str(), 0, REG_SZ, (BYTE *)szValue.c_str(), szValue.size()) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return false;
	}
	RegCloseKey(hKey);
#endif
	return true;
}

bool NewUbisoftClient::ReadStringFromRegistry(const string &szKeyName, const string &szValueName, string &szValue)
{
#if !defined(LINUX)
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
#endif
	return true;
}

bool NewUbisoftClient::RemoveStringFromRegistry(const string &szKeyName, const string &szValueName)
{
#if !defined(LINUX)
	HKEY hKey;

	string szREGKeyName = string(RegistryKeyName) + string("\\") + szKeyName;

	if (RegOpenKey(HKEY_LOCAL_MACHINE, szREGKeyName.c_str(), &hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD dwSize = 0;



	if (RegDeleteValue(hKey, szValueName.c_str()) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);

		return false;
	}

	RegCloseKey(hKey);
#endif
	return true;
}

bool NewUbisoftClient::IsValueOnRegistry(const string &szKeyName, const string &szValueName)
{
#if !defined(LINUX)
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
	RegCloseKey(hKey);
#endif
	return true;
}

bool NewUbisoftClient::EncryptString(unsigned char *szOut, const unsigned char *szIn)
{
	string szInPadded = (char *)szIn;

	while ((szInPadded.size() % 8) != 0)
	{
		szInPadded.push_back(0);
	}

	unsigned int Key[4] = {31337, 31337*2, 31337*4, 31337*8};

	TEA_ENCODE((unsigned int *)szInPadded.c_str(), (unsigned int *)szOut, szInPadded.size(), Key);
	szOut[szInPadded.size()] = 0;

	return true;
}

bool NewUbisoftClient::DecryptString(unsigned char *szOut, const unsigned char *szIn)
{
	string szInPadded = (char *)szIn;

	while ((szInPadded.size() % 8) != 0)
	{
		szInPadded.push_back(0);
	}

	unsigned int Key[4] = {31337, 31337*2, 31337*4, 31337*8};
  
	TEA_DECODE((unsigned int *)szIn, (unsigned int *)szOut, strlen((char *)szIn), Key);

	return true;
}

bool NewUbisoftClient::EncodeHex(unsigned char *szOut, const unsigned char *szIn)
{
	unsigned int len = strlen((char *)szIn);

	for (unsigned int i = 0; i < len; i++)
	{
		sprintf((char *)&szOut[i*2], "%02x", szIn[i]);
	}

	return true;
}

bool NewUbisoftClient::DecodeHex(unsigned char *szOut, const unsigned char *szIn)
{
	unsigned int len = strlen((char *)szIn) >> 1;
	char szAux[16];

	for (unsigned int i = 0; i < len; i++)
	{
		sprintf(szAux, "0x%c%c", szIn[i*2+0], szIn[i*2+1]);
		szOut[i] = strtol(szAux, 0, 0);
	}

	return true;
}

void NewUbisoftClient::Init( ISystem *inpSystem )
{
	m_pSystem = inpSystem;											assert(m_pSystem);
	m_pLog = m_pSystem->GetILog();							assert(m_pLog);

	IConsole *pConsole = m_pSystem->GetIConsole();

	sv_authport = pConsole->GetCVar("sv_authport");									assert(sv_authport);
	sv_regserver_port = pConsole->GetCVar("sv_regserver_port");			assert(sv_regserver_port);
}


bool NewUbisoftClient::Update()
{
#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	clMSClientClass::Engine();
#endif // EXCLUDE_UBICOM_CLIENT_SDK

	CRegisterServer::RegServer_Engine();

	//if (m_bUsingRegServer && !m_bServerLoggedIn)
		//Server_RecreateServer();

	{
		bool bCheckCDKey = m_pSystem->GetIGame()->GetModuleState(EGameMultiplayer)
										&& m_pSystem->GetINetwork() 
										&& m_pSystem->GetINetwork()->GetServerByPort(m_usGamePort) 
										&& m_pSystem->GetINetwork()->GetServerByPort(m_usGamePort)->GetServerType()!=eMPST_LAN;

		Server_CheckCDKeys(bCheckCDKey);
	}

	if (m_hCDKey)
		GSCDKey_Engine(m_hCDKey);

	// ensure server is shown in the UBI.com list
	// Server_CreateServer() has a timelimit

	if (m_eServerState == ServerDisconnected)
		Server_RecreateServer();

	if ((m_eClientState == ClientDisconnected) || (m_eClientState == GameServerDisconnected))
		Client_ReJoinGameServer();

#ifndef EXCLUDE_UBICOM_CLIENT_SDK
	if (m_eClientState == NoUbiClient)
		clMSClientClass::Uninitialize();
#endif // EXCLUDE_UBICOM_CLIENT_SDK

	return true;
}

void NewUbisoftClient::SetScriptObject( CScriptObjectNewUbisoftClient *inpObject )
{
	assert(!m_pScriptObject);				// called twice?
	assert(inpObject);							// param must be 0

	m_pScriptObject = inpObject;
}


bool NewUbisoftClient::DownloadGSini(const char *szUsername)
{
	if (m_bDownloadedGSini)
		return true;

#if defined(LINUX)
	static const char* pGSConnectFilename = "gsconnect.ini";
#endif

	// Get the URL to download the INI from
	// Get it from the reg key
	// HKEY_LOCAL_MACHINE\SOFTWARE\Ubi Soft\Game Service\ConnectURL
	// if we can, otherwise use the default URL
	// http://gsconnect.ubisoft.com/gsinit.php?user=%s&dp=%s
	// If that doesn't work check the local directory
	char *connectURL = NULL;
	DWORD dwBufLen = 0;
#if !defined(LINUX)
	//connectURL[0]=0;
	HKEY hKey;
	if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,"SOFTWARE\\Ubi Soft\\Game Service",0, KEY_QUERY_VALUE, &hKey ) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx( hKey, "ConnectURL", NULL, NULL, NULL, &dwBufLen) == ERROR_SUCCESS)
		{
			connectURL = (char*)malloc(dwBufLen);
			RegQueryValueEx( hKey, "ConnectURL", NULL, NULL, (LPBYTE) connectURL, &dwBufLen);
			RegCloseKey( hKey );
		}
	}
#else
	char iniEntry[512];
	//Check to see if the tmp file contains good info
	string GSConnectFilename(GetModulePath());
	if(GSConnectFilename.c_str()[GSConnectFilename.size()-1] != '/')
		GSConnectFilename += "/";
	GSConnectFilename += pGSConnectFilename;
	GetPrivateProfileString("Servers", "GSConnectURL", "Key not found", iniEntry, 512, GSConnectFilename.c_str());
	if (strcmp("Key not found", iniEntry) !=0 )
	{
		dwBufLen = strlen(iniEntry);
		connectURL = (char*)malloc(dwBufLen+1);
		strcpy(connectURL, iniEntry);
	}
#endif
	if (connectURL==NULL) // We didn't get the key from the registry so try the default url
	{
		char defURL[] = "http://gsconnect.ubisoft.com/gsinit.php?user=%s&dp=%s";
		dwBufLen = sizeof(defURL);
		connectURL = (char*)malloc(dwBufLen + 1);
		strcpy(connectURL, defURL);
	}

	const unsigned int cMaxCount = dwBufLen + strlen(szUsername) + strlen(GAME_NAME) + 1;
	char *szGSURL = (char*)malloc(cMaxCount); //size of the url + username + gamename

	_snprintf(szGSURL,cMaxCount - 1, connectURL, szUsername, GAME_NAME);
	free(connectURL);


	/*GShandle stHandle = GSHttpInitialize();
	// Returns 0 on failure, but for now I don't care
	int i=GSHttpSave(stHandle,connectURL,"gsinit.ini",GS_TRUE,NULL,NULL); // Store in the working dir
	assert(i);
	GSHttpUninitialize();*/

	//delete the current tmp file
	remove(GSINIFILETMP);


#if !defined(LINUX)
	HINTERNET hNet = InternetOpen("",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,NULL);
	if (!hNet)
	{
		free(szGSURL);
		return false;
	}
	else
	{
		HINTERNET hURL = InternetOpenUrl(hNet,szGSURL,NULL,0,INTERNET_FLAG_HYPERLINK,NULL);
		free(szGSURL);
		if (!hURL)
			return false;
		else
		{
			GSchar szBuffer[1024];
			GSint iSize = 1024;
			DWORD iRead = 0;

			FILE *pFile = fopen(GSINIFILETMP,"w"); 

			// If we can't open the tmp file return true and we will use the real one.
			if (!pFile)
			{
				InternetCloseHandle(hURL);
				return true;
			}

			while (InternetReadFile(hURL,szBuffer,iSize,&iRead))
			{
				if (iRead != 0)
				{
					fwrite(szBuffer,sizeof(GSchar),iRead,pFile);
				}
				else
				{
					fclose(pFile);
					break;
				} 
			}
		}
		InternetCloseHandle(hURL);
	} 
#else
	string strText;
	GetTextFromURL(szGSURL, strText);
	free(szGSURL);
	// write file
	FILE *pFile = fopen(GSINIFILETMP,"wb"); 
	if(NULL != pFile)
	{
		fwrite(strText.c_str(), 1, strText.size(), pFile);
		fclose(pFile);
	}
#endif  
	GSchar szIPAddress[100]; 
	//Check to see if the tmp file contains good info
	GetPrivateProfileString("Servers", "RouterIP0", "Key not found", szIPAddress, 100, GSINIFILETMP);
	if (strcmp("Key not found", szIPAddress)!=0)
	{
		//It contains good info so replace the real gs.ini file
		remove(GSINIFILE);
		rename(GSINIFILETMP,GSINIFILE);
	}

	m_bDownloadedGSini = true;

	return true;
}


// Script Callbacks -------------------------------------------

void NewUbisoftClient::Client_LoginSuccess(const char *szUsername)
{
	if (m_bSavePassword)
	{
		char szEncUsername[256] = {0};
		char szEncPassword[256] = {0};
		char szHexUsername[512] = {0};
		char szHexPassword[512] = {0};

		EncryptString((unsigned char *)szEncUsername, (unsigned char *)m_strUsername.c_str());
		EncryptString((unsigned char *)szEncPassword, (unsigned char *)m_strPassword.c_str());

		EncodeHex((unsigned char *)szHexUsername, (unsigned char *)szEncUsername);
		EncodeHex((unsigned char *)szHexPassword, (unsigned char *)szEncPassword);

		WriteStringToRegistry("Ubi.com", "username", szHexUsername);
		WriteStringToRegistry("Ubi.com", "password", szHexPassword);
	}
	else
	{
		RemoveStringFromRegistry("Ubi.com", "username");
		RemoveStringFromRegistry("Ubi.com", "password");
	}

	m_pScriptObject->Client_LoginSuccess(szUsername);
}
void NewUbisoftClient::Client_LoginFail(const char *szText)
{
	m_pScriptObject->Client_LoginFail(szText);
}
void NewUbisoftClient::Client_GameServer(int iLobbyID, int iRoomID, const char *szServerName, const char *szIPAddress,
	const char *szLANIPAddress, int iMaxPlayers, int iNumPlayers)
{
	m_pScriptObject->Client_GameServer(iLobbyID,iRoomID,szServerName,szIPAddress,szLANIPAddress,iMaxPlayers,iNumPlayers);
}
void NewUbisoftClient::Client_RequestFinished()
{
	m_pScriptObject->Client_RequestFinished();
}
void NewUbisoftClient::Client_JoinGameServerSuccess(const char *szIPAddress, const char *szLanIPAddress,unsigned short usPort)
{
	m_pScriptObject->Client_JoinGameServerSuccess(szIPAddress,szLanIPAddress,usPort);
}
void NewUbisoftClient::Client_JoinGameServerFail(const char *szText)
{
	m_pScriptObject->Client_JoinGameServerFail(szText);
}
void NewUbisoftClient::Client_CreateAccountSuccess()
{
	m_pScriptObject->Client_CreateAccountSuccess();
}
void NewUbisoftClient::Client_CreateAccountFail(const char *szText)
{
	m_pScriptObject->Client_CreateAccountFail(szText);
}
void NewUbisoftClient::Server_RegisterServerSuccess(GSint iLobbyID, GSint iRoomID)
{
	m_pScriptObject->Server_RegisterServerSuccess(iLobbyID,iRoomID);
}
void NewUbisoftClient::Server_RegisterServerFail()
{
	m_pScriptObject->Server_RegisterServerFail();
}
void NewUbisoftClient::Server_LobbyServerDisconnected()
{
	m_pScriptObject->Server_LobbyServerDisconnected();
}
void NewUbisoftClient::Server_PlayerJoin(const char *szUsername)
{
	m_pScriptObject->Server_PlayerJoin(szUsername);
}
void NewUbisoftClient::Server_PlayerLeave(const char *szUsername)
{
	m_pScriptObject->Server_PlayerLeave(szUsername);
}

void NewUbisoftClient::CDKey_Failed(const char *szText)
{
	m_pScriptObject->CDKey_Failed(szText);
}

void NewUbisoftClient::CDKey_GetCDKey()
{
	m_pScriptObject->CDKey_GetCDKey();
}

void NewUbisoftClient::CDKey_ActivationSuccess()
{
	m_pScriptObject->CDKey_ActivationSuccess();
}

void NewUbisoftClient::CDKey_ActivationFail(const char *szText)
{
	if(m_pSystem->GetIGame()->GetModuleState(EGameMultiplayer))
		m_pSystem->GetIRenderer()->ClearColorBuffer(Vec3(0,0,0));

	m_pSystem->GetIConsole()->ResetProgressBar(0);
	m_pSystem->GetIConsole()->ShowConsole(false);
	m_pSystem->GetIConsole()->SetScrollMax(600/2);

	m_pScriptObject->CDKey_ActivationFail(szText);
}
// --------------------------------------------------------------




bool NewUbisoftClient::GetRouterAddress(int iIndex, char *szIPAddress, unsigned short *pusClientPort,
									   unsigned short *pusRegServerPort)
{
	char szKey[50];
	char szPort[50];
 
	// Try to read from the tmp file first
	_snprintf(szKey, 50, "RouterIP%i", iIndex);
	GetPrivateProfileString("Servers", szKey,"Key not found", szIPAddress, 50, GSINIFILE); // Read from the working directory
	if (strcmp("Key not found", szIPAddress)==0)
		return false;

	_snprintf(szKey, 50, "RouterPort%i", iIndex);
	GetPrivateProfileString("Servers", szKey,"Key not found", szPort, 50, GSINIFILE); // Read from the working directory
	if (strcmp("Key not found", szPort)==0)
		return false;
	*pusClientPort = atoi(szPort);

	_snprintf(szKey, 50, "RouterLauncherPort%i", iIndex);
	GetPrivateProfileString("Servers", szKey,"Key not found", szPort, 50, GSINIFILE); // Read from the working directory
	if (strcmp("Key not found", szPort)==0)
		return false;
	*pusRegServerPort = atoi(szPort);
	return true;
}

bool NewUbisoftClient::GetCDKeyServerAddress(int iIndex, char *szIPAddress, unsigned short *pusPort)
{
	char szKey[50];
	char szPort[50];
	// Try to read from the tmp file first
	_snprintf(szKey, 50, "CDKeyServerIP%i", iIndex);
	GetPrivateProfileString("Servers", szKey,"Key not found", szIPAddress, 50, GSINIFILE); // Read from the working directory
	if (strcmp("Key not found", szIPAddress)==0)
		return false;
	_snprintf(szKey, 50, "CDKeyServerPort%i", iIndex);
	GetPrivateProfileString("Servers", szKey,"Key not found", szPort, 50, GSINIFILE); // Read from the working directory
	if (strcmp("Key not found", szPort)==0)
		return false;
	*pusPort = atoi(szPort);
	return true;
}

#endif // NOT_USE_UBICOM_SDK
