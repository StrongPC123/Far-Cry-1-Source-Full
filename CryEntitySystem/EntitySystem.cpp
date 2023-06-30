//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:CEntitySystem.cpp
//  Description: entity's management
//
//	History:
//	-March 07,2001:Originally created by Marco Corbetta
//  -: modified by everyone
//
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include <Cry_Math.h>
#include <list2.h>
#include "EntitySystem.h"
#include "EntityIt.h"
#include <CryPhysics.h>

#include <IScriptSystem.h>

#include "Entity.h"

#include <IRenderer.h>

#include <I3dengine.h>
#include <ILog.h>
#include <ITimer.h>
#include <ISystem.h>
#include <IGame.h>					// IGame

#if defined(_DEBUG) && !defined(LINUX)
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

/*
#include "..\cry3dengine\statobj.h"
#include "file.h"
*/

//////////////////////////////////////////////////////////////////////////
// Global var.
//////////////////////////////////////////////////////////////////////////
bool gPrecacheResourcesMode = false;


//////////////////////////////////////////////////////////////////////
CEntitySystem::CEntitySystem(ISystem *pSystem)
{
	m_bDynamicEntityIdMode=false;

	m_eDefaultUpdateLevel = eUT_Always;
	m_nGetEntityCounter=0;
	m_pISystem=pSystem;
	//m_pSink=NULL;
	m_mapEntities.clear();
	m_pScriptSystem=NULL;
	m_pTimer = pSystem->GetITimer();
	m_pCharacterIK = pSystem->GetIConsole()->CreateVariable("p_characterik","1",VF_CHEAT,
		"Toggles character IK.\n"
		"Usage: p_characterik [0/1]\n"
		"Default is 1 (on). Set to 0 to disable inverse kinematics.");	
	m_pShowEntityBBox = pSystem->GetIConsole()->CreateVariable("es_bboxes","0",VF_CHEAT,//this console variable is repeated
		"Toggles entity bounding boxes.\n"
		"Usage: es_bboxes [0/1]\n"
		"Default is 0 (off). Set to 1 to display bounding boxes.");
	m_pShowHelpers = pSystem->GetIConsole()->CreateVariable("es_helpers","0",VF_CHEAT,
		"Toggles helpers.\n"
		"Usage: es_helpers [0/1]\n"
		"Default is 0 (off). Set to 1 to display entity helpers.");
	m_pProfileEntities = pSystem->GetIConsole()->CreateVariable("es_profileentities","0",VF_CHEAT,
		"\n"
		"Usage: \n"
		"Default is 0 (off).");
	m_pUpdateInvisibleCharacter = pSystem->GetIConsole()->CreateVariable("es_UpdateInvisibleCharacter","0",VF_CHEAT,
		"\n"
		"Usage: \n"
		"Default is 0 (off).");
	m_pUpdateBonePositions = pSystem->GetIConsole()->CreateVariable("es_UpdateBonePositions","1",VF_CHEAT,
		"\n"
		"Usage: \n"
		"Default is 1 (on).");
	m_pUpdateScript = pSystem->GetIConsole()->CreateVariable("es_UpdateScript","1",VF_CHEAT,
		"\n"
		"Usage: \n"
		"Default is 1 (on).");
	m_pUpdatePhysics = pSystem->GetIConsole()->CreateVariable("es_UpdatePhysics","1",VF_CHEAT,
		"Toggles updating of entity physics.\n"
		"Usage: es_UpdatePhysics [0/1]\n"
		"Default is 1 (on). Set to 0 to prevent entity physics from updating.");
	m_pUpdateAI = pSystem->GetIConsole()->CreateVariable("es_UpdateAI","1",VF_CHEAT,
		"Toggles updating of AI entities.\n"
		"Usage: es_UpdateAI [0/1]\n"
		"Default is 1 (on). Set to 0 to prevent AI entities from updating.");
	m_pUpdateEntities = pSystem->GetIConsole()->CreateVariable("es_UpdateEntities","1",VF_CHEAT,
		"Toggles entity updating.\n"
		"Usage: es_UpdateEntities [0/1]\n"
		"Default is 1 (on). Set to 0 to prevent all entities from updating.");
	m_pUpdateCollision= pSystem->GetIConsole()->CreateVariable("es_UpdateCollision","1",VF_CHEAT,
		"Toggles updating of entity collisions.\n"
		"Usage: es_UpdateCollision [0/1]\n"
		"Default is 1 (on). Set to 0 to disable entity collision updating.");
	m_pUpdateContainer= pSystem->GetIConsole()->CreateVariable("es_UpdateContainer","1",VF_CHEAT,
		"\n"
		"Usage: es_UpdateContainer [0/1]\n"
		"Default is 1 (on).");
	m_pUpdateTimer = pSystem->GetIConsole()->CreateVariable("es_UpdateTimer","1",VF_CHEAT,
		"\n"
		"Usage: es_UpdateTimer [0/1]\n"
		"Default is 1 (on).");
	m_pDebugTimer = pSystem->GetIConsole()->CreateVariable("es_DebugTimer","0",VF_CHEAT,
		"\n"
		"Usage: es_DebugTimer [0/1]\n"
		"Default is 0 (on).");
	m_pUpdateCamera = pSystem->GetIConsole()->CreateVariable("es_UpdateCamera","1",VF_CHEAT,
		"Toggles camera updating.\n"
		"Usage: es_UpdateCamera [0/1]\n"
		"Default is 1 (on).");
	m_pUpdateCollisionScript = pSystem->GetIConsole()->CreateVariable("es_UpdateCollisionScript","1",VF_CHEAT,
		"\n"
		"Usage: es_UpdateCollisionScript [0/1]\n"
		"Default is 1 (on).");
	m_pUpdateCoocooEgg = pSystem->GetIConsole()->CreateVariable("es_UpdateCoocooEgg","1",VF_CHEAT,
		"\n"
		"Usage: es_UpdateCoocooEgg [0/1]\n"
		"Default is 1 (on).");
	m_pPiercingCamera = pSystem->GetIConsole()->CreateVariable("es_PiercingCamera","0",VF_CHEAT,
		"\n"
		"Usage: es_PiercingCamera [0/1]\n"
		"Default is 0 (off).");
  m_pVisCheckForUpdate = pSystem->GetIConsole()->CreateVariable("es_VisCheckForUpdate","1",VF_CHEAT,
		"\n"
		"Usage: es_VisCheckForUpdate [0/1]\n"
		"Default is 1 (on).");
  m_pEntityBBoxes = pSystem->GetIConsole()->CreateVariable("es_bboxes","0",VF_CHEAT,//this console variable is repeated, which is the right one
		"\n"
		"Usage: es_bboxes [0/1]\n"
		"Default is 0 (off).");
  m_pEntityHelpers = pSystem->GetIConsole()->CreateVariable("es_helpers","0",VF_CHEAT,
		"\n"
		"Usage: es_helpers [0/1]\n"
		"Default is 0 (off).");
  m_pOnDemandPhysics = pSystem->GetIConsole()->CreateVariable("es_OnDemandPhysics","0",VF_CHEAT,
		"\n"
		"Usage: es_OnDemandPhysics [0/1]\n"
		"Default is 1 (on).");
	m_pMinImpulseVel = pSystem->GetIConsole()->CreateVariable("es_MinImpulseVel","0.0",VF_CHEAT,
		"\n"
		"Usage: es_MinImpulseVel 0.0\n"
		"");
	m_pImpulseScale = pSystem->GetIConsole()->CreateVariable("es_ImpulseScale","0.0",VF_CHEAT,
		"\n"
		"Usage: es_ImpulseScale 0.0\n"
		"");
	m_pMaxImpulseAdjMass = pSystem->GetIConsole()->CreateVariable("es_MaxImpulseAdjMass","2000.0",VF_CHEAT,
		"\n"
		"Usage: es_MaxImpulseAdjMass 2000.0\n"
		"");
	m_pSplashThreshold = pSystem->GetIConsole()->CreateVariable("es_SplashThreshold","1.0",VF_CHEAT,
		"minimum instantaneous water resistance that is detected as a splash"
		"Usage: es_SplashThreshold 200.0\n"
		"");
	m_pSplashTimeout = pSystem->GetIConsole()->CreateVariable("es_SplashTimeout","3.0",VF_CHEAT,
		"minimum time interval between consecutive splashes"
		"Usage: es_SplashTimeout 3.0\n"
		"");
	m_pHitCharacters = pSystem->GetIConsole()->CreateVariable("es_HitCharacters","1",0,
		"specifies whether alive characters are affected by bullet hits (0 or 1)");
	m_pHitDeadBodies = pSystem->GetIConsole()->CreateVariable("es_HitDeadBodies","1",0,
		"specifies whether dead bodies are affected by bullet hits (0 or 1)");
	m_pEnableCloth = pSystem->GetIConsole()->CreateVariable("es_EnableCloth","1",VF_DUMPTODISK,
		"specifies whether cloth will be physicalized (0 or 1)");
	m_pCharZOffsetSpeed = pSystem->GetIConsole()->CreateVariable("es_CharZOffsetSpeed","2.0",VF_DUMPTODISK,
		"sets the character Z-offset change speed (in m/s), used for IK");
	m_bClient=false;
	m_bServer=false;	
	m_bTimersPause=false;
	m_nStartPause=-1;
}

//////////////////////////////////////////////////////////////////////
CEntitySystem::~CEntitySystem()
{
	//ShutDown();
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::InsertEntity( EntityId id, CEntity *pEntity )
{
	EntityMap::iterator it;
	assert( pEntity );
	
	it = m_mapEntities.find(id);

	if (it != m_mapEntities.end())
	{
		//assert(false);
		
		m_pISystem->GetILog()->Log("ENTITY id=%d class=\"%s\" already spawned on this client...override",it->second->GetId(),it->second->GetEntityClassName());
	}
		
//		CConsole::Exit("CEntitySystem::InsertEntity - Entity already in map !");

	m_mapEntities.insert( EntityMap::value_type(id, pEntity) );
}

//////////////////////////////////////////////////////////////////////
bool CEntitySystem::Init(ISystem *pSystem)
{
	if (!pSystem) return false;
	m_pISystem=pSystem;
	//m_pSink=NULL;
	m_lstSinks.clear();
	m_mapEntities.clear();

	m_pScriptSystem=pSystem->GetIScriptSystem();

	m_pScriptSystem->SetGlobalValue("ScriptEvent_Activate", ScriptEvent_Activate);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Deactivate", ScriptEvent_Deactivate);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_FireModeChange", ScriptEvent_FireModeChange);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_DropItem", ScriptEvent_DropItem);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Reset", ScriptEvent_Reset);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Timer", ScriptEvent_Timer);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_StartAnimation", ScriptEvent_StartAnimation);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_AnimationKey", ScriptEvent_AnimationKey);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_EndAnimation", ScriptEvent_EndAnimation);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Respawn", ScriptEvent_Respawn);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_ItemActivated", ScriptEvent_ItemActivated);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_ZoomToggle", ScriptEvent_ZoomToggle);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_ZoomIn", ScriptEvent_ZoomIn);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_ZoomOut", ScriptEvent_ZoomOut);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Hit", ScriptEvent_Hit);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_WeaponReady",ScriptEvent_WeaponReady);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Reload",ScriptEvent_Reload);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Fire", ScriptEvent_Fire);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_StopFiring", ScriptEvent_StopFiring);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_FireCancel", ScriptEvent_FireCancel);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_FireGrenade", ScriptEvent_FireGrenade);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Command", ScriptEvent_Command);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Die", ScriptEvent_Die);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Land", ScriptEvent_Land);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_PhysCollision", ScriptEvent_PhysCollision);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_ViewModeChange", ScriptEvent_ViewModeChange);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_SelectWeapon",ScriptEvent_SelectWeapon);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Deafened", ScriptEvent_Deafened);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_CycleGrenade", ScriptEvent_CycleGrenade);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Use",ScriptEvent_Use);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_MeleeAttack",ScriptEvent_MeleeAttack);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_PhysicalizeOnDemand",ScriptEvent_PhysicalizeOnDemand);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_StanceChange",ScriptEvent_StanceChange);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_FlashLightSwitch", ScriptEvent_FlashLightSwitch);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_EnterWater", ScriptEvent_EnterWater);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_CycleVehiclePos", ScriptEvent_CycleVehiclePos);  
	m_pScriptSystem->SetGlobalValue("ScriptEvent_AllClear", ScriptEvent_AllClear);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Expression", ScriptEvent_Expression);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_InVehicleAnimation", ScriptEvent_InVehicleAnimation);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_InVehicleAmmo", ScriptEvent_InVehicleAmmo);
  m_pScriptSystem->SetGlobalValue("ScriptEvent_ProcessCharacterEffects", ScriptEvent_ProcessCharacterEffects);
	m_pScriptSystem->SetGlobalValue("ScriptEvent_Jump", ScriptEvent_Jump);  

	return true;
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::Release()
{
	Reset();
	delete this;
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::Reset()
{
//	m_pISystem->GetILog()->Log("CEntitySystem::Reset()");

	EntityMap::iterator it = m_mapEntities.begin();
	while (it != m_mapEntities.end())
	{
		CEntity *ent = it->second;
		if (ent)
		{ 
 			ent->ShutDown();
			delete ent;
		}
		m_mapEntities.erase( it );
		it = m_mapEntities.begin();
	}

	m_EntityIDGenerator.Reset();
	m_timersMap.clear();
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::SetSink( IEntitySystemSink *pSink )
{
	if (pSink)
	{
		if (std::find(m_lstSinks.begin(),m_lstSinks.end(),pSink) == m_lstSinks.end())
			m_lstSinks.push_back(pSink);
	}
	//m_pSink=pSink;
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::RemoveSink( IEntitySystemSink *pSink )
{
	m_lstSinks.remove(pSink);
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::DeleteEntity( IEntity *entity )
{
	FUNCTION_PROFILER( m_pISystem,PROFILE_ENTITY );
	if (entity)
	{
		CEntity *ce = ((CEntity *)entity);
//		CLog::LogToConsole( "Entity %s removed (%d,%s)",entity->GetClassName().c_str(),entity->GetId(),entity->GetName().c_str() );
		ce->ShutDown();
		int id = ce->GetId();
		m_mapEntities.erase( id );

//		m_pISystem->GetILog()->Log("CEntitySystem::DeleteEntity %d",id);

		// [kirill] we keep list of entities visible in prev frame.
		// make sure no delited entity is in the list
		// not good to erase from list - can be chaged to vector - let's see
		EntityVector::iterator prevListItr = std::find(m_vEntitiesInFrustrumPrevFrame.begin(), m_vEntitiesInFrustrumPrevFrame.end(), ce);
		if(prevListItr != m_vEntitiesInFrustrumPrevFrame.end())
			m_vEntitiesInFrustrumPrevFrame.erase( prevListItr );

		// If the entity is bound, lets make sure that the ID is not reused before the parent has a chance to 
		// try to update it and discover that it is no longer present. The parent will then release the mark
		// so it will be safely reused.
		if (!entity->IsBound())
			m_EntityIDGenerator.Remove(id);
		delete (CEntity *)entity;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::SetDynamicEntityIdMode( const bool bActivate )
{
	m_bDynamicEntityIdMode=bActivate;
}

//load an entity from script,set position and add to the entity system
//////////////////////////////////////////////////////////////////////
IEntity* CEntitySystem::SpawnEntity( CEntityDesc &ed,bool bAutoInit )
{	
	FUNCTION_PROFILER( m_pISystem,PROFILE_ENTITY );
	//GarbageCollectorCycle();

	CEntity *pEntity =0;

	{

		//======== SPAWN NORMAL ENTITY

		if (ed.id == 0)
		{
			ed.id = m_EntityIDGenerator.GetNewDynamic();	// get entity id and mark it
		}
		else if (ed.id < 0)
		{
			// If negative if, means comming from editor and needs static id.
			ed.id = m_EntityIDGenerator.GetNewStatic();		// get entity id and mark it
		}
		else 
		{
			if(m_pISystem->GetIGame()->GetModuleState(EGameMultiplayer))
			{
				RemoveEntity(ed.id,true);
//				m_pISystem->GetILog()->Log("Entity %s [%d] updated",(const char*)ed.name,ed.id);
			}
			else
			if (m_EntityIDGenerator.IsUsed(ed.id))
			{
				m_pISystem->Warning( VALIDATOR_MODULE_ENTITYSYSTEM,VALIDATOR_WARNING,0,0,"CEntitySystem::SpawnEntity Failed, Can't spawn %s. ID %d is used",
					(const char*)ed.name,ed.id);
				return 0;
			}
			m_EntityIDGenerator.Mark(ed.id); 
		}

		
		IEntityContainer *pContainer=NULL;

		// new entity
		pEntity = new CEntity(this,m_pISystem,m_pScriptSystem);
		pEntity->SetClassId(ed.ClassId);

		//[PETAR] Set default update level
		pEntity->SetUpdateVisLevel(m_eDefaultUpdateLevel);

	//  first, create appropriate container 	
		pEntity->SetID(ed.id);
		pEntity->SetNetPresence(ed.netPresence);

	//	if (m_pSink)
		if (!m_lstSinks.empty())
		{
			for (SinkList::iterator si=m_lstSinks.begin();si!=m_lstSinks.end();si++)
				(*si)->OnSpawnContainer(ed,pEntity);
		}

		
		if(pEntity->GetContainer())
		{
				pEntity->SetClassName(ed.className.c_str());
		}
				
	// put it into the entity map
		InsertEntity( ed.id, pEntity);

		if (!m_lstSinks.empty())
		{
			for (SinkList::iterator si=m_lstSinks.begin();si!=m_lstSinks.end();si++)
				(*si)->OnSpawn(pEntity,ed);
		}

		if (bAutoInit)
		{
			if (!InitEntity( pEntity,ed ))
			{
				return NULL;
			}
#ifdef _DEBUG 
			m_pISystem->GetILog()->LogToFile("Entity spawned %d %s (%f %f %f)",pEntity->GetId(),pEntity->GetEntityClassName(),ed.pos.x,ed.pos.y,ed.pos.z);
#endif
		}
	}

//add the entity in all clones mgr
/*	EntityClonesMgrSetItor itor;
	itor=m_setEntityClonesMgrs.begin();
	while(itor!=m_setEntityClonesMgrs.end())
	{
		CEntityClonesMgr *pECM=(*itor);
		pECM->AddEntity(pEntity);
		++itor;
	}*/

	// Tell the client in lua that an entity has spawned
	if (pEntity->GetScriptObject())
	{
		//Timur, Look ups by name in lua must be replaced with table reference.
		IScriptSystem *pSS = GetScriptSystem();
		_SmartScriptObject pClientStuff(pSS,true);
		if(pSS->GetGlobalValue("ClientStuff",pClientStuff))
		{
			pSS->BeginCall("ClientStuff","OnSpawnEntity");
			pSS->PushFuncParam(pClientStuff);
			m_pScriptSystem->PushFuncParam(pEntity->GetScriptObject());
			pSS->EndCall();
		}
	}

	// set m_bHasEnvLighting flag for player and vehicles now to avoid calling virtual functions during rendering
	void*pDummy=0;
	if(	pEntity->GetContainer() && 
		(	pEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pDummy) ||
			pEntity->GetContainer()->QueryContainerInterface(CIT_IVEHICLE,(void**)&pDummy))) 
		pEntity->SetHasEnvLighting(true);
	else
		pEntity->SetHasEnvLighting(false);

	return (IEntity*)pEntity;
}

//////////////////////////////////////////////////////////////////////////
bool CEntitySystem::InitEntity( IEntity* pEntity,CEntityDesc &ed )
{
	assert( pEntity );
	CEntity *pCEntity = (CEntity*)pEntity;
	// initialize entity
	if (!pCEntity->Init(ed))
	{
		DeleteEntity(pEntity);
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
void CEntitySystem::RemoveEntity( EntityId entity,bool bRemoveNow )
{
///	m_pISystem->GetILog()->Log( "\001** Remove Entity Id %d %c",(int)entity,bRemoveNow?'t':'f' );
	EntityMap::iterator it = m_mapEntities.find(entity);

//	TRACE("Entity %d marked as garbage",entity);
	CEntity *pEntity=NULL;
	if (it != m_mapEntities.end())
	{
		pEntity = it->second;
		if (!m_lstSinks.empty())
		{
			for (SinkList::iterator si=m_lstSinks.begin();si!=m_lstSinks.end();si++)
				(*si)->OnRemove(pEntity);
		}
		pEntity->MarkAsGarbage();

		if (bRemoveNow && pEntity->IsDestroyable())
			DeleteEntity(pEntity);
	}

	/*
	it = m_vStaticEntities.find(entity);
	if (it != m_vStaticEntities.end())
	{
		pEntity = it->second;
//		pEntity->MarkAsGarbage();
		if (pEntity->IsDestroyable()) 
		{
			if (!pEntity->IsBound())
				m_EntityIDGenerator.Remove(pEntity->GetId());
			pEntity->ShutDown();
			m_vStaticEntities.erase(it);
			delete pEntity;
		}
	}
	*/

	//remove the entity in all clones mgr
	/*if(pEntity)
	{
		EntityClonesMgrSetItor itor;
		itor=m_setEntityClonesMgrs.begin();
		while(itor!=m_setEntityClonesMgrs.end())
		{
			CEntityClonesMgr *pECM=(*itor);
			pECM->RemoveEntity(pEntity);
			++itor;
		}
	}*/
}

//////////////////////////////////////////////////////////////////////
IEntity* CEntitySystem::GetEntity( EntityId id ) 
{
	m_nGetEntityCounter++;
	EntityMap::iterator it = m_mapEntities.find(id);
	if (it != m_mapEntities.end())
	{
		// Entity found.
		return (IEntity*)(it->second);
	}

	/*
	it = m_vStaticEntities.find(id);
	if (it != m_vStaticEntities.end())
	{
		return (IEntity*) (it->second);
	}
	*/
	
//  ASSERT(id==0);   // if we have a valid id here, someone is using the id after deleting the ent
  return NULL;
}

//////////////////////////////////////////////////////////////////////
EntityId CEntitySystem::FindEntity( const char *name ) const
{
	EntityMap::const_iterator itor;
	itor=m_mapEntities.begin();
	while(itor!=m_mapEntities.end())
	{
		CEntity *ce = itor->second;
		if (stricmp( ce->GetName(),name) == 0)
		{
			return ce->GetId();
		}
		++itor;
	}

	//CryWarning( "<CryEntitySystem> Entity %s not found",name );
 // if you are here you are doing something wrong!!!!
		// not in any case !
	return 0; //not found
}

//////////////////////////////////////////////////////////////////////////
IEntity* CEntitySystem::GetEntity(const char *sEntityName)
{
	if (!sEntityName || !sEntityName[0])
		return 0; // no entity name specified

	EntityMapItor itor=m_mapEntities.begin();
		while(itor!=m_mapEntities.end())
	{
		CEntity *ce = itor->second;
		if (stricmp( ce->GetName(),sEntityName) == 0)
		{
			return ce;
		}
		++itor;
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////
int CEntitySystem::GetNumEntities() const
{
	return m_mapEntities.size();
}

//////////////////////////////////////////////////////////////////////
/*void CEntitySystem::GetEntities( std::vector<IEntity*> &vecEntities ) const
{
	int i = 0;
	if(m_mapEntities.empty())
	{
		vecEntities.clear();
		return;
	}

	vecEntities.resize( m_mapEntities.size() );
	for (EntityMap::const_iterator it = m_mapEntities.begin(); it != m_mapEntities.end(); ++it)
  {
    vecEntities[i++] = (IEntity*)(it->second);
  }
}*/

//////////////////////////////////////////////////////////////////////
void CEntitySystem::GetEntitiesInRadius( const Vec3d &origin,float radius,std::vector<IEntity*> &entities,int physFlags ) const
{
	assert(m_pISystem);

	IPhysicalEntity **pList;
	Vec3d bmin = origin - Vec3d(radius,radius,radius);
	Vec3d bmax = origin + Vec3d(radius,radius,radius);

	vectorf v_min, v_max;
	v_min.x = bmin.x;
	v_min.y = bmin.y;
	v_min.z= bmin.z;
	v_max.x = bmax.x;
	v_max.y = bmax.y;
	v_max.z= bmax.z;

	int num = m_pISystem->GetIPhysicalWorld()->GetEntitiesInBox( v_min,v_max,pList,physFlags );

	entities.resize( num );
	for (int i = 0; i < num; i++)
	{
		IEntity *ce = (IEntity*)pList[i]->GetForeignData();
		if (ce)
		{
			entities[i] = ce;
		}
	}
}

// update the entity
//////////////////////////////////////////////////////////////////////////
void CEntitySystem::UpdateEntity(CEntity *ce,SEntityUpdateContext &ctx)
{
	if (ce->m_bGarbage)
	{
		if (ce->IsDestroyable())
		{
			DeleteEntity(ce);
		}
		else
		{	// [kirill] we must be in editor gamemode (Timur assures it wil newer happen in game)
			// can't destroy entity - just hide it/remove physics 
			ce->DestroyPhysics();
			ce->DrawObject(0);
			ce->DrawCharacter(0,0);
		}
		return;
	}

	if (ce->m_bHidden)
		return;

	if (ce->m_bTrackable)
	{
		//if (ce->GetDrawFrame() == ctx.nFrameID)
		//if (ce->m_nLastVisibleFrameID == ctx.nFrameID)
    if (abs(int(ctx.nFrameID - ce->m_nLastVisibleFrameID)) < MAX_FRAME_ID_STEP_PER_FRAME)
			m_vEntitiesInFrustrum.push_back(ce);
	}
	
	// Bound entites always updated by thier parents.
	if (ce->m_bIsBound)
		return;
	if (ce->m_eUpdateVisLevel==eUT_PhysicsPostStep)
		return;

	//if (bUpdateEntities)
	{
		bool bNeedUpdate = (ce->m_bUpdate && !ce->m_bSleeping) || (ce->m_awakeCounter > 0);
		if (bNeedUpdate)
		{
			ce->Update(ctx);
		}

		// If have childs.
		if (ce->m_bUpdateBinds)
		{
			// If not updated.
			ce->UpdateHierarchy( ctx );
		}
	}
}

//update the entity system
//////////////////////////////////////////////////////////////////////
void CEntitySystem::Update()
{
	g_bProfilerEnabled = m_pISystem->GetIProfileSystem()->IsProfiling();

	FUNCTION_PROFILER_FAST( m_pISystem,PROFILE_ENTITY,g_bProfilerEnabled );
	//char strNumStatObjs[100];
	//sprintf(strNumStatObjs," %d|Stat",(int)m_vStaticEntities.size() );

	UpdateTimers();

	{
		m_vEntitiesInFrustrumPrevFrame.resize(0);
		//m_vEntitiesInFrustrumPrevFrame.insert(m_vEntitiesInFrustrumPrevFrame.begin(), m_vEntitiesInFrustrum.begin(), m_vEntitiesInFrustrum.end());
		m_vEntitiesInFrustrumPrevFrame = m_vEntitiesInFrustrum;
		m_vEntitiesInFrustrum.resize(0);
	}

	CCamera Cam=m_pISystem->GetViewCamera();
	Vec3d Min, Max;
	EntityMap::iterator next;
	int nRendererFrameID=0;
	if(m_pISystem)
	{
		IRenderer *pRen=m_pISystem->GetIRenderer();
		if(pRen)
			nRendererFrameID=pRen->GetFrameID();
//			nRendererFrameID=pRen->GetFrameUpdateID();
	}
	bool bUpdateEntities=m_pUpdateEntities->GetIVal()?true:false;
	bool bProfileEntities = m_pProfileEntities->GetIVal() >= 1;
	bool bProfileEntitiesToLog = m_pProfileEntities->GetIVal() == 2;
	bool bProfileEntitiesAll = m_pProfileEntities->GetIVal() == 3;
	float fStartTime = 0;

	SEntityUpdateContext ctx;
	ctx.nFrameID = nRendererFrameID;
	ctx.pCamera = &Cam;
	ctx.fCurrTime = m_pISystem->GetITimer()->GetCurrTime();
	ctx.fFrameTime = m_pISystem->GetITimer()->GetFrameTime();
	ctx.bProfileToLog = bProfileEntitiesToLog;
	ctx.numVisibleEntities = 0;
	ctx.numUpdatedEntities = 0;
	ctx.fMaxViewDist = Cam.GetZMax();
	ctx.fMaxViewDistSquared = ctx.fMaxViewDist*ctx.fMaxViewDist;
	ctx.vCameraPos = Cam.GetPos();

	if (!bProfileEntities)
	{	
		for (EntityMap::iterator it = m_mapEntities.begin(); it != m_mapEntities.end();it = next)
		{
			next = it; next++;
			CEntity *ce= it->second;
			UpdateEntity(ce,ctx);
		}
	}
	else
	{		
		if (bProfileEntitiesToLog)
			m_pISystem->GetILog()->Log( "\001================= Entity Update Times =================" );

		char szProfInfo[256];
		float colors[4]={1,1,1,1};
		float colorsYellow[4]={1,1,0,1};
		float colorsRed[4]={1,0,0,1};
		int prevNumUpdated;
		float fProfileStartTime;
		for (EntityMap::iterator it = m_mapEntities.begin(); it != m_mapEntities.end();it = next)
		{
			next = it; next++;
			CEntity *ce= it->second;

			fProfileStartTime = m_pTimer->GetAsyncCurTime();
			prevNumUpdated = ctx.numUpdatedEntities;

			bool bGarbage = ce->m_bGarbage;

			UpdateEntity(ce,ctx);

			if (bGarbage)
				continue;

			if (prevNumUpdated != ctx.numUpdatedEntities || bProfileEntitiesAll)
			{
				float time = m_pTimer->GetAsyncCurTime() - fProfileStartTime;
				if (time < 0)
					time = 0;

				if (bProfileEntitiesToLog)
					GetISystem()->GetILog()->Log( "\001%.3f ms : %s (%s)",time*1000.0f,ce->GetName(),ce->GetEntityClassName() );

				sprintf(szProfInfo,"%.3f ms : %s (%s)",time*1000.0f,ce->GetName(),ce->GetEntityClassName() );
				//m_pISystem->GetIRenderer()->DrawLabel(ce->GetPos(),1,szProfInfo);
				if (time > 0.5f)
					m_pISystem->GetIRenderer()->DrawLabelEx(ce->GetPos(),1.1f,colorsYellow,true,true,szProfInfo);
				else if (time > 1.0f)
					m_pISystem->GetIRenderer()->DrawLabelEx(ce->GetPos(),1.5f,colorsRed,true,true,szProfInfo);
				else
					m_pISystem->GetIRenderer()->DrawLabelEx(ce->GetPos(),0.9f,colors,true,true,szProfInfo);
			}
		} //it

		if (bProfileEntitiesToLog)
		{
			m_pISystem->GetILog()->Log( "\001================= Entity Update Times =================" );
			m_pISystem->GetILog()->Log( "\001%d Entities Updated.",ctx.numUpdatedEntities );
			m_pISystem->GetILog()->Log( "\001%d Visible Entities Updated.",ctx.numVisibleEntities );
			m_pISystem->GetILog()->Log( "\001%d Active Entity Timers.",(int)m_timersMap.size() );
			m_pISystem->GetILog()->Log( "\001%d Trackable Visible Entities.",(int)m_vEntitiesInFrustrum.size() );
			m_pProfileEntities->Set(0);
		}

		sprintf(szProfInfo,"Entities Updated: %d",ctx.numUpdatedEntities );
		m_pISystem->GetIRenderer()->Draw2dLabel( 10,10,1,colors,false,szProfInfo );

		m_pISystem->GetITimer()->MeasureTime("REALEntUp");
	}
 					
	m_nGetEntityCounter=0;
}

/*
void CEntitySystem::GarbageCollectorCycle()
{
	EntityMap::iterator next;
	for (EntityMap::iterator it = m_mapEntities.begin(); it != m_mapEntities.end();it = next)
	{
		next = it; next++;
		CEntity *ce= it->second;
		if (ce->IsGarbage() && ce->IsDestroyable())
		{
			ce->ShutDown();
			m_EntityIDGenerator.Remove(ce->GetId());
			delete ce;
			EntityMap::iterator itTemp=it;
			m_mapEntities.erase(itTemp);		// map iterator doesn;t invalidate :)
			continue;
		}
	}
}
*/

//////////////////////////////////////////////////////////////////////////
IEntityCamera * CEntitySystem::CreateEntityCamera() 
{ 
	CEntityCamera *pNew = new CEntityCamera; 
	pNew->Init(m_pISystem->GetIPhysicalWorld(), 
		m_pISystem->GetIRenderer()->GetWidth(), 
		m_pISystem->GetIRenderer()->GetHeight(), m_pISystem->GetIConsole()); 
	return (IEntityCamera *) pNew; 
};

//////////////////////////////////////////////////////////////////////////
IEntityIt *CEntitySystem::GetEntityIterator()
{
	return (IEntityIt *) new CEntityItMap(&m_mapEntities);
}

//////////////////////////////////////////////////////////////////////////
IEntityIt *CEntitySystem::GetEntityInFrustrumIterator( bool bFromPrevFrame )
{
	if(bFromPrevFrame )
		return new CEntityItVec(&m_vEntitiesInFrustrumPrevFrame);

	return new CEntityItVec(&m_vEntitiesInFrustrum);
}

/*IEntityClonesMgr *CEntitySystem::CreateEntityClonesMgr()
{
	//CEntityClonesMgr *pECM=new CEntityClonesMgr(this);
	//<<FIXME>> implement add all entities
	//m_setEntityClonesMgrs.insert(pECM);
	return NULL;
}*/

/*void CEntitySystem::RemoveEntityCloneMgr(CEntityClonesMgr *p)
{
	//m_setEntityClonesMgrs.erase(p);
	}*/

//////////////////////////////////////////////////////////////////////////
bool CEntitySystem::IsIDUsed(EntityId nID)
{
	return m_EntityIDGenerator.IsUsed(nID);
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::ReleaseMark(unsigned int id)
{
	m_EntityIDGenerator.Remove(id);
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::ResetEntities(void)
{
	// call on reset for every entity
	EntityMap::iterator it = m_mapEntities.begin();
	while (it != m_mapEntities.end())
	{
		CEntity *ent = it->second;
		if (ent)
		{
			ent->Reset();
			ent->SendScriptEvent(ScriptEvent_Reset,0);
		}
		it++;
	}
/*
	it = m_mapEntities.begin();
	while (it != m_mapEntities.end())
	{
		CEntity *ent = it->second;
		if (ent)
		{
			ent->OnBindAI( 0 );
		}
		it++;
	}
*/
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::GetMemoryStatistics(ICrySizer *pSizer)
{
	unsigned nSize = sizeof(*this) + m_EntityIDGenerator.sizeofThis() - sizeof(m_EntityIDGenerator);
	EntityMapItor itor;
	for(itor=m_mapEntities.begin();itor!=m_mapEntities.end();++itor)
	{
		itor->second->GetMemoryStatistics(pSizer);
		nSize += sizeof(*itor);
	}
	
	nSize += m_lstSinks.size() * sizeof(IEntitySystemSink*);

	{
		nSize += m_vEntitiesInFrustrum.size() * sizeof(m_vEntitiesInFrustrum[0]);
		for (EntityVector::iterator it = m_vEntitiesInFrustrum.begin(); it != m_vEntitiesInFrustrum.end(); ++it)
			(*it)->GetMemoryStatistics(pSizer);
	}

	pSizer->AddObject(this, nSize);
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::OnBind(EntityId id,EntityId child,unsigned char param)
{
	if (!m_lstSinks.empty())
	{
		for (SinkList::iterator si=m_lstSinks.begin();si!=m_lstSinks.end();si++)
			(*si)->OnBind(id,child,param);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::OnUnbind(EntityId id,EntityId child,unsigned char param)
{
	if (!m_lstSinks.empty())
	{
		for (SinkList::iterator si=m_lstSinks.begin();si!=m_lstSinks.end();si++)
			(*si)->OnUnbind(id,child,param);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::AddTimerEvent( int delayTimeMillis,SEntityTimerEvent &event )
{
	int nCurrTimeMillis = FtoI(m_pISystem->GetITimer()->GetCurrTime() * 1000.0f);
	int nTriggerTime = nCurrTimeMillis + delayTimeMillis;
	m_timersMap.insert( EntityTimersMap::value_type(nTriggerTime,event) );
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::RemoveTimerEvent( EntityId id )
{
	for (EntityTimersMap::iterator it = m_timersMap.begin(); it != m_timersMap.end(); ++it)
	{
		if (id == it->second.entityId)
		{
			// Remove this item.
			m_timersMap.erase( it ); 
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::PauseTimers(bool bPause,bool bResume)
{	
	m_bTimersPause=bPause;	
	if (bResume)
	{
		m_nStartPause=-1;
		return; // just allow timers to be updated next time
	}

	if (m_pISystem->GetIGame())
	{		 
		if (m_pISystem->GetIGame()->GetModuleState(EGameMultiplayer)) 
			return;			
	}

	if (bPause)
	{ 
		// record when timers pause was called
		m_nStartPause = FtoI(m_pISystem->GetITimer()->GetCurrTime() * 1000.0f);
	}
	else if (m_nStartPause>0)
	{		
		// increase the timers by adding the delay time passed since when
		// it was paused
		int nCurrTimeMillis=FtoI(m_pISystem->GetITimer()->GetCurrTime() * 1000.0f);
		int nAdditionalTriggerTime = nCurrTimeMillis-m_nStartPause;
		
		EntityTimersMap::iterator it;
		EntityTimersMap lstTemp;

		for (it = m_timersMap.begin();it!=m_timersMap.end();it++)
		{
			lstTemp.insert( EntityTimersMap::value_type(it->first,it->second) );			
		} //it

		m_timersMap.clear();

		for (it = lstTemp.begin();it!=lstTemp.end();it++)
		{
			int nUpdatedTimer=it->first+nAdditionalTriggerTime;
			m_timersMap.insert( EntityTimersMap::value_type(nUpdatedTimer,it->second) );			
		} //it	

		m_nStartPause=-1;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::UpdateTimers()
{
	if (m_timersMap.empty())
		return;

	if (m_pISystem->GetIGame())
	{		 
		if ((!m_pISystem->GetIGame()->GetModuleState(EGameMultiplayer)) && m_bTimersPause)
			return;			
	}


	int nCurrTimeMillis = FtoI(m_pISystem->GetITimer()->GetCurrTime() * 1000.0f);

	// Iterate thru all matching timers.
	EntityTimersMap::iterator first = m_timersMap.begin();
	EntityTimersMap::iterator last = m_timersMap.upper_bound( nCurrTimeMillis );
	if (last != first)
	{
		// Make a separate list, because OnTrigger call can modify original timers map.
		m_currentTriggers.resize(0);
		m_currentTriggers.reserve(10); 
		for (EntityTimersMap::iterator it = first; it != last; ++it)
		{
			m_currentTriggers.push_back(it->second); 
		}

		// Delete these items from map.
		m_timersMap.erase( first,last );

		//////////////////////////////////////////////////////////////////////////
		// Execute OnTrigger events.
		for (int i = 0; i < (int)m_currentTriggers.size(); i++)
		{
			SEntityTimerEvent &event = m_currentTriggers[i];
			// Trigger it.
			CEntity *pEntity = (CEntity*)GetEntity( event.entityId );
			if (pEntity)
			{
				// Silently kill trigger.
				pEntity->m_nTimer = -1;
				pEntity->OnTimer( event.timerId );
			}
		}
	}
} 

//////////////////////////////////////////////////////////////////////////
void CEntitySystem::SetPrecacheResourcesMode( bool bPrecaching )
{
	gPrecacheResourcesMode = bPrecaching;
}