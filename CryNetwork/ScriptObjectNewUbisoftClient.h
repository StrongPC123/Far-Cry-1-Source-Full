// ScriptObjectInput.h: interface for the CScriptObjectNewUbisoftClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__SCRIPT_OBJECT_NEW_UBISOFT_CLIENT__INCLUDED_)
#define __SCRIPT_OBJECT_NEW_UBISOFT_CLIENT__INCLUDED_


#ifndef NOT_USE_UBICOM_SDK


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>

class NewUbisoftClient;

/*! This class implements ubisoft.com script functions.

	REMARKS:
	After initialization of the script-object it will be globally accessable through scripts using the namespace "UbisoftClient".

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectNewUbisoftClient : public _ScriptableEx<CScriptObjectNewUbisoftClient>
{
public:
	//! constructor
	CScriptObjectNewUbisoftClient();
	//! destructor
	virtual ~CScriptObjectNewUbisoftClient();
	//!
	//! /param pScriptSystem Pointer to the ScriptSystem-interface
	//! /param pSystem Pointer to the System-interface
	void Init( IScriptSystem *pScriptSystem, ISystem *pSystem, NewUbisoftClient *inUbiSoftClient );
	//!
	static void InitializeTemplate(IScriptSystem *pSS);
	//!
	static void ReleaseTemplate();

	int Client_GetStoredUsername(IFunctionHandler *pH);
	int Client_GetStoredPassword(IFunctionHandler *pH);

	// auto log in using info stored in registry
	int Client_AutoLogin(IFunctionHandler *pH);
	
	// Logs in using szUsername (string) and szPassword (string)
	int Client_Login(IFunctionHandler *pH);

	// Request list of Game Servers. No parameters
	int Client_RequestGameServers(IFunctionHandler *pH);

	// Join a Game Server. iLobbyID (int), iRoomID (int)
	int Client_JoinGameServer(IFunctionHandler *pH);

	// Tell the Game Service you have connected to the game server. No parameters
	int Client_ConnectedToGameServer(IFunctionHandler *pH);

	// Tell the Game Service that you have left the game server. No parameters
	int Client_LeaveGameServer(IFunctionHandler *pH);

	// Disconnect from Game Service. No parameters
	int Client_Disconnect(IFunctionHandler *pH);

	// Create an account. szUsername (string), szPassword (string)
	int Client_CreateAccount(IFunctionHandler *pH);

	// Check if the client is logged in
	int Client_IsConnected(IFunctionHandler *pH);

	//Set the cdkey
	int Client_SetCDKey(IFunctionHandler *pH);

	//Request the Message of the Day
	int Client_RequestMOTD(IFunctionHandler *pH);

	// Remove the server from Game Service. No Parameters
	int Server_DestroyServer(IFunctionHandler *pH);

	//! used by the NewUbisoftClient
	DWORD	GetAbsTimeInSeconds();

	// Script Callbacks -------------------------------------------
 
	void Client_LoginSuccess(const char *szUsername);
	void Client_LoginFail(const char *szText);
	void Client_GameServer(int iLobbyID, int iRoomID, const char *szGameServer,
		const char *szIPAddress, const char *szLANIPAddress, int iMaxPlayers, int iNumPlayers);
	void Client_RequestFinished();
	void Client_JoinGameServerSuccess(const char *szIPAddress, const char *szLanIPAddress,
		unsigned short usPort);
	void Client_JoinGameServerFail(const char *szText);

	void Client_CreateAccountSuccess();
	void Client_CreateAccountFail(const char *szText);

	void Client_MOTD(const char *szUbiMOTD, const char *szGameMOTD);

	void Server_RegisterServerSuccess(int iLobbyID, int iRoomID);
	void Server_RegisterServerFail();
	void Server_LobbyServerDisconnected();
	void Server_PlayerJoin(const char *szUsername);
	void Server_PlayerLeave(const char *szUsername);
	void CDKey_Failed(const char *szText);
	void CDKey_GetCDKey();
	void CDKey_ActivationSuccess();
	void CDKey_ActivationFail(const char *szText);

private: // --------------------------------------------------------------

	ISystem *								m_pSystem;					//!
	IConsole *							m_pConsole;					//!
	IScriptSystem *					m_pScriptSystem;		//!
	NewUbisoftClient *			m_pUbiSoftClient;		//! (a different class is responsible for destruction)
};


#endif // NOT_USE_UBICOM_SDK

#endif //__SCRIPT_OBJECT_NEW_UBISOFT_CLIENT__INCLUDED_
