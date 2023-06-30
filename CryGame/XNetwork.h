//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XNetwork.h
//  Description: Network stuffs.
//
//  History:
//  - August 6, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_XNETWORK_H
#define GAME_XNETWORK_H
#if _MSC_VER > 1000
# pragma once
#endif

#include "IPAddress.h"				// CIPAddress


//////////////////////////////////////////////////////////////////////////////////////////////
// Server to client messages

typedef unsigned char XSERVERMSG;

#define XSERVERMSG_UPDATEENTITY				((XSERVERMSG)(0))
#define XSERVERMSG_ADDENTITY					((XSERVERMSG)(1))
#define XSERVERMSG_REMOVEENTITY				((XSERVERMSG)(2))
#define XSERVERMSG_TIMESTAMP					((XSERVERMSG)(3))
#define XSERVERMSG_TEXT								((XSERVERMSG)(4))
#define XSERVERMSG_SETPLAYERSCORE			((XSERVERMSG)(5))
#define XSERVERMSG_SETENTITYSTATE			((XSERVERMSG)(6))
//#define XSERVERMSG_OBITUARY					  ((XSERVERMSG)(7))
#define XSERVERMSG_SETTEAMSCORE				((XSERVERMSG)(8))
#define XSERVERMSG_SETTEAMFLAGS				((XSERVERMSG)(9))
#define XSERVERMSG_SETPLAYER					((XSERVERMSG)(10))
#define XSERVERMSG_CLIENTSTRING				((XSERVERMSG)(11))
#define XSERVERMSG_CMD								((XSERVERMSG)(12))
#define XSERVERMSG_SETTEAM						((XSERVERMSG)(13))
#define XSERVERMSG_ADDTEAM						((XSERVERMSG)(14))
#define XSERVERMSG_REMOVETEAM					((XSERVERMSG)(15))
#define XSERVERMSG_SETENTITYNAME			((XSERVERMSG)(16))
#define XSERVERMSG_BINDENTITY					((XSERVERMSG)(17))
#define XSERVERMSG_SCOREBOARD					((XSERVERMSG)(18))
#define XSERVERMSG_SETGAMESTATE				((XSERVERMSG)(19))
#define XSERVERMSG_TEAMS							((XSERVERMSG)(20))
#define XSERVERMSG_SYNCVAR						((XSERVERMSG)(21))
#define XSERVERMSG_EVENTSCHEDULE			((XSERVERMSG)(22))
#define XSERVERMSG_UNIDENTIFIED				((XSERVERMSG)(23))		// 23 is not used as message
#define XSERVERMSG_REQUESTSCRIPTHASH	((XSERVERMSG)(24))
#define XSERVERMSG_AISTATE						((XSERVERMSG)(25))

#define XSERVERMSG_RESERVED_DONOT_USE1	((XSERVERMSG)(250))
#define XSERVERMSG_RESERVED_DONOT_USE2	((XSERVERMSG)(251))
#define XSERVERMSG_RESERVED_DONOT_USE3	((XSERVERMSG)(252))
#define XSERVERMSG_RESERVED_DONOT_USE4	((XSERVERMSG)(253))
#define XSERVERMSG_RESERVED_DONOT_USE5	((XSERVERMSG)(254))
#define XSERVERMSG_RESERVED_DONOT_USE6	((XSERVERMSG)(255))



//////////////////////////////////////////////////////////////////////////////////////////////
// Client to server messages

typedef unsigned char XCLIENTMSG;

#define XCLIENTMSG_UNKNOWN							((XCLIENTMSG)(0))
#define XCLIENTMSG_PLAYERPROCESSINGCMD	((XCLIENTMSG)(1))
#define XCLIENTMSG_TEXT									((XCLIENTMSG)(2))
#define XCLIENTMSG_JOINTEAMREQUEST			((XCLIENTMSG)(3))
#define XCLIENTMSG_CALLVOTE			        ((XCLIENTMSG)(4))
#define XCLIENTMSG_VOTE			            ((XCLIENTMSG)(5))
#define XCLIENTMSG_KILL			            ((XCLIENTMSG)(6))
#define XCLIENTMSG_NAME			            ((XCLIENTMSG)(7))
#define XCLIENTMSG_CMD									((XCLIENTMSG)(8))
#define XCLIENTMSG_RATE									((XCLIENTMSG)(9))
#define XCLIENTMSG_ENTSOFFSYNC					((XCLIENTMSG)(10))
#define XCLIENTMSG_RETURNSCRIPTHASH			((XCLIENTMSG)(11))
#define XCLIENTMSG_AISTATE							((XCLIENTMSG)(12))
#define XCLIENTMSG_UNIDENTIFIED					XSERVERMSG_UNIDENTIFIED		// 23 is not used as message

#define XCLIENTMSG_RESERVED_DONOT_USE1	((XCLIENTMSG)(250))
#define XCLIENTMSG_RESERVED_DONOT_USE2	((XCLIENTMSG)(251))
#define XCLIENTMSG_RESERVED_DONOT_USE3	((XCLIENTMSG)(252))
#define XCLIENTMSG_RESERVED_DONOT_USE4	((XCLIENTMSG)(253))
#define XCLIENTMSG_RESERVED_DONOT_USE5	((XCLIENTMSG)(254))
#define XCLIENTMSG_RESERVED_DONOT_USE6	((XCLIENTMSG)(255))


////////////////////////////////////////////////////
//CLIENT COMMAND
////////////////////////////////////////////////////
#define SEND_ONE	 1
#define SEND_MANY	 2
#define SEND_TEAM	 3

//<<>>list here the commands ids
//#define CMD_DONT_SEND_ME		0x00
#define CMD_SAY								0x01
#define CMD_SAY_TEAM					0x02
#define CMD_SAY_ONE						0x03

#define DEFAULT_TEXT_LIFETIME 7.5f

struct TextMessage
{
	//! constructor
	TextMessage()
	{
		fLifeTime=DEFAULT_TEXT_LIFETIME;
	}
	
	BYTE								cMessageType;			//!<
	unsigned short			uiSender;					//!<
	unsigned short			uiTarget;					//!<
  float								fLifeTime;				//!<
//	CStream							stmPayload;
	string							m_sText;					//!<

	//!
	bool Write(CStream &stm)
	{
		stm.Write(cMessageType);
		stm.Write(uiSender);
		stm.Write(uiTarget);
		if(fLifeTime==DEFAULT_TEXT_LIFETIME)
		{
			stm.Write(false);
		}
		else
		{
			stm.Write(true);
			unsigned char temp;
			temp = (unsigned char) (fLifeTime * 10.0f);
			stm.Write(temp);
		}
		return stm.Write(m_sText);
//		stm.Write((unsigned short)stmPayload.GetSize());
//		return stm.Write(stmPayload);
	}

	//!
	bool Read(CStream &stm)
	{
//		unsigned short usSize;
		bool b;

		if(!stm.Read(cMessageType))
			return false;
		if(!stm.Read(uiSender))
			return false;
		if(!stm.Read(uiTarget))
			return false;
		if(!stm.Read(b))
			return false;

		if(!b)
		{
			fLifeTime=DEFAULT_TEXT_LIFETIME;
		}
		else
		{
			unsigned char temp;
			temp = (unsigned char) (fLifeTime * 10.0f);
			stm.Read(temp);
			fLifeTime = temp / 10.0f;
		}
		return stm.Read(m_sText);
//		stm.Read(usSize);
//		stmPayload.SetSize(usSize);
//		return stm.ReadBits(stmPayload.GetPtr(), usSize);
	}
};


//////////////////////////////////////////////////////////////////////////////////////////////
// Utility classes

///////////////////////////////////////////////
// Infos about the server
struct SXServerInfos
{
	enum Flags
	{
		FLAG_PASSWORD		= 1 << 0,
		FLAG_CHEATS			= 1 << 1,
		FLAG_NET				= 1 << 2,
		FLAG_PUNKBUSTER	= 1 << 3,
	};

	string				strName;							//!< e.g. "Jack's Server"
	string				strMap;								//!< e.g. "MP_Airstrip"
	string				strGameType;					//!< e.g. "ASSAULT", "FFA", "TDM"
	string				strMod;								//!< e.g. "FarCry", "Counterstrike"  current TCM(Total Conversion Mod), specified with -MOD ...
	BYTE					nPlayers;							//!< current player count
	BYTE					nMaxPlayers;					//!< max player count
	WORD					nPort;								//!<
	WORD					nPing;								//!<
//	DWORD					dwGameVersion;				//!< still used?
	CIPAddress		IP;										//!< 
	BYTE					nComputerType;        //!< reserved, currently not used (CPU: AMD64,Intel  OS:Linux,Win)
	int						nServerFlags;					//!< Flags with special info about server.
	SFileVersion	VersionInfo;					//!<

	//! constructor
	SXServerInfos();
	virtual ~SXServerInfos() {};

	bool Read(const string &szServerInfoString);
};



///////////////////////////////////////////////
// Game context (sent to the client when connecting to a server)
struct SXGameContext
{
	unsigned char		ucServerInfoVersion;	//!< SERVERINFO_FORMAT_VERSION (needed to prevent connects from old clients)
	string					strMapFolder;					//!<
	string					strMod;								//!< e.g. "FarCry", "Counterstrike" current TCM(Total Conversion Mod), specified with -MOD ...
	string					strGameType;					//!< e.g. "ASSAULT", "FFA", "TDM"
	string					strMission;						//!<
	DWORD						dwNetworkVersion;			//!< NETWORK_FORMAT_VERSION
	WORD						wLevelDataCheckSum;		//!<
	bool						bForceNonDevMode;			//!< client is forced not to use the Devmode
	bool						bInternetServer;			//!< true=requires UBI login, false=LAN
	BYTE						nComputerType;        //!< HI:CPUType, LO:OSType

	//! constructor
	SXGameContext();

	//!
	bool Write(class CStream &stm);
	//!
	bool Read(class CStream &stm);
	//!
	bool IsVersionOk() const;
	//! \return 0 if unknown, zero terminated string otherwise
	const char *GetCPUTarget() const;
	//! \return 0 if unknown, zero terminated string otherwise
	const char *GetOSTarget() const;
};

#endif // GAME_XNETWORK_H
