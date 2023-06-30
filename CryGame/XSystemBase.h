//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XSystemBase.h
//  Description: Base class inherited by all CXSystem implementations.
//
//  History:
//  - August 8, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_XSYSTEMBASE_H
#define GAME_XSYSTEMBASE_H
#if _MSC_VER > 1000
# pragma once
#endif



class CXEntity;
class CXGame;
class CTeamMgr;
struct ILog;
struct IXMLDOMNode;
struct IConsole;
struct IEntitySystem;

class	CWeaponSystem;
class	CVehicleSystem;
class	CPlayerSystem;

struct Team
{
	Team()
	{
		nID=-1;
		sName="invalid";
		nScore=0;
		nFlags=0;
	}
	Team(const int id,const char *name,const int score,const int flags)
	{
		nID=id;
		sName=name;
		nScore=score;
		nFlags=flags;
	}
	Team(const Team& t)
	{
			sName=t.sName;
			nScore=t.nScore;
			nFlags=t.nFlags;
			nID=t.nID;
			m_setEntities=t.m_setEntities;
	}
	bool Write(CStream &stm);
	bool Read(CStream &stm);
	string sName;
	int nScore;
	int nFlags;
	int nID;
	EntitiesSet m_setEntities;
};

typedef std::map<int,Team> TeamsMap;
typedef TeamsMap::iterator TeamsMapItor;

//////////////////////////////////////////////////////////////////////////
struct EntityEventTarget
{
	int target;
	string event;
	string sourceEvent;
		
	EntityEventTarget() {};
	EntityEventTarget( const EntityEventTarget &e ) { *this = e; }
	void operator=( const EntityEventTarget &e ) {
		target = e.target;
		event = e.event;
		sourceEvent = e.sourceEvent;
	};
};

typedef std::vector<EntityEventTarget> eventTargetVec;
typedef eventTargetVec::iterator eventTargetVecIt;

// stores data needed to spawn/remove entities at runtime
//////////////////////////////////////////////////////////////////////////
#include "XSystemEntities.h"

//////////////////////////////////////////////////////////////////////////
class CXSystemBase : public IXSystem
{
public:
	struct SMissionInfo
	{
		bool bEditor;
		string sLevelFolder;
		string sLevelName;
		string sMissionName;
		string sMissionFilename;
		int dwProgressBarRange;
		XDOM::IXMLDOMDocumentPtr pLevelDataXML;
		XDOM::IXMLDOMDocumentPtr pMissionXML;
		unsigned int m_dwLevelDataCheckSum;
		unsigned int m_dwMissionCheckSum;

		void SetLevelFolder( const char *szLevelPath );
	};

	//! constructor
	CXSystemBase(CXGame *pGame,ILog *pLog);
	//! destructor
	virtual ~CXSystemBase();

	//// IXSystem ///////////////////////////////
	IEntity*	GetEntity(EntityId wID);
	IEntity*	GetEntity(const char *sEntity);
	IEntityIt *GetEntities();
	EntitiesSet& GetPlayerEntities() { return m_setPlayerEntities; }
	IEntity *GetLocalPlayer();
	int GetSurfaceIDByMaterialName(const char *sMaterialName)
	{
		return m_pGame->m_XSurfaceMgr.GetSurfaceIDByMaterialName(sMaterialName);
	}
	bool IsLevelEntity(EntityId id){EntityIdSet::iterator it=m_setLevelEntities.find(id); return it!=m_setLevelEntities.end(); }
	bool EntityExists(WORD id);
	unsigned short GetLevelDataCheckSum(){return m_wCheckSum;}
	virtual void OnSpawn(IEntity *ent, CEntityDesc & ed);
	virtual void OnSpawnContainer( CEntityDesc &ed,IEntity *pEntity );
	virtual void OnRemove(IEntity *ent);
	virtual void OnSetVar(ICVar *pVar) {};
	virtual void SetVariable(const char *sName,const char *sValue) {};

	//! Do common for Client and Server things when opening a new level.
	bool LoadLevelCommon( SMissionInfo &missionInfo );
	bool LoadMaterials(XDOM::IXMLDOMDocument *doc);
	bool LoadLanguageTable(const char *szLevelDir,const char *szMissionName);
	bool LoadLevelEntities( SMissionInfo &missionInfo );

	virtual void BindEntity(EntityId idParent,EntityId idChild){};
	virtual void UnbindEntity(EntityId idParent,EntityId idChild){};
	void	StartLoading(bool bEditor);
	void	EndLoading(bool bEditor);
	virtual int	AddTeam(const char *sTeam, int nTeamId=-1);
	int		GetTeamScore(int nTeamId);
	int		GetTeamFlags(int nTeamId);
	int		GetEntityTeam(int nEntity);
	bool	GetTeamName(int nTeamId,char *ret);
	int		GetTeamId(const char *name);
	int		GetTeamMembersCount(int nTeamiId);
	void	SetTeam(EntityId nEntId, int nTeamId);
	void	RemoveTeam(int nTeamId);
	void	SetTeamScore(int nTeamId, short nScore);
	void	SetTeamFlags(int nTeamId, int nFlags);
	bool	ReadTeams(CStream &stm);
	void	GetMission( XDOM::IXMLDOMDocument *doc,const char *sRequestedMission,SMissionInfo &missionInfo );
	bool	SpawnEntityFromXMLNode(XDOM::IXMLDOMNodePtr pNode,CEntityStreamData *pData);

	CXGame *								m_pGame;						//!< the game ptr
	ISystem *								m_pSystem;					//!<
	ILog *									m_pLog;							//!<
	IEntitySystem *					m_pEntitySystem;		//!< shortcut to the entity system
	CTeamMgr *							m_pTeamMgr;					//!<

private: // ----------------------------------------------------------------

	ITexPic *								m_pPrevConsoleImg;	//!<
	ITexPic *								m_pLoadingImg;			//!<

protected:

	void AddMPProtectedFiles( SMissionInfo &missionInfo );

	//! Can be overriden by client and server.
	//! Tells that level is about to be loaded.
	virtual void OnReadyToLoadLevel( SMissionInfo &missionInfo ) {};

	void LoadMusic( SMissionInfo &missionInfo );
	void LoadXMLNode(XDOM::IXMLDOMNode *pNode, bool bSpawn);
	void AddWeaponPack(XDOM::IXMLDOMNode *pPack);
	void InitRegistry(const char *szLevelDir);

	void SetEntityProperties( IEntity *entity, XDOM::IXMLDOMNode* pEntityTag );
	void RecursiveSetEntityProperties(_SmartScriptObject *pRoot, XDOM::IXMLDOMNodeList* pProps);

	void SetEntityEvents( IEntity *entity,struct XDOM::IXMLDOMNodeList* pEventsNode );	
	void SpawnStaticEntity(	XDOM::IXMLDOMNodePtr pEntityNode );
	int	EventNameToId( const char *str );
	void BindChildren();
	virtual void AddRespawnPoint(ITagPoint *pt){};

	// ------------------------------------------------------------------

	std::map< int, int >					m_ChildParentMap;				//!< 
	typedef std::set<EntityId>		EntityIdSet;						//!< 
	EntityIdSet										m_setLevelEntities;			//!< 
	TeamsMap											m_mapTeams;							//!< 
	IConsole *										m_pConsole;							//!< 
	unsigned short 								m_wCheckSum;						//!< level data checksum
	EntitiesSet 									m_setPlayerEntities;		//!< 
	CEntityStreamDataList					m_lstStreamEntities;		//!< 
};

#endif // GAME_XSYSTEMBASE_H
