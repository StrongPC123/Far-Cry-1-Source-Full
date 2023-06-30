//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: IXSystem.h
//  Description: Pure system interface.
//
//  History:
//  - August 8, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef GAME_IXSYSTEM_H
#define GAME_IXSYSTEM_H
#if _MSC_VER > 1000
# pragma once
#endif

#include <EntityDesc.h>
//////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
struct IEntity;
struct ICVar;

typedef std::set<int>					EntitiesSet;
typedef EntitiesSet::iterator	EntitiesSetItor;

#define MAXTEAMS 8
//////////////////////////////////////////////////////////////////////////////////////////////
// IXSystem interface
struct IXSystem
{
	//! delete the class
	virtual void		Release() = 0;
	//! load the level [EDITOR ONLY]
	virtual bool		LoadLevel(const char *szLevelDir,const char *szMissionName = 0,bool bEditor=false) = 0;
	//! add in the map and set the ID
	virtual	IEntity*	SpawnEntity(class CEntityDesc &ed) = 0;
	//! remove from the map and the entity system of the framework
	virtual	void		RemoveEntity(EntityId wID, bool bRemoveNow=false) = 0;
	//! remove and delete all existing entities still registered
	virtual void		DeleteAllEntities() = 0;
	//! called on a disconnection
	virtual void		Disconnected(const char *szCause) = 0;
	//! sets the team of an entity
	virtual	void		SetTeam(EntityId nEntId, int nTeamId) = 0;
	//! adds a team(return the new team id)
	virtual	int		AddTeam(const char *sTeam, int nTeamId=-1) = 0;
	//! removes a team
	virtual	void		RemoveTeam(int nTeamId) = 0;
	//! set team score
	virtual	void		SetTeamScore(int nTeamId,short nScore) = 0;
	//! set team flags
	virtual	void		SetTeamFlags(int nTeamId,int nFlags) = 0;
	//! get team score
	virtual	int		GetTeamScore(int nTeamId) = 0;
	//! get team flags
	virtual	int		GetTeamFlags(int nTeamId) = 0;
	//! get team score
	virtual	int		GetEntityTeam(int nEntity) = 0;
	//! get team name
	virtual	bool GetTeamName(int nTeamId,char *ret) = 0;
	//! get team id
	virtual	int GetTeamId(const char *name) = 0;
	//! get the number of players in a team
	virtual	int GetTeamMembersCount(int nTeamiId) = 0;
	//! serialize info for the context setup
	virtual bool	WriteTeams(CStream &stm) = 0;
	//! read infos from the context setup
	virtual bool	ReadTeams(CStream &stm) = 0;
	//!bind an entity to another one
	virtual void		BindEntity(EntityId idParent,EntityId idChild)=0;
	//!unbind an entity from another one
	virtual void		UnbindEntity(EntityId idParent,EntityId idChild)=0;
	//! return a pointer to the entity matching with entity ID
	virtual IEntity*	GetEntity(EntityId wID) = 0;
	//! retrun a pointer to the entity matching with a certain name
	virtual IEntity*	GetEntity(const char *sEntity) = 0;
	virtual bool IsLevelEntity(EntityId id) = 0;
	//! return the entity assigned as player of le local client if exists, if not return null
	virtual IEntity *GetLocalPlayer() = 0;
	//! return all entities
	virtual IEntityIt	*GetEntities() = 0;
	//! return all player entities
	virtual EntitiesSet& GetPlayerEntities() = 0;
	//! return true if the given entity is in the entity list
	virtual bool		EntityExists(WORD id) = 0;
	virtual unsigned short GetLevelDataCheckSum() = 0;
	// parses the properties from XML and fills the corresponding table with the values
	virtual void SetEntityProperties(IEntity * pEntity,XDOM::IXMLDOMNode * pPropNode) = 0;
	virtual void OnSpawn(IEntity *ent, CEntityDesc & ed)=0;
	virtual void OnSpawnContainer( CEntityDesc &ed,IEntity *pEntity )=0;
	virtual void OnSetVar(ICVar *pVar) = 0;
	virtual void SetVariable(const char *sName,const char *sValue) = 0;
	virtual	void BindChildren()=0;
};

#endif // GAME_IXSYSTEM_H
