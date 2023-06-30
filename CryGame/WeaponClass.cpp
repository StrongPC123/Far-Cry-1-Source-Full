//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	History:
//	- May 2003: Created by MarcoK
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "WeaponClass.h"
#include "XPlayer.h"
#include "XVehicle.h"
#include "Flock.h"
#include "WeaponSystemEx.h"
#include "ScriptObjectWeaponClass.h"
#include <IAISystem.h>


#define SCRIPT_BEGINCALL(host, func)\
	if (m_rWeaponSystem.GetGame()->Is##host() && m_h##host##Funcs[WeaponFunc_##func]){\
		m_pScriptSystem->BeginCall(m_h##host##Funcs[WeaponFunc_##func]);\
		m_pScriptSystem->PushFuncParam(m_soWeaponClass);

#define SCRIPT_ENDCALL() 		m_pScriptSystem->EndCall();}



//#define FIRE_DEBUG			// only for debugging

CWeaponClass::CWeaponClass(CWeaponSystemEx& rWeaponSystem) :
m_rWeaponSystem(rWeaponSystem)
{
	m_pObject = NULL;
	m_pCharacter = NULL;
	m_pMuzzleFlash = NULL;
	m_ID = 0;
	m_bIsLoaded = false;
	m_pScriptSystem = NULL;
	m_soWeaponClass = NULL;

	m_vAngles.Set(0,0,0);
	m_vPos.Set(0,0,0);
	m_fpvPos.Set(0,0,0);
	m_fpvAngles.Set(0,0,0);
	m_fpvPosOffset.Set(0,0,0);
	m_fpvAngleOffset.Set(0,0,0);
	m_fLastUpdateTime = 0;

	memset(m_hClientFuncs, 0, sizeof(m_hClientFuncs));
	memset(m_hServerFuncs, 0, sizeof(m_hServerFuncs));

	m_nAIMode = -1;
}

CWeaponClass::~CWeaponClass()
{
	Reset();
}

void CWeaponClass::OnStartAnimation(const char *sAnimation)
{
}

void CWeaponClass::OnAnimationEvent(const char *sAnimation, AnimSinkEventData UserData)
{
	FUNCTION_PROFILER( GetWeaponSystem().GetGame()->GetSystem(),PROFILE_GAME );

	IScriptObject *params = m_sso_Params_OnAnimationKey;
	params->SetValue("animation", sAnimation);
	USER_DATA udUserData = (USER_DATA)UserData.p;
	if (udUserData!=USER_DATA(-1))
		params->SetValue("userdata", udUserData);
	else
		params->SetToNull("userdata");

	SCRIPT_BEGINCALL(Client, OnAnimationKey);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, OnAnimationKey);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
}

void CWeaponClass::OnEndAnimation(const char *sAnimation)
{
}

bool CWeaponClass::Init(const string& sName)
{
	Reset();

	ILog *pLog = m_rWeaponSystem.GetGame()->GetSystem()->GetILog();
	assert(pLog);

	m_pScriptSystem = m_rWeaponSystem.GetGame()->GetSystem()->GetIScriptSystem();
	assert(m_pScriptSystem);

	m_sName	= sName;

	if (!InitWeaponClassVariables())
		return false;

	return true;
}

void CWeaponClass::Reset()
{
	m_bIsLoaded = false;

	// release loaded weapon models, if necessary
	if (m_pObject)
	{
		m_rWeaponSystem.GetGame()->GetSystem()->GetI3DEngine()->ReleaseObject(m_pObject);
		m_pObject = NULL;
	}
	if (m_pCharacter)
	{
		m_rWeaponSystem.GetGame()->GetSystem()->GetIAnimationSystem()->RemoveCharacter(m_pCharacter);
		m_pCharacter = NULL;
	}
	if (m_pMuzzleFlash)
	{
		m_rWeaponSystem.GetGame()->GetSystem()->GetI3DEngine()->ReleaseObject(m_pMuzzleFlash);
		m_pMuzzleFlash = NULL;
	}
	m_sBindBone.clear();

	// set the global table to NULL, and garbage collect it
	if (!m_pScriptSystem) return;

	if (m_soWeaponClass)
	{
		m_soWeaponClass->Release();
	}

	m_pScriptSystem->SetGlobalToNull(m_sName.c_str());

	m_rWeaponSystem.UnloadScript(m_sScript);

	// release script callback function
	for (int i = 0; i < WeaponFunc_Count; ++i)
	{
		if (m_hClientFuncs[i])
			m_pScriptSystem->ReleaseFunc(m_hClientFuncs[i]);
		if (m_hServerFuncs[i])
			m_pScriptSystem->ReleaseFunc(m_hServerFuncs[i]);
	}

	//Never force Lua GC, m_pScriptSystem->ForceGarbageCollection();

	while(!m_vFireModes.empty())
	{
		delete m_vFireModes.back();
		m_vFireModes.pop_back();
	}
}

void CWeaponClass::Read(CStream& stm)
{
	stm.Read(m_sName);
}

void CWeaponClass::Write(CStream& stm) const
{
	stm.Write(m_sName);
}

void CWeaponClass::ScriptOnInit()
{
	SCRIPT_BEGINCALL(Client, OnInit);
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, OnInit);
	SCRIPT_ENDCALL();
}

void CWeaponClass::ScriptOnActivate(IEntity *pShooter)
{
	IScriptObject *params = m_sso_Params_OnActivate;

	params->SetValue("shooter", pShooter->GetScriptObject());

	SCRIPT_BEGINCALL(Client, OnActivate);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, OnActivate);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
}

void CWeaponClass::ScriptOnDeactivate(IEntity *pShooter)
{
	IScriptObject *params = m_sso_Params_OnActivate;

	params->SetValue("shooter", pShooter->GetScriptObject());

	SCRIPT_BEGINCALL(Client, OnDeactivate);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, OnDeactivate);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
}

void CWeaponClass::ScriptWeaponReady(IEntity *pShooter)
{
	SCRIPT_BEGINCALL(Client, WeaponReady);
	m_pScriptSystem->PushFuncParam(pShooter->GetScriptObject());
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, WeaponReady);
	m_pScriptSystem->PushFuncParam(pShooter->GetScriptObject());
	SCRIPT_ENDCALL();
}

void CWeaponClass::ScriptOnStopFiring(IEntity *pShooter)
{
	SCRIPT_BEGINCALL(Client, OnStopFiring);
	m_pScriptSystem->PushFuncParam(pShooter->GetScriptObject());
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, OnStopFiring);
	m_pScriptSystem->PushFuncParam(pShooter->GetScriptObject());
	SCRIPT_ENDCALL();
}

void CWeaponClass::ScriptReload(IEntity *pShooter)
{
	SCRIPT_BEGINCALL(Client, Reload);
	m_pScriptSystem->PushFuncParam(pShooter->GetScriptObject());
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, Reload);
	m_pScriptSystem->PushFuncParam(pShooter->GetScriptObject());
	SCRIPT_ENDCALL();
}

void CWeaponClass::ScriptDrop(IScriptObject* params)
{
	SCRIPT_BEGINCALL(Client, Drop);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, Drop);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
}

void CWeaponClass::ScriptOnUpdate(float fDeltaTime, IEntity* pEntity)
{
	SCRIPT_BEGINCALL(Client, OnUpdate);
	m_pScriptSystem->PushFuncParam(fDeltaTime);
	m_pScriptSystem->PushFuncParam(pEntity->GetScriptObject());
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, OnUpdate);
	m_pScriptSystem->PushFuncParam(fDeltaTime);
	m_pScriptSystem->PushFuncParam(pEntity->GetScriptObject());
	SCRIPT_ENDCALL();
}

bool CWeaponClass::ScriptOnFireCancel(IScriptObject *params)
{
	/*SCRIPT_BEGINCALL(Client, OnFireCancel);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, OnStopFiring);
	m_pScriptSystem->PushFuncParam(params);*/
	return false;
}

//m_pEntity->SendScriptEvent(ScriptEvent_Fire, *m_ssoFireTable, &bWeaponReady);
bool CWeaponClass::ScriptOnFire(IScriptObject *pParamters)
{
	bool bRet = true;

	// Client side always first
	if (m_rWeaponSystem.GetGame()->IsClient() && m_hClientFuncs[WeaponFunc_OnFire])
	{
		FRAME_PROFILER( "CWeaponClass::Client_ScriptOnFire",GetISystem(),PROFILE_GAME );
		m_pScriptSystem->BeginCall(m_hClientFuncs[WeaponFunc_OnFire]);
		m_pScriptSystem->PushFuncParam(m_soWeaponClass);
		if (pParamters)
			m_pScriptSystem->PushFuncParam(pParamters);
		else
			m_pScriptSystem->PushFuncParam(false);

		// Only use return value if we ain't got a server event
		m_pScriptSystem->EndCall(bRet);
	}

	if (m_rWeaponSystem.GetGame()->IsServer() && m_hServerFuncs[WeaponFunc_OnFire])
	{
		FRAME_PROFILER( "CWeaponClass::Server_ScriptOnFire",GetISystem(),PROFILE_GAME );
		m_pScriptSystem->BeginCall(m_hServerFuncs[WeaponFunc_OnFire]);
		m_pScriptSystem->PushFuncParam(m_soWeaponClass);
		if (pParamters)
			m_pScriptSystem->PushFuncParam(pParamters);
		else
			m_pScriptSystem->PushFuncParam(false);

		m_pScriptSystem->EndCall(bRet);
	}

	return bRet;
}

void CWeaponClass::ScriptOnHit(IScriptObject* params)
{
	SCRIPT_BEGINCALL(Client, OnHit);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
	SCRIPT_BEGINCALL(Server, OnHit);
	m_pScriptSystem->PushFuncParam(params);
	SCRIPT_ENDCALL();
}

void CWeaponClass::ScriptOnEvent(int eventID, IScriptObject *pParameters, bool *pRet)
{
	// Client side always first
	if (m_rWeaponSystem.GetGame()->IsClient() && m_hClientFuncs[WeaponFunc_OnEvent])
	{
		m_pScriptSystem->BeginCall(m_hClientFuncs[WeaponFunc_OnEvent]);
		m_pScriptSystem->PushFuncParam(m_soWeaponClass);
		m_pScriptSystem->PushFuncParam(eventID);
		if (pParameters)
			m_pScriptSystem->PushFuncParam(pParameters);
		else
			m_pScriptSystem->PushFuncParam(false);

		// Only use return value if we ain't got a server event
		if (pRet)
			m_pScriptSystem->EndCall(*pRet);
		else
			m_pScriptSystem->EndCall();
	}

	// Server side always second
	if (m_rWeaponSystem.GetGame()->IsServer() && m_hServerFuncs[WeaponFunc_OnEvent])
	{
		m_pScriptSystem->BeginCall(m_hServerFuncs[WeaponFunc_OnEvent]);
		m_pScriptSystem->PushFuncParam(m_soWeaponClass);
		m_pScriptSystem->PushFuncParam((int)eventID);
		if (pParameters)
			m_pScriptSystem->PushFuncParam(pParameters);
		else
			m_pScriptSystem->PushFuncParam(false);
		
		if (pRet)
			m_pScriptSystem->EndCall(*pRet);
		else
			m_pScriptSystem->EndCall();
	}
}

void CWeaponClass::SetFirstPersonWeaponPos( const Vec3 &pos,const Vec3 &angles )
{
	// Move weapon for first person view.
	m_fpvPos = pos;
	m_fpvAngles = angles;
}

Vec3	CWeaponClass::GetFirePos( IEntity *pIEntity ) const
{
	ASSERT( pIEntity != 0 );
	Vec3 firepos = pIEntity->GetCamera()->GetPos();

	return firepos;
}

void CWeaponClass::SetFirstPersonOffset( const Vec3 &posOfs,const Vec3 &angOfs )
{
	m_fpvPosOffset = posOfs;
	m_fpvAngleOffset = angOfs;
}

void CWeaponClass::MoveToFirstPersonPos(IEntity *pIEntity)
{
	Vec3 pos = m_fpvPos+m_fpvPosOffset;

	Matrix44 m=Matrix34::CreateRotationXYZ( Deg2Rad(pIEntity->GetCamera()->GetAngles()), pIEntity->GetCamera()->GetPos() );	//set rotation and translation in one function call
	m	=	GetTransposed44(m); //TODO: remove this after E3 and use Matrix34 instead of Matrix44

	m_vPos = m.TransformPointOLD(pos);
	m_vAngles = pIEntity->GetCamera()->GetAngles()+m_fpvAngleOffset;
}

void CWeaponClass::Unload()
{
	Reset();
}

bool CWeaponClass::Load()
{
	if (m_bIsLoaded) return true;

	if (!InitScripts())
		return false;

	m_bIsLoaded = true;

	return true;
}

// get the fire rate appropriate for the fire type. If we are just pressing
// the mouse button (i.e. the first shot is being fired), we have to use the
// tap fire rate. Otherwise, we will compare against the hold fire rate
float CWeaponClass::GetFireRate(eFireType ft) const
{
	if (ft == ePressing)
		return m_fireParams.fTapFireRate;
	else
		return m_fireParams.fFireRate;
}

bool CWeaponClass::HasAIFireMode()	const
{
	return (m_nAIMode != -1);
}

int CWeaponClass::GetAIFireMode()	const
{
	if (HasAIFireMode())
		return m_nAIMode;
	else
		return 0;
}

int CWeaponClass::GetNextFireMode(int oldMode, bool isAI) const
{
	if (isAI)
		return GetAIFireMode();

	for (unsigned int i = 0; i < m_vFireModes.size(); ++i)
	{
		int nextMode = (oldMode + i + 1) % m_vFireModes.size();
		if (m_vFireModes[nextMode]->bAIMode == false)
			return nextMode;
	}

	return 0;
}

unsigned CWeaponClass::MemStats() const
{
	unsigned memSize = sizeof *this;

	memSize += m_sScript.capacity();
	memSize += m_sPickup.capacity();
	memSize += m_sName.capacity();
	memSize += m_sBindBone.capacity();
	memSize += m_vFireModes.capacity() * sizeof(void*) + m_vFireModes.size() * sizeof(WeaponParams);

	return memSize;
}


bool CWeaponClass::InitWeaponClassVariables()
{
	ILog *pLog = m_rWeaponSystem.GetGame()->GetSystem()->GetILog();
	assert(pLog);

	m_ssoFireTable.Create(m_pScriptSystem);
	m_ssoProcessHit.Create(m_pScriptSystem);
	m_ssoHitPosVec.Create(m_pScriptSystem);
	m_ssoHitDirVec.Create(m_pScriptSystem);
	m_ssoHitNormVec.Create(m_pScriptSystem);
	m_ssoHitPt.Create(m_pScriptSystem);
	m_ssoBulletPlayerPos.Create(m_pScriptSystem);

	m_sso_Params_OnAnimationKey.Create(m_pScriptSystem);
	m_sso_Params_OnActivate.Create(m_pScriptSystem);
	m_sso_Params_OnDeactivate.Create(m_pScriptSystem);

	// get entry in WeaponClasses table
	IScriptObject *soWeaponClasses = m_rWeaponSystem.GetWeaponClassesTable();

	_SmartScriptObject soObj(m_pScriptSystem, true);

	if (!soWeaponClasses->GetValue(m_sName.c_str(), soObj))
	{
		pLog->LogError("Cannot access %d weapon class entry", m_sName.c_str());
		return false;
	}

	// get values out of the entry in the WeaponClasses table
	char * sVal = 0;
	if (!soObj->GetValue("id", m_ID))
	{
		pLog->LogError("CWeaponClass: Cannot access field 'id'");
		return false;
	}
	if (!soObj->GetValue("script", (const char* &)sVal))
	{
		pLog->LogError("CWeaponClass: Cannot access field 'script'");
		return false;
	}
	m_sScript = sVal;
	if (soObj->GetValue("pickup", (const char* &)sVal))
	{
		m_sPickup = sVal;
	}

	return true;
}

bool CWeaponClass::InitScripts()
{
	ILog *pLog = m_rWeaponSystem.GetGame()->GetSystem()->GetILog();
	assert(pLog);

	// execute weapon script
	if (!m_rWeaponSystem.ExecuteScript(m_sScript))
	{
		return false;
	}

	CScriptObjectWeaponClass* soWC = new CScriptObjectWeaponClass();

	if (!soWC->Create(m_rWeaponSystem.GetGame(), this))
		return false;

	m_soWeaponClass = soWC->GetScriptObject();
	ASSERT(m_soWeaponClass)

	// get client/server functions
	bool bOnClient = m_rWeaponSystem.GetGame()->IsClient();
	bool bOnServer = m_rWeaponSystem.GetGame()->IsServer();

	#define GET_VALUE_EX(host, name)\
	if (bOn##host && m_soWeaponClass->GetValue( #host, soTemp) && soTemp->GetValue(#name, pFunc))\
		m_h##host##Funcs[WeaponFunc_##name] = pFunc;

	#define GET_VALUE(name)\
		GET_VALUE_EX(Client, name);\
		GET_VALUE_EX(Server, name);

	_SmartScriptObject soTemp(m_pScriptSystem, true);
	HSCRIPTFUNCTION pFunc = 0;

	GET_VALUE(OnInit);
	GET_VALUE(OnActivate);
	GET_VALUE(OnDeactivate);
	GET_VALUE(WeaponReady);
	GET_VALUE(OnAnimationKey);
	GET_VALUE(OnStopFiring);
	GET_VALUE(Reload);
	GET_VALUE(Drop);
	GET_VALUE(OnUpdate);
	GET_VALUE(OnFire);
	GET_VALUE(OnHit);
	GET_VALUE(OnEvent);

	#undef GET_VALUE
	#undef GET_VALUE_EX

	// make sure models are loaded before calling OnInit()
	if (!InitModels())
		return false;

	// call onInit
	ScriptOnInit();

	return true;
}



bool CWeaponClass::InitModels()
{
	// prepare to load models
	ILog *pLog = m_rWeaponSystem.GetGame()->GetSystem()->GetILog();
	assert(pLog);
	ISystem*		pSystem = m_rWeaponSystem.GetGame()->GetSystem();
	assert(pSystem);

	// load 3rd person model
	const char* pszObject;
	if(m_soWeaponClass->GetValue("object", pszObject))
	{
		//pLog->Log("WeaponClass '%s': Object -> '%s'", m_sName.c_str(), pszObject);
		m_pObject = pSystem->GetI3DEngine()->MakeObject(pszObject);
	}

	// load 1st person animated model
	const char* pszCharacter;
	if(m_soWeaponClass->GetValue("character", pszCharacter))
	{
		//pLog->Log("WeaponClass '%s': Character -> '%s'", m_sName.c_str(), pszCharacter);
		m_pCharacter = pSystem->GetIAnimationSystem()->MakeCharacter(pszCharacter, ICryCharManager::nHintModelTransient);
		if (m_pCharacter)
		{
			m_pCharacter->ResetAnimations();
			m_pCharacter->SetFlags(m_pCharacter->GetFlags() | CS_FLAG_DRAW_MODEL | CS_FLAG_UPDATE);
			if (m_rWeaponSystem.IsLeftHanded())
				m_pCharacter->SetScale(Vec3d(-1,1,1));
		}
	}

	return true;
}

bool CWeaponClass::LoadMuzzleFlash(const string& sGeometryName)
{
	ISystem*		pSystem = m_rWeaponSystem.GetGame()->GetSystem();
	assert(pSystem);

	if (m_pMuzzleFlash && !m_pMuzzleFlash->IsSameObject(sGeometryName.c_str(), NULL))
	{
		pSystem->GetI3DEngine()->ReleaseObject(m_pMuzzleFlash);
	}

	m_pMuzzleFlash = pSystem->GetI3DEngine()->MakeObject(sGeometryName.c_str());

	return (m_pMuzzleFlash != NULL);
}

//! Set parameters of this weapon.
WeaponParams *CWeaponClass::AddWeaponParams(const WeaponParams &params )
{
	WeaponParams *p=new WeaponParams;
	*p=params;
	m_vFireModes.push_back(p);

	m_fireParams = params;
	if (params.bAIMode)
	{
		m_nAIMode = m_vFireModes.size()-1;
	}
	return p;
}

//! Get parameters of this weapon.
void CWeaponClass::GetWeaponParams(WeaponParams &params )
{
	params = m_fireParams;
}

bool CWeaponClass::GetModeParams(int mode, WeaponParams &stats)
{
	int x = m_vFireModes.size();

	if ((mode >=0) && (mode < int(m_vFireModes.size())))
	{
		stats = *(m_vFireModes[mode]);
		return true;
	}
	return false;
}

bool CWeaponClass::CancelFire()
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	m_ssoFireTable->BeginSetGetChain();
	m_ssoFireTable->SetValueChain("fire_event_type", (int) eCancel);
	m_ssoFireTable->EndSetGetChain();

	bool bWeaponReady;

	if (m_fireParams.iFireModeType == FireMode_Projectile)
	{
		// Not an instant hit weapon and not a projectile weapon.
		// Special case, let the script handle it
		if (m_fireParams.sProjectileClass.empty())
			return true; 

		// Weapon not ready to fire ?
		ScriptOnEvent(ScriptEvent_FireCancel, *m_ssoFireTable, &bWeaponReady);
		if (!bWeaponReady)
			return true;
	}
	return true;
}

void CWeaponClass::Update(CPlayer *pPlayer)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	if (pPlayer == NULL)
	{
		m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("\001 CWeaponClass::Update - ERROR -> pPlayer == NULL");
		return;
	}
	assert(pPlayer->m_stats.firemode>=0 && pPlayer->m_stats.firemode<int(m_vFireModes.size()));
	if (m_vFireModes[pPlayer->m_stats.firemode] == NULL)
	{
		m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("\001 CWeaponClass::Update - ERROR -> m_vFireModes[pPlayer->m_stats.firemode] == NULL");
		return;
	}
	m_fireParams = *(m_vFireModes[pPlayer->m_stats.firemode]);

	IEntity *pEntity = pPlayer->GetEntity();
	float time = m_rWeaponSystem.GetGame()->GetSystem()->GetITimer()->GetCurrTime();

	ScriptOnUpdate(time-m_fLastUpdateTime, pEntity);

	m_fLastUpdateTime = time;

	ICryCharInstance *pChar = pEntity->GetCharInterface()->GetCharacter(1);
	if (pPlayer->IsMyPlayer() && m_pCharacter && pChar)
	{
		FRAME_PROFILER( "CWeaponClass::UpdateCharacter",GetISystem(),PROFILE_GAME );
		pChar->Update(GetPos());
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//weapon fire,play sounds and perform collision check
//////////////////////////////////////////////////////////////////////
int CWeaponClass::Fire(const Vec3d &origin, const Vec3d &angles, CPlayer *pPlayer, WeaponInfo &winfo, IPhysicalEntity *pIRedirected)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );
#ifdef FIRE_DEBUG
	CryLog("CWeaponClass::Fire");
#endif
	if (pPlayer == NULL)
	{
		m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("\001 CWeaponClass::Fire - ERROR -> pPlayer == NULL");
		return 0;
	}
	assert(pPlayer->m_stats.firemode>=0 && pPlayer->m_stats.firemode<int(m_vFireModes.size()));
	if (m_vFireModes[pPlayer->m_stats.firemode] == NULL)
	{
		m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("\001 CWeaponClass::Fire - ERROR -> m_vFireModes[pPlayer->m_stats.firemode] == NULL");
		return 0;
	}
	m_fireParams = *(m_vFireModes[pPlayer->m_stats.firemode]);

	bool aiming = pPlayer->m_stats.aiming;
	eFireType ft = pPlayer->m_stats.FiringType;
	float fAccuracy = pPlayer->m_stats.accuracy;

	IEntity *pIShooter = pPlayer->GetEntity();
	if (!pIShooter)
		return 0;

	Vec3d weapangles;
	ITimer *pTimer = m_rWeaponSystem.GetGame()->GetSystem()->GetITimer();
	IPhysicalWorld *pPhysicalWorld = m_rWeaponSystem.GetGame()->GetSystem()->GetIPhysicalWorld();

	if(m_fireParams.iFireModeType==FireMode_EngineerTool)
		return 0;				// action is handled in the script

	float currTime = pTimer->GetCurrTime();
	float fFireRate = GetFireRate(ft);

	//m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("Fire: FireModeType = %d", m_fireParams.iFireModeType);

	// Check if we are not shooting faster then fire rate, let the releasing of the trigger pass anyway
	if ( ft&m_fireParams.fire_activation)
	{
		if( (currTime - winfo.fireLastShot) < fFireRate)
			return 0;
	}
	else
	{
		if (ft == eHolding)
			return 0;
		if( (currTime!=winfo.fireTime) && ((currTime - winfo.fireTime) < fFireRate) && (ft!=eReleasing))
			return 0;
	}


	UINT iNumShots = (UINT) floor_tpl((currTime - winfo.fireTime) * (1/fFireRate));
	if ( ft==ePressing)
		iNumShots = 1;

	if (iNumShots > (UINT) pPlayer->m_stats.ammo_in_clip)
		iNumShots = pPlayer->m_stats.ammo_in_clip;
	if (iNumShots <= 0)
		iNumShots = 1;
	winfo.fireTime = currTime;

	//////////////////////////////////////////////////////////////////	
	// This block of code sets up a table of all the parameters needed by the weapon
	// script and then sends an ScriptEvent_Fire event, passing the table as the parameter

	m_ssoHitPosVec=origin;
	m_ssoHitNormVec=angles;//angles;

	Vec3 dir(0,-1,0);

	Matrix44 tm;
	tm.SetIdentity();
	tm=Matrix44::CreateRotationZYX(-angles*gf_DEGTORAD); //NOTE: angles in radians and negated 
	dir = GetTransposed44(tm)*(dir);

	m_ssoHitDirVec=dir;

	float fWaterLevel=m_rWeaponSystem.GetGame()->GetSystem()->GetI3DEngine()->GetWaterLevel(&origin);
	//BULDING PARAMS FOR WeaponScript:Fire
	m_ssoFireTable->BeginSetGetChain();
	m_ssoFireTable->SetValueChain("pos",m_ssoHitPosVec);
	m_ssoFireTable->SetValueChain("angles",m_ssoHitNormVec);
	m_ssoFireTable->SetValueChain("dir",m_ssoHitDirVec);
	m_ssoFireTable->SetValueChain("firemode",winfo.iFireMode);
	m_ssoFireTable->SetValueChain("shooter",pIShooter->GetScriptObject());
	m_ssoFireTable->SetValueChain("bullets", (int) iNumShots);
	m_ssoFireTable->SetValueChain("fire_event_type", (int) ft);

	if (fWaterLevel>origin.z)
	{
		m_ssoFireTable->SetValueChain("underwater",0);
		if (!m_fireParams.bShootUnderwater)
			return 0;
	}
	else
		m_ssoFireTable->SetToNullChain("underwater");

	m_ssoFireTable->EndSetGetChain();

	//only weapons which can fire underwater, can be fired while swimming
	if (!m_fireParams.bShootUnderwater && pPlayer->IsSwimming())
		return 0;

	bool bWeaponReady;

	if (m_fireParams.whizz_sound_radius)
	{
		CXClient *pClient=m_rWeaponSystem.GetGame()->GetClient();
		if (pClient)
		{
			IEntitySystem *pEntSys=pClient->m_pEntitySystem;
			if (pEntSys)
			{
				IEntity *pPlayerEntity=pEntSys->GetEntity(pClient->GetPlayerId());
				if (pPlayerEntity && pPlayer && (pPlayerEntity!=pPlayer->GetEntity()))
				{
					Vec3d ShooterPlayerVec=pPlayerEntity->GetPos()-pPlayer->GetEntity()->GetPos();
					float fDot=ShooterPlayerVec.Dot(dir);
					Vec3d BulletPlayerPos=dir*fDot+pPlayer->GetEntity()->GetPos();

					float fDist2 = GetLengthSquared(Vec3d(BulletPlayerPos-pPlayerEntity->GetPos()));
					if (fDist2<=m_fireParams.whizz_sound_radius)
					{
						m_ssoBulletPlayerPos.Set( BulletPlayerPos );
						m_ssoFireTable->SetValue("BulletPlayerPos", m_ssoBulletPlayerPos );
					}
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	//PROJECTILE
	///////////////////////////////////////////////////////////////////////////////////
	if (m_fireParams.iFireModeType == FireMode_Projectile)
	{
		// Not an instant hit weapon and not a projectile weapon.
		// Special case, let the script handle it
		if (m_fireParams.sProjectileClass.empty())
			return 1; 

		// Weapon not ready to fire ?
		{
			FRAME_PROFILER( "CWeaponClass::ScriptEvent_Fire",GetISystem(),PROFILE_GAME );
			ScriptOnEvent(ScriptEvent_Fire, *m_ssoFireTable, &bWeaponReady);
		}
		if (!bWeaponReady)
			return 0;

		winfo.fireLastShot = currTime;

		// Spawn is server-side
		if(!m_rWeaponSystem.GetGame()->IsServer())
			return 1;

		for (int bullet = 0; bullet < m_fireParams.nBulletpershot; bullet++)
		{
			// Ready to go, we need to spawn a standard projectile on the server

			EntityClass *pClass;
			CEntityDesc ed;

			// Get the entity class of the projectile
			pClass=m_rWeaponSystem.GetGame()->GetClassRegistry()->GetByClass(m_fireParams.sProjectileClass.c_str());
			if(!pClass)
			{
#ifdef _DEBUG
				m_rWeaponSystem.GetGame()->GetClassRegistry()->Debug();			// OutputDebugString
#endif
				assert(false);
				return 1;
			}

			ed.ClassId = pClass->ClassId;
			ed.className = pClass->strClassName.c_str();
			ed.pos = m_ssoHitPosVec.Get();
			ed.angles = angles;
			IEntity* pEntity;

			// Attempt to spawn the object using the class ID
			if((pEntity = m_rWeaponSystem.GetGame()->GetSystem()->GetIEntitySystem()->SpawnEntity(ed)) == NULL)
			{
				// Spawn failed
				assert(false);
			}
			{
				FRAME_PROFILER( "CWeaponClass::Launch",GetISystem(),PROFILE_GAME );
				// Call launch of the projectile to get it going
				string sClassName=pEntity->GetEntityClassName();
				m_pScriptSystem->BeginCall(sClassName.c_str(),"Launch");
				m_pScriptSystem->PushFuncParam(pEntity->GetScriptObject()); // self
				m_pScriptSystem->PushFuncParam(GetScriptObject()); // weapon entity
				m_pScriptSystem->PushFuncParam(pIShooter->GetScriptObject()); // shooter
				m_pScriptSystem->PushFuncParam(m_ssoHitPosVec); // position
				m_pScriptSystem->PushFuncParam(m_ssoHitNormVec); // angles
				m_pScriptSystem->PushFuncParam(m_ssoHitDirVec); // direction
				m_pScriptSystem->EndCall(bWeaponReady);
			}
		}

		return 1;
	}
	///////////////////////////////////////////////////////////////////////////////////
	//INSTANT BULLET
	///////////////////////////////////////////////////////////////////////////////////

	bool bFirstBullet = true;
	int bullet = 0;
	for (bullet = 0; bullet < m_fireParams.nBulletpershot; bullet++)
	{
		// Do shoot for each bullet.
		int res=0;
		float time = pTimer->GetCurrTime();

		weapangles = angles;


		//TRACE("pPlayer->m_stats.random_seed=%d fAccuracy=%f",m_pXSystem->GetRandomSeed(),fAccuracy);
		//take into account weapon's accuracy(the first bullet is 100% accurate
		{
			uint8 ucSeed = pPlayer->m_SynchedRandomSeed.GetRandomSeedC();

//			GetISystem()->GetILog()->Log(">> Bullet                           Bullet %d %d",(int)pPlayer->GetEntity()->GetId(),(int)((bullet<<3)+ucSeed));			// debug

			CalculateWeaponAngles((bullet<<3)+ucSeed, &weapangles, pPlayer->CalculateAccuracyFactor(fAccuracy));
			pPlayer->m_SynchedRandomSeed.IncreaseRandomSeedC();
		}

#define MAX_HITS 5
		static ray_hit hits[MAX_HITS];

		memset(hits,0,sizeof(hits));

		// Marco's change to take leaning into account:
		// trasform the weapons angles using the same as the camera matrix
		// create a vector pointing down the z-axis
		Vec3d dir(0,-1,0);
		Matrix44 tm = Matrix44::CreateRotationZYX(-weapangles*gf_DEGTORAD); //NOTE: angles in radians and negated 

		dir = GetTransposed44(tm)*(dir);

		dir*=m_fireParams.fDistance;

		IPhysicalEntity *skip = pIShooter->GetPhysics();

		IPhysicalEntity *skipMore = NULL;

		//	[kirill] we want to skip players vehicle - so you can NOT shoot your own car/boat
		if(pPlayer->GetVehicle())
			skipMore = pPlayer->GetVehicle()->GetEntity()->GetPhysics();

		bool bMeleeHit = false;

		// please, leave this in for now ... it is only temporary!!
		if (m_fireParams.iFireModeType == FireMode_Melee)
		{
			FRAME_PROFILER( "CWeaponClass::FireMode_Melee",GetISystem(),PROFILE_GAME );
			// center of the 'hit box' of the melee weapon
			Vec3 boxCenter = origin + 0.5f * dir;
			Vec3 offset(0.5f * dir.Length(), 0.5f * dir.Length(), 0.5f * dir.Length());
			IPhysicalEntity **pList;

			// get all entities in the hit box
			int num = pPhysicalWorld->GetEntitiesInBox(boxCenter-offset, boxCenter+offset, pList, ent_all);

			ray_hit closestHit;
			// check each entity in the box			
			for (int i = 0; i < num; ++i)
			{
				IEntity* pEntity = ((IEntity*)pList[i]->GetForeignData(OT_ENTITY));
				// skip the shooter
				if (pEntity && pEntity == pIShooter)
					continue;

				// cast a 'fat' line segment
				if (pPhysicalWorld->CollideEntityWithBeam(pList[i], origin, dir, 0.3f, hits))
				{
					if (res == 0 || hits[0].dist < closestHit.dist)
					{
						closestHit = hits[0];
					}
					bMeleeHit = true;
					res = 1;
				}
			}

			if (bMeleeHit)
				hits[0] = closestHit;
		}

		// we do the regular raycast if we did not have a melee hit
		if (!bMeleeHit)
		{
			FRAME_PROFILER( "CWeaponClass::RayWorldIntersection",GetISystem(),PROFILE_GAME );
			res = pPhysicalWorld->RayWorldIntersection(origin, dir, ent_all, rwi_separate_important_hits,hits,MAX_HITS, skip, skipMore);

#ifdef FIRE_DEBUG
				m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("CWeaponClass::Fire RayWorldIntersection d",res);
				for(int i=0;i<res;i++)
				{
					m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("CWeaponClass::Fire hits: dist=%.2f type=%d pos=(%.2f %.2f %.2f)",
						hits[i].dist,hits[i].pCollider?hits[i].pCollider->GetType():-1,hits[i].pt.x,hits[i].pt.y,hits[i].pt.z);				
				}
#endif

		}

		bool bCurve = false;

		Vec3d currangles= weapangles;
		Vec3d currpos	= origin;

		// First bullet ?
		if (bFirstBullet)
		{
			FRAME_PROFILER( "CWeaponClass::FirstBullet_ScriptOnFire",GetISystem(),PROFILE_GAME );

			bFirstBullet=false;
			// Call weapon script to play client side effects and to determine if the weapon is ready to fire
			currangles=weapangles;
			if (res != 0)
			{
				dir=ConvertToRadAngles(currangles);
				m_ssoHitDirVec = dir;
			}
			m_ssoFireTable->SetValue("dir",m_ssoHitDirVec);
      
			m_ssoHitPt = (Vec3d) hits[0].pt;
			m_ssoFireTable->SetValue("HitPt", m_ssoHitPt);

			m_ssoFireTable->SetValue("HitDist", hits[0].dist);

			if (!ScriptOnFire(*m_ssoFireTable))
				return false;
		}

		int nCount=0;
		m_nLastMaterial=-1;
		IEntity *pLastContact=0;
		pe_status_living sl;

		// the number of hits to process
		//int nHits = min(res, MAX_HITS);	
		// [Anton] it's possible that we don't have solid hits, in this case we'll have to 
		// process res+1 slots, starting from slot 0
		{
			FRAME_PROFILER( "CWeaponClass::BulletLoop",GetISystem(),PROFILE_GAME );

			for(nCount=0;nCount<MAX_HITS;nCount++)
			{
			IEntity *centycontact=NULL;	
			IEntityRender	*entrendercontact=NULL;

				if(hits[nCount].dist<=0)
					continue;
				int objecttype = OT_TERRAIN;
				if (res && hits[nCount].dist>0 && hits[nCount].pCollider)
				{
					int physType = hits[nCount].pCollider->GetiForeignData();
					if (physType == OT_BOID)
					{
						CBoidObject *pBoid = (CBoidObject*)hits[nCount].pCollider->GetForeignData(OT_BOID);
						if (pBoid)
						{
							string surfaceName;
							Vec3 vImpulse = GetNormalized(dir)*float(m_fireParams.iImpactForceMul)*float(m_fireParams.iImpactForceMulFinal);
							pBoid->Kill( hits[nCount].pt,vImpulse,surfaceName );
							hits[nCount].surface_idx = m_rWeaponSystem.GetGame()->m_XSurfaceMgr.GetSurfaceIDByMaterialName( surfaceName.c_str() );
							objecttype = OT_BOID;
						}
					}
					else if (physType == OT_RIGID_PARTICLE)
					{
						Vec3 vImpulse = GetNormalized(dir)*float(m_fireParams.iImpactForceMul);
						// Add impulse to it.
						pe_action_impulse ai;
						ai.point = hits[nCount].pt;
						ai.impulse = vImpulse;
						hits[nCount].pCollider->Action(&ai);
					}
					else if (physType == OT_STAT_OBJ)
					{
						entrendercontact = (IEntityRender *)hits[nCount].pCollider->GetForeignData(1);
#ifdef FIRE_DEBUG
						m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("IEntityRender %x",centycontact);
#endif
						objecttype = OT_STAT_OBJ;
					}
					else
					{
						centycontact = (IEntity *)hits[nCount].pCollider->GetForeignData();
#ifdef FIRE_DEBUG
						m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("centycontact %x",centycontact);
#endif

						if (centycontact && centycontact->IsGarbage())
						{
							res = 0;
						}
						if (centycontact && (!centycontact->IsGarbage()))
							objecttype = OT_ENTITY;
						else 
							objecttype = OT_STAT_OBJ;
					}
				}

				/*
				// see if it's knife/shocker and
				// target is not player - don't process hit
				void *ppInterface;
				bool bTragetIsPalyer = (objecttype==OT_ENTITY && 
				centycontact->GetContainer() && 
				centycontact->GetContainer()->QueryContainerInterface(CIT_IPLAYER, &ppInterface));
				if( m_fireParams.fDistance<3 && !bTragetIsPalyer) 
				continue;
				/*/
				float fDmgDrop = hits[nCount].dist * m_fireParams.fDamageDropPerMeters;

				// If collision check interescted something (for instant weapons).
				SWeaponHit hit;
				hit.pos = (Vec3d)hits[nCount].pt;
				hit.dir = GetNormalized(dir);
				hit.normal = (Vec3d)hits[nCount].n;
				hit.target = centycontact;
				hit.targetStat = entrendercontact;
				hit.ipart = hits[nCount].partid;
				hit.objecttype =objecttype;
				hit.shooter = pIShooter;
				hit.weapon = GetScriptObject();
				hit.projectile = 0; // Instant weapon, no projectiles.
				hit.damage = ((float)((float)(m_fireParams.nDamage-fDmgDrop) * iNumShots));

				//			if(hit.damage<1)
				//m.m. test				hit.damage = 1;
				if(centycontact!=0 && centycontact==pLastContact)
				{
					hit.damage=0;
				}
				pLastContact=centycontact;
				hit.surface_id=hits[nCount].surface_idx;
				hit.weapon_death_anim_id = m_fireParams.iDeathAnim;

				if (!(skip && skip->GetStatus(&sl) && sl.pGroundCollider==hits[nCount].pCollider))
				{
					hit.iImpactForceMul = m_fireParams.iImpactForceMul;
					hit.iImpactForceMulFinal = m_fireParams.iImpactForceMulFinal;
					hit.iImpactForceMulFinalTorso = m_fireParams.iImpactForceMulFinalTorso;
				}
				else // don't add impulse to the object we are standing on
					hit.iImpactForceMul=hit.iImpactForceMulFinal=hit.iImpactForceMulFinalTorso = 0;
				ProcessHit(hit);
			}
		}
	} // bullet loop

	//	if (m_fireParams.fire_activation & ePressing)
	{
		pPlayer->m_bWeaponJustFired = true;
	}

	winfo.fireLastShot = currTime;

	// AI Autobalance feature stuff ... tell the system how many bullets were fired
	if (iNumShots > 0 && pPlayer->IsMyPlayer())
	{
		FRAME_PROFILER( "CWeaponClass::AutoBalance",GetISystem(),PROFILE_GAME );
		IAutoBalance *pAutoBalance=GetISystem()->GetAISystem()->GetAutoBalanceInterface();

		if(pAutoBalance)					// e.g. Multiplyer doesn't used this feature
			pAutoBalance->RegisterPlayerFire(m_fireParams.nBulletpershot);
	}

	return iNumShots;
}

//////////////////////////////////////////////////////////////////////////
void CWeaponClass::ProcessHitTarget(const SWeaponHit &hit)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	if (hit.target!=NULL)
	{
#ifdef FIRE_DEBUG
		m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("hit.target!=NULL id=%d target=%s %x",hit.target->GetId(),(const char *)hit.target->GetName(),hit.target->GetScriptObject());
#endif
		m_ssoProcessHit->SetValueChain("target_id",hit.target->GetId());
		if (hit.target->GetScriptObject())
			m_ssoProcessHit->SetValueChain("target",hit.target->GetScriptObject());
		else
			m_ssoProcessHit->SetToNullChain("target");
		m_ssoProcessHit->SetToNullChain("targetStat");
	}
	else 	if (hit.targetStat!=NULL)
	{
		// Make user data for pointer.
		USER_DATA ud = m_pScriptSystem->CreateUserData( (ULONG_PTR)hit.targetStat,USER_DATA_POINTER );
		m_ssoProcessHit->SetValueChain("targetStat",ud);

		m_ssoProcessHit->SetToNullChain("target");
		m_ssoProcessHit->SetToNullChain("target_id");
	}
	else
	{
#ifdef FIRE_DEBUG
		m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("hit.target==NULL");
#endif

		m_ssoProcessHit->SetToNullChain("targetStat");
		m_ssoProcessHit->SetToNullChain("target");
		m_ssoProcessHit->SetToNullChain("target_id");
		m_ssoProcessHit->SetValueChain("objtype",hit.objecttype);
	}	
}

//////////////////////////////////////////////////////////////////////////
void CWeaponClass::ProcessHit(const SWeaponHit &hit)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );
#ifdef FIRE_DEBUG
	m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("CWeaponClass::ProcessHit target=%s pos=(%.2f %.2f %.2f)",
		hit.target?hit.target->GetName():"nil",hit.pos.x,hit.pos.y,hit.pos.z);
#endif

	IScriptObject *pTargetMaterial;
	float fWaterLevel=m_rWeaponSystem.GetGame()->GetSystem()->GetI3DEngine()->GetWaterLevel(&hit.pos);

	Vec3d vHitPoint;
	Vec3d vShooter=hit.shooter->GetCamera()->GetPos();
	///////////////////////////////////////////////////////////////
	//CHECK IF THE HIT POS IS UNDERWATER
	///////////////////////////////////////////////////////////////
	if(hit.pos.z<fWaterLevel && vShooter.z > fWaterLevel)
	{
		Vec3d vWaterNormal(0,0,1);
		Vec3d vTarget=hit.pos;
		Vec3d vNorm;

		float fHTotal = vShooter.z - vTarget.z;
		float fHHit = vShooter.z - fWaterLevel;
		vHitPoint = vShooter + fHHit*(vTarget-vShooter)/fHTotal;
		m_ssoHitPosVec=vHitPoint;

		m_ssoHitNormVec=vWaterNormal;
		m_ssoHitDirVec=hit.dir;
		m_ssoProcessHit->BeginSetGetChain();
		m_ssoProcessHit->SetValueChain("pos",m_ssoHitPosVec);
		m_ssoProcessHit->SetValueChain("normal",m_ssoHitNormVec);
		m_ssoProcessHit->SetValueChain("dir",m_ssoHitPosVec);		
		m_ssoProcessHit->SetValueChain("objtype",hit.objecttype);
		m_ssoProcessHit->SetValueChain("ipart",hit.ipart);
		m_ssoProcessHit->SetValueChain("shooter",hit.shooter->GetScriptObject());
		m_ssoProcessHit->SetValueChain("weapon",hit.weapon);

		// [marco] decrease damage if the hit pos is underwater
		m_ssoProcessHit->SetValueChain("damage",hit.damage*0.5f); 

		if(pTargetMaterial=m_rWeaponSystem.GetGame()->m_XSurfaceMgr.GetMaterialByName("mat_water"))
		{
			m_ssoProcessHit->SetValueChain("inwater",(bool)true );
			m_ssoProcessHit->SetValueChain("target_material",pTargetMaterial);
		}
		else
		{
			m_ssoProcessHit->SetToNullChain("target_material");
		}
		m_ssoProcessHit->EndSetGetChain();
		ScriptOnHit(*m_ssoProcessHit); 		
	}
	//IF THE HIT IS LESS THAN 1 METER UNDERWATER
	//HIT!
	//if(!((hit.pos.z-fWaterLevel)<-1))
	// [marco] I've removed this check - dangerous as the player
	// could stay close to the water surface and never get hit
	m_ssoHitPosVec=hit.pos;
	m_ssoHitNormVec=hit.normal;
	m_ssoHitDirVec=hit.dir;
	_VERIFY(m_ssoProcessHit->BeginSetGetChain());
	m_ssoProcessHit->SetValueChain("pos",m_ssoHitPosVec);
	m_ssoProcessHit->SetValueChain("normal",m_ssoHitNormVec);
	m_ssoProcessHit->SetValueChain("dir",m_ssoHitDirVec);	
	m_ssoProcessHit->SetValueChain("ipart",hit.ipart);
	m_ssoProcessHit->SetValueChain("shooter",hit.shooter->GetScriptObject());
	m_ssoProcessHit->SetValueChain("weapon",hit.weapon);
	m_ssoProcessHit->SetValueChain("damage",hit.damage);
	m_ssoProcessHit->SetValueChain("weapon_death_anim_id", hit.weapon_death_anim_id);
	m_ssoProcessHit->SetValueChain("impact_force_mul", hit.iImpactForceMul);
	m_ssoProcessHit->SetValueChain("impact_force_mul_final", hit.iImpactForceMulFinal);
	m_ssoProcessHit->SetValueChain("impact_force_mul_final_torso", hit.iImpactForceMulFinalTorso);

	if (m_fireParams.iFireModeType == FireMode_Melee)
		m_ssoProcessHit->SetValueChain("melee", true);
	else
		m_ssoProcessHit->SetValueChain("melee", false);


	// [marco] hit target code moved into a common function
	ProcessHitTarget(hit);

	if(pTargetMaterial=m_rWeaponSystem.GetGame()->m_XSurfaceMgr.GetMaterialBySurfaceID(hit.surface_id))
	{
		m_ssoProcessHit->SetValueChain("target_material",pTargetMaterial);
		//avoid to play the same materials sound twice in a row(avoid phasing)
		if(m_nLastMaterial==hit.surface_id)
		{
			m_ssoProcessHit->SetToNullChain("play_mat_sound");
		}
		else
		{
			m_ssoProcessHit->SetValueChain("play_mat_sound",1);
			m_nLastMaterial=hit.surface_id;
		}
	}
	else
	{
		m_ssoProcessHit->SetToNullChain("target_material");
	}
	if (hit.projectile)
		m_ssoProcessHit->SetValueChain("projectile",hit.projectile->GetScriptObject());
	m_ssoProcessHit->EndSetGetChain();

	ScriptOnHit(m_ssoProcessHit);

	if (hit.target)
	{
#ifdef FIRE_DEBUG
		m_rWeaponSystem.GetGame()->GetSystem()->GetILog()->Log("CWeaponClass::ProcessHit OnDamage");
#endif

		hit.target->OnDamage(m_ssoProcessHit);
		if(hit.target->IsStatic())
			hit.target->AddImpulse( hit.ipart, hit.pos, hit.dir*(float)(hit.iImpactForceMul));
	}
}


void CWeaponClass::CalculateWeaponAngles(BYTE random_seed, Vec3d* pVector, float fAccuracy)
{
	IXSystem *pXSystem = m_rWeaponSystem.GetGame()->GetXSystem();

	//take into account weapon's accuracy
	if(m_fireParams.fMinAccuracy<1) //if 100 accurate just return
	{
		float spread = 15.0f;

		float r1, r2;
		
		for(int i=0; i<256; ++i)
		{
			r1 = CSynchedRandomSeed::GetRandTable(random_seed+i);
			r2 = CSynchedRandomSeed::GetRandTable(random_seed+3+i);
			if ((r1*r1 + r2*r2) <= 1.0f) break;
		}

		r1 = -0.5f*spread + spread*r1;
		r2 = -0.5f*spread + spread*r2;

		pVector->x += r1 * fAccuracy;
		pVector->z += r2 * fAccuracy;
	}
}

