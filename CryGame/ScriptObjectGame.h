
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// ScriptObjectGame.h: interface for the CScriptObjectGame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTGAME_H__52FF12D6_6378_4A6E_AA1F_A7867997F86A__INCLUDED_)
#define AFX_SCRIPTOBJECTGAME_H__52FF12D6_6378_4A6E_AA1F_A7867997F86A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>
#include <ScriptObjectVector.h>

class CXGame;

#define PICK_SELONLY		0x00000001
#define PICK_SELADD			0x00000002
#define	PICK_SELSUB			0x00000003

class CScriptObjectRenderer;
typedef std::vector<CScriptObjectRenderer *> SORVec;
/*! This class implements script-functions for exposing the Game functionalities

	REMARKS:
	After initialization of the script-object it will be globally accessable through scripts using the namespace "Game".
		
	Example:
		local players=Game.GetPlayers();

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectGame :
public _ScriptableEx<CScriptObjectGame>
{
public: 
	CScriptObjectGame();
	virtual ~CScriptObjectGame();
	void Init(IScriptSystem *pScriptSystem,CXGame *pGame);
	void Reset();

	void OnNETServerFound(CIPAddress &ip, SXServerInfos &pServerInfo);

	void OnNETServerTimeout(CIPAddress &ip);

public:
	int GetCDPath(IFunctionHandler *pH);
	int GetUserName(IFunctionHandler *pH);
	int ReloadMaterials(IFunctionHandler *pH);
	int GetRandomRespawnPoint(IFunctionHandler *pH); //void (return vector)
	int RefreshServerList(IFunctionHandler *pH); //void (return void)
	int ClearServerInfo(IFunctionHandler *pH);
  int GetServerInfo(IFunctionHandler *pH);
	int GetServerListInfo(IFunctionHandler *pH);
//	int ConnectToRConServer(IFunctionHandler *pH);
	int ExecuteRConCommand(IFunctionHandler *pH);
	int GetPlayers(IFunctionHandler *pH); //void (return void)
	int SetHUDFont(IFunctionHandler *pH);
	int GetHudStringSize(IFunctionHandler *pH);
	int WriteHudNumber(IFunctionHandler *pH);
	int WriteHudString(IFunctionHandler *pH);	
	int WriteHudStringFixed(IFunctionHandler *pH);
	int GetActions(IFunctionHandler *pH);
	int IsPlayer(IFunctionHandler *pH);
	int	GetServerList(IFunctionHandler *pH);
	int	__RespawnEntity(IFunctionHandler *pH);
	int ListPlayers(IFunctionHandler *pH);
	int CheckMap(IFunctionHandler *pH);
	int GetMapDefaultMission(IFunctionHandler *pH);
	int CleanUpLevel(IFunctionHandler *pH);	// unload the level
	//////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////			
	int	LoadStreamingSound(IFunctionHandler *pH);
	int StopMusic(IFunctionHandler *pH);

	int SetTimer(IFunctionHandler *pH);
	int KillTimer(IFunctionHandler *pH);

	/////////////////////////////////////////////////////////////		
	int GetWeaponClassIDByName(IFunctionHandler *pH);

	/////////////////////////////////////////////////////////////		
	int PickEntities(IFunctionHandler *pH);

	/////////////////////////////////////////////////////////////		
	int GetEntitiesScreenSpace(IFunctionHandler *pH);
	int GetPlayerEntitiesInRadius(IFunctionHandler *pH);
	int DrawRadar(IFunctionHandler *pH);
	int DrawHalfCircleGauge(IFunctionHandler *pH);
	/////////////////////////////////////////////////////////////		
	int ShowIngameDialog(IFunctionHandler *pH);
	int HideIngameDialog(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 

	int EnableUIOverlay(IFunctionHandler *pH);
	int IsUIOverlay(IFunctionHandler *pH);

	//------------------------------------------------------------------------------------------------- 
	int EnableQuicksave(IFunctionHandler *pH);

	/////////////////////////////////////////////////////////////		
	int GetEntityTeam(IFunctionHandler *pH);
	int GetTeamScore(IFunctionHandler *pH);
	int GetTeamFlags(IFunctionHandler *pH);
	/////////////////////////////////////////////////////////////		

	int CreateVariable(IFunctionHandler *pH);//str
	int SetVariable(IFunctionHandler* pH);
	int RemoveVariable(IFunctionHandler *pH);//str
	int GetVariable(IFunctionHandler* pH);
	/////////////////////////////////////////////////////////////
	int Connect(IFunctionHandler *pH);
	int Reconnect(IFunctionHandler *pH);
	int Disconnect(IFunctionHandler *pH);
	int GetLevelList (IFunctionHandler* pH);
	int LoadLevel(IFunctionHandler *pH);
	int LoadLevelListen(IFunctionHandler *pH);
	int LoadLevelMPServer(IFunctionHandler *pH);  

	int GetVersion(IFunctionHandler *pH);
	int GetVersionString(IFunctionHandler *pH);

	int ReloadScripts(IFunctionHandler *pH);
	int Load(IFunctionHandler *pH);
	int Save(IFunctionHandler *pH);
	int LoadLatestCheckPoint(IFunctionHandler *pH);
	int ShowSaveGameMenu(IFunctionHandler *pH);
	int Quit(IFunctionHandler *pH);
	int IsPointInWater(IFunctionHandler *pH);
	int GetWaterHeight(IFunctionHandler *pH);
	int GetTagPoint(IFunctionHandler *pH);
	/////////////////////////////////////////////////////////////
	int IsServer(IFunctionHandler *pH);
	int IsClient(IFunctionHandler *pH);
	int IsMultiplayer(IFunctionHandler *pH);
	int GetMaterialIDByName(IFunctionHandler *pH);
	int ReloadMaterialPhysics(IFunctionHandler *pH);
	int StartRecord(IFunctionHandler *pH);
	int StopRecord(IFunctionHandler *pH);
	///////////////////////////////////////////////////////////////
	int Say(IFunctionHandler *pH);
	int SayTeam(IFunctionHandler *pH);
	int SayOne(IFunctionHandler *pH);
	int DisplayNetworkStats(IFunctionHandler *pH);
	int ForceScoreBoard(IFunctionHandler *pH);
	int GetMaterialBySurfaceID(IFunctionHandler *pH);

	int ReloadWeaponScripts(IFunctionHandler *pH);
	int AddWeapon(IFunctionHandler *pH);

	int SetViewAngles(IFunctionHandler *pH);
	int DumpEntities(IFunctionHandler *pH);
	int TouchCheckPoint(IFunctionHandler *pH);

	int GetSaveGameList(IFunctionHandler *pH);
	int ToggleMenu(IFunctionHandler *pH);
	int ShowMenu(IFunctionHandler *pH);
	int HideMenu(IFunctionHandler *pH);
	int IsInMenu(IFunctionHandler *pH);
//	int TraceGrenade(IFunctionHandler *pH);

	///NEW STUFF
	int SendMessage(IFunctionHandler *pH);
	int GetEntityClassIDByClassName(IFunctionHandler *pH);
	int SetCameraFov(IFunctionHandler *pH);
	int GetCameraFov(IFunctionHandler *pH);
	int ApplyStormToEnvironment(IFunctionHandler * pH);
	int CreateExplosion(IFunctionHandler *pH);
	int DrawLabel(IFunctionHandler *pH);
	int ForceEntitiesToSleep(IFunctionHandler *pH);
	int GetInstantHit(IFunctionHandler *pH);
	int GetMeleeHit(IFunctionHandler *pH);
	int SaveConfiguration(IFunctionHandler *pH);
	int LoadConfiguration(IFunctionHandler *pH);
	int LoadConfigurationEx(IFunctionHandler *pH);
	int RemoveConfiguration(IFunctionHandler *pH);
	//int SetListener(IFunctionHandler *pH);
	int DrawHealthBar(IFunctionHandler *pH);
	int LoadScript(IFunctionHandler *pH);
	int CreateRenderer(IFunctionHandler *pH);
	static void InitializeTemplate(IScriptSystem *pSS);
	int StartDemoPlay(IFunctionHandler *pH);
	int StopDemoPlay(IFunctionHandler *pH);
	int GetLevelName(IFunctionHandler *pH);
	int AddCommand(IFunctionHandler *pH);

	int SavePlayerPos(IFunctionHandler *pH);
	int LoadPlayerPos(IFunctionHandler *pH);

	int GetModsList(IFunctionHandler * pH);
	int LoadMOD(IFunctionHandler * pH);
	int GetCurrentModName(IFunctionHandler * pH);


private: // ------------------------------------------------------------------------------

	CXGame *											m_pGame;
	ISystem *											m_pSystem;
	IConsole *										m_pConsole;
	IRenderer *										m_pRenderer;
	I3DEngine *										m_p3DEngine;
	IInput *											m_pInput;
	IEntitySystem *								m_pEntitySystem;
	IPhysicalWorld *							m_pPhysicalWorld;
	IScriptObject *								m_psoNavigationPoint;
	IScriptObject *								m_psoVector;
	SORVec												m_vRenderersObjs;
	CScriptObjectVector						m_pGetTagPoint;
	std::vector<IScriptObject*>		m_pPlayersPool;	//!< This is pool of script objects passed back on request for players in radius.

	bool _GetProfileFileNames( IFunctionHandler *pH, string &outSystem, string &outGame, const char *insCallerName );

public: // ------------------------------------------------------------------------------

	int SetThirdPerson(IFunctionHandler * pH);
	int SoundEvent(IFunctionHandler * pH);
	int PlaySubtitle(IFunctionHandler * pH);
};

#endif // !defined(AFX_SCRIPTOBJECTGAME_H__52FF12D6_6378_4A6E_AA1F_A7867997F86A__INCLUDED_)
