//////////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XNetwork.cpp
//  Description: Network stuffs implementation.
//
//  History:
//  - August 6, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XNetwork.h"
#include "Game.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// SXServerInfos implementation.

//////////////////////////////////////////////////////////////////////////
SXServerInfos::SXServerInfos()
{
	strName = "@UnknownServer";
	strMap = "@UnknownMap";
	strGameType = "@UnknownGameType";
	strMod = "@UnknownMod";
	nPlayers = 0;
	nMaxPlayers = 0;
	nPort = DEFAULT_SERVERPORT;
	nPing = 0;
	nComputerType = 0;
	nServerFlags = 0;
}

//////////////////////////////////////////////////////////////////////////
bool SXServerInfos::Read(const string &szInfoString)
{
	if (szInfoString.empty() || szInfoString[0] != 'S')
	{
		return false;
	}

//	SFileVersion Version;
	const char *pBuf = szInfoString.c_str()+1;	// skip the 'S'

	// extract the port
	nPort	= *((int *)pBuf);	pBuf+=4;
	nComputerType = *pBuf++;
	VersionInfo.Set(pBuf); pBuf += strlen(pBuf)+1;
	strName = pBuf; pBuf += strName.size()+1;
	strMod = pBuf; pBuf += strMod.size()+1;
	strGameType = pBuf; pBuf += strGameType.size()+1;
	strMap = pBuf; pBuf += strMap.size()+1;
	nPlayers = *pBuf++;
	nMaxPlayers = *pBuf++;
	nServerFlags = 0;

	if (*pBuf++)
		nServerFlags |= SXServerInfos::FLAG_PASSWORD;
	if (*pBuf++)
		nServerFlags |= SXServerInfos::FLAG_CHEATS;
	if (*pBuf++)
		nServerFlags |= SXServerInfos::FLAG_NET;
	if (*pBuf++)
		nServerFlags |= SXServerInfos::FLAG_PUNKBUSTER;

	return true;
}


//////////////////////////////////////////////////////////////////////////
// SXGameContext implementation.

//////////////////////////////////////////////////////////////////////////
SXGameContext::SXGameContext()
{
	strMapFolder = "NoMapCtx";	// <<FIXME>> Use a better name for the final version...
	bForceNonDevMode=false;
	ucServerInfoVersion=SERVERINFO_FORMAT_VERSION;
	bInternetServer=false;

	// HI:CPUType

#ifdef AMD64
	nComputerType=1;		// AMD64
#else
	nComputerType=0;		// IntelCompatible
#endif

	nComputerType<<=4;

	// LO:OSType

#ifdef LIMUX
	nComputerType|=1;		// Linux
#else
	nComputerType|=0;		// Windows
#endif
}

const char *SXGameContext::GetCPUTarget() const
{
	switch(nComputerType>>4)
	{
		case 0:	return "IntelCompatible";
		case 1:	return "AMD64";
	}
	
	return 0;
}

const char *SXGameContext::GetOSTarget() const
{
	switch(nComputerType&0xf)
	{
		case 0:	return "Windows";
		case 1:	return "Linux";
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
bool SXGameContext::Write(CStream &stm)
{
	if(!stm.Write(ucServerInfoVersion)) 
		return false;

	if(!stm.Write(strMapFolder))
		return false;

	if(!stm.Write(strMod))
		return false;

	if(!stm.Write(strGameType))
		return false;

	if(!stm.Write((unsigned int)dwNetworkVersion))
		return false;

	if(!stm.Write(strMission))
		return false;

	if(!stm.Write(wLevelDataCheckSum))
		return false;
	
	if(!stm.Write(bForceNonDevMode))
		return false;

	if(!stm.Write(bInternetServer))
		return false;

	if(!stm.Write(nComputerType))
		return false;

	return true;
}

bool SXGameContext::IsVersionOk() const
{
	return dwNetworkVersion==NETWORK_FORMAT_VERSION && ucServerInfoVersion==(unsigned char)SERVERINFO_FORMAT_VERSION;
}


//////////////////////////////////////////////////////////////////////////
bool SXGameContext::Read(CStream &stm)
{
	if(!stm.Read(ucServerInfoVersion))	
		return false;

	if(!stm.Read(strMapFolder))
		return false;

	if(!stm.Read(strMod))
		return false;

	if(!stm.Read(strGameType))
		return false;

	if(!stm.Read((unsigned int&)dwNetworkVersion))
		return false;

	if(!stm.Read(strMission))
		return false;

	if(!stm.Read(wLevelDataCheckSum))
		return false;

	if(!stm.Read(bForceNonDevMode))
		return false;

	if(!stm.Read(bInternetServer))
		return false;

	if(!stm.Read(nComputerType))
		return false;

	return true;
}
