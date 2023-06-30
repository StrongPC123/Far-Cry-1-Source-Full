

#ifndef __CONFIGFILEPARSER_H__
#define __CONFIGFILEPARSER_H__

#include "define.h"
#include "GSTypes.h"

//////////////////////////////////////////////////
// Referrer string used in the config file parsing

// SERVER SECTION NAME
#define		SERVER_SECTION_NAME			"Servers"

// ROUTER KEYNAME
#define		ROUTER_ADDRESS_REFERRER		"RouterIP%d"
#define		ROUTER_PORT_REFERRER		"RouterPort%d"

// CDKEY SERVER KEYNAME
#define		CDKEY_ADDRESS_REFERRER		"CDKeyServerIP%d"
#define		CDKEY_PORT_REFERRER			"CDKeyServerPort%d"

// PROXY SERVER KEYNAME
#define		PROXY_ADDRESS_REFERRER		"ProxyIP%d"
#define		PROXY_PORT_REFERRER			"ProxyPort%d"

// NAT SERVER KEYNAME
#define		NAT_ADDRESS_REFERRER		"NATServerIP%d"
#define		NAT_PORT_REFERRER			"NATServerPort%d"

// CHAT SERVER KEYNAME
#define		CHAT_ADDRESS_REFERRER		"IRCIP%d"
#define		CHAT_PORT_REFERRER			"IRCPort%d"

// SERVER TYPES
enum SERVER_TYPE {
    SRV_ROUTER,
    SRV_CDKEY,
    SRV_PROXY,
	SRV_NAT,
	SRV_CHAT
};

extern "C" {
	
GSbool __stdcall InitializeFileParser(const GSchar *szConfigFilePath);
GSbool __stdcall InitializeStreamParser(GSchar **pszStream);

GSbool __stdcall GetServerAddress(SERVER_TYPE eServerType,GSuint uiIndex, GSchar *szAddress, GSushort *usPort);

GSbool __stdcall GetConfigStringValue(GSchar *szSectionName, GSchar *szKeyName, GSchar *szDefaultValue, GSchar *szBuffer, GSint iSize);
GSint  __stdcall GetConfigNumericValue(GSchar *szSectionName, GSchar *szKeyName, GSint iDefaultValue);

GSvoid __stdcall UninitializeParser();

} // extern "C"

#endif //__CONFIGFILEPARSER_H__
