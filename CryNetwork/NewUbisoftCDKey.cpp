#include "stdafx.h"

#ifndef NOT_USE_UBICOM_SDK

#include "IConsole.h"									// ICVar
#include "NewUbisoftClient.h"
#include "GSCDKeyInterface.h"
#include "CommonDefines.h"
#include <string>

static NewUbisoftClient *g_pUbisoftClient;

static const char CDKEYREGKEY[] =				 "CDKey";
static const char ACTIVATIONIDREGKEY[] = "ActivationID";




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


//////////////////////////////////////
//
// CDKey Callbacks
//
/////////////////////////////////////
extern "C"
{

static GSvoid __stdcall CDKey_RcvActivationID(PREPLY_INFORMATION psReplyInfo,
		PVALIDATION_SERVER_INFO psValidationServerInfo, GSubyte *pucActivationID, GSubyte *pucGlobalID)
{
	if (g_pUbisoftClient)
		g_pUbisoftClient->RcvActivationID(psReplyInfo,psValidationServerInfo,pucActivationID,pucGlobalID);
}

static GSvoid __stdcall CDKey_RcvAuthorizationID(PREPLY_INFORMATION psReplyInfo,
		PVALIDATION_SERVER_INFO psValidationServerInfo, GSubyte *pucAuhorizationID)
{
	if (g_pUbisoftClient)
		g_pUbisoftClient->RcvAuthorizationID(psReplyInfo, psValidationServerInfo, pucAuhorizationID);
}

static GSvoid __stdcall CDKey_RcvValidationResponse(PREPLY_INFORMATION psReplyInfo,
		PVALIDATION_SERVER_INFO psValidationServerInfo, GSubyte *pucAuhorizationID, CDKEY_PLAYER_STATUS eStatus,
		GSubyte *pucGlobalID)
{
	if (g_pUbisoftClient)
		g_pUbisoftClient->RcvValidationResponse(psReplyInfo,psValidationServerInfo, pucAuhorizationID, eStatus,
		pucGlobalID);
}

static GSvoid __stdcall CDKey_RcvPlayerStatusRequest(PVALIDATION_SERVER_INFO psValidationServerInfo,
		GSubyte *pucAuhorizationID)
{
	if (g_pUbisoftClient)
		g_pUbisoftClient->RcvPlayerStatusRequest(psValidationServerInfo,pucAuhorizationID);
}

}

void NewUbisoftClient::CopyIDToString(const CDKeyIDVector &stVector, string &strString)
{
	strString = "";
	for (unsigned int i=0; i < stVector.size(); i++)
	{
		char szChar[10];
		sprintf(szChar,"%.2X",stVector[i]);
		strString += szChar;
	}
}

void NewUbisoftClient::CopyIDToVector(CDKeyIDVector &stVector, const GSubyte *pubArray,
		unsigned int uiSize)
{
	stVector.clear();
	for (unsigned int i=0; i < uiSize; i++)
	{
		stVector.push_back(pubArray[i]);
	}
	GSint iCompare = memcmp(&stVector[0],pubArray,uiSize);
	return;
}


bool NewUbisoftClient::InitCDKeySystem()
{
	g_pUbisoftClient = this;

	if (!m_hCDKey)
	{
		int iPort = sv_authport->GetIVal();

		m_hCDKey = GSCDKey_Initialize(iPort);
		GSCDKey_FixRcvActivationID(m_hCDKey,CDKey_RcvActivationID);
		GSCDKey_FixRcvAuthorizationID(m_hCDKey,CDKey_RcvAuthorizationID);
		GSCDKey_FixRcvValidationResponse(m_hCDKey,CDKey_RcvValidationResponse);
		GSCDKey_FixRcvPlayerStatusRequest(m_hCDKey,CDKey_RcvPlayerStatusRequest);
	}

	if (!m_pCDKeyServer)
	{
		m_pCDKeyServer = (PVALIDATION_SERVER_INFO)malloc(sizeof(_VALIDATION_SERVER_INFO));
		GSint iIndex=0;
		GetCDKeyServerAddress(iIndex,m_pCDKeyServer->szIPAddress,&m_pCDKeyServer->usPort);
	}
	return true;
}

bool NewUbisoftClient::Client_GetCDKeyAuthorizationID()
{
	InitCDKeySystem();

	char szCDkey[CDKEY_SIZE+1];
	if (!LoadCDKey(szCDkey))
		strcpy(szCDkey,"");

	GSubyte pubActivationID[ACTIVATION_ID_SIZE];
	if (!LoadActivationID(pubActivationID))
		memset(pubActivationID,0,ACTIVATION_ID_SIZE);

	_VALIDATION_INFO stValidationInfo;
	memcpy(stValidationInfo.ucActivationID,pubActivationID,ACTIVATION_ID_SIZE);
	strncpy(stValidationInfo.szCDKey,szCDkey,CDKEY_SIZE+1);

	GSCDKey_RequestAuthorization(m_hCDKey,m_pCDKeyServer,&stValidationInfo,10);

	return true;
}

bool NewUbisoftClient::RequestCDKeyActivationID()
{
	InitCDKeySystem();

	char szCDkey[CDKEY_SIZE+1];
	memset(szCDkey, 0, CDKEY_SIZE+1);
	LoadCDKey(szCDkey);

	ACTIVATION_INFO stActivationInfo;
	strncpy(stActivationInfo.szGameName,GAME_NAME,GAMELENGTH);
	strncpy(stActivationInfo.szCDKey,szCDkey,CDKEY_SIZE+1);

	GSCDKey_RequestActivation(m_hCDKey,m_pCDKeyServer,&stActivationInfo,5);

	return true;
}


bool NewUbisoftClient::Client_CheckForCDKey()
{
	InitCDKeySystem();

	GSchar szCDKey[CDKEY_SIZE+1];
	if (!LoadCDKey(szCDKey))
	{
		CDKey_GetCDKey();
		return false;
	}

	GSubyte pubActivationID[ACTIVATION_ID_SIZE];
	if (!LoadActivationID(pubActivationID))
	{
		RequestCDKeyActivationID();
		return false;
	}
	CDKey_ActivationSuccess();
	return true;
}

bool NewUbisoftClient::Client_SetCDKey(const char *szCDKey)
{
	SaveCDKey(szCDKey);

	RequestCDKeyActivationID();
	return true;
}

GSvoid NewUbisoftClient::RcvActivationID(PREPLY_INFORMATION psReplyInfo,
		PVALIDATION_SERVER_INFO psValidationServerInfo, GSubyte *pucActivationID, GSubyte *pucGlobalID)
{

	if (!psReplyInfo->bSucceeded)
	{
		if (psReplyInfo->usErrorID == ERRORCDKEY_TIMEOUT)
		{
			m_pLog->Log("\001Ubi.com: CDKey RcvActivationID Failed: timeout");
		}
		// Since it didn't succeed we don't know if the cdkey is valid so delete it.
		SaveCDKey(NULL);
		SaveActivationID(NULL);
		//Ask to for the cdkey again.
		string strError;
		GetCDKeyErrorText(psReplyInfo->usErrorID,strError);
		CDKey_ActivationFail(strError.c_str());
		return;
	}

	m_pLog->Log("\001Ubi.com: CDKey RcvActivationID Success");

	//CopyIDToVector(m_stActivationID,pucActivationID,ACTIVATION_ID_SIZE);
	SaveActivationID(pucActivationID);
	CDKey_ActivationSuccess();
}

GSvoid NewUbisoftClient::RcvAuthorizationID(PREPLY_INFORMATION psReplyInfo,
		PVALIDATION_SERVER_INFO psValidationServerInfo, GSubyte *pucAuthorizationID)
{
	if(!m_pSystem->GetIGame()->GetModuleState(EGameClient))
		return;

	if (!psReplyInfo->bSucceeded)
	{
		if (psReplyInfo->usErrorID == ERRORCDKEY_TIMEOUT)
		{
			m_pLog->Log("\001Ubi.com: CDKey RcvAuthorizationID Failed: Timeout");
		}
		else
		{
			string strError;
			GetCDKeyErrorText(psReplyInfo->usErrorID,strError);
			m_pLog->Log("\001Ubi.com: CDKey RcvAuthorizationID Failed: %s",strError.c_str());

			if ((psReplyInfo->usErrorID != ERRORCDKEY_TIMEOUT) && (psReplyInfo->usErrorID != ERRORCDKEY_INTERNAL_ERROR))
			{
				CDKey_ActivationFail(strError.c_str());
				return;
			}
		}
		// If the request failed let the player connect anyway because the cdkey server may be down.
		// If the game server also times out when verfiying this fake ID then it will let the player connect.
		BYTE bFakeAuthorizationID[AUTHORIZATION_ID_SIZE];
		memset(bFakeAuthorizationID,0,AUTHORIZATION_ID_SIZE);

		assert(m_pSystem->GetINetwork()->GetClient());
		m_pSystem->GetINetwork()->GetClient()->OnCDKeyAuthorization(bFakeAuthorizationID);
		return;
	}

/*	CDKeyIDVector stAuthorizationID;
	CopyIDToVector(stAuthorizationID,pucAuthorizationID,AUTHORIZATION_ID_SIZE);

	string strAuthorizationID;
	CopyIDToString(stAuthorizationID,strAuthorizationID);*/

	m_pLog->Log("\001Ubi.com: CDKey RcvAuthorizationID Success");
	//TODO: Send the Authorization ID to the game server.

	assert(m_pSystem->GetINetwork()->GetClient());
	m_pSystem->GetINetwork()->GetClient()->OnCDKeyAuthorization(pucAuthorizationID);
}


bool NewUbisoftClient::Server_CheckPlayerAuthorizationID(BYTE bPlayerID,
		const BYTE *pubAuthorizationID)
{
//	m_pLog->Log("Ubi.com: CheckPlayerAuthorizationID: bCheck=%c",m_bCheckCDKeys?'t':'f');

	// If the server wasn't created through ubi.com don't check cdkeys.
	if (!m_bCheckCDKeys)
	{
		IServer *pServer = m_pSystem->GetINetwork()->GetServerByPort(m_usGamePort);
		if(!pServer)
		{
			// that happened on a dedicated LAN server when a client tried to connect
			return false;						// better we ignore this
		}

		IServerSlot *pServerSlot = pServer->GetServerSlotbyID(bPlayerID);

		//CXServerSlot *pSlot = itSlot->second;
		pServerSlot->OnPlayerAuthorization(true,"",NULL,0);
		return true;
	}

	InitCDKeySystem();

	CDKeyIDVector stID;
	if (!pubAuthorizationID)
	{
		m_pLog->Log("\001Ubi.com: CDKey CheckPlayerAuthorizationID: Authorization ID null");
		return false;
	}

	CopyIDToVector(stID,pubAuthorizationID,AUTHORIZATION_ID_SIZE);

	//string strAuthorizationID;
	//CopyIDToString(stID,strAuthorizationID);

	m_pLog->Log("\001Ubi.com: CDKey CheckPlayerAuthorizationID: %i",bPlayerID);

	AddAuthorizedID(bPlayerID,stID);
	GSCDKey_ValidateUser(m_hCDKey,m_pCDKeyServer,const_cast<GSubyte*>(pubAuthorizationID),GAME_NAME,6);
	return true;
}

bool NewUbisoftClient::Server_RemovePlayer(BYTE bPlayerID)
{
	CDKeyIDVector stID;
	if (FindAuthorizedID(bPlayerID,stID))
	{
		//string strID;
		//CopyIDToString(stID,strID);
		m_pLog->Log("\001Ubi.com: CdKey: RemovePlayer: %i",bPlayerID);
		GSCDKey_DisconnectUser(m_hCDKey,m_pCDKeyServer,&stID[0]);
		RemoveAuthorizedID(bPlayerID);
		return true;
	}
	return false;
}

bool NewUbisoftClient::Server_RemoveAllPlayers()
{
	CDKeyIDVector stID;

	AuthorizedIDs::iterator itID = m_stAuthorizedIDs.begin();
	while (itID != m_stAuthorizedIDs.end())
	{
		//string strID;
		//CopyIDToString(itID->first,strID);
		m_pLog->Log("\001Ubi.com: CDKey RemovePlayer: %i",itID->second);
		GSCDKey_DisconnectUser(m_hCDKey,m_pCDKeyServer,const_cast<GSubyte*>(&itID->first[0]));
		itID++;
	}
	m_stAuthorizedIDs.clear();
	return true;
}

void NewUbisoftClient::RcvValidationResponse(PREPLY_INFORMATION psReplyInfo,
		PVALIDATION_SERVER_INFO psValidationServerInfo, GSubyte *pucAuthorizationID, CDKEY_PLAYER_STATUS eStatus,
		GSubyte *pucGlobalID)
{
	CDKeyIDVector stID;
	CopyIDToVector(stID,pucAuthorizationID,AUTHORIZATION_ID_SIZE);

	BYTE bPlayerID;

	FindPlayerID(stID,bPlayerID);

	IServer *pServer = m_pSystem->GetINetwork()->GetServerByPort(m_usGamePort);
	if(!pServer)
	{
		assert(pServer);	
		return;						// better we ignore this
	}

	IServerSlot *pServerSlot = pServer->GetServerSlotbyID(bPlayerID);

	if(!pServerSlot)
	{
		m_pLog->Log("\001Ubi.com: CDKey RcvValidationResponse Failed: Player disconnected during authorization!");
		return;
	}

	//CXServerSlot *pSlot = itSlot->second;

	if (!psReplyInfo->bSucceeded)
	{
/*		if (psReplyInfo->usErrorID == ERRORCDKEY_TIMEOUT)
		{
			// don't try again
			m_pLog->Log("\001RcvValidationResponse Failed: timeout accept");
			itSlot->second->OnXPlayerAuthorization(true,"");
		}
		else
		{*/
			string strError;
			GetCDKeyErrorText(psReplyInfo->usErrorID,strError);
			m_pLog->Log("\001Ubi.com: CDKey RcvValidationResponse Failed: %s",strError.c_str());
			pServerSlot->OnPlayerAuthorization(false,strError.c_str(),NULL,0);
			RemoveAuthorizedID(stID);
		//}
		return;
	}

	if (eStatus == E_PLAYER_UNKNOWN)
	{
		m_pLog->Log("\001Ubi.com: CDKey RcvValidationResponse Success: PLAYERUNKNOWN");
		pServerSlot->OnPlayerAuthorization(false,INVALIDCDKEY,NULL,0);
		RemoveAuthorizedID(stID);
	}
	else if (eStatus == E_PLAYER_INVALID)
	{
		m_pLog->Log("\001Ubi.com: CDKey RcvValidationResponse Success: INVALIDCDKEY");
		pServerSlot->OnPlayerAuthorization(false,INVALIDCDKEY,NULL,0);
		RemoveAuthorizedID(stID);
	}
	else if (eStatus == E_PLAYER_VALID)
	{
		m_pLog->Log("\001Ubi.com: CDKey RcvValidationResponse Success: VALIDCDKEY");
		pServerSlot->OnPlayerAuthorization(true,"",pucGlobalID,GLOBAL_ID_SIZE);
	}

}

void NewUbisoftClient::RcvPlayerStatusRequest(PVALIDATION_SERVER_INFO psValidationServerInfo, GSubyte *pucAuthorizationID)
{
	if(!m_pSystem->GetIGame()->GetModuleState(EGameServer))
	{
		Server_RemoveAllPlayers();
		return;
	}

	CDKeyIDVector stID;
	CopyIDToVector(stID,pucAuthorizationID,AUTHORIZATION_ID_SIZE);

	AuthorizedIDs::iterator itID = m_stAuthorizedIDs.find(stID);
	if(itID != m_stAuthorizedIDs.end())
	{
		IServer *pServer = m_pSystem->GetINetwork()->GetServerByPort(m_usGamePort);
		if(!pServer)
		{
			assert(pServer);	
			return;						// better we ignore this
		}

		IServerSlot *pServerSlot = pServer->GetServerSlotbyID(itID->second);

		// If the player is online
		if(pServerSlot)
		{
			m_pLog->Log("\001Ubi.com: CDKey RcvPlayerStatusRequest: Player Valid");
			GSCDKey_PlayerStatusReply(m_hCDKey,psValidationServerInfo,const_cast<GSubyte*>(&(itID->first[0])),E_PLAYER_VALID);
		}
		else
		{
			m_pLog->Log("\001Ubi.com: CDKey RcvPlayerStatusRequest: Player Unknown");
			GSCDKey_PlayerStatusReply(m_hCDKey,psValidationServerInfo,const_cast<GSubyte*>(&(itID->first[0])),E_PLAYER_UNKNOWN);
		}
	}
	else
	{
		m_pLog->Log("\001Ubi.com: CDKey RcvPlayerStatusRequest: AuthorizationID Unknown");
		GSCDKey_PlayerStatusReply(m_hCDKey,psValidationServerInfo,const_cast<GSubyte*>(pucAuthorizationID),E_PLAYER_UNKNOWN);
	}
}

bool NewUbisoftClient::AddAuthorizedID(BYTE bPlayerID, const CDKeyIDVector &stAuthorizationID)
{
	if (m_stAuthorizedIDs.find(stAuthorizationID) == m_stAuthorizedIDs.end())
	{
		m_stAuthorizedIDs[stAuthorizationID] = bPlayerID;
		return true;
	}
	return false;
}

bool NewUbisoftClient::RemoveAuthorizedID(const CDKeyIDVector &stAuthorizationID)
{
	AuthorizedIDs::iterator itID = m_stAuthorizedIDs.find(stAuthorizationID);
	if (itID != m_stAuthorizedIDs.end())
	{
		m_stAuthorizedIDs.erase(itID);
		return true;
	}
	return false;
}

bool NewUbisoftClient::RemoveAuthorizedID(BYTE bPlayer)
{
	AuthorizedIDs::iterator itID = m_stAuthorizedIDs.begin();
	while (itID != m_stAuthorizedIDs.end())
	{
		if (itID->second == bPlayer)
		{
			m_stAuthorizedIDs.erase(itID);
			return true;
		}
		itID++;
	}
	return false;
}

bool NewUbisoftClient::FindPlayerID(const CDKeyIDVector &stAuthorizationID, BYTE &bPlayer)
{
	AuthorizedIDs::iterator itID = m_stAuthorizedIDs.find(stAuthorizationID);
	if (itID != m_stAuthorizedIDs.end())
	{
		bPlayer = itID->second;
		return true;
	}
	return false;
}

bool NewUbisoftClient::FindAuthorizedID(BYTE bPlayerID, CDKeyIDVector &stAuthorizationID)
{
	AuthorizedIDs::iterator itID = m_stAuthorizedIDs.begin();
	while (itID != m_stAuthorizedIDs.end())
	{
		if (itID->second == bPlayerID)
		{
			stAuthorizationID = itID->first;
			return true;
		}
		itID++;
	}
	return false;
}

bool NewUbisoftClient::GetCDKeyErrorText(GSushort usError,string &strText)
{
	switch(usError)
	{
		case ERRORCDKEY_INVALID_CDKEY:
			strText = INVALIDCDKEY;
			break;
		case ERRORCDKEY_TIMEOUT:
			strText = CDKEYTIMEOUT;
			break;
		case ERRORCDKEY_NOT_CHALLENGED:
			strText = CDKEYNOTCHALLENGED;
			break;
		case ERRORCDKEY_ALREADY_ONLINE:
			strText = CDKEYONLINE;
			break;
		case ERRORCDKEY_INTERNAL_ERROR:
			strText = CDKEYINTERNALERROR;
			break;
	}
	return true;
}

bool NewUbisoftClient::CDKey_Error(GSushort usError)
{
	string strText;

	GetCDKeyErrorText(usError,strText);
	CDKey_Failed(strText.c_str());

	return true;
}

void NewUbisoftClient::SaveCDKey(const GSchar *szCDKey)
{
	if (szCDKey && (((strlen(szCDKey)+1) % 8) == 0))
	{
		unsigned char szEncCDKey[128] = {0};
		unsigned int Key[4] = {1337, 1337*2, 1337*4, 1337*8};
		
		TEA_ENCODE((unsigned int *)szCDKey, (unsigned int *)szEncCDKey, strlen(szCDKey)+1, Key);

		char szCDKeyHex[256] = {0};

		sprintf(szCDKeyHex, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			szEncCDKey[0], szEncCDKey[1], szEncCDKey[2], szEncCDKey[3], szEncCDKey[4], szEncCDKey[5],
			szEncCDKey[6], szEncCDKey[7], szEncCDKey[8], szEncCDKey[9], szEncCDKey[10], szEncCDKey[11],
			szEncCDKey[12], szEncCDKey[13], szEncCDKey[14], szEncCDKey[15], szEncCDKey[16], szEncCDKey[17],
			szEncCDKey[18], szEncCDKey[19], szEncCDKey[20], szEncCDKey[21], szEncCDKey[22], szEncCDKey[23]);

		WriteStringToRegistry("Ubi.com", CDKEYREGKEY, szCDKeyHex);
	}
	else
	{
		RemoveStringFromRegistry("Ubi.com", CDKEYREGKEY);
	}
}

bool NewUbisoftClient::LoadCDKey(GSchar *szCDKey)
{
	if (IsValueOnRegistry("Ubi.com", CDKEYREGKEY))
	{
		string szReadCDKey;

		if (ReadStringFromRegistry("Ubi.com", CDKEYREGKEY, szReadCDKey))
		{
			// this is a pre1.2 stored cdkey
			if (((szReadCDKey.size()-1) == CDKEY_SIZE) && (szReadCDKey[0] == 'F') && (szReadCDKey[1] == 'C') && (szReadCDKey[2] == 'Y'))
			{
				strncpy(szCDKey, szReadCDKey.c_str(), CDKEY_SIZE);
				szCDKey[CDKEY_SIZE] = 0;

				// save it in the new format
				SaveCDKey(szCDKey);
			}
			else if ((szReadCDKey.size()-1) == (CDKEY_SIZE+1)*2) // -1 because of the terminating 0, x2 because of hex representation
			{
				char szAux[32];
				char szEncCDKey[128] = {0};

				for (int i = 0; i < CDKEY_SIZE+1; i++)
				{
					sprintf(szAux, "0x%c%c", szReadCDKey[i*2+0], szReadCDKey[i*2+1]);

					szEncCDKey[i] = strtol(szAux, 0, 0);
				}

				// decrypt it
				unsigned char szDecCDKey[128] = {0};
				unsigned int Key[4] = {1337, 1337*2, 1337*4, 1337*8};

				TEA_DECODE((unsigned int *)szEncCDKey, (unsigned int *)szDecCDKey, CDKEY_SIZE+1, Key);
				
				strncpy(szCDKey, (char *)szDecCDKey, min(strlen((char *)szDecCDKey), CDKEY_SIZE));
				szCDKey[CDKEY_SIZE] = 0;
			}
			else
			{
				return false;
			}

			return true;
		}
	}

	return false;
}

void NewUbisoftClient::SaveActivationID(const GSubyte *pubActivationID)
{
	if (pubActivationID)
	{
		char szActivationHex[256] = {0};

		sprintf(szActivationHex, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			pubActivationID[0], pubActivationID[1], pubActivationID[2], pubActivationID[3], pubActivationID[4], pubActivationID[5],
			pubActivationID[6], pubActivationID[7], pubActivationID[8], pubActivationID[9], pubActivationID[10], pubActivationID[11],
			pubActivationID[12], pubActivationID[13], pubActivationID[14], pubActivationID[15]);

		WriteStringToRegistry("Ubi.com", ACTIVATIONIDREGKEY, szActivationHex);
	}
}

bool NewUbisoftClient::LoadActivationID(GSubyte *pubActivationID)
{
	if (IsValueOnRegistry("Ubi.com", ACTIVATIONIDREGKEY))
	{
		string szReadActivationID;

		if (ReadStringFromRegistry("Ubi.com", ACTIVATIONIDREGKEY, szReadActivationID))
		{
			// this is pre 1.2
			if ((szReadActivationID.size()-1) == ACTIVATION_ID_SIZE) // size-1 because of the terminating 0
			{
				// load in the old form
				strncpy((char *)pubActivationID, szReadActivationID.c_str(), ACTIVATION_ID_SIZE);
				
				// save it the new form
				SaveActivationID(pubActivationID);
			}
			else if ((szReadActivationID.size()-1) == (ACTIVATION_ID_SIZE*2)) // size-1 because of the terminating 0
			{
				char szAux[32];

				for (int i = 0; i < ACTIVATION_ID_SIZE; i++)
				{
					sprintf(szAux, "0x%c%c", szReadActivationID[i*2+0], szReadActivationID[i*2+1]);

					pubActivationID[i] = strtol(szAux, 0, 0);
				}
			}
			else
			{
				return false;
			}

			return true;
		}
	}

	return false;
}

#endif // NOT_USE_UBICOM_SDK
