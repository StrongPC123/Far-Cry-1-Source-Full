
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:EntitySystem.h
//
//	History: 
//	-March 07,2001:Originally created by Marco Corbetta
//	-: modified by everyone
//
//
//////////////////////////////////////////////////////////////////////

#ifndef ENTITYSYSTEM_H
#define ENTITYSYSTEM_H

#if _MSC_VER > 1000
# pragma once
#endif

#include <map>
#include <set>
#include <vector>
#include <IEntitySystem.h>
#include "Entity.h"
#include "EntityCamera.h"
#include "IDGenerator.h"
#include <ISystem.h>
//#include "EntityIt.h"

class CEntity;
class CStaticObject;
class CStatObjInfo;
class CEntityClonesMgr;

#ifdef WIN64
// workaround for Amd64 compiler
#include <map>
#define hash_map map 
#else
#if defined(LINUX)
#include <ext/hash_map>
#include "ientityrenderstate.h"
#else
#include <hash_map>
#endif
#endif

#if defined(LINUX)
	typedef __gnu_cxx::hash_map<EntityId,CEntity*> EntityMap;
	typedef __gnu_cxx::hash_map<EntityId,CEntity*>::iterator EntityMapItor;
#else
	typedef std::hash_map<EntityId,CEntity*> EntityMap;
	typedef EntityMap::iterator EntityMapItor;
#endif

typedef std::vector<CEntity*> EntityVector;
typedef EntityVector::iterator EntityVectorItor;

typedef std::set<int> EntityIdSet;
typedef EntityIdSet::iterator EntityIdSetItor;

typedef std::list<CEntityObject*> EntityObjectList;
typedef std::list<IEntitySystemSink *> SinkList;

//typedef std::set<CEntityClonesMgr *> EntityClonesMgrSet;
//typedef EntityClonesMgrSet::iterator EntityClonesMgrSetItor;
/*
struct near_info_struct
{

	near_info_struct() {model=0;}
	ICryCharInstance * model;
	Vec3d pos, angles;
	list2<CStaticObject> * _stat_objects;
	list2<CStaticObject> * _stat_shadows;
	list2<CStaticObject> * _stat_shadows_plus;
	IEntity * o;
	float ent_distance;
};*/

//////////////////////////////////////////////////////////////////////////
struct SEntityTimerEvent
{
	int entityId;
	int timerId;
};

//////////////////////////////////////////////////////////////////////
class CEntitySystem : public IEntitySystem
{
	void InsertEntity( EntityId id,CEntity *pEntity );
//	void GenerateUniqId( CEntityDesc &ed ) { ed.id = m_LastId++; }
	void DeleteEntity( IEntity *entity );
//	void GarbageCollectorCycle();
public:
	CEntitySystem(ISystem *pSystem);
	~CEntitySystem();

	bool Init(ISystem *pSystem);
	void Release();	// Close entity system, free resources.
	void Reset();	// Reset whole entity system, and destroy all entities.
	
	void SetSink( IEntitySystemSink *pSink );
	void RemoveSink( IEntitySystemSink *pSink );

	void OnBind(EntityId id,EntityId child,unsigned char param);
	void OnUnbind(EntityId id,EntityId child,unsigned char param);

	//void SetMyPlayer(unsigned short id ){ m_wPlayerID = id;}
	//unsigned short GetMyPlayer() const {return m_wPlayerID;}
	
	IScriptSystem *GetScriptSystem() { return m_pScriptSystem;	}
	
	IEntity* SpawnEntity( CEntityDesc &ed,bool bAutoInit=true );
	bool InitEntity( IEntity* pEntity,CEntityDesc &ed );

	// Safely remove entity; Call to this function will not immediatly remove entity;
	// Entity will only be removed after update call unless bRemoveNow set.
	void	RemoveEntity( EntityId entity,bool bRemoveNow );

	IEntityCamera * CreateEntityCamera();

//	near_info_struct * GetWeaponInfo() { return &m_WeaponInfo; };

  IEntity* GetEntity( EntityId id );	// Get entity from id.
	IEntity* GetEntity(const char *sEntityName);
	EntityId FindEntity( const char *name ) const;	// Find first entity with given name.
	int GetNumEntities() const;
	//void GetEntities( std::vector<IEntity*> &entities ) const;

	IEntityIt * GetEntityIterator();
	IEntityIt * GetEntityInFrustrumIterator( bool bFromPrevFrame=false );

	void GetEntitiesInRadius( const Vec3d &origin,float radius,std::vector<IEntity*> &entities,int physFlags ) const;

	void Update();
	void UpdateTimers();

	// Sets new entity timer event.
	void	PauseTimers(bool bPause,bool bResume=false);
	void	RemoveTimerEvent( EntityId id );
	void	AddTimerEvent( int delayTimeMillis,SEntityTimerEvent &event );

	void EnableClient(bool bEnable)
	{
		m_bClient=bEnable;
	}
	void EnableServer(bool bEnable)
	{
		m_bServer=bEnable;
	}

	inline bool ClientEnabled(){return m_bClient;}
	inline bool ServerEnabled(){return m_bServer;}
	void GetMemoryStatistics(ICrySizer *pSizer);
	
	virtual bool IsIDUsed(EntityId nID);

	void ReleaseMark(unsigned int id);
	void ResetEntities(void);

	virtual void SetDynamicEntityIdMode( const bool bActivate );	

	void SetDefaultEntityUpdateLevel( EEntityUpdateVisLevel eDefault) { m_eDefaultUpdateLevel = eDefault;}
//	near_info_struct m_WeaponInfo;

	void SetPrecacheResourcesMode( bool bPrecaching );

//	CIDGenerator* GetIDGenerator() { return &m_EntityIDGenerator; }
	bool	IsDynamicEntityId( EntityId id )  { return m_EntityIDGenerator.IsDynamicEntityId( id ); }
	void	MarkId( EntityId id )		{ m_EntityIDGenerator.Mark( id ); }
	void	ClearId( EntityId id )		{ m_EntityIDGenerator.Remove( id ); }


private:
	// Return true if updated.
	void UpdateEntity(CEntity *ce,SEntityUpdateContext &ctx);

	//////////////////////////////////////////////////////////////////////////
	// Variables.
	//////////////////////////////////////////////////////////////////////////

	bool										m_bDynamicEntityIdMode;		//!< default=false (editor), true=game mode
	CIDGenerator						m_EntityIDGenerator;
	ISystem *								m_pISystem;
	EEntityUpdateVisLevel		m_eDefaultUpdateLevel;

	SinkList								m_lstSinks;
	EntityMap								m_mapEntities;
	EntityVector						m_vEntitiesInFrustrum;
	//[kirill] - need this one to get visible entities on update of some entity - so we don't depend on update's order 
	EntityVector						m_vEntitiesInFrustrumPrevFrame;

	//	unsigned short	m_wPlayerID;
	IScriptSystem *					m_pScriptSystem;

	//////////////////////////////////////////////////////////////////////////
	// Entity timers.
	//////////////////////////////////////////////////////////////////////////
	typedef std::multimap<int,SEntityTimerEvent> EntityTimersMap;
	EntityTimersMap m_timersMap;
	std::vector<SEntityTimerEvent> m_currentTriggers;
	bool	m_bTimersPause;
	int		m_nStartPause;
	//////////////////////////////////////////////////////////////////////////

public:
	struct ITimer *m_pTimer;
	ICVar *m_pCharacterIK;
	ICVar *m_pShowEntityBBox;
	ICVar *m_pShowHelpers;
	ICVar *m_pProfileEntities;
	ICVar *m_pUpdateInvisibleCharacter;
	ICVar *m_pUpdateBonePositions;
	ICVar *m_pUpdateScript;
	ICVar *m_pUpdateTimer;
	ICVar *m_pDebugTimer;
	ICVar *m_pUpdateCamera;
	ICVar *m_pUpdatePhysics;
	ICVar *m_pUpdateAI;
	ICVar *m_pUpdateEntities;
	ICVar *m_pUpdateCollision;
	ICVar *m_pUpdateCollisionScript;
	ICVar *m_pUpdateContainer;
	ICVar *m_pUpdateCoocooEgg;
	ICVar *m_pPiercingCamera;
	ICVar *m_pVisCheckForUpdate;
	ICVar *m_pEntityBBoxes;
	ICVar *m_pEntityHelpers;
	ICVar *m_pOnDemandPhysics;
	ICVar *m_pMinImpulseVel;
	ICVar *m_pImpulseScale;
	ICVar *m_pMaxImpulseAdjMass;
	ICVar *m_pSplashThreshold;
	ICVar *m_pSplashTimeout;
	ICVar *m_pHitCharacters;
	ICVar *m_pHitDeadBodies;
	ICVar *m_pEnableCloth;
	ICVar *m_pCharZOffsetSpeed;
	bool m_bClient;
	bool m_bServer;
	int m_nGetEntityCounter;
//	EntityClonesMgrSet m_setEntityClonesMgrs;

};

//////////////////////////////////////////////////////////////////////////
// Precache resources mode state.
//////////////////////////////////////////////////////////////////////////
extern bool gPrecacheResourcesMode;

#endif //Entitysystem.h
