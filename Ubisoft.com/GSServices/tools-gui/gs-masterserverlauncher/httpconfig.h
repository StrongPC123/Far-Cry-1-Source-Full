/***SDOC*******************************************************************************************
 *
 *                                UbiSoft Development Network
 *                                ---------------------------
 *
 * FILE........: httpconfig.h
 * CREATION....: 11 Dec. 2001
 * AUTHOR......: Guillaume Plante
 *
 * DESCRIPTION.: wrapper class around winsock to get a file on the net via the http protocol,
 *			     using event objects and WSASelectEvent() 
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


#ifndef _HTTPCONFIG_H_
#define _HTTPCONFIG_H_

#include <stdio.h>

#define		_DEFAULT_TIMEOUT		10

#define		NET_SERVICE_HTTP		"http"
#define		NET_SERVICE_FTP			"ftp"
#define		NET_SERVICE_GOPHER		"gopher"

#define		HTTP_DEFAULT_PORT		80
#define		HTTP_SEP_COLON			":"
#define		HTTP_SEP_COLON_INT		':'
#define		HTTP_SEP_SLASH			"/"
#define		HTTP_SEP_SLASH_INT		'/'
#define		HTTP_SEP_PROTO_HOST		"://"


typedef struct
{
	char szService[12];
	char szHost[128];
	char szObject[128];
	unsigned short int usPort;
} _urlinfo;

class CGSHttpConfig
{
	public:
		CGSHttpConfig();		// constructor
		~CGSHttpConfig(){};		// destructor
		bool Initialize();		// Initialize winsock.
		bool Uninitialize();	// clean up the place.
								// set the time out for a http query
		void SetTimeOut(int iTimeout){m_iTimeOut = iTimeout;};
								// get the timeout in second for a query
		int  GetTimeOut(){return m_iTimeOut;};
								// get the file and write it into a buffer 
								// (stdout if szFileBuf is NULL)
		int  GetWebFile(const char *szURL,const char *szFileBuf = NULL);
	private:
		bool ParseURL(const char *szURL,_urlinfo *UrlInfo);
		// create a http GET request
		char *MakeRequest(char *szBuffer,const char *szFile,const char *szHost);
		void PrintError(const char *szError);// print the last error encountered
		bool m_bInitialised;		// initialisation flag
		int m_iTimeOut;			// Time out value in second for a query
};

#endif //_HTTPCONFIG_H_