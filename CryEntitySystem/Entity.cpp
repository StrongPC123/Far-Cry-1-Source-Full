//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Entity.cpp
//  Description: Basic Entity functions
//  An entity can be any object (player,car,rocket,items etc.) with behaviours 
//  defined by script
//
//	History:
//	-Feb 14,2001:Originally created by Marco Corbetta
//	-: modified by Everyone :)
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "EntitySystem.h"

#include <string.h>
#include <algorithm>

#include "Entity.h"
#include <Stream.h>
#include <IScriptSystem.h>

#include <IProcess.h>
#include <ITimer.h>
#include <IGame.h>
#include <ISystem.h>
#include <IRenderer.h>
#include "CryHeaders.h"
#include <ILipSync.h>

#include <I3DEngine.h>
#include <ILog.h>
#include <IAISystem.h>

#include <IAgent.h>
#include <float.h>

//#include "list2.h"

#define POS_EPSILON 0.0001f
#define ANGLES_EPSILON 0.0001f

#define SCRIPT_SERVER_STATE	"Server"
#define SCRIPT_CLIENT_STATE	"Client"

#if defined(_DEBUG) && !defined(LINUX)
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

//////////////////////////////////////////////////////////////////////////
CEntity::EntPartEmitter::~EntPartEmitter()
{
	if (pEmitter)
		GetISystem()->GetI3DEngine()->DeleteParticleEmitter( pEmitter );
}

//////////////////////////////////////////////////////////////////////////
//! This structure must 100% match EScriptStateFunctions enum.
static const char*	s_ScriptStateFunctions[] = 
{
	"OnBeginState",
	"OnEndState",
	"OnUpdate",
	"OnContact",
	"OnTimer",
	"OnEvent",
	"OnDamage",
	"OnEnterArea",
	"OnLeaveArea",
	"OnProceedFadeArea",
	"OnBind",
	"OnUnBind",
	"OnMove",
	"OnCollide",
	"OnStopRollSlideContact",
	// reserved.
	"",
};

//////////////////////////////////////////////////////////////////////////
// checks if the input angles are valid and returns them;
// if a component is invalid, then returns 0 in that component
inline Vec3d ValidateAngles(Vec3d v)
{
	if (!(-1e+9 < v.x && v.x < 1e+9))
		v.x = 0;
	if (!(-1e+9 < v.y && v.y < 1e+9))
		v.y = 0;
	if (!(-1e+9 < v.z && v.z < 1e+9))
		v.z = 0;
	return v;
}

//////////////////////////////////////////////////////////////////////
CEntity::CEntity(CEntitySystem *pEntitySystem,  ISystem * pISystem,IScriptSystem *pSS):m_pAnimationEventParams(pSS),
m_pObjectCollide(pSS),m_vObjPosCollide(pSS),m_vObjVelCollide(pSS),m_pSplashList(pSS) //,m_vNormDirCollide(pSS)
{
	// m_pCurrIndoorSector = (CIndoorSector*)-1; // not defined at begining
	//bigger slot with a character	
	m_nLastVisibleFrameID = 0;
	m_bRecalcBBox = true;
	//[kirill]  no slipping entities for MP game - on client it can never wakeup
	if(GetISystem()->GetIGame()->GetModuleState(EGameMultiplayer))
		m_bSleeping = false;
	else
		m_bSleeping = true;
	m_matParentMatrix.SetIdentity();
	m_bForceBindCalculation = false;
	m_nMaxCharNum=0;
	m_bHidden=false;
	m_cLastStateID=0;
	m_bTrackable=false;
	m_fLastSubMergeFracion=0.0f;
	m_flags = 0;
	m_nTimer=-1;
	//m_nStartTimer=0;
	m_pLipSync=NULL;
	m_center(0, 0, 0);
	m_angles(0, 0, 0);
	m_physic = m_physPlaceholder = NULL;
	m_pPhysState = 0;
	m_iPhysStateSize = 0;
	m_iPhysType = PHYS_NONE;
	m_physicEnabled = true;
	m_fRollTimeout=0.0f;
	m_fSlideTimeout=0.0f;
	//m_static = false;
	for (int k = 0; k < MAX_ANIMATED_MODELS; k++)
	{
		m_pCryCharInstance[k] = NULL;
		m_pCharPhysPlaceholders[k] = 0;
	}

	// m_qsplat=NULL;
	SetName("No entity loaded");

	m_pSaveFunc = 0;
	m_pLoadFunc = 0;
	m_pLoadRELEASEFunc = 0;
	m_pLoadPATCH1Func = 0;

	m_awakeCounter = 0;
	m_bUpdate = true;
	m_bIsBound = false;
	m_pScriptObject = 0;

	m_pClientState = 0;
	m_pServerState = 0;

	//m_vBoxMin(0,0,0);
	//m_vBoxMax(0,0,0);	
	m_vBoxMin=m_vForceBBoxMin=SetMaxBB();
	m_vBoxMax=m_vForceBBoxMax=SetMinBB();


	m_fUpdateRadius = 0;
	m_fRadius = 0;
	m_fScale = 1;

	m_registeredInSector = false;
	//  m_pLSource=0;

	m_bGarbage = false;

	// m_flags|=ETY_FLAG_NEED_UPDATE;    
	m_flags |= ETY_FLAG_DESTROYABLE;

	m_pAIObject = 0;

	m_nID = 0;
	m_netPresence = true;

//	m_dirtyFlags = 0;

	m_pCamera = NULL;

	//How many times this entity was entity was serialized
	//m_nWriteNumber=0;
	//m_nLastNameUpdate=0;

	m_pEntitySystem = pEntitySystem;
	m_pScriptSystem =pSS;
	m_pContainer = NULL;

	m_pISystem = pISystem;
	
	m_bSave = true;
	m_pHeadBone = 0;

	//m_nIndoorArea=-1; //not set yet

	//register the default state
	RegisterState("");
	m_bForceBBox = false;

  m_vPrevDrawCenter(-1000,-1000,-1000);
  m_vPrevDrawAngles(-1000,-1000,-1000);

	m_pDynLight = NULL;
	m_pEntityRenderState = 0;//m_pISystem->GetI3DEngine()->MakeEntityRenderState();
	m_nSteeringWheelSlot=-1; // not found yet
	m_pOnCollide=NULL;
	m_pOnStopRollSlideContact=NULL;
	m_pParticleEmitters = NULL;
	m_dwRndFlags=0;

	m_fWaterDensity=1000.0f;
  m_eUpdateVisLevel = eUT_Always;

//  memset(m_narrDrawFrames,0,sizeof(m_narrDrawFrames));
	m_bHandIK = false;
	m_fLastCollideTime=0;
	m_fLastSplashTime = 0;
	m_bInitialized=false;
	m_fTimeRolling = m_fTimeNotRolling = 0;
	m_PrevVertVel = 0.0f;
	m_vPrevVel.Set(0,0,0);

	m_pBBox=NULL;
	m_pColliders = NULL;
	m_bTrackColliders = false;
	m_bUpdateSounds = false;
	m_bUpdateAI = false;
	m_bUpdateEmitters = false;
	m_bUpdateScript = false;
	m_bUpdateContainer = false;
	m_bUpdateCharacters = false;
	m_bUpdateCamera = false;
	m_bUpdateBinds = false;
	m_bIsADeadBody = 0;
	m_bEntityLightsOn = 1;
	m_bVisible = m_bWasVisible = 0;
	m_idBoundTo = 0;
	m_bStateClientside=false;

	m_fScriptUpdateRate = 0;
	m_fScriptUpdateTimer = 0;
	m_fCharZOffsetCur = m_fCharZOffsetTarget = 0;
	m_nFlyingFrames = 100;
}

//////////////////////////////////////////////////////////////////////
CEntity::~CEntity()
{
	// Destructor of entity must be almost empty.
	// All release functions must be in CEntity:ShutDown
	//m_pISystem->GetI3DEngine()->UnRegisterEntity(this);

	// [marco] make sure the timer gets stopped since is not
	// part of the entity anymore
	KillTimer();

	// free m_pEntityRenderState member
	m_pISystem->GetI3DEngine()->FreeEntityRenderState(this);
	m_pEntityRenderState = 0;
}

//////////////////////////////////////////////////////////////////////
void CEntity::ShutDown()
{		
	std::vector < CEntityObject>::iterator it;

	ShutDownScript();

	SAFE_RELEASE( m_pContainer );
	m_bUpdateContainer = false;

	if (m_pScriptObject)
		m_pScriptObject->Release();

	//if (HaveCamera() && m_lstBindings.empty())
	//SetCamera(0);

	SAFE_RELEASE( m_pCamera );
	m_bUpdateCamera = false;

	if (m_pDynLight)
	{
		delete m_pDynLight;
		m_pDynLight = NULL;
	}

	UnregisterInSector();

	// todo: remove this line when 3dengine will stop using deleted pointers to entity
	// prev line should be enough
	m_pISystem->GetI3DEngine()->UnRegisterInAllSectors(this);

	for (it = m_objects.begin(); it < m_objects.end(); it++) 
	{
		if ((* it).object)
		{			
			m_pISystem->GetI3DEngine()->ReleaseObject((* it).object);// NOTE
		}
		/*		if ((* it).image)
		{
		m_pISystem->GetIRenderer()->RemoveTexture((* it).image);
		}*/
	}
	std::vector < IStatObj* >::iterator itaux;
	for (itaux = m_auxObjects.begin(); itaux < m_auxObjects.end(); itaux++) if (*itaux)
		m_pISystem->GetI3DEngine()->ReleaseObject(*itaux);
	m_auxObjects.clear();

	ReleaseLipSyncInterface();	// we release lipsync before we destroy the character...
	// unload character
	for (int k = 0; k < MAX_ANIMATED_MODELS; k++)
	{
		if (m_pCryCharInstance[k])
			m_pISystem->GetIAnimationSystem()->RemoveCharacter(m_pCryCharInstance[k]);  
		if (m_pCharPhysPlaceholders[k])
			m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_pCharPhysPlaceholders[k]);
		m_pCryCharInstance[k] = NULL;
		m_pCharPhysPlaceholders[k] = NULL;
	}

	if (m_physic)
	{
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);
		m_physic = NULL;
	}
	if (m_physPlaceholder)
	{
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physPlaceholder);
		m_physPlaceholder = NULL;
	}
	if (m_pBBox)
	{
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_pBBox);
		m_pBBox=NULL;
	}
	if (m_pPhysState)
	{
		delete[] m_pPhysState;
		m_pPhysState = 0;
	}

	//	delete m_pLSource;
	//	m_pLSource=0;

	if (m_pAIObject)
	{
		m_pISystem->GetAISystem()->RemoveObject(m_pAIObject);
		m_pAIObject = 0;
		m_bUpdateAI = false;
	}

	if (!m_lstBindings.empty())
	{
		BINDLISTItor bi;
		for (bi=m_lstBindings.begin();bi!=m_lstBindings.end();bi++)
		{
			m_pEntitySystem->ReleaseMark( (*bi) );
		}
	}
	m_bUpdateBinds = false;

	//////////////////////////////////////////////////////////////////////////
	// Stop All attached sounds.
	//////////////////////////////////////////////////////////////////////////
	for (SoundsList::iterator sndit = m_lstAttachedSounds.begin(); sndit != m_lstAttachedSounds.end(); ++sndit)
	{
		SAttachedSound &snd = *sndit;
		if (snd.pSound)
			snd.pSound->Stop();
	}
	m_lstAttachedSounds.clear();
	m_bUpdateSounds = false;
	//////////////////////////////////////////////////////////////////////////

	// free part emitters
	SAFE_DELETE( m_pParticleEmitters );
	m_bUpdateEmitters = false;

	SAFE_RELEASE( m_pLipSync );
	SAFE_RELEASE( m_pCamera );

	//////////////////////////////////////////////////////////////////////////
	// Delete colliders list.
	//////////////////////////////////////////////////////////////////////////
	delete m_pColliders;
	m_pColliders = NULL;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::ShutDownScript()
{
	if (m_pClientState)
	{
		// Call Client.OnShutDown
		_SmartScriptObject pClient(m_pScriptSystem, true);
		if (m_pScriptObject->GetValue(SCRIPT_CLIENT_STATE,pClient))
		{
			HSCRIPTFUNCTION pOnInitFunc = 0;
			if (pClient->GetValue( SCRIPT_SHUTDOWN,pOnInitFunc ))
			{
				m_pScriptSystem->BeginCall(pOnInitFunc);
				m_pScriptSystem->PushFuncParam(m_pScriptObject);
				m_pScriptSystem->EndCall();
				m_pScriptSystem->ReleaseFunc(pOnInitFunc);
			}
		}
	}
	if (m_pServerState)
	{
		// Call Server.OnShutDown
		_SmartScriptObject pServer(m_pScriptSystem, true);
		if (m_pScriptObject->GetValue(SCRIPT_SERVER_STATE,pServer))
		{
			HSCRIPTFUNCTION pOnInitFunc = 0;
			if (pServer->GetValue( SCRIPT_SHUTDOWN,pOnInitFunc ))
			{
				m_pScriptSystem->BeginCall(pOnInitFunc);
				m_pScriptSystem->PushFuncParam(m_pScriptObject);
				m_pScriptSystem->EndCall();
				m_pScriptSystem->ReleaseFunc(pOnInitFunc);
			}
		}
		else
		{
			// Call OnShutDown
			HSCRIPTFUNCTION pOnInitFunc = 0;
			// Default fallback if Server table not exist.
			if (m_pScriptObject->GetValue(SCRIPT_SHUTDOWN,pOnInitFunc))
			{
				m_pScriptSystem->BeginCall(pOnInitFunc);
				m_pScriptSystem->PushFuncParam(m_pScriptObject);
				m_pScriptSystem->EndCall();
				m_pScriptSystem->ReleaseFunc(pOnInitFunc);
			}
		}
	}

	if (m_pClientState)
	{
		ReleaseStateTable(*m_pClientState);
	}
	if (m_pServerState)
	{
		ReleaseStateTable(*m_pServerState);
	}

	SAFE_DELETE( m_pServerState );
	SAFE_DELETE( m_pClientState );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetContainer(IEntityContainer* pContainer)
{
	m_pContainer = pContainer;
	if (m_pContainer)
		m_bUpdateContainer = true;
	else
		m_bUpdateContainer = false;
}

// set the bbox of the entity
//////////////////////////////////////////////////////////////////////
void CEntity::SetBBox(const Vec3d &mins, const Vec3d &maxs)
{
	UnregisterInSector();
	m_vForceBBoxMin=m_vBoxMin = mins;
	m_vForceBBoxMax=m_vBoxMax = maxs;

	if ((m_vBoxMin-m_center).Length() > (m_vBoxMax-m_center).Length())
		m_fRadius = (m_vBoxMin-m_center).Length();
	else
		m_fRadius = (m_vBoxMax-m_center).Length();

	if (m_bTrackColliders)
		CreatePhysicsBBox();

	m_bForceBBox = true;
	RegisterInSector();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::GetBBox(Vec3d &mins, Vec3d &maxs)
{
	mins = m_vBoxMin + m_center;
	maxs = m_vBoxMax + m_center;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::GetRenderBBox(Vec3d &mins, Vec3d &maxs)
{
	mins = m_vBoxMin + m_center;
	maxs = m_vBoxMax + m_center;

	// include attached light into bbox
	if(m_pDynLight)
	{ 
		mins.CheckMin(m_center-Vec3d(m_pDynLight->m_fRadius,m_pDynLight->m_fRadius,m_pDynLight->m_fRadius));
		maxs.CheckMax(m_center+Vec3d(m_pDynLight->m_fRadius,m_pDynLight->m_fRadius,m_pDynLight->m_fRadius));
	}

	// include container lights into bbox
	if(m_pContainer)
	{
		float fContainerLightRadius = m_pContainer->GetLightRadius();
		mins.CheckMin(m_center-Vec3d(fContainerLightRadius,fContainerLightRadius,fContainerLightRadius));
		maxs.CheckMax(m_center+Vec3d(fContainerLightRadius,fContainerLightRadius,fContainerLightRadius));
	}

	// make bbox at least 2 meter size particle system to make lighting calculations more stable
	if(m_pParticleEmitters && m_pParticleEmitters->size())
	{
		mins-=Vec3d(1,1,1);
		maxs+=Vec3d(1,1,1);
	}
}

//////////////////////////////////////////////////////////////////////////
float CEntity::GetRenderRadius() const
{ 
	return m_pDynLight ? max(m_fRadius,m_pDynLight->m_fRadius) : m_fRadius ;
}

// set the position of the entity
//////////////////////////////////////////////////////////////////////
void CEntity::SetPos(const Vec3d &pos, bool	bWorldOnly /* = true */)
{
	// if flag calc physic is set then force this new position
	// to the physic system

	if (!bWorldOnly && (m_bIsBound||m_bForceBindCalculation))
	{
		if (m_realcenter != pos)
		{
			// If moved
			m_awakeCounter = 1;
		}
		m_realcenter = pos;
		return;
	}

	//if (pos == m_center) - Kirill
	//	return;

	MoveTo(pos);
}

//set entity's scale
//////////////////////////////////////////////////////////////////////
void CEntity::SetScale( float scale )
{
	if(scale<0)
		scale=0;

	m_fScale = scale;

	// Set scale of physical entity if present.
	if (m_physic)
	{
		pe_params_pos params;
		params.scale = m_fScale;
		m_physic->SetParams(&params);
	}
	//CalcWholeBBox(); [PETAR] its now done in update
	m_bRecalcBBox = true;
}


void CEntity::SetPhysAngles(const Vec3d &angl)
{
	if (m_physic &&(m_flags & ETY_FLAG_CALC_PHYSICS) && !(m_flags&ETY_FLAG_IGNORE_PHYSICS_UPDATE))
	{
		pe_params_pos pp;

		//pp.q = quaternionf((vectorf)angl*(gf_PI/180.0f));
		pp.q.SetRotationXYZ( (vectorf)angl*(gf_PI/180.0f) );

		m_physic->SetParams(&pp);
	}
}

void CEntity::SetAngles(const Vec3d &sAngle, bool bNotifyContainer, bool bUpdatePhysics,bool forceInWorld)
{
	/*
	// we filter out NaNs here
	Vec3d sAngle = ValidateAngles(pos);
	*/

	//sAngle.Snap360();

	if (forceInWorld)
	{
		m_angles = sAngle;
		return;
	}
	else
		if (m_bIsBound||m_bForceBindCalculation)
		{
			m_realangles = sAngle;
			CalculateInWorld();
		}
		else
		{
			Vec3d diff;

			diff = sAngle - m_angles;
			if (!(fabs(diff.x) < ANGLES_EPSILON && fabs(diff.y) < ANGLES_EPSILON && fabs(diff.z) < ANGLES_EPSILON))
			{
				m_angles += diff;

				if (bUpdatePhysics && m_physic &&(m_flags & ETY_FLAG_CALC_PHYSICS))
				{
					SetPhysAngles( m_angles );
					//				pe_params_pos pp;
					//				pp.q = quaternionf(m_angles.z*(PI/180.0f),m_angles.y*(PI/180.0f),m_angles.x*(PI/180.0f));
					//				m_physic->SetParams(&pp);
				}
			}
		}

		// just in case :)
		//CalcWholeBBox(); {petar} its done in update now
		m_bRecalcBBox = true;

		if (m_pContainer && bNotifyContainer)
			m_pContainer->OnSetAngles(sAngle);	// m_angles);
}

//////////////////////////////////////////////////////////////////////
const Vec3d & CEntity::GetPos(bool bWorldOnly /* = false */) const
{
	if (!bWorldOnly && (m_bIsBound||m_bForceBindCalculation))
		return m_realcenter;
	else
		return (m_center); 
}

const Vec3d & CEntity::GetAngles(int realA) const
{	

	Vec3d	angle;
	if( realA && (m_bIsBound||m_bForceBindCalculation))
		return m_realangles;
	else
		return m_angles;

	/*if( angle->z < 0 )
	{
	angle->z = 360.f + ((int)angle->z%360);
	}
	else
	angle->z = (float) ((int)angle->z%360);

	return *angle;*/
}



// init the entity
//////////////////////////////////////////////////////////////////////
bool CEntity::Init(CEntityDesc &ed)
{
	m_nID = ed.id;
	m_netPresence = ed.netPresence;
	m_name = ed.name.c_str();
	m_fScale = ed.scale;
	//SetPos(ed.pos);
	m_center = ed.pos;
	m_bIsADeadBody = 0; // need this before calling OnInit - it might eb changed there

	m_angles = ed.angles;

	_SmartScriptObject pObj(m_pScriptSystem, true);

	// do not touch the order of the next 3 lines
	if (m_pContainer)
		m_pContainer->SetEntity(this);

	//////////////////////////////////////////////////////////////////////////
	// Check if needs to track collider.
	//[Timur] Only when explicitly set.
	//m_bTrackColliders = IsStateFunctionImplemented(ScriptState_OnEnterArea) || IsStateFunctionImplemented(ScriptState_OnLeaveArea);

	//////////////////////////////////////////////////////////////////////////
	// Check if need script update function.
	m_bUpdateScript = IsStateFunctionImplemented(ScriptState_OnUpdate);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Check if need ResolveCollisions for OnContact script function.
	m_bUpdateOnContact = IsStateFunctionImplemented(ScriptState_OnContact);
	//////////////////////////////////////////////////////////////////////////

	// Client/Server state init.
	if (m_pServerState)
	{
		// Call Server.OnInit
		_SmartScriptObject pServer(m_pScriptSystem, true);
		if (m_pScriptObject->GetValue(SCRIPT_SERVER_STATE,pServer))
		{
			HSCRIPTFUNCTION pOnInitFunc = 0;
			if (pServer->GetValue( SCRIPT_INIT,pOnInitFunc ))
			{
				m_pScriptSystem->BeginCall(pOnInitFunc);
				m_pScriptSystem->PushFuncParam(m_pScriptObject);
				m_pScriptSystem->EndCall();
				m_pScriptSystem->ReleaseFunc(pOnInitFunc);
			}
		}
		else 
		{
			// Default Fallback.
			m_pScriptSystem->BeginCall(m_sClassName.c_str(), SCRIPT_INIT);
			m_pScriptSystem->PushFuncParam(m_pScriptObject);
			m_pScriptSystem->EndCall();
		}
	}
	if (m_pClientState)
	{
		// Call Client.OnInit
		_SmartScriptObject pClient(m_pScriptSystem, true);
		if (m_pScriptObject->GetValue(SCRIPT_CLIENT_STATE,pClient))
		{
			HSCRIPTFUNCTION pOnInitFunc = 0;
			if (pClient->GetValue( SCRIPT_INIT,pOnInitFunc ))
			{
				m_pScriptSystem->BeginCall(pOnInitFunc);
				m_pScriptSystem->PushFuncParam(m_pScriptObject);
				m_pScriptSystem->EndCall();
			}
			m_pScriptSystem->ReleaseFunc(pOnInitFunc);
		}
	}

	//////////////////////////////////////////////////////////////////////////	
	if (m_pScriptObject)
		m_pScriptObject->SetValue("type",m_sClassName.c_str());

	//////////////////////////////////////////////////////////////////////////	
	if (m_pContainer)
		m_pContainer->Init();

	//SetCommonCallbacks(m_pScriptSystem);

	//if (strcmp("Pig1", m_name.c_str())==0) DEBUG_BREAK;

	m_bTrackable=false;
	if (m_pScriptObject)
	{
		bool bTrackable = false;
		// store certain properties so there is no need to get them every frame from script
		_SmartScriptObject pPropTable(m_pScriptSystem, true);
		if (m_pScriptObject->GetValue("Properties", pPropTable))
			pPropTable->GetValue("bTrackable", bTrackable);
		m_bTrackable = bTrackable;
	}

	//Timur[1/31/2002] 
	/*
	if (!m_pContactFunc)
	m_pISystem->GetILog()->LogToFile("[ENTITYWARNING] %s has no contact function, and no contact on it will be called",m_sClassName.c_str());
	*/

	CalcWholeBBox();	// here its ok because its on init

	m_bInitialized=true;

	// If anything during init, changed it.
	if (!m_bTrackColliders)
		m_awakeCounter = 0;
	m_bWasAwake = 1;
	m_fCharZOffsetCur = m_fCharZOffsetTarget = 0;
	m_nFlyingFrames = 100;

	return true;
}


//////////////////////////////////////////////////////////////////////////
void CEntity::SetScriptObject(IScriptObject *pObject)
{
	m_pScriptObject = pObject;

	if(m_pServerState){
		ReleaseStateTable(*m_pServerState);
		delete m_pServerState;
		m_pServerState = 0;
	}

	if(m_pClientState){
		ReleaseStateTable(*m_pClientState);
		delete m_pClientState;
		m_pClientState = 0;
	}

	if (!m_pScriptObject)
		return;

	// Cache script callbacks.
	HSCRIPTFUNCTION temp=0;
	m_pScriptObject->GetValue( "OnSave",temp );
	m_pSaveFunc.Init(m_pScriptSystem,temp);
	temp=0;
	m_pScriptObject->GetValue( "OnLoad",temp );
	m_pLoadFunc.Init(m_pScriptSystem,temp);
	temp=0;
	m_pScriptObject->GetValue( "OnLoadRELEASE",temp );
	m_pLoadRELEASEFunc.Init(m_pScriptSystem,temp);
	m_pScriptObject->GetValue( "OnLoadPATCH1",temp );
	m_pLoadPATCH1Func.Init(m_pScriptSystem,temp);
	bool bOnClient = m_pEntitySystem->ClientEnabled();
	bool bOnServer = m_pEntitySystem->ServerEnabled();

	_SmartScriptObject pServerTable(m_pScriptSystem,true);
	_SmartScriptObject pClientTable(m_pScriptSystem,true);

	// Get Server table if exist.
	bool bServerTable = m_pScriptObject->GetValue( SCRIPT_SERVER_STATE,pServerTable );
	// Get Client table if exist.
	bool bClientTable = m_pScriptObject->GetValue( SCRIPT_CLIENT_STATE,pClientTable );

	// Analyze script object for Client/Server states.
	if (bOnClient && bClientTable)
	{
		// Client state exist only on client and only if have Client table.
		m_pClientState = new SScriptState;
		InitializeStateTable( pClientTable,*m_pClientState );
	}

	// If Neither Client neither Server states exist, fallback to single server state.
	// This provided for backward compatability support and for classes that dont care about Client/Server.

	if (bOnServer || !m_pClientState)
	{
		// Server state always exist on server (have it Server table or not).
		m_pServerState = new SScriptState;
		if (bServerTable)
			InitializeStateTable( pServerTable,*m_pServerState );
		else
			InitializeStateTable( m_pScriptObject,*m_pServerState );
	}
}

/* it sets common callback functions for those entities
without a script, by calling a common script function
for materials / sounds etc. 
Client side only.
@param pScriptSystem pointer to script system
*/
//////////////////////////////////////////////////////////////////////////
void CEntity::SetCommonCallbacks(IScriptSystem *pScriptSystem)
{
	m_pOnCollide.Init(pScriptSystem,pScriptSystem->GetFunctionPtr("CommonCallbacks", "OnCollide"));
	m_pOnStopRollSlideContact.Init(pScriptSystem,pScriptSystem->GetFunctionPtr("CommonCallbacks", "OnStopRollSlideContact"));
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetName(const char*name)
{
	m_name = name;
}

//////////////////////////////////////////////////////////////////////////
Vec3d CEntity::GetSoundPos()
{
	Vec3d vPos;
	IEntityContainer *pICnt=GetContainer();
	if (pICnt)
		vPos=pICnt->CalcSoundPos();
	else
		vPos=GetPos();
	return vPos;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::PlaySound(ISound *pSound, float fSoundScale, Vec3d &Offset)
{	
	m_bUpdateSounds = true;
	if (m_lstAttachedSounds.size()<15)
	{
		bool bWasPlaying = pSound->IsPlaying();
		pSound->SetPosition(GetSoundPos());
		pSound->Play(fSoundScale);
		// If this sound already playing don`t add it again for playback.
		if (!bWasPlaying && pSound->IsPlaying())
		{
			m_lstAttachedSounds.push_back(SAttachedSound(pSound, Offset));
		}
	}
	else
	{
		m_pISystem->GetILog()->Log("\001 warning trying to attach more than 15 sounds to %s",m_name.c_str());	
		int ddd=1;
		for(SoundsList::iterator sItr = m_lstAttachedSounds.begin();sItr!=m_lstAttachedSounds.end(); ++sItr,++ddd)
		{
			ISound *pDbg = (*sItr).pSound;

			m_pISystem->GetILog()->Log("\005   %d. %s",ddd,pDbg->GetName());
		}
		// let's clear it, to restore from an erroneous situation
		m_lstAttachedSounds.clear();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::MoveTo(const Vec3d &pos, bool moveObjects, bool bUpdatePhysics)
{

	// move physics
	if (bUpdatePhysics && m_physic &&(m_flags & ETY_FLAG_CALC_PHYSICS))
	{
		pe_params_pos temp;
		temp.pos = vectorf(pos.x, pos.y, pos.z);
		m_physic->SetParams(&temp);
	}

	if (fabs(pos.x-m_center.x) < POS_EPSILON && fabs(pos.y-m_center.y) < POS_EPSILON && fabs(pos.z-m_center.z) < POS_EPSILON)
		return;

	// assign the new position
	m_center = pos;
	m_awakeCounter = 1; // Awake object for one frame at least if sleeping.

	if (moveObjects)
	{
		// recalc the bbox again
		//CalcWholeBBox(); [Petar] moved in Update
		m_bRecalcBBox = true;
	}

	// Must be already initialized to call OnMove callback.
	if (m_bInitialized)
	{
		CallStateFunction( ScriptState_OnMove );
	}
}


//////////////////////////////////////////////////////////////////////////
void CEntity::Reset()
{
	// Not garbage anymore.
	m_bGarbage = false;
	// Go to sleep
	m_bSleeping = true;
	m_awakeCounter = 0;

	m_fRollTimeout=0.0f;
	m_fSlideTimeout=0.0f;
	m_fLastSubMergeFracion=0.0f;

	// Call script OnReset.
	if (m_pScriptObject)
	{
		HSCRIPTFUNCTION pOnResetFunc = 0;
		if (m_pScriptObject->GetValue( SCRIPT_ONRESET,pOnResetFunc ))
		{
			m_pScriptSystem->BeginCall(pOnResetFunc);
			m_pScriptSystem->PushFuncParam(m_pScriptObject);
			m_pScriptSystem->EndCall();
			m_pScriptSystem->ReleaseFunc(pOnResetFunc);
		}
	}

	if (m_pPhysState)
	{
		delete[] m_pPhysState;
		m_pPhysState = 0;
	}

	//[Timur] Its enough to make self:AwakePhysics(0) at end of OnReset in script.
	//
	/*
	//put physics to sleep
	IPhysicalEntity *phys = GetPhysics();
	if(phys)
	{
	pe_action_awake	aa;
	aa.bAwake=0;
	phys->Action(&aa);
	}
	*/

	m_PrevVertVel = 0.0f;
	m_vPrevVel.Set(0,0,0);

	// This is for testing only (at least now): to find out if anybody starts animations after the character dies
	for (int i =0; i < MAX_ANIMATED_MODELS; ++i) 
		if (m_pCryCharInstance[i])
		{
			m_pCryCharInstance[i]->EnableStartAnimation(true);
			m_pCryCharInstance[i]->StopAllMorphs();
		}

	// Remove all colliders.
	if (m_pColliders)
	{
		delete m_pColliders;
		m_pColliders = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetDestroyable( bool b )
{

	if (b)
		m_flags |= ETY_FLAG_DESTROYABLE;
	else
		m_flags &= ~ETY_FLAG_DESTROYABLE;
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::IsDestroyable() const
{
	return (m_flags & ETY_FLAG_DESTROYABLE) == ETY_FLAG_DESTROYABLE;
}

// update the entity-this is called every frame and
// updtae a bunch of various stuff
//////////////////////////////////////////////////////////////////////
void CEntity::Update( SEntityUpdateContext &ctx )
{
	ENTITY_PROFILER

	// Decrease awake counter.
	if (m_awakeCounter > 0)
		m_awakeCounter--;

	/*
	//[Timur] This is not optimal to do for every entity every frame.
	// [Sergiy] this is a safety check to avoid invalid numbers that come from outside
	// and result in "invisible character" bug (because the entity can't calculate the bbox correctly
	// and can't be registered in a sector
	m_angles = ValidateAngles(m_angles);
	*/

	if (m_bIsBound || m_bForceBindCalculation)
	{
		// Must be before potential visibilty check, otherwise child entity position will be wrong.
		// this entity is bound or is forced to be calculated as bound
		CalculateInWorld();

		//[Timur] This is needed for binded Dynamic lights, or they are not passing CheckUpdateVisLevel check.
		if (m_bRecalcBBox)
		{
			// Reregister entity in sectors.
			UnregisterInSector();
			RegisterInSector();
		}
	}

	//  [PETAR] commented out this temp var - not really needed anymore
	//	bool bEntityVisible = (ctx.nFrameID == m_nLastVisibleFrameID);
	//	m_bWasVisible = bEntityVisible;
	m_bWasVisible = m_bVisible;
	m_bVisible = (m_eUpdateVisLevel==eUT_Unconditional) ? true : ((ctx.nFrameID - m_nLastVisibleFrameID)<MAX_FRAME_ID_STEP_PER_FRAME);

	if (m_bVisible != m_bWasVisible)
		OnVisibilityChange(m_bVisible);

	// check if entity logic passes selected visibility test
	if (m_eUpdateVisLevel && m_eUpdateVisLevel!=eUT_PhysicsPostStep && m_pEntitySystem->m_pVisCheckForUpdate->GetIVal())
	{
		if (!CheckUpdateVisLevel( ctx,m_eUpdateVisLevel ))
		{
			return;
		}
	}

	// Calculate how many entities where actually updated.
	ctx.numUpdatedEntities++;

	if (m_bUpdateSounds)
		UpdateSounds( ctx );

	// should be called before any rendering
	if (m_bVisible)
	{
		ctx.numVisibleEntities++;

		if (m_pLipSync)
			UpdateLipSync( ctx );

		if (m_bEntityHasLights && m_bEntityLightsOn)
		{
			ProcessEntityLightSources();
		}
	}

	if (m_bUpdateEmitters)
	{
		UpdateParticleEmitters( ctx );
	}

	//////////////////////////////////////////////////////////////////////////
	if (m_bUpdateOnContact)
	{
		//[Timur] @TODO remove this when will be possible.
		ResolveCollision();
	}
	//////////////////////////////////////////////////////////////////////////


	if (m_physic)
	{
		UpdatePhysics(ctx);
	};

	/*
	//////////////////////////////////////////////////////////////////////////
	// Update timer.
	//////////////////////////////////////////////////////////////////////////
	if(m_nTimer>0)
	{
		m_awakeCounter = 2; // As long as there is timer, must stay awake.
		if ((ctx.fCurrTime*1000) - m_nStartTimer >= m_nTimer)
		{
			int t = m_nTimer;
			KillTimer();
			OnTimer(t);
		}
	}
	*/

	// Update`s script function if present.
	if (m_bUpdateScript)
	{
		if (m_pEntitySystem->m_pUpdateScript->GetIVal())
		{
			m_fScriptUpdateTimer -= ctx.fFrameTime;
			if (m_fScriptUpdateTimer <= 0)
			{
				m_fScriptUpdateTimer = m_fScriptUpdateRate;
				ENTITY_PROFILER_NAME( "CEntity:Update:Script" )

					//////////////////////////////////////////////////////////////////////////
					// Script Update.
					if (ctx.pScriptUpdateParams)
						CallStateFunction( ScriptState_OnUpdate,ctx.fFrameTime,ctx.pScriptUpdateParams );
					else
						CallStateFunction( ScriptState_OnUpdate,ctx.fFrameTime );
				//
				//////////////////////////////////////////////////////////////////////////
			}
		}
	}

	if (m_bUpdateAI)
		UpdateAIObject(m_bVisible);

	// update container
	if (m_bUpdateContainer)
	{
		if (m_pContainer && m_pEntitySystem->m_pUpdateContainer->GetIVal())
		{
			ENTITY_PROFILER_NAME( "CEntity:Update:Container" )
				m_pContainer->Update();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Update Characters.
	if (m_bUpdateCharacters || m_physPlaceholder)
	{
		UpdateCharacters( ctx );
	}

	// update camera
	if (m_bUpdateCamera)
	{
		if (m_pCamera && m_pEntitySystem->m_pUpdateCamera->GetIVal())
		{
			m_pCamera->Update();
		}
	}

	if (m_bRecalcBBox)
	{
		CalcWholeBBox();
		m_bRecalcBBox = false;
	}

	if (m_bUpdateCharacters || m_physPlaceholder)
	{
		UpdatePhysPlaceholders( ctx );
	}

	if (m_bTrackColliders)
		CheckColliders();
 
	return;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UpdateCharacters( SEntityUpdateContext &ctx )
{
	if(!m_pEntitySystem->m_pUpdateBonePositions->GetIVal())
		return;

	bool bProcess=m_bVisible;

	IGame *pGame = GetISystem()->GetIGame();

	// ensure update to  collision detection in multiplayer
	// (only done on the server so the client decal might be produced
	// wrong if you don't look at the target but this way we can save computation time)
	if(pGame && pGame->GetModuleState(EGameMultiplayer) && pGame->GetModuleState(EGameServer))
		bProcess = true;

	if(m_eUpdateVisLevel == eUT_Physics)
		bProcess = true;

	if(bProcess)
	{
		ENTITY_PROFILER

		for(int k = 0; k < m_nMaxCharNum; k++) 
		{
			if(m_pCryCharInstance[k] && (m_pCryCharInstance[k]->GetFlags() & CS_FLAG_UPDATE))
			{
				m_pCryCharInstance[k]->Update(m_center,m_fRadius);  

				// recalc bbox if animated
				if(m_pCryCharInstance[k]->IsCharacterActive())
					//CalcWholeBBox(); look further down for actual call
					m_bRecalcBBox = true;
			}
		}
	}
	else
	{
		for(int k = 0; k < m_nMaxCharNum; k++)
		{
			if(m_pCryCharInstance[k] && (m_pCryCharInstance[k]->GetFlags() & CS_FLAG_UPDATE))
				m_pCryCharInstance[k]->Update( m_center,m_fRadius, ICryCharInstance::flagDontUpdateBones);
		}
	}

	if(bProcess)
	{
		UpdateCharacterPhysicsAndIK( ctx );
	}
	else if(m_physic)
	{
		pe_params_sensors ps;
		ps.nSensors = 0;
		m_physic->SetParams(&ps);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UpdateLipSync( SEntityUpdateContext &ctx )
{
	if (!m_pLipSync)
		return;

	ENTITY_PROFILER

		// if we're too far away we dont do facial-animation
		bool bAnimate=false;
	IRenderer *pRenderer=m_pISystem->GetIRenderer();
	if (pRenderer)
	{
		//[Timur] This code have no place here, it is hack and must be removed (to inside lip sync for example)

		// we project the top and bottom to screen-space and check if the character is visible enough to update the face...
		Vec3d Min, Max;
		GetBBox(Min, Max);
		Vec3d Center=(Max-Min)*0.5f+Min;
		Vec3d TopCenter(Center.x, Center.y, Max.z);
		Vec3d BotCenter(Center.x, Center.y, Min.z);
		Vec3d TopCenterProj, BotCenterProj;
		pRenderer->ProjectToScreen(TopCenter.x, TopCenter.y, TopCenter.z, &TopCenterProj.x, &TopCenterProj.y, &TopCenterProj.z);
		pRenderer->ProjectToScreen(BotCenter.x, BotCenter.y, BotCenter.z, &BotCenterProj.x, &BotCenterProj.y, &BotCenterProj.z);
		if (fabs(TopCenterProj.y-BotCenterProj.y)>50.0f)	// lets animate the face if the character  is higher than 50 pixels
			bAnimate=true;
	}
	//		if (!bAnimate)
	//			TRACE("Skipping lipsync for entity %s", m_name.c_str());
	m_pLipSync->Update(bAnimate);
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnCollide(float fDeltaTime)
{
	//m_pISystem->GetILog()->LogToConsole("diff=%0.2f",m_pISystem->GetITimer()->GetCurrTime()-m_fLastCollideTime);
	int bAwake = m_physic ? m_physic->GetStatus(&pe_status_awake()) : 0;
	float fFreq = m_physic && (m_physic->GetType()==PE_RIGID || m_physic->GetType()==PE_WHEELEDVEHICLE) && (bAwake+m_bWasAwake) ? 0.01f : 0.3f;
	float fFrameTime = m_pISystem->GetITimer()->GetCurrTime()-m_fLastCollideTime;
	if (!m_physic || fFrameTime<=fFreq && bAwake==m_bWasAwake)
		// avoid to create a rolling sound for impact, and avoid to call the
		// script function every frame
		return; 

	m_fLastCollideTime = m_pISystem->GetITimer()->GetCurrTime();

	coll_history_item contacts[32];	
	int nColls;
	float velImpact=0,velSlide2=0,velRoll2=0,velImpactCur,velSlide2Cur,velRoll2Cur;
	bool bCallOnCollide = false;
	IEntity	*pColliderEntity=NULL;

	pe_status_collisions sc;	
	pe_status_dynamics sd;
	//some lazy values to get only the latest collider from physics
	sc.age = 0.3f; 
	sc.bClearHistory = 1; 
	sc.pHistory = contacts;	
	sc.len = sizeof(contacts)/sizeof(contacts[0]);
	m_physic->GetStatus(&sd);

	float	velVertDelta = (sd.v - m_vPrevVel).len();
	m_vPrevVel = sd.v;
	//	m_PrevVertVel = sd.v.z;

	//	float	velVertDelta = sd.v.z - m_PrevVertVel;
	//	m_PrevVertVel = sd.v.z;

	Vec3	vVel(0,0,0);
	float	fVelSz=0;
	if ((bAwake+m_bWasAwake) && (nColls = m_physic->GetStatus(&sc)))
	{
	IPhysicalWorld *pWorld = m_pISystem->GetIPhysicalWorld();
		for(nColls--;nColls>=0;nColls--)
		{
			// finding max contact velocity
			float fTmp=contacts[nColls].v[0].len2();
			if(fTmp>fVelSz)
			{
				fVelSz = fTmp;
				vVel = contacts[nColls].v[0];
			}

			Vec3d vrel = contacts[nColls].v[1]-contacts[nColls].v[0], r = contacts[nColls].pt-sd.centerOfMass;
			if (sd.w.len2()>0.01f)
				r -= sd.w*((r*sd.w)/sd.w.len2());
			velImpactCur = fabs(vrel*contacts[nColls].n);
			velSlide2Cur = (vrel-contacts[nColls].n*velImpactCur).len2();
			velRoll2Cur = (sd.w^r).len2();

			if(velImpact<velImpactCur)
			{
				IPhysicalEntity *pCollider = pWorld->GetPhysicalEntityById(contacts[nColls].idCollider);
				if(pCollider)
					pColliderEntity = (IEntity *)pCollider->GetForeignData();
				velImpact = velImpactCur;
			}
//			velImpact = max(velImpact,velImpactCur);
			velSlide2 = max(velSlide2,velSlide2Cur);
			//m_pISystem->GetILog()->LogToConsole("test=%0.4f", (r*contacts[nColls].n)/r.len());
			if (sqr(r*contacts[nColls].n)>r.len2()*sqr(0.97f))
				velRoll2 = max(velRoll2,velRoll2Cur);
		}
		if (velRoll2<0.1f)
		{
			if ((m_fTimeNotRolling+=fFrameTime)>0.15f)
				m_fTimeRolling = 0;
		}
		else
		{
			m_fTimeRolling += fFrameTime;
			m_fTimeNotRolling = 0;
		}
		if (m_fTimeRolling<0.2f)
			velRoll2 = 0;

		//if (velImpact>2.0f)
		//	m_pISystem->GetILog()->LogToConsole("impact %.2f",velImpact);
		//if (velSlide2>1.0f)
		//	m_pISystem->GetILog()->LogToConsole("slide %.2f",cry_sqrtf(velSlide2));
		//m_pISystem->GetILog()->LogToConsole(velRoll2>0 ? "rolling":m_fTimeRolling>0 ? "potentially rolling":"not rolling");
		//		if (contacts[0].pCollider==NULL)
		//			return;

		//*
		float fSpeed=(contacts[0].v[0]-contacts[0].v[1]).len();
		//m_pISystem->GetILog()->LogToConsole("Speed=%0.2f",fSpeed);

		//if (fSpeed<1.5f)
		//	return; //do not bother if velocity is too small		
		//*/
		bCallOnCollide = true;
		m_pObjectCollide->BeginSetGetChain();
		m_pObjectCollide->SetValueChain("fSpeed",fSpeed);		
		m_pObjectCollide->SetValueChain("matId",contacts[0].idmat[1]);
		//pObject->SetValueChain("matId",tCollider.idmat[0]);
	
		// collision position
		m_vObjPosCollide->BeginSetGetChain();
		m_vObjPosCollide->SetValueChain("x",contacts[0].pt.x);
		m_vObjPosCollide->SetValueChain("y",contacts[0].pt.y);
		m_vObjPosCollide->SetValueChain("z",contacts[0].pt.z);
		m_vObjPosCollide->EndSetGetChain();

		m_pObjectCollide->SetValueChain("vPos",m_vObjPosCollide);

		// collision velocity
		m_vObjVelCollide->BeginSetGetChain();
		m_vObjVelCollide->SetValueChain("x",vVel.x);
		m_vObjVelCollide->SetValueChain("y",vVel.y);
		m_vObjVelCollide->SetValueChain("z",vVel.z);
		m_vObjVelCollide->EndSetGetChain();

		m_pObjectCollide->SetValueChain("vVel",m_vObjVelCollide);


		m_pObjectCollide->SetValueChain("impactVert",velVertDelta); // vertical impact contact

		m_pObjectCollide->SetToNullChain("impact");
		m_pObjectCollide->SetToNullChain("roll");
		m_pObjectCollide->SetToNullChain("slide");

		float velImpactThresh = 1.5f;
		if (velRoll2>0.1f)
		{
			m_fRollTimeout=0.5f;	// adjust if needed
			m_pObjectCollide->SetValueChain("roll",cry_sqrtf(velRoll2)); // roll contact
			velImpactThresh = 3.5f;
		} else if (velSlide2>0.1f)
		{
			m_fSlideTimeout=0.5f;	// adjust if needed
			m_pObjectCollide->SetValueChain("slide",cry_sqrtf(velSlide2)); // slide contact
		}
		if (velImpact>velImpactThresh)
			m_pObjectCollide->SetValueChain("impact",velImpact); // impact contact
		if(pColliderEntity)
			m_pObjectCollide->SetValueChain("collider",pColliderEntity->GetScriptObject());
		else
			m_pObjectCollide->SetToNullChain("collider");

		/*
		vNormDir->BeginSetGetChain();
		vNormDir->SetValueChain("x",contacts[0].n.x);
		vNormDir->SetValueChain("y",contacts[0].n.y);
		vNormDir->SetValueChain("z",contacts[0].n.z);
		vNormDir->EndSetGetChain();
		pObject->SetValueChain("vNorm",vNormDir);
		*/
	}
	m_bWasAwake = bAwake;

	int nCircles = 0;
	//_SmartScriptObject pSplashList(m_pScriptSystem);
	IScriptObject *psoSplashes[32],*psoCenters[32];

	if (m_fLastSplashTime>m_pEntitySystem->m_pSplashTimeout->GetFVal() && 
		sd.waterResistance>m_pEntitySystem->m_pSplashThreshold->GetFVal()) // now test if the object hit the water hard enough
	{
		pe_status_pos sp; sp.flags = 0;
		geom_world_data gwd;
		geom_contact *pContacts;
		IPhysicalWorld *pWorld = m_pISystem->GetIPhysicalWorld();
		int iCont,iCircle;
		Vec2 *pCenters;
		float *pRadii;
		Vec3 v;
		primitives::box boxWater;
		pe_params_bbox pbb;
		m_physic->GetParams(&pbb);

		boxWater.Basis.SetIdentity();
		boxWater.center = (pbb.BBox[0]+pbb.BBox[1])*0.5f;
		boxWater.size = pbb.BBox[1]-pbb.BBox[0];
		boxWater.center.z += m_pISystem->GetI3DEngine()->GetWaterLevel(this,&v) - (boxWater.center.z-boxWater.size.z);
		IGeometry *pWaterSurface = pWorld->GetGeomManager()->CreatePrimitive(primitives::box::type, &boxWater);
		m_pSplashList->Clear();

		for(sp.ipart=m_physic->GetStatus(&pe_status_nparts())-1; sp.ipart>=0; sp.ipart--)
		{
			m_physic->GetStatus(&sp);
			gwd.offset = sp.pos;
			gwd.R = matrix3x3f(sp.q);
			gwd.scale = sp.scale;
			for(iCont=sp.pGeomProxy->Intersect(pWaterSurface, &gwd,0,0, pContacts)-1; iCont>=0; iCont--)
				for(iCircle = pWorld->GetPhysUtils()->CoverPolygonWithCircles(strided_pointer<Vec2>((Vec2*)pContacts[iCont].ptborder,sizeof(Vec3)),
					pContacts[iCont].nborderpt,pContacts[iCont].bBorderConsecutive, (const Vec2&)pContacts[iCont].center, pCenters,pRadii, 0.5f)-1; 
					iCircle>=0; iCircle--)
				{
					psoCenters[nCircles] = m_pScriptSystem->CreateObject();
					psoCenters[nCircles]->BeginSetGetChain();
					psoCenters[nCircles]->SetValueChain("x",pCenters[iCircle].x);
					psoCenters[nCircles]->SetValueChain("y",pCenters[iCircle].y);
					psoCenters[nCircles]->SetValueChain("z",boxWater.center.z-boxWater.size.z);
					psoCenters[nCircles]->EndSetGetChain();

					psoSplashes[nCircles] = m_pScriptSystem->CreateObject();
					psoSplashes[nCircles]->BeginSetGetChain();
					psoSplashes[nCircles]->SetValueChain("center",psoCenters[nCircles]);
					psoSplashes[nCircles]->SetValueChain("radius",pRadii[iCircle]);
					psoSplashes[nCircles]->SetValueChain("intensity",sd.waterResistance);
					psoSplashes[nCircles]->EndSetGetChain();

					m_pSplashList->SetAt(nCircles+1, psoSplashes[nCircles]);
					if (++nCircles==6)
						goto CirclesNoMore;
				}
		}

CirclesNoMore:
		pWorld->GetGeomManager()->DestroyGeometry(pWaterSurface);
	}

	if (nCircles)
	{
		if (!bCallOnCollide)
		{
			m_pObjectCollide->BeginSetGetChain();
			m_pObjectCollide->SetToNullChain("impact");
			m_pObjectCollide->SetToNullChain("roll");
			m_pObjectCollide->SetToNullChain("slide");
		}
		bCallOnCollide = true;
		m_pObjectCollide->SetValueChain("num_splashes", nCircles);
		m_pObjectCollide->SetValueChain("splashes", m_pSplashList);
		m_fLastSplashTime = 0;
	}
	else if (bCallOnCollide)
	{
		m_pObjectCollide->SetToNullChain("num_splashes");
		m_pObjectCollide->SetToNullChain("splashes");
	}
	m_fLastSplashTime += fFrameTime;

	bool bSplash;
	if ((sd.waterResistance>10.0f) && (sd.submergedFraction>0.0f) && (m_fLastSubMergeFracion<=0.0f))
	{
		if (!bCallOnCollide)
			m_pObjectCollide->BeginSetGetChain();
		bCallOnCollide=true;
		m_fLastSubMergeFracion=sd.submergedFraction;
		bSplash=true;
	}else
	{
		if (bCallOnCollide)
			m_pObjectCollide->SetToNullChain("waterresistance");
		if (sd.submergedFraction==0.0f)
			m_fLastSubMergeFracion=0.0f;
		bSplash=false;
	}

	if (bCallOnCollide)
	{
		if (bSplash)
			m_pObjectCollide->SetValueChain("waterresistance", sd.waterResistance);
		m_pObjectCollide->EndSetGetChain();

		if (m_pOnCollide)
		{
			m_pScriptSystem->BeginCall(m_pOnCollide);			
			m_pScriptSystem->PushFuncParam(false);
			m_pScriptSystem->PushFuncParam(m_pObjectCollide);
			m_pScriptSystem->EndCall();
		}
		else
		{
			IScriptObject *pScriptObject = m_pObjectCollide;
			CallStateFunction( ScriptState_OnCollide,pScriptObject );
		}
	}

	for(nCircles--;nCircles>=0;nCircles--)
		psoSplashes[nCircles]->Release(),psoCenters[nCircles]->Release();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UpdateSounds( SEntityUpdateContext &ctx )
{
	if (!m_lstAttachedSounds.empty())
	{
		ENTITY_PROFILER

		SoundsListItor itor = m_lstAttachedSounds.begin();
		Vec3d vPos= GetSoundPos();
		while(itor!=m_lstAttachedSounds.end())
		{
			SAttachedSound &Sound=(*itor);			
#if !defined(LINUX64)
			if((Sound.pSound!=NULL) && (Sound.pSound->IsPlaying() || Sound.pSound->IsPlayingVirtual()))
#else
			if((Sound.pSound!=0) && (Sound.pSound->IsPlaying() || Sound.pSound->IsPlayingVirtual()))
#endif
			{
				Sound.pSound->SetPosition(vPos+Sound.Offset);
				++itor;
			}
			else
			{
				itor=m_lstAttachedSounds.erase(itor);
			}
		}
	}
	m_bUpdateSounds = !m_lstAttachedSounds.empty();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UpdatePhysics( SEntityUpdateContext &ctx )
{
	ENTITY_PROFILER

		if (!m_pEntitySystem->m_pUpdatePhysics->GetIVal())
			return;

	float fDeltaTime = ctx.fFrameTime;
	if (m_physic && (m_flags & ETY_FLAG_CALC_PHYSICS) && !(m_flags&ETY_FLAG_IGNORE_PHYSICS_UPDATE))
	{
		// get the new position from the physics system
		pe_status_pos pos;
		quaternionf qmaster, q[32];
		m_physic->GetStatus(&pos);
		qmaster = pos.q;

		Vec3d new_pos = pos.pos, new_angles;


		// get new angles from physics

		//CHANGED_BY_IVO (NOTE: order of angles is flipped!!!!)
		//pos.q.get_Euler_angles_xyz(new_angles.z, new_angles.y, new_angles.x);
		//EULER_IVO
		//Vec3 TempAng; new_angles=pos.q.GetEulerAngles_XYZ(TempAng);
		new_angles=Ang3::GetAnglesXYZ(matrix3x3f(pos.q));


		new_angles *= 180.0f/gf_PI;

		MoveTo(new_pos, new_pos.x!=m_center.x || new_pos.y!=m_center.y || new_pos.z!=m_center.z, false);

		if (m_physic->GetType() != PE_LIVING)
		{
			//SetAngles(new_angles, true, false);
			RotateTo(new_angles, false);
			//			m_angles = new_angles;
		}

		if (m_physic->GetType()==PE_SOFT && GetEntityStatObj(0))
		{
			SetBBox(pos.BBox[0],pos.BBox[1]);

			pe_params_softbody psb;
			m_physic->GetParams(&psb);
			pe_action_awake aa;
			aa.bAwake = m_bVisible;

			if (m_bVisible)
			{
				pe_status_softvtx ssv;
				CLeafBuffer *pLB = GetEntityStatObj(0)->GetLeafBuffer();
				if (pLB && pLB->m_arrVtxMap && m_physic->GetStatus(&ssv)) 
				{
					strided_pointer<Vec3d> pVtx,pNormals,pBinormals,pTangents;
					bool bHasTangents = false;
					pVtx.data = (Vec3*)pLB->GetPosPtr(pVtx.iStride);
					if (g_VertFormatNormalOffsets[pLB->m_nVertexFormat]!=-1)
						pNormals.data = (Vec3*)pLB->GetNormalPtr(pNormals.iStride);
					else
					{
						pNormals.data = (Vec3*)pLB->GetTNormalPtr(pNormals.iStride);
						pBinormals.data = (Vec3*)pLB->GetBinormalPtr(pBinormals.iStride);
						pTangents.data = (Vec3*)pLB->GetTangentPtr(pTangents.iStride);
						bHasTangents = true;
					}
					for(int i=0; i<pLB->m_SecVertCount; i++)
					{
						pVtx[i] = ssv.pVtx[pLB->m_arrVtxMap[i]];
						pNormals[i] = ssv.pNormals[pLB->m_arrVtxMap[i]];
						if (bHasTangents)
						{
							pBinormals[i] = GetOrthogonal(pNormals[i]).normalized();
							pTangents[i] = pNormals[i]^pBinormals[i];
						}
					}
					pLB->InvalidateVideoBuffer();
				}
			}
			if ((m_bVisible^m_bWasVisible) && (!m_bVisible || psb.wind*psb.airResistance>0))
				m_physic->Action(&aa);
		}


		int k = 0;
		pos.flags = status_local;

		bool bOnCollideImplemented = false;
		if (m_pServerState && m_pServerState->pFunction[ScriptState_OnCollide])
			bOnCollideImplemented=true;
		else 
			if (m_pClientState && m_pClientState->pFunction[ScriptState_OnCollide])
				bOnCollideImplemented=true;

		// call the OnCollide function only if the oncollide is implemented OR
		// if there is not script object AND the entity is a rigid body entity, in
		// which case it calls the common colliding behaviour (if necessary)
		if ((bOnCollideImplemented) || (m_pOnCollide))
			OnCollide(fDeltaTime);

		for (unsigned int j = 0; j < m_objects.size(); j++) 
		{
			pos.partid = j; 
			if (m_physic->GetStatus(&pos)) 
			{
				m_objects[j].pos =(Vec3d)pos.pos;
				q[j] = pos.q;

				//CHANGED_BY_IVO (NOTE: order of angles is flipped!!!!)
				//pos.q.get_Euler_angles_xyz(m_objects[j].angles.z, m_objects[j].angles.y, m_objects[j].angles.x);
				//EULER_IVO
				//Vec3 TempAng;	m_objects[j].angles=pos.q.GetEulerAngles_XYZ(TempAng);
				m_objects[j].angles = Ang3::GetAnglesXYZ(matrix3x3f(pos.q));

				m_objects[j].angles *= 180.0f/gf_PI;
			}

			if (m_objects[j].flags & ETY_OBJ_IS_A_LINK) 
			{
				float len0, len;
				matrix3x3in4x4Tf &mtx(*(matrix3x3in4x4Tf*)&m_objects[j].mtx);
				matrix3x3f R0,R1;
				m_objects[j].mtx.SetIdentity();
				Vec3d link_start, link_end, link_offset;

				// calculate current link start, link end in entity coordinates
				link_start = q[m_objects[j].ipart0]*m_objects[j].link_start0 + m_objects[m_objects[j].ipart0].pos;
				link_end = q[m_objects[j].ipart1]*m_objects[j].link_end0 + m_objects[m_objects[j].ipart1].pos;
				len0 =(m_objects[j].link_end0 - m_objects[j].link_start0).Length();
				len =(link_end - link_start).Length();

				mtx = matrix3x3in4x4Tf(qmaster);
				m_objects[j].mtx.SetRow(3,m_center); // initialize object matrix to entity world matrix
				link_offset = m_objects[j].mtx.TransformPointOLD(link_start);

				// build (rotate to previous orientation)*(scale along z axis) matrix
				R1 = matrix3x3f(GetRotationV0V1(vectorf(0,0,1), (link_end-link_start)/len ));
				R1.SetColumn(2,R1.GetColumn(2)*(len/len0));
				// build (rotate so that link axis becomes z axis) matrix
				R0 = matrix3x3f( GetRotationV0V1(vectorf(m_objects[j].link_end0-m_objects[j].link_start0)/len0, vectorf(0, 0, 1)));
				mtx *= (R1*R0);

				// offset matrix by difference between current and required staring points
				link_offset -= m_objects[j].mtx.TransformPointOLD(m_objects[j].link_start0);
				m_objects[j].mtx.SetRow(3,m_objects[j].mtx.GetRow(3)+link_offset);
				m_objects[j].flags |= ETY_OBJ_USE_MATRIX;
			} else 
				m_objects[j].flags &= ~ETY_OBJ_USE_MATRIX;
		}

		// Set water level to any physic entity (Ignore for soft invisible entity).
		if (m_bVisible || m_physic->GetType()!=PE_SOFT)
		{
			// set water level
			pe_params_buoyancy pb;

			//pb.waterDensity = m_fWaterDensity; 
			pb.waterPlane.n.Set(0,0,1);
			Vec3d vWaterFlowSpeed(0,0,0);
			//float fWaterLevel = m_pISystem->GetI3DEngine()->GetWaterLevel(&GetPos(), &vWaterFlowSpeed);
			float fWaterLevel = m_pISystem->GetI3DEngine()->GetWaterLevel(this, &vWaterFlowSpeed);
			pb.waterPlane.origin.Set(0,0,fWaterLevel);
			pb.waterFlow = vWaterFlowSpeed;
			m_physic->SetParams(&pb);
		}
	}

	if (m_fRollTimeout>0)
	{
		m_fRollTimeout -= fDeltaTime;
		if (m_fRollTimeout<=0.0f)
		{
			if (m_pOnStopRollSlideContact)
			{
				m_pScriptSystem->BeginCall(m_pOnStopRollSlideContact);
				m_pScriptSystem->PushFuncParam(false);
				m_pScriptSystem->PushFuncParam("roll");
				m_pScriptSystem->EndCall();
			}else
			{
				CallStateFunction(ScriptState_OnStopRollSlideContact, "roll");
			}
		}
	}

	if (m_fSlideTimeout>0)
	{
		m_fSlideTimeout -= fDeltaTime;
		if (m_fSlideTimeout<=0.0f)
		{
			if (m_pOnStopRollSlideContact)
			{
				m_pScriptSystem->BeginCall(m_pOnStopRollSlideContact);
				m_pScriptSystem->PushFuncParam(false);
				m_pScriptSystem->PushFuncParam("slide");
				m_pScriptSystem->EndCall();
			}else
			{
				CallStateFunction(ScriptState_OnStopRollSlideContact, "slide");
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UpdateCharacterPhysicsAndIK( SEntityUpdateContext &ctx )
{
	ENTITY_PROFILER

		//	for (int k = 0; k < m_nMaxCharNum; k++) if (m_pCryCharInstance[k] )
		for (int k = 0; k < m_nMaxCharNum; k++) if (m_pCryCharInstance[k] && (m_pCryCharInstance[k]->GetFlags() & CS_FLAG_UPDATE))
		{
			m_pCryCharInstance[k]->SetOffset(vectorf(0,0,0));
			if (m_physic && m_pEntitySystem->m_pUpdatePhysics->GetIVal())
			{
				pe_status_living livstat;
				m_physic->GetStatus(&livstat);
				pe_status_sensors sensors;


				if(m_bHandIK)
				{
					m_pCryCharInstance[k]->SetLimbIKGoal(LIMB_LEFT_ARM, m_vHandIKTarget, ik_arm, 0, Vec3d(1,0,0) );
					m_pCryCharInstance[k]->SetLimbIKGoal(LIMB_RIGHT_ARM, m_vHandIKTarget, ik_arm, 0, Vec3d(1,0,0) );
					m_bHandIK = false;		// has to be reset before next update
				}
				else
				{
					m_pCryCharInstance[k]->SetLimbIKGoal(LIMB_LEFT_ARM);
					m_pCryCharInstance[k]->SetLimbIKGoal(LIMB_RIGHT_ARM);
				}

				if (!m_bIsBound && m_pEntitySystem->m_pCharacterIK->GetIVal() && m_physic->GetType()==PE_LIVING && 
					!(livstat.bFlying && livstat.timeFlying>0.2f) &&
					m_physic->GetStatus(&sensors) && sensors.flags) 
				{
					pe_player_dimensions livdim;
					m_physic->GetParams(&livdim);
					float fCharZOffsetBase = 0, fOffsetSpeed = m_pEntitySystem->m_pCharZOffsetSpeed->GetFVal();
					if (livstat.timeSinceStanceChange>3.0f) // don't offset character during stance changes, since it's done by animation blending
						fCharZOffsetBase = livstat.camOffset.z-livdim.heightEye;
					m_fCharZOffsetTarget = 0;
					//m_pCryCharInstance[k]->SetOffset(livstat.camOffset-vectorf(0,0,livdim.heightEye));

					Vec3 feet[2];
					int i, ikflags = ik_leg;
					//if (livstat.vel.len2()>0.01f)
					ikflags |= ik_avoid_stretching;
					for(i=0; i<2; i++)	if (sensors.pPoints[i].z > livdim.heightCollider*0.7f)
						sensors.pPoints[i].z = livdim.heightCollider*0.7f;
					m_nFlyingFrames = m_nFlyingFrames+livstat.bFlying & -livstat.bFlying;
					if (m_nFlyingFrames<4 && livstat.vel.len2()<16 && (livstat.vel.len2()<0.01f || !livstat.bOnStairs))	
						for(i=0; i<2; i++) if (sensors.pPoints[i].z - fCharZOffsetBase/*m_pCryCharInstance[k]->GetOffset().z*/ < -0.02f && sensors.flags&1<<i)
							m_fCharZOffsetTarget = sensors.pPoints[i].z-fCharZOffsetBase;
							//m_pCryCharInstance[k]->SetOffset(m_pCryCharInstance[k]->GetOffset()+
							//vectorf(0, 0, sensors.pPoints[i].z - m_pCryCharInstance[k]->GetOffset().z));// + 0.02f));
					if (fabs_tpl(m_fCharZOffsetCur-m_fCharZOffsetTarget) < fOffsetSpeed*ctx.fFrameTime)
						m_fCharZOffsetCur = m_fCharZOffsetTarget;
					else
						m_fCharZOffsetCur += fOffsetSpeed*sgnnz(m_fCharZOffsetTarget-m_fCharZOffsetCur)*ctx.fFrameTime;
					m_pCryCharInstance[k]->SetOffset(Vec3(0,0,fCharZOffsetBase+m_fCharZOffsetCur));
					for(i=0; i<2; i++)
					{
						feet[i].Set(IK_NOT_USED, IK_NOT_USED, sensors.pPoints[i].z - m_pCryCharInstance[k]->GetOffset().z);
						if (feet[i].z > livdim.heightCollider*0.5f)
							feet[i].z = livdim.heightCollider*0.5f;
						if (sensors.flags&1<<i)
							m_pCryCharInstance[k]->SetLimbIKGoal(LIMB_LEFT_LEG + i, feet[i], ikflags, 0, sensors.pNormals[i]);
						else
							m_pCryCharInstance[k]->SetLimbIKGoal(LIMB_LEFT_LEG + i);
					}
				} else 
				{
					m_pCryCharInstance[k]->SetLimbIKGoal(LIMB_LEFT_LEG);
					m_pCryCharInstance[k]->SetLimbIKGoal(LIMB_RIGHT_LEG);
				}
			}	else 
			{
				m_pCryCharInstance[k]->SetLimbIKGoal(LIMB_LEFT_LEG);
				m_pCryCharInstance[k]->SetLimbIKGoal(LIMB_RIGHT_LEG);
			}

#ifdef DEBUG_BONES_SYNC
			if (m_pEntitySystem->m_bServer)
#endif
			m_pCryCharInstance[k]->UpdatePhysics(m_fScale);
			m_pCryCharInstance[k]->SynchronizeWithPhysicalEntity(m_physic, m_center, GetRotationXYZ<float>(m_angles*(gf_PI/180.0f)));

			pe_params_sensors sensors;
			if (!m_bIsBound && m_physic && m_pEntitySystem->m_pCharacterIK->GetIVal() && m_physic->GetType()==PE_LIVING) 
			{
				Vec3 feet[2], offset = m_pCryCharInstance[k]->GetOffset(); // generally should be transformed into local coordinates
				offset.z = max(offset.z,0.0f); 
				sensors.nSensors = 2;
				sensors.pOrigins = feet;
				feet[0] = m_pCryCharInstance[k]->GetLimbEndPos(LIMB_LEFT_LEG);
				feet[1] = m_pCryCharInstance[k]->GetLimbEndPos(LIMB_RIGHT_LEG) + offset;
				if (!(feet[0].x*feet[0].x>=0) || !(feet[1].x*feet[1].x>=0))	// temporary validity check
					sensors.nSensors = 0; 
				feet[0].z -= 0.4f;
				feet[1].z -= 0.4f;
				m_physic->SetParams(&sensors);
			}	
			else if (m_physic	&& m_physic->GetType()==PE_LIVING) 
			{
				sensors.nSensors = 0;
				m_physic->SetParams(&sensors);
			}

			// recalc bbox if animated
			if (m_pCryCharInstance[k]->IsCharacterActive())
				//CalcWholeBBox(); [Petar] moved to Update
				m_bRecalcBBox = true;
		}	
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UpdatePhysPlaceholders( SEntityUpdateContext &ctx )
{
	ENTITY_PROFILER

		if (m_vBoxMin.x>=m_vBoxMax.x || m_vBoxMin.y>=m_vBoxMax.y || m_vBoxMin.z>=m_vBoxMax.z)
			return;

	pe_params_bbox pbb;
	pe_status_placeholder spc;
	pbb.BBox[0] = m_vBoxMin+m_center;
	pbb.BBox[1] = m_vBoxMax+m_center;

	if (m_bUpdateCharacters)
	{
		for(int i=0;i<m_nMaxCharNum;i++) 
		{
			if (m_pCharPhysPlaceholders[i])
			{
				m_pCharPhysPlaceholders[i]->GetStatus(&spc);
				if (!spc.pFullEntity)
					m_pCharPhysPlaceholders[i]->SetParams(&pbb);
			}
		}
	}

	if (m_physPlaceholder && !m_physic)
		m_physPlaceholder->SetParams(&pbb);
}


void CEntity::UpdateAIObject( bool bEntityVisible )
{
	// reflect changes in position or orientation to the AI object
	ENTITY_PROFILER

		if (!m_pAIObject)
			return;

	if (!m_pEntitySystem->m_pUpdateAI->GetIVal())
		return;

	if ( m_pHeadBone )
		//	if (m_pHeadBone && (m_pCryCharInstance[0]->GetFlags() & CS_FLAG_DRAW_MODEL))
	{
		Vec3d pos;
		Vec3d angles = m_angles;
		pos = m_pHeadBone->GetBonePosition();


//		// if bound - in vehicle - force eyeheight to make it more noticable
//		if(IsBound())
//			m_pAIObject->SetEyeHeight(pos.z+1);
//		else
		m_pAIObject->SetEyeHeight(pos.z);

		bool bWorldSpace = false;
		if (m_pAIObject->GetProxy())
			bWorldSpace = m_pAIObject->GetProxy()->CustomUpdate(pos,angles);

		if (!bWorldSpace)
		{
			angles.x = 0;
			//	angles.Set(0,0,0);

			Matrix44 mat;

			mat.SetIdentity();

			//mat.RotateMatrix_fix(angles);
			mat=Matrix44::CreateRotationZYX(-gf_DEGTORAD*angles)*mat; //NOTE: angles in radians and negated 

			pos = mat.TransformPointOLD(pos);

			m_pAIObject->SetPos(pos+m_center,false);
			m_pAIObject->SetAngles(m_angles);
		}
		else
		{
			m_pAIObject->SetPos(pos,false);
			m_pAIObject->SetAngles(m_angles);
		}

	}
	else
	{
		m_pAIObject->SetPos(m_center, m_pAIObject->GetType()!=AIOBJECT_SNDSUPRESSOR && m_pAIObject->GetType()!=AIOBJECT_VEHICLE );
		m_pAIObject->SetAngles(m_angles);
	}

	if (bEntityVisible)
		m_pAIObject->Event(AIEVENT_WAKEUP,0);
}

IEntityCamera* CEntity::GetCamera() const
{
	if (m_pCamera)
	{
		return m_pCamera;
	}
	/*if (m_bind)
	{
	return m_bind->GetCamera();
	}*/
	return 0;
}

void CEntity::SetThirdPersonCameraMode(const Vec3d &center, const Vec3d &angles, int mode, float frameTime, float range, int dangleAmmount, IPhysicalEntity *physic)
{
	if (m_pCamera)
	{
		m_pCamera->SetThirdPersonMode(center, angles, mode, frameTime, range, dangleAmmount, physic);
	}
}


void CEntity::SetPhysics(IPhysicalEntity* physic)
{
	if (m_physic)
	{
		// destroy previous physics object.
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);

		// Check if we need to unregister this entity from sector.
		if (m_registeredInSector && CheckFlags(ETY_FLAG_NOT_REGISTER_IN_SECTORS))
		{
			UnregisterInSector();
		}
	}
	m_physic = physic;
	if (m_physic)
	{
		// enable physics calculations.
		SetFlags(ETY_FLAG_CALC_PHYSICS);
	}
}

void CEntity::DestroyPhysics()
{
	if (m_physic)
	{
		m_physicEnabled = false;
		ClearFlags(ETY_FLAG_CALC_PHYSICS);
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);
		m_physic=NULL;
	}
	if (m_physPlaceholder)
	{
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physPlaceholder);
		m_physPlaceholder=NULL;
	}
	if (m_pPhysState)
	{
		delete[] m_pPhysState;
		m_pPhysState = 0;
	}
	m_iPhysType = PHYS_NONE;
}


int CEntity::CreatePhysicalEntityCallback(int iForeignFlags)
{
	if (iForeignFlags & 1<<15)
	{
		int iPos = iForeignFlags>>12 & 7;
		PhysicalizeCharacter(iPos, m_charPhysData[iPos].mass,m_charPhysData[iPos].surface_idx,m_charPhysData[iPos].stiffness_scale, true);
		SendScriptEvent(ScriptEvent_PhysicalizeOnDemand,0);
	}
	else switch (m_iPhysType)
	{
		case PHYS_RIGID: 
			{
				CreateRigidBody(m_PhysData.RigidBody.type,m_PhysData.RigidBody.density,m_PhysData.RigidBody.mass,
					m_PhysData.RigidBody.surface_idx,0,m_PhysData.RigidBody.slot,true);
				SendScriptEvent(ScriptEvent_PhysicalizeOnDemand,0);
				if (m_pPhysState && m_physic)
				{
					CStream stm(m_iPhysStateSize,m_pPhysState);
					m_physic->SetStateFromSnapshot(stm);
					m_physic->PostSetStateFromSnapshot();
					delete[] m_pPhysState; m_pPhysState = 0;
				}
			}
			break;

		case PHYS_STATIC:
			CreateStaticEntity(0,m_PhysData.Static.surface_idx,m_PhysData.Static.slot,true);
	}
	SetUpdateVisLevel(m_eUpdateVisLevel);
	return 1;
}

int CEntity::DestroyPhysicalEntityCallback(IPhysicalEntity *pent)
{
	pe_params_foreign_data pfd;
	pent->GetParams(&pfd);
	if (pfd.iForeignFlags & 1<<15)
		m_pCryCharInstance[pfd.iForeignFlags>>12 & 7]->DestroyCharacterPhysics();
	else 
	{
		if (m_iPhysType==PHYS_RIGID)
		{
			static CStream stm;
			stm.SetSize(0);
			m_physic->GetStateSnapshot(stm);
			m_iPhysStateSize = ((stm.GetSize()-1)>>3)+1;
			if (m_pPhysState)	{
				delete[] m_pPhysState; m_pPhysState = 0;
			}
			if (m_iPhysStateSize)
				memcpy(m_pPhysState = new unsigned char[m_iPhysStateSize],stm.GetPtr(),m_iPhysStateSize);
			ClearFlags(ETY_FLAG_CALC_PHYSICS);
		}
		m_physic = 0;
	}

	return 1;
}

void CEntity::EnablePhysics(bool enable)
{
	if (!enable && m_physPlaceholder)
	{
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physPlaceholder);
		m_physPlaceholder = 0; m_physic = 0;
		m_physicEnabled = false;
		if (m_pPhysState)	{
			delete[] m_pPhysState; m_pPhysState = 0;
		}
		return;
	}
	else if (enable && !m_physic && m_iPhysType!=PHYS_NONE)
	{
		switch (m_iPhysType)
		{
		case PHYS_RIGID: 
			CreateRigidBody(m_PhysData.RigidBody.type,m_PhysData.RigidBody.density,m_PhysData.RigidBody.mass,
				m_PhysData.RigidBody.surface_idx,0,m_PhysData.RigidBody.slot);
			break;
		case PHYS_STATIC:
			CreateStaticEntity(0,m_PhysData.Static.surface_idx,m_PhysData.Static.slot);
		}
		m_physicEnabled = true;
	} 
	else
	{
		if (!m_physic)
			return;

		if (enable)
		{
			m_physicEnabled = true;
			SetFlags(ETY_FLAG_CALC_PHYSICS);
			m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic, 2);
		}
		else 
		{
			m_physicEnabled = false;
			ClearFlags(ETY_FLAG_CALC_PHYSICS);
			m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic, 1);
		}
	}
}


void CEntity::AddImpulse(int ipart, Vec3d pos, Vec3d impulse,bool bPos,float fAuxScale)
{
#ifndef _ISNOTFARCRY
	IXGame *pXGame = (IXGame*) GetISystem()->GetIGame();
#endif

	IPhysicalEntity *physic = GetPhysics();
	if (physic && (!m_bIsADeadBody 
#ifndef GERMAN_GORE_CHECK
		|| (m_pEntitySystem->m_pHitDeadBodies->GetIVal() 
	#ifndef _ISNOTFARCRY
		&& pXGame->GoreOn()
	#endif
		)
#endif
		))
	{
		Vec3d mod_impulse = impulse;
		if (!(physic->GetStatus(&pe_status_nparts())>5 && physic->GetType()==PE_ARTICULATED))
		{	// don't scale impulse for complex articulated entities
			pe_status_dynamics sd;
			float minVel = m_pEntitySystem->m_pMinImpulseVel->GetFVal();
			if (minVel>0 && physic->GetStatus(&sd) && mod_impulse*mod_impulse<sqr(sd.mass*minVel))
			{
				float fScale = m_pEntitySystem->m_pImpulseScale->GetFVal();
				if (fScale>0)
					mod_impulse *= fScale;
				else if (sd.mass<m_pEntitySystem->m_pMaxImpulseAdjMass->GetFVal())
					mod_impulse = mod_impulse.normalized()*(minVel*sd.mass);
			}
		}
		pe_action_impulse ai;
		ai.partid = ipart;
		if(bPos)
			ai.point = pos;
		ai.impulse = mod_impulse;
		physic->Action(&ai);
	}

	if (m_bVisible && (!m_physic || m_physic->GetType()!=PE_LIVING 
#ifndef GERMAN_GORE_CHECK
		|| (m_pEntitySystem->m_pHitCharacters->GetIVal() 
	#ifndef _ISNOTFARCRY
		&& pXGame->GoreOn()
	#endif
		)
#endif
		))
		if (bPos) for (int i = 0; i < MAX_ANIMATED_MODELS; i++)
			if (m_pCryCharInstance[i])
				m_pCryCharInstance[i]->AddImpact(ipart, pos, impulse*fAuxScale);
}

void CEntity::SetRegisterInSectors(bool needToRegister)
{
	if (needToRegister)
	{
		if (!m_pEntityRenderState)
			InitEntityRenderState();
		ClearFlags(ETY_FLAG_NOT_REGISTER_IN_SECTORS);
		if (!m_registeredInSector)
		{
			RegisterInSector();
		}
	}
	else
	{
		SetFlags(ETY_FLAG_NOT_REGISTER_IN_SECTORS);
		if (m_registeredInSector)
		{
			UnregisterInSector();
		}
	}
}

void CEntity::GetEntityDesc(CEntityDesc &desc) const
{
	desc.id = m_nID;
	desc.ClassId = m_ClassId;
	desc.name = m_name.c_str();
	desc.netPresence = m_netPresence;
	desc.angles=GetAngles();
	desc.pos=GetPos();
	desc.scale = m_fScale;
	if(m_pContainer)
		m_pContainer->GetEntityDesc(desc);
}

//////////////////////////////////////////////////////////////////////////
void CEntity::GetHelperPosition(const char *helper, Vec3d &vPos, bool objectspace)
{
	if (m_objects.empty())
		return;

	std::vector < CEntityObject>::iterator oi;

	bool bDone = false;
	for (oi = m_objects.begin(); !bDone && oi != m_objects.end(); oi++)
	{
		CEntityObject eo =(*oi);
		if (!eo.object)
			continue;

		vPos=eo.object->GetHelperPos(helper); 

		bDone = !IsEquivalent(vPos,Vec3d(0, 0, 0));
	}

	// the character might also contain a helper we'd like
	if (!bDone)
	{
		ICryCharInstance *character = GetCharInterface()->GetCharacter(PLAYER_MODEL_IDX);	

		if (character)
			vPos = character->GetHelperPos(helper);

		bDone = !IsEquivalent(vPos,Vec3d(0, 0, 0));
	}

	// found...
	if (bDone && !objectspace)		
	{
		//OPTIMISED_BY_IVO  
		Matrix44 mtx=Matrix34::CreateRotationXYZ( Deg2Rad(m_angles),m_center);
		mtx=GetTransposed44(mtx);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44

		vPos = mtx.TransformPointOLD(vPos);
	}
}


bool CEntity::IsObjectLoaded(unsigned int slot)
{
	if (slot < m_objects.size())
	{
		if (m_objects[slot].object != 0)
			return true;
	}
	return false;
}

bool CEntity::SetEntityObject(unsigned int slot, const CEntityObject &object)
{
	if (slot < m_objects.size())
	{
		if (m_objects[slot].object != object.object)
		{
			// Release old object.
			m_pISystem->GetI3DEngine()->ReleaseObject(m_objects[slot].object);
		}
		/*		if (m_objects[slot].image != object.image)
		{
		m_pISystem->GetIRenderer()->RemoveTexture(m_objects[slot].image);
		}*/
		m_objects[slot] = object;
		return true;
	}

	return false;
}

bool CEntity::GetEntityObject(unsigned int slot, CEntityObject &object)
{
	if (slot < m_objects.size())
	{
		object = m_objects[slot];
		return true;
	}
	return false;
}


void CEntity::RegisterInSector()
{
	// register this entity for rendering
	if (!m_registeredInSector && 
		GetRadius() && 
		!IsHidden() && 
		m_pEntityRenderState /*&& 
												 IsEntityHasSomethingToRender()*/) 
	{
		m_pISystem->GetI3DEngine()->RegisterEntity(this);
		m_registeredInSector = true;
	}
}

void CEntity::UnregisterInSector()
{
	// unregister this entity from the list of sectors
	if (m_registeredInSector)
	{
		m_pISystem->GetI3DEngine()->UnRegisterEntity(this);
		m_registeredInSector = false;
	}
}

void CEntity::ForceRegisterInSectors()
{
	if (!CheckFlags(ETY_FLAG_NOT_REGISTER_IN_SECTORS))
	{
		UnregisterInSector();
		RegisterInSector();
	}
}


// 
void CEntity::SinkRebind(IEntitySystemSink *pSink)
{
	for( BINDLIST::iterator iCurBind=m_lstBindings.begin(); iCurBind!=m_lstBindings.end(); ++iCurBind)
	{
	CEntity *pChild =(CEntity *) m_pEntitySystem->GetEntity((*iCurBind));	
		if(pChild)
			pSink->OnBind(GetId(),(*iCurBind),pChild->m_cBind);
	}
}

// bSetPos -- needed to be able to set position before OnBind is called (it can set some other position)
// 
void CEntity::Bind(EntityId id,unsigned char cBind, const bool bClientOnly, const bool bSetPos ) 
{
	// safe upcast since we know what the entity system holds
	CEntity *pEntity =(CEntity *) m_pEntitySystem->GetEntity(id);
	if (!pEntity)
		return;

	// only add if it is not there already
	//if (std::find(m_lstBindings.begin(), m_lstBindings.end(), pEntity) == m_lstBindings.end())
	if (std::find(m_lstBindings.begin(), m_lstBindings.end(), id) == m_lstBindings.end())
	{
		m_bUpdateBinds = 1;
		m_lstBindings.push_back(id);
		pEntity->m_bIsBound = 1;
		pEntity->m_bForceBindCalculation = 0;
		// [kirill] makes problem for network
		// so just set position after you bind something
		if(bSetPos)
			pEntity->m_realcenter = pEntity->m_center;
		pEntity->m_realangles = pEntity->m_angles;
		pEntity->m_idBoundTo = GetId();
		pEntity->m_cBind = cBind;

		if(!bClientOnly)																				// to prevent circular loop between client and server
			m_pEntitySystem->OnBind(GetId(),id,cBind);

		OnBind( pEntity, cBind); 
	}
}

void CEntity::Unbind(EntityId id,unsigned char cBind, const bool bClientOnly)
{
	CEntity *pEntity =(CEntity *) m_pEntitySystem->GetEntity(id);

	//if (std::find(m_lstBindings.begin(), m_lstBindings.end(), pEntity) != m_lstBindings.end())
	if (std::find(m_lstBindings.begin(), m_lstBindings.end(), id) != m_lstBindings.end())
	{
		m_lstBindings.remove(id);
		if (pEntity)
		{
			pEntity->m_bIsBound = false;
	
			if(!bClientOnly)																			// to prevent circular loop between client and server
				m_pEntitySystem->OnUnbind(GetId(),id,cBind);

			OnUnBind( pEntity, cBind); 
		}
	}
	m_bUpdateBinds = !m_lstBindings.empty();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::ResolveCollision()
{	
	ENTITY_PROFILER

		if (!m_pEntitySystem->m_pUpdateCollision->GetIVal())
			return;

	// resolve collisions
	IPhysicalWorld *pWorld = m_pISystem->GetIPhysicalWorld();
	IPhysicalEntity **ppColliders;
	int cnt = 0;

	Vec3d mins, maxs;
	GetBBox(mins, maxs);

	if (mins.x > maxs.x)
	{
		// Bad bounding box.
		return;
	}

	cnt = pWorld->GetEntitiesInBox(mins, maxs, ppColliders, 14 );
	if (cnt > 0)
	{
		static std::vector<IPhysicalEntity*> s_colliders;
		s_colliders.resize(cnt);
		memcpy( &s_colliders[0],ppColliders,cnt*sizeof(IPhysicalEntity*) );

		// execute on collide for all of the entities
		for (int i = 0; i < cnt; i++)
		{
			CEntity *pEntity = (CEntity *)s_colliders[i]->GetForeignData();
			//IStatObj *pStatObj = (IStatObj *) ppColliders[i]->GetForeignData(1);

			if (pEntity) 
			{
				if (pEntity->IsGarbage())
					continue;

				if (pEntity->GetId() == m_nID) 
					continue;

				if (!m_pEntitySystem->m_pUpdateCollisionScript->GetIVal())
					continue;

				CallStateFunction( ScriptState_OnContact,pEntity->GetScriptObject() );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
IStatObj *CEntity::GetIStatObj(unsigned int pos)
{
	CEntityObject object;

	if (pos < m_objects.size())
	{
		object = m_objects[pos];
		return object.object;
	}

	return (NULL);
}


void CEntity::SendScriptEvent(enum EScriptEventId Event, IScriptObject *pParamters, bool *pRet)
{
	// Server side always first.
	bool bClientReturn=true;
	if (m_pServerState && m_pServerState->pFunction[ScriptState_OnEvent])
	{
		m_pScriptSystem->BeginCall(m_pServerState->pFunction[ScriptState_OnEvent]);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam((int)Event);
		if (pParamters)
			m_pScriptSystem->PushFuncParam(pParamters);
		else
			m_pScriptSystem->PushFuncParam(false);
		if (pRet){
			m_pScriptSystem->EndCall(*pRet);
		}
		else
			m_pScriptSystem->EndCall();
		// Only use return value if we ain't got a server event
		bClientReturn=false;
	}
	// Client side always second.
	if (m_pClientState && m_pClientState->pFunction[ScriptState_OnEvent])
	{
		m_pScriptSystem->BeginCall(m_pClientState->pFunction[ScriptState_OnEvent]);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam((int)Event);
		if (pParamters)
			m_pScriptSystem->PushFuncParam(pParamters);
		else
			m_pScriptSystem->PushFuncParam(false);

		// Only use return value if we ain't got a server event
		if (pRet && bClientReturn)
			m_pScriptSystem->EndCall(*pRet);
		else
			m_pScriptSystem->EndCall();
	}
}

void CEntity::SendScriptEvent(enum EScriptEventId Event, const char *str, bool *pRet )
{
	// Server side always first.
	bool bClientReturn=true;
	if (m_pServerState && m_pServerState->pFunction[ScriptState_OnEvent])
	{
		m_pScriptSystem->BeginCall(m_pServerState->pFunction[ScriptState_OnEvent]);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam((int)Event);
		if (str)
			m_pScriptSystem->PushFuncParam( str );
		else
			m_pScriptSystem->PushFuncParam( "" );
		if (pRet)
			m_pScriptSystem->EndCall(*pRet);
		else
			m_pScriptSystem->EndCall();
		bool bClientReturn=false;
	}
	// Client side always second.
	if (m_pClientState && m_pClientState->pFunction[ScriptState_OnEvent])
	{
		m_pScriptSystem->BeginCall(m_pClientState->pFunction[ScriptState_OnEvent]);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam((int)Event);
		if (str)
			m_pScriptSystem->PushFuncParam( str );
		else
			m_pScriptSystem->PushFuncParam(false);

		if (pRet && bClientReturn)
			m_pScriptSystem->EndCall(*pRet);
		else
			m_pScriptSystem->EndCall();
	}
}


void CEntity::SendScriptEvent(enum EScriptEventId Event, int nParam, bool *pRet )
{
	// Server side always first.
	bool bClientReturn=true;
	if (m_pServerState && m_pServerState->pFunction[ScriptState_OnEvent])
	{
		m_pScriptSystem->BeginCall(m_pServerState->pFunction[ScriptState_OnEvent]);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam((int)Event);
		m_pScriptSystem->PushFuncParam(nParam);
		if (pRet)
			m_pScriptSystem->EndCall(*pRet);
		else
			m_pScriptSystem->EndCall();
		bClientReturn = false;
	}
	// Client side always second.
	if (m_pClientState && m_pClientState->pFunction[ScriptState_OnEvent])
	{
		m_pScriptSystem->BeginCall(m_pClientState->pFunction[ScriptState_OnEvent]);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam((int)Event);
		m_pScriptSystem->PushFuncParam(nParam);
		if (pRet&&bClientReturn)
			m_pScriptSystem->EndCall(*pRet);
		else
			m_pScriptSystem->EndCall();
	}
}

void CEntity::RotateTo(const Vec3d &angles, bool bUpdatePhysics)
{
	//	Vec3d diff;
	//	diff = angles - m_angles;

	Ang3 diff = angles;
	diff.Snap360();
	diff -= m_angles;

	if (fabs(diff.x) < ANGLES_EPSILON && fabs(diff.y) < ANGLES_EPSILON && fabs(diff.z) < ANGLES_EPSILON)
		return;

	m_angles += diff;  
	m_angles.Snap360();
	//	SetAngles( angles );
	//	SetAngles(m_angles + diff);

	if (bUpdatePhysics && m_physic &&(m_flags & ETY_FLAG_CALC_PHYSICS))
	{
		SetPhysAngles( m_angles );
		//		pe_params_pos pp;
		//		pp.q = quaternionf(m_angles.z*(PI/180.0f),m_angles.y*(PI/180.0f),m_angles.x*(PI/180.0f));
		//		m_physic->SetParams(&pp);
	}
}

IAIObject * CEntity::GetAI()
{
	if (m_pAIObject)
		return m_pAIObject;
	return 0; 
}

void CEntity::LoadBreakableObject(const char *pFileName)
{
std::vector < CEntityObject>::iterator it;
	for (it = m_objects.begin(); it < m_objects.end(); it++) 
	{
		if ((* it).object)
			m_pISystem->GetI3DEngine()->ReleaseObject((* it).object);
	}
	m_objects.clear();

	{ // load unbreaked (initial) version
		IStatObj * cobj;
		cobj = m_pISystem->GetI3DEngine()->MakeObject(pFileName,"unbreaked");
		if (!cobj)	
			m_pISystem->GetILog()->Log("CEntity::LoadBreakableObject: Could not load unbreaked version of object %s", pFileName);
		else
		{
			CEntityObject ceo;
			ceo.object = cobj;
			ceo.pos = Vec3d(0,0,0);
			m_objects.push_back(ceo);
		}
	}

	// load broken version
	{
		IStatObj * cobj;
		cobj = m_pISystem->GetI3DEngine()->MakeObject(pFileName,"broken");
		if (!cobj)	
			m_pISystem->GetILog()->Log("CEntity::LoadBreakableObject: Could not load unbreaked version of object %s", pFileName);
		else
		{
			CEntityObject ceo;
			ceo.object = cobj;
			ceo.pos = Vec3d(0,0,0);
			m_objects.push_back(ceo);
		}
	}

	// load pieces of object
	char geomname[50];
	int i=1;

	sprintf(geomname,"piece%02d",i);
	IStatObj * cobj;
	while (cobj = m_pISystem->GetI3DEngine()->MakeObject(pFileName,geomname))	
	{
		if (cobj->IsDefaultObject())
			break;

		CEntityObject ceo;
		ceo.object = cobj;
		ceo.pos = Vec3d(0,0,0);
		m_objects.push_back(ceo);
		i++;
		sprintf(geomname,"piece%02d",i);
	}

	if (i==1)	
		m_pISystem->GetILog()->Log("CEntity::LoadBreakableObject: Could not load pieces of breakable object %s", pFileName);

	m_pISystem->GetI3DEngine()->FreeEntityRenderState(this);
	InitEntityRenderState();


	//UnregisterInSector();
	// leave this call... it will cause the geometry to be properly registered
	CalcWholeBBox(); 

	//	RegisterInSector();
}


void CEntity::OnStartAnimation(const char *sAnimation)
{
	ENTITY_PROFILER
		//char sTemp[200];
		//sprintf(sTemp,"OnStartAnimation(%s)\n",sAnimation);
		//::OutputDebugString(sTemp);

		SendScriptEvent( ScriptEvent_StartAnimation,sAnimation );
}

void CEntity::OnAnimationEvent(const char *sAnimation,AnimSinkEventData UserData)
{
	ENTITY_PROFILER

	USER_DATA udUserData=(USER_DATA)UserData.p;
	//_SmartScriptObject pObject(m_pScriptSystem);
	m_pAnimationEventParams->SetValue("animation",sAnimation);

//	if(udUserData>0)
	m_pAnimationEventParams->SetToNull("number");
	m_pAnimationEventParams->SetToNull("userdata");
	if (UserData.n == svtUserData)
	{
		m_pAnimationEventParams->SetValue("userdata",udUserData);
	}
	else if (UserData.n == svtNumber)
	{
		m_pAnimationEventParams->SetValue("number", (int) udUserData);
	}
	// add other types as necessary

/*
	if(udUserData!=USER_DATA(-1))
		m_pAnimationEventParams->SetValue("userdata",udUserData);
	else
		m_pAnimationEventParams->SetToNull("userdata");
*/

	SendScriptEvent( ScriptEvent_AnimationKey,m_pAnimationEventParams );
}

void CEntity::OnEndAnimation(const char *sAnimation)
{
	ENTITY_PROFILER
	//char sTemp[200];
	//sprintf(sTemp,"OnEndAnimation(%s)\n",sAnimation);
	//OutputDebugString(sTemp);

	SendScriptEvent( ScriptEvent_EndAnimation,sAnimation );
}

//////////////////////////////////////////////////////////////////////////
// Calls Event_##sEvent function from script.
void CEntity::CallEventHandler( const char *sEvent )
{
	if (m_pScriptObject)
	{
		char funcName[1024];
		strcpy( funcName,"Event_" );
		strcat( funcName,sEvent );

		HSCRIPTFUNCTION pEventFunc = 0;
		if (m_pScriptObject->GetValue( funcName,pEventFunc ))
		{
			m_pScriptSystem->BeginCall(pEventFunc);
			// Pass itself as a sender.
			m_pScriptSystem->PushFuncParam(m_pScriptObject);
			m_pScriptSystem->EndCall();
			m_pScriptSystem->ReleaseFunc(pEventFunc);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
const char* CEntity::GetState()
{
	return m_sStateName.c_str();
}

//////////////////////////////////////////////////////////////////////////
int CEntity::GetStateIdx()
{
	EntityStateMapItor itor;
	itor=m_mapStates.find(m_sStateName);
	if(itor!=m_mapStates.end())
		return itor->second;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
bool CEntity::IsInState( int nState )
{
	EntityStateMapItor itor=m_mapStates.begin();
	while(itor!=m_mapStates.end())
	{
		if(itor->second==nState)
		{
			return true;
		}
		++itor;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool CEntity::GotoState( int nState )
{
	EntityStateMapItor itor=m_mapStates.begin();
	while(itor!=m_mapStates.end())
	{
		if(itor->second==nState)
		{
			GotoState(itor->first.c_str());
			return true;
		}
		++itor;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool CEntity::GotoState( const char *sState )
{
	if (!m_pScriptObject)
		return false;

	// State name is case sensetive.
	if (m_sStateName==sState)
	{
		// Entity is already in this state, so ignore this call.
		return true;
	}

	_SmartScriptObject pServerTable(m_pScriptSystem,true);
	_SmartScriptObject pClientTable(m_pScriptSystem,true);

	// Get Server table if exist.
	bool bServerTable = m_pScriptObject->GetValue( SCRIPT_SERVER_STATE,pServerTable );

	// Get Client table if exist.
	bool bClientTable = m_pScriptObject->GetValue( SCRIPT_CLIENT_STATE,pClientTable );

	SScriptState newServerState;
	SScriptState newClientState;

	// If state name is empty assume we want to reference main entity script table.
	if (sState[0] != 0)
	{
		// Non empty state name

		// Find if state with givven name exist.
		bool bStateFound = false;

		// Check for table in Server part.
		if (m_pServerState && bServerTable)
		{
			_SmartScriptObject pStateTable(m_pScriptSystem,true);
			if (pServerTable->GetValue( sState,pStateTable ))
			{
				// If state table found initialize it
				InitializeStateTable( pStateTable,newServerState );
				bStateFound = true;
			}
		}

		// Check for table in Client part.
		if (m_pClientState && bClientTable)
		{
			_SmartScriptObject pStateTable(m_pScriptSystem,true);
			if (pClientTable->GetValue( sState,pStateTable ))
			{
				// If state table found initialize it
				InitializeStateTable( pStateTable,newClientState );
				bStateFound = true;
			}
		}

		if (!bStateFound)
		{
			_SmartScriptObject pStateTable(m_pScriptSystem,true);
			if (m_pScriptObject->GetValue( sState,pStateTable ))
			{
				// If state table found in entity table initialize it for both client and server states.
				bStateFound = true;
				if (m_pServerState)
					InitializeStateTable( pStateTable,newServerState );

				// Initialize Client State table only if no server state also exist.
				if (m_pClientState && !m_pServerState)
					InitializeStateTable( pStateTable,newClientState );
			}
		}

		if (!bStateFound)
		{
			m_pISystem->GetILog()->Log("[ENTITYWARNING] GotoState of name='%s' called with unknown State: %s",GetName(),sState );
			return false;
		}
	}
	else
	{
		if (m_pServerState)
		{
			if (bServerTable)
				// If Have Server table initialize state to this Table.
				InitializeStateTable( pServerTable,newServerState );
			else
				// If Server table not exist use Entity mail table.
				InitializeStateTable( m_pScriptObject,newServerState );
		}

		if (m_pClientState && bClientTable)
		{
			InitializeStateTable( pClientTable,newClientState );
		}
	}

	// Call End state event.
	CallStateFunction( ScriptState_OnEndState );

	// If state changed kill old timer.
	KillTimer();

	// Goto to new state.
	SScriptState newState;
	m_sStateName = sState;

	// Copy states.
	if (m_pServerState)
	{
		ReleaseStateTable(*m_pServerState);
		*m_pServerState = newServerState;
	}
	if (m_pClientState){
		ReleaseStateTable(*m_pClientState);
		*m_pClientState = newClientState;
	}

	// Call BeginState event.
	CallStateFunction( ScriptState_OnBeginState );

	//////////////////////////////////////////////////////////////////////////
	// Repeat check if update script function is implemented.
	m_bUpdateScript = IsStateFunctionImplemented(ScriptState_OnUpdate);
	//////////////////////////////////////////////////////////////////////////
	// Check if need ResolveCollisions for OnContact script function.
	m_bUpdateOnContact = IsStateFunctionImplemented(ScriptState_OnContact);
	//////////////////////////////////////////////////////////////////////////

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::ReleaseStateTable(SScriptState &scriptState )
{
	// Query state table for supported functions.
	for (int stateFunc = 0; stateFunc < sizeof(scriptState.pFunction)/sizeof(scriptState.pFunction[0]); stateFunc++)
	{
		assert( stateFunc < sizeof(s_ScriptStateFunctions)/sizeof(s_ScriptStateFunctions[0]) );
		if(scriptState.pFunction[stateFunc])
			m_pScriptSystem->ReleaseFunc(scriptState.pFunction[stateFunc]);
	}
}
//////////////////////////////////////////////////////////////////////////
void CEntity::InitializeStateTable( IScriptObject *pStateTable,SScriptState &scriptState )
{
	// Query state table for supported functions.
	for (int stateFunc = 0; stateFunc < sizeof(scriptState.pFunction)/sizeof(scriptState.pFunction[0]); stateFunc++)
	{
		assert( stateFunc < sizeof(s_ScriptStateFunctions)/sizeof(s_ScriptStateFunctions[0]) );
		scriptState.pFunction[stateFunc] = 0;
		pStateTable->GetValue( s_ScriptStateFunctions[stateFunc],scriptState.pFunction[stateFunc] );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::IsInState( const char *sState )
{
	// State name is case sensetive.
#if defined(LINUX)
	if (strcasecmp(sState,m_sStateName.c_str()) == 0)
#else
	if (strcmp(sState,m_sStateName.c_str()) == 0)
#endif
	{
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnTimer( int nTimerId )
{
	ENTITY_PROFILER
	if (m_pEntitySystem->m_pDebugTimer->GetIVal())
	{
		CryLog( "OnTimer: %d ms for Entity: %s (%s)",nTimerId,m_name.c_str(),m_sClassName.c_str() );
	}
	if (m_pEntitySystem->m_pUpdateTimer->GetIVal())
		CallStateFunction( ScriptState_OnTimer,nTimerId );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnDamage( IScriptObject *pObj )
{
	if(m_bGarbage)
		return;

	//
	//	store hit parameters
	//	to use by CPlayer::StartDeathAnimation()
//	CScriptObjectVector deathDir(m_pScriptSystem,true);
	if(!pObj->GetValue("weapon_death_anim_id", m_DeathType))
	    m_DeathType = 100;
	pObj->GetValue("dir", m_DeathDirection);
	if(pObj->GetValue("ipart", m_DeathZone))
	    m_DeathZone = GetBoneHitZone(m_DeathZone);
	else
	    m_DeathZone = 100;

	CallStateFunction( ScriptState_OnDamage,pObj );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnEnterArea( IEntity* entity, const int areaID )
{
	CallStateFunction( ScriptState_OnEnterArea,entity->GetScriptObject(),areaID );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnLeaveArea( IEntity* entity, const int areaID )
{
	CallStateFunction( ScriptState_OnLeaveArea,entity->GetScriptObject(),areaID );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnProceedFadeArea( IEntity* entity, const int areaID, const float fadeCoeff )
{
	// Server side always first.
	CallStateFunction( ScriptState_OnProceedFadeArea,entity->GetScriptObject(),areaID,fadeCoeff );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnBind( IEntity* entity, const char par )
{
	// don't call it on server - client only
	if (m_pClientState && m_pClientState->pFunction[ScriptState_OnBind])
	{
		m_pScriptSystem->BeginCall(m_pClientState->pFunction[ScriptState_OnBind]);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam(entity->GetScriptObject());
		m_pScriptSystem->PushFuncParam(par);
		m_pScriptSystem->EndCall();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnUnBind( IEntity* entity, const char par )
{
	// don't call it on server - client only
	if (m_pClientState && m_pClientState->pFunction[ScriptState_OnUnBind])
	{
		m_pScriptSystem->BeginCall(m_pClientState->pFunction[ScriptState_OnUnBind]);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam(entity->GetScriptObject());
		m_pScriptSystem->PushFuncParam(par);
		m_pScriptSystem->EndCall();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetTimer(int msec)
{	
	// Remove old timer.
	if (m_nTimer > 0)
		m_pEntitySystem->RemoveTimerEvent( m_nID );

	//m_nStartTimer = (int)(m_pISystem->GetITimer()->GetCurrTime()*1000);
	m_nTimer = msec;
	//m_awakeCounter = 2; // As long as there is timer, must stay awake.

	if (m_nTimer > 0)
	{
		SEntityTimerEvent event;
		event.timerId = msec;
		event.entityId = m_nID;
		m_pEntitySystem->AddTimerEvent( msec,event );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::KillTimer()
{
	//if (m_nTimer > 0)
	m_pEntitySystem->RemoveTimerEvent( m_nID );
	m_nTimer = -1;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::RegisterState(const char *sState)
{
	EntityStateMapItor itor=m_mapStates.find(sState);
	if(itor!=m_mapStates.end())
		return;
	m_mapStates.insert(EntityStateMapItor::value_type(sState,m_cLastStateID));
	m_cLastStateID++;
}


void CEntity::ApplyForceToEnvironment(const float radius, const float force)
{
	m_pISystem->GetI3DEngine()->ApplyForceToEnvironment( m_center, radius, force);

}

//
//------------------------------------------------------------------
//!determens on each side if entity direction is
//!1-front 2-back 3-left 4- right
int CEntity::GetSide(const Vec3d& direction)
{
	bool	forward;

	// Get our current orientation angles
	Vec3d playerDir = GetAngles();
	// Convert to a direction vector
	playerDir=ConvertToRadAngles(playerDir);
	// don't need vertical component to determin left/right direction
	playerDir.z = 0;
	playerDir = GetNormalized(playerDir);
	// Create a working copy of the direction vector
	Vec3d playerRight = playerDir;
	// Create a vector pointing up
	Vec3d up(0,0,1);
	// Create a vector perpendicular to both the direction vector and the up vector
	// This vector will point to the players right
	playerRight = playerRight.Cross(up);
	// Generate the Cos of the angle between the player facing 
	// direction and the current player velocity
	float dotDir = playerDir.Dot( direction );
	// Determine if the velocity vector and the facing vector differ by less than 90 degrees
	if (dotDir <= 0.0)
		// We have determined that the player's general direction 
		// matches direction (within 90 degrees)
		forward = true;
	else
		forward = false;

	if(forward)
		return 1;
	else
		return 2;
}
//
//------------------------------------------------------------------

void CEntity::AttachToBone(EntityId id,const char* boneName)
{
	// safe upcast since we know what the entity system holds
	CEntity *pEntity =(CEntity *) m_pEntitySystem->GetEntity(id);
	ICryCharInstance *character = GetCharInterface()->GetCharacter(PLAYER_MODEL_IDX);	

	if (character)
	{
		//		m_pSelectedInfo->pBindInfo = 
		character->AttachObjectToBone( pEntity->GetIStatObj(0),boneName, false );
	}
}

//
//------------------------------------------------------------------
BoneBindHandle CEntity::AttachObjectToBone(int slot,const char* boneName,bool bMultipleAttachments, bool bUseZOffset )
{
	ICryCharInstance *character = GetCharInterface()->GetCharacter(PLAYER_MODEL_IDX);	

	if (character)
	{
		if (!bMultipleAttachments)
		{
			// Old way.
			//		m_pSelectedInfo->pBindInfo = 
			return character->AttachObjectToBone( GetIStatObj(slot),boneName, false, bUseZOffset ? ICryCharInstance::FLAGS_ATTACH_ZOFFSET : 0 );
		}
		else
		{
			// New way.
			int boneid = character->GetModel()->GetBoneByName(boneName);
			if (boneid >= 0)
			{
				return character->AttachToBone( GetIStatObj(slot),boneid, bUseZOffset ? ICryCharInstance::FLAGS_ATTACH_ZOFFSET : 0);
			}
		}
	}
	return -1;
}

//
//------------------------------------------------------------------
void CEntity::DetachObjectToBone( const char* boneName,BoneBindHandle objectBindHandle/* =-1  */ )
{
	ICryCharInstance *character = GetCharInterface()->GetCharacter(PLAYER_MODEL_IDX);	

	if (character)
	{
		if (objectBindHandle == -1)
		{
			// Old way.
			//		m_pSelectedInfo->pBindInfo = 
			character->AttachObjectToBone( NULL, boneName, false );
		}
		else
		{
			character->Detach( objectBindHandle );
		}
	}
}


// returns true if the entity changed in any way trough moving or animation

bool CEntity::HasChanged(void)
{
	if (m_pCryCharInstance[0]) 
		if (m_pCryCharInstance[0]->IsCharacterActive())
			return true;
	
	return IsMoving();
}

void CEntity::CalculateInWorld(void)
{

		// rotate the point offset
		Vec3d vBpos = m_matParentMatrix.TransformPointOLD(m_realcenter);		

		MoveTo(vBpos);

		Matrix44 tempMat = GetTransposed44(m_matParentMatrix);
		tempMat.NoScale();
		//M2Q_CHANGED_BY_IVO
		//CryQuat cxquat(m_matParentMatrix);
		//CryQuat cxquat = CovertMatToQuat<float>( GetTransposed44(m_matParentMatrix) );
		CryQuat cxquat = Quat( tempMat );

		CryQuat rxquat;
		rxquat.SetRotationXYZ(DEG2RAD(m_realangles));

		CryQuat result = cxquat*rxquat;

		Vec3d finalangles = Ang3::GetAnglesXYZ(Matrix33(result));

		RotateTo(RAD2DEG(finalangles));
}

void CEntity::ForceBindCalculation(bool bEnable)
{
	m_bForceBindCalculation = bEnable;
	//m_realcenter = m_center;
	//m_realangles = m_angles;
	//@FIXME: Real scale..
}

void CEntity::SetParentLocale(const Matrix44 & matParent)
{
	// If matrices differ, entity moved so update it.
	if (memcmp(&m_matParentMatrix,&matParent,sizeof(matParent)) != 0)
		m_awakeCounter = 1; // Awake for at least one frame.
	m_matParentMatrix = matParent;
}

//
//--------------------------------------------------------------------------------------
bool	CEntity::InitLight( const char* sImg, const char* sShader, bool bUseAsCube, float fAnimSpeed, int nLightStyle, float fCoronaScale )
{
	if (m_pDynLight)
		delete m_pDynLight;
	m_pDynLight = new CDLight();

	if(sImg && sImg[0])
	{
    m_pDynLight->m_fAnimSpeed = fAnimSpeed;
    int nFlags2 = FT2_FORCECUBEMAP;
    if (bUseAsCube)
      nFlags2 |= FT2_REPLICATETOALLSIDES;
    if (fAnimSpeed)
      nFlags2 |= FT2_CHECKFORALLSEQUENCES;
		m_pDynLight->m_pLightImage = m_pISystem->GetIRenderer()->EF_LoadTexture(sImg, 0, nFlags2, eTT_Cubemap);
		if(!m_pDynLight->m_pLightImage || !m_pDynLight->m_pLightImage->IsTextureLoaded())
			CryWarning(VALIDATOR_MODULE_ENTITYSYSTEM,VALIDATOR_WARNING,"Light projector texture not found: %s", sImg);
		m_pDynLight->m_Flags = DLF_PROJECT;
	}
	else
		m_pDynLight->m_Flags = DLF_POINT;
  m_pDynLight->m_nLightStyle = nLightStyle;
  m_pDynLight->m_fCoronaScale = fCoronaScale;

	if(sShader && sShader[0])
		m_pDynLight->m_pShader = m_pISystem->GetIRenderer()->EF_LoadShader((char*)sShader, eSH_World);

  InitEntityRenderState();

	return true;
}


//
//--------------------------------------------------------------------------------------
CDLight	*CEntity::GetLight( )
{
	return m_pDynLight;
}


//----------------------------------------------------------------------------------------------------
//
int CEntity::GetBoneHitZone( int boneIdx ) const
{
	ICryCharInstance *pIChar = GetCharInterface()->GetCharacter(0);

	if (!pIChar)
		return 0;

	int hitZone = pIChar->GetDamageTableValue(boneIdx);
#ifdef _DEBUG
	const char *szBoneName = pIChar->GetModel()->GetBoneName(boneIdx);
#endif
//	TRACE( "%s hit", name );

	return  pIChar->GetDamageTableValue(boneIdx);

}


//----------------------------------------------------------------------------------------------------
//	hit parameters
//	set by OnDamage()
//	used by CPlayer::StartDeathAnimation()
void CEntity::GetHitParms( int &deathType, int &deathDir, int &deathZone ) const
{
	deathType = m_DeathType;				
	deathDir = m_DeathDirection;
	deathZone = m_DeathZone;
}

//
//--------------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
void CEntity::Hide( bool bHide)
{
	if (m_bHidden != bHide)// && m_physic != NULL)
	{
		SetRndFlags(ERF_HIDDEN,bHide);

		bool bEnablePhysics = !bHide;
		if (m_physicEnabled != bEnablePhysics)
			EnablePhysics( bEnablePhysics );

		m_bHidden = bHide;

		if(!bHide)
			ForceRegisterInSectors();
		
		if (!bHide)
			CryLogComment( "Entity %s Unhidden",m_name.c_str() );
		else
			CryLogComment( "Entity %s Hidden",m_name.c_str() );
	}
	m_bHidden = bHide;
}


void CEntity::UpdateHierarchy( SEntityUpdateContext &ctx )
{

	if (!m_lstBindings.empty())
	{
	ENTITY_PROFILER

	//[kirill] 
	//	when we update child - it can unbind itself - so m_lstBindings changes - let's keep a copy
		static std::vector<EntityId> binds;
		binds.resize(0);
	//this does not work on AMD64
	//	binds.insert( binds.end(),m_lstBindings.begin(),m_lstBindings.end() );

		BINDLISTItor bi;
		for (bi = m_lstBindings.begin(); bi != m_lstBindings.end(); ++bi)
			binds.push_back((*bi));

		for(std::vector<EntityId>::iterator bndItr=binds.begin();bndItr!=binds.end();++bndItr )
		{
		
			CEntity *pEntity =(CEntity *)m_pEntitySystem->GetEntity((*bndItr));
			
			if (pEntity)
			{
				//OPTIMISED_BY_IVO  
				Matrix44 mat=Matrix34::CreateRotationXYZ( Deg2Rad(m_angles),m_center);
				mat=GetTransposed44(mat);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44

				//pEntity->SetParentLocale(m_center, m_angles);
				pEntity->SetParentLocale(mat);
				
				bool bNeedUpdate = (pEntity->m_bUpdate && !pEntity->m_bSleeping) || (pEntity->m_awakeCounter > 0);
				if (bNeedUpdate)
				{
					pEntity->Update(ctx);
				}
				// If have childs.
				if (pEntity->m_bUpdateBinds)
				{
					// If not updated.
					pEntity->UpdateHierarchy( ctx );
				}
			}
			else
			{
				// this entity was deleted. Lets release its ID so it can be reused
				//	m_pEntitySystem->ReleaseMark((*bi));
				GetISystem()->GetILog()->LogToFile( "UpdateHierarchy <%s> has dead bind id >> %d",GetName(), (*bndItr) );
			}
		}
	}

/*
	if (!m_lstBindings.empty())
	{
		ENTITY_PROFILER

		BINDLISTItor bi,next;
	//[kirill] 
	//	when we update child - it can unbind itself - so m_lstBindings changes
		for (bi = m_lstBindings.begin(); bi != m_lstBindings.end() && !m_lstBindings.empty(); bi = next)
		{
			next = bi; ++next;
			CEntity *pEntity =(CEntity *)m_pEntitySystem->GetEntity((*bi));
			
			if (pEntity)
			{
				//Matrix44 mat;
				//mat.Identity();
				//mat=GetTranslationMat(m_center)*mat;
				//mat=GetRotationZYX44(-gf_DEGTORAD*m_angles)*mat; //NOTE: angles in radians and negated 

				//OPTIMISED_BY_IVO  
				Matrix44 mat=Matrix34::GetRotationXYZ34( Deg2Rad(m_angles),m_center);
				mat=GetTransposed44(mat);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44

				//pEntity->SetParentLocale(m_center, m_angles);
				pEntity->SetParentLocale(mat);
				
				bool bNeedUpdate = (pEntity->m_bUpdate && !pEntity->m_bSleeping) || (pEntity->m_awakeCounter > 0);
				if (bNeedUpdate)
				{
					pEntity->Update(ctx);
				}
				// If have childs.
				if (pEntity->m_bUpdateBinds)
				{
					// If not updated.
					pEntity->UpdateHierarchy( ctx );
				}
			}
			else
			{
				// this entity was deleted. Lets release its ID so it can be reused
				//	m_pEntitySystem->ReleaseMark((*bi));

GetISystem()->GetILog()->LogToFile( "UpdateHierarchy <%s> has dead bind id >> %d",GetName(), (*bi) );

//				m_lstBindings.erase(bi);
				m_lstBindings.remove((*bi));
			}
			
		}
	}
*/
}

//
// gets radius from physics bounding box	
float	CEntity::GetRadiusPhys() const
{
	IPhysicalEntity *physic = GetPhysics();
	if (physic)
	{
		Vec3d	min, max;
		pe_params_bbox pbb;
		physic->GetParams(&pbb);
		min = pbb.BBox[0]-m_center;
		max = pbb.BBox[1]-m_center;
    //pe_status_pos status_pos;
    //m_physic->GetStatus(&status_pos);
		//min = status_pos.BBox[0];
		//max = status_pos.BBox[1];
		if (min.Length() > max.Length())
			return min.Length();
		return max.Length();
	}
	return 0.0f;
}

//! 
//
void CEntity::ActivatePhysics( bool activate )
{
	if(!m_physic)
		return;
	pe_player_dynamics pd;
	pd.bActive = activate;
	m_physic->SetParams(&pd);		// phisics switched
}

void CEntity::CheckBBox(void)
{
	if (m_bRecalcBBox)
	{
		CalcWholeBBox();
		m_bRecalcBBox = false;
	}
}

#define sizeof_std_string(m_sClassName) ((m_sClassName.length()<16)?sizeof(string):m_sClassName.length()+1+sizeof(string))
void CEntity::GetMemoryStatistics(ICrySizer *pSizer)
{
	size_t nSize = sizeof(CEntity)
		+ sizeof_std_string(m_sClassName)
		+ sizeof_std_string(m_name)
		+ sizeof_std_string(m_sStateName);

	if (!pSizer->AddObject(this,nSize))
		return;

	pSizer->Add(*m_pCamera);
	pSizer->Add(*m_pDynLight);
	pSizer->Add(*m_pClientState);
	pSizer->Add(*m_pServerState);
	if (m_pPhysState && m_iPhysStateSize>0)
		pSizer->AddObject(m_pPhysState,m_iPhysStateSize);
}


void CEntity::SetHandsIKTarget( const Vec3d* target )
{
	if(target)
	{
		m_vHandIKTarget = *target;

		m_vHandIKTarget -= m_center;

/*
		Matrix44	mat;
		Matrix44	matInv;
		mat.Identity();
//		mat.SetRotationXYZ44(-gf_DEGTORAD*GetAngles());
//		matInv = mat.GetInverted44( mat );
//		m_vHandIKTarget = matInv.TransformVector(m_vHandIKTarget);
		mat.SetRotationZ44(gf_DEGTORAD*(GetAngles().z));
		m_vHandIKTarget = mat.TransformVector(m_vHandIKTarget);
*/

		Matrix44	mat = Matrix33::CreateRotationZ( DEG2RAD(GetAngles().z) );
		m_vHandIKTarget = mat.TransformVectorOLD(m_vHandIKTarget);




		m_bHandIK = true;
	}
	else
		m_bHandIK = false;
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::IsStateFunctionImplemented( EScriptStateFunctions function )
{
	bool bRes = false;
	if (m_pServerState && m_pServerState->pFunction[function])
		bRes = true;
	else if (m_pClientState && m_pClientState->pFunction[function])
		bRes = true;
	return bRes;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::CallStateFunction( EScriptStateFunctions function )
{
	HSCRIPTFUNCTION funcServer = 0;
	HSCRIPTFUNCTION funcClient = 0;
	if (m_pServerState)
		funcServer = m_pServerState->pFunction[function];
	if (m_pClientState)
		funcClient = m_pClientState->pFunction[function];

	if (funcClient)
	{
		m_pScriptSystem->BeginCall(funcClient);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->EndCall();
	}
	if (funcServer)
	{
		m_pScriptSystem->BeginCall(funcServer);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->EndCall();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::MarkAsGarbage()
{
	m_bGarbage = true;	
	if (m_registeredInSector)
		UnregisterInSector();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetGarbageFlag( bool bGarbage )
{
	m_bGarbage = bGarbage;
	if (m_bGarbage && m_registeredInSector)
		UnregisterInSector();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::AddCollider( EntityId id )
{
	if (!m_bTrackColliders)
		return;

	if (!m_pColliders)
		m_pColliders = new Colliders;

	// Try to insert new colliding entity to our colliders set.
	std::pair<Colliders::iterator,bool> result = m_pColliders->insert(id);
	if (result.second == true)
	{
		// New collider was successfully added.
		IEntity *pEntity = m_pEntitySystem->GetEntity(id);
		if (pEntity)
		{
			//GetISystem()->GetILog()->Log( "Add Collider %s",pEntity->GetName() );
			OnEnterArea( pEntity,0 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::RemoveCollider( EntityId id )
{
	if (!m_pColliders)
		return;

	// Try to remove collider from set, then compare size of set before and after erase.
	int prevSize = m_pColliders->size();
	m_pColliders->erase(id);
	if (m_pColliders->size() != prevSize)
	{
		// If anything was erased.
		IEntity *pEntity = m_pEntitySystem->GetEntity(id);
		if (pEntity)
		{
			//GetISystem()->GetILog()->Log( "Remove Collider %s",pEntity->GetName() );
			OnLeaveArea( pEntity,0 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::CheckColliders()
{
	ENTITY_PROFILER

	if (!m_bTrackColliders)
		return;

	Vec3d mins, maxs;
	GetBBox(mins,maxs);
	if (maxs.x<=mins.x || maxs.y<=mins.y || maxs.z<=mins.z)
		return; // something wrong

	// resolve collisions
	IPhysicalWorld *pWorld = m_pISystem->GetIPhysicalWorld();
	IPhysicalEntity **ppColliders;
	int nCount = 0;

	// Prepare temporary set of colliders.
	// Entities which will be reported as colliders will be erased from this set.
	// So that all the entities which are left in the set are not colliders anymore.
	Colliders tempSet;
	if (m_pColliders)
		tempSet = *m_pColliders;

	if (nCount = pWorld->GetEntitiesInBox(mins,maxs, ppColliders, 14 ))
	{
		static std::vector<IPhysicalEntity*> s_colliders;
		s_colliders.resize(nCount);
		memcpy( &s_colliders[0],ppColliders,nCount*sizeof(IPhysicalEntity*) );

		// Check if colliding entities are in list.
		for (int i = 0; i < nCount; i++)
		{
			CEntity *pEntity = (CEntity *)s_colliders[i]->GetForeignData(0);
			if (!pEntity)
				continue;
			int id = pEntity->GetId();
			int prevSize = tempSet.size();
			tempSet.erase( id );
			if (tempSet.size() == prevSize)
			{
				// pEntity is a new entity not in list.
				AddCollider( id );
			}
		}
	}
	// If any entity left in this set, then they are not colliders anymore.
	if (!tempSet.empty())
	{
		for (Colliders::iterator it = tempSet.begin(); it != tempSet.end(); ++it)
		{
			RemoveCollider( *it );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnPhysicsBBoxOverlap(IEntity *pOther)
{
//#ifdef USE_PHYSICS_EVENT_CALLBACK
	if (!m_bTrackColliders)
		return;

	// Awake entity for at least 5 frames.
	m_awakeCounter = 5;
//#endif
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnPhysicsStateChange( int nNewSymClass,int nPrevSymClass )
{
	// If its update depends on physics, physics state defines if this entity is to be updated.
	if (m_eUpdateVisLevel == eUT_Physics || m_eUpdateVisLevel == eUT_PhysicsVisible)
	{
		// Awake entity for few frames.
		m_awakeCounter = 4;
		if (nNewSymClass == SC_ACTIVE_RIGID)
		{
			//m_pISystem->GetILog()->Log("Phys AWAKE" );
			SetNeedUpdate( true );
		}
		else if (nNewSymClass == SC_SLEEPING_RIGID)
		{
			// Entity must go to sleep.
			//m_pISystem->GetILog()->Log("Phys SLEEP" );
			SetNeedUpdate( false );
			CallStateFunction(ScriptState_OnStopRollSlideContact, "roll");
			CallStateFunction(ScriptState_OnStopRollSlideContact, "slide");
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// creates the physical bbox for callback
void CEntity::CreatePhysicsBBox()
{
	//#ifdef USE_PHYSICS_EVENT_CALLBACK

	//m_vBoxMin = mins;
	//m_vBoxMax = maxs;
	if (!m_bTrackColliders)
		return;

	if (!m_pBBox)
	{	
		//////////////////////////////////////////////////////////////////////////
		// Create a physics bbox to get the callback from physics when a move event happens.
		m_pBBox = m_pISystem->GetIPhysicalWorld()->CreatePhysicalEntity(PE_STATIC);
		pe_params_pos parSim;
		parSim.iSimClass = 6;
		m_pBBox->SetParams(&parSim);
		pe_params_foreign_data  foreignData;
		m_pBBox->GetParams(&foreignData);
		foreignData.pForeignData = (IEntity*)this;
		foreignData.iForeignFlags |= ent_rigid | ent_living;
		m_pBBox->SetParams(&foreignData);		
	}

	if (m_pBBox)
	{
		Vec3 min,max;
		GetBBox(min,max);

		// Check for invalid bounding box.
		if (GetLengthSquared(max-min) < 10000*10000)
		{
			pe_params_bbox parBBox;
			parBBox.BBox[0] = min;
			parBBox.BBox[1] = max;
			m_pBBox->SetParams(&parBBox);
		}
	}
	//#endif
}

//////////////////////////////////////////////////////////////////////////
void CEntity::TrackColliders( bool bEnable )
{
	if (bEnable && bEnable != m_bTrackColliders)
	{
		m_bTrackColliders = bEnable;
		CreatePhysicsBBox();
		m_awakeCounter = 5; // Awake entity for few updates.
	}
	m_bTrackColliders = bEnable;
}


void	CEntity::Remove( )
{
	m_pEntitySystem->RemoveEntity( GetId(), false );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetPhysicsState( const char *sPhysicsState )
{
	if (m_physic)
	{
		m_physic->SetStateFromSnapshotTxt( sPhysicsState,strlen(sPhysicsState) );
		// Update entity few times to get physics data to character.
		m_awakeCounter = 5;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::InvalidateBBox()
{
	m_awakeCounter = 1;
	m_bRecalcBBox = true;
}


//////////////////////////////////////////////////////////////////////////
void CEntity::SetUpdateVisLevel(EEntityUpdateVisLevel nUpdateVisLevel) 
{ 
	m_eUpdateVisLevel = nUpdateVisLevel; 
	if (m_physic)
	{
		pe_params_flags pf;
		if (m_eUpdateVisLevel==eUT_PhysicsPostStep)
			pf.flagsOR = pef_custom_poststep;
		else
			pf.flagsAND = ~pef_custom_poststep;
		m_physic->SetParams(&pf);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnVisibilityChange( bool bVisible )
{
	if (!bVisible)
	{
		// Turn off cloth update.
		if (m_physic && (m_flags & ETY_FLAG_CALC_PHYSICS) && !(m_flags&ETY_FLAG_IGNORE_PHYSICS_UPDATE) && m_physic->GetType()==PE_SOFT)
		{
			pe_action_awake aa;
			aa.bAwake = false;
			m_physic->Action(&aa);
		}
	}
}