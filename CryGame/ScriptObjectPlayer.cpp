
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// ScriptObjectPlayer.cpp: implementation of the CScriptObjectPlayer class.
//
//////////////////////////////////////////////////////////////////////
 
#include "stdafx.h"
#include "ScriptObjectPlayer.h"
#include "XPlayer.h"
#include "Game.h"
#include "WeaponSystemEx.h"
#include "WeaponClass.h"
#include "XVehicle.h"
#include "ScriptObjectVector.h"
#include <ICryAnimation.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

_DECLARE_SCRIPTABLEEX(CScriptObjectPlayer)


IScriptObject *CScriptObjectPlayer::m_pTempObj = 0;
IScriptObject *CScriptObjectPlayer::m_pTempAng = 0;
IScriptObject *CScriptObjectPlayer::m_pWeaponSlots = 0;
IScriptObject *CScriptObjectPlayer::m_pTempBloodObj = 0;
IScriptObject *CScriptObjectPlayer::m_pBlindScreenPos = 0;

IScriptObject* CScriptObjectPlayer::m_memberSO[SOP_MEMBER_LAST];

CScriptObjectPlayer::CScriptObjectPlayer():
m_fSpeedRun(0.0f),
m_fSpeedWalk(0.0f),
m_fSpeedCrouch(0.0f),
m_fSpeedProne(0.0f)
{
	m_LastTouchedMaterialID = -1;
}

CScriptObjectPlayer::~CScriptObjectPlayer()
{
	
}

bool CScriptObjectPlayer::Create(IScriptSystem *pScriptSystem)
{
	
	Init(pScriptSystem,this);
	
	m_pScriptThis->RegisterParent(this);
	m_pCameraOffset.Create( pScriptSystem );
	m_pGetColor.Create( pScriptSystem );
	m_pWeaponInfo.Create(pScriptSystem);
	return true;
}

void CScriptObjectPlayer::ReleaseTemplate()
{
	SAFE_RELEASE(m_pTempObj);
	SAFE_RELEASE(m_pTempAng);
	SAFE_RELEASE(m_pWeaponSlots);
	SAFE_RELEASE(m_pTempBloodObj);
	SAFE_RELEASE(m_pBlindScreenPos);
	for (int i = 0; i < SOP_MEMBER_LAST; i++)
	{
		SAFE_RELEASE( m_memberSO[i] );
	}
	_ScriptableEx<CScriptObjectPlayer>::ReleaseTemplate();
}

void CScriptObjectPlayer::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectPlayer>::InitializeTemplate(pSS);
	m_pTempObj=pSS->CreateObject();
	m_pTempAng=pSS->CreateObject();
	m_pWeaponSlots=pSS->CreateObject();
	m_pTempBloodObj=pSS->CreateObject();
	m_pBlindScreenPos=pSS->CreateObject();
	for (int i = 0; i < SOP_MEMBER_LAST; i++)
	{
		m_memberSO[i] = pSS->CreateObject();
	}

	REG_FUNC(CScriptObjectPlayer,GetWeaponInfo);
	REG_FUNC(CScriptObjectPlayer,GetWeaponsSlots);
	REG_FUNC(CScriptObjectPlayer,CalculateAccuracyFactor);
	REG_FUNC(CScriptObjectPlayer,WaitForFireRelease);
	//REG_FUNC(CScriptObjectPlayer,SetWeaponInfo);
	REG_FUNC(CScriptObjectPlayer,SetCurrWeapon);
	REG_FUNC(CScriptObjectPlayer,GetCurrWeapon);
	REG_FUNC(CScriptObjectPlayer,SetSwayAmp);
	REG_FUNC(CScriptObjectPlayer,SetSwayFreq);
	REG_FUNC(CScriptObjectPlayer,GetViewIntersection);
	REG_FUNC(CScriptObjectPlayer,SetGravity);
	REG_FUNC(CScriptObjectPlayer,SetAngleLimitBase);
	REG_FUNC(CScriptObjectPlayer,SetMinAngleLimitV);
	REG_FUNC(CScriptObjectPlayer,SetMaxAngleLimitV);
	REG_FUNC(CScriptObjectPlayer,EnableAngleLimitV);
	REG_FUNC(CScriptObjectPlayer,SetMinAngleLimitH);
	REG_FUNC(CScriptObjectPlayer,SetMaxAngleLimitH);
	REG_FUNC(CScriptObjectPlayer,EnableAngleLimitH);
	REG_FUNC(CScriptObjectPlayer,SetName);
	REG_FUNC(CScriptObjectPlayer,GetName);
	REG_FUNC(CScriptObjectPlayer,InitWeapons);
	REG_FUNC(CScriptObjectPlayer,GetCurrWeaponId);
	REG_FUNC(CScriptObjectPlayer,CalcDmgShakeAxis);
	REG_FUNC(CScriptObjectPlayer,ShakeCamera);
	REG_FUNC(CScriptObjectPlayer,DeselectWeapon);
	REG_FUNC(CScriptObjectPlayer,RedirectInputTo);
	REG_FUNC(CScriptObjectPlayer,SetCameraOffset);
	REG_FUNC(CScriptObjectPlayer,GetCameraOffset);
	REG_FUNC(CScriptObjectPlayer,StartDie);
//	REG_FUNC(CScriptObjectPlayer,Die);
	REG_FUNC(CScriptObjectPlayer,SetDimNormal);
	REG_FUNC(CScriptObjectPlayer,SetDimCrouch);
	REG_FUNC(CScriptObjectPlayer,SetDimProne);	
	REG_FUNC(CScriptObjectPlayer,GetBoneHitZone);		
	REG_FUNC(CScriptObjectPlayer,GetArmDamage);
	REG_FUNC(CScriptObjectPlayer,GetLegDamage);
	REG_FUNC(CScriptObjectPlayer,SetMoveParams);
	REG_FUNC(CScriptObjectPlayer,HasCollided);
	REG_FUNC(CScriptObjectPlayer,SetDynamicsProperties);
	REG_FUNC(CScriptObjectPlayer,GetTreadedOnMaterial);
	REG_FUNC(CScriptObjectPlayer,GetTPVHelper);
	REG_FUNC(CScriptObjectPlayer,GetHelperPos);
	REG_FUNC(CScriptObjectPlayer,GetTouchedMaterial);
	//REG_FUNC(CScriptObjectPlayer,GetTargetScreenPos);
	//REG_FUNC(CScriptObjectPlayer,GetTargetTime);
	REG_FUNC(CScriptObjectPlayer,GetCharacterAngles);
	REG_FUNC(CScriptObjectPlayer,ShakeCameraL);
	REG_FUNC(CScriptObjectPlayer,MakeWeaponAvailable);
	REG_FUNC(CScriptObjectPlayer,SelectFirstWeapon);
	REG_FUNC(CScriptObjectPlayer,StartFire);
	REG_FUNC(CScriptObjectPlayer,PlaySound);
	REG_FUNC(CScriptObjectPlayer,GetFirePosAngles);
	REG_FUNC(CScriptObjectPlayer,GetCurVehicle);
	REG_FUNC(CScriptObjectPlayer,SetAnimationRefSpeedRun);
	REG_FUNC(CScriptObjectPlayer,SetAnimationRefSpeedWalk);
	REG_FUNC(CScriptObjectPlayer,SetAnimationRefSpeedWalkRelaxed);
	REG_FUNC(CScriptObjectPlayer,SetAnimationRefSpeedXWalk);
	REG_FUNC(CScriptObjectPlayer,SetAnimationRefSpeedXRun);
	REG_FUNC(CScriptObjectPlayer,SetAnimationRefSpeedCrouch);
	REG_FUNC(CScriptObjectPlayer,SelectNextWeapon);
	REG_FUNC(CScriptObjectPlayer,DrawThirdPersonWeapon);
	REG_FUNC(CScriptObjectPlayer,SetDimOverride);
	REG_FUNC(CScriptObjectPlayer,CounterAdd);
	REG_FUNC(CScriptObjectPlayer,CounterIncrement);
	REG_FUNC(CScriptObjectPlayer,CounterGetValue);
	REG_FUNC(CScriptObjectPlayer,CounterSetValue);
	REG_FUNC(CScriptObjectPlayer,CounterSetEvent);
	REG_FUNC(CScriptObjectPlayer,SetHeatVisionValues);
	REG_FUNC(CScriptObjectPlayer,HolsterGun);
	REG_FUNC(CScriptObjectPlayer,HoldGun);
	REG_FUNC(CScriptObjectPlayer,SetBlendTime);
	REG_FUNC(CScriptObjectPlayer,SwitchFlashLight);
	REG_FUNC(CScriptObjectPlayer,GiveFlashLight);
	REG_FUNC(CScriptObjectPlayer,GiveBinoculars);
	REG_FUNC(CScriptObjectPlayer,IsSwimming);
	REG_FUNC(CScriptObjectPlayer,GetBlindScreenPos);
	REG_FUNC(CScriptObjectPlayer,SetAISpeedMult);
	REG_FUNC(CScriptObjectPlayer,InitDynamicLight);
	REG_FUNC(CScriptObjectPlayer,GetColor);
	REG_FUNC(CScriptObjectPlayer,InitStaminaTable);
	REG_FUNC(CScriptObjectPlayer,SavePlayerElements);
	REG_FUNC(CScriptObjectPlayer,LoadPlayerElements);
	REG_FUNC(CScriptObjectPlayer,GetProjectedBloodPos);
	REG_FUNC(CScriptObjectPlayer,UseLadder);
	REG_FUNC(CScriptObjectPlayer,GetCrosshairState);
	REG_FUNC(CScriptObjectPlayer,ResetCamera);
	REG_FUNC(CScriptObjectPlayer,ResetRotateHead);
	REG_FUNC(CScriptObjectPlayer,CanStand);
	REG_FUNC(CScriptObjectPlayer,SetSmoothInput);

	pSS->SetGlobalValue("BITMASK_PLAYER",BITMASK_PLAYER);
	pSS->SetGlobalValue("BITMASK_WEAPON",BITMASK_WEAPON);	
	pSS->SetGlobalValue("BITMASK_OBJECT",BITMASK_OBJECT);

	AllowPropertiesMapping(pSS);
	//PROPERTIES
	RegisterProperty( "health",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.health));
	RegisterProperty( "max_health",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.maxHealth));	
	RegisterProperty( "armhealth",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.armHealth));	
	RegisterProperty( "leghealth",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.legHealth));	
	RegisterProperty( "armor",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.armor));
	RegisterProperty( "max_armor",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.maxArmor));
	RegisterProperty( "score",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.score));
	RegisterProperty( "deaths",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.deaths));
	RegisterProperty( "speedscale",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_stats.fSpeedScale));
	RegisterProperty( "firing_grenade",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.firing_grenade));
	RegisterProperty( "running",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.running));
	RegisterProperty( "jumping",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.jumping));
	RegisterProperty( "moving",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.moving));
	RegisterProperty( "flying",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.flying));
	RegisterProperty( "landing",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.landing));
	RegisterProperty( "crouching",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.crouch));
	RegisterProperty( "proning",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.prone));
	RegisterProperty( "climbing",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.climbing));
	RegisterProperty( "use_pressed",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.use_pressed));
	RegisterProperty( "first_person",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_bFirstPerson));
	RegisterProperty( "ammo_in_clip",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.ammo_in_clip));
	RegisterProperty( "ammo",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.ammo));
	RegisterProperty( "numofgrenades",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.numofgrenades));
	RegisterProperty( "grenadetype",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.grenadetype));
	RegisterProperty( "dmgFireAccuracy",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_stats.dmgFireAccuracy));
	RegisterProperty( "holding_breath",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.holding_breath));
	RegisterProperty( "melee_attack",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.melee_attack));
	RegisterProperty( "has_flashlight",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.has_flashlight));
	RegisterProperty( "has_binoculars",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.has_binoculars));
	RegisterProperty( "AnimationSystemEnabled",PROPERTY_TYPE_INT,offsetof(CPlayer,m_AnimationSystemEnabled));
	RegisterProperty( "concentration",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.concentration));
	RegisterProperty( "bForceWalk",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.bForceWalk));
	RegisterProperty( "model",PROPERTY_TYPE_STRING,offsetof(CPlayer,m_strModel));
	RegisterProperty( "reloading",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.reloading));
	RegisterProperty( "weapon_busy",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_stats.weapon_busy));
	RegisterProperty( "drawfpweapon",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.drawfpweapon));
	RegisterProperty( "underwater",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_stats.underwater));
	RegisterProperty( "lock_weapon",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.lock_weapon));
	RegisterProperty( "firemode",PROPERTY_TYPE_INT,offsetof(CPlayer,m_stats.firemode));
	RegisterProperty( "canfire",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.canfire));
	RegisterProperty( "accuracy",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_stats.accuracy));
	RegisterProperty( "melee_distance",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_stats.melee_distance));
	RegisterProperty( "aiming",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.aiming));
	RegisterProperty( "fire_rate_busy",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.fire_rate_busy));
	RegisterProperty( "vel",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_stats.fVel));
	RegisterProperty( "blinding_value",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_stats.curBlindingValue));
	RegisterProperty( "firing",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.firing));
	RegisterProperty( "stamina",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_StaminaTable.StaminaHUD));
	RegisterProperty( "breath",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_StaminaTable.BreathHUD));
	RegisterProperty( "modelhidden",PROPERTY_TYPE_BOOL,offsetof(CPlayer,m_stats.bModelHidden));
	RegisterProperty( "SafeMntAngZ",PROPERTY_TYPE_FLOAT,offsetof(CPlayer,m_vSafeAngAtMountedWeapon.z));
}

//////////////////////////////////////////////////////////////////////////
void CScriptObjectPlayer::SetMemberVector( SOP_MEMBER_LUA_TABLES member,const Vec3 &vec )
{
	IScriptObject *pVec = m_memberSO[member];
	pVec->BeginSetGetChain();
	pVec->SetValueChain("x",vec.x);
	pVec->SetValueChain("y",vec.y);
	pVec->SetValueChain("z",vec.z);
	pVec->EndSetGetChain();
}

void CScriptObjectPlayer::SetPlayer(CPlayer *pPlayer)
{
	m_pPlayer=pPlayer;

	if(!EnablePropertiesMapping(pPlayer))
	{
		CryError( "<CryGame> (CScriptObjectPlayer::SetPlayer) failed" );
		return;
	}
	
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectPlayer::IsSwimming(IFunctionHandler *pH)
{
	bool bIsSwimming=m_pPlayer->IsSwimming();	
	return pH->EndFunction(bIsSwimming);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectPlayer::SelectFirstWeapon(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pPlayer->SelectFirstWeapon();
	return pH->EndFunction();
}

int CScriptObjectPlayer::DrawThirdPersonWeapon(IFunctionHandler *pH)
{
	bool bDraw;
	pH->GetParam(1,bDraw);
	m_pPlayer->DrawThirdPersonWeapon(bDraw);
	return pH->EndFunction();
}

int CScriptObjectPlayer::MakeWeaponAvailable(IFunctionHandler *pH)
{
	int nID;
	int iMakeAvail;
	pH->GetParam(1, nID);

	if (pH->GetParamCount() == 2)
		pH->GetParam(2, iMakeAvail);
	else
		iMakeAvail = 1;

	return pH->EndFunction(m_pPlayer->MakeWeaponAvailable(nID, (iMakeAvail == 1)));
}

int CScriptObjectPlayer::GetWeaponInfo(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	if (m_pPlayer->GetSelectedWeaponId() == -1)
		return pH->EndFunctionNull();

	WeaponInfo &wi = m_pPlayer->GetWeaponInfo();

	m_pWeaponInfo->BeginSetGetChain();
	m_pWeaponInfo->SetValue( "max_ammo",wi.maxAmmo );
	m_pWeaponInfo->SetValue( "owns",wi.owns );
	m_pWeaponInfo->SetValue( "reloading",wi.reloading );
	// Only get, script can't change theose.
	m_pWeaponInfo->SetValue( "fire_time",wi.fireTime );
	m_pWeaponInfo->SetValue( "firemode",wi.iFireMode );
	m_pWeaponInfo->EndSetGetChain();

	return pH->EndFunction(m_pWeaponInfo);
}

int CScriptObjectPlayer::GetWeaponsSlots(IFunctionHandler *pH)
{
	m_pWeaponSlots->Clear();
	for(int n=0;n<4;n++)
	{
		if(m_pPlayer->m_vWeaponSlots[n])
		{
			CWeaponClass* pWC=m_pPlayer->GetGame()->GetWeaponSystemEx()->GetWeaponClassByID(m_pPlayer->m_vWeaponSlots[n]);
			if(pWC && pWC->GetScriptObject())m_pWeaponSlots->PushBack(pWC->GetScriptObject());
			else m_pWeaponSlots->PushBack(0);
		}
		else m_pWeaponSlots->PushBack(0);
	}
	return pH->EndFunction(m_pWeaponSlots);
}

int CScriptObjectPlayer::CalculateAccuracyFactor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	float accuracy;
	if (pH->GetParam(1, accuracy))
	{
		return pH->EndFunction(m_pPlayer->CalculateAccuracyFactor(accuracy));
	}

	return pH->EndFunction(0.0f);
}

int CScriptObjectPlayer::WaitForFireRelease(IFunctionHandler *pH)
{
	if(pH->GetParamCount()==1)
	{
		bool bVal;
		if (pH->GetParam(1, bVal))
		{
			m_pPlayer->SetWaitForFireRelease(bVal);
		}
	}
	
	return pH->EndFunction(m_pPlayer->GetWaitForFireRelease());
}

int CScriptObjectPlayer::SetCurrWeapon(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(1);
	int nWeaponIndex;
	
	if(pH->GetParamCount()==2)
	{
		int	id;
		pH->GetParam(1,nWeaponIndex);
		m_pPlayer->SelectWeapon(nWeaponIndex);

		pH->GetParam(2,id);
		IEntity *pEntity = m_pPlayer->GetGame()->GetSystem()->GetIEntitySystem()->GetEntity(id);
		if (pEntity)
		{
			m_pPlayer->GoStand();	// standup - can't use mounted weapon in prone/crouche
			m_pPlayer->m_pMountedWeapon = pEntity;
			m_pPlayer->m_vSafeAngAtMountedWeapon = m_pPlayer->GetEntity()->GetAngles();
		}
		else
			m_pPlayer->m_pMountedWeapon = NULL;
	}
	else if(pH->GetParamCount()==1)
	{
		pH->GetParam(1,nWeaponIndex);
		m_pPlayer->SelectWeapon(nWeaponIndex);
		m_pPlayer->m_pMountedWeapon = NULL;
	}
	else
	{
		m_pPlayer->SelectWeapon( -1 );
		m_pPlayer->m_pMountedWeapon = NULL;
	}

	return pH->EndFunction();
}

int CScriptObjectPlayer::GetCurrWeapon(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	CWeaponClass *weapon = m_pPlayer->GetSelectedWeapon();
	if (!weapon)
		return pH->EndFunctionNull();

	return pH->EndFunction(weapon->GetScriptObject());
}

int CScriptObjectPlayer::SetSwayAmp(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fValue;
	pH->GetParam(1,fValue);
	m_pPlayer->SetSwayAmp(fValue);
	return pH->EndFunction();
}

int CScriptObjectPlayer::SetSwayFreq(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fValue;
	pH->GetParam(1,fValue);
	m_pPlayer->SetSwayFreq(fValue);
	return pH->EndFunction();
}

CPlayer * CScriptObjectPlayer::GetPlayer()
{
	return m_pPlayer;
}

int CScriptObjectPlayer::GetViewIntersection(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	IEntityCamera *pCam=m_pPlayer->GetEntity()->GetCamera();
	if (!pCam)
		return pH->EndFunctionNull(); 
	float fMaxDist=m_pPlayer->GetGame()->GetSystem()->GetI3DEngine()->GetMaxViewDist(); //pCam->GetCamera().GetZMax();

	Vec3 pos,angle;

	if (m_pPlayer->GetVehicle())
	{
		pos=pCam->GetPos();
		angle=pCam->GetAngles();
	}
	else
	{
		//we must always take leaning into account!
		angle = m_pPlayer->m_vEyeAngles+m_pPlayer->m_vShake;
		pos = m_pPlayer->m_vEyePos;
	}

	// [Kirill]- do here same calculations as when finding fireAngles in int CWeaponClass::Fire
	//------------------------------------------------------------------------------------------------
	// Marco's change to take leaning into account:
	// transform the weapons angles using the same as the camera matrix
	// create a vector pointing down the z-axis
	//------------------------------------------------------------------------------------------------
	Vec3d dir(0,-1,0);
	Matrix44 tm = Matrix44::CreateRotationZYX(-angle*gf_DEGTORAD); //NOTE: angles in radians and negated 
	dir = GetTransposed44(tm)*(dir);

//	dir=ConvertToRadAngles(dir);		
	dir*=fMaxDist;
	ray_hit hit;
	float fDist=-1.0;
	if (m_pPlayer->GetGame()->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection(vectorf(pos), vectorf(dir), ent_all,
			13/*pirceability*/, &hit,1, m_pPlayer->GetEntity()->GetPhysics()))
	{
		vectorf vecRay=vectorf(pos)-hit.pt;
		fDist=vecRay.len();
		IEntityCamera *pEC=m_pPlayer->GetEntity()->GetCamera();
		CCamera cam=pEC->GetCamera();
		Vec3 vAngles=cam.GetAngles();
		
		//m_pTempAng->BeginSetGetChain();
		//m_pTempAng->SetValueChain("x",vAngles.x);
		//m_pTempAng->SetValueChain("y",vAngles.y);
		//m_pTempAng->SetValueChain("z",vAngles.z);
		//m_pTempAng->EndSetGetChain();
		//////////////////////////////////////////
		m_pTempObj->BeginSetGetChain(); 
		m_pTempObj->SetValueChain("x",hit.pt.x);
		m_pTempObj->SetValueChain("y",hit.pt.y);
		m_pTempObj->SetValueChain("z",hit.pt.z);
		m_pTempObj->SetValueChain("angles",m_pTempAng);
		m_pTempObj->SetValueChain("len",fDist);

		float fWinX,fWinY,fWinZ;
		IRenderer *pRend=m_pPlayer->GetGame()->GetSystem()->GetIRenderer();
		pRend->ProjectToScreen(hit.pt.x,hit.pt.y,hit.pt.z,&fWinX,&fWinY,&fWinZ);

		// silly renderer values scaled by 100
		fWinX=fWinX*pRend->GetWidth()/100.0f;
		fWinY=fWinY*pRend->GetHeight()/100.0f;

		m_pTempObj->SetValueChain("winx",fWinX);
		m_pTempObj->SetValueChain("winy",fWinY);

		m_pTempObj->SetToNullChain("id");
		m_pTempObj->SetToNullChain("ent");
		//check if is an entity
		if(hit.pCollider){
			IEntity *pE=(IEntity *)hit.pCollider->GetForeignData();
			if(pE){
				m_pTempObj->SetValueChain("id",pE->GetId());
				IScriptObject *p=pE->GetScriptObject();
				if(p)
					m_pTempObj->SetValueChain("ent",p);
			}
		}
		
		
		m_pTempObj->EndSetGetChain();
		return pH->EndFunction(m_pTempObj);
	}
	return pH->EndFunctionNull();
}

/*!sets the gravity applyed to the player
	@param fGravity the gravity
*/
int CScriptObjectPlayer::SetGravity(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fGravity;
	pH->GetParam(1, fGravity);
	m_pPlayer->SetGravityOverride(fGravity);
	return pH->EndFunction();
}

int CScriptObjectPlayer::SetAngleLimit(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fLimit;
	pH->GetParam(1,fLimit);
	if(fLimit>0)
	{
		m_pPlayer->SetMinAngleLimitH(-fLimit);
		m_pPlayer->SetMaxAngleLimitH(fLimit);	
		m_pPlayer->SetMinAngleLimitV(-fLimit);
		m_pPlayer->SetMaxAngleLimitV(fLimit);	
		m_pPlayer->SetAngleLimitBaseOnCamera();
		m_pPlayer->EnableAngleLimitH(1);
		m_pPlayer->EnableAngleLimitV(1);
	}
	else
	{
		m_pPlayer->EnableAngleLimitH(0);
		m_pPlayer->EnableAngleLimitV(0);
	}
//	m_pPlayer->SetAngleLimit(fLimit);
	return pH->EndFunction();
}

int CScriptObjectPlayer::SetAngleLimitH(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fLimit;
	pH->GetParam(1,fLimit);
	if(fLimit>0)
	{
		m_pPlayer->EnableAngleLimitH(1);
		m_pPlayer->SetMinAngleLimitH(-fLimit);
		m_pPlayer->SetMaxAngleLimitH(fLimit);	
		m_pPlayer->SetAngleLimitBaseOnCamera();
	}		
	else
		m_pPlayer->EnableAngleLimitH(0);
//	m_pPlayer->SetAngleLimitH(fLimit);
	return pH->EndFunction();
}

int CScriptObjectPlayer::SetAngleLimitV(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fLimit;
	pH->GetParam(1,fLimit);
	if(fLimit>0)
	{
		m_pPlayer->EnableAngleLimitV(1);
		m_pPlayer->SetMinAngleLimitV(-fLimit);
		m_pPlayer->SetMaxAngleLimitV(fLimit);	
		m_pPlayer->SetAngleLimitBaseOnCamera();
	}
	else
		m_pPlayer->EnableAngleLimitH(0);
	return pH->EndFunction();
}

int CScriptObjectPlayer::GetAngleLimitH(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	float flag = m_pPlayer->IsAngleLimitHOn();
	return pH->EndFunction(flag);
}

int CScriptObjectPlayer::GetAngleLimitV(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	bool flag = m_pPlayer->IsAngleLimitVOn();
	return pH->EndFunction(flag);

}

int CScriptObjectPlayer::SetAngleLimitBase(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(0);
	if (pH->GetParamCount() == 1)
	{
		Vec3 base;
		CScriptObjectVector oVec(m_pScriptSystem,true);
		pH->GetParam(1,*oVec);
		base=oVec.Get();
		m_pPlayer->SetAngleLimitBase( base );
	}
	else
		m_pPlayer->SetAngleLimitBase( );
	return pH->EndFunction();
}



int CScriptObjectPlayer::SetMinAngleLimitV(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fLimit;
	pH->GetParam(1,fLimit);
	m_pPlayer->SetMinAngleLimitV(fLimit);
	return pH->EndFunction();
}

int CScriptObjectPlayer::SetMaxAngleLimitV(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fLimit;
	pH->GetParam(1,fLimit);
	m_pPlayer->SetMaxAngleLimitV(fLimit);
	return pH->EndFunction();
}

int CScriptObjectPlayer::EnableAngleLimitV(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	bool bLimit;
	pH->GetParam(1,bLimit);
	m_pPlayer->EnableAngleLimitV(bLimit);
	return pH->EndFunction();
}

int CScriptObjectPlayer::SetMinAngleLimitH(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fLimit;
	pH->GetParam(1,fLimit);
	m_pPlayer->SetMinAngleLimitH(fLimit);
	return pH->EndFunction();
}

int CScriptObjectPlayer::SetMaxAngleLimitH(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	float fLimit;
	pH->GetParam(1,fLimit);
	m_pPlayer->SetMaxAngleLimitH(fLimit);
	return pH->EndFunction();
}

int CScriptObjectPlayer::EnableAngleLimitH(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	bool bLimit;
	pH->GetParam(1,bLimit);
	m_pPlayer->EnableAngleLimitH(bLimit);
	return pH->EndFunction();
}

int CScriptObjectPlayer::SetName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	char *szName;

	if (pH->GetParam(1, szName))
	{
		m_pPlayer->SetName(szName);
	}

	return pH->EndFunction();
}

int CScriptObjectPlayer::GetName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction(m_pPlayer->GetName());
}

int CScriptObjectPlayer::InitWeapons(IFunctionHandler *pH)
{
	m_pPlayer->InitWeapons();
	
	return pH->EndFunction();
}

int CScriptObjectPlayer::GetCurrWeaponId(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pPlayer->GetSelectedWeaponId());
}

int CScriptObjectPlayer::CalcDmgShakeAxis(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pTable(m_pScriptSystem, true);
	pH->GetParam(1, *pTable);
	Vec3 ProjDir;
	pTable->GetValue("x", ProjDir.x);
	pTable->GetValue("y", ProjDir.y);
	pTable->GetValue("z", ProjDir.z);
	ProjDir.Normalize();
	Vec3 PlayerDir=m_pPlayer->GetWalkParams().dir;
	PlayerDir.z=0.0f;
	PlayerDir.Normalize();
	Vec3 Top(0.0f, 0.0f, 1.0f);
	Vec3 Tangent=PlayerDir.Cross(Top);
	float fDot1=ProjDir.Dot(PlayerDir);
	float fDot2=ProjDir.Dot(Tangent);
	Vec3 Result(fDot1, fDot2, 0);

	SetMemberVector( SOP_MEMBER_SHAKE_AXIS,Vec3(Result.x,-Result.y,Result.z) );
	return pH->EndFunction( m_memberSO[SOP_MEMBER_SHAKE_AXIS] );
}

int CScriptObjectPlayer::ShakeCamera(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);
	_SmartScriptObject pTable(m_pScriptSystem, true);
	pH->GetParam(1, *pTable);
	Vec3 Axis;
	pTable->GetValue("x", Axis.x);
	pTable->GetValue("y", Axis.y);
	pTable->GetValue("z", Axis.z);
	float fDeg, fFreq, fTime;
	pH->GetParam(2, fDeg);
	pH->GetParam(3, fFreq);
	pH->GetParam(4, fTime);

	if (m_pPlayer->m_bFirstPerson)
		m_pPlayer->SetShake(Axis, fDeg, fFreq, fTime);

	return pH->EndFunction();
}

int CScriptObjectPlayer::SetCameraOffset(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	
	_SmartScriptObject pTable(m_pScriptSystem, true);
	pH->GetParam(1, *pTable);
	
	Vec3 Offset;
	pTable->GetValue("x", Offset.x );
	pTable->GetValue("y", Offset.y );
	pTable->GetValue("z", Offset.z );

	m_pPlayer->SetCameraOffset(Offset);
	return pH->EndFunction();
}

int CScriptObjectPlayer::GetCameraOffset(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	Vec3 Offset;
	m_pPlayer->GetCameraOffset(Offset);
	m_pCameraOffset.Set( Offset );
	return pH->EndFunction(m_pCameraOffset);
}


int CScriptObjectPlayer::GetColor(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	Vec3 Color = m_pPlayer->GetColor();
	m_pGetColor.Set( Color );
	
	return pH->EndFunction(m_pGetColor);
}

int CScriptObjectPlayer::DeselectWeapon(IFunctionHandler *pH)
{
	CHECK_PARAMETERS( 0);

	CWeaponClass *pDeselected = m_pPlayer->DeselectWeapon();

	if (pDeselected)
		return pH->EndFunction(pDeselected->GetScriptObject());
	else
		return pH->EndFunctionNull();
}

int CScriptObjectPlayer::RedirectInputTo(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(1);

	ASSERT(pH->GetParamCount() == 1 || pH->GetParamCount() == 2);

	int id;
	pH->GetParam(1,id);
	int	angleDelta;
	if(pH->GetParamCount() == 2)
		pH->GetParam(2,angleDelta);
	else
		angleDelta = -1;
	

	m_pPlayer->RedirectInputToEntity(id, angleDelta);

	return pH->EndFunction();
}


int CScriptObjectPlayer::StartDie(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);

	Vec3 impuls;
	_SmartScriptObject pTable(m_pScriptSystem, true);
	if(!pH->GetParam(1, *pTable))
		{ CryError("CScriptObjectPlayer::StartDie parameter 1 failed");return pH->EndFunction(); }

	pTable->GetValue("x", impuls.x);
	pTable->GetValue("y", impuls.y);
	pTable->GetValue("z", impuls.z);

	Vec3 point;

	if(!pH->GetParam(2, *pTable))
		{ CryError("CScriptObjectPlayer::StartDie parameter 2 failed");return pH->EndFunction(); }
	
	pTable->GetValue("x", point.x);
	pTable->GetValue("y", point.y);
	pTable->GetValue("z", point.z);

	int	partid,deathType;

	pH->GetParam(3, partid);
	pH->GetParam(4, deathType);

	m_pPlayer->StartDie( impuls, point, partid, deathType );

	return pH->EndFunction();
}

int CScriptObjectPlayer::SetDimOverride(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	pH->GetParam(1,*pObj);
	pe_player_dimensions	dim;
	pObj->GetValue("eye_height",dim.heightEye);
	pObj->GetValue("ellipsoid_height",dim.heightCollider);
	pObj->GetValue("x",dim.sizeCollider.x);
	pObj->GetValue("y",dim.sizeCollider.y);
	pObj->GetValue("z",dim.sizeCollider.z); 
	dim.headRadius = 0;
	dim.heightHead = dim.heightCollider;
	pObj->GetValue("head_height", dim.heightHead);
	pObj->GetValue("head_radius", dim.headRadius);

	IPhysicalEntity*	phys = m_pPlayer->GetEntity()->GetPhysics();
	if ( phys )		
	{
		// Use normal physics dimensions.
//		if(phys->SetParams( &dim ))
		phys->SetParams( &dim );
	}

	return pH->EndFunction();
}


int CScriptObjectPlayer::SetDimNormal(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	pH->GetParam(1,*pObj);
	pe_player_dimensions	dim;
	pObj->GetValue("eye_height",dim.heightEye);
	pObj->GetValue("ellipsoid_height",dim.heightCollider);
	pObj->GetValue("x",dim.sizeCollider.x);
	pObj->GetValue("y",dim.sizeCollider.y);
	pObj->GetValue("z",dim.sizeCollider.z); 
	dim.headRadius = 0;
	dim.heightHead = dim.heightCollider;
	pObj->GetValue("head_height", dim.heightHead);
	pObj->GetValue("head_radius", dim.headRadius);

	m_pPlayer->SetDimNormal(&dim);
	return pH->EndFunction();
}


int CScriptObjectPlayer::SetDimCrouch(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	pH->GetParam(1,*pObj);
	pe_player_dimensions	dim;
	pObj->GetValue("eye_height",dim.heightEye);
	pObj->GetValue("ellipsoid_height",dim.heightCollider);
	pObj->GetValue("x",dim.sizeCollider.x);
	pObj->GetValue("y",dim.sizeCollider.y);
	pObj->GetValue("z",dim.sizeCollider.z);
	dim.headRadius = 0;
	dim.heightHead = dim.heightCollider;
	pObj->GetValue("head_height", dim.heightHead);
	pObj->GetValue("head_radius", dim.headRadius);

	m_pPlayer->SetDimCrouch(&dim);
	return pH->EndFunction();
}


int CScriptObjectPlayer::SetDimProne(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	pH->GetParam(1,*pObj);
	pe_player_dimensions	dim;
	pObj->GetValue("eye_height",dim.heightEye);
	pObj->GetValue("ellipsoid_height",dim.heightCollider);
	pObj->GetValue("x",dim.sizeCollider.x);
	pObj->GetValue("y",dim.sizeCollider.y);
	pObj->GetValue("z",dim.sizeCollider.z);
	dim.headRadius = 0;
	dim.heightHead = dim.heightCollider;
	pObj->GetValue("head_height", dim.heightHead);
	pObj->GetValue("head_radius", dim.headRadius);

	m_pPlayer->SetDimProne(&dim);
	return pH->EndFunction();
}



/*!returns damage zone for the bone boneIdx
	@param bIdx bone's index
		DMG_HEAD		1
		DMG_TORSO		2
		DMG_ARM			3
		DMG_LEG			4
		DMG_DEFAULT	2
*/
int CScriptObjectPlayer::GetBoneHitZone(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int bIdx;
	pH->GetParam(1,bIdx);

	return pH->EndFunction( m_pPlayer->GetBoneHitZone( bIdx ) );
}


//
//-----------------------------------------------------------------------
/*! return arm damage in percentage
	@return arm damage in percentage
*/
int CScriptObjectPlayer::GetArmDamage(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction( m_pPlayer->GetArmDamage( ) );
}


//
//-----------------------------------------------------------------------

/*! return leg damage in percentage
	@return arm damage in percentage
*/

int CScriptObjectPlayer::GetLegDamage(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	return pH->EndFunction( m_pPlayer->GetLegDamage( ) );
}


int CScriptObjectPlayer::SetMoveParams(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	if(pH->GetParam(1,pObj))
	{
		pObj->BeginSetGetChain();
		//normal speeds
		float fSpeedRun=0;
		float fSpeedWalk=0;
		float fSpeedCrouch=0;
		float fSpeedProne=0;
		float fSpeedSwim=0;
		// strafe speeds
		float fSpeedRunStrafe=0;
		float fSpeedWalkStrafe=0;
		float fSpeedCrouchStrafe=0;
		float fSpeedProneStrafe=0;
		float fSpeedSwimStrafe=0;
		//
		float fJumpForce=0;
		float fLeanAngle=0;
		float fBobPitch=0;
		float fBobRoll=0;
		float fBobLenght=0;
		float fBobWeapon=0;
		pObj->GetValueChain("speed_run",fSpeedRun);
		m_pPlayer->SetRunSpeed(fSpeedRun);
		fSpeedRunStrafe = fSpeedRun;
		pObj->GetValueChain("speed_walk",fSpeedWalk);
		m_pPlayer->SetWalkSpeed(fSpeedWalk);
		fSpeedWalkStrafe = fSpeedWalk;
		pObj->GetValueChain("speed_crouch",fSpeedCrouch);
		m_pPlayer->SetCrouchSpeed(fSpeedCrouch);
		fSpeedCrouchStrafe = fSpeedCrouch;
		pObj->GetValueChain("speed_prone",fSpeedProne);
		m_pPlayer->SetProneSpeed(fSpeedProne);
		fSpeedProneStrafe = fSpeedProne;

		if(pObj->GetValueChain("speed_swim",fSpeedSwim))
			m_pPlayer->SetSwimSpeed(fSpeedSwim);
		else
			m_pPlayer->SetSwimSpeed(fSpeedWalk);
		fSpeedSwimStrafe = fSpeedSwim;

		m_pPlayer->m_StealthSpeed = (m_pPlayer->m_CrouchSpeed + m_pPlayer->m_WalkSpeed)*.5f;

		// getting strafe speeds
		pObj->GetValueChain("speed_run_strafe",fSpeedRunStrafe);
		pObj->GetValueChain("speed_walk_strafe",fSpeedWalkStrafe);
		pObj->GetValueChain("speed_crouch_strafe",fSpeedCrouchStrafe);
		pObj->GetValueChain("speed_prone_strafe",fSpeedProneStrafe);
		pObj->GetValueChain("speed_swim_strafe",fSpeedSwimStrafe);
		m_pPlayer->m_RunSpeedStrafe = fSpeedRunStrafe;
		m_pPlayer->m_WalkSpeedStrafe = fSpeedWalkStrafe;
		m_pPlayer->m_CrouchSpeedStrafe = fSpeedCrouchStrafe;
		m_pPlayer->m_ProneSpeedStrafe = fSpeedProneStrafe;
		m_pPlayer->m_SwimSpeedStrafe = fSpeedSwimStrafe;
		m_pPlayer->m_StealthSpeedStrafe = (fSpeedWalkStrafe+fSpeedCrouchStrafe)*.5f ;

		// back speeds
		m_pPlayer->m_RunSpeedBack = fSpeedRun;
		m_pPlayer->m_WalkSpeedBack = fSpeedWalk;
		m_pPlayer->m_CrouchSpeedBack = fSpeedCrouch;
		m_pPlayer->m_ProneSpeedBack = fSpeedProne;
		m_pPlayer->m_SwimSpeedBack = fSpeedSwim;
		m_pPlayer->m_StealthSpeedBack = (fSpeedWalk+fSpeedCrouch)*.5f ;
		pObj->GetValueChain("speed_run_back",m_pPlayer->m_RunSpeedBack);
		pObj->GetValueChain("speed_walk_back",m_pPlayer->m_WalkSpeedBack);
		pObj->GetValueChain("speed_crouch_back",m_pPlayer->m_CrouchSpeedBack);
		pObj->GetValueChain("speed_prone_back",m_pPlayer->m_ProneSpeedBack);
		pObj->GetValueChain("speed_swim_back",m_pPlayer->m_SwimSpeedBack);

		pObj->GetValueChain("jump_force",fJumpForce);
		m_pPlayer->SetJumpForce(fJumpForce);
		pObj->GetValueChain("lean_angle",fLeanAngle);
		m_pPlayer->SetLean(fLeanAngle);
		pObj->GetValueChain("bob_pitch",fBobPitch);
		pObj->GetValueChain("bob_roll",fBobRoll);
		pObj->GetValueChain("bob_lenght",fBobLenght);
		m_pPlayer->SetCameraBob(fBobPitch, fBobRoll, fBobLenght);
		pObj->GetValueChain("bob_weapon",fBobWeapon);
		m_pPlayer->SetWeaponBob(fBobWeapon);
		pObj->EndSetGetChain();
	}
	return pH->EndFunction();
}


//
//-----------------------------------------------------------------------
int CScriptObjectPlayer::HasCollided(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pPlayer->HasCollided());
}
//
//-----------------------------------------------------------------------


int CScriptObjectPlayer::SetDynamicsProperties(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pTable(m_pScriptSystem,true);
	pH->GetParam(1,*pTable);

	PlayerDynamics dyn;
	pTable->GetValue("air_control", dyn.air_control);
	pTable->GetValue("inertia", dyn.inertia);
	pTable->GetValue("swimming_inertia", dyn.swimming_inertia);
	pTable->GetValue("gravity", dyn.gravity);
	pTable->GetValue("swimming_gravity", dyn.swimming_gravity);
	pTable->GetValue("jump_gravity", dyn.jump_gravity);

	m_pPlayer->SetDynamics(&dyn);

	pe_player_dynamics pd;
	pTable->GetValue("min_slide_angle", pd.minSlideAngle);
	pTable->GetValue("max_climb_angle", pd.maxClimbAngle);
	pTable->GetValue("max_jump_angle", pd.maxJumpAngle);
	pTable->GetValue("min_fall_angle", pd.minFallAngle);
	pTable->GetValue("nod_speed", pd.nodSpeed);

	if(m_pPlayer->m_pGame->IsMultiplayer())
	{
		pd.bNetwork=true;
	}
	IPhysicalEntity *pPhys = m_pPlayer->GetEntity()->GetPhysics();
	if (pPhys)
		pPhys->SetParams(&pd);
	
	int bPushPlayers=1,bPushableByPlayers=1;
	if  ( pTable->GetValue("push_players", bPushPlayers) &&
		  pTable->GetValue("pushable_by_players", bPushableByPlayers) )
	{
		pe_params_flags ppf;
		if (bPushableByPlayers)
			ppf.flagsOR  = pef_pushable_by_players;
		else
			ppf.flagsAND =~pef_pushable_by_players;
		if (bPushPlayers)
			ppf.flagsOR|= lef_push_players;
		else
			ppf.flagsAND&=~lef_push_players;

		if (pPhys)
			pPhys->SetParams(&ppf);
	}

	return pH->EndFunction();
}

int CScriptObjectPlayer::GetTreadedOnMaterial(IFunctionHandler *pH)
{
	IPhysicalEntity *pe;
	IEntity *pEntity=m_pPlayer->GetEntity();
	CXGame *pGame=m_pPlayer->GetGame();
  if(pEntity)
	{
		pEntity=m_pPlayer->GetEntity();
		pe=pEntity->GetPhysics();
		if(pe)
		{
			if (pe->GetType()==PE_LIVING)
			{
				pe_status_living status;
				if(pe->GetStatus(&status))
				{
					if( !status.bFlying )
						m_LastTouchedMaterialID = status.groundSurfaceIdx;

					if( m_pPlayer->m_FlyTime<.25f || m_pPlayer->m_stats.landing)// if in the air for less than .1 sec - get material
					{
						IScriptObject *pObj=pGame->m_XSurfaceMgr.GetMaterialBySurfaceID(m_LastTouchedMaterialID);
						if(pObj)
							return pH->EndFunction(pObj);
					}
				}
			}
			else
			{
				coll_history_item	item;
				pe_status_collisions col;
				col.pHistory = &item;
				col.age = 0.5f;
				if (pe->GetStatus(&col))
				{
					IScriptObject *pObj=pGame->m_XSurfaceMgr.GetMaterialBySurfaceID(item.idmat[1]);
					if(pObj)
						return pH->EndFunction(pObj);
				}
			}
		}
	}
	return pH->EndFunctionNull();
}

int CScriptObjectPlayer::GetTouchedMaterial(IFunctionHandler *pH)
{
	IPhysicalEntity *pe;
	IEntity *pEntity=m_pPlayer->GetEntity();
	CXGame *pGame=m_pPlayer->GetGame();

	if(pEntity)
	{
		pEntity = m_pPlayer->GetEntity();
		pe = pEntity->GetPhysics();
		pe_status_pos sp;
		pe_status_living sl;
		ray_hit hit;
		IPhysicalWorld *pWorld = m_pPlayer->GetGame()->GetSystem()->GetIPhysicalWorld();
		IPhysicalEntity **pEnts;
		int nEnts;

		if (pe->GetStatus(&sp) && pe->GetStatus(&sl) && (nEnts = pWorld->GetEntitiesInBox(sp.pos+sp.BBox[0],sp.pos+sp.BBox[1], pEnts, ent_static)))
		{
			for(nEnts--; nEnts>=0; nEnts--) if (pEnts[nEnts]!=pe)
			{
				// [lennert] for bush sounds this doesnt work since the bush itself isnt physicalized so we never collide...
//				pEnts[nEnts]->GetStatus(&sp);
//				if (!(sp.flagsOR & geom_collides))
//				{
					if (pWorld->RayTraceEntity(pEnts[nEnts], sp.pos+sl.camOffset,sl.camOffset*-1.4f, &hit))
					{
						IScriptObject *pObj=pGame->m_XSurfaceMgr.GetMaterialBySurfaceID(hit.surface_idx);
						if(pObj)
							return pH->EndFunction(pObj);
					}
//				}
			}
		}
	}

	return pH->EndFunctionNull();
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectPlayer::GetTPVHelper(IFunctionHandler *pH)
{
	const char *pszName = NULL;
	ICryCharInstance *pInstance = NULL; 
	int iPos;
	Vec3 vHelperPos;

	CHECK_PARAMETERS(2);

	// Character index and helper name
	pH->GetParam(1, iPos);
	pH->GetParam(2, pszName);

	bool	bVehicleWeapon = false;
	if(m_pPlayer->m_pVehicle)
	{
//		if(!(m_pPlayer->m_pVehicle->GetWeaponName( m_pPlayer->m_stats.inVehicleState)) .empty())
		if(m_pPlayer->m_pVehicle->GetWeaponName( m_pPlayer->m_stats.inVehicleState) != CVehicle::m_sNoWeaponName)
			bVehicleWeapon = true;

	}

	// Get position of helper
	if( bVehicleWeapon )	
	{
		IEntityCharacter *pIChar = m_pPlayer->m_pVehicle->GetEntity()->GetCharInterface();
		ICryCharInstance * cmodel = pIChar->GetCharacter(0);    

			if (!cmodel) 
				return pH->EndFunctionNull();

			ICryBone * pBone = cmodel->GetBoneByName(pszName);
			if(!pBone)
			{
			//    m_pSystem->GetILog()->Log("ERROR: CScriptObjectPlayer::GetTPVHelper: Bone not found: %s", pszName);
				return pH->EndFunctionNull();
			}
			Vec3 vBonePos;
			Vec3 angles;
			m_pPlayer->m_pVehicle->GetFirePosAngles(vBonePos, angles);

			m_pPlayer->m_pVehicle->GetEntity()->GetHelperPosition("gun", vBonePos);

			Matrix44 m=Matrix34::CreateRotationXYZ( Deg2Rad(angles), vBonePos );	//set rotation and translation in one function call
			m	=	GetTransposed44(m); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

			vBonePos = pBone->GetBonePosition();
			// get result
			vHelperPos = m.TransformPointOLD(vBonePos);
	}
	else
	if( m_pPlayer->m_pMountedWeapon )	// if at mounted weapon - get position of bone from weapon character
	{
//		m_pPlayer->m_pMountedWeapon->GetHelperPosition(pszName, vHelperPos);
//		oVec.Set(vHelperPos);

		IEntityCharacter *pIChar = m_pPlayer->m_pMountedWeapon->GetCharInterface();
		ICryCharInstance * cmodel = pIChar->GetCharacter(0);    

			if (!cmodel) 
				return pH->EndFunctionNull();

			ICryBone * pBone = cmodel->GetBoneByName(pszName);
			if(!pBone)
			{
			//    m_pSystem->GetILog()->Log("ERROR: CScriptObjectPlayer::GetTPVHelper: Bone not found: %s", pszName);
				return pH->EndFunctionNull();
			}

			Vec3 vBonePos = pBone->GetBonePosition();
			Vec3 angles = m_pPlayer->m_pMountedWeapon->GetAngles();

			Matrix44 m=Matrix34::CreateRotationXYZ( Deg2Rad(angles), m_pPlayer->m_pMountedWeapon->GetPos() );	//set rotation and translation in one function call
			m	=	GetTransposed44(m); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

			// get result
			vHelperPos = m.TransformPointOLD(vBonePos);
	}
	else
	{
		pInstance = m_pPlayer->GetEntity()->GetCharInterface()->GetCharacter(iPos);
		if (pInstance)
		{
			Matrix44 m;
			if(!pInstance->IsBindingValid(m_pPlayer->GetWeaponInfo().hBindInfo))
				return pH->EndFunctionNull();

			vHelperPos = pInstance->GetTPVWeaponHelper(pszName,m_pPlayer->GetWeaponInfo().hBindInfo);
			m.SetIdentity();
			Vec3 ang=m_pPlayer->m_vCharacterAngles;
			
			//m.RotateZ(DEG2RAD(-ang.z));
			m=m*Matrix33::CreateRotationZ(DEG2RAD(-ang.z)); 

			vHelperPos=m.TransformPointOLD(vHelperPos);

			vHelperPos=m_pPlayer->GetEntity()->GetPos()+vHelperPos;
		}
		else
			return pH->EndFunctionNull();
	}

	SetMemberVector( SOP_MEMBER_TV_HELPER,vHelperPos );
	return pH->EndFunction( m_memberSO[SOP_MEMBER_TV_HELPER] );
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectPlayer::GetHelperPos(IFunctionHandler *pH)
{
	ASSERT(pH->GetParamCount() == 1 || pH->GetParamCount() == 2);

	const char *helper;
	bool bUseObjectSpace =	false;

	pH->GetParam(1, helper);

	if(pH->GetParamCount() == 2)
	{
		pH->GetParam(2, bUseObjectSpace);
	}

	Vec3 pos;
	pos(0,0,0);

	m_pPlayer->GetEntity()->GetHelperPosition(helper, pos, bUseObjectSpace);
	pos = pos - m_pPlayer->GetEntity()->GetPos();

	Vec3 rot = m_pPlayer->GetActualAngles();
	rot.x = 0;
	rot.z = m_pPlayer->m_LegAngle - rot.z;
	Matrix33 legMatrix = Matrix33::CreateRotationXYZ( Deg2Rad(rot));

	pos = m_pPlayer->GetEntity()->GetPos() + legMatrix * pos;

	SetMemberVector(SOP_MEMBER_TV_HELPER, pos);
	return pH->EndFunction(m_memberSO[SOP_MEMBER_TV_HELPER]);
}

int	CScriptObjectPlayer::GetCharacterAngles(IFunctionHandler *pH)
{
	ICryCharInstance *pInstance = NULL; 
	Vec3 vec;
	vec=m_pPlayer->m_vCharacterAngles;
	vec=ConvertToRadAngles(vec);
	m_pTempAng->SetValue("x",vec.x);
	m_pTempAng->SetValue("y",vec.y);
	m_pTempAng->SetValue("z",vec.z);
	return pH->EndFunction(m_pTempAng);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectPlayer::ShakeCameraL(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	_SmartScriptObject pTable(m_pScriptSystem, true);
float	ampl, freq, time;
	pH->GetParam(1, ampl);
	pH->GetParam(2, freq);
	pH->GetParam(3, time);
	Vec3	vAmpl=Vec3(ampl, ampl, ampl);
	Vec3	vFreq=Vec3(freq, freq, freq);

	if (m_pPlayer->m_bFirstPerson)//apply shake only in firstperson
		m_pPlayer->SetShakeL2(vAmpl, vFreq, time);

//	m_pPlayer->SetShakeL(vAmpl, vFreq, time);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
//	for debug/test purposes
int CScriptObjectPlayer::StartFire(IFunctionHandler *pH)
{
	m_pPlayer->StartFire();
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectPlayer::PlaySound(IFunctionHandler *pH)
{
	int nCookie=0;
	float fVolumeScale=1.0f;
	USER_DATA npSound = 0;

	pH->GetParamUDVal(1,npSound,nCookie);
	ISound *pSound=(ISound*)npSound;

	if(pSound && (nCookie==USER_DATA_SOUND))
	{
		if(pH->GetParamCount()>=2)
		{
			if(!pH->GetParam(2,fVolumeScale))
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
		m_pPlayer->GetEntity()->PlaySound(pSound, fVolumeScale, Offset);
	}
	else
	{
		m_pScriptSystem->RaiseError("PlaySound NULL SOUND!!");
	}
	
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectPlayer::GetFirePosAngles(IFunctionHandler *pH)
{
	CScriptObjectVector pPos(m_pScriptSystem,true);
	CScriptObjectVector pAng(m_pScriptSystem,true);
	Vec3 tpos,tang;
	if(!pH->GetParam(1,pPos))return pH->EndFunctionNull();
	if(!pH->GetParam(2,pAng))return pH->EndFunctionNull();
	m_pPlayer->GetFirePosAngles(tpos,tang);
	pPos=tpos;
	pAng=tang;

	// also get direction vector
	if(pH->GetParamCount() == 3)
	{
		CScriptObjectVector pDir(m_pScriptSystem,true);
		if (pH->GetParam(3,pDir))
		{
			Vec3d dir=tang;
			dir=ConvertToRadAngles(dir);	
			pDir = dir;
		}
		else
			pH->EndFunctionNull();
	}

	return pH->EndFunction(true);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectPlayer::GetCurVehicle(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	CVehicle* vehicle = m_pPlayer->GetVehicle();
	if (!vehicle)
		return pH->EndFunctionNull();
	IEntity *pEntity = vehicle->GetEntity();
	return pH->EndFunction(pEntity->GetScriptObject());
}

/*
int CScriptObjectPlayer::SetAnimationRefSpeed(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);
	float	sRun, sWalk, sCrouch, sProne;
	pH->GetParam(1,sRun);
	pH->GetParam(2,sWalk);
	pH->GetParam(3,sCrouch);
	pH->GetParam(4,sProne);

	m_pPlayer->SetAnimationRefSpeed( sRun, sWalk, sCrouch, sProne );

	return pH->EndFunction();
}

/*/
int CScriptObjectPlayer::SetAnimationRefSpeedRun(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	float	sFwd, sSide, sBack;
	pH->GetParam(1,sFwd);
	pH->GetParam(2,sSide);
	pH->GetParam(3,sBack);

	m_pPlayer->SetAnimationRefSpeedRun( sFwd, sSide, sBack );

	return pH->EndFunction();
}

int CScriptObjectPlayer::SetAnimationRefSpeedWalk(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	float	sFwd, sSide, sBack;
	pH->GetParam(1,sFwd);
	pH->GetParam(2,sSide);
	pH->GetParam(3,sBack);

	m_pPlayer->SetAnimationRefSpeedWalk( sFwd, sSide, sBack );

	return pH->EndFunction();
}

int CScriptObjectPlayer::SetAnimationRefSpeedWalkRelaxed(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	float	sFwd, sSide, sBack;
	pH->GetParam(1,sFwd);
	pH->GetParam(2,sSide);
	pH->GetParam(3,sBack);

	m_pPlayer->SetAnimationRefSpeedWalkRelaxed( sFwd, sSide, sBack );

	return pH->EndFunction();
}


int CScriptObjectPlayer::SetAnimationRefSpeedXWalk(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	float	sFwd, sSide, sBack;
	pH->GetParam(1,sFwd);
	pH->GetParam(2,sSide);
	pH->GetParam(3,sBack);

	m_pPlayer->SetAnimationRefSpeedXWalk( sFwd, sSide, sBack );

	return pH->EndFunction();
}

int CScriptObjectPlayer::SetAnimationRefSpeedXRun(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	float	sFwd=-1.0f, sSide=-1.0f, sBack=-1.0f;
	pH->GetParam(1,sFwd);
	pH->GetParam(2,sSide);
	pH->GetParam(3,sBack);

	m_pPlayer->SetAnimationRefSpeedXRun( sFwd, sSide, sBack );

	return pH->EndFunction();
}


int CScriptObjectPlayer::SetAnimationRefSpeedCrouch(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);
	float	sFwd, sSide, sBack;
	pH->GetParam(1,sFwd);
	pH->GetParam(2,sSide);
	pH->GetParam(3,sBack);

	m_pPlayer->SetAnimationRefSpeedCrouch( sFwd, sSide, sBack );

	return pH->EndFunction();
}

//*/
int CScriptObjectPlayer::SelectNextWeapon(IFunctionHandler * pH)
{
	m_pPlayer->SelectNextWeapon();
	return pH->EndFunction();
}


//
//	name, timeScale
int CScriptObjectPlayer::CounterAdd(IFunctionHandler *pH)
{
CHECK_PARAMETERS(2);
	const char *name;
	float	timeScale;

	pH->GetParam(1, name);
	pH->GetParam(2, timeScale);
	m_pPlayer->CounterAdd( name, timeScale );
	return pH->EndFunction();
}

int CScriptObjectPlayer::CounterIncrement(IFunctionHandler *pH)
{
CHECK_PARAMETERS(2);
	const char *name;
	float	delta;

	pH->GetParam(1, name);
	pH->GetParam(2, delta);
	m_pPlayer->CounterIncrement( name, delta );
	return pH->EndFunction();
}

int CScriptObjectPlayer::CounterGetValue(IFunctionHandler *pH)
{
CHECK_PARAMETERS(1);
	const char *name;

	pH->GetParam(1, name);
	return pH->EndFunction( m_pPlayer->CounterGetValue( name ) );
}

int CScriptObjectPlayer::CounterSetValue(IFunctionHandler *pH)
{
CHECK_PARAMETERS(2);
	const char *name;
	float	value;

	pH->GetParam(1, name);
	pH->GetParam(2, value);
	m_pPlayer->CounterSetValue( name, value );
	return pH->EndFunction( );
}

int CScriptObjectPlayer::CounterSetEvent(IFunctionHandler *pH)
{
CHECK_PARAMETERS(3);
	const char *name;
	const char *eventName;
	float	value;

	pH->GetParam(1, name);
	pH->GetParam(2, value);
	pH->GetParam(3, eventName);
	m_pPlayer->CounterSetEvent( name, value, eventName );
	return pH->EndFunction( );
}

int CScriptObjectPlayer::SetHeatVisionValues(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);
	float fFloat,fFadeValue;
  const char *sName;
	int		dwMask;

  pH->GetParam(1, sName);
	pH->GetParam(2, fFloat);
	pH->GetParam(3, dwMask);
	pH->GetParam(4, fFadeValue);

	m_pPlayer->SetHeatVisionValues(dwMask,sName,fFloat,fFadeValue);

	return pH->EndFunction();
}

int CScriptObjectPlayer::HolsterGun(IFunctionHandler *pH)
{
	m_pPlayer->HolsterWeapon();
	return pH->EndFunction();
}

int CScriptObjectPlayer::HoldGun(IFunctionHandler *pH)
{
	m_pPlayer->HoldWeapon();
	return pH->EndFunction();
}

int CScriptObjectPlayer::SetBlendTime(IFunctionHandler *pH)		// sets blend time for particular animation
{
	CHECK_PARAMETERS(2);
	float fBlendTime;
  const char *sName;

  pH->GetParam(1, sName);
	pH->GetParam(2, fBlendTime);

	m_pPlayer->SetBlendTime( sName, fBlendTime);
	return pH->EndFunction();
}

//
// 0 - off
// 1 - on
int CScriptObjectPlayer::SwitchFlashLight(IFunctionHandler *pH)		
{
	CHECK_PARAMETERS(1);
	int switchState;

	pH->GetParam(1, switchState);

	m_pPlayer->SwitchFlashLight( (switchState!=0) );
	return pH->EndFunction();
}

// 1 - player has flashlight
// 0 - player has no flashlight
int CScriptObjectPlayer::GiveFlashLight(IFunctionHandler *pH)		
{
	CHECK_PARAMETERS(1);
	int value;

	pH->GetParam(1, value);

	m_pPlayer->GiveFlashLight( value!=0 );
	return pH->EndFunction();
}
// 1 - player has binoculars
// 0 - player has no binoculars
int CScriptObjectPlayer::GiveBinoculars(IFunctionHandler *pH)		
{
	CHECK_PARAMETERS(1);
	int value;

	pH->GetParam(1, value);

	m_pPlayer->GiveBinoculars( value!=0 );
	return pH->EndFunction();
}

//
int CScriptObjectPlayer::GetBlindScreenPos(IFunctionHandler *pH)		
{
	CHECK_PARAMETERS(0);
	if(m_pPlayer->m_LastUsed == m_pPlayer->m_vBlindingList.end())
		return pH->EndFunctionNull();

	Vec3 vec = m_pPlayer->m_LastUsed->second;
	m_pBlindScreenPos->BeginSetGetChain();
	m_pBlindScreenPos->SetValueChain("x",vec.x);
	m_pBlindScreenPos->SetValueChain("y",vec.y);
	m_pBlindScreenPos->SetValueChain("z",vec.z);
	m_pBlindScreenPos->EndSetGetChain();

	m_pPlayer->m_LastUsed++;
	return pH->EndFunction(m_pBlindScreenPos);
}

//
//	sets speeds multiplyers for AI puppets

int CScriptObjectPlayer::SetAISpeedMult(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pObj(m_pScriptSystem,true);
	if(pH->GetParam(1,pObj))
	{
		pObj->BeginSetGetChain();
		float run = 0.0f; 
		float crouch = 0.0f; 
		float prone = 0.0f;  
		float xrun = 0.0f;  
		float xwalk = 0.0f;  
		float rrun = 0.0f;  
		float rwalk = 0.0f; 		
		pObj->GetValueChain("run",run);
		pObj->GetValueChain("crouch",crouch);
		pObj->GetValueChain("prone",prone);
		pObj->GetValueChain("xrun",xrun);
		pObj->GetValueChain("xwalk",xwalk);
		pObj->GetValueChain("rrun",rrun);
		pObj->GetValueChain("rwalk",rwalk);
		pObj->EndSetGetChain();
		m_pPlayer->SetSpeedMult( run, crouch, prone, xrun, xwalk, rrun, rwalk );
	}
	return pH->EndFunction();
}


// projTexName shaderName flags 
int CScriptObjectPlayer::InitDynamicLight(IFunctionHandler *pH)
{
const char *sTexName=NULL;
const char *sShaderName=NULL;

	if (pH->GetParamCount()>0)
	{
		pH->GetParam(1,sTexName);
	}
	if (pH->GetParamCount()>1)
	{
		pH->GetParam(2,sShaderName);
	}

	m_pPlayer->InitLight( sTexName, sShaderName );

	return pH->EndFunction();
}


int CScriptObjectPlayer::InitStaminaTable(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pTable(m_pScriptSystem,true);
	pH->GetParam(1,*pTable);

	StaminaTable	staminaT;

	pTable->GetValue("sprintScale", staminaT.RunSprintScale);
	pTable->GetValue("sprintSwimScale", staminaT.SwimSprintScale);
	pTable->GetValue("decoyRun",	staminaT.DecoyRun);
	pTable->GetValue("decoyJump",	staminaT.DecoyJump);
	pTable->GetValue("decoyUnderWater",	staminaT.DecoyUnderWater);
	pTable->GetValue("restoreRun",	staminaT.RestoreRun);
	pTable->GetValue("restoreWalk", staminaT.RestoreWalk);
	pTable->GetValue("restoreIdle", staminaT.RestoreIdle);

	pTable->GetValue("breathDecoyUnderwater",	staminaT.BreathDecoyUnderwater);
	pTable->GetValue("breathDecoyAim",			staminaT.BreathDecoyAim);
	pTable->GetValue("breathRestore",			staminaT.BreathRestore);

	if(m_pPlayer)
	{
		m_pPlayer->SetStaminaTable( staminaT );
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectPlayer::SavePlayerElements(IFunctionHandler *pH)
{
	// let's not do this when we play in StatisticsMode
	ICVar *g_LevelStated = GetISystem()->GetIConsole()->GetCVar("g_LevelStated");
	if (g_LevelStated && g_LevelStated->GetFVal() == 1.0f)
		return pH->EndFunction();	

	tPlayerPersistentData *pData=&m_pPlayer->GetGame()->m_tPlayerPersistentData;	

	// we don't want to save vehicle's weapons
	if( m_pPlayer->m_pVehicle )
		m_pPlayer->m_pVehicle->ReleaseWeaponUser( true );

	//////////////////////////////////////////////////////////////////////////	
	// store health and armor
	pData->m_nHealth=m_pPlayer->m_stats.health;
	pData->m_nArmor=m_pPlayer->m_stats.armor;

	//////////////////////////////////////////////////////////////////////////	
	// store health and armor
	// weapons data which needs to be saved:
	// weapons held by the player
	// current weapon
	// possibily current weapon's fire mode
	pData->m_nSelectedWeaponID = m_pPlayer->GetSelectedWeaponId();
	if (pData->m_nSelectedWeaponID != -1)
	{
		pData->m_nAmmo = m_pPlayer->m_stats.ammo;
		pData->m_nAmmoInClip = m_pPlayer->m_stats.ammo_in_clip;
	}
	else
	{
		pData->m_nAmmo = 0;
		pData->m_nAmmoInClip = 0;
	}

	m_pPlayer->DeselectWeapon();

	_SmartScriptObject weaponstate(m_pScriptSystem,true);

	if (m_pPlayer->GetEntity()->GetScriptObject()->GetValue("WeaponState", weaponstate))
	{
		// make sure the cached ammo values are written back to the respective stores
		m_pScriptSystem->BeginCall("BasicPlayer", "SyncCachedAmmoValues");
		m_pScriptSystem->PushFuncParam(m_pPlayer->GetEntity()->GetScriptObject());
		m_pScriptSystem->EndCall();

		pData->m_mapWeapons.clear();
		CPlayer::PlayerWeaponsItor itor;
		for(itor = m_pPlayer->m_mapPlayerWeapons.begin(); itor!=m_pPlayer->m_mapPlayerWeapons.end(); ++itor)
		{
			WeaponInfo &wi = itor->second;
			if(wi.owns)
			{
				int weaponid = itor->first;
				_SmartScriptObject curWeaponState(m_pScriptSystem,true);
				_SmartScriptObject ammoInClip(m_pScriptSystem,true);
				tWeaponPersistentData weaponData;
				if (weaponstate->GetAt(weaponid, curWeaponState) && curWeaponState->GetValue("AmmoInClip", ammoInClip))
				{
					// transfer ammo in the clips
					ammoInClip->BeginIteration();
					while(ammoInClip->MoveNext())
					{
						int nAmount;

						ammoInClip->GetCurrent(nAmount);
						weaponData.m_nAmmoInClip.push_back(nAmount);
					}
					ammoInClip->EndIteration();
				}
				weaponData.m_nFireMode = wi.iFireMode;
				pData->m_mapWeapons[weaponid] = weaponData;
			}
		}
		memcpy(pData->m_vWeaponSlots, m_pPlayer->m_vWeaponSlots, sizeof(m_pPlayer->m_vWeaponSlots));

		// ammo for each weapon
		// grenades
		_SmartScriptObject ammo(m_pScriptSystem,true);
		pData->m_mapAmmo.clear();
		if (m_pPlayer->GetEntity()->GetScriptObject()->GetValue("Ammo", ammo))
		{
			ammo->BeginIteration();
			while(ammo->MoveNext())
			{
				const char *sAmmoType;
				int nAmount;

				ammo->GetCurrentKey(sAmmoType);
				ammo->GetCurrent(nAmount);
				pData->m_mapAmmo[sAmmoType]=nAmount;
			}
			ammo->EndIteration();
		}
	}

	// items
	pData->m_lItems.clear();
	if (m_pPlayer->HasBinoculars())
		pData->m_lItems.push_back("Binoculars");

	if (m_pPlayer->HasFlashLight())
		pData->m_lItems.push_back("FlashLight");

	_SmartScriptObject items(m_pScriptSystem,true);
	if (m_pPlayer->GetEntity()->GetScriptObject()->GetValue("items", items))
	{
		items->BeginIteration();
		while(items->MoveNext())
		{
			const char *sItem;

			items->GetCurrentKey(sItem);
			pData->m_lItems.push_back(sItem);
		}
		items->EndIteration();
	}

	//////////////////////////////////////////////////////////////////////////	
	// specifiy that some data has been saved, since it is not possible
	// to reload from script certain player's elements - so after
	// loading the level these data are restored if this value is set to
	// true.
	pData->m_bDataSaved=true; 
	
	return pH->EndFunction();	
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectPlayer::LoadPlayerElements(IFunctionHandler *pH)
{
	tPlayerPersistentData *pData=&m_pPlayer->GetGame()->m_tPlayerPersistentData;	

	if (!pData->m_bDataSaved)
		return pH->EndFunction(); // nothing to restore

	// data have been restored, must be triggered again
	// to make new data persistent
	pData->m_bDataSaved=false;

	//////////////////////////////////////////////////////////////////////////	
	// restore health and armor
	//m_pPlayer->m_stats.health=pData->m_nHealth;
	m_pPlayer->m_stats.health=255;
	m_pPlayer->m_stats.armor=pData->m_nArmor;

	//////////////////////////////////////////////////////////////////////////	
	// restore weapons and ammo
	// [...] MarcoK
	m_pPlayer->DeselectWeapon();
	m_pPlayer->InitWeapons();

	// ammo and grenades
	// ammo for each weapon
	// grenades
	std::map<string, int>::iterator j;

	_SmartScriptObject ammo(m_pScriptSystem,true);

	m_pPlayer->GetEntity()->GetScriptObject()->SetValue("DontInitWeapons", true);

	// set all used ammos to 0
	if (m_pPlayer->GetEntity()->GetScriptObject()->GetValue("Ammo", ammo))
	{
		ammo->Clear();
		ammo->BeginSetGetChain();
		for(j = pData->m_mapAmmo.begin(); j != pData->m_mapAmmo.end(); ++j)
		{
			ammo->SetValueChain((j->first).c_str(), 0);
		}
		ammo->EndSetGetChain();
	}

	m_pPlayer->RemoveAllWeapons();
	std::map<int, tWeaponPersistentData>::iterator i;
	for (i = pData->m_mapWeapons.begin(); i != pData->m_mapWeapons.end(); ++i)
	{
			int weaponid = i->first;
			tWeaponPersistentData &weaponData = i->second;
			m_pPlayer->MakeWeaponAvailable(weaponid, true);
			m_pPlayer->m_mapPlayerWeapons[weaponid].iFireMode = weaponData.m_nFireMode;

			CWeaponClass *pWC = m_pPlayer->GetGame()->GetWeaponSystemEx()->GetWeaponClassByID(weaponid);
			if (pWC)
			{
				m_pScriptSystem->BeginCall("BasicPlayer", "ScriptInitWeapon");
				m_pScriptSystem->PushFuncParam(m_pPlayer->GetEntity()->GetScriptObject());
				m_pScriptSystem->PushFuncParam(pWC->GetName().c_str());
				m_pScriptSystem->EndCall();

				// should have the correct weapon state at this point
				_SmartScriptObject weaponState(m_pScriptSystem,true);
				_SmartScriptObject curWeaponState(m_pScriptSystem,true);
				if (m_pPlayer->GetEntity()->GetScriptObject()->GetValue("WeaponState", weaponState) && weaponState->GetAt(weaponid, curWeaponState))
				{
					// time to poke in the correct ammo in clip values
					std::vector<int>::iterator curFireMode;
					_SmartScriptObject ammoInClip(m_pScriptSystem,true);

					if (curWeaponState->GetValue("AmmoInClip", ammoInClip))
					{
						int idx = 1;
						for (curFireMode = weaponData.m_nAmmoInClip.begin(); curFireMode != weaponData.m_nAmmoInClip.end(); ++curFireMode)
						{
							ammoInClip->SetAt(idx, *curFireMode);
							++idx;
						}
					}
				}
			}
	}
	memcpy(m_pPlayer->m_vWeaponSlots, pData->m_vWeaponSlots, sizeof(pData->m_vWeaponSlots));

	// set all used ammos to correct values
	if (m_pPlayer->GetEntity()->GetScriptObject()->GetValue("Ammo", ammo))
	{
		ammo->Clear();
		ammo->BeginSetGetChain();
		for(j = pData->m_mapAmmo.begin(); j != pData->m_mapAmmo.end(); ++j)
		{
			string sType = j->first;
			int nAmount = j->second;
			ammo->SetValueChain(sType.c_str(), nAmount);
		}
		ammo->EndSetGetChain();
	}

	// items
	std::list<string>::iterator k;
	_SmartScriptObject items(m_pScriptSystem,true);
	if (m_pPlayer->GetEntity()->GetScriptObject()->GetValue("items", items))
	{
		for (k = pData->m_lItems.begin(); k != pData->m_lItems.end(); ++k)
		{
			if ((*k) == "Binoculars")
			{
				m_pPlayer->GiveBinoculars(true);
			}else	if ((*k) == "FlashLight")
			{
				m_pPlayer->GiveFlashLight(true);
			}else
			{
				items->SetValue((*k).c_str(), 1);
			}
		}
	}

	m_pScriptSystem->BeginCall("BasicPlayer", "InitAllWeapons");
	m_pScriptSystem->PushFuncParam(m_pPlayer->GetEntity()->GetScriptObject());
	m_pScriptSystem->PushFuncParam((int)1);
	m_pScriptSystem->EndCall();

	// select the right weapon
	if (pData->m_nSelectedWeaponID!=-1)
	{
		m_pPlayer->SelectWeapon(pData->m_nSelectedWeaponID);
		m_pPlayer->m_stats.ammo = pData->m_nAmmo;
		m_pPlayer->m_stats.ammo_in_clip = pData->m_nAmmoInClip;
	}

	return pH->EndFunction();	
}

//////////////////////////////////////////////////////////////////////////
/*
int CScriptObjectPlayer::GetProjectedBloodPos(IFunctionHandler *pH)
{
	float	fDist;
	Vec3	dir, pos;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*oVec);
	pos=oVec.Get();
	pH->GetParam(2,*oVec);
	dir=oVec.Get();
	pH->GetParam(3,fDist);
	ray_hit hit;
	if (m_pPlayer->GetGame()->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection(vectorf(pos), vectorf(dir*fDist), ent_all,
			0,&hit,1, m_pPlayer->GetEntity()->GetPhysics()))
	{
		IEntity *centycontact=NULL;	

		CScriptObjectVector oVecDir(m_pScriptSystem);
		CScriptObjectVector oVecNorm(m_pScriptSystem);

		m_pTempBloodObj->BeginSetGetChain();
		oVec = hit.pt;
		m_pTempBloodObj->SetValueChain("pos",	oVec);
		oVecNorm = hit.n;
		m_pTempBloodObj->SetValueChain("normal",oVecNorm);
		oVecDir = dir;
		m_pTempBloodObj->SetValueChain("dir",	oVecDir);
		m_pTempBloodObj->SetValueChain("dist",	hit.dist);

		pe_params_foreign_data fd;
//		hit.pCollider()->GetParams();
//		fd.type
		centycontact = (IEntity *)hit.pCollider->GetForeignData();
		if (centycontact && centycontact->IsGarbage())
		{
			return pH->EndFunctionNull();
		}
		if (centycontact && (!centycontact->IsGarbage()))
			m_pTempBloodObj->SetValueChain("target_id",centycontact->GetId());
		else 
			m_pTempBloodObj->SetToNull("target_id");
		m_pTempBloodObj->EndSetGetChain();
		return pH->EndFunction(m_pTempBloodObj);
	}
	return pH->EndFunctionNull();
}
*/

IEntityRender * CScriptObjectPlayer::GetIEntityRender(const pe_params_foreign_data & fd)
{
	IEntityRender * pEntityRender = NULL;
	if(fd.iForeignData == 0)
		pEntityRender = (IEntityRender*)(IEntity*)fd.pForeignData;
	else if(fd.iForeignData == 1)
		pEntityRender = (IEntityRender*)fd.pForeignData;
	else
		assert(0); // unknown object type
	
	return pEntityRender;
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectPlayer::GetProjectedBloodPos(IFunctionHandler *pH)
{
	const char *decalTableName;
	float	fDist;
	Vec3	dir, pos;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*oVec);
	pos=oVec.Get();
	pH->GetParam(2,*oVec);
	dir=oVec.Get().GetNormalized();
	pH->GetParam(3,decalTableName);
	pH->GetParam(4,fDist);
	ray_hit hit;


	// parse the particle table
	_SmartScriptObject	pDecalsTable(m_pScriptSystem, true);
	_SmartScriptObject	pTheDecalTable(m_pScriptSystem, true);
	char	nameBuff[32];
	int decalNumber=0;

	if(!m_pScriptSystem->GetGlobalValue(decalTableName,pDecalsTable))
		return pH->EndFunctionNull();
	pDecalsTable->GetValue("count", decalNumber);

	if(decalNumber == 0)
		return pH->EndFunctionNull();
	decalNumber = (rand()%decalNumber) + 1;
	sprintf(nameBuff,"dec%d",decalNumber);
	if(!pDecalsTable->GetValue(nameBuff, pTheDecalTable))
		return pH->EndFunctionNull();

	CryEngineDecalInfo Decal;
	int nCookie=0;
	int	rand_size=0;

	if(!pTheDecalTable->GetUDValue("texture",Decal.nTid, nCookie))
		return pH->EndFunctionNull();
	pTheDecalTable->GetValue("scale",Decal.fSize);
	pTheDecalTable->GetValue("random_scale",rand_size);
	pTheDecalTable->GetValue("random_rotation",Decal.fAngle);
	pTheDecalTable->GetValue("life_time",Decal.fLifeTime);
	pTheDecalTable->GetValue("grow_time",Decal.m_fGrowTime);

	if( rand_size>0 )
		Decal.fSize += ((Decal.fSize*0.01f)*(rand()%rand_size));

	// [Vlad] spawn decals on everything collidable with player
	int dwObjTypes = ent_terrain|ent_static;// (no blood in the air) |ent_sleeping_rigid|ent_rigid;

	if (m_pPlayer->GetGame()->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection(vectorf(pos), vectorf(dir*fDist), dwObjTypes,
		( dir.x==0 && dir.y==0 ) ? 0 : rwi_stop_at_pierceable, // exclude pierceable objects for blood pool
			&hit,1, m_pPlayer->GetEntity()->GetPhysics()))
	{
		pe_params_foreign_data fd;
		hit.pCollider->GetParams(&fd);

		IEntityRender * pFirstDecalOwner = Decal.pDecalOwner = GetIEntityRender(fd);
		Decal.nPartID = hit.ipart;
		Decal.vPos = hit.pt;

		Decal.vNormal = hit.n;
		Decal.vHitDirection = dir;

		Decal.fAngle = float(rand()%3600)*0.1f;
	
		if(hit.n.Dot(dir) < 0 && !m_pPlayer->GetGame()->GetSystem()->GetI3DEngine()->IsPointInWater(hit.pt))
			m_pPlayer->GetGame()->GetSystem()->GetI3DEngine()->CreateDecal(Decal);

		// if it's a blood pool on ground - check points around to see if there are more entities/brushes - 
		// spawn decal on every object
		if( dir.x==0 && dir.y==0 )
		{
			for(float x=-1.f; x<=1.f; x+=2.f)
			for(float y=-1.f; y<=1.f; y+=2.f)
			{
				ray_hit hitAux;
				if (m_pPlayer->GetGame()->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection(vectorf(pos)+vectorf(Decal.fSize*x,Decal.fSize*y,0), 
					vectorf(dir*fDist), dwObjTypes,
					0,&hitAux,1, m_pPlayer->GetEntity()->GetPhysics()) && hitAux.pCollider!=hit.pCollider 
					&& hitAux.n.Dot(dir) < 0 )
				{
					pe_params_foreign_data fd;
					hitAux.pCollider->GetParams(&fd);
					Decal.pDecalOwner = GetIEntityRender(fd);
					Decal.nPartID = hitAux.ipart;
					Decal.vPos = hit.pt;
					Decal.vNormal = hitAux.n;
					
					if( Decal.pDecalOwner != pFirstDecalOwner // reduce decal duplications
						 && !m_pPlayer->GetGame()->GetSystem()->GetI3DEngine()->IsPointInWater(hit.pt))
						m_pPlayer->GetGame()->GetSystem()->GetI3DEngine()->CreateDecal(Decal);
				}
			}
		}
	}
	return pH->EndFunctionNull();
}

//
int CScriptObjectPlayer::UseLadder(IFunctionHandler *pH)
{
	//ASSERT(pH->GetParamCount() == 1 || pH->GetParamCount() == 2);
	if (pH->GetParamCount()>0)
	{
		int	onLadder=0;

		pH->GetParam(1,onLadder);

		//if we cann useladder with -1 as first parameter , the second params represent a new ladder origin
		if (onLadder==-1 && m_pPlayer->m_stats.onLadder)
		{
			if (pH->GetParamCount()>1)
			{
				CScriptObjectVector oVec(m_pScriptSystem,true);
				pH->GetParam(2,*oVec);

				m_pPlayer->m_vLadderPosition = oVec.Get();
			}
		}
		else 
		{
			if(onLadder!=0)
			{
				m_pPlayer->m_PrevWeaponID=m_pPlayer->GetSelectedWeaponId();
				m_pPlayer->SelectWeapon(-1);
				m_pPlayer->m_stats.onLadder = true;
				m_pPlayer->GoStand();

				// save current movement speeds
				m_fSpeedRun = m_pPlayer->m_RunSpeed;
				m_fSpeedWalk = m_pPlayer->m_WalkSpeed;
				m_fSpeedCrouch = m_pPlayer->m_CrouchSpeed;
				m_fSpeedProne = m_pPlayer->m_ProneSpeed;
				float fSpeed=0;
				pH->GetParam(2,fSpeed);

				m_pPlayer->SetRunSpeed(fSpeed);
				m_pPlayer->SetWalkSpeed(fSpeed);
				m_pPlayer->SetCrouchSpeed(fSpeed);
				m_pPlayer->SetProneSpeed(fSpeed);

				if (pH->GetParamCount()>2)
				{
					CScriptObjectVector oVec(m_pScriptSystem,true);
					pH->GetParam(3,*oVec);

					m_pPlayer->m_vLadderPosition = oVec.Get();
				}
				else
				{
					m_pPlayer->m_vLadderPosition.Set(0,0,0);
				}

				m_pPlayer->m_vLadderAngles.Set(0,0,0);
			}
			else
			{
				// restore speeds only if saved before
				if(m_fSpeedRun)
				{
					m_pPlayer->SetRunSpeed(m_fSpeedRun);
					m_pPlayer->SetWalkSpeed(m_fSpeedWalk);
					m_pPlayer->SetCrouchSpeed(m_fSpeedCrouch);
					m_pPlayer->SetProneSpeed(m_fSpeedProne);
				}

				m_pPlayer->m_stats.onLadder = false;
				if (m_pPlayer->m_PrevWeaponID>=0) 
					m_pPlayer->SelectWeapon(m_pPlayer->m_PrevWeaponID);
			}

			m_pPlayer->InitCameraTransition( CPlayer::PCM_CASUAL, true );
		}
	}

	return pH->EndFunctionNull();
}

//
// for invehicle weapons  -  if crosshair is on screen/notSnapped
int CScriptObjectPlayer::GetCrosshairState(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	bool bResult=false;
	if(m_pPlayer->m_pVehicle)
		bResult = m_pPlayer->m_pVehicle->CrossOnScreen();
	return pH->EndFunction(bResult);
}

//
int CScriptObjectPlayer::ResetCamera(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

//	m_pPlayer->m_vCurCamposVhcl = m_pPlayer->m_vEyePos;
//	m_pPlayer->m_vCurAngleVhcl = m_pPlayer->m_vEyeAngles; 

	m_pPlayer->m_vEyePos = m_pPlayer->GetEntity()->GetPos() + Vec3(0,0, m_pPlayer->m_PlayerDimNormal.heightEye);
	m_pPlayer->m_vEyeAngles = m_pPlayer->GetEntity()->GetAngles(); 
	m_pPlayer->m_CameraMode = CPlayer::PCM_OUTVEHICLE;
	m_pPlayer->m_fCameraTime = 0.0f;
	m_pPlayer->m_bLastDeltaEyeVehicle = false;


	return pH->EndFunctionNull();
}

//
int CScriptObjectPlayer::ResetRotateHead(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);
	m_pPlayer->ResetRotateHead();
	m_pPlayer->m_sPrevAniName.clear();
	return pH->EndFunctionNull();
}

//
//checks with physics if can stand at current position (not inside of something)
int CScriptObjectPlayer::CanStand(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	Vec3 vec;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*oVec);
	vec=oVec.Get();
	bool	bCanStand = m_pPlayer->CanStand( vec ); 
	return pH->EndFunction(bCanStand);
}

//this function set the smoothing for the player input, was made for AIs but its also usable with players.
int CScriptObjectPlayer::SetSmoothInput(IFunctionHandler *pH)
{
	//CHECK_PARAMETERS(2);

	float input_accel = 0;
	float input_stop_accel = 0;
	float input_accel_indoor = 0;
	float input_stop_accel_indoor = 0;

	if (pH->GetParamCount()>1)
	{
		pH->GetParam(1,input_accel);
		pH->GetParam(2,input_stop_accel);
	}
	
	//if more than 3 params there are special values for indoors, only for AI
	if (pH->GetParamCount()>3 && m_pPlayer->IsAI())
	{
		pH->GetParam(3,input_accel_indoor);
		pH->GetParam(4,input_stop_accel_indoor);
	}

	m_pPlayer->m_input_accel = input_accel;
	m_pPlayer->m_input_stop_accel = input_stop_accel;

	m_pPlayer->m_input_accel_indoor = input_accel_indoor;
	m_pPlayer->m_input_stop_accel_indoor = input_stop_accel_indoor;

	return pH->EndFunction();
}

