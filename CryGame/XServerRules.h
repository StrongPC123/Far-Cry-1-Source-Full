//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XServerRules.h
//  Description: Server rules class.
//
//  History:
//  - August 9, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_XSERVERRULES_H
#define GAME_XSERVERRULES_H
#if _MSC_VER > 1000
# pragma once
#endif

// Forward declarations.
class CLuaState;
class CXGame;
struct SWeaponHit;
struct IScript;
struct ICVar;
struct IScriptObject;
///////////////////////////////////////////////
#define TEAM_HAS_NOT_CHANGED -1
#define SPECTATORS_TEAM 0
#include "ScriptObjectServerSlot.h"

class CXServerRules
{
public:
	CXServerRules();
	virtual ~CXServerRules();
	
	//!	Load the rules set by the console variables
	bool	Init(CXGame *pGame, IConsole *pConsole,IScriptSystem *pScriptSystem, ILog *pLog = NULL);
  void	Update();
	//! Unload the rules
	void	ShutDown();

  void CallVote(CScriptObjectServerSlot &sss, char *command, char *arg1);
  void Vote(CScriptObjectServerSlot &sss, int vote);
  void Kill(CScriptObjectServerSlot &sss);
  void MapChanged();
	//! \return 0 if the class is not initialized (e.g. during loading)
  const char *GetGameType();
	IScriptObject *GetScriptObject() { return m_pGameRulesObj; };

  void  PrintEnterGameMessage(const char *playername,int color);
  void  OnHitObject( const SWeaponHit &hit );
	void  OnHitPlayer( const SWeaponHit &hit );
  
	//! When new player connected.
	int	OnClientConnect(IScriptObject *pSS,int nRequestedClassID);
	//! When new player connected.
	void	OnClientDisconnect( IScriptObject *pSS );
	void	OnClientRequestRespawn( IScriptObject *pSS , const EntityClassId nRequestedClassID);
	//! When player respawn after death.
	void	OnPlayerRespawn( IEntity *player );
	//! When player try to change team
	int OnClientMsgJoinTeamRequest(CXServerSlot *pSS,BYTE nTeamId,const char *sClass);
	int OnClientCmd(CXServerSlot *pSS,const char *sCmd);
	//! when a spectator whant to switch spectating mode
	void OnSpectatorSwitchModeRequest(IEntity *spec);
	void OnClientMsgText(EntityId sender,TextMessage &tm);
	void SetGameStuffScript(string sName);
	string GetGameStuffScript();

	void SendEntityTextMessage(EntityId sender,TextMessage &tm);
	void SendTeamTextMessage(EntityId sender,TextMessage &tm);
	void SendWorldTextMessage(EntityId sender,TextMessage &tm);
//	void SendEntityCommand(EntityId sender,ClientCommand &cc);
//	void SendTeamCommand(EntityId sender,ClientCommand &cc);

	//! After the map and its entities have been loaded
	void	OnAfterLoad();

	

public:
	// console variable used to set the rules
	IConsole *m_pConsole;
	/*ICVar* m_pcvRuleProfile;
	ICVar* m_pcvRuleTask;
	ICVar* m_pcvRuleTaskNum;
	ICVar* m_pcvRuleModGame;
	ICVar* m_pcvScoreBlue;
	ICVar* m_pcvScoreRed;
	ICVar* m_pcvDomStat;
	ICVar* m_pcvDomPoints;
	ICVar* m_pcvgFriendlyFire;
	ICVar* m_pcvgForceRespawn;
	ICVar* m_pcvgWeaponsStay;
	ICVar* m_pcvgFallingDamage;
	ICVar* m_pcvgSameMap;
	ICVar* m_pcvgDropableItems;
	ICVar* m_pcvgAllowPowerups;*/

private:
	//! Get rules script.
	//IScriptObject *GetRulesTable() const;
	//CScriptObjectServerSlot m_ScriptObjectServerSlot;
	CXGame *m_pGame;
	IScriptSystem *m_pScriptSystem;
	IScriptObject *m_pGameRulesObj;
	bool m_init;
	string m_sGameStuffScript;

	//! load the GameRules for the given gametype
	//! /param inszGameType gametype e.g. "Default" or "TDM", must not be 0
	//! /return true=success, false=failed
	bool ChangeGameRules( const char *inszGameType );
};

#endif // GAME_XSERVERRULES_H
