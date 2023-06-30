
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef _SCRIPTOBJECTSERVER_H_
#define _SCRIPTOBJECTSERVER_H_

#include <IScriptSystem.h>
#include <_ScriptableEx.h>
class CXServer;

/*! This class implements script-functions for exposing the server functionalities

	REMARKS:
	After initialization of the script-object it will be globally accessable through scripts using the namespace "Server".
	This object isn't instantiated when the client is connected to a remote server
	
	Example:
		Server.SpawnEntity("Rocket");

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectServer :
public _ScriptableEx<CScriptObjectServer> 
{
public:
	CScriptObjectServer();
	virtual ~CScriptObjectServer();
	bool Create(IScriptSystem *pScriptSystem,IXSystem *pCSystem,CXGame *pGame);
	void SetServer(CXServer *pServer){m_pServer=pServer;}

	int Unban(IFunctionHandler *pH);
	int ListBans(IFunctionHandler *pH);
	int GetServerSlotBySSId(IFunctionHandler *pH);
	int GetServerSlotByEntityId(IFunctionHandler *pH);
	int GetServerSlotMap(IFunctionHandler *pH);
//	int IsValidServerSlotId(IFunctionHandler *pH);
//  int MultiCastObituary(IFunctionHandler *pH);
	int GetNumPlayers(IFunctionHandler *pH);
	int BroadcastText(IFunctionHandler *pH);
	int SpawnEntity(IFunctionHandler *pH);
	int RemoveEntity(IFunctionHandler *pH);

	int AddTeam(IFunctionHandler *pH);
	int RemoveTeam(IFunctionHandler *pH);
	int AddToTeam(IFunctionHandler *pH);
	int RemoveFromTeam(IFunctionHandler *pH);
	int SetTeamScoreByEntity(IFunctionHandler *pH);
	int SetTeamScore(IFunctionHandler *pH);
	int GetTeamMemberCount(IFunctionHandler *pH);
	int SetTeamFlags(IFunctionHandler *pH);

	int GetRespawnPoint(IFunctionHandler *pH);
	int GetFirstRespawnPoint(IFunctionHandler *pH);
	int GetNextRespawnPoint(IFunctionHandler *pH);
	int GetPrevRespawnPoint(IFunctionHandler *pH);
	int GetRandomRespawnPoint(IFunctionHandler *pH);
	int GetName(IFunctionHandler *pH);
	int DebugTest(IFunctionHandler *pH);

	int BroadcastCommand(IFunctionHandler *pH);
  static void InitializeTemplate(IScriptSystem *pSS);
private:
	static void MakeTagScriptObject(ITagPoint* pInTagPoint, _SmartScriptObject& rOut);

	CXServer *m_pServer;
	IScriptObject *m_pSlotMap;
	IXSystem *m_pXSystem;
	CXGame *m_pGame;
};
#endif //_SCRIPTOBJECTCLIENT_H_