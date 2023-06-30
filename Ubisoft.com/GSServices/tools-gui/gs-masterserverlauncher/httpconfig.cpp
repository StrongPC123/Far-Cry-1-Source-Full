/***SDOC*******************************************************************************************
 *
 *                                UbiSoft Development Network
 *                                ---------------------------
 *
 * FILE........: httpconfig.cpp
 * CREATION....: 11 Dec. 2001
 * AUTHOR......: Guillaume Plante
 *
 * DESCRIPTION.: implementation of the CGSHttpConfig class
 *
 **************************************************************************************************
 *                                         FILE HISTORY
 **************************************************************************************************
 *
 * DATE........: 
 * AUTHOR......: 
 * DESCRIPTION.: 
 *
 ******************************************************************************************EDOC***/

#include "httpconfig.h"

//these 2 are included for the binary mode in case of binary download
#include <io.h>
#include <fcntl.h>
#include <winsock2.h>


//===================================================================================================
// CLASS		 CGSHttpConfig
// METHOD		 CGSHttpConfig
// AUTHOR:       Guillaume Plante
// CREATION:     April 4, 2002
//
// DESCRIPTION:  Default constructor
//===================================================================================================
CGSHttpConfig::CGSHttpConfig()
{
	m_iTimeOut = 0;
	m_bInitialised = false;
}

//===================================================================================================
// CLASS		 CGSHttpConfig
// METHOD		 PrintError
// AUTHOR:       Guillaume Plante
// CREATION:     April 4, 2002
//
// DESCRIPTION:  Print error code on stderr
//===================================================================================================
void CGSHttpConfig::PrintError(const char * szError)
{
	fprintf(stderr,"\nerror: %s\nwsalasterror: %d\n", szError, WSAGetLastError());
}

//===================================================================================================
// CLASS		 CGSHttpConfig
// METHOD		 Initialize
// AUTHOR:       Guillaume Plante
// CREATION:     April 4, 2002
//
// DESCRIPTION:  Initialize the class (winsock2) 
//===================================================================================================
bool CGSHttpConfig::Initialize()
{
	if(m_bInitialised)
		return false;

	// set the default time out value
	m_iTimeOut = _DEFAULT_TIMEOUT;
	m_bInitialised = true;
	return true;
}

//===================================================================================================
// CLASS		 CGSHttpConfig
// METHOD		 Uninitialize
// AUTHOR:       Guillaume Plante
// CREATION:     April 4, 2002
//
// DESCRIPTION:  Uninitialize the class (winsock2) 
//===================================================================================================
bool CGSHttpConfig::Uninitialize()
{
	if(!m_bInitialised)
		return false;
	m_bInitialised = false;
	return true;
}

//===================================================================================================
// CLASS		 CGSHttpConfig
// METHOD		 GetWebFile
// AUTHOR:       Guillaume Plante
// CREATION:     April 4, 2002
//
// DESCRIPTION:  Get the a file given a url and write it to a location
//===================================================================================================
int CGSHttpConfig::GetWebFile(const char * szURL,const char * szLocalFileName)
{	
	_urlinfo UrlInfo;
	if(!ParseURL(szURL,&UrlInfo))
		return -1;
	

	// Lookup host
	LPHOSTENT lpHostEntry;
	lpHostEntry = gethostbyname(UrlInfo.szHost);
	if (lpHostEntry == NULL)
	{
		PrintError("gethostbyname()");
		return -1;
	}

	// file object
	FILE *fData = NULL;
	// make sure the file is empty first
	if(szLocalFileName)
	{
		// write to file
		if(!(fData = fopen(szLocalFileName,"w")))
		{
			PrintError("Could not open local file");
			return -1;
		}
		// set the output as binary in case of a jpg or else
		_setmode(_fileno(fData), _O_BINARY);
		fwrite("", 1, 1, fData);
		fclose(fData);
	}
	// Fill in the server address structure
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	sa.sin_port = htons(UrlInfo.usPort);

	// Create a TCP/IP stream socket
	SOCKET	Socket;
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		PrintError("socket()"); 
		return -1;
	}

	//
	// Create an event object to be used with this socket
	//
	WSAEVENT hEvent;
	hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		PrintError("WSACreateEvent()");
		closesocket(Socket);
		return -1;
	}

	// Make the socket non-blocking and 
	// associate it with network event
	int nRet;
	nRet = WSAEventSelect(Socket,
						  hEvent,
						  FD_READ|FD_CONNECT|FD_CLOSE);
	if (nRet == SOCKET_ERROR)
	{
		PrintError("EventSelect()");
		closesocket(Socket);
		WSACloseEvent(hEvent);
		return -1;
	}

	// Request a connection
	nRet = connect(Socket, (LPSOCKADDR)&sa, sizeof(SOCKADDR_IN));

	if (nRet == SOCKET_ERROR)
	{
		nRet = WSAGetLastError();
		if (nRet == WSAEWOULDBLOCK)
		{
			fprintf(stderr,"\nConnect would block");
		}
		else
		{
			PrintError("connect()");
			closesocket(Socket);
			WSACloseEvent(hEvent);
			return -1;
		}
	}
	
	// Handle async network events
	char szBuffer[4096];
	WSANETWORKEVENTS events;
	while(1)
	{
		// Wait for something to happen
		//fprintf(stderr,"\nWaitForMultipleEvents()");
		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1,
									 &hEvent,
									 FALSE,
									 1000,
									 FALSE);
		
		if (dwRet == WSA_WAIT_TIMEOUT)
		{
			fprintf(stderr,"\nWait timed out");
			break;
		}

		// Figure out what happened
		//fprintf(stderr,"\nWSAEnumNetworkEvents()");
		nRet = WSAEnumNetworkEvents(Socket, hEvent,&events);
		if (nRet == SOCKET_ERROR)
		{
			PrintError("WSAEnumNetworkEvents()");
			break;
		}

		// handling events
		
		// Connect event
		if (events.lNetworkEvents & FD_CONNECT)
		{
			// Send the http request
			// make the request
			MakeRequest(szBuffer,UrlInfo.szObject,UrlInfo.szHost);
			nRet = send(Socket, szBuffer, strlen(szBuffer), 0);
			if (nRet == SOCKET_ERROR)
			{
				PrintError("send()");
				break;
			}
		}

		// Read event
		if (events.lNetworkEvents & FD_READ)
		{
			// Read the data and write it to stdout
			nRet = recv(Socket, szBuffer, sizeof(szBuffer), 0);
			if (nRet == SOCKET_ERROR)
			{
				PrintError("recv()");
				break;
			}
			//fprintf(stderr,"\nRead %d bytes", nRet);

			if(szLocalFileName)
			{
				// write to file
				if(!(fData = fopen(szLocalFileName,"a+")))
				{
					PrintError("Could not open local file");
					return -1;
				}
				// set the output as binary in case of a jpg or else
				_setmode(_fileno(fData), _O_BINARY);
				fwrite(szBuffer, nRet, 1, fData);
				fclose(fData);
			}
			else
			{
				// set the output as binary in case of a jpg or else
				_setmode(_fileno(stdout), _O_BINARY);				
				// Write to stdout
				fwrite(szBuffer, nRet, 1, stdout);
			}
		}

		// Close event
		if (events.lNetworkEvents & FD_CLOSE)
			break;
	}
	closesocket(Socket);	
	WSACloseEvent(hEvent);
	return 1;
}

//===================================================================================================
// CLASS		 CGSHttpConfig
// METHOD		 MakeRequest
// AUTHOR:       Guillaume Plante
// CREATION:     April 4, 2002
//
// DESCRIPTION:  Make a standard http "GET" request based on a absolute file path and hostname
//===================================================================================================
char *CGSHttpConfig::MakeRequest(char * szBuffer,const char * szFile,const char * szHostname)
{
	sprintf(szBuffer, "GET %s HTTP/1.1\nHost: %s\n\n", szFile,szHostname);
	return szBuffer;
}

//===================================================================================================
// CLASS		 CGSHttpConfig
// METHOD		 ParseURL
// AUTHOR:       Guillaume Plante
// CREATION:     April 4, 2002
//
// DESCRIPTION:  Parse a URL to give back the server, object, service name and port number in a struct
//===================================================================================================
bool CGSHttpConfig::ParseURL(const char *szURL,_urlinfo *UrlInfo)
{
	char szTempURL[256];
	char *pc = NULL;
	char *pcPort = NULL;
	char *pcObj = NULL;
	int cpt = 0;
	int len = 0;

	memset(UrlInfo->szHost,0,sizeof(UrlInfo->szHost));
	memset(UrlInfo->szService,0,sizeof(UrlInfo->szService));
	memset(UrlInfo->szObject,0,sizeof(UrlInfo->szObject));
	UrlInfo->usPort = 0;

	// make a copy of the url
	strncpy(szTempURL,szURL,255);

	if(!strstr(szTempURL,HTTP_SEP_PROTO_HOST))
		return false;

	// first find the service type
	pc = strtok(szTempURL,HTTP_SEP_PROTO_HOST);
	if(!pc)
		return false;
	// put service in uppercase
	len = strlen(pc);
	for(cpt = 0;cpt<=len;cpt++)
	{
		pc[cpt] = tolower(pc[cpt]);
	}
	// copy the service name
	strncpy(UrlInfo->szService,pc,sizeof(UrlInfo->szService));

	// check for invalid service.
	if((strcmp(UrlInfo->szService,NET_SERVICE_HTTP))&&(strcmp(UrlInfo->szService,NET_SERVICE_FTP))&&
		(strcmp(UrlInfo->szService,NET_SERVICE_GOPHER)))
		return false;

	// make a copy of the url
	strncpy(szTempURL,szURL,255);
    // find the port is there is none, port is 80 (standard)
	pc = strchr(szTempURL,':');
	if(pc)
	{
		if(pc = strtok(++pc,HTTP_SEP_SLASH))
			pcPort = strrchr(pc,HTTP_SEP_COLON_INT);
		else
			return false;  // could not determine hostname
		if(pcPort)
		{
			UrlInfo->usPort = atoi(++pcPort);
			if(pc = strtok(pc,HTTP_SEP_COLON))
				strncpy(UrlInfo->szHost,pc,sizeof(UrlInfo->szHost));
			else
				return false; // could not find hostname
		}
		else
		{
			strncpy(UrlInfo->szHost,pc,sizeof(UrlInfo->szHost));
			UrlInfo->usPort = HTTP_DEFAULT_PORT; // well known http port
		}
	}
	else
		return false;
	// in case of badly formed url put the default http port
	if(!UrlInfo->usPort)
		UrlInfo->usPort = HTTP_DEFAULT_PORT;
	// make a copy of the url
	strncpy(szTempURL,szURL,255);
	// now find the object
	pcObj = strstr(szTempURL,UrlInfo->szHost);
	if(pcObj)
	{
		if(pcObj = strchr(pcObj+cpt,HTTP_SEP_SLASH_INT))
			strncpy(UrlInfo->szObject,pcObj,sizeof(UrlInfo->szObject));
		else
			strncpy(UrlInfo->szObject,"/",sizeof(UrlInfo->szObject));
	}
	else
		return false;
	return true;
}