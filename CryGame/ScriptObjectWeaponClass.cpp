
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	File: ScriptObjectWeapon.cpp
//
//  Description: 
//		ScriptObjectWeapon.cpp: implementation of the CScriptObjectWeapon class.
//
//	History: 
//		- created by Marco C.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptObjectWeaponClass.h"
#include "ScriptObjectVector.h"
#include "WeaponClass.h"
#include "WeaponSystemEx.h"
#include <CryCharAnimationParams.h>
#include <float.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

_DECLARE_SCRIPTABLEEX(CScriptObjectFireParam)
_DECLARE_SCRIPTABLEEX(CScriptObjectWeaponClass)

CScriptObjectFireParam::CScriptObjectFireParam()
{
}
CScriptObjectFireParam::~CScriptObjectFireParam()
{
}

bool CScriptObjectFireParam::Create(IScriptSystem *pScriptSystem,WeaponParams *p)
{
	Init(pScriptSystem,this);
	m_pWeaponParams=p;

	if(!EnablePropertiesMapping(p))
	{
		CryError( "<CryGame> (CScriptObjectFireParam::Create) failed" );
		return false;
	}

	return true;
}
void CScriptObjectFireParam::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectFireParam>::InitializeTemplate(pSS);
	AllowPropertiesMapping(pSS);
	////set default as holding
	RegisterProperty( "ai_mode",PROPERTY_TYPE_BOOL,offsetof(WeaponParams,bAIMode));
	RegisterProperty( "allow_hold_breath",PROPERTY_TYPE_BOOL,offsetof(WeaponParams,bAllowHoldBreath));
	RegisterProperty( "auto_aiming_dist",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fAutoAimDist));
	RegisterProperty( "aim_recoil_modifier",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fAimRecoilModifier));

	RegisterProperty( "accuracy_modifier_standing",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fAccuracyModifierStanding));
	RegisterProperty( "accuracy_modifier_crouch",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fAccuracyModifierCrouch));
	RegisterProperty( "accuracy_modifier_prone",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fAccuracyModifierProne));
	RegisterProperty( "recoil_modifier_standing",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fRecoilModifierStanding));
	RegisterProperty( "recoil_modifier_crouch",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fRecoilModifierCrouch));
	RegisterProperty( "recoil_modifier_prone",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fRecoilModifierProne));
	
	RegisterProperty( "min_accuracy",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fMinAccuracy));
	RegisterProperty( "max_accuracy",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fMaxAccuracy));
	RegisterProperty( "aim_improvement",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fAimImprovement));
	RegisterProperty( "sprint_penalty",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fSprintPenalty));
	RegisterProperty( "reload_time",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fReloadTime));
	RegisterProperty( "fire_rate",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fFireRate));
	RegisterProperty( "distance",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fDistance));
	RegisterProperty( "damage",PROPERTY_TYPE_INT,offsetof(WeaponParams,nDamage));
	RegisterProperty( "damage_drop_per_meters",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,fDamageDropPerMeters));
	RegisterProperty( "bullet_per_shot",PROPERTY_TYPE_INT,offsetof(WeaponParams,nBulletpershot));
	RegisterProperty( "fire_mode_type",PROPERTY_TYPE_INT,offsetof(WeaponParams,iFireModeType));
	// projectile
	RegisterProperty( "projectile_class",PROPERTY_TYPE_STRING,offsetof(WeaponParams,sProjectileClass));
	RegisterProperty( "bullets_per_clip",PROPERTY_TYPE_INT,offsetof(WeaponParams,iBulletsPerClip));
	RegisterProperty( "fire_activation",PROPERTY_TYPE_INT,offsetof(WeaponParams,fire_activation));
	RegisterProperty( "max_recoil",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,max_recoil));
	RegisterProperty( "min_recoil",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,min_recoil));
	RegisterProperty( "no_ammo",PROPERTY_TYPE_BOOL,offsetof(WeaponParams,no_ammo));
	RegisterProperty( "accuracy_decay_on_run",PROPERTY_TYPE_FLOAT,offsetof(WeaponParams,accuracy_decay_on_run));
}

void CScriptObjectFireParam::ReleaseTemplate()
{
	_ScriptableEx<CScriptObjectFireParam>::ReleaseTemplate();
}


IScriptObject *CScriptObjectWeaponClass::m_pMemberBonePos = 0;

CScriptObjectWeaponClass::CScriptObjectWeaponClass()
{
	m_pWeaponClass = 0;
}

CScriptObjectWeaponClass::~CScriptObjectWeaponClass()
{
	// does not have to be deleted here ... it's managed by the weapon system
	m_pWeaponClass = 0;

	// these do have to be deleted, though ;)
	while(!m_vFireParams.empty())
	{
		delete m_vFireParams.back();
		m_vFireParams.pop_back();
	}
}

bool CScriptObjectWeaponClass::Create(CXGame* pGame, CWeaponClass* pWeaponClass)
{
	m_pGame = pGame;
	m_pSystem = pGame->GetSystem();
	m_pWeaponClass = pWeaponClass;

	if (!m_pGame || !m_pSystem || !m_pWeaponClass)
		return false;

	Init(m_pSystem->GetIScriptSystem(), this);

	m_pScriptThis->RegisterParent(this);

	_SmartScriptObject pObj(m_pScriptSystem,true);

	if (!m_pScriptSystem->GetGlobalValue(m_pWeaponClass->GetName().c_str(), pObj))
	{
		m_pSystem->GetILog()->LogToFile("[FATAL ERROR] Script table %s not found. Probably script was not loaded because of an error.",m_pWeaponClass->GetName().c_str());
		return false;
	}	
	m_pScriptThis->Clone(*pObj);
	m_pScriptThis->SetValue("cnt", m_pScriptThis);
	m_pScriptThis->SetValue("classid", m_pWeaponClass->GetID());

	return true;
}

void CScriptObjectWeaponClass::InitializeTemplate(IScriptSystem *pSS)
{
	//////////////////////////////////////////////////////////////////////////
	m_pMemberBonePos = pSS->CreateObject();

	_ScriptableEx<CScriptObjectWeaponClass>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectWeaponClass,SetName);
	REG_FUNC(CScriptObjectWeaponClass,SetShaderFloat);
	REG_FUNC(CScriptObjectWeaponClass,SetBindBone);
	REG_FUNC(CScriptObjectWeaponClass,SetAnimationKeyEvent);
	REG_FUNC(CScriptObjectWeaponClass,StartAnimation);
	REG_FUNC(CScriptObjectWeaponClass,ResetAnimation);
	REG_FUNC(CScriptObjectWeaponClass,GetAnimationLength);
	REG_FUNC(CScriptObjectWeaponClass,GetCurAnimation);
	REG_FUNC(CScriptObjectWeaponClass,IsAnimationRunning);
	REG_FUNC(CScriptObjectWeaponClass,GetPos);

	REG_FUNC(CScriptObjectWeaponClass,SetWeaponFireParams);
	REG_FUNC(CScriptObjectWeaponClass,SetFirstPersonWeaponPos);
	REG_FUNC(CScriptObjectWeaponClass,GetInstantHit);
	//REG_FUNC(CScriptObjectWeapon,Mount);
	REG_FUNC(CScriptObjectWeaponClass,GetBonePos);
	REG_FUNC(CScriptObjectWeaponClass,GetProjectileFiringAngle);
	REG_FUNC(CScriptObjectWeaponClass,Hit);
	REG_FUNC(CScriptObjectWeaponClass,SetHoldingType);

	REG_FUNC(CScriptObjectWeaponClass,LoadObject);
	REG_FUNC(CScriptObjectWeaponClass,AttachObjectToBone);
	REG_FUNC(CScriptObjectWeaponClass,DetachObjectToBone);

	REG_FUNC(CScriptObjectWeaponClass,CacheObject);

	REG_FUNC(CScriptObjectWeaponClass,DrawScopeFlare);
}

void CScriptObjectWeaponClass::ReleaseTemplate()
{
	_ScriptableEx<CScriptObjectWeaponClass>::ReleaseTemplate();
	SAFE_RELEASE( m_pMemberBonePos );
}

int CScriptObjectWeaponClass::SetName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sName;
	pH->GetParam(1,sName);
	m_pWeaponClass->SetName(sName);
	return pH->EndFunction();
}

int CScriptObjectWeaponClass::SetShaderFloat(IFunctionHandler *pH)
{
	float fFloat,fFadeValue;
	const char *sName;
	int		dwMask;

	CHECK_PARAMETERS(4);

	pH->GetParam(1, sName);
	pH->GetParam(2, fFloat);
	pH->GetParam(3, dwMask);
	pH->GetParam(4, fFadeValue);

	// set it for object
	if ((dwMask) && m_pWeaponClass->GetObject())
		m_pWeaponClass->GetObject()->SetShaderFloat(sName, fFloat);

	// set it for character
	if ((dwMask == 0) && m_pWeaponClass->GetCharacter())
		m_pWeaponClass->GetCharacter()->SetShaderFloat(sName, fFloat);

	return pH->EndFunction();
}

int CScriptObjectWeaponClass::SetBindBone(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *sBone;
	pH->GetParam(1,sBone);
	m_pWeaponClass->SetBindBone(sBone);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectWeaponClass::SetAnimationKeyEvent(IFunctionHandler *pH)
{
	if (pH->GetParamCount()<2 || pH->GetParamCount()>3)
	{
		m_pScriptSystem->RaiseError("CScriptObjectWeaponClass::SetAnimationKeyEvent wrong number of arguments");
		return pH->EndFunctionNull();
	};

	const char *szAnimation;
	int nFrameID;
	USER_DATA udUserData = USER_DATA(-1);
	pH->GetParam(1,szAnimation);
	pH->GetParam(2,nFrameID);
	if(pH->GetParamCount()>2)
		pH->GetParam(3,udUserData);

	if(m_pWeaponClass->GetCharacter())
	{
		m_pWeaponClass->GetCharacter()->AddAnimationEventSink(szAnimation, m_pWeaponClass);
		m_pWeaponClass->GetCharacter()->AddAnimationEvent(szAnimation, nFrameID, (void*)udUserData);
	}

	return pH->EndFunction();
}

int CScriptObjectWeaponClass::StartAnimation(IFunctionHandler *pH)
{
	if (!m_pWeaponClass->GetCharacter())
		return pH->EndFunctionNull();

	const char *animname;
	int pos, layer=0;
	bool bLooping = false;
	bool bLoopSpecified = false;
	float fBlendTime = 0.15f;
	float fAniSpeed = 1.0f;
	pH->GetParam(1,pos);
	if (!pH->GetParam(2,animname))
	{
		//m_pGame->GetSystem()->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,0,
		//	0,"CScriptObjectWeaponClass::StartAnimation, animation name not specified, in WeaponClass %s", m_pWeaponClass->GetName().c_str() );
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
				if (pH->GetParamCount() > 5)
				{
					bLoopSpecified = true;
					pH->GetParam(6,bLooping);
				}
			}
		}
	}

	if (bLoopSpecified)
	{
		ICryCharInstance *pCharacter = m_pWeaponClass->GetCharacter();
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

	if (string(animname) == "NULL")
		animname = 0;

	m_pWeaponClass->GetCharacter()->SetAnimationSpeed( fAniSpeed );

	bool result = false;

	if (animname)
	{
		CryCharAnimationParams ccap;
		ccap.fBlendInTime = fBlendTime;
		ccap.fBlendOutTime = 0;
		ccap.nLayerID = layer;
		result = m_pWeaponClass->GetCharacter()->StartAnimation(animname,ccap);
		
		//[PETAR] StartAnimation2 is now obsolete. Call commented and left as reference
		//result = m_pWeaponClass->GetCharacter()->StartAnimation2(animname, fBlendTime, fBlendTime, layer, true, false);
	}
	else
		result = m_pWeaponClass->GetCharacter()->StopAnimation(layer);
	return pH->EndFunction(result);
}

int CScriptObjectWeaponClass::ResetAnimation(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	if (m_pWeaponClass->GetCharacter())
	{
		m_pWeaponClass->GetCharacter()->ResetAnimations();
	}
	return pH->EndFunction();
}

int CScriptObjectWeaponClass::GetAnimationLength(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	const char *aniName;
	pH->GetParam(1, aniName);

	float length = 0.0f;

	if (m_pWeaponClass->GetCharacter())
		length = m_pWeaponClass->GetCharacter()->GetModel()->GetAnimationSet()->GetLength(aniName);
	return pH->EndFunction(length);
}

int CScriptObjectWeaponClass::GetCurAnimation(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	if (m_pWeaponClass->GetCharacter())
	{
		if (m_pWeaponClass->GetCharacter()->GetCurAnimation() && m_pWeaponClass->GetCharacter()->GetCurAnimation()[0] != '\0')

			return pH->EndFunction(m_pWeaponClass->GetCharacter()->GetCurAnimation());
	}

	return pH->EndFunction();
}

int CScriptObjectWeaponClass::IsAnimationRunning(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	bool bResult=false;
	if (m_pWeaponClass->GetCharacter())
		if (m_pWeaponClass->GetCharacter()->GetCurAnimation())
			bResult=true;

	return pH->EndFunction(bResult);
}

int CScriptObjectWeaponClass::GetPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	CScriptObjectVector oVec(m_pScriptSystem);

	oVec=m_pWeaponClass->GetPos();

	return pH->EndFunction(*oVec);
}

/*! Adds a set of fire-parameters
@param pObj table of fireparams (accuracy, ammo, max_ammo, reload_time, fire_rate, distance, damage, blast_radius, bullet_per_shot, instant, projectile_mass, projectile_size, projectile_velocity, acc_thrust, acc_lift, air_resistance, gravity, surface_idx, projectile_class, fx, fy, bullets_per_clip)
*/ 
int CScriptObjectWeaponClass::SetWeaponFireParams(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pObj(m_pScriptSystem,true);

	WeaponParams s;
	s.fDamageDropPerMeters=0;

	pH->GetParam(1,*pObj);
	pObj->BeginSetGetChain();

	pObj->GetValueChain( "ai_mode",					s.bAIMode );
	pObj->GetValueChain( "allow_hold_breath",s.bAllowHoldBreath);
	pObj->GetValueChain( "auto_aiming_dist",s.fAutoAimDist );
	pObj->GetValueChain( "min_accuracy",		s.fMinAccuracy );
	pObj->GetValueChain( "max_accuracy",		s.fMaxAccuracy );
	pObj->GetValueChain( "aim_improvement",	s.fAimImprovement );
	pObj->GetValueChain( "sprint_penalty",	s.fSprintPenalty );
	pObj->GetValueChain( "aim_recoil_modifier",	s.fAimRecoilModifier );
	pObj->GetValueChain( "reload_time",			s.fReloadTime );
	pObj->GetValueChain( "fire_rate",				s.fFireRate );
	pObj->GetValueChain( "tap_fire_rate",		s.fTapFireRate );
	pObj->GetValueChain( "distance",				s.fDistance );
	pObj->GetValueChain( "damage",					s.nDamage );
	pObj->GetValueChain( "damage_drop_per_meter",s.fDamageDropPerMeters );
	pObj->GetValueChain( "bullet_per_shot",	s.nBulletpershot );
	pObj->GetValueChain( "fire_mode_type",	s.iFireModeType );
	pObj->GetValueChain( "shoot_underwater",s.bShootUnderwater );

	// accuracy and recoil modifier
	pObj->GetValueChain( "accuracy_modifier_standing", s.fAccuracyModifierStanding );
	pObj->GetValueChain( "accuracy_modifier_crouch", s.fAccuracyModifierCrouch );
	pObj->GetValueChain( "accuracy_modifier_prone", s.fAccuracyModifierProne );
	pObj->GetValueChain( "recoil_modifier_standing", s.fRecoilModifierStanding );
	pObj->GetValueChain( "recoil_modifier_crouch", s.fRecoilModifierCrouch );
	pObj->GetValueChain( "recoil_modifier_prone", s.fRecoilModifierProne );

	// projectile
	const char *sClass = NULL;
	pObj->GetValueChain( "projectile_class", sClass);
	if (sClass)
		s.sProjectileClass = sClass;

	pObj->GetValueChain( "bullets_per_clip",			s.iBulletsPerClip);
	pObj->GetValueChain( "fire_activation",			s.fire_activation);
	pObj->GetValueChain( "min_recoil",			s.min_recoil);
	pObj->GetValueChain( "max_recoil",			s.max_recoil);
	pObj->GetValueChain( "no_ammo",			s.no_ammo);
	pObj->GetValueChain( "accuracy_decay_on_run",			s.accuracy_decay_on_run);
	if (!pObj->GetValueChain( "whizz_sound_radius",			s.whizz_sound_radius))
		s.whizz_sound_radius=0.0f;
	s.whizz_sound_radius*=s.whizz_sound_radius;	// keep it squared
	if (!pObj->GetValueChain( "iImpactForceMul",			s.iImpactForceMul))
		s.iImpactForceMul = 2;
	if (!pObj->GetValueChain( "iImpactForceMulFinal",			s.iImpactForceMulFinal))
		s.iImpactForceMulFinal = 10;
	if (!pObj->GetValueChain( "iImpactForceMulFinalTorso",			s.iImpactForceMulFinalTorso))
		s.iImpactForceMulFinalTorso = 0;

	pObj->EndSetGetChain();

	int nFireParamIndex=m_vFireParams.size();

	CScriptObjectFireParam *pFireParamObj=new CScriptObjectFireParam;

	WeaponParams *pReadAddress=m_pWeaponClass->AddWeaponParams( s );
	pFireParamObj->Create(m_pScriptSystem,pReadAddress);
	m_vFireParams.push_back(pFireParamObj);
	pFireParamObj->GetScriptObject()->Clone(pObj);
	_SmartScriptObject soCnt(m_pScriptSystem, true);
	if (m_pScriptThis->GetValue("cnt", soCnt))
	{
		soCnt->SetAt(nFireParamIndex, pFireParamObj->GetScriptObject());
	}
	
	return pH->EndFunction();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Sets the first-person-weapon position and orientation
@param v3Pos position of weapon
@param v3Angles orientation of weapon
*/
int CScriptObjectWeaponClass::SetFirstPersonWeaponPos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	Vec3d v3Pos;
	Vec3d v3Angles;
	pH->GetParam(1,*oVec);
	v3Pos=oVec.Get();
	pH->GetParam(2,*oVec);
	v3Angles=oVec.Get();
	m_pWeaponClass->SetFirstPersonWeaponPos(v3Pos,v3Angles);	
	return pH->EndFunction();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_HITS 4
/*! Traces a ray to determine what and where an instant weapon hits
@param pObj ray-description-table (shooter, pos, angles, dir, distance)
@return target-description-table (objtype (0=entity, 1=stat-obj, 2=terrain), pos, normal, dir, target (nil if objtype!=0))
*/
int CScriptObjectWeaponClass::GetInstantHit(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	_SmartScriptObject pTempObj(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	CScriptObjectVector oVec1(m_pScriptSystem,false);
	CScriptObjectVector oVec2(m_pScriptSystem,false);
	CScriptObjectVector oVec3(m_pScriptSystem,false);

	IEntity *shooter;
	int nID;
	Vec3d pos, angles, dir;
	float fDistance;
	int res;

	pH->GetParam(1,*pObj);
	pObj->GetValue( "shooter",*pTempObj );
	pTempObj->GetValue("id",nID);
	
	shooter= m_pSystem->GetIEntitySystem()->GetEntity(nID);
	if(shooter==NULL)
	{
		TRACE("CScriptObjectWeapon::GetInstantHit() shooter in nil");
		return pH->EndFunctionNull();
	}
	pObj->GetValue( "pos", *oVec );
	pos=oVec.Get();
	pObj->GetValue( "angles", *oVec );
	angles=oVec.Get();
	pObj->GetValue( "dir", *oVec );
	dir=oVec.Get();
	pObj->GetValue( "distance", fDistance );
	dir*=fDistance;

	IPhysicalEntity *skip=shooter->GetPhysics();

	ray_hit hits[MAX_HITS];
	res=m_pSystem->GetIPhysicalWorld()->RayWorldIntersection(pos,dir, ent_all, 0,hits,MAX_HITS, skip);			
	if(res){
		_SmartScriptObject pTable(m_pScriptSystem);
		for(int nCount=0;nCount<MAX_HITS;nCount++)
		{
			_SmartScriptObject pOut(m_pScriptSystem);
			CScriptObjectVector oVec1(m_pScriptSystem);
			CScriptObjectVector oVec2(m_pScriptSystem);
			CScriptObjectVector oVec3(m_pScriptSystem);
			int objecttype;
			if(hits[nCount].dist<=0)
				continue;
			IEntity *centycontact = NULL;
			if (res && hits[nCount].dist>0 && hits[nCount].pCollider)
			{
				centycontact = (IEntity *)hits[nCount].pCollider->GetForeignData();
				if (centycontact && centycontact->IsGarbage())
				{
					res = 0;
				}
				if (centycontact && (!centycontact->IsGarbage()))
					objecttype = OT_ENTITY;
				else 
					objecttype = OT_STAT_OBJ;
			}

			if (hits[nCount].pCollider)
			{
				if (centycontact)
				{
					objecttype=0;	// entity
					pOut->SetValue("target", centycontact->GetId());
				}
				else
				{
					objecttype=1;	// stat obj
					pOut->SetToNull("target");
				}
			}else
			{
				objecttype=2; // terrain
				pOut->SetToNull("target");
			}

			pOut->SetValue("objtype", objecttype);
			oVec1=(Vec3d)hits[nCount].pt;
			pOut->SetValue("pos", *oVec1);
			oVec2=(Vec3d)hits[nCount].n;
			pOut->SetValue("normal", *oVec2);
			oVec3=GetNormalized(dir);
			pOut->SetValue( "dir", *oVec3 );
			pOut->SetValue( "surfaceid", hits[nCount].surface_idx );
			pOut->SetValue( "partid", hits[nCount].partid);
			pOut->SetValue( "shooter",nID);

			pTable->SetAt(nCount,pOut);
			if (res && hits[0].dist>0) 
				break;       
		}
		return pH->EndFunction(pTable);
	}
	else{
		return pH->EndFunctionNull();
	}
}

/*! Makes sure mounted weapons are updated
*/
/*int CScriptObjectWeapon::Mount(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	m_pWeapon->Mount(true);

	return pH->EndFunctionNull();
}*/


/*! Obtains the position of a bone in the weapon object while taking into account
that weapons are rendered with a different FOV
@param sBoneName Name of the bone
*/
int CScriptObjectWeaponClass::GetBonePos(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	
	const char * sBoneName = "wt";
	pH->GetParam(1,sBoneName);

	ICryCharInstance * cmodel = m_pWeaponClass->GetCharacter();    

	if (!cmodel) 
		return pH->EndFunctionNull();

	ICryBone * pBone = cmodel->GetBoneByName(sBoneName);
	if(!pBone)
	{
		return pH->EndFunctionNull();
	}

	Vec3d vBonePos = pBone->GetBonePosition();
	Vec3d angles = m_pWeaponClass->GetAngles();

	Matrix44 m=Matrix34::CreateRotationXYZ( Deg2Rad(angles), m_pWeaponClass->GetPos() );	//set rotation and translation in one function call
	m	=	GetTransposed44(m); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

	// get result
	Vec3 vec = m.TransformPointOLD(vBonePos);

	m_pMemberBonePos->BeginSetGetChain();
	m_pMemberBonePos->SetValueChain("x",vec.x);
	m_pMemberBonePos->SetValueChain("y",vec.y);
	m_pMemberBonePos->SetValueChain("z",vec.z);
	m_pMemberBonePos->EndSetGetChain();

	return pH->EndFunction(m_pMemberBonePos);
}

/*! Calculate the needed firing angle to fire a projectile with the
passed properties to the passed spot
@param v Velocity of the projectile
@param g Gravity of the projectile
@param x horizontal distance to the required impact point
@param y vertical distance between shooter and projectile impact point


input: x - horizontal distance to target (always positive), y - vertical distance to target (pisitive or negative), v - initial particle velocity (scalar, positive), g - gravity (positive scalar, assume that it vector points down)

d = sqrt(v^4 - 2*g*y*v^2-g^2*x^2)
(if expression is negative, no solution)
a = v^2-g*y
t = sqrt(2*(a+d)/g^2)
angle = acos(x/(v*t))

then compute y_test = v*sin_tpl(angle)*t-g*t^2/2,
if (y_test*y<0 (false root) && a-d>0), try t = sqrt(2*(a-d)/g^2) (else no solution)



*/
int CScriptObjectWeaponClass::GetProjectileFiringAngle(IFunctionHandler *pH)
{

	float x,y,v,g;
	CHECK_PARAMETERS(4);

	pH->GetParam(1, v);
	pH->GetParam(2, g);
	pH->GetParam(3, x);
	pH->GetParam(4, y);

	float angle=0.0,t,a;

	// Avoid square root in script
	float d = cry_sqrtf(powf(v,4)-2*g*y*powf(v,2)-powf(g,2)*powf(x,2));
	if(d>=0)
	{
		a=powf(v,2)-g*y;
		if (a-d>0) {
			t=cry_sqrtf(2*(a-d)/powf(g,2));
			angle = (float)acos_tpl(x/(v*t));	
			float y_test;
			y_test=float(-v*sin_tpl(angle)*t-g*powf(t,2)/2);
			if (fabsf(y-y_test)<0.02f)
				return pH->EndFunction(RAD2DEG(-angle));
			y_test=float(v*sin_tpl(angle)*t-g*pow(t,2)/2);
			if (fabsf(y-y_test)<0.02f)
				return pH->EndFunction(RAD2DEG(angle));
		}
		t = cry_sqrtf(2*(a+d)/powf(g,2));
		angle = (float)acos_tpl(x/(v*t));	
		float y_test=float(v*sin_tpl(angle)*t-g*pow(t,2)/2);

		if (fabsf(y-y_test)<0.02f)
			return pH->EndFunction(RAD2DEG(angle));

		return pH->EndFunction(0);
	}
	return pH->EndFunction(0);
}


int CScriptObjectWeaponClass::Hit(IFunctionHandler *pH)
{
	_SmartScriptObject pHit(m_pScriptSystem,true);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	if(pH->GetParam(1,pHit))
	{
		if(pHit->Count())
		{
			pHit->BeginIteration();
			while (pHit->MoveNext())
			{
				if(pHit->GetCurrent(pObj))
				{
					int idShooter,idTarget,objtype,partid,surfaceid;
					pObj->BeginSetGetChain();

					SWeaponHit hit;
					pObj->GetValueChain("pos",oVec);
					hit.pos = oVec.Get();
					pObj->GetValueChain("dir",oVec);
					hit.dir = GetNormalized(((Vec3d&)oVec));
					pObj->GetValueChain("normal",oVec);
					hit.normal = oVec.Get();
					pObj->GetValueChain("target",idTarget);
					hit.target = m_pSystem->GetIEntitySystem()->GetEntity(idTarget);
					pObj->GetValueChain("partid",partid);
					hit.ipart = partid;
					pObj->GetValueChain("objectype",objtype);
					hit.objecttype =objtype;
					pObj->GetValueChain("shooter",idShooter);
					hit.shooter = m_pSystem->GetIEntitySystem()->GetEntity(idShooter);
					hit.weapon = m_pWeaponClass->GetScriptObject();
					hit.projectile = 0; // Instant weapon, no projectiles.
					hit.damage = (float)m_pWeaponClass->m_fireParams.nDamage;
					hit.iImpactForceMul = m_pWeaponClass->m_fireParams.iImpactForceMul;
					hit.iImpactForceMulFinal = m_pWeaponClass->m_fireParams.iImpactForceMulFinal;
					hit.iImpactForceMulFinalTorso = m_pWeaponClass->m_fireParams.iImpactForceMulFinalTorso;
					pObj->GetValueChain("surfaceid",surfaceid);
					hit.surface_id=surfaceid;
					hit.weapon_death_anim_id = m_pWeaponClass->m_fireParams.iDeathAnim;
					// hit.iNumBullets = iNumShots;
					pObj->EndSetGetChain();
					//m_pWeaponClass->ProcessHit(hit);
				}
			}
			pHit->EndIteration();
		}
	}
	return pH->EndFunction();
}


/*
*/
int CScriptObjectWeaponClass::SetHoldingType(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	//	int iCrosshairIdx;
	int type;
	pH->GetParam(1, type);
	m_pWeaponClass->m_HoldingType = type;
	return pH->EndFunction();
}

int CScriptObjectWeaponClass::LoadObject(IFunctionHandler *pH)
{
	char *geometryName;
	pH->GetParam(1, geometryName);

	m_pWeaponClass->LoadMuzzleFlash(geometryName);
	return pH->EndFunction();
}

//------------------------------------------------------------------
int CScriptObjectWeaponClass::AttachObjectToBone(IFunctionHandler *pH)
{
	char *boneName;
	pH->GetParam(2, boneName);

	if (m_pWeaponClass->GetCharacter() && m_pWeaponClass->GetMuzzleFlash())
	{
		int boneid = m_pWeaponClass->GetCharacter()->GetModel()->GetBoneByName(boneName);
		if (boneid >= 0)
		{
			ICryCharInstance::ObjectBindingHandle boneHandler;
			boneHandler = m_pWeaponClass->GetCharacter()->AttachToBone(m_pWeaponClass->GetMuzzleFlash(), boneid);

			// Make user data for bone handler.
			USER_DATA ud = m_pScriptSystem->CreateUserData( boneHandler,USER_DATA_BONEHANDLER );
			return pH->EndFunction(ud);
		}
	}
	
	return pH->EndFunctionNull();
}

//
//------------------------------------------------------------------
int CScriptObjectWeaponClass::DetachObjectToBone(IFunctionHandler *pH)
{
	char *boneName;
	pH->GetParam(1,boneName);

	int BAD_HANDLER = -1;
	int nCookie;
	ULONG_PTR boneHandler = BAD_HANDLER;

	if (!pH->GetParamUDVal(2,boneHandler,nCookie))
	{
		boneHandler = BAD_HANDLER;
	}
	if (nCookie != USER_DATA_BONEHANDLER)
	{
		boneHandler = BAD_HANDLER;
	}

	if (m_pWeaponClass->GetCharacter())
	{
		if (boneHandler == -1)
		{
			m_pWeaponClass->GetCharacter()->AttachObjectToBone( NULL, boneName, false );
		}
		else
		{
			m_pWeaponClass->GetCharacter()->Detach( boneHandler );
		}
	}
	return pH->EndFunction();
}

int CScriptObjectWeaponClass::CacheObject(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	char *geometryName;
	pH->GetParam(1, geometryName);

	m_pWeaponClass->GetWeaponSystem().CacheObject(geometryName);
	return pH->EndFunction();
}

int CScriptObjectWeaponClass::DrawScopeFlare(IFunctionHandler *pH)
{
	CScriptObjectVector oVec(m_pScriptSystem,true);
	_SmartScriptObject  pObj(m_pScriptSystem,true);		

	if (!pH->GetParam(1,*pObj))
		return pH->EndFunction(-1);

	IScriptObject *pITable=*pObj;

	if (!pITable->BeginSetGetChain())
		return pH->EndFunction(-1);

	CDLight DynLight;

	//////////////////////////////////////////////////////////////////////////
	const char *sShaderName=NULL;

	if (!pITable->GetValueChain( "lightShader",sShaderName))
		m_pScriptSystem->RaiseError( "<DrawScopeFlare> sShaderName not specified" );

	DynLight.m_Flags = DLF_POINT;

	DynLight.m_pShader = m_pGame->GetSystem()->GetIRenderer()->EF_LoadShader(sShaderName, eSH_World);

	//////////////////////////////////////////////////////////////////////////
	// cast shadows 
	int	nThisAreaOnly = 0;
	if (!pITable->GetValueChain("areaonly",nThisAreaOnly))
		m_pScriptSystem->RaiseError( "<DrawScopeFlare> thisareaonly not specified" );
	else
	{
		if (nThisAreaOnly==1)
			DynLight.m_Flags |= DLF_THIS_AREA_ONLY;
	}

	//////////////////////////////////////////////////////////////////////////
	// shaders stuff
	DynLight.m_Flags |= DLF_LIGHTSOURCE;

	//////////////////////////////////////////////////////////////////////////
	// more shaders stuff
	DynLight.m_pLightImage = NULL;
	DynLight.m_Flags |= DLF_POINT;

	//////////////////////////////////////////////////////////////////////////	
	int shooterid;
	float factor;
	IEntity* pShooter = NULL;
	if (pITable->GetValueChain("shooterid", shooterid))
	{
		pShooter = m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(shooterid);
		assert(pShooter);
		if (pShooter->GetEntityVisArea() != 0)
			return pH->EndFunction();	

		IEntity* pPlayer = m_pGame->GetMyPlayer();

		if (pPlayer == NULL)
			return pH->EndFunction();

		if (pPlayer == pShooter)
			return pH->EndFunction();	

		CPlayer *pShooterPlayer=NULL;
		if (pShooter->GetContainer()) pShooter->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pShooterPlayer);
		assert(pShooterPlayer);
		
		// position flare relative to weapon_bone
		if (pShooter->GetCharInterface()->GetCharacter(0) == NULL)
			return pH->EndFunction();	

		ICryBone *pBone = pShooter->GetCharInterface()->GetCharacter(0)->GetBoneByName("weapon_bone");

		if (!pBone)
			return pH->EndFunction();	

		Vec3 rot = pShooterPlayer->GetActualAngles();
		rot.x = 0;
		rot.z = pShooterPlayer->m_LegAngle;
		Matrix33 shooterMatrix = Matrix33::CreateRotationXYZ( Deg2Rad(rot));

		DynLight.m_Origin = pShooter->GetPos() + shooterMatrix * (pBone->GetBonePosition() + 0.1f*pBone->GetBoneAxis('z'));

		Vec3 vPlayerDir;
		GetDirection(pPlayer, vPlayerDir); 
		Vec3 vShooterDir = pBone->GetBoneAxis('x');
		vShooterDir = shooterMatrix * vShooterDir;

		Vec3 diff = pShooter->GetPos() - pPlayer->GetPos();
		diff.Normalize();

		factor = fabs(2.0f * (vPlayerDir.Dot(diff) - 0.3f)); 
		factor = FClamp(factor, 0, 1); 
		factor = -vShooterDir.Dot(diff) * factor;  

		const float fAngle=30.0f * gf_DEGTORAD;		// -30..30

		float fAngleCos=cry_cosf(fAngle);

		factor = (factor-fAngleCos)/(1.0f-fAngleCos);

		float f=m_pGame->GetSystem()->GetITimer()->GetCurrTime()*7.0f;

		float fNoise= (cry_sinf(f*3.0f) + cry_sinf(f*2.0f) + cry_sinf(f*5.0f + 1.0f))/3.0f;		// -1..1

		factor -= 0.3f* (fNoise*0.5f+0.5f);

		if(factor < 0) 
			factor = 0;

		//////////////////////////////////////////////////////////////////////////		
		if (!pITable->GetValueChain( "orad",DynLight.m_fRadius))
			return pH->EndFunction();	

    //////////////////////////////////////////////////////////////////////////	
    if (!pITable->GetValueChain( "coronaScale", DynLight.m_fCoronaScale))
      return pH->EndFunction();	

		DynLight.m_fCoronaScale*= factor;

		Vec3 vSunColor = m_pGame->GetSystem()->GetI3DEngine()->GetSunColor();
    DynLight.m_Color = CFColor (vSunColor.x, vSunColor.y, vSunColor.z) * 1.0f * DynLight.m_fCoronaScale;      
		DynLight.m_Color.Clamp();

		DynLight.m_SpecColor.Set(1, 1, 1, 1);

		//////////////////////////////////////////////////////////////////////////
		// finally add it to the engine
		m_pGame->GetSystem()->GetIRenderer()->EF_UpdateDLight(&DynLight);

		m_pGame->GetSystem()->GetI3DEngine()->AddDynamicLightSource(DynLight, pShooter);

		if (DynLight.m_pShader->GetRefCount() == 1)
			DynLight.m_pShader->AddRef();
	}

	pITable->EndSetGetChain();

	return pH->EndFunction();	
}

void CScriptObjectWeaponClass::GetDirection(IEntity *pEntity, Vec3& vDir)
{
	Matrix44 tm;

	vDir.Set(0,-1,0);
	tm.SetIdentity();
	tm=Matrix44::CreateRotationZYX(-pEntity->GetAngles()*gf_DEGTORAD)*tm; //NOTE: angles in radians and negated 
	vDir = GetTransposed44(tm)*vDir;
}