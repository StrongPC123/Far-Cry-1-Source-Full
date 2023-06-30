// ScriptObjectEntity.cpp: implementation of the CScriptObjectEntity class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <ISystem.h>
#include <ILog.h>
#include <IEntitySystem.h>
#include <IAISystem.h>
#include <IAgent.h>
#include <ICryAnimation.h>
#include <IGame.h>
#include <ISound.h>
#include <IRenderer.h>
#include <IConsole.h>
#include "ScriptObjectEntity.h"
#include <ScriptObjectVector.h>
#include <CryCharMorphParams.h>
#include <ILipSync.h>
#include <ITimer.h>
//#include "XServer.h"
//#include "XWeaponSystem.h"
//#include "XPuppetProxy.h"
//#include "XVehicleProxy.h"
//#include "XObjectProxy.h"
#include "i3dengine.h"
//#include "TeamMgr.h"
//#include "XPlayer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAX_PARTICLES_SLOTS 100

_DECLARE_SCRIPTABLEEX(CScriptObjectEntity)

#ifdef USE_MEMBER_POS
IScriptObject *CScriptObjectEntity::m_pObjectPos=NULL;
IScriptObject *CScriptObjectEntity::m_pObjectAngles=NULL;
IScriptObject *CScriptObjectEntity::m_pCameraPosition=NULL;
IScriptObject *CScriptObjectEntity::m_pGenVector=NULL;
#endif

IScriptObject* CScriptObjectEntity::m_memberSO[SOE_MEMBER_LAST];

CScriptObjectEntity::CScriptObjectEntity()
{
	m_nCurrSoundId=-1;
}

CScriptObjectEntity::~CScriptObjectEntity()
{
}

bool CScriptObjectEntity::Create(IScriptSystem *pScriptSystem, ISystem *pSystem)
{
	m_pEntitySystem = (IEntitySystem *)pSystem->GetIEntitySystem();
	m_pISystem = pSystem;
	m_pSoundSystem=m_pISystem->GetISoundSystem();
	m_nCurrSoundId=-1;
	Init(pScriptSystem,this);
	assert(m_pScriptThis);
	m_pScriptThis->RegisterParent(this);

	assert(m_pScriptThis);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CScriptObjectEntity::SetMemberVector( SOE_MEMBER_LUA_TABLES member,const Vec3 &vec )
{
	IScriptObject *pVec = m_memberSO[member];
	pVec->BeginSetGetChain();
	pVec->SetValueChain("x",vec.x);
	pVec->SetValueChain("y",vec.y);
	pVec->SetValueChain("z",vec.z);
	pVec->EndSetGetChain();
}

void CScriptObjectEntity::SetEntity(IEntity *pEntity)
{
	assert(m_pScriptThis);
	m_pEntity=pEntity;
	assert(m_pScriptThis);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	assert(m_pScriptThis);

	string sClassname=m_pEntity->GetEntityClassName();
	assert(m_pScriptThis);
	if (!m_pScriptSystem->GetGlobalValue(sClassname.c_str(),*pObj))
	{
		m_pISystem->GetILog()->LogToFile("[FATAL ERROR] Script table %s not found. Probably script was not loaded because of an error.",sClassname.c_str());				

		//return; // vlad: to be able to load level created for xbox

		CryError("[FATAL ERROR] Script table %s not found. Probably script was not loaded because of an error.",sClassname.c_str());

		//m_pISystem->GetIConsole()->Exit("[FATAL ERROR] Script table %s not found. Probably script was not loaded because of an error.",sClassname.c_str());

		//m_pISystem->Quit();
	}	
	assert(m_pScriptThis);
	m_pScriptThis->Clone(*pObj);
	assert(m_pScriptThis);
	if(m_pEntity!=NULL)
	{
		assert(m_pScriptThis);
		m_pScriptThis->SetValue("id",((int)m_pEntity->GetId()));
		assert(m_pScriptThis);
		m_pScriptThis->SetValue("classid",((int)m_pEntity->GetClassId()));
		assert(m_pScriptThis);
		m_pScriptThis->SetValue("classname",m_pEntity->GetEntityClassName());
		assert(m_pScriptThis);
	}
	else
	{
		assert(m_pScriptThis);
		m_pScriptThis->SetToNull("id");
		assert(m_pScriptThis);
		m_pScriptThis->SetToNull("classid");
		assert(m_pScriptThis);
		m_pScriptThis->SetToNull("classname");
		assert(m_pScriptThis);
	}
	assert(m_pScriptThis);
}

void CScriptObjectEntity::ReleaseTemplate()
{
#ifdef USE_MEMBER_POS
	SAFE_RELEASE( m_pObjectPos );
	SAFE_RELEASE( m_pObjectAngles );
	SAFE_RELEASE( m_pCameraPosition );
	SAFE_RELEASE( m_pGenVector );
#endif
	for (int i = 0; i < SOE_MEMBER_LAST; i++)
	{
		SAFE_RELEASE( m_memberSO[i] );
	}

	_ScriptableEx<CScriptObjectEntity>::ReleaseTemplate();
}
//////////////////////////////////////////////////////////////////////////
void CScriptObjectEntity::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectEntity>::InitializeTemplate(pSS);
#ifdef USE_MEMBER_POS
	m_pObjectPos=pSS->CreateObject();
	m_pObjectAngles=pSS->CreateObject();
	m_pCameraPosition=pSS->CreateObject();
	m_pGenVector=pSS->CreateObject();
#endif
	for (int i = 0; i < SOE_MEMBER_LAST; i++)
	{
		m_memberSO[i] = pSS->CreateObject();
	}

	REG_FUNC(CScriptObjectEntity,GetPos);
	REG_FUNC(CScriptObjectEntity,GetCenterOfMassPos);
	REG_FUNC(CScriptObjectEntity,SetPos);
	REG_FUNC(CScriptObjectEntity,SetName);
	REG_FUNC(CScriptObjectEntity,GetName);
	REG_FUNC(CScriptObjectEntity,SetAIName);
	REG_FUNC(CScriptObjectEntity,GetAIName);
	REG_FUNC(CScriptObjectEntity,LoadCharacter);
	REG_FUNC(CScriptObjectEntity,PhysicalizeCharacter);
	REG_FUNC(CScriptObjectEntity,KillCharacter);
	REG_FUNC(CScriptObjectEntity,LoadObject);
	REG_FUNC(CScriptObjectEntity,SetObjectPos);
	REG_FUNC(CScriptObjectEntity,GetObjectPos);
	REG_FUNC(CScriptObjectEntity,SetObjectAngles);
	REG_FUNC(CScriptObjectEntity,GetObjectAngles);
	REG_FUNC(CScriptObjectEntity,GetEntitiesInContact)
	REG_FUNC(CScriptObjectEntity,LoadVehicle);
	REG_FUNC(CScriptObjectEntity,DrawObject);
	REG_FUNC(CScriptObjectEntity,CreateParticlePhys);
	REG_FUNC(CScriptObjectEntity,CreateRigidBody);
	REG_FUNC(CScriptObjectEntity,CreateArticulatedBody);
	REG_FUNC(CScriptObjectEntity,CreateRigidBodyPiece);
	REG_FUNC(CScriptObjectEntity,AwakePhysics);
	REG_FUNC(CScriptObjectEntity,ResetPhysics);
	REG_FUNC(CScriptObjectEntity,AwakeCharacterPhysics);
	REG_FUNC(CScriptObjectEntity,CreateStaticEntity);
	REG_FUNC(CScriptObjectEntity,CreateLivingEntity);
	REG_FUNC(CScriptObjectEntity,CreateSoftEntity);
	REG_FUNC(CScriptObjectEntity,SetAngles);
	REG_FUNC(CScriptObjectEntity,GetAngles);
	REG_FUNC(CScriptObjectEntity,Bind);
	REG_FUNC(CScriptObjectEntity,Unbind);
	REG_FUNC(CScriptObjectEntity,IsBound);
	REG_FUNC(CScriptObjectEntity,NetPresent);
	REG_FUNC(CScriptObjectEntity,StartAnimation);
	REG_FUNC(CScriptObjectEntity,ResetAnimation);
	REG_FUNC(CScriptObjectEntity,GetHelperPos);
	REG_FUNC(CScriptObjectEntity,RenderShadow);
	REG_FUNC(CScriptObjectEntity,DrawCharacter);
	REG_FUNC(CScriptObjectEntity,SetRegisterInSectors);
	REG_FUNC(CScriptObjectEntity,CreateParticleEntity);
	REG_FUNC(CScriptObjectEntity,SetPhysicParams);
	REG_FUNC(CScriptObjectEntity,SetCharacterPhysicParams);
	REG_FUNC(CScriptObjectEntity,GetParticleCollisionStatus);
	REG_FUNC(CScriptObjectEntity,GetObjectStatus);
	REG_FUNC(CScriptObjectEntity,SetObjectStatus);
	REG_FUNC(CScriptObjectEntity,GetDirectionVector);
	REG_FUNC(CScriptObjectEntity,IsAnimationRunning);
	REG_FUNC(CScriptObjectEntity,AddImpulse);
	REG_FUNC(CScriptObjectEntity,AddImpulseObj);
	REG_FUNC(CScriptObjectEntity,IsPointWithinRadius);
	REG_FUNC(CScriptObjectEntity,GetDistanceFromPoint);
	REG_FUNC(CScriptObjectEntity,DestroyPhysics);
	REG_FUNC(CScriptObjectEntity,EnablePhysics);
	REG_FUNC(CScriptObjectEntity,EnableSave);
	REG_FUNC(CScriptObjectEntity,TriggerEvent);
	REG_FUNC(CScriptObjectEntity,SetShader);
	REG_FUNC(CScriptObjectEntity,SetSecondShader);
	REG_FUNC(CScriptObjectEntity,GetShader);
	REG_FUNC(CScriptObjectEntity,GetCameraPosition);
	REG_FUNC(CScriptObjectEntity,SetBBox);
	REG_FUNC(CScriptObjectEntity,GetBBox);
	REG_FUNC(CScriptObjectEntity,GetLocalBBox);
	REG_FUNC(CScriptObjectEntity,SetRadius);
	REG_FUNC(CScriptObjectEntity,SetUpdateRadius);
	REG_FUNC(CScriptObjectEntity,GetUpdateRadius);
	REG_FUNC(CScriptObjectEntity,EnableUpdate);
	REG_FUNC(CScriptObjectEntity,SetUpdateIfPotentiallyVisible);
	REG_FUNC(CScriptObjectEntity,SetUpdateType);
	REG_FUNC(CScriptObjectEntity,SetShaderFloat);
  REG_FUNC(CScriptObjectEntity,SetColor);
	REG_FUNC(CScriptObjectEntity,SetStatObjScale);
	REG_FUNC(CScriptObjectEntity,SetAnimationEvent);
	REG_FUNC(CScriptObjectEntity,SetAnimationTime);
	REG_FUNC(CScriptObjectEntity,GetAnimationTime);
	REG_FUNC(CScriptObjectEntity,SetAnimationKeyEvent);
	REG_FUNC(CScriptObjectEntity,DisableAnimationEvent);
	REG_FUNC(CScriptObjectEntity,SetAnimationSpeed);
	REG_FUNC(CScriptObjectEntity,SelectPipe);
	REG_FUNC(CScriptObjectEntity,InsertSubpipe);
	REG_FUNC(CScriptObjectEntity,GetCurAnimation);
	REG_FUNC(CScriptObjectEntity,SetTimer);
	REG_FUNC(CScriptObjectEntity,KillTimer);
	REG_FUNC(CScriptObjectEntity,SetScriptUpdateRate);
	REG_FUNC(CScriptObjectEntity,GetAnimationLength);
	REG_FUNC(CScriptObjectEntity,ReleaseLipSync);
	REG_FUNC(CScriptObjectEntity,DoRandomExpressions);
	REG_FUNC(CScriptObjectEntity,DoExpression);
	REG_FUNC(CScriptObjectEntity,SayDialog);
	REG_FUNC(CScriptObjectEntity,StopDialog);
	REG_FUNC(CScriptObjectEntity,ApplyForceToEnvironment);
	REG_FUNC(CScriptObjectEntity,PlaySound);
	REG_FUNC(CScriptObjectEntity,GetBonePos);
	REG_FUNC(CScriptObjectEntity,GetBoneDir);
	REG_FUNC(CScriptObjectEntity,GetBoneNameFromTable);
	REG_FUNC(CScriptObjectEntity,LoadObjectPiece);
	REG_FUNC(CScriptObjectEntity,GetTouchedSurfaceID);
	REG_FUNC(CScriptObjectEntity,GetTouchedPoint);
	REG_FUNC(CScriptObjectEntity,AttachToBone);
	REG_FUNC(CScriptObjectEntity,AttachObjectToBone);
	REG_FUNC(CScriptObjectEntity,DetachObjectToBone);
	REG_FUNC(CScriptObjectEntity,InitDynamicLight);
	REG_FUNC(CScriptObjectEntity,AddDynamicLight);
	REG_FUNC(CScriptObjectEntity,AddDynamicLight2);
	REG_FUNC(CScriptObjectEntity,RemoveLight);
	REG_FUNC(CScriptObjectEntity,DoHam);
	REG_FUNC(CScriptObjectEntity,ResetHam);
	REG_FUNC(CScriptObjectEntity,LoadBoat);
	REG_FUNC(CScriptObjectEntity,GotoState);
	REG_FUNC(CScriptObjectEntity,IsInState);
	REG_FUNC(CScriptObjectEntity,GetState);
	REG_FUNC(CScriptObjectEntity,RegisterState);
	REG_FUNC(CScriptObjectEntity,IsVisible);
//	REG_FUNC(CScriptObjectEntity,GetBuildingId);
//	REG_FUNC(CScriptObjectEntity,GetSectorId);
	REG_FUNC(CScriptObjectEntity,Damage);
//	REG_FUNC(CScriptObjectEntity,UpdateInSector);
	REG_FUNC(CScriptObjectEntity,GetCameraAngles);
	REG_FUNC(CScriptObjectEntity,ChangeAIParameter);
//	REG_FUNC(CScriptObjectEntity,TranslatePartIdToDeadBody);
	REG_FUNC(CScriptObjectEntity,SetAICustomFloat);
	REG_FUNC(CScriptObjectEntity,ActivatePhysics);
	REG_FUNC(CScriptObjectEntity,CreateParticleEmitter);
	REG_FUNC(CScriptObjectEntity,DeleteParticleEmitter);
	REG_FUNC(CScriptObjectEntity,CreateParticleEmitterEffect );
	REG_FUNC(CScriptObjectEntity,IsAffectedByExplosion);
	REG_FUNC(CScriptObjectEntity,LoadBreakable);
	REG_FUNC(CScriptObjectEntity,BreakEntity);
	REG_FUNC(CScriptObjectEntity,SetMaterial);
	REG_FUNC(CScriptObjectEntity,GetMaterial);
	REG_FUNC(CScriptObjectEntity,SetHandsIKTarget);
	REG_FUNC(CScriptObjectEntity,SetDefaultIdleAnimations);
	REG_FUNC(CScriptObjectEntity,GetVelocity);
	REG_FUNC(CScriptObjectEntity,ApplyImpulseToEnvironment);
	REG_FUNC(CScriptObjectEntity,EnableProp);
	REG_FUNC(CScriptObjectEntity,TrackColliders);

	REG_FUNC(CScriptObjectEntity,GetViewDistRatio);
	REG_FUNC(CScriptObjectEntity,SetViewDistRatio);
	REG_FUNC(CScriptObjectEntity,SetViewDistUnlimited);
	REG_FUNC(CScriptObjectEntity,RemoveDecals);
	REG_FUNC(CScriptObjectEntity,SwitchLight);
	REG_FUNC(CScriptObjectEntity,ForceCharacterUpdate);
	REG_FUNC(CScriptObjectEntity,Hide);
	REG_FUNC(CScriptObjectEntity,CheckCollisions);
	REG_FUNC(CScriptObjectEntity,AwakeEnvironment);
	REG_FUNC(CScriptObjectEntity,SetStateClientside);
	REG_FUNC(CScriptObjectEntity,NoExplosionCollision);

	pSS->SetGlobalValue("PHYSICPARAM_FLAGS", PHYSICPARAM_FLAGS);
	pSS->SetGlobalValue("PHYSICPARAM_PARTICLE", PHYSICPARAM_PARTICLE);
	pSS->SetGlobalValue("PHYSICPARAM_VEHICLE", PHYSICPARAM_VEHICLE);
	pSS->SetGlobalValue("PHYSICPARAM_WHEEL", PHYSICPARAM_WHEEL);
	pSS->SetGlobalValue("PHYSICPARAM_SIMULATION", PHYSICPARAM_SIMULATION);
	pSS->SetGlobalValue("PHYSICPARAM_ARTICULATED", PHYSICPARAM_ARTICULATED);
	pSS->SetGlobalValue("PHYSICPARAM_JOINT", PHYSICPARAM_JOINT);
	pSS->SetGlobalValue("PHYSICPARAM_ROPE", PHYSICPARAM_ROPE);
	pSS->SetGlobalValue("PHYSICPARAM_SOFTBODY", PHYSICPARAM_SOFTBODY);
	pSS->SetGlobalValue("PHYSICPARAM_BUOYANCY", PHYSICPARAM_BUOYANCY);
	pSS->SetGlobalValue("PHYSICPARAM_CONSTRAINT", PHYSICPARAM_CONSTRAINT);
	pSS->SetGlobalValue("PHYSICPARAM_REMOVE_CONSTRAINT", PHYSICPARAM_REMOVE_CONSTRAINT);
	pSS->SetGlobalValue("PHYSICPARAM_PLAYERDYN", PHYSICPARAM_PLAYERDYN);
	pSS->SetGlobalValue("PHYSICPARAM_PLAYERDIM", PHYSICPARAM_PLAYERDIM);
	pSS->SetGlobalValue("PHYSICPARAM_VELOCITY", PHYSICPARAM_VELOCITY);
	pSS->SetGlobalValue("PHYSICPARAM_PART_FLAGS", PHYSICPARAM_PART_FLAGS);
	pSS->SetGlobalValue("pef_pushable_by_players", pef_pushable_by_players);
	pSS->SetGlobalValue("pef_monitor_state_changes", pef_monitor_state_changes);
	pSS->SetGlobalValue("pef_monitor_collisions", pef_monitor_collisions);
	pSS->SetGlobalValue("pef_never_affect_triggers", pef_never_affect_triggers);
	pSS->SetGlobalValue("pef_fixed_damping", pef_fixed_damping);
	pSS->SetGlobalValue("lef_push_objects", lef_push_objects);
	pSS->SetGlobalValue("lef_push_players", lef_push_players);
	pSS->SetGlobalValue("particle_single_contact", particle_single_contact);
	pSS->SetGlobalValue("particle_constant_orientation", particle_constant_orientation);
	pSS->SetGlobalValue("particle_no_roll", particle_no_roll);
	pSS->SetGlobalValue("particle_no_path_alignment", particle_no_path_alignment);
	pSS->SetGlobalValue("particle_no_spin", particle_no_spin);
	pSS->SetGlobalValue("pef_traceable", pef_traceable);
	pSS->SetGlobalValue("ref_use_simple_solver", ref_use_simple_solver);
	pSS->SetGlobalValue("particle_traceable", particle_traceable);
	pSS->SetGlobalValue("lef_snap_velocities", lef_snap_velocities);
	pSS->SetGlobalValue("rope_collides", rope_collides);
	pSS->SetGlobalValue("rope_traceable", rope_traceable);
	pSS->SetGlobalValue("geom_colltype_ray", geom_colltype_ray);
	pSS->SetGlobalValue("geom_colltype_vehicle", geom_colltype3);
	pSS->SetGlobalValue("geom_collides", geom_collides);
	pSS->SetGlobalValue("geom_floats", geom_floats);
	pSS->SetGlobalValue("geom_colltype0", geom_colltype0);
	pSS->SetGlobalValue("geom_colltype_player", geom_colltype_player);
	pSS->SetGlobalValue("geom_colltype_explosion", geom_colltype_explosion);
	pSS->SetGlobalValue("ent_static", ent_static);
	pSS->SetGlobalValue("ent_sleeping_rigid", ent_sleeping_rigid);
	pSS->SetGlobalValue("ent_rigid", ent_rigid);
	pSS->SetGlobalValue("ent_living", ent_living);
	pSS->SetGlobalValue("ent_independent", ent_independent);
	pSS->SetGlobalValue("ent_terrain", ent_terrain);

	pSS->SetGlobalValue( "eUT_Always",eUT_Always );
	pSS->SetGlobalValue( "eUT_InViewRange",eUT_InViewRange );
	pSS->SetGlobalValue( "eUT_PotVisible",eUT_PotVisible );
	pSS->SetGlobalValue( "eUT_Visible",eUT_Visible );
	pSS->SetGlobalValue( "eUT_Physics",eUT_Physics );
	pSS->SetGlobalValue( "eUT_PhysicsVisible",eUT_PhysicsVisible );
	pSS->SetGlobalValue( "eUT_PhysicsPostStep",eUT_PhysicsPostStep );
	pSS->SetGlobalValue( "eUT_Never",eUT_Never );
	pSS->SetGlobalValue( "eUT_Unconditional",eUT_Unconditional );

//	pSS->SetGlobalValue("BITMASK_PLAYER",BITMASK_PLAYER);
//	pSS->SetGlobalValue("BITMASK_WEAPON",BITMASK_WEAPON);	
//	pSS->SetGlobalValue("BITMASK_OBJECT",BITMASK_OBJECT);
}

void CScriptObjectEntity::SetContainer(IScriptObject *pContainer)
{
	/*if(pContainer!=NULL)
	{
		EnablePropertiesMapping();
		RegisterProperty("cnt",pContainer);
	}*/
	if(pContainer!=NULL)
		m_pScriptThis->SetValue("cnt",pContainer);
	else
		m_pScriptThis->SetToNull("cnt");
//	_SmartScriptObject pCntTest(m_pScriptSystem,true);
//	if(!m_pScriptThis->GetValue("cnt",pCntTest))

}

/*! Get the position of the entity
	@return Three component vector containing the entity position
*/
int CScriptObjectEntity::GetPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	Vec3 vec;
#ifndef USE_MEMBER_POS
	CScriptObjectVector oVec(m_pScriptSystem);
//	vec=m_pEntity->GetPos(false);
	vec=m_pEntity->GetPos(true);	
	oVec=vec;
	return pH->EndFunction(*oVec);
#else
	vec=m_pEntity->GetPos(true);
	m_pObjectPos->BeginSetGetChain();
	m_pObjectPos->SetValueChain("x",vec.x);
	m_pObjectPos->SetValueChain("y",vec.y);
	m_pObjectPos->SetValueChain("z",vec.z);
	m_pObjectPos->EndSetGetChain();
	return pH->EndFunction(m_pObjectPos);
#endif

}


/*! Get the position of the entity's physical center of mass
	@return Three component vector containing the entity's center of mass
*/
int CScriptObjectEntity::GetCenterOfMassPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	pe_status_dynamics sd;
	IPhysicalEntity *pent = m_pEntity->GetPhysics();
	if (pent) 
		pent->GetStatus(&sd);
	else
		sd.centerOfMass = m_pEntity->GetPos(true);

#ifndef USE_MEMBER_POS
	CScriptObjectVector oVec(m_pScriptSystem);
//	vec=m_pEntity->GetPos(false);
	oVec = sd.centerOfMass;
	return pH->EndFunction(*oVec);
#else
	m_pGenVector->BeginSetGetChain();
	m_pGenVector->SetValueChain("x",sd.centerOfMass.x);
	m_pGenVector->SetValueChain("y",sd.centerOfMass.y);
	m_pGenVector->SetValueChain("z",sd.centerOfMass.z);
	m_pGenVector->EndSetGetChain();
	return pH->EndFunction(m_pGenVector);
#endif

}



/*! Set the position of the entity
	@param vec Three component vector containing the entity position
*/
int CScriptObjectEntity::SetPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	Vec3 vec;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*oVec);
	vec=oVec.Get();
	m_pEntity->SetPos(vec, false);
	return pH->EndFunction();
}

/*! Set the AI name of the entity. Can be different than actual entity name
@param sName String containing the new AI name
*/
int CScriptObjectEntity::SetAIName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sName;
	pH->GetParam(1,sName);
	if (m_pEntity->GetAI())
        m_pEntity->GetAI()->SetName(sName);
	return pH->EndFunction();
}

/*! Set the name of the entity
	@param sName String containing the new entity name
*/
int CScriptObjectEntity::SetName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sName;
	pH->GetParam(1,sName);
	m_pEntity->SetName(sName);
	return pH->EndFunction();
}

/*! Get the AI name of the entity. Can be different than entity name.
@return AI Name of the entity
*/
int CScriptObjectEntity::GetAIName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	string sName;
	if (m_pEntity->GetAI())
		sName=m_pEntity->GetAI()->GetName();

	return pH->EndFunction(sName.c_str());
}



/*! Get the name of the entity
	@return Name of the entity
*/
int CScriptObjectEntity::GetName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	string sName;
	sName=m_pEntity->GetName();
	return pH->EndFunction(sName.c_str());
}

/*! Load an animated character
	@param sFileName File name of the .cid file
	@param nPos Number of the slot for the character
	@see CScriptObjectEntity::StartAnimation
*/
int CScriptObjectEntity::LoadCharacter(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *sFileName;
	int nPos;
	pH->GetParam(1,sFileName);
	pH->GetParam(2,nPos);

	bool bRes=false;

	if(m_pEntity)
	{
		bRes=m_pEntity->GetCharInterface()->LoadCharacter(nPos,sFileName);

/*
		//<<FIXME>> when loading in setplayer model is uncommented, delete this code
		IEntityContainer *pCont = m_pEntity->GetContainer();
		if (pCont)
		{
			CPlayer *pPlayer; 
			if (pCont->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer))
				pPlayer->SetPlayerModel(sFileName);
		}
*/
	}
	return pH->EndFunction(bRes);
}

/*!physicalize the character attached to the entity
	@param mass the mass of the entity
	@param surface_idx the material id
	@param stiffness_scale the stiffness scale
	@param nPos slot number of the geometry to physicalize
*/
int CScriptObjectEntity::PhysicalizeCharacter(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);
	float mass,stiffness_scale;
	int nPos,surface_idx;
	
	pH->GetParam(1,mass);
	pH->GetParam(2,surface_idx);
	pH->GetParam(3,stiffness_scale);
	pH->GetParam(4,nPos);

	
	if(m_pEntity)
	{
		IEntityCharacter *pC=m_pEntity->GetCharInterface();
		if(pC)
		{
			ICryCharInstance *pCryCharInstance=pC->GetCharacter(nPos);
			
			if(pCryCharInstance)
			{
				pC->PhysicalizeCharacter(nPos, mass,surface_idx,stiffness_scale);
			}
		}
	}
	return pH->EndFunction();
}

int CScriptObjectEntity::KillCharacter(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

//	if(m_pGame->p_DeadBody->GetFVal()==0)
//		return pH->EndFunction();

	int nPos;
	pH->GetParam(1,nPos);
	if (m_pEntity)
		m_pEntity->GetCharInterface()->KillCharacter(nPos);
	return pH->EndFunction();
}

/*! Load a static object
	@param sFileName File name of the .cgf file
	@param nPos Number of the slot for the character (Zero for first 1st person weapons, one for 3rd person weapons)
	@see CScriptObjectEntity::StartAnimation
*/
int CScriptObjectEntity::LoadObject(IFunctionHandler *pH)
{
	assert((unsigned int)(pH->GetParamCount()-3)<2u);
	const char *sFileName;
	const char *sGeomName;
	int nPos;
	float fScale;
	pH->GetParam(1,sFileName);
	pH->GetParam(2,nPos);
	pH->GetParam(3,fScale);
	if (pH->GetParamCount() > 3)
		pH->GetParam(4,sGeomName);
	else
		sGeomName = 0;

	if(!sFileName)
		m_pScriptSystem->RaiseError("Entity.LoadObject filename is nil");
	else
	{
		if(m_pEntity)
			m_pEntity->LoadObject(nPos,sFileName,fScale,sGeomName);
	}

	return pH->EndFunction();
}

int CScriptObjectEntity::IsAffectedByExplosion(IFunctionHandler *pH)
{
	float f=0.0f;
	IPhysicalEntity *pPE=m_pEntity->GetPhysics();
	if(pPE)
	{
		IPhysicalWorld *pW=m_pISystem->GetIPhysicalWorld();
		f=pW->IsAffectedByExplosion(pPE);
	}
	return pH->EndFunction(f);
}

int CScriptObjectEntity::LoadObjectPiece(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *sFileName;
	int idx;
	pH->GetParam(1,sFileName);
	pH->GetParam(2,idx);
	if (m_pEntity)
	{
		int res;
		if( idx==-1 )
			res = m_pEntity->LoadObject(0,sFileName,0,"unbreaked");
		else
		{
			char geomname[50];	
			sprintf(geomname,"piece%02d",idx);
			res = m_pEntity->LoadObject(0,sFileName,0,geomname);
			if(res)
			{
				sprintf(geomname,"piece%02d_proxy",idx);
				if(m_pEntity->LoadObject(1,sFileName,0,geomname))
				{
					m_pEntity->DrawObject(1, ETY_DRAW_NONE);
				}
			}
		}
		return pH->EndFunction(res);
  }
	return pH->EndFunction(0);
}

/*!return the relative position in the entity space of a certaing geometry(object) attached to the entity
	@param nSlot slot of the geometry
	@return a table with the x,y,z fields that contain the position of the object
*/
int CScriptObjectEntity::GetObjectPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int nSlot;
	Vec3 vPos;
	pH->GetParam(1,nSlot);
	if(m_pEntity->GetObjectPos(nSlot,vPos))
	{
		m_pObjectPos->BeginSetGetChain();
		m_pObjectPos->SetValueChain("x",vPos.x);
		m_pObjectPos->SetValueChain("y",vPos.y);
		m_pObjectPos->SetValueChain("z",vPos.z);
		m_pObjectPos->EndSetGetChain();
		return pH->EndFunction(m_pObjectPos);
	}
	return pH->EndFunction();
}

/*!set the relative position in the entity space of a certaing geometry(object) attached to the entity
	@param nSlot slot of the geometry
	@param pPos table with the x,y,z fields that contain the position of the object
*/
int CScriptObjectEntity::SetObjectPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	CScriptObjectVector pPos(m_pScriptSystem,true);
	int nSlot;
	Vec3 vPos;
	pH->GetParam(1,nSlot);
	pH->GetParam(2,pPos);
	vPos=pPos.Get();
	m_pEntity->SetObjectPos(nSlot,vPos);
	return pH->EndFunction();
}

/*!return the relative orientation in the entity space of a certaing geometry(object) attached to the entity
	@param nSlot slot of the geometry
	@return a table with the x,y,z fields that contain the orientation of the object(in degrees)
*/
int CScriptObjectEntity::GetObjectAngles(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int nSlot;
	Vec3 vAng;
	pH->GetParam(1,nSlot);
	if(m_pEntity->GetObjectAngles(nSlot,vAng))
	{
		m_pObjectAngles->BeginSetGetChain();
		m_pObjectAngles->SetValueChain("x",vAng.x);
		m_pObjectAngles->SetValueChain("y",vAng.y);
		m_pObjectAngles->SetValueChain("z",vAng.z);
		m_pObjectAngles->EndSetGetChain();
		return pH->EndFunction(m_pObjectAngles);
	}
	return pH->EndFunction();
}

/*!set the relative orientation in the entity space of a certaing geometry(object) attached to the entity
	@param nSlot slot of the geometry
	@param pPos table with the x,y,z fields that contain the orientation of the object(in degrees)
*/
int CScriptObjectEntity::SetObjectAngles(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	CScriptObjectVector pAng(m_pScriptSystem,true);
	int nSlot;
	Vec3 vAng;
	pH->GetParam(1,nSlot);
	pH->GetParam(2,pAng);
	vAng=pAng.Get();
	m_pEntity->SetObjectAngles(nSlot,vAng);
	return pH->EndFunction();
}



/*! Control drawing of static objects
	@param nPos Number of the slot for the character - if negativ - applay to ol objects in entity
	@param nMode 0 = Don't draw, 1 = Draw normally, 3 = Draw near
	@see CScriptObjectEntity::LoadObject
	@see CEntity::DrawObject
*/
int CScriptObjectEntity::DrawObject(IFunctionHandler *pH)
{

	CHECK_PARAMETERS(2);
	int nPos,nMode;
	pH->GetParam(1,nPos);
	pH->GetParam(2,nMode);

	if(m_pEntity)
	{
		if( nPos<0 )
			m_pEntity->DrawObject(nMode);
		else
			m_pEntity->DrawObject(nPos,nMode);
	}
	return pH->EndFunction();
}

/*! Destroys the physics of an entity object
	@see CEntity::DestroyPhysics
*/
int CScriptObjectEntity::DestroyPhysics(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pEntity->DestroyPhysics();
	return pH->EndFunction();
}

/*! Enables or disables the physics of an entity object
	@param bEnable True will enable physics, false will disable
	@see CEntity::EnablePhysics
*/
int CScriptObjectEntity::EnablePhysics(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	bool bEnable;
	pH->GetParam(1,bEnable);
	m_pEntity->EnablePhysics(bEnable);
	return pH->EndFunction();
}

int CScriptObjectEntity::CreateParticlePhys(IFunctionHandler *pH)
{
	// CHECK_PARAMETERS(2);
	assert(pH->GetParamCount() == 2 || pH->GetParamCount() == 3);
	float fSize;
	float fMass;
	//int nSurfaceID=m_pGame->m_XSurfaceMgr.GetSurfaceIDByMaterialName("mat_bounce");
	int nSurfaceID=-1;
	int iSingleContact;
	pH->GetParam(1,fSize);
	pH->GetParam(2,fMass);
	if (!pH->GetParam(3, iSingleContact))
		iSingleContact = 1;

	m_pEntity->CreateParticleEntity(fMass, fSize, Vec3(0, 1, 0), 0, 0, 0, -9.8f, nSurfaceID, iSingleContact != 0);
	return pH->EndFunction();
}

int CScriptObjectEntity::CreateRigidBody(IFunctionHandler *pH)
{
	return CreateRigidOrArticulatedBody(PE_RIGID, pH);
}

int CScriptObjectEntity::CreateArticulatedBody(IFunctionHandler *pH)
{
	return CreateRigidOrArticulatedBody(PE_ARTICULATED, pH);
}

int CScriptObjectEntity::CreateRigidOrArticulatedBody(pe_type type, IFunctionHandler *pH)
{
	assert(pH->GetParamCount() == 3 || pH->GetParamCount() == 4 || pH->GetParamCount() == 5);
	float fDensity;
	float fMass;
	int nSurfaceID=0;

	if(m_pEntity)
	{
		// Extract the mandatory parameters
		pH->GetParam(1,fDensity);
		pH->GetParam(2,fMass);
		pH->GetParam(3,nSurfaceID);

			// Check if the optional velocity vector was passed, use the slot nomber
		if(pH->GetParamCount() == 5)
		{
			// Get the direction angles, convert to a direction vector
			CScriptObjectVector oVec(m_pScriptSystem,true);
			pH->GetParam(4,*oVec);
			Vec3 vInitialVelocity = oVec.Get();
			int slot = 0;
			pH->GetParam(5,slot);
			m_pEntity->CreateRigidBody(type, fDensity,fMass,nSurfaceID,&vInitialVelocity, slot);
		} 
		else if(pH->GetParamCount() == 4)
		{
			// Get the direction angles, convert to a direction vector
			CScriptObjectVector oVec(m_pScriptSystem,true);
			pH->GetParam(4,*oVec);
			Vec3 vInitialVelocity = oVec.Get();
			m_pEntity->CreateRigidBody(type, fDensity,fMass,nSurfaceID,&vInitialVelocity);
		} 
		else
		{
			m_pEntity->CreateRigidBody(type, fDensity,fMass,nSurfaceID);
		}
	}
	
	return pH->EndFunction();
}

int CScriptObjectEntity::CreateRigidBodyPiece(IFunctionHandler *pH)
{
	assert(pH->GetParamCount() == 3 || pH->GetParamCount() == 4);
	float fDensity;
	float fMass;
	int nSurfaceID=0;

	if(m_pEntity)
	{
		// Extract the mandatory parameters
		pH->GetParam(1,fDensity);
		pH->GetParam(2,fMass);
		pH->GetParam(3,nSurfaceID);

		Vec3 vInitialVelocity(0,0,0);
		CEntityObject	obj;
		if(m_pEntity->GetEntityObject(1, obj))		// there is object in slot 1 - it is proxy
			m_pEntity->CreateRigidBody(PE_RIGID, fDensity,fMass,nSurfaceID,&vInitialVelocity, 1);
		else
			m_pEntity->CreateRigidBody(PE_RIGID, fDensity,fMass,nSurfaceID,&vInitialVelocity, 0);
	}
	return pH->EndFunction();
}


int CScriptObjectEntity::CreateStaticEntity(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(2);
	assert((unsigned int)pH->GetParamCount()-1u<3u);

	float fMass;
	int nSurfaceID=-1;
	int	nSlotToUse=-1;
	pH->GetParam(1,fMass);
	pH->GetParam(2,nSurfaceID);
	if(pH->GetParamCount() == 3)
		pH->GetParam(3,nSlotToUse);
		
	if(m_pEntity)
		m_pEntity->CreateStaticEntity(fMass,nSurfaceID,nSlotToUse);
	return pH->EndFunction();
}


int CScriptObjectEntity::CreateSoftEntity(IFunctionHandler *pH)
{
	assert(pH->GetParamCount()>=2);
	float fMass,fDensity;
	int bCloth=1;
	IEntity *pEnt;
	IPhysicalEntity *pPhysEnt = WORLD_ENTITY;
	int idEnt,iPart=-1;
	pH->GetParam(1,fMass);
	pH->GetParam(2,fDensity);
	pH->GetParam(3, bCloth);
	if (pH->GetParam(4, idEnt) && idEnt>=0 && (pEnt = m_pEntitySystem->GetEntity(idEnt)))
		if (!(pPhysEnt = pEnt->GetPhysics()))
			pPhysEnt = WORLD_ENTITY;
	pH->GetParam(5, iPart);

	m_pEntity->CreateSoftEntity(fMass,fDensity, bCloth!=0, pPhysEnt,iPart);
	return pH->EndFunction();
}

/*!set the orientation of the entity in world space
	@param a table containing the x,y,z fields that specify the orientation of the entity(in degrees)
	@param notify physics flag

*/
int CScriptObjectEntity::SetAngles(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(1);
	int parCount = pH->GetParamCount();
	assert(parCount>0 && parCount<3);
	Vec3 vec;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*oVec);
	vec=oVec.Get();
	if(parCount == 1)
		m_pEntity->SetAngles(vec);
	else
		m_pEntity->SetAngles(vec,true,false);
	return pH->EndFunction();
}

/*!set the scale of the geometry objects attached to the entity
	@param fScale the scale of the objects
*/
int CScriptObjectEntity::SetStatObjScale(IFunctionHandler *pH)
{
	float fScale;
	CHECK_PARAMETERS(1);
	pH->GetParam(1, fScale);
	m_pEntity->SetScale(fScale);
	return pH->EndFunction();
}

/*!get the orientation of the entity in world space
	@param [optional] if the parameter is present return the real angles if not the entity angles
					NOTE: if the entity is bound to another parent entity the real angles are the absolute angles
						if is not bound there is no difference
	@return a table containing the x,y,z fields that specify the orientation of the entity(in degrees)
*/
int CScriptObjectEntity::GetAngles(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(0);
	assert(pH->GetParamCount() == 0 || pH->GetParamCount() == 1);

	Ang3 vec;
	if(pH->GetParamCount() == 0)
		vec=m_pEntity->GetAngles(0);
	else
		vec=m_pEntity->GetAngles(1);
	vec.Snap360();


	m_pObjectAngles->BeginSetGetChain();
	m_pObjectAngles->SetValueChain("x",vec.x);
	m_pObjectAngles->SetValueChain("y",vec.y);
	m_pObjectAngles->SetValueChain("z",vec.z);
	m_pObjectAngles->EndSetGetChain();
	return pH->EndFunction(m_pObjectAngles);
}

//	Get Forward direction if no parameters
//	if parameter is passed - get Up direction
//	[filippo]: now more complete, if the param is specified we can get the axis we want.
int CScriptObjectEntity::GetDirectionVector(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(0);
	assert(pH->GetParamCount() == 0 || pH->GetParamCount() == 1);

	//forward default
	Vec3 vec(0,-1,0);

	//if there is a parameter we want to get something different by the forward vector, 0=x, 1=y, 2=z
	if( pH->GetParamCount()==1 )
	{
		int dir;
		pH->GetParam(1,dir);

		switch(dir)
		{
		default:
		case 0:
			vec.Set(0,-1,0);//forward
			break;
		case 1:
			vec.Set(-1,0,0);//right
			break;
		case 2:vec.Set(0,0,1);//up
			break;
		}
	}

	//CScriptObjectVector oVec(m_pScriptSystem);
	
	//vec = m_pEntity->GetAngles();

	Matrix44 tm;
	tm.SetIdentity();
	//tm.RotateMatrix_fix( m_pEntity->GetAngles() );
	tm=Matrix44::CreateRotationZYX(-m_pEntity->GetAngles()*gf_DEGTORAD)*tm; //NOTE: angles in radians and negated 

	//CHANGED_BY_IVO
	//vec = tm.TransformVector(vec);
	vec = GetTransposed44(tm)*vec;

	m_pObjectAngles->BeginSetGetChain();
	m_pObjectAngles->SetValue("x",vec.x);
	m_pObjectAngles->SetValue("y",vec.y);
	m_pObjectAngles->SetValue("z",vec.z);
	m_pObjectAngles->EndSetGetChain();
	//vec.ConvertToRadAngles();
	//oVec=vec;
	return pH->EndFunction(m_pObjectAngles);
}

int CScriptObjectEntity::AttachObjectToBone(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(2); can be 2 or 3 or 4 params.

	char *boneName;
	int slot;
	bool bMultipleAttachments = false;
	bool bUseZOffset = false;
	pH->GetParam(1,slot);
	pH->GetParam(2,boneName);

	if (!pH->GetParam(3,bMultipleAttachments))
		bMultipleAttachments = false;

	if (pH->GetParamCount()>=4 && !pH->GetParam(4,bUseZOffset))
		bUseZOffset = false;

	BoneBindHandle boneHandler = m_pEntity->AttachObjectToBone(slot, boneName,bMultipleAttachments, bUseZOffset);

	if (boneHandler == -1)
	{
		return pH->EndFunctionNull();
	}

	if (bMultipleAttachments)
	{
		// Make user data for bone handler.
		USER_DATA ud = m_pScriptSystem->CreateUserData( boneHandler,USER_DATA_BONEHANDLER );
		return pH->EndFunction(ud);
	}

	return pH->EndFunctionNull();
}

int CScriptObjectEntity::DetachObjectToBone(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(1); Can be 1 or 2 params.

	char *boneName;
	pH->GetParam(1,boneName);

	int BAD_HANDLER = -1;
	int nCookie;
	INT_PTR boneHandler = BAD_HANDLER;

	if (!pH->GetParamUDVal(2,boneHandler,nCookie))
	{
		boneHandler = BAD_HANDLER;
	}
	if (nCookie != USER_DATA_BONEHANDLER)
	{
		boneHandler = BAD_HANDLER;
	}

	m_pEntity->DetachObjectToBone(boneName,boneHandler );

	return pH->EndFunction();
}



int CScriptObjectEntity::AttachToBone(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	char *boneName;
	int nID;
	_SmartScriptObject pObj(m_pScriptSystem,true);
	pH->GetParam(1,*pObj);
	pH->GetParam(2,boneName);
	pObj->GetValue("id",nID);

	m_pEntity->AttachToBone(nID, boneName);

	return pH->EndFunction();
}

int CScriptObjectEntity::Bind(IFunctionHandler *pH)
{
	assert(pH->GetParamCount() == 1 || pH->GetParamCount() == 2);
//	CHECK_PARAMETERS(1);
	int nID;
	int cParam=0;
	_SmartScriptObject pObj(m_pScriptSystem,true);
	pH->GetParam(1,*pObj);
	//optional
	pH->GetParam(2,cParam);
	pObj->GetValue("id",nID);

	//m_pEntity->Bind(nID);
	//CXServer *pSrv=m_pGame->GetServer();
	//if(pSrv)
	//{
		m_pEntity->Bind(nID,cParam);
		//pSrv->BindEntity(m_pEntity->GetId(),nID,(unsigned char)cParam);
	//}
	return pH->EndFunction();
}

int CScriptObjectEntity::Unbind(IFunctionHandler *pH)
{
	assert(pH->GetParamCount() == 1 || pH->GetParamCount() == 2);
//	CHECK_PARAMETERS(1);
	int nID;
	int cParam=0;
	_SmartScriptObject pObj(m_pScriptSystem,true);
	pH->GetParam(1,*pObj);
	//optional
	pH->GetParam(2,cParam);
	pObj->GetValue("id",nID);
	//m_pEntity->Unbind(nID);
	//CXServer *pSrv=m_pGame->GetServer();
	//if(pSrv)
	//{
		m_pEntity->Unbind(nID,cParam);
		//pSrv->UnbindEntity(m_pEntity->GetId(),nID,(unsigned char)cParam);
	//}
	return pH->EndFunction();
}

int CScriptObjectEntity::IsBound(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	bool bRes;
	bRes=m_pEntity->IsBound();
	if(bRes)
		return pH->EndFunction(1);
	else
		return pH->EndFunctionNull();
}

int CScriptObjectEntity::CreateParticleEntity(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pTable(m_pScriptSystem, true);
	pH->GetParam(1, *pTable);
	float Size, Mass;
	Vec3 Heading;
	float Thrust, Resistance, Lift, Gravity;
	int Surface;
	bool b;
	pTable->GetValue("size", Size);
	pTable->GetValue("mass", Mass);
	CScriptObjectVector oVecHeading(m_pScriptSystem, true);
	pTable->GetValue("heading", *oVecHeading);
	Heading = oVecHeading.Get();
	pTable->GetValue("acc_thrust", Thrust);
	pTable->GetValue("k_air_resistance", Resistance);
	pTable->GetValue("acc_lift", Lift);
	pTable->GetValue("gravity", Gravity);
	pTable->GetValue("surface_idx", Surface);
	pTable->GetValue("constant_orientation",b);
	
	m_pEntity->CreateParticleEntity(Size, Mass, Heading, Thrust, Resistance, Lift, Gravity, (b?particle_constant_orientation:0));

	return pH->EndFunctionNull();
}


//<<<FIXME>>> pass the correct surface id from script
int CScriptObjectEntity::CreateLivingEntity(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	_SmartScriptObject pTable(m_pScriptSystem,true);
	int nSurfaceID=0;
	pH->GetParam(1,*pTable);
	float mass,height,eyeh,sph,rad,grav,aircontrol;
	int	collide = 0;
	
	pTable->GetValue("mass",mass);
	pTable->GetValue("height",height);
	pTable->GetValue("eyeheight",eyeh);
	pTable->GetValue("sphereheight",sph);
	pTable->GetValue("radius",rad);
	if (!pTable->GetValue("gravity",grav))
		grav = 9.81f;
	if (!pTable->GetValue("aircontrol",aircontrol))
		aircontrol = 0.f;
	if (!pTable->GetValue("collide",collide))
		collide = 0;

	pH->GetParam(2,nSurfaceID);
	
//<<FIXME>> give the entity system access to the material enumrator
//	nSurfaceID=m_pGame->m_XSurfaceMgr.GetSurfaceIDByMaterialName("mat_meat");
	
	m_pEntity->CreateLivingEntity(mass,height,eyeh,sph,rad,nSurfaceID,grav,aircontrol, collide!=0);

	return pH->EndFunctionNull();
}

int CScriptObjectEntity::LoadVehicle(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(1);
	_SmartScriptObject pTable(m_pScriptSystem,true);
	pH->GetParam(1,*pTable);
	bool bDestroy=false;
	if (pH->GetParamCount()>1)	
		pH->GetParam(2,bDestroy);			

	const char *filename;
	std::vector<pe_cargeomparams> pparts; 
	
	pe_params_car params;
  
	pTable->GetValue("file",filename);
	pTable->GetValue("engine_power",params.enginePower);
	//pTable->GetValue("engine_power_back",params.enginePowerBack);
	pTable->GetValue("engine_maxrpm",params.engineMaxRPM);
	pTable->GetValue("axle_friction",params.axleFriction);
	pTable->GetValue("max_steer",params.maxSteer);   
	pTable->GetValue("integration_type",params.iIntegrationType);
	pTable->GetValue("max_time_step_vehicle",params.maxTimeStep);
	float velSleep;
	if (pTable->GetValue("sleep_speed_vehicle",velSleep))
		params.minEnergy = velSleep*velSleep;
	pTable->GetValue("damping_vehicle",params.damping);

	int i=2;
	char str[255];
	sprintf(str,"hull1");
	_SmartScriptObject pHull(m_pScriptSystem,true);
	while (pTable->GetValue(str,*pHull))
	{
		pe_cargeomparams pone;
		pHull->GetValue("mass",pone.mass);
		pHull->GetValue("flags",(int&)pone.flags);
		pHull->GetValue("yoffset",pone.pos.y);
		pHull->GetValue("zoffset",pone.pos.z);
		pone.flagsCollider = geom_colltype3;
		sprintf(str,"hull%d",i++);
		pparts.push_back(pone);
		//if (i==5) break;		
	}

	i=2;
	sprintf(str,"wheel1");
	while (pTable->GetValue(str,*pHull))
	{
		pe_cargeomparams pone;
		pone.density = 5000;
		pone.flagsCollider = geom_colltype3;
		pHull->GetValue("driving",pone.bDriving);
		pHull->GetValue("axle",pone.iAxle);
		pHull->GetValue("can_brake",pone.bCanBrake);

		pHull->GetValue("len_max",pone.lenMax);		
		pHull->GetValue("stiffness",pone.kStiffness);
		pHull->GetValue("damping",pone.kDamping);

		pHull->GetValue("surface_id", pone.surface_idx);
		pHull->GetValue("min_friction", pone.minFriction);
		pHull->GetValue("max_friction", pone.maxFriction);
		sprintf(str,"wheel%d",i++);
		pparts.push_back(pone);
		//if (i==5) break;
	}


	if (m_pEntity->LoadVehicle(filename,&pparts[0],&params,bDestroy))
	{
		// get the steering wheel	(if any)					
		int nSlot=m_pEntity->GetSteeringWheelSlot();
		if (nSlot>=0)
		{
			// set the correct position the first time
			// since the steering wheel is at pos. 0,0,0
			Vec3 vCurrAngles;
			m_pEntity->GetObjectAngles(nSlot,vCurrAngles);
			vCurrAngles.y=0; //reset

			Vec3 vRotPointObjSpace;
			m_pEntity->GetHelperPosition("steering_pivot",vRotPointObjSpace,true);
			m_pEntity->SetObjectPos(nSlot,vRotPointObjSpace);				
			m_pEntity->SetObjectAngles(nSlot,vCurrAngles);
		}
	}

	return pH->EndFunctionNull();
}

int CScriptObjectEntity::NetPresent(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	
	bool bPresence;
	
	pH->GetParam(1,bPresence);
	m_pEntity->SetNetPresence(bPresence);
	
	return pH->EndFunction();
}

// true=prevents error when state changes on the client and does not sync state changes to the client 
int CScriptObjectEntity::SetStateClientside(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	
	bool bEnable;
	
	pH->GetParam(1,bEnable);
	m_pEntity->SetStateClientside(bEnable);

	return pH->EndFunction();
}


/*! Starts an animation of a character
	@param pos Number of the character slot
	@param animname Name of the aniamtion from the .cal file
	@see CScriptObjectEntity::LoadCharacter
	@see CEntity::StartAnimation
*/
int CScriptObjectEntity::StartAnimation(IFunctionHandler *pH)
{
	
	//CHECK_PARAMETERS(2);
	const char *animname;
	int pos, layer=0;
	bool bLooping = false;
	bool bLoopSpecified = false;
	float fBlendTime = 0.15f;
	float fAniSpeed = 1.0f;
	pH->GetParam(1,pos);
	if (!pH->GetParam(2,animname))
	{
		m_pISystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,
			0,"CScriptObjectEntity::StartAnimation, animation name not specified, in Entity %s",m_pEntity->GetName() );
		return pH->EndFunction(false);
	}

	if (pH->GetParamCount() > 2)
	{
	  pH->GetParam(3,layer);
		if (pH->GetParamCount() > 3)
		{
			pH->GetParam(4,fBlendTime);
			if (pH->GetParamCount() > 4)
			{
				pH->GetParam(5,fAniSpeed);
				m_pEntity->SetAnimationSpeed( fAniSpeed );
				if (pH->GetParamCount() > 5)
				{
					bLoopSpecified = true;
					pH->GetParam(6,bLooping);
				}
			}
		}
	}

	// no character no animation
	if (!m_pEntity->GetCharInterface() || !m_pEntity->GetCharInterface()->GetCharacter(pos))
		return pH->EndFunction(false);

	
	if (bLoopSpecified)
	{
		ICryCharInstance *pCharacter = m_pEntity->GetCharInterface()->GetCharacter(pos);
		if (pCharacter)
		{
			ICryAnimationSet *animSet = pCharacter->GetModel()->GetAnimationSet();
			if (animSet)
			{
				int animId = animSet->Find(animname);
				if (animId >= 0)
					animSet->SetLoop( animId,bLooping );
			}
		}
	}

	if (string(animname) == string("NULL"))
	{
		ICryCharInstance *pCharacter = m_pEntity->GetCharInterface()->GetCharacter(pos);
		bool result = pCharacter->StopAnimation(layer);
		return pH->EndFunction(result);
	}

	
	bool result = m_pEntity->StartAnimation(pos, animname, layer, fBlendTime);
	return pH->EndFunction(result);

//	return pH->EndFunction(m_pEntity->StartAnimation(pos, animname, layer, fBlendTime));

//	m_pEntity->StartAnimation(pos, animname, layer, fBlendTime);
//	return pH->EndFunction();
}

/*! Resets the animation of a character
	@param pos Number of the character slot
	@see CScriptObjectEntity::LoadCharacter
	@see CEntity::StartAnimation
*/
int CScriptObjectEntity::ResetAnimation(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
//	char *animname;
	int pos;
	pH->GetParam(1,pos);

	m_pEntity->ResetAnimations(pos);

	return pH->EndFunction();
}

/*int CScriptObjectEntity::GetID(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	

	int id = m_pEntity->GetId();

	return pH->EndFunction(id);
}*/


/*! Retrieves the position of a helper (placeholder) object
	@param helper Name of the helper object in the model
	@return Three component vector cotaining the position
*/
int CScriptObjectEntity::GetHelperPos(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(1);
	assert(pH->GetParamCount() == 1 || pH->GetParamCount() == 2);
	
	const char *helper;
	bool bUseObjectSpace =	false;

	pH->GetParam(1, helper);

	if(pH->GetParamCount() == 2)
	{
		pH->GetParam(2, bUseObjectSpace);
	}
	
	Vec3 pos;
	pos(0,0,0);

	m_pEntity->GetHelperPosition(helper, pos, bUseObjectSpace);

//	CScriptObjectVector oVec(m_pScriptSystem);
//	oVec.Set(pos);
//	return pH->EndFunction(*oVec);

	SetMemberVector( SOE_MEMBER_HELPER_POS,pos );
	return pH->EndFunction( m_memberSO[SOE_MEMBER_HELPER_POS] );
}

int CScriptObjectEntity::RenderShadow(IFunctionHandler *pH)
{
  
  if(pH->GetParamCount()<1)
  {
    m_pScriptSystem->RaiseError("CScriptObjectEntity::RenderShadow wrong number of arguments"); 
    return pH->EndFunction();
  }

  bool bRender;
  pH->GetParam(1,bRender);

  int iEntityRender=-1;
  if(pH->GetParamCount()>1)
  {
    pH->GetParam(2, iEntityRender);
  }
  
  // tiago: ok, hacked this, since i don't know if fixing this will break stuff, so correct way of using RenderShadow is to pass second parameter
  // with on/off, and keep first parameter just in case..

  // m_pEntity->SetRndFlags(ERF_CASTSHADOWVOLUME|ERF_SELFSHADOW|ERF_CASTSHADOWMAPS|ERF_RECVSHADOWMAPS, true);

  if(iEntityRender==-1)
  {
    m_pEntity->SetRndFlags(ERF_CASTSHADOWVOLUME|ERF_SELFSHADOW|ERF_CASTSHADOWMAPS|ERF_RECVSHADOWMAPS, true);
  }
  else
  {
    m_pEntity->SetRndFlags(ERF_CASTSHADOWVOLUME|ERF_SELFSHADOW|ERF_CASTSHADOWMAPS|ERF_RECVSHADOWMAPS, (iEntityRender==1)? true:false);
  }

  return pH->EndFunction();

}
/*! Controls the visibility and drawing of characters
	@param nPos Number of the character slot
	@param nMode 0 = Don't draw, 1 = Draw normally, 3 = Draw near
	@see CScriptObjectEntity::LoadCharacter
*/
int CScriptObjectEntity::DrawCharacter(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	int nPos,nMode;
	pH->GetParam(1,nPos);
	pH->GetParam(2,nMode);

	if(m_pEntity)
	{
		//((IEntityCharacter *)m_pEntity)->DrawCharacter(nPos, nMode == 1);
		m_pEntity->GetCharInterface()->DrawCharacter(nPos, nMode );
	}

	return pH->EndFunction();
}

int CScriptObjectEntity::SetRegisterInSectors(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	bool bFlag;
	pH->GetParam(1,bFlag);
	
	if(m_pEntity)
	{
		(m_pEntity)->SetRegisterInSectors(bFlag);
	}

	return pH->EndFunction();	
}

int CScriptObjectEntity::AwakePhysics(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	IPhysicalEntity *pe=m_pEntity->GetPhysics();
	if (!pe)
		return pH->EndFunction();
	
	int nAwake = 1;
	pH->GetParam(1,nAwake);
	
	if(m_pEntity)
	{
		pe_action_awake aa;
		aa.bAwake = nAwake;
		pe->Action(&aa);

		//pe_params_pos p;
		//p.iSimClass = nAwake+1;
		//pe->SetParams( &p );
	}

	return pH->EndFunction();	
}

int CScriptObjectEntity::ResetPhysics(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	IPhysicalEntity *pe=m_pEntity->GetPhysics();
	if (!pe)
		return pH->EndFunction();
	
	pe_action_reset ra;
	pe->Action(&ra);

	return pH->EndFunction();	
}


int CScriptObjectEntity::AwakeCharacterPhysics(IFunctionHandler *pH)
{
	assert(pH->GetParamCount()==2 || pH->GetParamCount()==3);
	int iSlot,nAwake=1;
	pe_action_awake aa;
	const char *pRootBoneName;
	pH->GetParam(1, iSlot);
	pH->GetParam(2, pRootBoneName);
	pH->GetParam(3, aa.bAwake);

	IPhysicalEntity *pe;
	if (m_pEntity && m_pEntity->GetCharInterface() && m_pEntity->GetCharInterface()->GetCharacter(iSlot) &&
			(pe = m_pEntity->GetCharInterface()->GetCharacter(iSlot)->GetCharacterPhysics(pRootBoneName)))
		pe->Action(&aa);

	return pH->EndFunction();
}
 
int CScriptObjectEntity::SetCharacterPhysicParams(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);
	int iSlot;
	const char *pRootBoneName;
	pH->GetParam(1, iSlot);
	pH->GetParam(2, pRootBoneName);

	IPhysicalEntity *pe;
	if (!m_pEntity || !m_pEntity->GetCharInterface() || !m_pEntity->GetCharInterface()->GetCharacter(iSlot) || 
			!(pe = m_pEntity->GetCharInterface()->GetCharacter(iSlot)->GetCharacterPhysics(pRootBoneName)))
		return pH->EndFunction();

	return SetEntityPhysicParams(pe, pH,2, m_pEntity->GetCharInterface()->GetCharacter(iSlot));
}

int CScriptObjectEntity::SetPhysicParams(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	IPhysicalEntity *pe=m_pEntity->GetPhysics();
	if (!pe)
		return pH->EndFunction();

	return SetEntityPhysicParams(pe, pH);
}


int CScriptObjectEntity::SetEntityPhysicParams(IPhysicalEntity *pe, IFunctionHandler *pH,int iOffs, ICryCharInstance *pIChar)
{
	int nType;
	pH->GetParam(1+iOffs, nType);
	CScriptObjectVector vec(m_pScriptSystem,true);
	_SmartScriptObject pTable(m_pScriptSystem,true);
	_SmartScriptObject pTempObj(m_pScriptSystem,true);
	pH->GetParam(2+iOffs, *pTable);
	int nId;
	pe_params_particle particle_params;
	pe_simulation_params sim_params;
	pe_params_car vehicle_params;
	pe_params_wheel wheel_params;
	pe_player_dynamics playerdyn_params;
	pe_player_dimensions playerdim_params;
	pe_params_articulated_body artic_params;
	pe_params_joint joint_params;
	pe_params_rope rope_params,rope_params1;
	pe_params_softbody soft_params;
	pe_params_buoyancy buoy_params;
	pe_action_add_constraint constr_params;
	pe_action_remove_constraint remove_constr_params;
	pe_params_flags flags_params;
	pe_action_set_velocity asv;
	pe_params_part pp;
	const char *strName;
	int idEnt;
	IEntity *pEnt;
	ICryBone *pBone;
	float gears[8];
	//float fDummy;
	vectorf gravity(zero); float fSpeed;

	switch (nType)
	{
		case PHYSICPARAM_FLAGS:
			if (pTable->GetValue("flags_mask", (int&)flags_params.flagsAND))	
			{
				flags_params.flagsAND = ~flags_params.flagsAND;
				pTable->GetValue("flags", (int&)flags_params.flagsOR);
			} 
			else
				pTable->GetValue("flags", (int&)flags_params.flags);
			pe->SetParams(&flags_params);
			break;
		case PHYSICPARAM_PART_FLAGS:
			if (pTable->GetValue("flags_mask", (int&)pp.flagsAND))
				pp.flagsAND = ~pp.flagsAND;
			pTable->GetValue("flags", (int&)pp.flagsOR);
			if (pTable->GetValue("flags_collider_mask", (int&)pp.flagsColliderAND))
				pp.flagsColliderAND = ~pp.flagsColliderAND;
			pTable->GetValue("flags_collider", (int&)pp.flagsColliderOR);
			if (!pTable->GetValue("partid",pp.partid))
				pp.ipart = 0;
			do {
				if (!pe->SetParams(&pp))
					break;
				pp.ipart++;
			} while(is_unused(pp.partid));
			break;
		case PHYSICPARAM_PARTICLE:
			pTable->GetValue("flags", (int&)particle_params.flags);
			pTable->GetValue("mass", particle_params.mass);
			pTable->GetValue("size", particle_params.size);
			if (pTable->GetValue("heading", *vec) && fabsf(vec.Get().GetLengthSquared()-1.0f)<0.01f)
				particle_params.heading=(vectorf)vec.Get();
			pTable->GetValue("initial_velocity", particle_params.velocity);
			pTable->GetValue("k_air_resistance", particle_params.kAirResistance);
			pTable->GetValue("k_water_resistance",particle_params.kWaterResistance);
			pTable->GetValue("acc_thrust", particle_params.accThrust);
			pTable->GetValue("acc_lift", particle_params.accLift);
			pTable->GetValue("min_bounce_vel", particle_params.minBounceVel);
			pTable->GetValue("surface_idx", particle_params.surface_idx);
			pTable->GetValue("pierceability", particle_params.iPierceability);
			if (pTable->GetValue("w", *vec))
				particle_params.wspin=(vectorf)vec.Get();
			if (pTable->GetValue("gravity", *vec))
				particle_params.gravity=(vectorf)vec.Get();
			if (pTable->GetValue("water_gravity", *vec))
				particle_params.waterGravity=(vectorf)vec.Get();
			if (pTable->GetValue("collider_to_ignore", *pTempObj))
			{
				if (pTempObj->GetValue("id",nId))
				{
					IEntity *pEntity=m_pEntitySystem->GetEntity((EntityId)nId);
					if (pEntity)
						particle_params.pColliderToIgnore=pEntity->GetPhysics();
				}
			}
			{
			bool bconstant_orientation=false;
			pTable->GetValue("constant_orientation", bconstant_orientation);
				if(bconstant_orientation){
					particle_params.flags=particle_constant_orientation|particle_no_path_alignment|particle_no_roll|particle_traceable|particle_single_contact;
				}
			}
			pe->SetParams(&particle_params);
//			m_RocketParticlePar = particle_params;
			break;
		case PHYSICPARAM_VEHICLE:
			pTable->GetValue("axle_friction", vehicle_params.axleFriction);
			pTable->GetValue("engine_power", vehicle_params.enginePower);
			pTable->GetValue("max_steer", vehicle_params.maxSteer);
			pTable->GetValue("engine_maxrpm", vehicle_params.engineMaxRPM);
			pTable->GetValue("engine_maxRPM", vehicle_params.engineMaxRPM);
			pTable->GetValue("intergration_type", vehicle_params.iIntegrationType);
			pTable->GetValue("max_time_step_vehicle", vehicle_params.maxTimeStep);
			if (pTable->GetValue("sleep_speed_vehicle", fSpeed))
				vehicle_params.minEnergy = fSpeed*fSpeed;
			pTable->GetValue("damping_vehicle",vehicle_params.damping);
			pTable->GetValue("max_braking_friction",vehicle_params.maxBrakingFriction);
			pTable->GetValue("engine_minRPM",vehicle_params.engineMinRPM);
			pTable->GetValue("engine_idleRPM",vehicle_params.engineIdleRPM);
			pTable->GetValue("engine_shiftupRPM",vehicle_params.engineShiftUpRPM);
			pTable->GetValue("engine_shiftdownRPM",vehicle_params.engineShiftDownRPM);
			pTable->GetValue("stabilizer",vehicle_params.kStabilizer);
			pTable->GetValue("clutch_speed",vehicle_params.clutchSpeed);
			if (pTable->GetValue("gears",pTempObj))
			{
				for(vehicle_params.nGears=0; vehicle_params.nGears<sizeof(gears)/sizeof(gears[0]); vehicle_params.nGears++)
					if (!pTempObj->GetAt(vehicle_params.nGears+1, gears[vehicle_params.nGears]))
						break;
				vehicle_params.gearRatios = gears;
			}
			pTable->GetValue("brake_torque",vehicle_params.brakeTorque);
      pTable->GetValue("dyn_friction_ratio",vehicle_params.kDynFriction);
			pTable->GetValue("gear_dir_switch_RPM",vehicle_params.gearDirSwitchRPM);
			pTable->GetValue("slip_threshold",vehicle_params.slipThreshold);
			pTable->GetValue("engine_startRPM",vehicle_params.engineStartRPM);
			pe->SetParams(&vehicle_params);
			break;
		case PHYSICPARAM_WHEEL:
			pTable->GetValue("wheel", wheel_params.iWheel);
			pTable->GetValue("is_driving", wheel_params.bDriving);
			pTable->GetValue("susp_len", wheel_params.suspLenMax);
			pTable->GetValue("min_friction", wheel_params.minFriction);
			pTable->GetValue("max_friction", wheel_params.maxFriction);
			pTable->GetValue("surface_idx", wheel_params.surface_idx);
			pe->SetParams(&wheel_params);
			break;
		case PHYSICPARAM_SIMULATION:
			pTable->GetValue("max_time_step", sim_params.maxTimeStep);
			if (pTable->GetValue("sleep_speed", fSpeed))
				sim_params.minEnergy = fSpeed*fSpeed;
			if (pTable->GetValue("gravity", *vec))
				sim_params.gravity = (vectorf)vec.Get();
			if (pTable->GetValue("gravityx",gravity.x) | pTable->GetValue("gravityy",gravity.y) | pTable->GetValue("gravityz",gravity.z))
				sim_params.gravity = gravity;
			if (pTable->GetValue("freefall_gravity", *vec))
				sim_params.gravityFreefall = (vectorf)vec.Get();
			gravity.Set(0,0,0);
			if (pTable->GetValue("freefall_gravityx",gravity.x) | pTable->GetValue("freefall_gravityy",gravity.y) | 
					pTable->GetValue("freefall_gravityz",gravity.z))
				sim_params.gravityFreefall = gravity;
			pTable->GetValue("damping", sim_params.damping);
			pTable->GetValue("freefall_damping", sim_params.dampingFreefall);
			pTable->GetValue("softness", sim_params.softness);
			pTable->GetValue("angular_softness", sim_params.softnessAngular);
			pTable->GetValue("softness_group", sim_params.softnessGroup);
			pTable->GetValue("angular_softness_group", sim_params.softnessAngularGroup);
			pTable->GetValue("mass", sim_params.mass);
			pTable->GetValue("density", sim_params.density);
			//if (pTable->GetValue("water_density",fDummy))
			//	m_pEntity->SetWaterDensity(fDummy);

			pe->SetParams(&sim_params);
			break;
		case PHYSICPARAM_VELOCITY:
			if (pTable->GetValue("v",*vec))
				asv.v = vec.Get();
			if (pTable->GetValue("w",*vec))
				asv.w = vec.Get();
			pe->Action(&asv);
			break;
		case PHYSICPARAM_BUOYANCY:
			pTable->GetValue("water_density", buoy_params.waterDensity);
			pTable->GetValue("water_damping", buoy_params.waterDamping);
			pTable->GetValue("water_resistance", buoy_params.waterResistance);
			if (pTable->GetValue("water_sleep_speed", fSpeed))
				buoy_params.waterEmin = fSpeed*fSpeed;
			if (pTable->GetValue("water_normal", *vec))
				buoy_params.waterPlane.n = (vectorf)vec.Get();
			if (pTable->GetValue("water_origin", *vec))
				buoy_params.waterPlane.origin = (vectorf)vec.Get();
			pe->SetParams(&buoy_params);
			break;
		case PHYSICPARAM_ARTICULATED:
			pTable->GetValue("lying_mode_ncolls", artic_params.nCollLyingMode);
			if (pTable->GetValue("lying_gravity", *vec))
				artic_params.gravityLyingMode = (vectorf)vec.Get();
			if (pTable->GetValue("lying_gravityx",gravity.x) | pTable->GetValue("lying_gravityy",gravity.y) | 
					pTable->GetValue("lying_gravityz",gravity.z))
				artic_params.gravityLyingMode = gravity;
			pTable->GetValue("lying_damping", artic_params.dampingLyingMode);
			if (pTable->GetValue("lying_sleep_speed", fSpeed))
				artic_params.minEnergyLyingMode = fSpeed*fSpeed;
			pTable->GetValue("is_grounded", artic_params.bGrounded);
			if (pTable->GetValue("check_collisions", artic_params.bCheckCollisions))
				artic_params.bCollisionResp = artic_params.bCheckCollisions;
			pTable->GetValue("sim_type", artic_params.iSimType);
			pTable->GetValue("lying_sim_type", artic_params.iSimTypeLyingMode);
			pTable->GetValue("expand_hinges", artic_params.bExpandHinges);
			pe->SetParams(&artic_params);
			break;
		case PHYSICPARAM_JOINT:
			pTable->GetValue("bone_name", strName);
			if (pIChar && (pBone=pIChar->GetBoneByName(strName))) 
			{
				pTable->GetValue("flags", (int&)joint_params.flags);
				if (pTable->GetValue("min", *vec))
					joint_params.limits[0] = (vectorf)vec.Get();
				if (pTable->GetValue("max", *vec))
					joint_params.limits[1] = (vectorf)vec.Get();
				if (pTable->GetValue("stiffness", *vec))
					joint_params.ks = (vectorf)vec.Get();
				if (pTable->GetValue("damping", *vec))
					joint_params.kd = (vectorf)vec.Get();
				if (pTable->GetValue("dashpot", *vec))
					joint_params.qdashpot = (vectorf)vec.Get();
				if (pTable->GetValue("kdashpot", *vec))
					joint_params.kdashpot = (vectorf)vec.Get();
				pe->SetParams(&joint_params);
			}
			break;
		case PHYSICPARAM_ROPE:
			pTable->GetValue("length", rope_params.length);
			pTable->GetValue("mass", rope_params.mass);
			pTable->GetValue("coll_dist", rope_params.collDist);
			pTable->GetValue("surface_idx", rope_params.surface_idx);
			pTable->GetValue("friction", rope_params.friction);

			pe->GetParams(&rope_params1);
			if (rope_params1.pEntTiedTo[0]==0 || rope_params1.pEntTiedTo[1]==0)
			{
				int iEnd = rope_params1.pEntTiedTo[1]==0;
				if (pTable->GetValue("entity_id", idEnt)) 
				{	
					if (idEnt<-1)
						rope_params.pEntTiedTo[iEnd] = 0;
					else if (idEnt!=-1 && (pEnt=m_pEntitySystem->GetEntity(idEnt)) && pEnt->GetPhysics())
						rope_params.pEntTiedTo[iEnd] = pEnt->GetPhysics();
					else
						rope_params.pEntTiedTo[iEnd] = WORLD_ENTITY;
				}
				pTable->GetValue("entity_part_id", rope_params.idPartTiedTo[iEnd]);
				if (pTable->GetValue("end", *vec))
					rope_params.ptTiedTo[iEnd] = (vectorf)vec.Get();		
			}

			if (pTable->GetValue("entity_id_2", idEnt)) 
			{			
				if (idEnt<-1)
						rope_params.pEntTiedTo[1] = 0;
				else if (idEnt!=-1 && (pEnt=m_pEntitySystem->GetEntity(idEnt)) && pEnt->GetPhysics())
					rope_params.pEntTiedTo[1] = pEnt->GetPhysics();
				else
					rope_params.pEntTiedTo[1] = WORLD_ENTITY;
			}
			pTable->GetValue("entity_part_id_2", rope_params.idPartTiedTo[1]);
			if (pTable->GetValue("end2", *vec))
				rope_params.ptTiedTo[1] = (vectorf)vec.Get();
			
			if (pTable->GetValue("entity_id_1", idEnt))
			{
				if (idEnt<-1)
					rope_params.pEntTiedTo[0] = 0;
				else if (idEnt!=-1 && (pEnt=m_pEntitySystem->GetEntity(idEnt)) && pEnt->GetPhysics())
					rope_params.pEntTiedTo[0] = pEnt->GetPhysics();
				else
					rope_params.pEntTiedTo[0] = WORLD_ENTITY;
			}
			pTable->GetValue("entity_part_id_1", rope_params.idPartTiedTo[0]);
			if (pTable->GetValue("end1", *vec))
				rope_params.ptTiedTo[0] = (vectorf)vec.Get();

			pe->SetParams(&rope_params);

			flags_params.flagsOR = 0;
			flags_params.flagsAND = -1;
			if (pTable->GetValue("check_collisions", idEnt))
				if (idEnt)
					flags_params.flagsOR = rope_collides;
				else
					flags_params.flagsAND = ~rope_collides;
			if (pTable->GetValue("bCheckCollisions", idEnt))
				if (idEnt)
					flags_params.flagsOR = rope_collides;
				else
					flags_params.flagsAND = ~rope_collides;
			if (pTable->GetValue("bCheckTerrainCollisions", idEnt))
				if (idEnt)
					flags_params.flagsOR |= rope_collides_with_terrain;
				else
					flags_params.flagsAND &= ~rope_collides_with_terrain;
			if (pTable->GetValue("shootable", idEnt))
				if (idEnt)
					flags_params.flagsOR |= rope_traceable;
				else
					flags_params.flagsAND &= ~rope_traceable;
			if (pTable->GetValue("bShootable", idEnt))
				if (idEnt)
					flags_params.flagsOR |= rope_traceable;
				else
					flags_params.flagsAND &= ~rope_traceable;
			pe->SetParams(&flags_params);

			break;
		case PHYSICPARAM_SOFTBODY:
			pTable->GetValue("thickness", soft_params.thickness);
			pTable->GetValue("max_safe_step", soft_params.maxSafeStep);
			pTable->GetValue("stiffness", soft_params.ks);
			pTable->GetValue("damping_ratio", soft_params.kdRatio);
			pTable->GetValue("air_resistance", soft_params.airResistance);
			if (pTable->GetValue("wind", *vec))
				soft_params.wind = (vectorf)vec.Get();
			pTable->GetValue("max_iters", soft_params.nMaxIters);
			pTable->GetValue("accuracy", soft_params.accuracy);
			pTable->GetValue("friction", soft_params.friction);
			pTable->GetValue("impulse_scale", soft_params.impulseScale);
			pTable->GetValue("explosion_scale", soft_params.explosionScale);
			pTable->GetValue("collision_impulse_scale", soft_params.collisionImpulseScale);
			pTable->GetValue("max_collision_impulse", soft_params.maxCollisionImpulse);
			pTable->GetValue("collision_mask", soft_params.collTypes);
			pe->SetParams(&soft_params);
			break;
		case PHYSICPARAM_CONSTRAINT:
			if (!(pTable->GetValue("entity_id",idEnt) && idEnt!=-1 && (pEnt=m_pEntitySystem->GetEntity(idEnt)) && (constr_params.pBuddy=pEnt->GetPhysics())))
				constr_params.pBuddy = WORLD_ENTITY;
			pTable->GetValue("entity_part_id_1", constr_params.partid[0]);
			pTable->GetValue("entity_part_id_2", constr_params.partid[2]);
			constr_params.flags = local_frames;
			pTable->GetValue("xmin", constr_params.xlimits[0]);
			pTable->GetValue("xmax", constr_params.xlimits[1]);
			pTable->GetValue("yzmin", constr_params.yzlimits[0]);
			pTable->GetValue("yzmax", constr_params.yzlimits[1]);
			return pH->EndFunction(pe->Action(&constr_params));
		case PHYSICPARAM_REMOVE_CONSTRAINT:
			pTable->GetValue("id", remove_constr_params.idConstraint);
			pe->Action(&remove_constr_params);
			break;
		case PHYSICPARAM_PLAYERDYN:
			pTable->GetValue("k_inertia", playerdyn_params.kInertia);
			pTable->GetValue("k_air_control", playerdyn_params.kAirControl);
			pTable->GetValue("gravity", playerdyn_params.gravity);
			pTable->GetValue("bSwimming", playerdyn_params.bSwimming);
			pTable->GetValue("mass", playerdyn_params.mass);
			pTable->GetValue("surface_idx", playerdyn_params.surface_idx);
			pTable->GetValue("is_active", playerdyn_params.bActive);
			pe->SetParams(&playerdyn_params);
			break;
		case PHYSICPARAM_PLAYERDIM:
			pTable->GetValue("pivot_height", playerdim_params.heightPivot);
			pTable->GetValue("eye_height", playerdim_params.heightEye);
			pTable->GetValue("cyl_r", playerdim_params.sizeCollider.x);
			if (!pTable->GetValue("cyl_height", playerdim_params.sizeCollider.z))
				playerdim_params.sizeCollider.z = playerdim_params.sizeCollider.x;
			playerdim_params.sizeCollider.y = playerdim_params.sizeCollider.x;
			pTable->GetValue("cyl_pos", playerdim_params.heightCollider);
			pe->SetParams(&playerdim_params);
			break;
	}
	return pH->EndFunction();
}

int CScriptObjectEntity::GetParticleCollisionStatus(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	IPhysicalEntity *pe=m_pEntity->GetPhysics();
	if (pe)
	{
		pe_status_collisions sc;
		coll_history_item hit;
		sc.pHistory = &hit;

		if (pe->GetStatus(&sc))
		{
			IEntity *collider =NULL;
			//if (hit.pCollider)
      //{ collider = (IEntity*)hit.pCollider->GetForeignData();
			IPhysicalEntity *pCollider = m_pISystem->GetIPhysicalWorld()->GetPhysicalEntityById(hit.idCollider);
			if (pCollider)
				collider = (IEntity*)pCollider->GetForeignData();

//			int type=collider->GetType();
			//string pszName=collider->GetEntityClassName();
			//}
			int nType;
			if ( collider )
				nType = 0;
			//else if ( status.pLastCollider )
			//	nType = 1;
			else
				nType = 2; //terrain
//<<FIXME>> remove this table ceration every frame
			_SmartScriptObject pObj(m_pScriptSystem);
			_SmartScriptObject oPos(m_pScriptSystem),oNormal(m_pScriptSystem),oDir(m_pScriptSystem);

			pObj->SetValue("IsPlayer", 0);
			if (collider)
			{
				void *pInterface = NULL;
				IEntityContainer *pICnt = collider->GetContainer();
				if (pICnt)
					if (pICnt->QueryContainerInterface(CIT_IPLAYER, &pInterface))
					{
						// We have a player
						pObj->SetValue("IsPlayer", 1);
					}
			}

		//	IScriptObject *pTargetMaterial;

			/*if(pTargetMaterial=m_pGame->m_XSurfaceMgr.GetMaterialBySurfaceID(hit.idmat[1]))
			{
				pObj->SetValue("target_material",pTargetMaterial);
			}
			else
			{*/
				pObj->SetValue("target_material",hit.idmat[1]);
			//}

			oPos->SetValue("x",hit.pt.x);
			oPos->SetValue("y",hit.pt.y);
			oPos->SetValue("z",hit.pt.z);
			oNormal->SetValue("x",hit.n.x);
			oNormal->SetValue("y",hit.n.y);
			oNormal->SetValue("z",hit.n.z);
			Vec3 vrel = (hit.v[0]-hit.v[1]).normalized();
			oDir->SetValue("x",vrel.x);
			oDir->SetValue("y",vrel.y);
			oDir->SetValue("z",vrel.z);
			pObj->SetValue("objtype",nType);
			pObj->SetValue("pos",*oPos);
			pObj->SetValue("normal",*oNormal);
			pObj->SetValue("dir",*oDir);
			if (collider && collider->GetScriptObject())
				pObj->SetValue("target",collider->GetScriptObject());
			return pH->EndFunction(*pObj);
		}
	}
	return pH->EndFunctionNull();
}

int CScriptObjectEntity::GetObjectStatus(IFunctionHandler *pH)
{
	// Given the slot number this routine returns a table containing all
	// useful information about a script object
	
	CHECK_PARAMETERS(1);
	_SmartScriptObject pTable(m_pScriptSystem);
	CScriptObjectVector oVecPos(m_pScriptSystem);
	CScriptObjectVector oVecAngles(m_pScriptSystem);
	CScriptObjectVector oVecScale(m_pScriptSystem);
	CScriptObjectVector oVecOffset(m_pScriptSystem);
	
	int nSlot;
	CEntityObject theEntityObject;

	pH->GetParam(1,nSlot);	

	if (m_pEntity && m_pEntity->GetEntityObject(nSlot, theEntityObject))
	{
		oVecPos = theEntityObject.pos;
		oVecAngles = theEntityObject.angles;
		oVecScale = theEntityObject.scale;

		// Calculate the mispoint of the bounding box
		oVecOffset = (theEntityObject.object->GetBoxMax() + theEntityObject.object->GetBoxMin()) / 2.0; 
		
		pTable->SetValue("flags", theEntityObject.flags);
		pTable->SetValue("pos", *oVecPos);
		pTable->SetValue("angles", *oVecAngles);
		pTable->SetValue("scale", *oVecScale);
		pTable->SetValue("offset", *oVecOffset);

		return pH->EndFunction(*pTable);
	}

	return pH->EndFunctionNull();
}

int CScriptObjectEntity::SetObjectStatus(IFunctionHandler *pH)
{
	// Given a slot number and a table this routine finds the indicated
	// script object and updates it with the information contained in the table
	
	CHECK_PARAMETERS(2);
	_SmartScriptObject pTable(m_pScriptSystem, true);
	CScriptObjectVector oVecPos(m_pScriptSystem);
	CScriptObjectVector oVecAngles(m_pScriptSystem);
	CScriptObjectVector oVecScale(m_pScriptSystem);
	CScriptObjectVector oVecOffset(m_pScriptSystem);
	CEntityObject theEntityObject;
	int nSlot;

	pH->GetParam(1,nSlot);
	pH->GetParam(2,*pTable);

	if (m_pEntity && m_pEntity->GetEntityObject(nSlot, theEntityObject))
	{
		pTable->GetValue("flags", theEntityObject.flags);
		pTable->GetValue("pos", *oVecPos);
		pTable->GetValue("angles", *oVecAngles);
		pTable->GetValue("scale", *oVecScale);
		pTable->GetValue("offset", *oVecOffset);

		theEntityObject.pos = oVecPos.Get();
		theEntityObject.angles = oVecAngles.Get();
		theEntityObject.scale = oVecScale.Get();
		
		m_pEntity->SetEntityObject(nSlot, theEntityObject);

		return pH->EndFunction();
	}
	
	return pH->EndFunctionNull();
}

/*! Retrieves if a characters currently plays an animation
	@param iAnimationPos Number of the character slot
	@return nil or not nil
	@see CScriptObjectEntity::StartAnimation
*/
int CScriptObjectEntity::IsAnimationRunning(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	int iAnimationPos;
	ICryCharInstance *pCharacter = NULL;
	
	pH->GetParam(1, iAnimationPos);
	
	pCharacter = m_pEntity->GetCharInterface()->GetCharacter(iAnimationPos);

	bool bResult=false;
	if (pCharacter)
		if (pCharacter->GetCurAnimation())
			bResult=true;

	return pH->EndFunction(bResult);
}

int CScriptObjectEntity::AddImpulse(IFunctionHandler *pH)
{
	assert(pH->GetParamCount()==4 || pH->GetParamCount()==5);


//	float hitImpulse = m_pGame->p_HitImpulse->GetFVal();
//	if(hitImpulse == 0)
//		return pH->EndFunctionNull();

	int ipart;
	bool bPos;
	Vec3 pos,dir;
	float impulse,impulseScale=1.0f;

	pH->GetParam(1, ipart);

	CScriptObjectVector oVec(m_pScriptSystem,true);
	if(pH->GetParam(2,*oVec))
	{
		pos=oVec.Get();
		bPos=true;
	}
	else
	{
		bPos=false;
	}
	pH->GetParam(3,*oVec); dir=oVec.Get();
	pH->GetParam(4, impulse);
	pH->GetParam(5, impulseScale);
	bPos = bPos && GetLengthSquared(pos)>0;

	if (GetLengthSquared(dir)>0)
	{
		m_pEntity->AddImpulse(ipart,pos,GetNormalized(dir)*impulse,bPos,impulseScale );
//		m_pEntity->AddImpulse(ipart,pos,dir.Normalized()*impulse*hitImpulse,bPos );
	}

	return pH->EndFunctionNull();
}

int CScriptObjectEntity::AddImpulseObj(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	Vec3 dir;
	float impulse;

	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*oVec); 
	dir=oVec.Get();
	pH->GetParam(2, impulse);
	if (GetLengthSquared(dir)*impulse>0) 
	{
		m_pEntity->AddImpulse(0,m_pEntity->GetPos(),GetNormalized(dir)*impulse,true);
		m_pEntity->AddImpulse(0,m_pEntity->GetPos(),GetNormalized(dir)*impulse*2,false);
	}
	return pH->EndFunctionNull();
}


/*! Determines whether a given point is within an entities radius
	@param Vector 3d representing a point in world space
	@return true or false
	@see CEntity::GetRadius
*/
int CScriptObjectEntity::IsPointWithinRadius(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	Vec3 vPosition;

	// Get the passed position vector
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*oVec); 
	vPosition = oVec.Get();

	// Sean, Nov 23
	// The players radius is far too large, like 5+ meters
	// So I am scaling it by 50% for now.
	// Talk to Petar about fixing player radius
//	TRACE("Fix hack in CScriptObjectEntity::IsPointWithinRadius");
	float HackedValue = m_pEntity->GetRadius()*.5f;// / 2.0f;
//	float HackedValue = m_pEntity->GetRadiusPhys()*.5f;// / 2.0f;
	
	// Calculate the distance of this position from the player
	// If the distance is within the player's radius, return true
	// otherwise return false

//Vec3	epos = m_pEntity->GetPos(false);
//float dst = vPosition.Distance(m_pEntity->GetPos(false));	
	return pH->EndFunction( GetDistance(vPosition,m_pEntity->GetPos(false)) <= HackedValue);

}

/*! Determines distance between the entity and a given 3d point
	@param Vector 3d representing a point in world space
	@return distance as float
	@see CEntity::GetPos
*/
int CScriptObjectEntity::GetDistanceFromPoint(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	Vec3 vPosition;

	// Get the passed position vector
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*oVec); 
	vPosition = oVec.Get();

	// Calculate the distance of this position from the player
	return pH->EndFunction( GetDistance(vPosition,m_pEntity->GetPos(false)) );
}




int CScriptObjectEntity::EnableSave(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	bool bEnable;
	pH->GetParam(1,bEnable);
	m_pEntity->EnableSave(bEnable);

	return pH->EndFunction();
}

int CScriptObjectEntity::PlaySound(IFunctionHandler *pH)
{
	int nCookie=0;
	float fVolumeScale=1.0f;
	ISound *pSound=NULL;

	pH->GetParamUDVal(1,(INT_PTR&)pSound,nCookie);

	if(pSound && (nCookie==USER_DATA_SOUND))
	{
		if (pH->GetParamCount()>1)
		{
			if (!pH->GetParam(2,fVolumeScale))
			{
				fVolumeScale=1.0f;
			}
		}
		Vec3 Offset;
		if(pH->GetParamCount()>=3)
		{
			CScriptObjectVector oVec(m_pScriptSystem, true);
			pH->GetParam(3, *oVec);
			Offset=oVec.Get();
		}else
		{
			Offset=Vec3(0.0f, 0.0f, 0.0f);
		}
		m_pEntity->PlaySound(pSound, fVolumeScale, Offset);
		//pSound->SetPosition(m_pEntity->GetPos());
		//pSound->Play(fVolumeScale);
	}
	/*
	else
	{
		m_pScriptSystem->RaiseError("PlaySound NULL SOUND!!");
	}
	*/
	
	return pH->EndFunction();
}



int CScriptObjectEntity::TriggerEvent(IFunctionHandler *pH)
{

	int eventType;

	pH->GetParam(1,eventType);
	SAIEVENT eventParams;

	switch (eventType)
	{
		case AIEVENT_ONBODYSENSOR:
		{
			float fSuspendFireTimeout;
			pH->GetParam(2,fSuspendFireTimeout);
			eventParams.fInterest = fSuspendFireTimeout;	// interest used just for convinience
		}
		break;
		case AIEVENT_AGENTDIED:
		{
			if (pH->GetParamCount()>1)
				pH->GetParam(2,eventParams.nDeltaHealth);
			else
				eventParams.nDeltaHealth = 0;
		}
		break;
		case AIEVENT_SLEEP:
		case AIEVENT_WAKEUP:
		case AIEVENT_ENABLE:
		case AIEVENT_DISABLE:
		case AIEVENT_REJECT:
		case AIEVENT_PATHFINDON:
		case AIEVENT_PATHFINDOFF:
		case AIEVENT_CLEAR:
		case AIEVENT_DROPBEACON:
			break;
		default:
			return pH->EndFunction();
	}

	if (m_pEntity->GetAI())
			m_pEntity->GetAI()->Event(eventType,&eventParams);

	return pH->EndFunction();
}

int CScriptObjectEntity::SetSecondShader(IFunctionHandler *pH)
{
  CHECK_PARAMETERS(2);
  const char *pszName;
  int iMask=0;
  pH->GetParam(1,pszName);   // shader name
  pH->GetParam(2,iMask);     // where to apply shader ? (0= char only, 1=char + attached, 2=char + hands only, 3=char + hands + attached, 4=all)
  
  if(!m_pEntity)
  {
    return pH->EndFunction();
  }

  IEntityCharacter *pEntChar=m_pEntity->GetCharInterface();

  // where to use ?
  bool bOnCharAttached=0, bOnArmsAttached=0, bUseOnArms=0;
  switch(iMask) {
  case 1:    
    bUseOnArms=0;
    bOnCharAttached=1;
    bOnArmsAttached=0;
  	break;
  case 2:    
    bUseOnArms=1;
    bOnCharAttached=0;
    bOnArmsAttached=0;
    break;
  case 3:    
    bUseOnArms=1;
    bOnCharAttached=0;
    bOnArmsAttached=1;
    break;
  case 4:    
    bUseOnArms=1;
    bOnCharAttached=1;
    bOnArmsAttached=1;
    break;

  default:
    bUseOnArms=0;
    bOnCharAttached=0;
    bOnArmsAttached=0;
    break;
  }

  // set third person shader
  if(pEntChar->GetCharacter(0))
  {    
    pEntChar->GetCharacter(0)->SetShaderTemplateName(pszName, 1, 0, m_pEntity->GetMaterial(),(bOnCharAttached)?1:0);    
  }

  if(m_pEntity->GetIStatObj(0))
  {
    m_pEntity->GetIStatObj(0)->SetShaderTemplate(-1, pszName, 0);
  }  
      
  // set first person shader
  if(bUseOnArms) 
  {
    if(pEntChar && pEntChar->GetCharacter(1))
    {
      // hack: mask out player hands if flag for useOnAttached not set, else use normal rendering (arms/weapon with same shader)    
      if(!bOnArmsAttached)
      {
        // this can only be in setSecondShader, since it disables second pass rendering of attached objects
        pEntChar->GetCharacter(1)->SetShaderTemplateName("TemplNull", 1, 0, m_pEntity->GetMaterial(), 0);    
        pEntChar->GetCharacter(1)->SetShaderTemplateName(pszName, 1, "s_hands", m_pEntity->GetMaterial(), 0);    

      }
      else
      {      
        pEntChar->GetCharacter(1)->SetShaderTemplateName(pszName, 1, 0, m_pEntity->GetMaterial(), 0);    
      }    
    }

    if(m_pEntity->GetIStatObj(1))
    {
      m_pEntity->GetIStatObj(1)->SetShaderTemplate(-1, pszName, 0);  
    }
  }
         
  return pH->EndFunction();
}

// note: this should be something more generic/abstract, something like SetShader(iShaderLayer, szShaderName, pUseList(list of items in entity, to use this shader))
int CScriptObjectEntity::SetShader(IFunctionHandler *pH)
{
  CHECK_PARAMETERS(2);
  const char *pszName;
  int iMask=0;
  pH->GetParam(1,pszName);   // shader name
  pH->GetParam(2,iMask);     // where to apply shader ? (0= char only, 1=char + attached, 2=char + hands only, 3=char + hands + attached, 4=all)

  if(!m_pEntity)
  {
    return pH->EndFunction();
  }

  IEntityCharacter *pEntChar=m_pEntity->GetCharInterface();

  // where to use ?
  bool bOnCharAttached=0, bOnArmsAttached=0, bUseOnArms=0;
  switch(iMask) {
  case 1:    
    bUseOnArms=0;
    bOnCharAttached=1;
    bOnArmsAttached=0;
    break;
  case 2:    
    bUseOnArms=1;
    bOnCharAttached=0;
    bOnArmsAttached=0;
    break;
  case 3:    
    bUseOnArms=1;
    bOnCharAttached=0;
    bOnArmsAttached=1;
    break;
  case 4:    
    bUseOnArms=1;
    bOnCharAttached=1;
    bOnArmsAttached=1;
    break;

  default:
    bUseOnArms=0;
    bOnCharAttached=0;
    bOnArmsAttached=0;
    break;
  }

  // set third person shader
  if(pEntChar->GetCharacter(0))
  {    
    pEntChar->GetCharacter(0)->SetShaderTemplateName(pszName, 0, 0, m_pEntity->GetMaterial(),(bOnCharAttached)?1:0);    
  }

  if(m_pEntity->GetIStatObj(0))
  {
    m_pEntity->GetIStatObj(0)->SetShaderTemplate(-1, pszName, 0);
  }  

  // set first person shader
  if(bUseOnArms) 
  {
    if(pEntChar && pEntChar->GetCharacter(1))
    {
      // hack: mask out player hands if flag for useOnAttached not set, else use normal rendering (arms/weapon with same shader)    
      if(!bOnArmsAttached)
      {
        // this can only be in setSecondShader, since it disables second pass rendering of attached objects
        // pEntChar->GetCharacter(1)->SetShaderTemplateName("TemplNull", 1, 0, m_pEntity->GetMaterial(), 0);    
        pEntChar->GetCharacter(1)->SetShaderTemplateName(pszName, 0, "s_hands", m_pEntity->GetMaterial(), 0);    
      }
      else
      {      
        pEntChar->GetCharacter(1)->SetShaderTemplateName(pszName, 0, 0, m_pEntity->GetMaterial(), 0);    
      }    
    }

    if(m_pEntity->GetIStatObj(1))
    {
      m_pEntity->GetIStatObj(1)->SetShaderTemplate(-1, pszName, 0);  
    }
  }
  
  return pH->EndFunction();
}

int CScriptObjectEntity::GetShader(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	// TODO
//	if(m_pPlayer->GetEntity() && m_pPlayer->GetEntity()->GetCharInterface() && m_pPlayer->GetEntity()->GetCharInterface()->GetCharacter(0))
  //  return pH->EndFunction(m_pPlayer->GetEntity()->GetCharInterface()->GetCharacter(0)->GetShaderTemplateId());

  return pH->EndFunction(0);
}

int CScriptObjectEntity::GetCameraPosition(IFunctionHandler *pH)
{
	Vec3 vPos;
	IEntityCamera *pICam = NULL;

	CHECK_PARAMETERS(0);

	pICam = m_pEntity->GetCamera(); 

	if (!pICam)
		return pH->EndFunction();

	vPos = pICam->GetPos();
	
	m_pCameraPosition->SetValue("x", vPos.x);
	m_pCameraPosition->SetValue("y", vPos.y);
	m_pCameraPosition->SetValue("z", vPos.z);
	return pH->EndFunction(m_pCameraPosition);

	
}

int CScriptObjectEntity::GetCameraAngles(IFunctionHandler *pH)
{
	Vec3 vAng;
	IEntityCamera *pICam = NULL;

	CHECK_PARAMETERS(0);

	pICam = m_pEntity->GetCamera(); 

	if (!pICam)
		return pH->EndFunction();

	vAng = pICam->GetAngles();
	
	m_pCameraPosition->SetValue("x", vAng.x);
	m_pCameraPosition->SetValue("y", vAng.y);
	m_pCameraPosition->SetValue("z", vAng.z);
	return pH->EndFunction(m_pCameraPosition);

	
}


int CScriptObjectEntity::LoadBreakable(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sFileName;

	pH->GetParam(1,sFileName);

	if (m_pEntity)
		m_pEntity->LoadBreakableObject(sFileName);

  return pH->EndFunction();
}

int CScriptObjectEntity::BreakEntity(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(2);

	Vec3 vDir(0,0,1);
  CScriptObjectVector oVec(m_pScriptSystem,true);
	if (pH->GetParam(1,*oVec))
	{
		vDir = oVec.Get();
	}
	float	explosionForce = 0.01f; // small small force.
	pH->GetParam(2,explosionForce); 
	
	float fDensity = 10;
	float fLifeTime = 20;
	bool bRigidBody = false;
	pH->GetParam(3,bRigidBody);
	pH->GetParam(4,fLifeTime);
	pH->GetParam(5,fDensity);

  if (m_pEntity)
  for(int n=2; n<m_pEntity->GetNumObjects(); n++)
  {
    CEntityObject obj;
    if(!m_pEntity->GetEntityObject(n,obj))
      continue;

    Vec3 vOffSet = (obj.object->GetBoxMin()+obj.object->GetBoxMax())*0.5f;
		Vec3 vSize = obj.object->GetBoxMax()-obj.object->GetBoxMin();

		// get entity matrix
		//Matrix44 EntityMatrix;
		//EntityMatrix.Identity();
		//EntityMatrix = GetTranslationMat(m_pEntity->GetPos())*EntityMatrix;
		//EntityMatrix = GetRotationZYX44(-gf_DEGTORAD*m_pEntity->GetAngles())*EntityMatrix;
		//EntityMatrix = GetScale33( Vec3(m_pEntity->GetScale(),m_pEntity->GetScale(),m_pEntity->GetScale()) )*EntityMatrix;

		//OPTIMISED_BY_IVO  
		Matrix33diag diag		=	Vec3(m_pEntity->GetScale(),m_pEntity->GetScale(),m_pEntity->GetScale());		//use diag-matrix for scaling
		Matrix34 rt34				=	Matrix34::CreateRotationXYZ( Deg2Rad(m_pEntity->GetAngles()), m_pEntity->GetPos() );	//set rotation and translation in one function call
		Matrix44 EntityMatrix	=	rt34*diag;			//optimised concatenation: m34*diag
		
		EntityMatrix	=	GetTransposed44(EntityMatrix);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44



		vOffSet = EntityMatrix.TransformVectorOLD(vOffSet);

    ParticleParams SpawnParticleParams;
    SpawnParticleParams.vPosition = m_pEntity->GetPos()+vOffSet;
    SpawnParticleParams.vDirection = vDir + vOffSet;//Vec3 (0,0,0);
    SpawnParticleParams.fFocus = 30.f;
    SpawnParticleParams.vColorStart = Vec3 (0,1,1);
    SpawnParticleParams.vColorEnd   = Vec3 (1,1,0);
//    SpawnParticleParams.fSpeed = 1.0f;
    SpawnParticleParams.fSpeed = explosionForce*0.7f;//0.25f;
		SpawnParticleParams.fSpeed.variation = 0.25f;
		SpawnParticleParams.nCount = 1;
    //SpawnParticleParams.fSize = max(max(vSize.x,vSize.y),vSize.z);
		SpawnParticleParams.fSize = m_pEntity->GetScale();
		SpawnParticleParams.fThickness = max(0.02f,min(min(vSize.x,vSize.y),vSize.z));
		SpawnParticleParams.vNormal.Set(0,0,0);
		SpawnParticleParams.vNormal[idxmin3((float*)&vSize)] = 1.0f;
		SpawnParticleParams.iPhysMat = m_pISystem->GetI3DEngine()->GetPhysMaterialEnumerator()->EnumPhysMaterial(obj.object->GetScriptMaterialName());
    SpawnParticleParams.fSizeSpeed = 0;
//    SpawnParticleParams.vGravity = Vec3 (0,0,-0.5);
    SpawnParticleParams.vGravity = Vec3 (0,0,-9.81f);//-4.5);
//    SpawnParticleParams.fLifeTime = 4;
    SpawnParticleParams.fLifeTime = fLifeTime;
    SpawnParticleParams.nTexId = 0;
    SpawnParticleParams.nTexAnimFramesCount = 0;
    SpawnParticleParams.eBlendType = ParticleBlendType_AlphaBased;
    SpawnParticleParams.nParticleFlags = 0;
    SpawnParticleParams.fTailLenght = 0;
    SpawnParticleParams.bRealPhysics = true;
    SpawnParticleParams.vRotation = explosionForce*Vec3(float(3.f*(rnd()-0.5)),float(3.f*(rnd()-0.5)),float(3.f*(rnd()-0.5)));
    SpawnParticleParams.pStatObj = obj.object;
		SpawnParticleParams.vInitAngles = m_pEntity->GetAngles();
		SpawnParticleParams.fBouncenes = 0.5f;

		if (bRigidBody)
		{
			SpawnParticleParams.vPosition = m_pEntity->GetPos();
			SpawnParticleParams.vDirection = vDir;

			SpawnParticleParams.fThickness = fDensity;
			SpawnParticleParams.fSize = m_pEntity->GetScale();
			SpawnParticleParams.nParticleFlags |= PART_FLAG_RIGIDBODY;
		}

    m_pISystem->GetI3DEngine()->SpawnParticles(SpawnParticleParams);
  }

	return pH->EndFunction();
}

int CScriptObjectEntity::SetBBox(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(2);

	Vec3 mins,maxs;
	int	phys=0;

	CScriptObjectVector minVec(m_pScriptSystem,true);
	CScriptObjectVector maxVec(m_pScriptSystem,true);

	pH->GetParam(1,*minVec);
	pH->GetParam(2,*maxVec);
	if(pH->GetParamCount()>2)
		pH->GetParam(3,phys);



	if (m_pEntity)
	{
		Vec3 min = minVec.Get();
		Vec3 max = maxVec.Get();

		if(phys)
		{
		IPhysicalEntity *pPhysEnt = m_pEntity->GetPhysics();
			if(m_pEntity->GetPhysics())
			{

				pe_status_pos	spos;
				pPhysEnt->GetStatus( &spos );
				pe_params_bbox	pbbox;
				pbbox.BBox[0] = spos.pos + min;
				pbbox.BBox[1] = spos.pos + max;
				pPhysEnt->SetParams( &pbbox );
			}
		}
		else
		{
			m_pEntity->SetBBox(min,max); 
		}
	}
	return pH->EndFunction();
}

int CScriptObjectEntity::GetBBox(IFunctionHandler *pH)
{
	int	phys=0;	
	pH->GetParam(1,phys);
	Vec3 mins,maxs;

	CScriptObjectVector minVec(m_pScriptSystem,false);
	CScriptObjectVector maxVec(m_pScriptSystem,false);
	_SmartScriptObject res(m_pScriptSystem,false);

	if (m_pEntity)
	{
		Vec3 min,max;
		if(phys)
		{
		IPhysicalEntity *pPhysEnt = m_pEntity->GetPhysics();
			if(m_pEntity->GetPhysics())
			{
				pe_status_pos	spos;
				pPhysEnt->GetStatus( &spos );
				min=spos.BBox[0];
				max=spos.BBox[1];
			}
		}
		else
		{
			m_pEntity->GetBBox(min,max);
		}
		minVec=min;
		maxVec=max;
		res->SetValue("min",minVec);
		res->SetValue("max",maxVec);
	}

	return pH->EndFunction(res);
}

int CScriptObjectEntity::GetLocalBBox(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	Vec3 mins,maxs;

	CScriptObjectVector minVec(m_pScriptSystem,false);
	CScriptObjectVector maxVec(m_pScriptSystem,false);
	_SmartScriptObject res(m_pScriptSystem,false);

	if (m_pEntity)
	{
		Vec3 min,max;
		m_pEntity->GetLocalBBox(min,max);
		minVec=min;
		maxVec=max;
		res->SetValue("min",minVec);
		res->SetValue("max",maxVec);
	}
	return pH->EndFunction(res);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::SetRadius(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float radius;

	pH->GetParam(1,radius);

	if (m_pEntity)
	{
		m_pEntity->SetRadius(radius);
	}

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::SetUpdateRadius(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float radius = 0;
	pH->GetParam(1,radius);
	if (m_pEntity)
	{
		m_pEntity->SetUpdateRadius(radius);
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::GetUpdateRadius(IFunctionHandler *pH)
{
	float radius = 0;
	if (m_pEntity)
	{
		radius = m_pEntity->GetUpdateRadius();
	}
	return pH->EndFunction(radius);
}


int CScriptObjectEntity::SetShaderFloat(IFunctionHandler *pH)
{
	float fFloat,fFadeValue;
  const char *sName;
	int		dwMask;

	CHECK_PARAMETERS(4);
	
  pH->GetParam(1, sName);
	pH->GetParam(2, fFloat);
	pH->GetParam(3, dwMask);
	pH->GetParam(4, fFadeValue);

	if (m_pEntity)
	{		
		if ((dwMask) && m_pEntity->GetIStatObj(0))
			m_pEntity->SetShaderFloat(sName, fFloat);
////		if ((dwMask & BITMASK_OBJECT) && m_pEntity->GetIStatObj(0))
	//		m_pEntity->GetIStatObj(0)->SetShaderFloat(sName, fFloat);

		// set it for character
		if ((dwMask == 0) && m_pEntity->GetCharInterface() && m_pEntity->GetCharInterface()->GetCharacter(0))
			m_pEntity->GetCharInterface()->GetCharacter(0)->SetShaderFloat(sName, fFloat);

  //  if ((dwMask == 0) && m_pEntity->GetCharInterface() && m_pEntity->GetCharInterface()->GetCharacter(1))
//      m_pEntity->GetCharInterface()->GetCharacter(1)->SetShaderFloat(sName, fFloat);

/*
		IEntityContainer *pCont = m_pEntity->GetContainer();
		if (pCont && (dwMask & (BITMASK_WEAPON | BITMASK_PLAYER)))
		{
			CPlayer *pPlayer; 
			if (pCont->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer))
			{			
					pPlayer->SetHeatVisionValues(dwMask,sName,fFloat,fFadeValue);						
				//&& m_pEntity->GetCharInterface() && m_pEntity->GetCharInterface()->GetCharacter(0))
				//m_pEntity->GetCharInterface()->GetCharacter(0)->SetShaderFloat(sName, fFloat,fFadeValue);				
			} //query play
		} //pcont
*/		
	}

	return pH->EndFunction();
}

int CScriptObjectEntity::SetColor(IFunctionHandler *pH)
{
  float fR, fG, fB, fA;
  const char *sName;
  int		dwMask;

  CHECK_PARAMETERS(6);

  pH->GetParam(1, sName);
  pH->GetParam(2, fR);
  pH->GetParam(3, fG);
  pH->GetParam(4, fB);
  pH->GetParam(5, fA);
  pH->GetParam(6, dwMask);

  if (m_pEntity)
  {		
    if ((dwMask) && m_pEntity->GetIStatObj(0))
      //		if ((dwMask & BITMASK_OBJECT) && m_pEntity->GetIStatObj(0))
      m_pEntity->GetIStatObj(0)->SetColor(sName, fR, fG, fB, fA);

    // set it for character
    if ((dwMask == 0) && m_pEntity->GetCharInterface() && m_pEntity->GetCharInterface()->GetCharacter(0))
      m_pEntity->GetCharInterface()->GetCharacter(0)->SetColor(fR, fG, fB, fA);
  }

  return pH->EndFunction();
}

int CScriptObjectEntity::EnableUpdate(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	bool bEnable;
	pH->GetParam(1,bEnable);
	m_pEntity->SetNeedUpdate( bEnable );

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::SetUpdateIfPotentiallyVisible(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	bool bEnable;
	pH->GetParam(1,bEnable);
  m_pEntity->SetUpdateVisLevel(bEnable ? eUT_PotVisible : eUT_Always);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::SetUpdateType(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int updateType = eUT_Always;
	if (pH->GetParam(1,updateType))
		m_pEntity->SetUpdateVisLevel( (EEntityUpdateVisLevel)updateType );
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::SetAnimationEvent(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int idSource;
	const char *sAnimation;
	pH->GetParam(1,idSource);
	pH->GetParam(2,sAnimation);
	IEntity *pSource=m_pEntitySystem->GetEntity(idSource);
	if(!pSource)
		pH->EndFunction();
	IEntityCharacter *pCharacter=pSource->GetCharInterface();
	ICryCharInstance* pCryCharacter=pCharacter->GetCharacter(0);
	pCryCharacter->AddAnimationEventSink(sAnimation,m_pEntity);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::SetAnimationKeyEvent(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(3);

	if (pH->GetParamCount()<2 || pH->GetParamCount()>3)
	{
		m_pScriptSystem->RaiseError("CScriptObjectEntity::SetAnimationKeyEvent wrong number of arguments");
		return pH->EndFunctionNull();
	};


	const char *szAnimation;
	int nFrameID;
	INT_PTR nActionType = -1;
	USER_DATA udUserData = USER_DATA(-1);
	AnimSinkEventData ased;
	
	pH->GetParam(1,szAnimation);
	pH->GetParam(2,nFrameID);
	if(pH->GetParamCount()>2)
	{
		ased.n = pH->GetParamType(3);
		//pH->GetParam(3,udUserData);
		if (pH->GetParamType(3)==svtUserData)
		{
			pH->GetParam(3,udUserData);
			ased.p = (void*) udUserData;
		}
		else if (pH->GetParamType(3)==svtNumber)
		{
			pH->GetParam(3,nActionType);
			ased.p = (void*) nActionType;
		}
		
	}
	

	IEntityCharacter *pCharacter=m_pEntity->GetCharInterface();
	ICryCharInstance* pCryCharacter=pCharacter->GetCharacter(0);
	
	if(pCryCharacter)
	{
		pCryCharacter->AddAnimationEventSink(szAnimation,m_pEntity);
		//pCryCharacter->AddAnimationEvent(szAnimation, nFrameID, (void*)udUserData);
		pCryCharacter->AddAnimationEvent(szAnimation, nFrameID, ased);
	}

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::DisableAnimationEvent(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int idSource;
	const char *sAnimation;
	pH->GetParam(1,idSource);
	pH->GetParam(2,sAnimation);
	IEntity *pSource=m_pEntitySystem->GetEntity(idSource);
	if(!pSource)
		pH->EndFunction();
	IEntityCharacter *pCharacter=pSource->GetCharInterface();
	ICryCharInstance* pCryCharacter=pCharacter->GetCharacter(0);
	if (pCryCharacter)
		pCryCharacter->RemoveAnimationEventSink (sAnimation, m_pEntity);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::SetAnimationSpeed(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fSpeed;
	pH->GetParam(1,fSpeed);
	m_pEntity->SetAnimationSpeed( fSpeed );
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::SetAnimationTime(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3)
	int nSlot = 0;
	int nLayer = 0;
	float fTime = 0;
	pH->GetParam(1,nSlot);
	pH->GetParam(2,nLayer);
	pH->GetParam(3,fTime);

	ICryCharInstance* pCryCharacter = m_pEntity->GetCharInterface()->GetCharacter(nSlot);
	if (pCryCharacter)
	{
		pCryCharacter->SetLayerTime(nLayer,fTime);
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::GetAnimationTime(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2)
	int nSlot = 0;
	int nLayer = 0;
	float fTime = 0;
	pH->GetParam(1,nSlot);
	pH->GetParam(2,nLayer);

	ICryCharInstance* pCryCharacter = m_pEntity->GetCharInterface()->GetCharacter(nSlot);
	if (pCryCharacter)
	{
		fTime = pCryCharacter->GetLayerTime(nLayer);
	}
	return pH->EndFunction(fTime);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::SelectPipe(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(2);

	int iIdentifier;
	int nID=0;
	const char *pName;
	const char *pTargetName=0;

	pH->GetParam(1,iIdentifier);
	pH->GetParam(2,pName);

	if (pH->GetParamCount() > 2)
	{
		if (pH->GetParamType(3) == svtNumber)
			pH->GetParam(3,nID);
		else
			pH->GetParam(3,pTargetName);
	}
	
	IAIObject *pTargetAI=0;
	if (nID)
	{
		IEntity *pTarget = m_pEntitySystem->GetEntity(nID);
		if (pTarget)
			pTargetAI = pTarget->GetAI();		
	}
	else if (pTargetName)
	{
		pTargetAI = m_pISystem->GetAISystem()->GetAIObjectByName(0,pTargetName);
	}

	IAIObject *pObject = m_pEntity->GetAI();

	IPipeUser *pPipeUser;
	bool res = false;
	if (pObject)
		if (pObject->CanBeConvertedTo(AIOBJECT_PIPEUSER,(void**) &pPipeUser))
		{
			if (pTargetAI)
				res = pPipeUser->SelectPipe(iIdentifier,pName,pTargetAI);
			else
				res = pPipeUser->SelectPipe(iIdentifier,pName);
		}


	if (!res)
		m_pISystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,VALIDATOR_FLAG_AI,0,"[AIWARNING] Entity %s failed to select goal pipe %s",m_pEntity->GetName(),pName);


	return pH->EndFunction();
}


int CScriptObjectEntity::InsertSubpipe(IFunctionHandler * pH)
{
	int iIdentifier;
	int nID=0;
	const char *pName;
	const char *pTargetName=0;
	pH->GetParam(1,iIdentifier);
	pH->GetParam(2,pName);

	if (pH->GetParamCount() > 2)
	{
		if (pH->GetParamType(3) == svtNumber)
			pH->GetParam(3,nID);
		else
			pH->GetParam(3,pTargetName);
	}
	
	IAIObject *pTargetAI=0;
	if (nID)
	{
		IEntity *pTarget = m_pEntitySystem->GetEntity(nID);
		if (pTarget)
			pTargetAI = pTarget->GetAI();		
	}
	else if (pTargetName)
	{
		pTargetAI = m_pISystem->GetAISystem()->GetAIObjectByName(0,pTargetName);
	}

	IAIObject *pObject = m_pEntity->GetAI();

	IPipeUser *pPipeUser;
	bool res = false;
	if (pObject)
		if (pObject->CanBeConvertedTo(AIOBJECT_PIPEUSER,(void**) &pPipeUser))
		{
			if (pTargetAI)
				res = pPipeUser->InsertSubPipe(iIdentifier,pName,pTargetAI);
			else
				res = pPipeUser->InsertSubPipe(iIdentifier,pName);
		}

	/*
	m_pISystem->GetILog()->SetFileName("AILOG.txt");
	if (res) 
		m_pISystem->GetILog()->LogToFile("[%d]	%s	SELECT_PIPE	%s",m_pISystem->GetAISystem()->GetAITickCount(),pObject->GetName(),pName);
	else
		m_pISystem->GetILog()->LogToFile("[%d]	%s	PIPE_SELECTION_FAILED	%s",m_pISystem->GetAISystem()->GetAITickCount(),pObject->GetName(),pName);
	m_pISystem->GetILog()->SetFileName("Log.txt");
	*/

	return pH->EndFunction(res);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::GotoState(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *sState;
	pH->GetParam(1,sState);

	bool res = false;
	if (sState)
	{
		if(!m_pEntity->IsStateClientside() && !GetISystem()->GetIGame()->GetModuleState( EGameServer ))
		{
			// GotoState should only be called on the client when StateClientSide is on for this entity
			CryLog("\001ERROR: ScriptObjectEntity:GotoState on the client! (EntityClass:'%s' Name:'%s', State:'%s')", 
				m_pEntity->GetEntityClassName(), m_pEntity->GetName(), sState);
		}
		res = m_pEntity->GotoState(sState);
	}
	return pH->EndFunction(res);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::IsInState(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *sState;
	pH->GetParam(1,sState);

	bool res = false;
	if (sState)
	{
		res = m_pEntity->IsInState(sState);
	}
	return pH->EndFunction(res);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::GetState(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction( m_pEntity->GetState() );
}

int CScriptObjectEntity::GetCurAnimation(IFunctionHandler *pH)
{
	int iPos;
	ICryCharInstance *pCharacter = NULL;

	CHECK_PARAMETERS(1);

	pH->GetParam(1, iPos);

	pCharacter = m_pEntity->GetCharInterface()->GetCharacter(iPos);
	if (pCharacter)
	{
		if (pCharacter->GetCurAnimation() && pCharacter->GetCurAnimation()[0] != '\0')

			return pH->EndFunction(pCharacter->GetCurAnimation());
	}
	
	return pH->EndFunction();
}


/*int CScriptObjectEntity::SetDamage(IFunctionHandler *pH)
{
	int dmg;
	CHECK_PARAMETERS(1);

	pH->GetParam(1, dmg);

	m_pEntity->SetDamage(dmg);
	
	return pH->EndFunction();
}*/


int CScriptObjectEntity::SetTimer(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int msec;
	pH->GetParam(1, msec);
	m_pEntity->SetTimer(msec);
	return pH->EndFunction();
}

int CScriptObjectEntity::KillTimer(IFunctionHandler *pH)
{
	m_pEntity->KillTimer();
	return pH->EndFunction();
}

int CScriptObjectEntity::SetScriptUpdateRate(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int msec;
	pH->GetParam(1, msec);
	m_pEntity->SetScriptUpdateRate( ((float)msec)/1000.0f );
	return pH->EndFunction();
}

int CScriptObjectEntity::RegisterState(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sState;
	pH->GetParam(1,sState);
	m_pEntity->RegisterState(sState);
	return pH->EndFunction();
}


int CScriptObjectEntity::ApplyForceToEnvironment(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	float	force;
	float	radius;
	pH->GetParam(1, radius);
	pH->GetParam(2, force);
	m_pEntity->ApplyForceToEnvironment(radius, force);

	return pH->EndFunction();
}

int CScriptObjectEntity::IsVisible(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

//  const char * pName = m_pEntity->GetName();
//  int bRend = m_pISystem->GetIRenderer()->GetFrameID();
//  int bVis  = bRend;//m_pEntity->GetDrawFrame(0);

	bool bVis = true;
  //m_pISystem->GetI3DEngine()->
		//IsSphereVisibleOnTheScreen(m_pEntity->GetPos(), m_pEntity->GetRadius()+16.f);

//  this will force entity to be rendered and to get valid entity frame id
//	m_pEntity->InitEntityRenderState();

	return pH->EndFunction( int(bVis) );
}


/*! Retrieves the position of a bone 
	@param bone Name of the helper object in the model
	@return Three component vector cotaining the position
*/
int CScriptObjectEntity::GetBonePos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char * sBoneName = "wt";
	pH->GetParam(1,sBoneName);

  IEntityCharacter *pIChar = m_pEntity->GetCharInterface();
  ICryCharInstance * cmodel = pIChar->GetCharacter(0);    

	if (!cmodel) 
	    return pH->EndFunctionNull();

	ICryBone * pBone = cmodel->GetBoneByName(sBoneName);
  if(!pBone)
  {
    m_pISystem->GetILog()->Log("ERROR: CScriptObjectWeapon::GetBonePos: Bone not found: %s", sBoneName);
    return pH->EndFunctionNull();
  }

	Vec3 vBonePos = pBone->GetBonePosition();
	Vec3 angles = m_pEntity->GetAngles();

	// transform into entity space
	//Matrix44 m;
	//m.Identity();
	//m = GetTranslationMat(m_pEntity->GetPos())*m;
	//m = GetRotationZYX44(-angles*gf_DEGTORAD)*m; //NOTE: angles in radians and negated 

	//OPTIMISED_BY_IVO  
	Matrix44 m=Matrix34::CreateRotationXYZ( Deg2Rad(angles),m_pEntity->GetPos());
	m=GetTransposed44(m); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

	SetMemberVector( SOE_MEMBER_BONE_POS,m.TransformPointOLD(vBonePos) );
	//Vec3 tmp = m.TransformPoint(vBonePos);
	return pH->EndFunction( m_memberSO[SOE_MEMBER_BONE_POS] );
}


/*! Retrieves the world direction of a bone - direction of Y axis of the bone
	@param bone Name of the helper object in the model
	@return Three component vector cotaining the direction in degrees
*/
int CScriptObjectEntity::GetBoneDir(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char * sBoneName = "wt";
	pH->GetParam(1,sBoneName);

  IEntityCharacter *pIChar = m_pEntity->GetCharInterface();
  ICryCharInstance * cmodel = pIChar->GetCharacter(0);    

	if (!cmodel) 
    return pH->EndFunctionNull();

	ICryBone * pBone = cmodel->GetBoneByName(sBoneName);
  if(!pBone)
  {
    m_pISystem->GetILog()->Log("ERROR: CScriptObjectWeapon::GetBonePos: Bone not found: %s", sBoneName);
    return pH->EndFunctionNull();
  }


	Vec3 vBoneDir = pBone->GetBoneAxis('z');
//	vBoneDir = vBoneDir*(-1.0f);
	Vec3 angles = m_pEntity->GetAngles();
	Vec3 worldAngles;
	Matrix44 m;

	// transform into entity space
	m.SetIdentity();
	//m.RotateMatrix_fix(angles);
	m=Matrix44::CreateRotationZYX(-angles*gf_DEGTORAD)*m; //NOTE: angles in radians and negated 

	// get result
	//CHANGED_BY_IVO
	//worldAngles = m.TransformVector(vBoneDir);
	worldAngles = GetTransposed44(m)*vBoneDir;
	
	
	worldAngles=ConvertVectorToCameraAngles(worldAngles);
	SetMemberVector( SOE_MEMBER_BONE_DIR,worldAngles );
	return pH->EndFunction( m_memberSO[SOE_MEMBER_BONE_DIR] );
}



int CScriptObjectEntity::GetBoneNameFromTable(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int idx;
	pH->GetParam(1,idx);

  IEntityCharacter *pIChar = m_pEntity->GetCharInterface();
	ICryCharInstance * pCharacter = pIChar->GetCharacter(0);    
	if (!pCharacter) 
    return pH->EndFunctionNull();

	const char *pName = pCharacter->GetModel()->GetBoneName(idx);
	return pH->EndFunction(pName);
}

/*!retrieve the material id(surfaceid) of the object in contact with the entity
	@return the material id of the colliding entity
*/
int CScriptObjectEntity::GetTouchedSurfaceID(IFunctionHandler *pH)
{
	IPhysicalEntity *pe;


	//pEntity=m_pPlayer->GetEntity();
	pe=m_pEntity->GetPhysics();
	if(pe)
	{
		coll_history_item hItem;
		pe_status_collisions status;
		status.pHistory = &hItem;
		status.age = .2f;
		if(pe->GetStatus(&status))// && (!status.bFlying))
		{
//int tmpIdx = hItem.idmat[1];
			return pH->EndFunction(hItem.idmat[1]);
		}
	}
	return pH->EndFunction(-1);
}

//
//!retrieves point of collision for rigid body
int CScriptObjectEntity::GetTouchedPoint(IFunctionHandler *pH)
{
	IPhysicalEntity *pe;

	//pEntity=m_pPlayer->GetEntity();
	pe=m_pEntity->GetPhysics();
	if(pe)
	{
		coll_history_item hItem;
		pe_status_collisions status;
		status.pHistory = &hItem;
		status.age = .2f;
		if(pe->GetStatus(&status))// && (!status.bFlying))
		{
		CScriptObjectVector oVec(m_pScriptSystem);
//	vec=m_pEntity->GetPos(false);
			Vec3 vec=hItem.pt;	
			oVec=vec;
			return pH->EndFunction(*oVec);
		}
	}
	return pH->EndFunction(-1);
}


// projTexName shaderName flags 
int CScriptObjectEntity::InitDynamicLight(IFunctionHandler *pH)
{
const char *sTexName=NULL;
const char *sShaderName=NULL;

  int nAsCubemap = 0;
  float fAnimSpeed = 0;
  float fCoronaSize = 0;
  int nLightStyle = 0;
	if (pH->GetParamCount()>0)
	{
		pH->GetParam(1,sTexName);
	}
	if (pH->GetParamCount()>1)
	{
		pH->GetParam(2,sShaderName);
	}
	if (pH->GetParamCount()>2)
	{
		pH->GetParam(3,nAsCubemap);
	}
	if (pH->GetParamCount()>3)
	{
		pH->GetParam(4,fAnimSpeed);
	}
  if (pH->GetParamCount()>4)
  {
    pH->GetParam(5,nLightStyle);
  }
  if (pH->GetParamCount()>5)
  {
    pH->GetParam(6,fCoronaSize);
  }


  bool bAsCubemap = (nAsCubemap >= 1);
	m_pEntity->InitLight( sTexName, sShaderName, bAsCubemap, fAnimSpeed, nLightStyle, fCoronaSize );

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::RemoveLight(IFunctionHandler *pH)
{
	if (pH->GetParamCount()<1)
		return pH->EndFunction();

	USER_DATA nLightId;
	int nCookie=0;

	if (pH->GetParamUDVal(1,nLightId,nCookie) && nLightId && (nCookie==USER_DATA_LIGHT))
	{	
		m_pISystem->GetI3DEngine()->DeleteStaticLightSource(nLightId);
	}

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::AddDynamicLight2(IFunctionHandler *pH)
{	 	
		
	//if(pH->GetParamCount()<11)
	//	return pH->EndFunction();

	CDLight DynLight;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	_SmartScriptObject  pObj(m_pScriptSystem,true);		

	if (!pH->GetParam(1,*pObj))
		return pH->EndFunction(-1);

	IScriptObject *pITable=*pObj;

	if (!pITable->BeginSetGetChain())
		return pH->EndFunction(-1);
	
	//////////////////////////////////////////////////////////////////////////
	bool bAttachToBone;
	
	if (!pITable->GetValueChain( "useModel",bAttachToBone))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> use of model not specified" );

	//////////////////////////////////////////////////////////////////////////
	const char *sTexName=NULL;
	const char *sShaderName=NULL;

	if (!pITable->GetValueChain( "ProjTexture",sTexName))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> ProjTexture not specified" );

	if (!pITable->GetValueChain( "lightShader",sShaderName))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> sShaderName not specified" );

	if (sTexName && sTexName[0])
	{
    //m_pDynLight->m_fAnimSpeed = fAnimSpeed;
    int nFlags2 = FT2_FORCECUBEMAP;
		// [marco] ??? If it is supposed to be used as cubemap or whatever
		// must be specified by the texture name or in the shader
    //if (bUseAsCube)
      //nFlags2 |= FT2_REPLICATETOALLSIDES;
    //if (fAnimSpeed)
      //nFlags2 |= FT2_CHECKFORALLSEQUENCES;
		DynLight.m_pLightImage = m_pISystem->GetIRenderer()->EF_LoadTexture(sTexName, 0, nFlags2, eTT_Cubemap);
		DynLight.m_Flags = DLF_PROJECT;
	}
	else
		DynLight.m_Flags = DLF_POINT;

	if (sShaderName && sShaderName[0])
		DynLight.m_pShader = m_pISystem->GetIRenderer()->EF_LoadShader(sShaderName, eSH_World);

	//////////////////////////////////////////////////////////////////////////	
	if (!pITable->GetValueChain("Pos",*oVec))
		m_pScriptSystem->RaiseError( "<AddDynamicLight2> Pos not specified" );
	else
		DynLight.m_Origin=oVec.Get();
	
	//////////////////////////////////////////////////////////////////////////		
	if (!pITable->GetValueChain( "orad",DynLight.m_fRadius))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> use of model not specified" );

	//////////////////////////////////////////////////////////////////////////	
	float fR,fG,fB,fA;

	if (!pITable->GetValueChain( "diffR",fR))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> diffuse not specified" );
	if (!pITable->GetValueChain( "diffG",fG))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> diffuse not specified" );
	if (!pITable->GetValueChain( "diffB",fB))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> diffuse not specified" );
	if (!pITable->GetValueChain( "diffA",fA))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> diffuse not specified" );

	DynLight.m_Color = CFColor (fR,fG,fB,fA);
  //DynLight.m_Color.Clamp();

	if (!pITable->GetValueChain( "specR",fR))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> diffuse not specified" );
	if (!pITable->GetValueChain( "specG",fG))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> diffuse not specified" );
	if (!pITable->GetValueChain( "specB",fB))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> diffuse not specified" );
	if (!pITable->GetValueChain( "specA",fA))
    m_pScriptSystem->RaiseError( "<AddDynamicLight2> diffuse not specified" );

	DynLight.m_SpecColor = CFColor (fR, fG, fB, fA);
  //DynLight.m_SpecColor.Clamp();

	//////////////////////////////////////////////////////////////////////////	
	if (!pITable->GetValueChain("Dir",*oVec))
		m_pScriptSystem->RaiseError( "<AddDynamicLight2> Dir not specified" );
	else	
		DynLight.m_ProjAngles=oVec.Get();			

	//////////////////////////////////////////////////////////////////////////
	if (!pITable->GetValueChain("projectorFov",DynLight.m_fLightFrustumAngle))
		m_pScriptSystem->RaiseError( "<AddDynamicLight2> frustum angle not specified" );
	else
		DynLight.m_fLightFrustumAngle/=2; 

	//////////////////////////////////////////////////////////////////////////
	// cast shadows 
	int	nThisAreaOnly = 0;
	if (!pITable->GetValueChain("areaonly",nThisAreaOnly))
		m_pScriptSystem->RaiseError( "<AddDynamicLight2> thisareaonly not specified" );
	else
	{
		if (nThisAreaOnly==1)
			DynLight.m_Flags |= DLF_THIS_AREA_ONLY;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// shaders stuff
	bool bDummy=false;
	if (!pITable->GetValueChain("bHeatSource",bDummy))
		m_pScriptSystem->RaiseError( "<AddDynamicLight2> bHeatSource not specified" );

	if (bDummy)
		DynLight.m_Flags|=DLF_HEATSOURCE;	

	bDummy=false;
	if (!pITable->GetValueChain("bFakeLight",bDummy))
		m_pScriptSystem->RaiseError( "<AddDynamicLight2> bFakeLight not specified" );

	pITable->EndSetGetChain();

	if (!bDummy)
		DynLight.m_Flags |= DLF_LIGHTSOURCE;

	//////////////////////////////////////////////////////////////////////////
	// more shaders stuff

	if (m_pEntity && m_pEntity->GetRndFlags()&ERF_CASTSHADOWVOLUME && !m_pEntity->GetContainer())
		DynLight.m_Flags |= DLF_CASTSHADOW_VOLUME;

	if (m_pEntity && m_pEntity->GetRndFlags()&ERF_CASTSHADOWMAPS && !m_pEntity->GetContainer())
		DynLight.m_Flags |= DLF_CASTSHADOW_MAPS;

  DynLight.m_pOwner = m_pEntity;

	if (DynLight.m_fLightFrustumAngle && DynLight.m_pLightImage!=NULL && DynLight.m_pLightImage->IsTextureLoaded())
		DynLight.m_Flags |= DLF_PROJECT;
	else
	{
		DynLight.m_pLightImage = NULL;
		DynLight.m_Flags |= DLF_POINT;
	} 	

	if(m_pEntity->GetRndFlags()&ERF_CASTSHADOWINTOLIGHTMAP)
		DynLight.m_Flags |= DLF_LM;
 
	//////////////////////////////////////////////////////////////////////////
	// finally add it to the engine

	m_pISystem->GetIRenderer()->EF_UpdateDLight(&DynLight);

	int nLightId;
	ICryCharInstance *pChar=m_pEntity->GetCharInterface()->GetCharacter(0);
	if (bAttachToBone && pChar)	
		nLightId=m_pISystem->GetI3DEngine()->AddStaticLightSource(DynLight, m_pEntity,pChar,"LightBone");	
	else
		nLightId=m_pISystem->GetI3DEngine()->AddStaticLightSource(DynLight, m_pEntity);	

	USER_DATA ud=m_pScriptSystem->CreateUserData(nLightId,USER_DATA_LIGHT);
	return pH->EndFunction(ud);	
}


// vPos, fRadius, DiffR, DiffG, DiffB, DiffA, SpecR, SpecG, SpecB, SpecA, fLifeTime
//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::AddDynamicLight(IFunctionHandler *pH)
{
	//CryError("DATA ERROR: Obsolete version of dynamiclight.lua - update and re-export the map.");
	_SmartScriptObject pOptimizationTable(m_pScriptSystem,true);
		
	if(pH->GetParamCount()<11)
		return pH->EndFunction();

	// use entity lsource
	if(!m_pEntity->GetLight())
		m_pEntity->InitLight();

	CDLight * pDynLight = m_pEntity->GetLight();
	IShader *pShader=pDynLight->m_pShader;

	{ // reset light source
    //memset(pDynLight, 0, sizeof(CDLight));
    pDynLight->m_fLightFrustumAngle = 45.0f;
    pDynLight->m_fRadius = 4.0f;
    pDynLight->m_fDirectFactor = 1.0f;
    pDynLight->m_Flags = DLF_LIGHTSOURCE;
    pDynLight->m_Orientation.m_vForward = Vec3(1,0,0);
    pDynLight->m_Orientation.m_vUp    = Vec3(0,1,0);
    pDynLight->m_Orientation.m_vRight = Vec3(0,0,1);
    pDynLight->m_NumCM = -1;
		pDynLight->m_nEntityLightId = -1;
	}

	pDynLight->m_pShader = pShader;

	CScriptObjectVector oVec(m_pScriptSystem,true);

	// get basic parameters
	if (pH->GetParam(1,*oVec))
		pDynLight->m_Origin=oVec.Get();
	pH->GetParam(2,pDynLight->m_fRadius);
	float r, g, b, a;
	pH->GetParam(3,r);
	pH->GetParam(4,g);
	pH->GetParam(5,b);
	pH->GetParam(6,a);
	pDynLight->m_Color = CFColor (r, g, b, a);
//  pDynLight->m_Color.Clamp();
	pH->GetParam(7,r);
	pH->GetParam(8,g);
	pH->GetParam(9,b);
	pH->GetParam(10,a);
	pDynLight->m_SpecColor = CFColor (r, g, b, a);
  //pDynLight->m_SpecColor.Clamp();
	pH->GetParam(11,pDynLight->m_fLifeTime);

	// get direction of projection
	if (pH->GetParamCount()>=13)
	{
		if (pH->GetParam(13,*oVec))
		{
			pDynLight->m_ProjAngles = oVec.Get();
		}
	}

	// get fov
	if (pH->GetParamCount()>=14)
	{
		float	fAngl=0;
		pH->GetParam(14,fAngl);
		pDynLight->m_fLightFrustumAngle = fAngl/2;
	}	

	// get cubemap
	if (pDynLight->m_fLightFrustumAngle && pH->GetParamCount()>=15)
	{
		int	nTid=0;
		pH->GetParam(15,nTid);
		//pDynLight->m_pLightImage = m_pISystem->GetIRenderer()->EF_GetTextureByID(nTid);
	}	

	// cast shadows
	if (pH->GetParamCount()>=16)
	{
		int	nThisAreaOnly = 0;
		pH->GetParam(16,nThisAreaOnly);
		if(nThisAreaOnly)
			pDynLight->m_Flags |= DLF_THIS_AREA_ONLY;
	}

  // used in realtime
  int	nUsedInRealTime = 1;
  if (pH->GetParamCount()>=17)
  {
    pH->GetParam(17,nUsedInRealTime);
  }

	if(m_pEntity && m_pEntity->GetRndFlags()&ERF_CASTSHADOWVOLUME && !m_pEntity->GetContainer())
		pDynLight->m_Flags |= DLF_CASTSHADOW_VOLUME;

	if (m_pEntity && m_pEntity->GetRndFlags()&ERF_CASTSHADOWMAPS && !m_pEntity->GetContainer())
		pDynLight->m_Flags |= DLF_CASTSHADOW_MAPS;

  pDynLight->m_pOwner = m_pEntity;

	if (pDynLight->m_fLightFrustumAngle && pDynLight->m_pLightImage!=NULL && pDynLight->m_pLightImage->IsTextureLoaded())
		pDynLight->m_Flags |= DLF_PROJECT;
	else
	{
		pDynLight->m_pLightImage = NULL;
		pDynLight->m_Flags |= DLF_POINT;
	}

  int bHeatSource=0;
  if (pH->GetParamCount()>=18)
    pH->GetParam(18,bHeatSource);
  if (bHeatSource)
    pDynLight->m_Flags|=DLF_HEATSOURCE;	
  else
    pDynLight->m_Flags&=~DLF_HEATSOURCE;	

  int bFake=0; 
  if (pH->GetParamCount()>=19)
    pH->GetParam(19,bFake);
  if (bFake)
    pDynLight->m_Flags|=DLF_FAKE;	
  else
    pDynLight->m_Flags&=~DLF_FAKE;	

	if(m_pEntity->GetRndFlags()&ERF_CASTSHADOWINTOLIGHTMAP)
		pDynLight->m_Flags |= DLF_LM;

	int bIgnoreTerrain=0;
	if (pH->GetParamCount()>=20)
		pH->GetParam(20,bIgnoreTerrain);
	if (bIgnoreTerrain)
		pDynLight->m_Flags|=DLF_IGNORE_TERRAIN;	
	else
		pDynLight->m_Flags&=~DLF_IGNORE_TERRAIN;	

	bool bOnlyForHighSpec = false;
	bool bSpecularOnlyForHighSpec = false;
	if (pH->GetParamCount()>=21)
	{
		if (pH->GetParam(21,pOptimizationTable))
		{
			// Read optimization table.
			pOptimizationTable->BeginSetGetChain();
			pOptimizationTable->GetValue( "bOnlyForHighSpec",bOnlyForHighSpec );
			pOptimizationTable->GetValue( "bSpecularOnlyForHighSpec",bSpecularOnlyForHighSpec );
			pOptimizationTable->EndSetGetChain();
			if (bOnlyForHighSpec)
				pDynLight->m_Flags |= DLF_ONLY_FOR_HIGHSPEC;
			else
				pDynLight->m_Flags &= ~DLF_ONLY_FOR_HIGHSPEC;
			if (bSpecularOnlyForHighSpec)
				pDynLight->m_Flags |= DLF_SPECULAR_ONLY_FOR_HIGHSPEC;
			else
				pDynLight->m_Flags &= ~DLF_SPECULAR_ONLY_FOR_HIGHSPEC;
		}
	}
	int bDot3Type=0;
	if (pH->GetParamCount()>=22)
		pH->GetParam(22,bDot3Type);
	if (bDot3Type)
		pDynLight->m_Flags|=DLF_LMDOT3;	
	else
		pDynLight->m_Flags&=~DLF_LMDOT3;	
	int bFakeRadiosity=0;
	if (pH->GetParamCount()>=23)
		pH->GetParam(23,bFakeRadiosity);
	if (bFakeRadiosity)
		pDynLight->m_Flags|=DLF_FAKE_RADIOSITY;	
	else
		pDynLight->m_Flags&=~DLF_FAKE_RADIOSITY;	

//- Get LightDir --------------------------------------------------------------------------------------------

	// get custom angles
	Vec3d vProjAnglesPlus(0,0,0);
	if (pH->GetParamCount()>=24 && pH->GetParam(24,*oVec))
		vProjAnglesPlus = oVec.Get();

	int bFixedLightDir=0;
	if (pH->GetParamCount()>=25)
		pH->GetParam(25,bFixedLightDir);

	if (bFixedLightDir)
	{
		Matrix33 mat=Matrix33::CreateRotationXYZ(Ang3(DEG2RAD(pDynLight->m_ProjAngles.x),DEG2RAD(pDynLight->m_ProjAngles.y),DEG2RAD(pDynLight->m_ProjAngles.z)));
		Matrix33 matRotate = Matrix33::CreateRotationXYZ(Ang3(DEG2RAD(vProjAnglesPlus.x),DEG2RAD(vProjAnglesPlus.y),DEG2RAD(vProjAnglesPlus.z)));
		mat = mat * matRotate;
		Ang3 angles;
		angles = angles.GetAnglesXYZ(mat);
		pDynLight->m_ProjAngles.x = RAD2DEG(angles.x);
		pDynLight->m_ProjAngles.y = RAD2DEG(angles.y);
		pDynLight->m_ProjAngles.z = RAD2DEG(angles.z);
	}
	else
		pDynLight->m_ProjAngles += vProjAnglesPlus;
//---------------------------------------------------------------------------------------------
	int bOcclType=0;
	if (pH->GetParamCount()>=26)
		pH->GetParam(26,bOcclType);
	if (bOcclType)
		pDynLight->m_Flags|=DLF_LMOCCL;	
	else
		pDynLight->m_Flags&=~DLF_LMOCCL;	

  pDynLight->MakeBaseParams();

  if(nUsedInRealTime)
  {
    pDynLight->m_Flags &= ~DLF_TEMP;
	  m_pISystem->GetIRenderer()->EF_UpdateDLight(pDynLight);
	  m_pISystem->GetI3DEngine()->AddDynamicLightSource(*pDynLight, m_pEntity);
  }
  else
    pDynLight->m_Flags |= DLF_TEMP; // light should be totaly ignored for realtime lighting

	m_pEntity->InvalidateBBox();

	return pH->EndFunction();
}


//
//updates huming rockets
int CScriptObjectEntity::DoHam(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	pe_params_particle rpp;
	
	Vec3 target;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*oVec);
	target=oVec.Get();

	IPhysicalEntity *pe;
	pe=m_pEntity->GetPhysics();
	if(pe)
	{
		pe->GetParams(&rpp);

		Vec3 pos = m_pEntity->GetPos();
		target = target - pos;
		Vec3 dir = rpp.heading;

		Vec3 diff = target-dir;
		dir+=diff*m_pISystem->GetITimer()->GetFrameTime()*0.5f;

//		float	dist = target.Length();
	//	target.Normalize();
	//	Vec3 delta = target - (Vec3)rpp.heading;

		
		rpp.heading = dir;
		rpp.heading.normalize();
		
		pe->SetParams(&rpp);
	}
	return pH->EndFunction(-1);
}



//
//resets huming rockets
int CScriptObjectEntity::ResetHam(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	//m_ControlTime = 0;
	//m_Control = Vec3(0,0,0);
	//m_ControlTimeLimit = 0.05f;
	return pH->EndFunction(-1);
}

int CScriptObjectEntity::LoadBoat(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	const char *sFileName;
	float fMass;
	int nSurfaceID=0;

	pH->GetParam(1,sFileName);
	pH->GetParam(2,fMass);
	pH->GetParam(3,nSurfaceID);

	if (m_pEntity)
	{
/*
		m_pEntity->LoadObject(0,sFileName,0,"boat_hull");
		m_pEntity->CreateRigidBody(PE_RIGID,0,fMass,nSurfaceID,NULL,0);
		return pH->EndFunction(1);
/*/
//*
		if( m_pEntity->LoadBoat(sFileName, fMass, nSurfaceID) )
			return pH->EndFunction(1);
/*/
		int res;

		res = m_pEntity->LoadObject(0,sFileName,0,"boat");
		if(res)
		{
			if(m_pEntity->LoadObject(1,sFileName,0,"boat_hull"))
			{
				m_pEntity->CreateRigidBody(PE_RIGID,0,fMass,nSurfaceID,NULL,1);
				m_pEntity->DrawObject(1, ETY_DRAW_NONE);
			}
		}

		return pH->EndFunction(res);
//*/
  }
//	m_pISystem->GetILog()->LogToFile("[FATAL ERROR] Script table %s not found. Probably script was not loaded because of an error.",sClassname.c_str());
//	CryWarning("ERROR - Can not load boat %s ", sFileName);
	return pH->EndFunction(0);
}


// Enable/ disable various entity features
int CScriptObjectEntity::EnableProp(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(2);
	int propertyID;
	bool enable;

	pH->GetParam(1,propertyID);
	pH->GetParam(2,enable);

	switch (propertyID)
	{
	case ENTITYPROP_CASTSHADOWS:
		m_pEntity->SetRndFlags(ERF_CASTSHADOWVOLUME|ERF_SELFSHADOW|ERF_CASTSHADOWMAPS|ERF_RECVSHADOWMAPS,enable);
		break;
	case ENTITYPROP_DONOTCHECKVIS:
		m_pEntity->SetRndFlags(ERF_DONOTCHECKVIS,enable);
		break;
	}

	return pH->EndFunction();
}
/*
//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::GetBuildingId(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	int nBuildingId = -1;
	int nSectorId = -1;
	Vec3 pos = m_pEntity->GetPos();
	IIndoorBase *pBase=m_pISystem->GetI3DEngine()->GetBuildingManager();
	if (!pBase || !pBase->CheckInside(pos,nBuildingId,nSectorId))
	{
		nBuildingId = nSectorId = -1;
	}
	return pH->EndFunction(nBuildingId);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::GetSectorId(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	int nBuildingId = -1;
	int nSectorId = -1;
	Vec3 pos = m_pEntity->GetPos();
	IIndoorBase *pBase=m_pISystem->GetI3DEngine()->GetBuildingManager();
	if (!pBase || !pBase->CheckInside(pos,nBuildingId,nSectorId))
	{
		nBuildingId = nSectorId = -1;
	}
	return pH->EndFunction(nSectorId);
}
*/

//
//----------------------------------------------------------------------------------------------------
int CScriptObjectEntity::Damage(IFunctionHandler *pH)
{
	_SmartScriptObject pObj(m_pScriptSystem,true);
	if(pH->GetParam(1,pObj))
	{
		m_pEntity->OnDamage(pObj);
	}
	return pH->EndFunction();
}

/*
//
//----------------------------------------------------------------------------------------------------
int CScriptObjectEntity::UpdateInSector(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	int curBuildingId,curSectorId;
	int nBuildingId,nSectorId;

	pH->GetParam(1, curBuildingId);
	pH->GetParam(2, curSectorId);

	Vec3 pos = m_pEntity->GetPos();
	if (m_pISystem->GetI3DEngine()->GetBuildingManager()->CheckInside(pos,nBuildingId,nSectorId))
	{
		if(curBuildingId == -1)	// it's first time - add area
		{
			m_pGame->m_XAreaMgr.AddArea( nBuildingId, nSectorId, m_pEntity->GetId() );
		}
		if( (curBuildingId != nBuildingId) || (curSectorId != nSectorId) ) // area already added - now moved to other sector
		{
			CXArea *curArea = m_pGame->m_XAreaMgr.GetArea( curBuildingId, curSectorId, m_pEntity->GetId() );
			if(curArea)
			{
				curArea->SetBuilding( nBuildingId);
				curArea->SetSector( nSectorId );
			}
		}
	}
	else
	{
		if(curBuildingId != -1)	// was in some sector - now out. remove from areaMgr
		{
			CXArea *curArea = m_pGame->m_XAreaMgr.GetArea( curBuildingId, curSectorId, m_pEntity->GetId() );
			if(curArea)
				m_pGame->m_XAreaMgr.DeleteArea( curArea );
		}
		nBuildingId = nSectorId = -1;
	}

	CScriptObjectVector oVec(m_pScriptSystem);
	oVec.Set(Vec3((float)nBuildingId, (float)nSectorId, 0.0f));
	return pH->EndFunction(*oVec);

//	return pH->EndFunction();
}
*/
//
//----------------------------------------------------------------------------------------------------

int CScriptObjectEntity::ChangeAIParameter(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(2);

	int nParameter;
	float fValue;
	pH->GetParam(1,nParameter);
	pH->GetParam(2,fValue);

	AgentParameters ap;
	IAIObject *pObject = m_pEntity->GetAI();
	if (pObject)
	{
		IPuppet *pPuppet = 0;
		if (pObject->CanBeConvertedTo(AIOBJECT_PUPPET, (void**) &pPuppet))
		{
			ap = pPuppet->GetPuppetParameters();
			switch (nParameter)
			{
				case AIPARAM_SIGHTRANGE:
					ap.m_fSightRange = fValue;
					break;
				case AIPARAM_ATTACKRANGE:
					ap.m_fAttackRange = fValue;
					break;
				case AIPARAM_ACCURACY:
					ap.m_fAccuracy = fValue;
					break;
				case AIPARAM_AGGRESION:
					ap.m_fAggression = 1.0f - fValue;
					break;
				case AIPARAM_GROUPID:
					ap.m_nGroup = (int) fValue;
					break;
				case AIPARAM_SOUNDRANGE:
					ap.m_fSoundRange = fValue;
					break;
				case AIPARAM_FOV:
					ap.m_fHorizontalFov = fValue;
					break;
				case AIPARAM_COMMRANGE:
					ap.m_fCommRange = fValue;
					break;
				case AIPARAM_RESPONSIVENESS:
					ap.m_fResponsiveness = fValue;
					break;
				case AIPARAM_SPECIES:
					ap.m_nSpecies = (int) fValue;
					break;

					// more to come as needed
			}
			pPuppet->SetPuppetParameters(ap);
		}
		else if( pObject->CanBeConvertedTo(AIOBJECT_VEHICLE, (void**) &pPuppet))
		{
			switch (nParameter)
			{
				case AIPARAM_FWDSPEED:
					IVehicleProxy *proxy=NULL;
					if(pObject->GetProxy()->QueryProxy(AIPROXY_VEHICLE, (void**)&proxy))
					{
						proxy->SetSpeeds( fValue, -1 );
					}
					return pH->EndFunction();
					// more to come as needed
			}	
		}
	}
	return pH->EndFunction();
}

//
//----------------------------------------------------------------------------------------------------
/*
int CScriptObjectEntity::TranslatePartIdToDeadBody(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int partid = -1;
	pH->GetParam(1,partid);

	IEntityCharacter *pIChar = m_pEntity->GetCharInterface();
	if (pIChar) {
		ICryCharInstance * cmodel = pIChar->GetCharacter(0);
		if (cmodel)
			pH->EndFunction(cmodel->TranslatePartIdToDeadBody(partid));
	}

	return pH->EndFunction(-1);
}
*/

//
//----------------------------------------------------------------------------------------------------
int CScriptObjectEntity::SetAICustomFloat(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float radius = 0.0f;
	pH->GetParam(1,radius);

	if( !m_pEntity->GetAI() )
		return pH->EndFunction(-1);

	m_pEntity->GetAI()->SetEyeHeight( radius );

	return pH->EndFunction(-1);
}

//
//----------------------------------------------------------------------------------------------------n
//
//----------------------------------------------------------------------------------------------------
int CScriptObjectEntity::ActivatePhysics(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	if (!m_pEntity)
		return pH->EndFunction(0);

	int active;
	pH->GetParam(1, active);
	m_pEntity->ActivatePhysics( active!=0 );
	return pH->EndFunction(0);
}


//
//----------------------------------------------------------------------------------------------------

int CScriptObjectEntity::DeleteParticleEmitter(IFunctionHandler *pH)
{
	int nSlotId = 0;
	if(!pH->GetParam(1,nSlotId))
	{
		nSlotId = 0;
	}
	m_pEntity->DeleteParticleEmitter(nSlotId);
	return pH->EndFunction();
}

int CScriptObjectEntity::CreateParticleEmitter(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	_SmartScriptObject  pObj(m_pScriptSystem,true);
	_SmartScriptObject  pChildObj(m_pScriptSystem,true);
	static ParticleParams sParam;
	float fTimeDelay=0;

	if(!pH->GetParam(1,*pObj))
	{
		m_pScriptSystem->RaiseError( "<CreateParticleEmitter> parameter 1 not specified or nil(perticle struct)" );
		return pH->EndFunction();
	}
	if(!pH->GetParam(2,fTimeDelay))
		m_pScriptSystem->RaiseError( "<CreateParticleEmitter> parameter 2 not specified or nil(fTimeDelay)" );

	ReadParticleTable(*pObj, sParam);
	sParam.vPosition = Vec3(0,0,0);
	sParam.vDirection = Vec3(0,0,0);
	pObj->BeginSetGetChain();
	if ((sParam).fChildSpawnPeriod && pObj->GetValueChain("ChildProcess", *pChildObj))
	{
		ParticleParams sChildParams;
		ReadParticleTable(*pChildObj, sChildParams);
		sParam.pChild = &sChildParams;
	}
	else
		sParam.pChild = NULL;

	//STATIC OBJECT BASED PARTICLES
	sParam.pStatObj = NULL;
	INT_PTR nValue=0;
	int nCookie=0;
	if(pObj->GetUDValueChain("geometry",nValue,nCookie) && (nCookie==USER_DATA_OBJECT))
		sParam.pStatObj=(IStatObj *)nValue;
	
	sParam.vPosition = Vec3(0,0,0);

	// Default direction is Positive Y.
	m_pEntity->CreateEntityParticleEmitter(0,sParam,fTimeDelay,Vec3(0,0,0),Vec3(0,1,0));

	pObj->EndSetGetChain();

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::CreateParticleEmitterEffect(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(6);
	float fTimeDelay = 0;
	Vec3 vOffset;
	Vec3 vDir;
	const char *sEffectName = 0;
	int nSlotId = 0;
	float fScale = 1.0f;
	CScriptObjectVector oVec(m_pScriptSystem,true);

	if(!pH->GetParam(1,nSlotId))
	{
		m_pScriptSystem->RaiseError( "<CreateParticleEmitterEffect> parameter 1 not specified or nil(Slot Id)" );
		return pH->EndFunction();
	}
	if(!pH->GetParam(2,sEffectName))
	{
		m_pScriptSystem->RaiseError( "<CreateParticleEmitterEffect> parameter 2 not specified or nil(Effect Name)" );
		return pH->EndFunction();
	}
	if(!pH->GetParam(3,fTimeDelay))
		m_pScriptSystem->RaiseError( "<CreateParticleEmitterEffect> parameter 3 not specified or nil(fTimeDelay)" );
	
	if(!pH->GetParam(4,oVec))
		m_pScriptSystem->RaiseError( "<CreateParticleEmitterEffect> parameter 4 not specified or nil(Offset)" );
	vOffset = oVec.Get();

	if(!pH->GetParam(5,oVec))
		m_pScriptSystem->RaiseError( "<CreateParticleEmitterEffect> parameter 5 not specified or nil(Direction)" );
	vDir = oVec.Get();

	if(!pH->GetParam(6,fScale))
		m_pScriptSystem->RaiseError( "<CreateParticleEmitterEffect> parameter 6 not specified or nil(Scale)" );

	if (nSlotId < 0)
		nSlotId = 0;
	if (nSlotId >= MAX_PARTICLES_SLOTS)
	{
		nSlotId = MAX_PARTICLES_SLOTS-1;
	}

	if (sEffectName)
	{
		IParticleEffect *pEffect = m_pISystem->GetI3DEngine()->FindParticleEffect( sEffectName );
		if (pEffect)
		{
			ParticleParams params;
			m_pEntity->CreateEntityParticleEmitter(nSlotId,params,fTimeDelay,vOffset,vDir,pEffect,fScale );
		}
	}

	return pH->EndFunction();
}


// copy of function from ScriptObjectParticle
bool CScriptObjectEntity::ReadParticleTable(IScriptObject *pITable, ParticleParams &sParamOut)
{
	CScriptObjectColor  oCol(m_pScriptSystem,true);
	Vec3 v3Pos,v3Offset(0,0,0);
	CScriptObjectVector oVec(m_pScriptSystem,true);

	//default params
	float   focus = 0;
	Vec3  vStartColor(1,1,1);
	Vec3  vEndColor(1,1,1);
	Vec3  vRotation(0,0,0);
	Vec3  vGravity(0,0,0);
	float speed    = 0;
	int	  count    = 1;
	float size       = 0.05f;
	float size_speed = 0;
	float gravity    = 0;	
	float lifetime   = 0;
	float fadeintime = 0;
	INT_PTR tid      = 0;
	int	  frames   = 0;
	int draw_last  = 0;
	int  blendType = ParticleBlendType_AlphaBased;
	int	 color_based_blending = 0;	
	int iParticleType = 0;
	float fTailLength = 0.0f;
	int bRealPhys = 0;
	float fDirVecScale = 1.0f;
	int nEntityID=0;
	IShader * pShader = 0;
	Vec3d vSpaceLoopBoxSize(0,0,0);
	int nBindToCamera=0;
	int nNoIndoor=0;

	if(!pITable->BeginSetGetChain())
		return false;
	//FOCUS////////////////////////////////////
	if(!pITable->GetValueChain( "focus",focus ))            
		m_pScriptSystem->RaiseError( "<CreateParticleEmitter> focus field not specified" );
	//START COLOR////////////////////////////////
	if (pITable->GetValueChain( "start_color",oCol ))
		vStartColor = oCol.Get();
	//END COLOR////////////////////////////////
	if (pITable->GetValueChain( "end_color",oCol ))
		vEndColor = oCol.Get();
	//SPEED////////////////////////////////
	if(!pITable->GetValueChain( "speed",speed ))            
		m_pScriptSystem->RaiseError( "<CreateParticleEmitter> speed field not specified" );
	//ROTATION////////////////////////////////
	if (pITable->GetValueChain( "rotation",oVec ))
		vRotation = oVec.Get();
	//COUNT////////////////////////////////
	if(!pITable->GetValueChain( "count",count ))            
		m_pScriptSystem->RaiseError( "<CreateParticleEmitter> count field not specified" );
	//SIZE////////////////////////////////
	if(!pITable->GetValueChain( "size" ,size  ))            
		m_pScriptSystem->RaiseError( "<CreateParticleEmitter> size field not specified"  );
	//SIZE SPEED////////////////////////////////
	if(!pITable->GetValueChain( "size_speed",size_speed ))  
		size_speed=0;
	//GRAVITY////////////////////////////////
	if (pITable->GetValueChain( "gravity",oVec ))        
		vGravity = oVec.Get();
	//LIFETIME////////////////////////////////
	if(!pITable->GetValueChain( "lifetime",lifetime ))      
		m_pScriptSystem->RaiseError( "<CreateParticleEmitter> lifetime field not specified" );
	//FADEINTIME////////////////////////////////
	if(!pITable->GetValueChain( "fadeintime",fadeintime ))      
		fadeintime=0;//m_pScriptSystem->RaiseError( "<CreateParticleEmitter> fadeintime field not specified" );
	//FRAMES////////////////////////////////
	if(!pITable->GetValueChain( "frames",frames ))          
		frames=0;
	//TID////////////////////////////////
	int nCookie=0;
	if(!pITable->GetUDValueChain( "tid",tid,nCookie)) 
		tid=0;
	//PARTICLE TYPE////////////////////////////////
	if(!pITable->GetValueChain( "particle_type", iParticleType ))
		iParticleType = PART_FLAG_BILLBOARD;
	//TAIL LENGHT////////////////////////////////
	if(!pITable->GetValueChain( "tail_length", fTailLength))
		fTailLength = 0.0f;
	//PHYSICS////////////////////////////////
	if(!pITable->GetValueChain( "physics", bRealPhys ))
		bRealPhys = 0;
	//BIND EMITTER TO CAMERA/////////////////////
	if(!pITable->GetValueChain( "BindToCamera", nBindToCamera ))
		nBindToCamera = 0;
	//KILL PARTICLES IN VISAREAS/////////////////////
	if(!pITable->GetValueChain( "NoIndoor", nNoIndoor ))
		nNoIndoor = 0;

	//DRAW LAST////////////////////////////////
	if(!pITable->GetValueChain( "draw_last",draw_last ))
		draw_last=0;
	//COLOR BASED BLENDING (legacy)////////////////////////////////
	if (pITable->GetValueChain( "color_based_blending",color_based_blending ))
	{
		// This for backward compatability.
		if (color_based_blending == 3)
			blendType = ParticleBlendType_ColorBased;
	}
	//BLEND TYPE////////////////////////////////
	// Read particles blending type.
	pITable->GetValueChain( "blend_type",blendType );
	//BOUNCENES/////////////////////////////////
	float fBouncenes;
	if(!pITable->GetValueChain( "bouncyness", fBouncenes))
		fBouncenes = 0.5f;
	//INIT ANGLE/////////////////////////////////
	Vec3  vAngles(0,0,0);
	if (pITable->GetValue( "init_angles",oVec ))        
		vAngles = oVec.Get();

	pITable->GetValueChain( "dir_vec_scale", fDirVecScale );

	// turbulence
	float fTurbulenceSize=0;
	pITable->GetValueChain( "turbulence_size", fTurbulenceSize);
	float fTurbulenceSpeed=0;
	pITable->GetValueChain( "turbulence_speed", fTurbulenceSpeed);
	int nLinearSizeSpeed=0;
	pITable->GetValueChain( "bLinearSizeSpeed", nLinearSizeSpeed);

	float fChildSpawnPeriod=0;
	pITable->GetValueChain( "ChildSpawnPeriod", fChildSpawnPeriod);

	sParamOut.fPosRandomOffset=0;
	pITable->GetValueChain( "fPosRandomOffset", sParamOut.fPosRandomOffset);

	char szShaderName[256]="";
	char * pShaderName = szShaderName;
	if(pITable->GetValue("ShaderName", (const char* &)pShaderName))
		pShader = m_pISystem->GetIRenderer()->EF_LoadShader(pShaderName, eSH_World);

	// SpaceLoopBoxSize ////////////////////////////////
	if (pITable->GetValueChain( "SpaceLoopBoxSize",oVec ))        
		vSpaceLoopBoxSize = oVec.Get();

	// after this line GetValueChain will crash
	pITable->EndSetGetChain();

	//////////////////////////////////////////////////////////////////////////////////////
	sParamOut.fFocus = focus;
	sParamOut.vColorStart = vStartColor;
	sParamOut.vColorEnd = vEndColor;
	sParamOut.fSpeed = speed;
	sParamOut.nCount = count;
	sParamOut.fSize = size;
	sParamOut.fSizeSpeed = size_speed;
	sParamOut.vGravity = vGravity;
	sParamOut.fLifeTime = lifetime;
	sParamOut.fFadeInTime = fadeintime;

	sParamOut.nTexId = tid;
	sParamOut.nTexAnimFramesCount = frames;
	sParamOut.eBlendType = (ParticleBlendType)blendType;

	sParamOut.nParticleFlags = iParticleType;

	if(nLinearSizeSpeed)
		sParamOut.nParticleFlags |= PART_FLAG_SIZE_LINEAR;

	sParamOut.bRealPhysics = bRealPhys != 0;
	sParamOut.pChild = NULL;
	sParamOut.fChildSpawnPeriod = fChildSpawnPeriod;
	sParamOut.fTailLenght = fTailLength;

	sParamOut.nDrawLast = draw_last;

	sParamOut.vRotation = vRotation;

	sParamOut.fBouncenes = fBouncenes;

	sParamOut.vInitAngles = vAngles;

	// Scale the direction vsector based on fDirVecScale
	sParamOut.vDirection *= fDirVecScale;

	sParamOut.fTurbulenceSize=fTurbulenceSize;
	sParamOut.fTurbulenceSpeed=fTurbulenceSpeed;  
	sParamOut.pShader = pShader;

	sParamOut.vSpaceLoopBoxSize = vSpaceLoopBoxSize;
	if(vSpaceLoopBoxSize.x && vSpaceLoopBoxSize.y && vSpaceLoopBoxSize.z)
		sParamOut.nParticleFlags |= PART_FLAG_SPACELOOP;

	if(nBindToCamera)
		sParamOut.nParticleFlags |= PART_FLAG_BIND_EMITTER_TO_CAMERA;

	if(nNoIndoor)
		sParamOut.nParticleFlags |= PART_FLAG_NO_INDOOR;

	return true;
}
 
int CScriptObjectEntity::GetEntitiesInContact(IFunctionHandler *pH)
{
	Vec3 mins, maxs;
	m_pEntity->GetBBox(mins, maxs);
	vectorf minbox, maxbox;
	minbox = vectorf(mins);
	maxbox = vectorf(maxs);
	IPhysicalWorld *pWorld=m_pISystem->GetIPhysicalWorld();
	IPhysicalEntity **ppColliders;
	int cnt = 0,valid=0;

	if (cnt = pWorld->GetEntitiesInBox(minbox, maxbox, ppColliders,ent_living|ent_rigid|ent_sleeping_rigid|ent_static))
	{
		// execute on collide for all of the entities
		_SmartScriptObject pObj(m_pScriptSystem);
		for (int i = 0; i < cnt; i++)
		{
			
			IEntity *pEntity =(IEntity *) ppColliders[i]->GetForeignData();
			
			if (pEntity) 
			{
				if (pEntity->IsGarbage())
					continue;

				if (pEntity->GetId() == m_pEntity->GetId()) 
					continue;

				if (pEntity->IsStatic())
					continue;
				valid++;
				pObj->SetAt(pEntity->GetId(),pEntity->GetScriptObject());
			}
		}
		if(valid)
		{
			return pH->EndFunction(pObj);
		}
	}

	return pH->EndFunctionNull();
}


int CScriptObjectEntity::SetDefaultIdleAnimations(IFunctionHandler *pH)
{
//CHECK_PARAMETERS(1);
	assert(pH->GetParamCount() == 1 || pH->GetParamCount() == 2);

	const char *animname=NULL;
	int pos;
	pH->GetParam(1,pos);
	if (pH->GetParamCount() > 1 )
		pH->GetParam(2,animname);

	m_pEntity->SetDefaultIdleAnimation( pos, animname );

	return pH->EndFunctionNull();

}

int CScriptObjectEntity::GetAnimationLength(IFunctionHandler *pH)
{
CHECK_PARAMETERS(1);
	const char *aniName;
	pH->GetParam(1, aniName);

	return pH->EndFunction(m_pEntity->GetAnimationLength(aniName));

/*
	int iAnimationPos;
	const char *pszAnimName;
	float fSecLen = 0.0f;

	assert(m_pEntity->GetCharInterface());

	CHECK_PARAMETERS(2);

	pH->GetParam(1, iAnimationPos);
	pH->GetParam(2, pszAnimName);
	
	ICryCharInstance* pCharacter = m_pEntity->GetCharInterface()->GetCharacter(iAnimationPos);
	if (!pCharacter)
		return pH->EndFunction(fSecLen);

	IAnimationSet* pAnimations = pCharacter->GetModel()->GetAnimationSet();
	assert (pAnimations);
	int nAnimationId = pAnimations->Find (pszAnimName);
	if (nAnimationId >= 0)
		return pH->EndFunction(pAnimations->GetLength(nAnimationId));

		return pH->EndFunction(fSecLen);
*/
}

/*! disables and releases all lipsync-functions
*/
int CScriptObjectEntity::ReleaseLipSync(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	if (m_pEntity && m_pEntity->GetCharInterface())
		m_pEntity->GetCharInterface()->ReleaseLipSyncInterface();
	return pH->EndFunction(1);
}

/*! loads (enables) expressions for a character
	@param pszFilename script-filename ("" to disable)
*/
int CScriptObjectEntity::DoRandomExpressions(IFunctionHandler *pH)
{
	if (pH->GetParamCount()<1)
		CHECK_PARAMETERS(1);
	const char *pszFilename;
	bool bRaiseError=true;
	pH->GetParam(1, pszFilename);
	if (pH->GetParamCount()>=2)
		pH->GetParam(2, bRaiseError);
	if(m_pEntity && m_pEntity->GetCharInterface())
	{
		ILipSync *pLipSync=m_pEntity->GetCharInterface()->GetLipSyncInterface();
		if (!pLipSync)
		{
			m_pISystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,
				0,"Could not create lip-sync interface ! Does this entity %s have a character ?",m_pEntity->GetName() );
			return pH->EndFunctionNull();
		}
		if (!pLipSync->LoadRandomExpressions(pszFilename, bRaiseError))
		{
			m_pISystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,
				0,"CLipSync::LoadDialog failed for Entity %s",m_pEntity->GetName() );
			return pH->EndFunctionNull();
		}
	}
	return pH->EndFunction((int)1);
}

/*! plays an expression for a character
	@param pszMorphTarget name of morph-target
	@param fAmplitude amplitude-scale
	@param fBlendIn blend-in time in seconds
	@param fLength hold time in seconds
	@param fBlendOut blend-out time in seconds
*/
int CScriptObjectEntity::DoExpression(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(5);
	const char *pszMorphTarget;
	CryCharMorphParams MorphParams;
	pH->GetParam(1, pszMorphTarget);
	pH->GetParam(2, MorphParams.fAmplitude);
	pH->GetParam(3, MorphParams.fBlendIn);
	pH->GetParam(4, MorphParams.fLength);
	pH->GetParam(5, MorphParams.fBlendOut);
	MorphParams.fStartTime=0.0f;
	if(m_pEntity && m_pEntity->GetCharInterface())
	{
		ILipSync *pLipSync=m_pEntity->GetCharInterface()->GetLipSyncInterface();
		if (!pLipSync)
		{
			m_pISystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,
				0,"Could not create lip-sync interface ! Does this entity %s have a character ?",m_pEntity->GetName() );
			return pH->EndFunctionNull();
		}
		if (!pLipSync->DoExpression(pszMorphTarget, MorphParams))
		{
			m_pISystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,
				0,"CLipSync::DoExpression failed, Entity: %s",m_pEntity->GetName() );
			return pH->EndFunctionNull();
		}
	}
	return pH->EndFunction((int)1);
}

/*! says dialog (lip-synced); must be a character
	@param pszFilename name of dialog-filename (*.lsf is expected in the same folder)
	@param nVol Volume of sound
	@param fMin Min sound-distance
	@param fMax Max sound-distance
	@param nFlags Additional sound-loading-flags
*/
int CScriptObjectEntity::SayDialog(IFunctionHandler *pH)
{
	if (pH->GetParamCount()<4)
		CHECK_PARAMETERS(4);
	const char *pszFilename;
	float fMin, fMax;
	int nVol, nFlags=0;
	float fClipDistance=500.0f;
	pH->GetParam(1, pszFilename);
	pH->GetParam(2, nVol);
	pH->GetParam(3, fMin);
	pH->GetParam(4, fMax);
	if (pH->GetParamCount()>=5)
		pH->GetParam(5, nFlags);
	//if (pH->GetParamCount()>=6)
	//	pH->GetParam(6, fClipDistance);

	IScriptObject *pAITable=NULL;
	if (pH->GetParamCount()>=6)
	{
		pAITable=m_pScriptSystem->CreateEmptyObject();
		if (!pH->GetParam(6, pAITable))
		{
			pAITable->Release();
			pAITable=NULL;
		}
	}

	if (fClipDistance>1000.0f)
		fClipDistance=1000.0f;
	if(m_pEntity && m_pEntity->GetCharInterface())
	{
		ILipSync *pLipSync=m_pEntity->GetCharInterface()->GetLipSyncInterface();
		if (!pLipSync)
		{
			m_pISystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,
				0,"Could not create lip-sync interface ! Does this entity %s have a character ?",m_pEntity->GetName() );
			return pH->EndFunctionNull();
		}
		if (!pLipSync->LoadDialog(pszFilename, nVol, fMin, fMax, fClipDistance, nFlags,pAITable))
		{
			m_pISystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,
				0,"CLipSync::LoadDialog failed for Entity %s",m_pEntity->GetName() );
			return pH->EndFunctionNull();
		}
		/*
		if (!pLipSync->PlayDialog())
		{
			m_pISystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,
				0,"CLipSync::PlayDialog failed for Entity %s",m_pEntity->GetName() );
			return pH->EndFunctionNull();
		}
		*/
	}
	return pH->EndFunction(1);
}

/*! stops dialog (lip-synced); must be a character
*/
int CScriptObjectEntity::StopDialog(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	if(m_pEntity && m_pEntity->GetCharInterface())
	{
		ILipSync *pLipSync=m_pEntity->GetCharInterface()->GetLipSyncInterface();
		if (!pLipSync)
		{
			m_pISystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,
				0,"Could not create lip-sync interface ! Does this entity %s have a character ?",m_pEntity->GetName() );
			return pH->EndFunctionNull();
		}
		if (!pLipSync->StopDialog())
		{
			m_pISystem->GetILog()->Log("\005CLipSync::StopDialog failed for Entity %s",m_pEntity->GetName());
			return pH->EndFunctionNull();
		}
	}
	return pH->EndFunction(1);
}

/*! Set the material of the entity
	@param materialName Name of material.
*/
int CScriptObjectEntity::SetMaterial(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sMaterialName = 0;
	if (pH->GetParam(1,sMaterialName))
	{
		IMatInfo *pMaterial = m_pISystem->GetI3DEngine()->FindMaterial( sMaterialName );
		m_pEntity->SetMaterial( pMaterial );
	}
	return pH->EndFunction();
}

/*! Get the name of custom entity material.
*/
int CScriptObjectEntity::GetMaterial(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	if (m_pEntity && m_pEntity->GetMaterial())
	{
		return pH->EndFunction( m_pEntity->GetMaterial()->GetName() );
	}
	return pH->EndFunctionNull();
}


/*! Attach character hands with IK to some position
	@param vPos IK target position (world).
*/
int CScriptObjectEntity::SetHandsIKTarget(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	Vec3 vec;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*oVec);
	vec=oVec.Get();
	m_pEntity->SetHandsIKTarget(&vec);
	return pH->EndFunction();
}


/*! Get the velosity of the entity
	@return Three component vector containing the velosity position
*/
int CScriptObjectEntity::GetVelocity(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	IPhysicalEntity *phys = m_pEntity->GetPhysics();
	if(phys)
	{
		pe_status_dynamics	dyn;
		phys->GetStatus(&dyn);
		SetMemberVector( SOE_MEMBER_OBJ_VEL,dyn.v );
		return pH->EndFunction(m_memberSO[SOE_MEMBER_OBJ_VEL]);
	}
	return pH->EndFunctionNull();
}


int CScriptObjectEntity::ApplyImpulseToEnvironment(IFunctionHandler * pH)
{
	_SmartScriptObject pObj(m_pScriptSystem,true);
	_SmartScriptObject pTempObj(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);

	Vec3 pos;
	float rmin, rmax, impulsive_pressure;
	float rmin_occ = 0.1f;
	int nOccRes=0,nGrow=0;

	pH->GetParam(1,*pObj);

	pObj->GetValue("pos",*oVec );
	pos=oVec.Get();
	pObj->GetValue("rmin",rmin );
	pObj->GetValue("rmax",rmax );
	pObj->GetValue("impulsive_pressure",impulsive_pressure );
	pObj->GetValue("rmin_occlusion", rmin_occ);
	pObj->GetValue("occlusion_res", nOccRes);
	pObj->GetValue("occlusion_inflate", nGrow);

	Vec3d angles = m_pEntity->GetAngles();
	angles = ConvertToRadAngles(angles);
	pos+=angles;
	m_pEntity->EnablePhysics(false);
	IPhysicalEntity *pPhys = m_pEntity->GetCharInterface()->GetCharacter(0)->GetCharacterPhysics();
	m_pISystem->GetIPhysicalWorld()->SimulateExplosion(pos,pos, rmin, rmax, rmin, impulsive_pressure, nOccRes,nGrow,rmin_occ,&pPhys,1);
	m_pEntity->EnablePhysics(true);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::TrackColliders(IFunctionHandler * pH)
{
	bool bEnable = false;

	pH->GetParam(1,bEnable);
	m_pEntity->TrackColliders( bEnable );

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::GetViewDistRatio(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(0);
	int value =	m_pEntity->GetViewDistRatio();
	return pH->EndFunction(value);
}

//////////////////////////////////////////////////////////////////////////
//! 0..254 (value is autmatically clamed to to this range)
int CScriptObjectEntity::SetViewDistRatio(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	int value;
	pH->GetParam(1, value);
	m_pEntity->SetViewDistRatio(value);

	return pH->EndFunction();
}


//////////////////////////////////////////////////////////////////////////
//!  do not fade out object - no matter how far away it is
int CScriptObjectEntity::SetViewDistUnlimited(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pEntity->SetViewDistUnlimited();

	return pH->EndFunction();
}


//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::RemoveDecals(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(0);
	m_pISystem->GetI3DEngine()->DeleteEntityDecals( m_pEntity );
	return pH->EndFunction();
}


//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::SwitchLight(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);
	int	lightOn;
	pH->GetParam(1, lightOn);
	m_pEntity->SwitchLights(lightOn!=0);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::ForceCharacterUpdate(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int pos;
	pH->GetParam(1,pos);
	m_pEntity->ForceCharacterUpdate(pos);
	return pH->EndFunction();
}


//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::Hide(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int hide=0;
	pH->GetParam(1,hide);
	m_pEntity->Hide(hide!=0);
	return pH->EndFunction();
}


//////////////////////////////////////////////////////////////////////////
int CScriptObjectEntity::CheckCollisions(IFunctionHandler *pH)
{
	assert(pH->GetParamCount()<=2);
	int iEntTypes = ent_sleeping_rigid|ent_rigid|ent_living, iCollTypes = -1;
	pH->GetParam(1,iEntTypes);
	pH->GetParam(2,iCollTypes);

	IPhysicalEntity **ppEnts,*pEnt = m_pEntity->GetPhysics();
	if (pEnt)
	{
		int nEnts,i,nParts,nCont,nTotCont,nEntCont,nContactEnts;
		pe_params_bbox pbb;
		pe_params_foreign_data pfd;
		pe_status_pos sp[2];
		pe_params_part pp[2];
		geom_world_data gwd[2];
		intersection_params ip;
		geom_contact *pContacts;
		IEntity *pIEnt;
		_SmartScriptObject psoRes(m_pScriptSystem),psoContactList(m_pScriptSystem),psoEntList(m_pScriptSystem);
		IScriptObject *psoEnt,*psoNormals[32],*psoCenters[32],*psoContacts[32];

		pEnt->GetParams(&pbb);
		pEnt->GetParams(&pfd);
		nParts = pEnt->GetStatus(&pe_status_nparts());
		pEnt->GetStatus(sp+0);
		ip.bNoAreaContacts = true;
		ip.vrel_min = 1E10f;

		nEnts = m_pISystem->GetIPhysicalWorld()->GetEntitiesInBox(pbb.BBox[0],pbb.BBox[1],ppEnts,iEntTypes);
		nContactEnts = nTotCont = 0;
		for(i=0; i<nEnts; i++)
			if (ppEnts[i]!=pEnt && !(pfd.pForeignData && ppEnts[i]->GetForeignData(pfd.iForeignData)==pfd.pForeignData))
			{
				ppEnts[i]->GetStatus(sp+1);
				psoEnt = (pIEnt = (IEntity*)ppEnts[i]->GetForeignData()) ? pIEnt->GetScriptObject() : 0;
				nEntCont = 0;

				for(pp[1].ipart=ppEnts[i]->GetStatus(&pe_status_nparts())-1; pp[1].ipart>=0; pp[1].ipart--)
				{
					MARK_UNUSED(pp[1].partid); ppEnts[i]->GetParams(pp+1);
					gwd[1].offset = sp[1].pos + sp[1].q*pp[1].pos;
					gwd[1].R = matrix3x3f(sp[1].q*pp[1].q);
					gwd[1].scale = pp[1].scale;
					for(pp[0].ipart=0; pp[0].ipart<nParts; pp[0].ipart++)
					{
						MARK_UNUSED(pp[0].partid); pEnt->GetParams(pp+0);
						if ((iCollTypes==-1 ? pp[0].flagsColliderOR : iCollTypes) & pp[1].flagsOR)
						{
							gwd[0].offset = sp[0].pos + sp[0].q*pp[0].pos;
							gwd[0].R = matrix3x3f(sp[0].q*pp[0].q);
							gwd[0].scale = pp[0].scale;
							for(nCont = pp[0].pPhysGeomProxy->pGeom->Intersect(pp[1].pPhysGeomProxy->pGeom, gwd+0,gwd+1, &ip, pContacts)-1; 
									nCont>=0 && nTotCont<sizeof(psoContacts)/sizeof(psoContacts[0]); nCont--)
							{
								psoCenters[nTotCont] = m_pScriptSystem->CreateObject();
								psoCenters[nTotCont]->BeginSetGetChain();
								psoCenters[nTotCont]->SetValueChain("x",pContacts[nCont].center.x);
								psoCenters[nTotCont]->SetValueChain("y",pContacts[nCont].center.y);
								psoCenters[nTotCont]->SetValueChain("z",pContacts[nCont].center.z);
								psoCenters[nTotCont]->EndSetGetChain();

								psoNormals[nTotCont] = m_pScriptSystem->CreateObject();
								psoNormals[nTotCont]->BeginSetGetChain();
								psoNormals[nTotCont]->SetValueChain("x",-pContacts[nCont].n.x);
								psoNormals[nTotCont]->SetValueChain("y",-pContacts[nCont].n.y);
								psoNormals[nTotCont]->SetValueChain("z",-pContacts[nCont].n.z);
								psoNormals[nTotCont]->EndSetGetChain();

								psoContacts[nTotCont] = m_pScriptSystem->CreateObject();
								psoContacts[nTotCont]->BeginSetGetChain();
								psoContacts[nTotCont]->SetValueChain("center",psoCenters[nTotCont]);
								psoContacts[nTotCont]->SetValueChain("normal",psoNormals[nTotCont]);
								psoContacts[nTotCont]->SetValueChain("partid0",pp[0].partid);
								psoContacts[nTotCont]->SetValueChain("partid1",pp[1].partid);
								if (psoEnt)
									psoContacts[nTotCont]->SetValueChain("collider",psoEnt);
								else
									psoContacts[nTotCont]->SetToNullChain("collider");
								psoContacts[nTotCont]->EndSetGetChain();

								psoContactList->SetAt(nTotCont+1, psoContacts[nTotCont]);
								nTotCont++; nEntCont++;
							}
						}
					}
				}

				if (nEntCont && psoEnt)
					psoEntList->SetAt(nContactEnts+++1, psoEnt);	
			}

		psoRes->SetValue("contacts", psoContactList);
		psoRes->SetValue("entities", psoEntList);
		for(i=0;i<nTotCont;i++)
			psoNormals[i]->Release(), psoCenters[i]->Release(), psoContacts[i]->Release();

		return pH->EndFunction(psoRes);
	}

	return pH->EndFunction();
}


int CScriptObjectEntity::AwakeEnvironment(IFunctionHandler *pH)
{
	pe_params_bbox pbb;
	pe_action_awake aa;
	IPhysicalEntity **ppEnts;
	Vec3 vMin,vMax,vDelta;
	int i,nEnts;

	m_pEntity->GetBBox(vMin,vMax);
	vDelta.x=vDelta.y=vDelta.z = m_pISystem->GetIPhysicalWorld()->GetPhysVars()->maxContactGap*4;
	nEnts = m_pISystem->GetIPhysicalWorld()->GetEntitiesInBox(vMin-vDelta,vMax+vDelta, ppEnts, ent_sleeping_rigid|ent_living|ent_independent);
	for(i=0;i<nEnts;i++)
		ppEnts[i]->Action(&aa);

	return pH->EndFunction();
}

int CScriptObjectEntity::NoExplosionCollision(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	if(m_pEntity->GetPhysics())
	{
		pe_params_part ppart;
		ppart.flagsAND = ~geom_colltype_explosion;
		ppart.ipart = -1;
		do { ++ppart.ipart; } while(m_pEntity->GetPhysics()->SetParams(&ppart));
	}
	return pH->EndFunction();
}
