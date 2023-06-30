//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XEntityPlayer.cpp
//  Description: Entity player class.
//
//  History:
//  - August 16, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "XPlayer.h"
#include "WeaponClass.h"
#include "XVehicle.h"
#include "WeaponSystemEx.h"
#include "ScriptObjectStream.h"

#include <IEntitySystem.h>
#include <IAISystem.h>
#include <IAgent.h>

// remove this
//#include <IAISystem.h>
#include <ISound.h>
// <<FIXME>> look above
#include "I3DEngine.h"
#include <crycharanimationparams.h>


// to use _isnan()
#include <float.h>

//! Minimal time before player can be alive again.
#define PLAYER_RESPAWN_TIME 1.0f
//! Minimal time before player can be respawned.
#define PLAYER_DEATH_TIME 1.0f

//! Minimal time before player can change weapon.
#define PLAYER_WEAPON_CHANGE_TIME 0.1f

/*
static int64 g_nValidateHeapCounter = 0;
// central place to turn on/off the heap validation
inline void ValidateHeap()
{
#if defined(WIN64) && defined(_DEBUG) // on AMD64, heap validation is extremely slow
	if (g_nValidateHeapCounter > 1000*1000 || !((++g_nValidateHeapCounter)&0xFF))
#endif
		assert(IsHeapValid());
}
*/

//-------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------

bool CheckIfNAN( const Vec3d& vPos )
{
	if (_isnan(vPos.x) || _isnan(vPos.y) || _isnan(vPos.z))
	{
		GameWarning( "NotANumber tried to be set for position of CPlayer" );
		return  true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//! CPlayer implemenation.

///////////////////////////////////////////////
CPlayer::CPlayer(CXGame *pGame) :
	m_ssoHitPosVec(pGame->GetScriptSystem()),
	m_ssoHitDirVec(pGame->GetScriptSystem()),
	m_ssoHitNormVec(pGame->GetScriptSystem()),
	m_vColor(1,0,1),
	m_vShake(0,0,0),
	m_pDynLight( NULL ),
	m_vCharacterAngles( 0,0,0 ),
	m_JumpStage(0),
	m_JumpAniLenght(0),
	m_bInertiaOverride(false),
	m_bIsFish(false),
	m_AutoCenter(0),
	m_fNoChangeangleTime(0.0f),
	m_fProcessTime(0.0f),
	m_fCameraTime(0.0f),
	m_vEyeAnglesBaseOffset(0,0,0),
	m_bLastDeltaEyeVehicle( false ),
	m_vSafeAngAtMountedWeapon(0,0,0),
	m_MntWeaponStandTime(0.0f),
	m_vProneEnvNormal(0,0,0),
	m_vCurEntAngle(0,0,0),
	m_bGrenadeAnimation(false)
{
	m_pLastUsedCharacter = NULL;
	m_weaponPositionState = WEAPON_POS_UNDEFINED;
	m_vEyePos.Set(0,0,0);
	m_bWeaponJustFired = false;
	m_pLightTarget = 0;
	float tm=.3f;	

	m_pBoneHead = NULL;
	m_pBoneNeck = NULL;
	m_pBoneSpine = NULL;
	m_pBoneSpine1 = NULL;
	m_pBoneSpine2 = NULL;

	m_pGame = pGame;
	m_pTimer = pGame->GetSystem()->GetITimer();
	m_pScriptSystem = pGame->GetScriptSystem();
	m_p3DEngine = pGame->GetSystem()->GetI3DEngine();
	m_pPhysicalWorld=pGame->GetSystem()->GetIPhysicalWorld();
	m_stmScriptStream.Create(m_pScriptSystem);
	m_nSelectedWeaponID = -1;
	m_PrevWeaponID = -1;

	ZeroStruct( m_stats );
	m_stats.weapon = -1;
	m_stats.inVehicleState = PVS_OUT;
//		m_stats.inVehicleActive = false;

	ZeroStruct( m_walkParams );
	// Init Walk params.
	m_walkParams.runRoll = 0.2f;
	m_walkParams.runPitch = 0.5f;
	m_walkParams.curLength = 0.0f;
	m_walkParams.stepLength = 3.5f;
	m_walkParams.flyCoeff = 15.0f;
	m_walkParams.prevF = 0.0f;
	m_walkParams.weaponCycle = 0.006f;
	m_walkParams.weaponCycleSpeed = 8;
	m_walkParams.swayAmp=0.0f;
	m_walkParams.swayFreq=0.0f;
	m_walkParams.swayOfs=0.0f;
	m_walkParams.shakeDegree=0.0f;
	m_walkParams.leanAmount = 0.0f;
	m_walkParams.leanStart = 0.0f;
	m_walkParams.leanEnd = 0.0f;
	m_walkParams.leanFactor = 0.0f;
	m_walkParams.fWaterPitchPhase=0.0f;
	m_walkParams.fWaterRollPhase=0.0f;

	m_walkParams.fLeanTarget=0;
	m_walkParams.fCurrLean=0;

	m_pVehicle = NULL;

	//@FIXME Give player some life
	m_stats.health = 100;
	m_stats.armor = 0;
	m_stats.maxHealth = 100;
	m_stats.maxArmor = 100;

	m_stats.legHealth = 100;
	m_stats.maxLegHealth = 100;
	m_stats.armHealth = 100;
	m_stats.maxArmHealth = 100;

	m_stats.dmgFireAccuracy = 100;
	m_stats.dmgSpeedCoeff = 100;
	

	m_stats.FiringType = m_stats.LastFiringType = eNotFiring;
	m_stats.cancelFireFlag = false;

	m_stats.weapon = 0;
	m_stats.score=0;
	m_stats.deaths=0;
	m_stats.firing_grenade = false;
	m_stats.firemode = 0;
	m_stats.canfire = true;
	m_stats.crosshairOnScreen = true;
	m_stats.fSpeedScale = 1.0f;
	m_stats.climbing = false;
	m_stats.holding_breath=false;
	m_stats.concentration=false;
	m_stats.firing=false;
	m_stats.reloading=false;
	m_stats.weapon_busy=0;
	m_stats.grenadetype = 1;
	m_stats.numofgrenades = 0;
	m_stats.drawfpweapon=true;
	m_stats.aiming=false;
	m_stats.fire_rate_busy=false;
	m_stats.fVel=1.0f;	
	m_stats.melee_distance = 0.0f;
	m_stats.has_flashlight = false;
	m_stats.has_binoculars = false;
	m_stats.fInWater = 0.0f;
	m_stats.swimTime = 0.0f;
	m_staminaClient = m_stats.stamina = 100;
//		m_breathClient = m_stats.breath = 100;

	SetDimNormal();		
	SetDimCrouch();
	SetDimProne();
	SetDimStealth();

	m_currDimensions = eDimNone;

	ZeroStruct( m_AngleLimitBase );
	m_AngleLimitVFlag = false;
	m_AngleLimitHFlag = false;

	m_fGravityOverride=1E10;

	m_pScriptObject=NULL;
	m_pRedirected = NULL;

	m_nStanding = -1;
	m_nMode = -1;
	m_nStrafe = -1;
	m_nForward = -1;

	m_bFirstPerson = false;
	m_bFirstPersonLoaded = true;

	m_bLightOn = false;

	m_LegAngle = 0;
	m_LegAngleDesired = 0;
	m_LegDeltaAngle = 0;
	m_LegAngleIdleTime = 0;
	m_LegADeltaLimit = 110;
	m_LegADeltaLimitForce = 120;
	m_LegAIdleTimeLimit = 2;
	m_LegAngleVelMoving = 350;
	m_LegAngleVelTimed = 320;
	m_LegAngleVel = m_LegAngleVelMoving;


	m_FlyTime = 0;
	m_NotRunTime = 0;
	
	m_RunSpeed = 4;
	m_WalkSpeed = 3;
	m_CrouchSpeed = 2;
	m_ProneSpeed = 1;
	m_SwimSpeed	= 3.5f;
	m_Sprinting = false;

	m_CameraMode=PCM_OUTVEHICLE;
	m_bCameraTransitionOnlyZ = false;

//		SetAnimationRefSpeed();
	SetAnimationRefSpeedRun( );
	SetAnimationRefSpeedWalk( );
	SetAnimationRefSpeedXWalk( );
	SetAnimationRefSpeedCrouch( );


	m_JumpForce = 4.0f;
	m_LeanDegree = 9.0f;

	m_Dynamics.gravity = 9.81f;
	m_Dynamics.swimming_gravity = 2.0f;
	m_Dynamics.inertia = 10.0f;
	m_Dynamics.swimming_inertia = 1.0f;
	m_Dynamics.air_control = 0.1f;

	m_CurStance = eNone;
	m_PrevStance = m_CurStance;

	m_aimLook = true;
	m_bIsAI = false;

	//m_PrevAniName = "NoAni";

	m_AnimationSystemEnabled = 1;

	m_bAlwaysRun = true;
	m_bSwimming=false;
	m_bEyesInWater=false; // not in water at the beginning
	m_stats.underwater = 0.0f;
	// standing jump
	m_JumpDist[0] = 3.0f;
	m_JumpHeight[0] = 1.3f;
	// walk jump
	m_JumpDist[1] = 3.0f;
	m_JumpHeight[1] = 1.3f;
	// run jump
	m_JumpDist[2] = 4.0f;
	m_JumpHeight[2] = 1.3f;
	m_fRecoilXDelta=0;
	m_fRecoilZDelta=0;
	m_fRecoilXUp=0;
	m_fRecoilZUp=0;
	m_fRecoilX=0;
	m_fRecoilZ=0;
	m_fAccuracy=0;
	m_fAccuracyMod=0;
	memset(m_vWeaponSlots,0,sizeof(m_vWeaponSlots));
	m_stats.lock_weapon=false;

	m_fHeatBodyFadingSpeed=m_fHeatWeaponsFadingSpeed=-1; // switch immediately
	m_fHeatBodyDesiredValue=m_fHeatWeaponsDesiredValue=-1; // not set
	m_fHeatBodyCurrValue=m_fHeatWeaponsCurrValue=0;

	m_LastUsed = m_vBlindingList.end();

	m_RunSpeedScale			= 3.63f;
	m_CrouchSpeedScale	= .8f;
	m_ProneSpeedScale		= .5f;
	m_XRunSpeedScale		= 1.5f;		// stealthth
	m_XWalkSpeedScale		= .81f;	
	m_RRunSpeedScale		= 3.63f;		// relaxed
	m_RWalkSpeedScale		= .81f;

	m_stats.bIsBlinded = false;
	m_stats.curBlindingValue = 0.0f;

	m_pMountedWeapon = NULL;

	m_stats.bIsLimping = false;

	m_pPrevDmgCollider = 0;
	m_timeDmgCollision = 0;
	m_prevCollDamage = 0;

	m_sPrevAniName = "";
	m_sPrevAniNameLayer1 = "";
	m_sPrevAniNameLayer2 = "";
	m_fShootAniLength = 0.0f;

	SetBlendTime("aidle_jump_air", .2f);

	m_fDeathTimer = -1;
	m_stats.onLadder = false;
	m_RunningTime = m_RunningTimeClient = 0;
	m_WalkingTime = m_WalkingTimeClient = 0;
	m_NotRunTime = m_NotRunTimeClient = 0;

	m_JTWalk = .2f;
	m_JTRun = .4f;
	m_fLastCamUpdateTime = 0;

	m_vDeltaCamAngles.Set(0,0,0);
	m_vDeltaCamPos.Set(0,0,0);
	m_vPrevCamAngles.Set(0,0,0);
	m_vPrevCamPos.Set(0,0,0);
	m_vWeaponAngles.Set(0,0,0);

	m_AniSpeedXRun[0] = -1.0f;
	m_AniSpeedXRun[1] = -1.0f;
	m_AniSpeedXRun[2] = -1.0f;

	m_sVehicleEyeHelper.clear();
	m_bWaitForFireRelease = false;

	m_vLastMotionDir.Set(0,0,0);
	
	m_input_accel = 0;//default is no input acceleration.
	m_input_stop_accel = 0;

	m_input_accel_indoor = 0;//default is no input acceleration.
	m_input_stop_accel_indoor = 0;

	m_bWriteOccured = false;

	m_bStayCrouch = false;

	m_SynchedRandomSeed.SetParent(this);

	m_vLadderPosition.Set(0,0,0);
	m_vLadderAngles.Set(0,0,0);
	m_vHeadAngles.Set(0,0,0);

	m_AreaUser.SetGame( pGame );
	m_AreaUser.SetEntity( GetEntity() );

	m_fLastProneTime = 0;

	m_pLastAiming=NULL;
}


///////////////////////////////////////////////
CPlayer::~CPlayer()
{
	SwitchFlashLight( false );

	ListOfPlayers::iterator	self = std::find(m_pGame->m_DeadPlayers.begin(), m_pGame->m_DeadPlayers.end(), this);
	if(self!=m_pGame->m_DeadPlayers.end())
		m_pGame->m_DeadPlayers.erase(self);

	m_pGame->m_XAreaMgr.ExitAllAreas( m_AreaUser );
	// remove shared weapon
	GetEntity()->GetCharInterface()->SetCharacter(1, 0);
//	if(m_pSelectedWeapon){
//		m_pSelectedWeapon->GetEntity()->DrawCharacter(0,0);
//	}
	SetEntity(0);

	if(m_pScriptObject)
		m_pScriptObject->Release();
	m_pScriptObject=NULL;
	
	
	m_mapPlayerWeapons.clear();

	if (m_pDynLight)
	{
		delete m_pDynLight;
		m_pDynLight=NULL;
	}
}

///////////////////////////////////////////////
/*! Initializes the player-container.
		@return true if succeeded, false otherwise
*/
bool CPlayer::Init()
{
	//@FIXME probably should be created in script.
	m_pGame->m_pLog->Log("XEntityPlayer %d initialised\n", m_pEntity->GetId() );

	m_pEntity->GetScriptObject()->SetValue("type", "Player");

	IAIObject *pObject = m_pEntity->GetAI();
	if (pObject)
	{
		IPuppet *pPuppet;
		if (pObject->CanBeConvertedTo(AIOBJECT_PUPPET, (void **) &pPuppet))
			m_bIsAI = true;
	}

	if (!m_bIsAI)
//	if(m_pEntity->GetAI()->GetType() == AIOBJECT_PLAYER)
		m_pEntity->SetNeedUpdate(true);
//	else
//		m_pEntity->SetDestroyable( true );

/*
//	if (!m_bIsAI)
	if(m_pEntity->GetAI()->GetType() == AIOBJECT_PLAYER)
	{
		m_pEntity->SetNeedUpdate(true);

		m_pGame->p_speed_run->Set(m_RunSpeed);
		m_pGame->p_speed_walk->Set(m_WalkSpeed);
		m_pGame->p_speed_crouch->Set(m_CrouchSpeed);
		m_pGame->p_speed_prone->Set(m_ProneSpeed);
	}
*/
	// create the camera
	// <<FIXME>> create only for client which do the rendering

	ICryCharInstance* pCharacter = m_pEntity->GetCharInterface()->GetCharacter(0);
	if (pCharacter)
	{
		pCharacter->EnableLastIdleAnimationRestart(0,true);
		if (m_bIsAI)
			pCharacter->EnableLastIdleAnimationRestart(3,true);
	}

	//////////////////////////////////////////////////////////////////////////
	// Initialize Player Camera Fov.
	//////////////////////////////////////////////////////////////////////////
	IEntityCamera *pEntCamera = m_pGame->GetSystem()->GetIEntitySystem()->CreateEntityCamera();
	m_pEntity->SetCamera(pEntCamera);
	int rw = m_pGame->GetSystem()->GetIRenderer()->GetWidth();
	int rh = m_pGame->GetSystem()->GetIRenderer()->GetHeight();
	float fFOV = DEG2RAD(90.0f); // Initialize with 90 degree fov.
	pEntCamera->SetFov(fFOV,rw,rh);
	pEntCamera->GetCamera().Init(rw,rh,fFOV);
	pEntCamera->GetCamera().Update();
	//////////////////////////////////////////////////////////////////////////

	Vec3d default_offset;
	default_offset(0,m_pGame->cl_ThirdPersonRange->GetFVal(),4);
	m_pEntity->GetCamera()->SetCameraOffset(default_offset);

	// [kirill] need this to use only z component of entity angles (same as for drawing)
	// when calculating BBox
	GetEntity()->SetFlags( ETY_FLAG_CALCBBOX_ZROTATE );

	GoStand();

	return true;
}

/*!Called by the entity when the angles are changed.
This notification is used to force the orientation of the player.
*/
void CPlayer::OnSetAngles( const Vec3d &ang )
{
  // say to Vladimir if you want to comment this code again
//	if(m_pGame->m_pClient && !m_pVehicle)
	//[kirill] need it in cars for autocentering
	if(m_pGame->m_pClient)
	{
//		if(m_pGame->m_pClient->GetPlayerId()==m_pEntity->GetId() || m_pGame->m_pClient->m_bLocalHost)
		if(m_pGame->m_pClient->GetPlayerId()==m_pEntity->GetId())
			m_pGame->m_pClient->m_PlayerProcessingCmd.SetDeltaAngles(ang);
	}
}

// set a certain shader for the current selected weapon
//////////////////////////////////////////////////////////////////////////
void CPlayer::SetHeatVisionValues(int dwFlags,const char *szName,float fValue,float fFadingValue)
{	
	if (dwFlags & BITMASK_WEAPON)
	{
		m_fHeatWeaponsDesiredValue=fValue;
		m_fHeatWeaponsFadingSpeed=fFadingValue;
	}

	if (dwFlags & BITMASK_PLAYER)
	{
		m_fHeatBodyDesiredValue=fValue;
		m_fHeatBodyFadingSpeed=fFadingValue;		
	}
}

/*! Initializes the weapon-information stored in the player-container.
*/
void CPlayer::InitWeapons()
{
//	ValidateHeap();
	IEntitySystem *pEntitySystem = (IEntitySystem *) m_pGame->GetSystem()->GetIEntitySystem();

	SetWeapon(-1);
	m_mapPlayerWeapons.clear();

	for (int i=0;i<PLAYER_MAX_WEAPONS;i++)
		m_vWeaponSlots[i]=0;

	// init weapon instances
	unsigned int weaponCount = GetGame()->GetWeaponSystemEx()->GetNumWeaponClasses();

	for (unsigned int i = 0; i < weaponCount; ++i)
	{
		CWeaponClass *pWC = GetGame()->GetWeaponSystemEx()->GetWeaponClass(i);
		assert(pWC);
		WeaponInfo wi;
		wi.fireTime = m_pTimer->GetCurrTime();
		wi.fireLastShot = 0;
		wi.reloading = false;
		wi.iFireMode = 0;

		assert(m_mapPlayerWeapons.count(pWC->GetID())==0);		// otherwise we produce memory leaks

		wi.owns = false;
		m_mapPlayerWeapons[pWC->GetID()] = wi;
	}
}

void CPlayer::SelectFirstWeapon()
{
	if (!m_pGame->IsServer())
		return;

	int slot=0;
	while(slot<PLAYER_MAX_WEAPONS && m_vWeaponSlots[slot]==0)
		slot++;

	if(m_vWeaponSlots[slot]!=0)
		SelectWeapon(m_vWeaponSlots[slot]);
}

///////////////////////////////////////////////
/*! Updates the player torso/head angles. Should be called every frame. CAlled from Update()
*/
//////////////////////////////////////////////////////////////////////////////
//UPDATE HEAD ROTATION
void CPlayer::UpdateRotateHead()
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	m_LegAngleVelMoving = m_pGame->pa_leg_velmoving->GetFVal();
	m_LegAngleVelTimed = m_pGame->pa_leg_velidle->GetFVal();
	m_LegAIdleTimeLimit = m_pGame->pa_leg_idletime->GetFVal();

	if( !m_bRotateHead )
	{
		if( m_pEntity->GetCurrentAnimation(0, 3)>=0
			|| m_pEntity->IsBound()
			|| m_stats.onLadder 							// no rotatehead on ladders
//			|| m_stats.landing || m_stats.flying			// no rotatehead while jumping
			)
			m_LegNeedsForceAngle = true;

		m_LegRotation = false;
		return;
	}

	Vec3d angles = GetActualAngles();
	if(m_LegNeedsForceAngle || m_pMountedWeapon)	// some animation was played - need to reset all rotations
	{
		m_LegAngle = angles.z;
		m_LegAngleDesired = m_LegAngle;
		m_LegDeltaAngle = 0.0f;
		m_LegNeedsForceAngle = false;
	}
	m_LegRotation = true;

	float	legDeltaAngle = m_LegAngleVel*m_pTimer->GetFrameTime();
	float	legDiff = Ffabs(Snap_s180(m_LegAngleDesired - m_LegAngle));
	if(legDiff<legDeltaAngle)
		m_LegAngle = m_LegAngleDesired;
	else
	{
		if(legDiff>160)
		{
			if(Ffabs(Snap_s180(m_LegAngle+legDeltaAngle-angles.z))<Ffabs(Snap_s180(m_LegAngle-legDeltaAngle-angles.z)))
				m_LegAngle += legDeltaAngle;
			else
				m_LegAngle -= legDeltaAngle;
		}
		else
			m_LegAngle += (Snap_s180(m_LegAngleDesired-m_LegAngle))>0?m_LegAngleVel*m_pTimer->GetFrameTime():-m_LegAngleVel*m_pTimer->GetFrameTime();
		m_LegAngle = Snap_s180(m_LegAngle);
	}

	if( m_stats.moving )
	{
		m_LegDeltaAngle = Snap_s180(m_LegAngle - angles.z);

/*
		if(Ffabs(m_LegDeltaAngle) > m_LegADeltaLimitForce)
		{
			if(m_LegDeltaAngle<0)
			{
				m_LegAngleDesired = Snap180(angles.z-m_LegADeltaLimit);
				m_LegAngle = m_LegAngleDesired;
			}
			else
			{
				m_LegAngleDesired = Snap180(angles.z+m_LegADeltaLimit);
				m_LegAngle = m_LegAngleDesired;	
			}
		}
*/
		return;
	}

	if( m_bRotateHead && IsAlive())
	{
		float prevDelta = m_LegDeltaAngle;

		m_LegDeltaAngle = Snap_s180(m_LegAngle - angles.z);

		if(Ffabs(m_LegDeltaAngle) > m_LegADeltaLimit)
		{
			if(m_LegDeltaAngle<0)
			{
				m_LegAngleDesired = Snap_s180(angles.z-m_LegADeltaLimit);
			}
			else
			{
				m_LegAngleDesired = Snap_s180(angles.z+m_LegADeltaLimit);
			}
			m_LegAngleVel = m_LegAngleVelMoving;
			if(Ffabs(m_LegDeltaAngle) > m_LegADeltaLimit*2 && !m_bIsAI)		// to prevent player's from twisting legs/upper_body
				m_LegAngleVel *= 100;
		}

		if(Ffabs(m_LegDeltaAngle) > m_LegADeltaLimit-5 )	
		{
			m_LegAngleIdleTime += m_pTimer->GetFrameTime();
			if( m_LegAngleIdleTime>m_LegAIdleTimeLimit )
			{
				m_LegAngleDesired = angles.z;
				m_LegAngleVel = m_LegAngleVelTimed;
			}
		}
		else
			m_LegAngleIdleTime = 0;

/*
		// if less than 5 degrees - don't autorotate.
		if(Ffabs(prevDelta)< 5 )
			prevDelta = 0;
		if(m_LegDeltaAngle!=0 && Ffabs(m_LegDeltaAngle - prevDelta)<1.0f)	
		{
			m_LegAngleIdleTime += m_pTimer->GetFrameTime();
			if( m_LegAngleIdleTime>m_LegAIdleTimeLimit )
			{
				m_LegAngleDesired = angles.z;
				m_LegAngleVel = m_LegAngleVelTimed;
			}
		}
		else
			m_LegAngleIdleTime = 0;
*/
	}
	else
	{
		m_LegAngle = angles.z;
		m_LegAngleDesired = m_LegAngle;
		m_LegDeltaAngle = 0.0f;
	}
}

///////////////////////////////////////////////
/*! Updates the player-container. Should be called every frame.
*/
void CPlayer::UpdateDead( SPlayerUpdateContext &ctx )
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	if (m_bIsAI && m_fDeathTimer>=0.0f )
	{
		m_fDeathTimer -= m_pTimer->GetFrameTime();
		if(m_fDeathTimer <= 0.0f)
		{
			m_fDeathTimer = 0.0f;
			int nRendererFrameID = m_pGame->GetSystem()->GetIRenderer()->GetFrameID();
			if( nRendererFrameID - m_pEntity->GetDrawFrame() > 60*2 )
			{
				m_pEntity->Remove();
				return;
			}
			// restart the timer
			m_fDeathTimer=m_pGame->p_deathtime->GetFVal();
		}
	}

	// don't check/update areas for AI's
	if( !m_bIsAI )
	{
//		m_pGame->m_XAreaMgr.UpdatePlayer( this );
	//////////////////////////////////////////////////////////////////////////////
	//UPDATE THE CAMERA
		SetEyePosDead();
		UpdateCamera();
	}

//		UpdateDrawAngles( );

	if (m_bFirstPerson)
	{
		m_pEntity->SetRegisterInSectors(false);
		UpdateFirstPersonView();
	}
	else
	{
		m_pEntity->SetRegisterInSectors(true);
		UpdateThirdPersonView();
	}
}

//////////////////////////////////////////////////////////////////////////
void CPlayer::AutoAiming()
{
	const char *pszBoneName="Bip01 Spine1\0";
	//const char *pszBoneName=NULL;

	Vec3	Center;
	Vec3	bestPoint3D;
	float fBestDist=99999;
	IEntity *pBest=NULL;

	IEntityItPtr It=m_pGame->GetSystem()->GetIEntitySystem()->GetEntityInFrustrumIterator( true );
	CCamera Cam=m_pGame->GetSystem()->GetViewCamera();
	IEntity *pEnt;
	//ray_hit RayHit;
	IEntity *pLocal=m_pGame->GetMyPlayer();

	Vec3 m_vWpnPos=pLocal->GetPos();

	while (pEnt=It->Next())
	{
		if (pEnt==m_pEntity)
			continue;
		if (!pEnt->IsTrackable())
			continue;

		CPlayer *pPlayer;
		if(	pEnt->GetContainer() && 
			pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer) &&
			!pPlayer->IsAlive() )
			continue;

		IPhysicalEntity *pPE=pEnt->GetPhysics();
		if (!pPE)
			continue;

		if (pszBoneName)	// if we want a bone instead of bbox-center lets do so...
		{
			IEntityCharacter *pIChar=pEnt->GetCharInterface();
			if (pIChar)
			{
				ICryCharInstance *cmodel=pIChar->GetCharacter(0);    
				if (cmodel)
				{
					ICryBone *pBone = cmodel->GetBoneByName(pszBoneName);
					if (pBone)
					{
						Center=pBone->GetBonePosition();

						Matrix44 m;
						m.SetIdentity();
						m=GetTranslationMat(pEnt->GetPos())*m;
						m=Matrix44::CreateRotationZYX(-pEnt->GetAngles()*gf_DEGTORAD)*m; //NOTE: angles in radians and negated 
						Center=m.TransformPointOLD(Center);
					}
				} 
			} 
			else
			{
				Center=pEnt->GetPos();
			}
		}

		Vec3 diff(Center-m_vWpnPos);

		float	length2=GetLengthSquared(diff);
		if(length2>15*15 || length2<1.5*1.5)
			continue;

		// pick the closest
		float fDist=(float)sqrt(length2);
		if (fDist<fBestDist)
		{
			pBest=pEnt;
			fBestDist=fDist;
			bestPoint3D=Center;
		}
	}

	float fCurrTime=m_pGame->GetSystem()->GetITimer()->GetCurrTime();

	//ICVar *pCvar=m_pGame->GetSystem()->GetIConsole()->GetCVar("fixed_time_step");	

	if (!pBest)
	{
		m_pLastAiming=NULL;		
		return;
	}

	if (m_pLastAiming)
	{
		CPlayer *pPlayer;
		if(m_pLastAiming->GetContainer() && 
			m_pLastAiming->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer) &&
			!pPlayer->IsAlive() )
			m_pLastAiming=NULL;		
	}
	
	if (m_pLastAiming && m_pLastAiming!=pBest)
	{				
		if (fCurrTime-m_fLastTime<2.0f)
			return;

		m_fLastTime=fCurrTime;
	}
	else
		m_fLastTime=m_pGame->GetSystem()->GetITimer()->GetCurrTime();

	m_pLastAiming=pBest;

	bestPoint3D.z-=1.5f;
	Vec3 diff(bestPoint3D-m_vWpnPos);
	Vec3 vCurrAng=pLocal->GetAngles();
	Vec3 vNewAng=ConvertVectorToCameraAngles(diff);
	//vCurrAng.z+=1.0f;

	pLocal->SetAngles(vNewAng);
}

///////////////////////////////////////////////
/*! Updates the player-container. Should be called every frame.
*/
void CPlayer::Update()
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	bool bMyPlayer = IsMyPlayer();
	bool bPlayerVisible = bMyPlayer || IsVisible();

	SPlayerUpdateContext ctx;
	ctx.bPlayerVisible = bPlayerVisible;

	if (bMyPlayer)
	{
		// force physics proxy being updated in first person view, if somebody has a better idea
		// I'm open to suggestions
		m_pEntity->SetNeedUpdate(true);
	}

	if( !IsAlive() )
	{
		UpdateDead(ctx);
		// [marco] even after the player dies, he can slide and fall into water - 
		// this call will create the splash sounds
		UpdateSwimState(false);
		return;
	}
	
	// fixme - remove this after artists done with animation params tweaking
	if(m_pGame->pa_forcerelax->GetIVal())
	{
		GoRelaxed();
		m_pEntity->StartAnimation(0, NULL, 1, .15f);	// to stop all the fire animations
	}
	// fixme end

	if(m_pGame->p_limp->GetIVal() == 0)			// no limping
		m_stats.bIsLimping = false;
	else if(m_pGame->p_limp->GetIVal() == 1)	// limp if wounded
	{
		if(m_stats.health<50&&(!m_stats.bIsLimping))
			m_stats.bIsLimping = true;
	}
	else if(m_pGame->p_limp->GetIVal() == 2)	// limp always
		m_stats.bIsLimping = true;

	if(bMyPlayer &&  m_pGame->m_bHideLocalPlayer)
	{
		FRAME_PROFILER( "CPlayerUpdate::UpdateCamera",GetISystem(),PROFILE_GAME );

		m_AreaUser.SetEntity( GetEntity());
		m_pGame->m_XAreaMgr.UpdatePlayer( m_AreaUser );
		UpdateCamera();
		///m_pEntity->DrawCharacter(0, ETY_DRAW_NONE);
		//m_bFirstPerson = false;
		//UpdateThirdPersonView();
		return;
	}

	//if (bMyPlayer)
	//	AutoAiming();

	m_bRotateHead = m_pEntity->GetCurrentAnimation(0, 3)<0 && m_pGame->p_RotateHead->GetIVal()!=0 &&
					IsAlive() && !m_pEntity->IsBound() 
					&& !m_stats.onLadder 							// no rotatehead on ladders
					&& !m_bIsFish									// it's not a fish
//					&& m_pEntity->WasVisible()   //Removed to fix BUG6238 (you always hear Valeries footsteps when she is behind you)
					;

	//if (!m_bIsAI)	[Anton] moved to UpdatePhysics
	//	UpdateStamina();

	// ladder stuff
//	if(m_stats.onLadder)
//	{
//		SetGravityOverride(0.0f);
//	}

	// update and store player's current velocity
	IPhysicalEntity *physEnt = m_pEntity->GetPhysics();	
	pe_status_living &status = ctx.status;

	if (!physEnt || !physEnt->GetStatus(&status))
	{
		GameWarning( "Bad Phyics Entity for Player %s",m_pEntity->GetName() );
		return;
	}

	Vec3d vel = (Vec3d)status.vel;	
	if (status.pGroundCollider)
		vel -= status.velGround;
	m_stats.fVel = vel.Length();

	//fixme - remove this after done with lights tweaking
	if(m_pGame->pl_force->GetIVal() && m_bIsAI && IsAlive())
		SwitchFlashLight(true);


	//fixme - this has to be done once - when model is loaded
	UpdateBonesPtrs();

	// Only updates when player have flash light on.
	ProceedFLight();

	UpdateLightBlinding();

	//UpdateCollisionDamage(); [Anton] moved to UpdatePhysics	
	CounterUpdateAll( m_pTimer->GetFrameTime() );

	//---------------------------------------------------------------	 	
	//Removed to fix BUG6238 (you always hear Valeries footsteps when she is behind you)
	//As long as the character is NOT visible, the angles were not updated and that is the reason 
	// why the "aidle" animation doesn't start!
	//	if (bPlayerVisible)
		UpdateRotateHead();

	if (bPlayerVisible && !m_pVehicle)
	{
		FRAME_PROFILER( "CPlayerUpdate::RotateHead",GetISystem(),PROFILE_GAME );
		if( m_CurStance != eProne )
		{
			//@FIXME remove this
			pe_params_pos pp;
			//				if (m_pVehicle)
			//				{
			//					Vec3d angles = GetActualAngles();
			//					pp.q = GetRotationAA(angles.z*(gf_PI/180.0f),vectorf(0,0,1));
			//				}
			if (m_bRotateHead) 
				pp.q = GetRotationAA(m_LegAngle*(gf_PI/180.0f),vectorf(0,0,1));
			else
				pp.q = GetRotationAA(m_pEntity->GetAngles().z*(gf_PI/180.0f),vectorf(0,0,1));
			pp.bRecalcBounds = 0;
			m_pEntity->GetPhysics()->SetParams(&pp);
		}
	}

	//work in progress
	//do this to prevent sliding when prone
	if( m_CurStance == eProne )
	{
//		if( !m_stats.flying && !CanProne() )
		if( !CanProne() )
			GoStand();
		else
		{
			m_EnvTangent = CalcTangentOnEnviroment( m_pEntity->GetAngles() );
			m_pEntity->SetPhysAngles( m_EnvTangent );					// set angels for phisics (body is flat on surfece tangent space)
		}
	}

	if (!m_bIsAI)
		UpdateLean();

	if (bPlayerVisible)
		UpdateBonesRotation();
	
	SetEyePos();

	if (physEnt && physEnt->GetType()==PE_LIVING)
	{
		FRAME_PROFILER( "CPlayerUpdate::PostStep",GetISystem(),PROFILE_GAME );
		pe_params_flags pf;
		pf.flagsOR = pef_custom_poststep;
		if (m_bIsAI)
			pf.flagsOR |= lef_loosen_stuck_checks;
		physEnt->SetParams(&pf);
	}

	//
	//	in water checks, swimming states updates
	bool	bPrevSweem = m_bSwimming;
	UpdateSwimState(true);
	if( m_bSwimming )
		m_stats.swimTime +=m_pTimer->GetFrameTime();
	else
		m_stats.swimTime = 0.0f;

	// update stances
	if( !m_pVehicle )
	{
		if( m_bSwimming && m_CurStance!=eSwim )
			GoSwim();
		else if(bPrevSweem != m_bSwimming && !m_bSwimming)
		{
			if(!RestorePrevStence())
				if(!GoCrouch())
					GoProne();
		}
	}

	//	if(!m_bIsAI)	// don't check/update areas for AI's
	if(bMyPlayer)		// areas are only used for the local player (they will not trigger things on the server)
	{
		FRAME_PROFILER( "CPlayerUpdate::XAreaMgrUpdatePlayer",GetISystem(),PROFILE_GAME );

m_AreaUser.SetEntity( GetEntity());
		m_pGame->m_XAreaMgr.UpdatePlayer( m_AreaUser );

		// used for camera transitions e.g. when entering a vehicle
		UpdateAutoCenter();

		// update the camera
		UpdateCamera();
	}

	if (bPlayerVisible)
		UpdateDrawAngles();

	if (!m_bIsAI)
	{
		// Only for local player.
		if (m_bFirstPerson)
		{
			m_pEntity->SetRegisterInSectors(false);
			UpdateFirstPersonView();
		}
		else
		{
			m_pEntity->SetRegisterInSectors(true);
			UpdateThirdPersonView();
		}
	}
	
	//////////////////////////////////////////////////////////////////////////////
	//UPDATE THE WEAPON
	
	if(m_stats.health>0)
	{
		if(m_stats.weapon_busy>0)
			m_stats.weapon_busy-=m_pTimer->GetFrameTime();

		UpdateMelee();
		UpdateWeapon();
	}

	if(m_CurStance == eProne || m_bSwimming)	// disable IK in prone mode and when swimming
	{
		FRAME_PROFILER( "CPlayerUpdate::DisableIK",GetISystem(),PROFILE_GAME );
		pe_params_sensors ps;
		ps.nSensors = 0;
		physEnt->SetParams(&ps);
	}

	//////////////////////////////////////////////////////////////////////////////
	//UPDATE THE ANIMATION
	// layer 0 is used for movement/stance animations (lower body)
	// layer 1 is used for aiming/shooting animations
	// layer 2 is used for jump/land animations
	UpdateCharacterAnimations( ctx );
	UpdateFireAnimations();
//	UpdateJumpAnimations();

}


void CPlayer::UpdatePhysics(float fDeltaTime)
{
	m_fLastDeltaTime = fDeltaTime;

	IPhysicalEntity *physEnt = m_pEntity->GetPhysics();	
	float MIN_WALK_SPEED = m_WalkSpeed*.15f;
	pe_status_living sl;
	physEnt->GetStatus(&sl);

	if (!m_bIsAI)
		UpdateStamina(fDeltaTime);
	UpdateCollisionDamage();

	//keep count of the last groudheight
	if (!sl.bFlying)
	{
		m_fLastGroundHeight = m_pEntity->GetPos(true).z;
		m_bHasJumped = false;
	}

	// when on ladder - no gravity
	// when using mounted weapon - no gravity 
	if(m_stats.onLadder || m_pMountedWeapon )
	{
		SetGravityOverride(0.0f);
	}

	// [Anton] - moved from ProcessMovements, since it should be called for remote clients in multiplayer
//	float	kwater;
//	int bSwimming = IsSwimming( kwater );
	pe_player_dynamics movedyn;
	movedyn.kInertia = m_Dynamics.inertia*(1-m_stats.fKWater) + m_Dynamics.swimming_inertia*m_stats.fKWater;

	movedyn.kAirControl = m_Dynamics.air_control*(1-m_stats.fKWater);
	movedyn.bSwimming = m_bSwimming;
	if (m_fGravityOverride!=1E10)
	{
		movedyn.gravity = m_fGravityOverride;
		if (!m_fGravityOverride)
		{
			movedyn.kAirControl=1.0f;
			movedyn.bSwimming=true;
		}
		m_fGravityOverride = 1E10;
	}
	else
	{
		float fgravity = m_Dynamics.gravity;
	
		//if we are jumping and there is a special gravity for jump use it.
		if (sl.bFlying && m_bHasJumped && m_Dynamics.jump_gravity!=1E10)
		{
			//use jump gravity only if we are over the jump starting pos.
			if (m_pEntity->GetPos(true).z > m_fLastGroundHeight)
				fgravity = m_Dynamics.jump_gravity;
		}

		//we land, in any case use standard gravity.
		/*if (!sl.bFlying)
		{
			fgravity = m_Dynamics.gravity;
			m_bHasJumped = false;
		}*/

		//GetISystem()->GetILog()->Log("%f",fgravity);
			
		movedyn.gravity = fgravity*(1-m_stats.fKWater)*m_pGame->p_gravity_modifier->GetFVal()+m_Dynamics.swimming_gravity*m_stats.fKWater;
	}

	// 0 gravity behaves like swimming (up and down movement is possible)
	if(movedyn.gravity==0.0f)
	{
		movedyn.bSwimming=true;												// allow not only movements in ground plane
		movedyn.kAirControl=m_Dynamics.air_control;		// water should not affect control
	}

	Vec3 vAngles = ConvertToRad(m_pEntity->GetAngles()), 
		vDir(-cry_sinf(vAngles[YAW])*cry_sinf(vAngles[PITCH]),cry_cosf(vAngles[YAW])*cry_sinf(vAngles[PITCH]),-cry_cosf(vAngles[PITCH]));
	// make underwater movement smoother
	if (m_stats.fKWater>=1.0f && sl.velRequested.len2()>0 && sqr(sl.velRequested*vDir)>sl.velRequested.len2()*0.5f
		&& m_stats.swimTime>1.0f)
		movedyn.kAirControl = 1.0f;

	if(m_bInertiaOverride)
	{
		if(movedyn.bSwimming)
			movedyn.kAirControl = 1.0f;
		else
			movedyn.kInertia = 0;	// the bigger value - the faster we can change speed (0 is special case, not warking in water)
	}

	physEnt->SetParams(&movedyn);

//	if(movedyn.bSwimming && m_bSwimming)
//		GoSwim();
//	else if(m_bSwimming && !movedyn.bSwimming)
//		RestorePrevStence();
//	m_bSwimming = movedyn.bSwimming && (m_stats.fInWater>0.0f);

	m_stats.moving = false;
	m_stats.running = false;
	
	// Calculate our current speed
	Vec3 vel = sl.vel;
	if (sl.pGroundCollider)
		vel -= sl.velGround;
	float speed = vel.Length();

	m_walkParams.speed = speed;
	m_walkParams.vertSpeed = vel.z;
	// Calculate projected current speed 
	vel.z = 0.0f;
	float speed2d = vel.Length();


	if ((speed2d > .0f && m_NotRunTime>1.5f) || (speed2d > MIN_WALK_SPEED) )
	{
		// Have the player continue moving
		m_stats.moving = true;
		m_NotRunTime = 0;
	}
	//else
	if (speed>0)
		m_NotRunTime += fDeltaTime;
	else
		m_NotRunTime = 0;

	if (m_stats.moving && (speed2d-m_WalkSpeed>0.1f || ((m_stats.flying||m_stats.landing)&&m_RunningTime>.5f)) )	// if jumping in air - slows down, not to change snimation
	{
		// Have the player start/continue running
		m_stats.running = true;
	}

	if( m_stats.running )
		m_RunningTime += fDeltaTime;
	else
		m_RunningTime = 0;

	if( m_stats.moving )
		m_WalkingTime += fDeltaTime;
	else
		m_WalkingTime = 0;

	m_pGame->ConstrainToSandbox(GetEntity());
}

///////////////////////////////////////////////

/*! Sets and loads the player-model.
		@param model filename of the player-model
*/
void CPlayer::SetPlayerModel( const string &model )
{
		
	//@FIXME: enable this later.
	//if (m_pEntity->GetCharInterface()->LoadCharacter(PLAYER_MODEL_IDX,model.c_str()))
	//		m_strModel = model;
	m_strModel = model;
	if (m_nSelectedWeaponID == -1)
		SelectWeapon(m_stats.weapon);

	UpdateBonesPtrs();
}

///////////////////////////////////////////////
/*! Retrieves a weapon-info-structure for a certain weapon
		@param nWeaponIndex weapon-id to retrieve from (negative value will return info of current weapon)
		@return WeaponInfo if the function succeeds, NULL otherwise
*/
WeaponInfo & CPlayer::GetWeaponInfo(int nWeaponIndex /* = -1 */) 
{
	if(nWeaponIndex == -1)
		nWeaponIndex = m_nSelectedWeaponID;

	PlayerWeaponsItor wi = m_mapPlayerWeapons.find(nWeaponIndex);
		
	if (wi != m_mapPlayerWeapons.end())
	{
		return wi->second;
	}

	// this should NEVER EVER be reached
	assert(false);
	return m_mapPlayerWeapons.begin()->second;
}

void CPlayer::GetCurrentWeaponParams(WeaponParams& wp)
{
	WeaponInfo wi = GetWeaponInfo();
	GetSelectedWeapon()->GetModeParams(wi.iFireMode, wp);
}

/*! Executes the processing commands
		@param ProcessingCmd structure of commands to process
*/
void CPlayer::ProcessCmd(unsigned int nPing,CXEntityProcessingCmd &ProcessingCmd)
{
	ProcessMovements(ProcessingCmd);

	// client in MP processes fire event differently
	// here the server relays the fire event to the client, but due to 
	// heavier client-side simulation of firing (see XClient.cpp) we
	// keep the state of our client-side calculation
	bool bClientFire = false;
	bool bPrevFiring = ProcessingCmd.CheckAction(ACTION_FIRE0);
	bool bPrevFireGrenade = ProcessingCmd.CheckAction(ACTION_FIRE_GRENADE);

	if (m_pGame->IsMultiplayer() && m_pGame->IsClient() && !m_pGame->IsServer() && IsMyPlayer())
	{
		bClientFire = true;
		ProcessingCmd.RemoveAction(ACTION_FIRE0);
		if (m_stats.firing)
			ProcessingCmd.AddAction(ACTION_FIRE0);

		ProcessingCmd.RemoveAction(ACTION_FIRE_GRENADE);
		if (m_stats.firing_grenade)
			ProcessingCmd.AddAction(ACTION_FIRE_GRENADE);
	}
	
	ProcessWeapons(ProcessingCmd);

	if (bClientFire)
	{
		ProcessingCmd.RemoveAction(ACTION_FIRE0);
		if (bPrevFiring)
			ProcessingCmd.AddAction(ACTION_FIRE0);

		ProcessingCmd.RemoveAction(ACTION_FIRE_GRENADE);
		if (bPrevFireGrenade)
			ProcessingCmd.AddAction(ACTION_FIRE_GRENADE);
	}
}


//
//claps angl to be within min-max range. Check fro special case when min>max -- for example min=350 max=40
inline float	ClampAngle180( float min, float max, float angl )
{

	if( angl>0 && angl<min ) 
		return min;
	else if( angl<0 && angl>max )
		return max;

	return angl;
}

///////////////////////////////////////////////
/*! Updates the orientation of the player
		@param ProcessingCmd structure of commands to process
*/
void CPlayer::ProcessAngles(CXEntityProcessingCmd &ProcessingCmd)
{
	if (IsMyPlayer() && !IsAlive())
		return;

	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	Vec3d Angles=ProcessingCmd.GetDeltaAngles();

	if (IsMyPlayer() && m_pVehicle && !m_bFirstPerson && !m_pMountedWeapon)
	{
		// [marco] if this is the localplayer, and we are driving the vehicle,
		// and we are in third person mode, AND we are not at mounted weapon
		// then do not allow the player
		// to mess around with the camera - he can switch
		// to first person mode to shoot from inside the vehicle
//		Angles.Set(0,0,180);
	}

	if(m_AngleLimitVFlag)
	//check vertical limits	
	{
		Angles.x = ClampAngle(	Snap_s360(m_AngleLimitBase.x + m_MinVAngle),
										Snap_s360(m_AngleLimitBase.x + m_MaxVAngle),
										Snap_s360(Angles.x));
		Angles.x = Snap_s180(Angles.x);
	}

	//
	//probably not the best way. If we will have angle restriction (like mounted weapon) in prone - have to chage it
	if( m_CurStance == eProne )
		m_AngleLimitBase = m_EnvTangent;

	if(m_AngleLimitHFlag)
	{
	//check horizontal limits
		bool bClamped = false;
		
		Angles.z = Snap_s180(Angles.z);
		Angles.z = ClampAngle(	Snap_s360(m_AngleLimitBase.z + m_MinHAngle),
										Snap_s360(m_AngleLimitBase.z + m_MaxHAngle),
										Snap_s360(Angles.z),bClamped);
		if (m_bIsAI && bClamped)
			m_pEntity->GetAI()->SetSignal(1,"RETURN_TO_NORMAL");

		Angles.z = Snap_s180(Angles.z);

	}

	// if at the mounted weapon - check for collisions coz player position is forced bu the weapon 
	if( m_pMountedWeapon )
	{
		Vec3d pos2check = m_pEntity->GetPos();
		if(!CanStand( pos2check ) )
		{
			// this position is bad (collides with something) - restore prev pos/angles 
//			float diff = Snap_s360(m_vSafeAngAtMountedWeapon.z) - Snap_s360(Angles.z);
			float diff = Snap_s180(Snap_s180(m_vSafeAngAtMountedWeapon.z) - Snap_s180(Angles.z));
			Angles.z += diff*.31f;
//GetISystem()->GetILog()->Log(">>> %.2f ", m_vSafeAngAtMountedWeapon.z);
		}
		else if(CanStand( pos2check ) )
		{
			// this position is good (no intersetion/collisions) - remember it 
			m_vSafeAngAtMountedWeapon = m_pEntity->GetAngles();
//GetISystem()->GetILog()->Log("<<<GD  %.2f ", m_vSafeAngAtMountedWeapon.z);
		}
	}

	ProcessingCmd.SetDeltaAngles(Angles);
	if(IsMyPlayer())
	{
		float stanceRecoilModifier = 1.0f;

		// process weapon specific recoil modifier based on stance
		if (m_nSelectedWeaponID != -1)
		{
			CWeaponClass *pSelectedWeapon = GetSelectedWeapon();

			WeaponParams wp;
			GetCurrentWeaponParams(wp);
		
			stanceRecoilModifier = wp.fRecoilModifierStanding;
			if (m_CurStance == eCrouch)
			{
				stanceRecoilModifier = wp.fRecoilModifierCrouch;
			}
			else if (m_CurStance == eProne)
			{
				stanceRecoilModifier = wp.fRecoilModifierProne;
			}
		}
		Vec3d vA=Angles;

		//GetISystem()->GetILog()->Log("RECOIL: %f %f %f", m_fRecoilXUp, m_fRecoilZUp, m_fRecoilXDelta);
		if((m_fRecoilXUp==0 && (m_fRecoilZUp==0)) && (m_fRecoilXDelta!=0)  )//blend back recoil 
		{
			//TRACE("RETURNING m_fRecoilXDelta=%f",m_fRecoilXDelta);
			float multiplier=m_stats.firing?m_pGame->w_recoil_speed_down*0.2f:m_pGame->w_recoil_speed_down;
			float m=min(1,m_pTimer->GetFrameTime()*multiplier);
			float xdiff=m_fRecoilXDelta>m_fRecoilX*m?m_fRecoilX*m:m_fRecoilXDelta;
			//float zdiff=m_fRecoilZDelta>0?min(m_fRecoilZDelta,m_fRecoilZ*m):-m_fRecoilZDelta;

			xdiff *= stanceRecoilModifier;

			if(m_fRecoilXDelta-xdiff<=0){
				xdiff=m_fRecoilXDelta;
			}
			vA.x+=xdiff;
			m_fRecoilXDelta-=xdiff;
			
			//TRACE("m_fRecoilXDelta=%f xdiff=%f m_fRecoilZDelta=%f \n",m_fRecoilXDelta,xdiff,m_fRecoilZDelta);
			ProcessingCmd.SetDeltaAngles(vA);
			//TRACE("RETURNING ENDm_fRecoilXDelta=%f",m_fRecoilXDelta);
		}

		//APPLY RECOIL
		if((m_fRecoilXUp!=0 || m_fRecoilZUp!=0) )//apply recoil
		{
			//GetISystem()->GetILog()->Log("APPLYING m_fRecoilXDelta=%f",m_fRecoilXDelta);
			float deltatime=m_pTimer->GetFrameTime()*m_pGame->w_recoil_speed_up;
			//GetISystem()->GetILog()->Log("deltatime=%f",deltatime);
			float dx=m_fRecoilXUp>0?min(m_fRecoilXUp,m_fRecoilX*deltatime):max(m_fRecoilXUp,m_fRecoilX*deltatime);
			float dz=m_fRecoilZUp>0?(float)min(m_fRecoilZUp,m_fRecoilZ*deltatime):(float)max(m_fRecoilZUp,-fabs(m_fRecoilZ*deltatime));

			if(m_fRecoilXDelta+dx>=m_pGame->w_recoil_max_degree){
				//GetISystem()->GetILog()->Log("w_recoil_max_degree=%f",m_pGame->w_recoil_max_degree);
				dx=m_fRecoilXUp;
			}
			//GetISystem()->GetILog()->Log("m_fRecoilXDelta=%f dx=%f dz=%f m_fRecoilXUp=%f m_fRecoilZUp=%f\n",m_fRecoilXDelta,dx,dz,m_fRecoilXUp,m_fRecoilZUp);
			
			m_fRecoilXUp-=dx;
			if(m_fRecoilXUp<0)m_fRecoilXUp=0;
			m_fRecoilZUp-=dz;

			dx *= stanceRecoilModifier;
			dz *= stanceRecoilModifier;

			vA.x-=dx;
			if(!m_pGame->w_recoil_vertical_only)
			{
				vA.z-=dz;
			}
			
			m_fRecoilXDelta+=dx;
			ProcessingCmd.SetDeltaAngles(vA);
			//TRACE("APPLYING END m_fRecoilXDelta=%f",m_fRecoilXDelta);
		}
		//////////////////////////////////////////////////////////////////////////
		float fWaterPitch=m_pGame->p_waterbob_pitch->GetFVal();
		float fWaterPitchSpeed=m_pGame->p_waterbob_pitchspeed->GetFVal();
		float fWaterRoll=m_pGame->p_waterbob_roll->GetFVal();
		float fWaterRollSpeed=m_pGame->p_waterbob_rollspeed->GetFVal();
		float fWaterDepthWobble=m_pGame->p_waterbob_mindepth->GetFVal();
		if (m_bEyesInWater)	// wobble camera if in water
		{
			float fWobbleScale=(m_stats.fInWater-fWaterDepthWobble);
			if (fWobbleScale<0.0f)
				fWobbleScale=0.0f;
			if (fWobbleScale>1.0f)
				fWobbleScale=1.0f;
			float fFrameTime=m_pTimer->GetFrameTime();
			m_walkParams.fWaterPitchPhase+=fWaterPitchSpeed*fFrameTime;
			if (m_walkParams.fWaterPitchPhase>=1.0f)
				m_walkParams.fWaterPitchPhase-=1.0f;
			m_walkParams.fWaterRollPhase+=fWaterRollSpeed*fFrameTime;
			if (m_walkParams.fWaterRollPhase>=1.0f)
				m_walkParams.fWaterRollPhase-=1.0f;
			//Vec3d cangles = Angles;
			vA.x += fWobbleScale*fWaterPitch*cry_sinf(m_walkParams.fWaterPitchPhase*gf_PI*2.0f);
			vA.y = (fWaterRoll *cry_sinf(m_walkParams.fWaterRollPhase *gf_PI*2.0f))*fWobbleScale;
		}else
		{
			//Vec3d cangles = vA;
			vA.y=0.0f;
			//ProcessingCmd.SetDeltaAngles(vA);
			m_walkParams.fWaterPitchPhase=0.0f;
			m_walkParams.fWaterRollPhase=0.0f;
		}
		ProcessingCmd.SetDeltaAngles(vA);
		//////////////////////////////////////////////////////////////////////////
	}

	
	///

	if (!m_pRedirected)
	{ 
		//////////////////////////////////////////////////////////////////////////
		
		//////////////////////////////////////////////////////////////////////////
		//if (!m_pVehicle)
		m_pEntity->SetAngles( Angles,false, false );

		m_vShake=Vec3d(0,0,0);

		//
		// so here goes wapon sway
		if(m_Dynamics.gravity!=0.0f)		// only if there is gravity
		{
			// add some swaying to the angles
			m_vShake.x+=(cry_cosf(m_walkParams.swayOfs*15.9f+0.3f)*m_walkParams.swayAmp*0.1f+cry_sinf(m_walkParams.swayOfs*5.9f+0.0f)*m_walkParams.swayAmp*0.45f+cry_cosf(m_walkParams.swayOfs*0.95f+0.5f)*m_walkParams.swayAmp)*0.33333f;
			m_vShake.z+=(cry_cosf(m_walkParams.swayOfs*14.9f+0.7f)*m_walkParams.swayAmp*0.12f+cry_sinf(m_walkParams.swayOfs*5.05f+0.6f)*m_walkParams.swayAmp*0.5f+cry_cosf(m_walkParams.swayOfs*1.0f+0.2f)*m_walkParams.swayAmp)*0.33333f;
			m_walkParams.swayOfs+=m_walkParams.swayFreq*m_pTimer->GetFrameTime();
		}

		//
		// after explosion camera shacking - not when in vehicle
		// when proning - aligne on terrain
		if (IsAlive() && !m_pEntity->IsBound() )
		{
			// add shaking
			if (IsMyPlayer())
			{
				if((fabsf(m_walkParams.shakeDegree)>0.1f) && (m_walkParams.shakeTime>0.0f))
				{
					m_walkParams.shakeOffset += m_walkParams.shakeFreq * m_pTimer->GetFrameTime();
					m_walkParams.shakeElapsedTime += m_pTimer->GetFrameTime();
					if (m_walkParams.shakeElapsedTime>m_walkParams.shakeTime)
					{
						m_walkParams.shakeDegree=0.0f;
					}else
					{
						float fDeg = m_walkParams.shakeDegree * ( 1 - ( m_walkParams.shakeElapsedTime / m_walkParams.shakeTime ) );
						float fSinOfs = cry_sinf(m_walkParams.shakeOffset * 6.283185307179586476925286766559f);
//						m_vShake.x += m_walkParams.shakeAxis.x * fSinOfs * fDeg;
//						m_vShake.y += m_walkParams.shakeAxis.y * fSinOfs * fDeg;
//						m_vShake.z += m_walkParams.shakeAxis.z * fSinOfs * fDeg;

					// make it shake on camera roll axis
					// create the matrix here
						Matrix44 matWorld;
						matWorld.SetIdentity();

						matWorld=Matrix44::CreateRotationZYX(-gf_DEGTORAD*m_vEyeAngles)*matWorld; //NOTE: angles in radians and negated 

						CryQuat cxquat = Quat( GetTransposed44(matWorld) );

						CryQuat rxquat;
						Vec3d	shake( 0, fSinOfs * fDeg, 0 );
						rxquat.SetRotationXYZ(DEG2RAD(shake));

						CryQuat result = cxquat*rxquat;
						Vec3d finalangles = RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(result)));
						m_vShake += m_vEyeAngles - finalangles;
					}
				}
			}
		}
	}
	else 
	{
		ProcessingCmd.SetDeltaAngles(Angles);

//		m_pEntity->SetAngles( Angles );//,false, false );
		if (m_pRedirected)	// [marco] Kirill does this check make sense - 
												// this should never happens but it crashes with the helicopter
			m_pRedirected->SetAngles(Angles);
	//	ProcessingCmd.Reset();
	}
}

//#define UNDERWATER_SPEED	0.19f/8.0f

///////////////////////////////////////////////
/*! Updates the position and stats of the player
		@param ProcessingCmd structure of commands to process
*/
void CPlayer::ProcessMovements(CXEntityProcessingCmd &cmd, bool bScheduled)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	IPhysicalEntity *pPhysEnt = (IPhysicalEntity *)m_pEntity->GetPhysics();

	if(!pPhysEnt)
		return;
	
	//[kirill] need this to process autocentering
	// no autocentering when moving mouse or fiering
	m_bMouseMoved = cmd.CheckAction(ACTION_TURNLR) || cmd.CheckAction(ACTION_TURNUD) || cmd.CheckAction(ACTION_FIRE0);

	PhysicsVars *physvars = m_pGame->GetSystem()->GetIPhysicalWorld()->GetPhysVars();
	// [Anton] setting dynprops for the physics is moved to ::Update
	pe_player_dynamics pd;
	pPhysEnt->GetParams(&pd);
	
	bool	bSuperDesignerMode = pd.gravity==0 && pd.kInertia==0;
	bool	bNoGravity = !bSuperDesignerMode && (physvars->bFlyMode || IsSwimming() || m_stats.onLadder);

//return;
/* this blick would be needed only when designers tweak player's speeds
//FixME fix me FIXME!!!
	//if(IsMyPlayer())
	{
//		m_RunSprintDecoyScale = m_pGame->p_sprint_decoy->GetFVal();
//		m_RunSprintRestoreScaleRun = m_pGame->p_sprint_restore_run->GetFVal();
//		m_RunSprintRestoreScaleIdle = m_pGame->p_sprint_restore_idle->GetFVal();
//		m_RunSprintScale = m_pGame->p_sprint_scale->GetFVal();

		m_RunSpeed = m_pGame->p_speed_run->GetFVal();
		m_WalkSpeed = m_pGame->p_speed_walk->GetFVal();
		m_CrouchSpeed = m_pGame->p_speed_crouch->GetFVal();
		m_ProneSpeed = m_pGame->p_speed_prone->GetFVal();
		m_JumpForce = m_pGame->p_jump_force->GetFVal();
		m_LeanDegree = m_pGame->p_lean->GetFVal();

		m_JTWalk = m_pGame->p_jump_walk_time->GetFVal();
		m_JTRun = m_pGame->p_jump_run_time->GetFVal();
	}
*/
	m_StealthSpeed = (m_CrouchSpeed + m_WalkSpeed)*.5f;

	
	IPhysicalWorld *pW=pPhysEnt->GetWorld();

	float fTimeDelta=0;
	CXClient *pClient=m_pGame->GetClient();

	if(!pClient || pClient->GetPlayerId()!=m_pEntity->GetId())
		fTimeDelta=cmd.GetServerPhysDelta();
	
	if (!m_pVehicle && !m_pMountedWeapon && fTimeDelta>0)
	{
		pPhysEnt->StepBack(fTimeDelta);

		m_NotRunTime = m_NotRunTimeClient;
		m_RunningTime = m_RunningTimeClient;
		m_WalkingTime = m_WalkingTimeClient;
		m_stats.stamina = m_staminaClient;
//		m_stats.breath = m_breathClient;
	}

	if (!IsAlive())
		return;

	if (cmd.CheckAction(ACTION_FLASHLIGHT))
	{
		if (m_pVehicle && m_stats.inVehicleState == PVS_DRIVER)
			m_pVehicle->SwitchLights(5);	// switch car's lights
		else if (m_stats.has_flashlight)
			SwitchFlashLight(!m_bLightOn);
//		cmd.RemoveAction(ACTION_FLASHLIGHT);
	}

	// when in vehicle - use crouch key to toggle camera autocentering
	if ((cmd.CheckAction(ACTION_MOVEMODE) || cmd.CheckAction(ACTION_MOVEMODE_TOGGLE)) && m_pVehicle)
	{
		StartAutoCenter( false );

		cmd.RemoveAction(ACTION_MOVEMODE);
		cmd.CheckAction(ACTION_MOVEMODE_TOGGLE);
	}

	// use button has been removed		
	m_stats.melee_attack=false;
	if(cmd.CheckAction(ACTION_USE))
	{
		bool bUsed=false;
		m_pEntity->SendScriptEvent(ScriptEvent_Use,0,&bUsed);
		if (bUsed==false)
		{
			m_stats.use_pressed=true;
		}
	}
	else
	{
		m_stats.use_pressed=false;
	}
	
	if (cmd.CheckAction(ACTION_CHANGE_VIEW))
	{		
		//if (m_pGame->IsDevModeEnable())
		{
			m_pGame->SetViewMode(m_bFirstPerson);		
		}
		cmd.RemoveAction(ACTION_CHANGE_VIEW);
	}

	if ( m_pVehicle )
	{
		//if (!m_pGame->IsMultiplayer() || !m_pGame->UseFixedStep() || bScheduled)
			ProcessVehicleMovements(cmd);	// so that we dont need any reference to vechile in this file
			// can't sprint in vehicle
			m_Sprinting = false;
		return;
	}

	// if using mounted weapon - can't move
	if(m_pMountedWeapon)
	{
		// can't sprint when useing mounted weapon
		m_Sprinting = false;
		return;
	}

	if (m_nSelectedWeaponID != -1)
	{
		WeaponParams wp;
		GetCurrentWeaponParams(wp);

		m_stats.holding_breath=false;
		if (wp.bAllowHoldBreath && cmd.CheckAction(ACTION_HOLDBREATH) && m_stats.stamina>m_StaminaTable.BreathDecoyAim*.5f)
		{
			m_stats.holding_breath = true;
		}
	}

	//lets remove the action flag every frame if in crouching mode.
	if(m_pGame->IsServer())
	{
		//crouch toggle
		if (cmd.CheckAction(ACTION_MOVEMODE_TOGGLE))
		{
			//apply the toggle crouch only if we are outside a vehicle
			if (!m_pVehicle)
				m_bStayCrouch = (m_bStayCrouch)?false:true;

			cmd.RemoveAction(ACTION_MOVEMODE_TOGGLE);
		}

		bool bGoCrouch = cmd.CheckAction(ACTION_MOVEMODE) || m_bStayCrouch;
		bool bGoProne = cmd.CheckAction(ACTION_MOVEMODE2);

		if (m_stats.crouch && !m_bSwimming && !m_stats.onLadder)
		{
			//if it was crouching lets remove it otherwise the "onhold" will not work
			if (!bGoCrouch)
				RestorePrevStence();
//				GoStand();		
		}

		if (bGoCrouch || bGoProne)
		{
			if (!m_stats.onLadder)
			{
				if (m_stats.crouch)
				{
					if (bGoProne)
					{
						if (m_PrevStance == eProne)
							GoStand(false);
						else
							GoProne( );
					}
				}
				else 
				if (m_CurStance == eProne)
				{
					if (bGoProne)
	//					RestorePrevStence();
						GoStand(false);
					else
						GoCrouch( );
				}
				else 
				if (m_CurStance == eSwim)
				{
					if (bGoProne)
	//				RestorePrevStence();
						GoStand(false);
				}
				else
				{
					if (bGoProne)
						GoProne( );
					else
						GoCrouch( );
				}
			}
			cmd.RemoveAction(ACTION_MOVEMODE);
			cmd.RemoveAction(ACTION_MOVEMODE2);
		}
	}

	int bJump = 0;

	if (  (	cmd.CheckAction(ACTION_WALK)^m_bAlwaysRun)&&
				(	cmd.CheckAction(ACTION_MOVE_RIGHT)||cmd.CheckAction(ACTION_MOVE_LEFT) ||
					cmd.CheckAction(ACTION_MOVE_FORWARD)||cmd.CheckAction(ACTION_MOVE_BACKWARD) ) &&
					!m_stats.bForceWalk )
	{
		if (!m_Running)
		{
			// cannot run if in crouch mode
			if (!m_stats.crouch&&!m_stats.prone&&!m_bSwimming)
			{
				GoStand();			
				m_Running = true;
			}			
		}
		
	}
	else
	{
		if( m_Running )
		{
			RestorePrevStence();
		}

		m_Running = false;
	}

	if (m_Running)
	{
		if ( cmd.CheckAction(ACTION_RUNSPRINT) && m_stats.stamina>m_StaminaTable.DecoyRun*.5f 
			|| cmd.CheckAction(ACTION_RUNSPRINT) && m_Sprinting && m_stats.stamina>1)
			m_Sprinting = true;
		else
			m_Sprinting = false;
		m_stats.running = true;
	}
	else 
	{
		m_stats.running = false;
		m_Sprinting = false;
	}

	m_walkParams.fLeanTarget=0;
	// calculate leaning (if not proning and not at mounted weapon)
	if (cmd.CheckAction(ACTION_LEANLEFT) && (!m_stats.lock_weapon) && (m_CurStance != eProne))
	{
		if(IsMyPlayer())
		{
			if (m_walkParams.leanEnd!=1.0f)
			{
				m_walkParams.leanStart=m_walkParams.leanAmount;
				m_walkParams.leanEnd=1.0f;
				m_walkParams.leanFactor=0.0f;
				m_walkParams.leanDegree=m_LeanDegree;
			}
			m_walkParams.fLeanTarget=.5;
			m_walkParams.leanSpeed=1.5f;
		}
	}else
	if (cmd.CheckAction(ACTION_LEANRIGHT) && (!m_stats.lock_weapon) && (m_CurStance != eProne))
	{
		if(IsMyPlayer())
		{
			if (m_walkParams.leanEnd!=-1.0f)
			{
				m_walkParams.leanStart=m_walkParams.leanAmount;
				m_walkParams.leanEnd=-1.0f;
				m_walkParams.leanFactor=0.0f;
				m_walkParams.leanDegree=m_LeanDegree;


			}
			m_walkParams.fLeanTarget=-1;
			m_walkParams.leanSpeed=1.5f;
		}
	}else if (cmd.CheckAction(ACTION_MOVE_LEFT) )
	{
		if( !m_stats.onLadder )
		{
			if (m_walkParams.leanEnd!=0.1f) 
			{
				m_walkParams.leanStart=m_walkParams.leanAmount;
				m_walkParams.leanEnd=0.1f;
				m_walkParams.leanFactor=0.0f;
				m_walkParams.leanDegree=m_LeanDegree;
			}
			if ((m_walkParams.leanAmount>=0.0f) && (m_walkParams.leanAmount<=0.1f))
				m_walkParams.leanSpeed=0.1f;

			m_walkParams.fLeanTarget=0.05f;

			if(m_walkParams.fCurrLean==0)
				m_walkParams.leanSpeed=0.2f;
		}	
	}else	if (cmd.CheckAction(ACTION_MOVE_RIGHT) )
	{
		if( !m_stats.onLadder )
		{
			if (m_walkParams.leanEnd!=-0.1f)
			{
				m_walkParams.leanStart=m_walkParams.leanAmount;
				m_walkParams.leanEnd=-0.1f;
				m_walkParams.leanFactor=0.0f;
				m_walkParams.leanDegree=m_LeanDegree;
			}
			if ((m_walkParams.leanAmount<=0.0f) && (m_walkParams.leanAmount>=-0.1f))
				m_walkParams.leanSpeed=0.1f;


			m_walkParams.fLeanTarget=-0.05f;

			if(m_walkParams.fCurrLean==0)
				m_walkParams.leanSpeed=0.2f;
		}
	}else	if (m_walkParams.leanEnd!=0.0f)
		{
			m_walkParams.leanStart=m_walkParams.leanAmount;
			m_walkParams.leanEnd=0.0f;
			m_walkParams.leanFactor=0.0f;
		}
	
	//////////////////
	//convert in the format used by physics
	Vec3d tempangle = cmd.GetDeltaAngles();
	Vec3d dirangle = tempangle;
	dirangle=ConvertToRad(dirangle);	

	float m_pcos = cry_cosf(dirangle[YAW]);//*inputspeed;
	float m_psin = cry_sinf(dirangle[YAW]);//*inputspeed;
	
	float m_pcos2 = cry_cosf(dirangle[PITCH]);
	float m_psin2 = cry_sinf(dirangle[PITCH]);

	m_walkParams.dir.x = -m_psin;
	m_walkParams.dir.y =	m_pcos;
	m_walkParams.dir.z = 0.0f;

	Vec3d speedxyz(0.0f, 0.0f, 0.0f);

	pe_status_dynamics dynStatus;
	pPhysEnt->GetStatus((pe_status*)&dynStatus);

	float	fWaterMoveScale = m_SwimSpeed*0.5f*m_stats.fKWater + (1.0f-m_stats.fKWater);
	
	if (cmd.CheckAction(ACTION_MOVE_FORWARD)) 
	{				            
		if(m_stats.onLadder)	// when on ladder - move mostly UP/DOWN
		{
			speedxyz[0] += -m_psin*.3f;
			speedxyz[1] +=	m_pcos*.3f;
		}
		else
		{
			speedxyz[0] += -m_psin;
			speedxyz[1] +=	m_pcos;
		}
		if (bNoGravity) 
		{
			speedxyz[0] *= m_psin2;
			speedxyz[1] *= m_psin2;		
			//
			//if swimming up - make sure not to go too fast to prevent jumping out of water
			if(m_bSwimming && m_pcos2<0)
			{
				if(	(m_stats.fInWater < m_PlayerDimSwim.heightEye && 
//								m_walkParams.vertSpeed > .02f &&
					!m_stats.onLadder) || dynStatus.v.z<-7.0f)
					speedxyz[2] = -m_pcos2*fWaterMoveScale*m_stats.fKWater*.02f;	// if close to surface - don't go up too fast - not to jump out of water
																			// or big vertical vel - falling down				
				else
					speedxyz[2] = -m_pcos2*fWaterMoveScale;	// deep enoght - can go up
			}
			else
				speedxyz[2] = -m_pcos2;//*inputspeed;
		}
		else if(bSuperDesignerMode)
		{
			speedxyz[2] = -m_pcos2;//*inputspeed;
		}
	}
	if (cmd.CheckAction(ACTION_MOVE_BACKWARD)) 
	{			
		m_stats.back_pressed = true;

		//FIXME: would be nice if backward key detach us from the ladder when we approach the ground (instead use the jump button), but for this
		//more info about the ladder are needed, like the center position, to know if we are going up or down.
//		if(m_stats.onLadder /*&& (m_fLastGroundHeight+1.0 <m_pEntity->GetPos(true).z)*/)// when on ladder - move mostly UP/DOWN
//		{
//			speedxyz[0] += -m_psin*.3f;
//			speedxyz[1] -=	m_pcos*.3f;
//		}
//		else
		{
			speedxyz[0] -= -m_psin;
			speedxyz[1] -=	m_pcos;
		}
		if ( bNoGravity ) 
		{
			speedxyz[0] *= m_psin2;
			speedxyz[1] *= m_psin2;		
			//
			//if swimming up - make sure not to go too fast to prevent jumping out of water
			if(m_bSwimming && m_pcos2>0)
			{
				if(	(m_stats.fInWater < m_PlayerDimSwim.heightEye && 
//								m_walkParams.vertSpeed > .02f &&
					!m_stats.onLadder) || dynStatus.v.z<-7.0f)
					speedxyz[2] = m_pcos2*fWaterMoveScale*m_stats.fKWater*.02f;	// if close to surface - don't go up too fast - not to jump out of water
				else
					speedxyz[2] = m_pcos2*fWaterMoveScale;	// deep enoght - can go up
			}
			else
				speedxyz[2] = m_pcos2;//*inputspeed;
		}
		else if(bSuperDesignerMode)
		{
			speedxyz[2] = m_pcos2;//*inputspeed;
		}
	}	
	else
		m_stats.back_pressed = false;

	bool bStrafe = false;
	if (cmd.CheckAction(ACTION_MOVE_LEFT)) //&& !m_stats.onLadder) 
	{								
		/*if (m_stats.onLadder)
		{
			speedxyz[0] -= m_pcos*0.1f;
			speedxyz[1] -= m_psin*0.1f;
		}
		else*/
		{
			speedxyz[0] -= m_pcos;
			speedxyz[1] -= m_psin;
		}
		bStrafe = true;
	}	

	if (cmd.CheckAction(ACTION_MOVE_RIGHT)) //&& !m_stats.onLadder) 
	{			
		/*if (m_stats.onLadder)
		{
			speedxyz[0] += m_pcos*0.1f;
			speedxyz[1] += m_psin*0.1f;
		}
		else*/
		{
			speedxyz[0] += m_pcos;
			speedxyz[1] += m_psin;
		}

		bStrafe = true;
	}

	if (GetLengthSquared(speedxyz)>0)
	{
	float fSpeedScale = m_stats.fSpeedScale*m_stats.dmgSpeedCoeff/100.0f;
	float inputspeed;

		if(m_stats.back_pressed && !m_stats.onLadder)
		{
			switch(m_CurStance)
			{
				case eStealth:
					inputspeed = m_StealthSpeedBack;
					break;
				case eCrouch:
					inputspeed = m_CrouchSpeedBack;
					break;
				case eProne:
					inputspeed = m_ProneSpeedBack;
					break;
				case eSwim:
					inputspeed = m_SwimSpeedBack;
					break;
				default:
					inputspeed = m_WalkSpeedBack;
					break;
			}

			if(m_Sprinting)
				inputspeed = m_RunSpeedBack*m_StaminaTable.RunSprintScale;
			else if(m_stats.running)
				inputspeed = m_RunSpeedBack;
		}
		else if(!bStrafe || m_stats.onLadder)
		{
			switch(m_CurStance)
			{
				case eStealth:
					inputspeed = m_StealthSpeed;
					break;
				case eCrouch:
					inputspeed = m_CrouchSpeed;
					break;
				case eProne:
					inputspeed = m_ProneSpeed;
					break;
				case eSwim:
					inputspeed = m_SwimSpeed;
					break;
				default:
					inputspeed = m_WalkSpeed;
					break;
			}

			if(m_Sprinting)
				inputspeed = m_RunSpeed*m_StaminaTable.RunSprintScale;
			else if(m_stats.running)
				inputspeed = m_RunSpeed;
		}
		else
		{
			switch(m_CurStance)
			{
				case eStealth:
					inputspeed = m_StealthSpeedStrafe;
					break;
				case eCrouch:
					inputspeed = m_CrouchSpeedStrafe;
					break;
				case eProne:
					inputspeed = m_ProneSpeedStrafe;
					break;
				case eSwim:
					inputspeed = m_SwimSpeedStrafe;
					break;
				default:
					inputspeed = m_WalkSpeedStrafe;
					break;
			}

			if(m_Sprinting)
				inputspeed = m_RunSpeedStrafe*m_StaminaTable.RunSprintScale;
			else if(m_stats.running)
				inputspeed = m_RunSpeedStrafe;
		}

		if(bSuperDesignerMode)
		{
			if (m_Running)
				inputspeed = m_pGame->p_speed_run->GetFVal();
			else
				inputspeed = m_pGame->p_speed_walk->GetFVal();
		}

		inputspeed *= fSpeedScale;
		speedxyz.Normalize();
		speedxyz*=inputspeed;
	}
	

	//Vec3 ppos=m_pEntity->GetPos(true);

	// if player wants to jump AND proning - stand up
	// otherwice if not crouching AND has enough stamina for jump 
	// AND not in air right now OR in water deep enough (go up then)
	// AND not in landing animation
	// do jump
 	if ((cmd.CheckAction(ACTION_JUMP)))
	{
		//if player is jumping when on ladder throw him back, to detach from the ladder.
		if (m_stats.onLadder)
		{
			speedxyz[0] -= -m_psin;
			speedxyz[1] -=	m_pcos;
		}
		else if(m_CurStance == eProne)
		{
			GoStand(false);
			cmd.RemoveAction(ACTION_JUMP);
		}
		else 
		if (m_CurStance != eCrouch			
//			&& m_CurStance != eProne
				&& ((!m_stats.flying) || IsSwimming()) 
				&& ((m_stats.stamina>m_StaminaTable.DecoyJump&&!m_stats.flying) || IsSwimming())
				&& m_JumpAniLenght<=0
				)
		{
			m_JumpHeight[0] = m_pGame->p_jump_walk_h->GetFVal();
			m_JumpHeight[1] = m_pGame->p_jump_walk_h->GetFVal();
 			m_JumpHeight[2] = m_pGame->p_jump_run_h->GetFVal();

			m_JumpDist[0] = m_pGame->p_jump_walk_d->GetFVal();
			m_JumpDist[1] = m_pGame->p_jump_walk_d->GetFVal();
			m_JumpDist[2] = m_pGame->p_jump_run_d->GetFVal();

			float	jumpSpeedH, jumpSpeedV;
			bJump = 1;
			Vec3d	horSpeed = speedxyz;

			if(!IsSwimming())
			{
				if( m_RunningTime >= m_JTRun )	// if runs long enough - long jump
					CalcJumpSpeed( m_JumpDist[2], m_JumpHeight[2], jumpSpeedH, jumpSpeedV );
				else if( m_WalkingTime >= m_JTWalk )	// if runs long enough - long jump
					CalcJumpSpeed( m_JumpDist[1], m_JumpHeight[1], jumpSpeedH, jumpSpeedV );				
				else
					CalcJumpSpeed( m_JumpDist[0], m_JumpHeight[0], jumpSpeedH, jumpSpeedV );

				horSpeed.z = 0;
				horSpeed.Normalize();
				speedxyz[0] = horSpeed.x*jumpSpeedH;
				speedxyz[1] = horSpeed.y*jumpSpeedH;
				speedxyz[2] = jumpSpeedV;

				// decrease stamina - jumping is not for free!
				if((m_stats.stamina-=m_StaminaTable.DecoyJump)<0)
					m_stats.stamina = 0;

				m_bHasJumped = true;
			}
			else
			{
			float depth = m_p3DEngine->GetWaterLevel(m_pEntity) - m_p3DEngine->GetTerrainElevation(m_pEntity->GetPos().x, m_pEntity->GetPos().y);
				// if not deep and pressed jupm key - stand up, if can't stand (too deep) - push player up
				if( depth<1.5f )
				{
					GoStand(false);
					cmd.RemoveAction(ACTION_JUMP);
				}
				else
				{
				// if pressing jump in water - just push playet up
					speedxyz[2] = 4.0f;

				// when swimming underwater and goung up by press JUMP 
				// if too close to surface - don't aplpy much UP impulse 
				// to prevent jumping out of water
					if(m_stats.fInWater<.75f)
						speedxyz[2] = 0;
					else if(m_stats.fInWater<2.25f)
  						speedxyz[2] *= (m_stats.fInWater/2.25f)*.2f;
				}
			}
		}
	}

	pe_status_living status;
	pPhysEnt->GetStatus((pe_status*)&status);
	m_stats.jumping = (bJump)?true:false;
	
	pe_action_move hike;
	hike.iJump = bJump && !bNoGravity;
	hike.dir=speedxyz;     

	DampInputVector(hike.dir,m_input_accel,m_input_stop_accel,true,false);
	
	pPhysEnt->Action(&hike);

	float moveVel = speedxyz[0]*speedxyz[0] + speedxyz[1]*speedxyz[1];
	// if the velosity is too low - not sprinting then
	if(moveVel<2.0f)//|| (m_CurStance!=eStand && m_CurStance!=eRelaxed && m_CurStance!=eSwim))
		m_Sprinting = false;

	
	/*if(pPhysEnt) do
	{
		pPhysEnt->Step(min(fTimeDelta,0.02f));
		fTimeDelta -= 0.02f;
	} while(fTimeDelta>1E-4f);*/
	float *pfSlices,sumSlice;
	int i,nSlices = cmd.GetTimeSlices(pfSlices);
	for(i=0,sumSlice=0; i<nSlices; i++)
		sumSlice += pfSlices[i];
	pPhysEnt->StartStep(sumSlice);
  for(i=0; i<nSlices-1 && pPhysEnt; i++)
	{
		pPhysEnt->Step(pfSlices[i]);
		pPhysEnt = m_pEntity->GetPhysics();	// update pPhysEnt since Step callback might change it (destroy/create new)
	}
	m_NotRunTimeClient = m_NotRunTime;
	m_RunningTimeClient = m_RunningTime;
	m_WalkingTimeClient = m_WalkingTime;
	m_staminaClient = m_stats.stamina;
//	m_breathClient = m_stats.breath;
	if (nSlices>0 && pPhysEnt) // the last slice is teh 'server catch-up' slice, client doesn't have it
		pPhysEnt->Step(pfSlices[nSlices-1]);

}

///////////////////////////////////////////////
/*! Retrieves the currently selected weapon
		@return selected weapon
*/ 
CWeaponClass* CPlayer::GetSelectedWeapon() const
{
	CWeaponClass *pSelectedWeapon = m_pGame->GetWeaponSystemEx()->GetWeaponClassByID(m_nSelectedWeaponID);

	// make sure the weapon class (models, scripts, etc..) is loaded ...
	if (pSelectedWeapon && !pSelectedWeapon->IsLoaded())
	{
		pSelectedWeapon->Load();
	}
	return pSelectedWeapon;
}
 
//////////////////////////////////////////////////////////////////////////
void CPlayer::SelectNextWeapon()
{
	int curr=0;
	if(m_nSelectedWeaponID != -1)
		curr = m_nSelectedWeaponID;

	int slot=0;

	while(slot<PLAYER_MAX_WEAPONS && m_vWeaponSlots[slot]!=curr)
		slot++;

	slot=(slot+1)%PLAYER_MAX_WEAPONS;

	int iteration=0;
	while(iteration<PLAYER_MAX_WEAPONS && (m_vWeaponSlots[slot]==0 || m_vWeaponSlots[slot]==curr))
	{
		iteration++;
		slot=(slot+1)%PLAYER_MAX_WEAPONS;
	}
	if(m_vWeaponSlots[slot]!=0 && m_vWeaponSlots[slot]!=curr)
		SelectWeapon(m_vWeaponSlots[slot]);
}

//////////////////////////////////////////////////////////////////////////
void CPlayer::SelectPrevWeapon()
{
	int curr=0;
	if(m_nSelectedWeaponID != -1)
		curr=m_nSelectedWeaponID;

	int slot=0;

	while(slot<PLAYER_MAX_WEAPONS && m_vWeaponSlots[slot]!=curr)
		slot++;

	slot=(slot-1);

	int iteration=0;
	while(iteration<PLAYER_MAX_WEAPONS && (m_vWeaponSlots[slot]==0 || m_vWeaponSlots[slot]==curr))
	{
		iteration++;
		slot=slot-1;
		if(slot<0)
		{
			slot = PLAYER_MAX_WEAPONS-1;
			while(slot>=0 && (m_vWeaponSlots[slot]==0))slot--;
			if(slot<0)slot=0;
		}
	}
	if(m_vWeaponSlots[slot]!=0 && m_vWeaponSlots[slot]!=curr)
		SelectWeapon(m_vWeaponSlots[slot]);
}

/*! Updates the weapons of the player
		@param ProcessingCmd structure of commands to process
*/
//////////////////////////////////////////////////////////////////////////
void CPlayer::ProcessWeapons(CXEntityProcessingCmd &cmd)
{	
	// do not allow to use weapons and move when underwater or in a wtaer volume, and 
	// he is actively swimming			

	if(!IsMyPlayer())
		m_walkParams.fCurrLean=cmd.GetLeaning();

	if (cmd.CheckAction(ACTION_NEXT_WEAPON) && (!m_stats.lock_weapon))
	{
		SelectNextWeapon();		
	}
	else if (cmd.CheckAction(ACTION_PREV_WEAPON) && (!m_stats.lock_weapon))
	{
		SelectPrevWeapon();
	}
	else
	{
		if(!m_stats.lock_weapon)
		{
			bool bSkip = false;
			int w = 0;
			int WeaponGroup=0;
			if (cmd.CheckAction(ACTION_WEAPON_0))
			{
				WeaponGroup = 10;
				cmd.RemoveAction(ACTION_WEAPON_0);
			}
			else if (cmd.CheckAction(ACTION_WEAPON_1))
			{
				WeaponGroup = 1;
				cmd.RemoveAction(ACTION_WEAPON_1);
			}
			else if (cmd.CheckAction(ACTION_WEAPON_2))
			{
				WeaponGroup = 2;
				cmd.RemoveAction(ACTION_WEAPON_2);
			}
			else if (cmd.CheckAction(ACTION_WEAPON_3))
			{
				WeaponGroup = 3;
				cmd.RemoveAction(ACTION_WEAPON_3);
			}
			else if (cmd.CheckAction(ACTION_WEAPON_4))
			{
				WeaponGroup = 4;
				cmd.RemoveAction(ACTION_WEAPON_4);
			}
/*			else if (cmd.CheckAction(ACTION_WEAPON_5))
			{
				WeaponGroup = 5;
				cmd.RemoveAction(ACTION_WEAPON_5);
			}
			else if (cmd.CheckAction(ACTION_WEAPON_6))
			{
				WeaponGroup = 6;
				cmd.RemoveAction(ACTION_WEAPON_6);
			}
			else if (cmd.CheckAction(ACTION_WEAPON_7))
			{
				WeaponGroup = 7;
				cmd.RemoveAction(ACTION_WEAPON_7);
			}
			else if (cmd.CheckAction(ACTION_WEAPON_8))
			{
				WeaponGroup = 8;
				cmd.RemoveAction(ACTION_WEAPON_8);
			}
*/		else
				bSkip = true;

			if (!bSkip)
			{
				w = m_nSelectedWeaponID;
				bool bFirstWeapon = true;
				int iFirstID=-1;
				WeaponGroup-=1;
				if(WeaponGroup<PLAYER_MAX_WEAPONS)
				{
					if(m_vWeaponSlots[WeaponGroup])
					{
						SelectWeapon(m_vWeaponSlots[WeaponGroup]);
					}
				}
			}
		}
	}

	// get the selected weapon
	CWeaponClass *pSelectedWeapon = GetSelectedWeapon();

	//uses this flag to affect the accuracy
	if(!m_bFirstPerson)
		m_stats.aiming=cmd.CheckAction(ACTION_ZOOM_TOGGLE);

	if (cmd.CheckAction(ACTION_CYCLE_GRENADE))
	{
		m_pEntity->SendScriptEvent(ScriptEvent_CycleGrenade,0);
		cmd.RemoveAction(ACTION_CYCLE_GRENADE);
	}

	if (cmd.CheckAction(ACTION_FIREMODE))
	{
		SwitchFiremode();
		cmd.RemoveAction(ACTION_FIREMODE);
	}
	
	if (cmd.CheckAction(ACTION_RELOAD) && (!m_stats.lock_weapon) &&
			(m_stats.underwater==0.0f) && !m_bSwimming)
	{
		if (pSelectedWeapon)
		{
			pSelectedWeapon->ScriptOnStopFiring(m_pEntity);
			pSelectedWeapon->ScriptReload(m_pEntity);
		}
		
		cmd.RemoveAction(ACTION_RELOAD);
	}

	// Check if the player is dropping his weapon
	if(cmd.CheckAction(ACTION_DROPWEAPON) && (!m_stats.lock_weapon))
	{

		if (pSelectedWeapon && (CountAvaliableWeapons()>1))
		{					
			// Remove the current weapon from the player's inventory
			PlayerWeaponsItor WeaponIterator = m_mapPlayerWeapons.find(m_nSelectedWeaponID);
			if (WeaponIterator == m_mapPlayerWeapons.end()) 
			{
				// This should not ever happen
				TRACE("Could not find dropped weapon in weapon list");
				return;
			}

			// Notify the weapon that it has been dropped
			_SmartScriptObject pTable(m_pScriptSystem);
			pTable->SetValue("Player", m_pEntity->GetScriptObject());
			pTable->SetValue("WeaponID", m_nSelectedWeaponID);
			pSelectedWeapon->ScriptDrop(pTable);
		}

		// Remove the command
		cmd.RemoveAction(ACTION_DROPWEAPON);
	}

	if (m_bWaitForFireRelease && !cmd.CheckAction(ACTION_FIRE0))
	{
		m_bWaitForFireRelease = false;
	}

	if (!m_bWaitForFireRelease)
	{
		m_stats.firing = (cmd.CheckAction(ACTION_FIRE0)) != 0;
	}
	else
		m_stats.firing = false;


	if(cmd.CheckAction(ACTION_FIRECANCEL))
		m_stats.cancelFireFlag = true;
	else
		m_stats.cancelFireFlag = false;

	if (!GetGame()->IsMultiplayer() || m_bWriteOccured)
		m_stats.firing_grenade = false;

	if ((cmd.CheckAction(ACTION_FIRE_GRENADE)))
	{
		if(m_stats.weapon_busy<=0)
		{
			m_stats.firing_grenade = true;
		}
	}

	if (m_bWriteOccured) m_bWriteOccured = false;
}

//////////////////////////////////////////////////////////////////////////
void CPlayer::FireGrenade(const Vec3d &origin, const Vec3d &angles, IEntity *pIShooter)
{
	_SmartScriptObject pTable(m_pScriptSystem);

	m_ssoHitPosVec=origin;
	m_ssoHitNormVec=angles;//angles;
	Vec3d dir=angles;
	dir=ConvertToRadAngles(dir);	
	 
	// projectiles (grenades, for ex) should inherit player's velocity	
	m_ssoHitDirVec=dir; //+(m_walkParams.dir.normalize()*(m_stats.fVel*0.125f));	
	//m_pGame->GetSystem()->GetILog()->LogToConsole("vel=%f,dir=%f,%f,%f",m_stats.fVel,m_walkParams.dir.x,m_walkParams.dir.y,m_walkParams.dir.z);
	float fWaterLevel=m_p3DEngine->GetWaterLevel(m_pEntity);
	pTable->SetValue("pos",m_ssoHitPosVec);
	pTable->SetValue("angles",m_ssoHitNormVec);
	pTable->SetValue("dir",m_ssoHitDirVec);

	pTable->SetValue("lifetime",3000);

	if(m_nSelectedWeaponID != -1 && m_stats.firing && !(m_stats.grenadetype!= 1 && m_stats.numofgrenades <= 0))
		GetSelectedWeapon()->ScriptOnStopFiring(m_pEntity);

	if (fWaterLevel>origin.z)
		pTable->SetValue("underwater",0);
	else
		pTable->SetToNull("underwater");
	m_pEntity->SendScriptEvent(ScriptEvent_FireGrenade, *pTable);
	if(m_pMountedWeapon)	// stop fire animation on mounted weapon
		m_pMountedWeapon->SendScriptEvent(ScriptEvent_Fire,0);
}

//////////////////////////////////////////////////////////////////////////
void CPlayer::SetFiring(bool bIsFiring)
{
	eFireType lastft=m_stats.FiringType;
	if (/*m_pSelectedWeapon &&*/ bIsFiring && IsAlive() && !m_stats.onLadder)
	{
		switch (m_stats.FiringType)
		{
		case ePressing: m_stats.FiringType=eHolding; break;
		case eHolding: m_stats.FiringType=eHolding; break;
		case eReleasing: m_stats.FiringType=ePressing; break;
		case eNotFiring: m_stats.FiringType=ePressing; break;
		default: m_stats.FiringType=eNotFiring;
		}
		
	}
	else 
	{
		switch (m_stats.FiringType)
		{
		case ePressing: m_stats.FiringType=eNotFiring; break;
		case eHolding: m_stats.FiringType=eReleasing; break;
		case eReleasing: m_stats.FiringType=eNotFiring; break;
		case eNotFiring: m_stats.FiringType=eNotFiring; break;
		default: m_stats.FiringType=eNotFiring;
		}
		
	}
	m_stats.LastFiringType=lastft;
	/*if (m_stats.FiringType == PlayerStats::eFireType::ePressing)
		m_stats.FiringType = PlayerStats::eFireType::eHolding;
	else if (m_stats.FiringType == PlayerStats::eFireType::eReleasing)
		m_stats.FiringType = PlayerStats::eFireType::eNotFiring;*/
}

//////////////////////////////////////////////////////////////////////////
void CPlayer::DrawThirdPersonWeapon(bool bDraw)
{
	ICryCharInstance *character = m_pEntity->GetCharInterface()->GetCharacter(PLAYER_MODEL_IDX);
	if (character && m_nSelectedWeaponID != -1)
	{
		WeaponInfo &wi = GetWeaponInfo();

		if(bDraw)
		{
			CWeaponClass* pSelectedWeapon = GetSelectedWeapon();
			
			string sBone = pSelectedWeapon->GetBindBone();
			if (!sBone.empty())
			{
				wi.hBindInfo = character->AttachObjectToBone( pSelectedWeapon->GetObject(), sBone.c_str() );
			}
		}
		else
		{
			wi.DetachBindingHandles(character);
			character->AttachObjectToBone(NULL,NULL);
		}
	}
}
///////////////////////////////////////////////
/*! Selects a weapon the player should hold
		@param weapon id of weapon to set
*/
bool CPlayer::SelectWeapon( int weapon, bool bCheckForAvailability )
{
	PlayerWeaponsItor wi;

	// if it's shooter from the vehicle - can't select weapons
	if (m_nSelectedWeaponID>0 && m_pVehicle && m_pVehicle->GetWeaponUser()==this)
		return false;
	if (m_stats.onLadder)
		return false;

	if ((wi = m_mapPlayerWeapons.find(weapon)) == m_mapPlayerWeapons.end()  && weapon != -1) 
		return false;

	if (bCheckForAvailability && weapon != -1)
	{
		if ((* wi).second.owns == false)
			return false;
	}

	m_pEntity->SendScriptEvent(ScriptEvent_SelectWeapon,0);
	// deselect old weapon, if selected
	if (m_nSelectedWeaponID != -1 && (weapon!=m_stats.weapon))
	{
		// unbind the entity from the player
		WeaponInfo &info=GetWeaponInfo();

		info.iFireMode=m_stats.firemode;
		_SmartScriptObject cOnUpdateParam(m_pScriptSystem);
		cOnUpdateParam->SetValue("shooter", m_pEntity->GetScriptObject());
		GetSelectedWeapon()->ScriptOnDeactivate(m_pEntity);
	}

	SetWeapon(weapon);

	// call OnActivate when the weapon changes
	if (m_nSelectedWeaponID != -1 && (weapon != m_stats.weapon))
	{
		WeaponInfo &info=GetWeaponInfo();
		m_stats.firemode=info.iFireMode;
		m_bWaitForFireRelease = true;
		GetSelectedWeapon()->ScriptOnActivate(m_pEntity);

		// make sure that the firemode is correct on script side
		_SmartScriptObject pObj(m_pScriptSystem);
		pObj->SetValue( "firemode", m_stats.firemode);
		pObj->SetValue( "ignoreammo", true);

		bool bCanSwitch;
		m_pEntity->SendScriptEvent(ScriptEvent_FireModeChange, *pObj, &bCanSwitch);

		// force animation update on first person weapon
		if (GetSelectedWeapon()->GetCharacter())
			GetSelectedWeapon()->GetCharacter()->ForceUpdate();
	}
	m_stats.weapon = m_nSelectedWeaponID;
	
	return true;
}

void CPlayer::SetWeapon(int iClsID)
{
//	ValidateHeap();
	// check if we want to select the weapon we are holding
	if(iClsID == m_nSelectedWeaponID)
		return;

	// we want to hide the currently selected weapon
	if(m_nSelectedWeaponID != -1)
	{
		if (m_stats.LastFiringType!=eNotFiring)
			GetSelectedWeapon()->ScriptOnStopFiring(m_pEntity);

		SetWeaponPositionState(WEAPON_POS_UNDEFINED);

		if (IsMyPlayer())
		{
			ICryCharInstance *pChar = GetEntity()->GetCharInterface()->GetCharacter(1);
			if (pChar)
				pChar->DetachAll();

			GetEntity()->DrawCharacter(1,0);
			GetEntity()->ResetAnimations(1);
		}
	}

	// here we actually set the new weapon
	m_nSelectedWeaponID = iClsID;
	// returns the newly selected weapon!!!
	CWeaponClass* pSelectedWeapon = GetSelectedWeapon();


	if(pSelectedWeapon)
	{
		// attach to bone for third person view
		HoldWeapon();
		// set first person weapon
		if (IsMyPlayer() || m_pGame->IsMultiplayer())
			GetEntity()->GetCharInterface()->SetCharacter(1, pSelectedWeapon->GetCharacter());

		m_pScriptObject->SetValue("weapon", pSelectedWeapon->GetScriptObject());
		m_pScriptObject->SetValue("weaponid", m_nSelectedWeaponID);
	}	else	{
		GetEntity()->GetCharInterface()->SetCharacter(1, 0);
		m_pScriptObject->SetToNull("weapon");
		m_pScriptObject->SetToNull("weaponid");
		m_stats.fSpeedScale = 1.0f;
		m_nSelectedWeaponID = -1;
	}
}

/*! return the position and orientation of the firepoint of a player
	@param firePos position
	@param fireAngles orientation(in degrees)
*/
void CPlayer::GetFirePosAngles(Vec3d& firePos, Vec3d& fireAngles)
{
	if (!m_bIsAI)
	{
		//we must always take leaning into account!
		fireAngles = m_vEyeAngles+m_vShake;
		//fireAngles = m_pEntity->GetAngles()+m_vShake;
		firePos = m_vEyePos;

/* 
		if (m_pMountedWeapon)
		{
		IEntityCharacter *pIChar = m_pMountedWeapon->GetCharInterface();
		ICryCharInstance * cmodel = pIChar->GetCharacter(0);    

			if (!cmodel) 
				return;

			ICryBone * pBone = cmodel->GetBoneByName("spitfire");
			if(!pBone)
				return;

			Vec3 vBonePos = pBone->GetBonePosition();
			Vec3 angles = m_pMountedWeapon->GetAngles();
			Matrix44 m=Matrix34::GetRotationXYZ34( Deg2Rad(angles), m_pMountedWeapon->GetPos() );	//set rotation and translation in one function call
			m	=	GetTransposed44(m); //TODO: remove this after E3 and use Matrix34 instead of Matrix44
			// get result
			firePos = m.TransformPointOLD(vBonePos);
		}
		else 
*/
		if (m_pVehicle)
		{
		// if we are in vehicle, the cam offset seems to be wrong -
		// move to the exact camera position
		//[kirill] - coz camera is offseted a bit for gunner - away from weapon
		//	if gunner's camera moved - add same offset here - now it's eyePos
//			firePos=m_pEntity->GetCamera()->GetPos();
			// if driver - using vehicle's autoveapon
			// - just get current weapon pos/angles from the vehicle
//			if (m_stats.inVehicleState==PVS_DRIVER && m_pVehicle->GetWeaponUser()==this )
			if (m_stats.inVehicleState==PVS_DRIVER )
			{
				string vehicleWeaponName = m_pVehicle->GetWeaponName( m_stats.inVehicleState );
				if(!vehicleWeaponName.empty() && vehicleWeaponName != m_pVehicle->m_sNoWeaponName)
					m_pVehicle->GetFirePosAngles( firePos, fireAngles );
				else if(IsMyPlayer())	// use vehicle camera position only for local player
					firePos = m_pVehicle->GetCamPos();
				else
					firePos += Vec3(0.0f, 0.0f, -1.04f); // UGLY FIX FOR THE INFLATABLE BOAT!
																						 // firePos on the server was 0.6 meters less than the client
			}
			// if passenger - get camera position from vehicle, it's smoothed with some spring simulation
			else if (m_stats.inVehicleState == PVS_PASSENGER)
			{
				if(IsMyPlayer())	// use vehicle camera position only for local player
					firePos = m_pVehicle->GetCamPos();
				else				// in MP on server use eye helper position
					m_pVehicle->GetEntity()->GetHelperPosition(m_sVehicleEyeHelper.c_str(), firePos);
			}
			
			return;
		}
	}
	else
	{
		if (m_pEntity->GetAI())
		{
			//<<FIXME>> this code will go - testing purposes
			firePos = m_pEntity->GetAI()->GetPos();
//			Vec3d angles = m_pEntity->GetAI()->GetState()->vFireDir;
//			angles=ConvertVectorToCameraAngles(angles);
			if(!m_pMountedWeapon)	// normal weapon - shoot wherever
				fireAngles = m_vDEBUGAIFIREANGLES;
//				fireAngles = angles;
			else					// if using mounted weapon - angles are restricted
				fireAngles = m_pEntity->GetAngles();
		}
	}
}

float CPlayer::CalculateAccuracyFactor(float accuracy)
{
	if(m_nSelectedWeaponID == -1) return 0.0f;

	CWeaponClass *pSelectedWeapon = GetSelectedWeapon();
	WeaponParams wp;

	GetCurrentWeaponParams(wp);

	float factor = (wp.fMaxAccuracy - wp.fMinAccuracy) * (1.0f-accuracy) + wp.fMinAccuracy;

	// factor 0 = bad, 1 = good
	if (m_stats.aiming)
	{
		factor = factor + wp.fAimImprovement;
	}

	// apply sprinting if the sprint button is pressed and the player is moving
	if (m_Sprinting && m_stats.moving)
	{
		factor = factor - wp.fSprintPenalty;
	}

	factor = 1 - factor;
	factor = __max(factor, 0.000f);

	float stanceAccuracyModifier = 1.0f;

	// process weapon specific accuracy modifier based on stance
	stanceAccuracyModifier = wp.fAccuracyModifierStanding;
	if (m_CurStance == eCrouch)
	{
		stanceAccuracyModifier = wp.fAccuracyModifierCrouch;
	}
	else if (m_CurStance == eProne)
	{
		stanceAccuracyModifier = wp.fAccuracyModifierProne;
	}

	return factor * stanceAccuracyModifier;
}

void CPlayer::UpdateMelee()
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );
	//MELEE ATTACK/////////////////////////////////////
	if(m_stats.melee_attack && (m_stats.weapon_busy<=0) && (m_stats.melee_distance > 0.0f))
	{
		//TRACE("UATTATATATATATATATA!");
		_SmartScriptObject so(m_pScriptSystem);
		Vec3d firePos;
		Vec3d fireAngles;

		GetFirePosAngles(firePos, fireAngles);

		Vec3d dir(0,-1,0);
		Matrix44 tm = Matrix44::CreateRotationZYX(-fireAngles*gf_DEGTORAD); //NOTE: angles in radians and negated ;

		dir = GetTransposed44(tm)*(dir);

		m_ssoHitPosVec = firePos;
		m_ssoHitDirVec = dir;

		so->BeginSetGetChain();
		so->SetValueChain("pos",m_ssoHitPosVec);
		so->SetValueChain("dir",m_ssoHitDirVec);
		so->SetValueChain("distance", m_stats.melee_distance);
		so->SetValueChain("shooter",m_pEntity->GetScriptObject());

		_SmartScriptObject soTarget(m_pScriptSystem,true);
		if (GetScriptObject()->GetValue("melee_target", soTarget))
		{
			so->SetValueChain("melee_target", *soTarget);
		}
		so->EndSetGetChain();

		m_pEntity->SendScriptEvent(ScriptEvent_MeleeAttack,so);
		m_stats.melee_attack = false;
		m_pEntity->GetScriptObject()->SetToNull("melee_target");
	}
}
///////////////////////////////////////////////
/*! Updates the selected weapon
*/
void CPlayer::UpdateWeapon()
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );
	
	float frametime=m_pTimer->GetFrameTime();

	if(m_stats.firing_grenade)
	{
		FRAME_PROFILER( "UpdateWeapon::Grenade",GetISystem(),PROFILE_GAME );
		Vec3d firePos;
		Vec3d fireAngles;
		GetFirePosAngles(firePos, fireAngles);

		if(m_stats.weapon_busy<=0)
			FireGrenade(firePos, fireAngles, m_pEntity);	

		if (!GetGame()->IsMultiplayer())
			m_stats.firing_grenade = false;
	}

		
	if(m_nSelectedWeaponID == -1) return;

	WeaponParams wp;
	CWeaponClass* pSelectedWeapon = GetSelectedWeapon();

	GetCurrentWeaponParams(wp);
	m_fAccuracyMod=1;
	if(m_pGame->IsServer())
	{
		// make sure that the AI is always using the right firemode
		if(m_bIsAI)
		{
			FRAME_PROFILER( "UpdateWeapon::SwitchFiremode",GetISystem(),PROFILE_GAME );
			SwitchFiremode(pSelectedWeapon->GetAIFireMode());
		}

		if(m_stats.firing){
			m_fAccuracy=m_fAccuracy-(frametime);
		}else{
			m_fAccuracy=m_fAccuracy-(m_pGame->w_accuracy_gain_scale*frametime);
		}
		m_fAccuracy=__max(m_fAccuracy, 0.1f);

		if(!m_pMountedWeapon && !m_pVehicle)
		{
			if(m_stats.running)
			{
				if(m_fAccuracy<wp.accuracy_decay_on_run)
				{
					m_fAccuracy=min(wp.accuracy_decay_on_run,m_fAccuracy+(frametime*6));
				} 
			}
			else if(m_stats.moving)
			{
				if(m_fAccuracy<(wp.accuracy_decay_on_run))
				{
					m_fAccuracy=min(wp.accuracy_decay_on_run,m_fAccuracy+(frametime*3));
				}
			}
		}
		if(m_stats.crouch || m_stats.prone){
			m_fAccuracyMod-=0.2f;
		}

		if(m_stats.aiming)
		{
			m_fAccuracyMod-=0.3f;
		}

		//TRACE("m_stats.accuracy=%f",m_stats.accuracy);
		//clamp accuracy to 1 byte
		BYTE acc=(BYTE)((m_fAccuracy*m_fAccuracyMod)/(1.f/255));
		m_stats.accuracy=(float)((acc)*(1.f/255));
	}

	WeaponInfo &wi=GetWeaponInfo();

	if(m_stats.weapon_busy>0)
	{
		m_stats.firing=false;
	}
	else{
		if(m_stats.weapon_busy!=0)
		{
			FRAME_PROFILER( "UpdateWeapon::ScriptWeaponReady",GetISystem(),PROFILE_GAME );
			wi.fireTime=m_pTimer->GetCurrTime();
			//wi->fireFirstBulletTime=0;
			pSelectedWeapon->ScriptWeaponReady(m_pEntity);
		}
		m_stats.weapon_busy=0;
	}

	
//Update the weapon entity
	if (IsMyPlayer() || (m_pGame->cl_scope_flare->GetIVal() == 1 && m_weaponPositionState == WEAPON_POS_HOLD ))
		pSelectedWeapon->Update(this);

	if (m_stats.cancelFireFlag)
		pSelectedWeapon->CancelFire();

	if((m_stats.ammo_in_clip<=0 && m_stats.ammo>0 && (!m_stats.reloading) && (m_stats.weapon_busy<=0)) && (!m_stats.firing) &&
		 (m_stats.underwater==0.0f) && !m_bSwimming)
	{
		pSelectedWeapon->ScriptOnStopFiring(m_pEntity);
		pSelectedWeapon->ScriptReload(m_pEntity);
		wi.fireTime=m_pTimer->GetCurrTime();
	}
	else if(!m_stats.reloading && m_stats.weapon_busy<=0)
	{
		if(m_stats.ammo_in_clip>0 || wp.no_ammo)
		{
			if ((m_pTimer->GetCurrTime() - wi.fireLastShot) < pSelectedWeapon->GetFireRate(m_stats.FiringType))
				m_stats.fire_rate_busy = true;
			else
				m_stats.fire_rate_busy = false;

			m_stats.canfire = true;
		}
		else
		{	
			if(m_stats.canfire)
				pSelectedWeapon->ScriptOnStopFiring(m_pEntity);

			m_stats.canfire=false;
		}

    SetFiring(m_stats.firing);
		
		if (m_stats.FiringType != eNotFiring)
		{
			float fFireRate = pSelectedWeapon->GetFireRate(m_stats.FiringType);
			//is in the right state for firing
			if( wp.fire_activation&m_stats.FiringType)
			{
				Vec3d firePos;
				Vec3d fireAngles;

				GetFirePosAngles(firePos, fireAngles);

				if(!(m_stats.LastFiringType&wp.fire_activation))
				{
					wi.fireTime=m_pTimer->GetCurrTime();
					wi.fireFirstBulletTime=wi.fireTime;
					m_fRecoilXDelta=0;		
				}
				float deltatime=max(0.001f,m_pTimer->GetCurrTime()-wi.fireFirstBulletTime);
				
				int bullets=0;
				//[kirill] - can't shoot if using mounted vehicle weapon and weapon is off of angle limits
				if (m_pGame->m_nDEBUG_TIMING==1)
				{
					m_pGame->m_nDEBUG_TIMING = 2;
					//[Michael Glueck] has nothing to do with AI
					if(m_bIsAI)
					{
						float fTime = m_pGame->GetSystem()->GetITimer()->GetAsyncCurTime();
						m_pGame->m_pLog->Log(" Time between loading and having AI shooting is %.3f seconds",fTime-m_pGame->m_fDEBUG_STARTTIMER);
					}
				}
				if(m_stats.crosshairOnScreen)
					if(bullets = pSelectedWeapon->Fire( firePos,fireAngles, this, wi, GetRedirected() ? GetRedirected()->GetPhysics() : NULL))
				{
					FRAME_PROFILER( "UpdateWeapon::ScriptEvent_Fire",GetISystem(),PROFILE_GAME );

					if(m_pMountedWeapon)	// start fire animation on mounted weapon
						m_pMountedWeapon->SendScriptEvent(ScriptEvent_Fire,1);
					else if( m_pVehicle )
					{
						WeaponInfo &wi = GetWeaponInfo();
						m_pVehicle->WeaponState( m_pEntity->GetId(), true, wi.iFireMode);
					}

					if(m_pGame->IsServer())
					{
						m_stats.last_accuracy=(BYTE)(m_stats.accuracy/(1.f/255));
						m_fAccuracy=min(1,m_fAccuracy+((deltatime*(m_pGame->w_accuracy_decay_speed*fFireRate)*bullets)));
						BYTE acc=(BYTE)((m_fAccuracy*m_fAccuracyMod)/(1.f/255));
						m_stats.accuracy=(float)((acc)*(1.f/255));	
					}
					
					//add recoil			
					if(!IsAI())	//IsMyPlayer())				// every player should have recoil
					{
						//pseudo realistic recoil
						if((m_fRecoilXDelta+1)<m_pGame->w_recoil_max_degree)
						{
								float delta=min(1, (m_pTimer->GetCurrTime()-wi.fireFirstBulletTime)+wp.fFireRate);
								float recoilx=wp.max_recoil*((delta*2)*2);
								float recoilz=recoilx;

								uint8 ucSeed = m_SynchedRandomSeed.GetRandomSeedC();

								m_SynchedRandomSeed.IncreaseRandomSeedC();

//								GetISystem()->GetILog()->Log(">> Recoil                           Recoil %d %d",(int)GetEntity()->GetId(),(int)(ucSeed));			// debug

								float fRandA = m_SynchedRandomSeed.GetRandTable(ucSeed);
								float fRandB = m_SynchedRandomSeed.GetRandTable(ucSeed+13);			
								
								m_fRecoilXUp=m_fRecoilX=(min(wp.max_recoil,wp.min_recoil+recoilx+(recoilx*fRandA)))*bullets;
								m_fRecoilZUp=m_fRecoilZ=((m_fRecoilXUp)-(m_fRecoilXUp*fRandA)*2)*bullets;
								if (m_stats.aiming)
								{
									m_fRecoilX *= wp.fAimRecoilModifier;
									m_fRecoilXUp *= wp.fAimRecoilModifier;
									m_fRecoilZ *= wp.fAimRecoilModifier;
									m_fRecoilZUp *= wp.fAimRecoilModifier;
								}
						}
						else
						{
								//m_fRecoilXUp=0;
						}
					}
				}
			}
			//
		}
		else if(m_stats.LastFiringType!=eNotFiring)
		{
			FRAME_PROFILER( "UpdateWeapon::ScriptOnStopFiring",GetISystem(),PROFILE_GAME );
			//was firing and now stoppped
			//sending an event for stooping various effects
			wi.fireFirstBulletTime=0;
			pSelectedWeapon->ScriptOnStopFiring(m_pEntity);

			if(m_pMountedWeapon)	// stop fire animation on mounted weapon
				m_pMountedWeapon->SendScriptEvent(ScriptEvent_Fire,0);
			else if( m_pVehicle )	// stop fire animation on weapon on wehicle
				m_pVehicle->WeaponState( m_pEntity->GetId(), false );
		}
	}
}

// returns ACTUAL plaerys direction angle - the one it's shooting/looking at
Vec3d CPlayer::GetActualAngles()
{
	Vec3d angles; 
	
	if( m_pGame->p_EyeFire->GetIVal()==0)
	{
		if(m_aimLook)
		{
			Vec3d pos;
			GetFirePosAngles( pos, angles );
		}
		else
			angles = m_pEntity->GetAngles();
	}
	else
		angles = m_pEntity->GetAngles();

	angles.x = Snap_s180(angles.x);
	angles.y = Snap_s180(angles.y);
	angles.z = Snap_s180(angles.z);
	return angles;
}


///////////////////////////////////////////////
/*! Updates procedural animation of character
*/
void CPlayer::UpdateBonesRotation( )
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );
//m_pGame->m_pLog->LogToConsole("\003 %.2f %.2f ",m_LegAngle,m_LegDeltaAngle);

	//also when on ladder we have to rotate some bones (head)
	if (m_LegRotation || m_stats.onLadder)
	{
		// apply lean angles to player character	
		ICryCharInstance *pChar = m_pEntity->GetCharInterface()->GetCharacter(0);
		Vec3d angles = GetActualAngles();

		if (pChar)
		{
		CryQuat	qtH;
		CryQuat	qtV;
		CryQuat	qtR;
		CryQuat	qtTotal;
		CryQuat	qtParent;
		CryQuat	qtParentCnj;
		float	xAngle = -angles.x;
//		float	xAngle = Snap180(-angles.x);
/*
			if( xAngle>180 )
				xAngle = xAngle - 360.0f;
			else if( xAngle<-180 )
				xAngle = 360.0f+xAngle;
*/
//		float leanBodyAngle = -(180.0f - (180 - m_walkParams.fCurrLean*m_pGame->p_lean->GetFVal()*2.0f));
		float leanBodyAngle = -(180.0f - (180 - m_walkParams.fCurrLean*m_LeanDegree*2.0f));
		//					leanBodyAngle = 0;
			leanBodyAngle *= 2.0f;
//			leanBodyAngle *= 5.0f;
	
//		float	zAngle = m_LegDeltaAngle + m_LegMovingAngle;
		float	zAngle = m_LegDeltaAngle;

/*
if(m_pBoneSpine1)
	qtParent = m_pBoneSpine1->GetParentWQuat();

	qtParentCnj = qtParent;
	qtParentCnj.w = -qtParentCnj.w;


qtH.SetRotationAA( DEG2RAD(m_LegDeltaAngle),	Vec3(0.0f, 0.0f, 1.0f) );
//qtH.SetRotationAA( DEG2RAD(45),	Vec3(0.0f, 0.0f, 1.0f) );
qtV.SetRotationAA( DEG2RAD(xAngle),						Vec3(1.0f, 0.0f, 0.0f) );
qtR.SetRotationAA( DEG2RAD(leanBodyAngle),		Vec3(0.0f, 1.0f, 0.0f) );
qtTotal = qtR*qtV*qtH;

qtTotal = qtParent*qtR*qtV*qtH*qtParentCnj;


if(m_pBoneSpine1)
	m_pBoneSpine1->SetPlusRotation( qtTotal );
return;
//*/

			//on ladder we rotate just the head bone to follow the player view.
			if (m_stats.onLadder)
			{
				if(m_pBoneNeck)
					m_pBoneNeck->ResetPlusRotation();
				if(m_pBoneSpine1)
					m_pBoneSpine1->ResetPlusRotation();
				if(m_pBoneSpine2)
					m_pBoneSpine2->ResetPlusRotation();

				if (m_stats.onLadder)
				{
					zAngle = Snap_s180(m_vLadderAngles.z - m_pEntity->GetAngles().z);
					//m_pGame->GetSystem()->GetILog()->Log("head:%.1f,body:%.1f",zAngle,m_vLadderAngles.z);
				}

				//limit the head angle in a 120 degree range.
				if (zAngle > 60.0f)
					zAngle = 60.0f;

				if (zAngle < -60.0f)
					zAngle = -60.0f;

				//smooth the head rotation.
				Ang3 delta = Ang3(xAngle*0.5f,0,zAngle) - m_vHeadAngles;
				m_vHeadAngles = m_vHeadAngles + delta*min(1.0f,m_pTimer->GetFrameTime()*10.0f);

				m_vHeadAngles.Snap180();

				//apply rotation
				qtH.SetRotationAA( DEG2RAD(m_vHeadAngles.z), Vec3(0.0f, 0.0f, 1.0f) );
				qtV.SetRotationAA( DEG2RAD(m_vHeadAngles.x), Vec3(1.0f, 0.0f, 0.0f) );

				qtParent = m_pBoneHead->GetParentWQuat();
				qtParentCnj = qtParent;
				qtParentCnj.w = -qtParentCnj.w;
				qtTotal = qtParent*qtV*qtH*qtParentCnj;

				m_pBoneHead->SetPlusRotation( qtTotal );
			}
			else if(m_CurStance == eProne)
			{
				m_LegADeltaLimit = 0;
				if(m_pBoneHead)
					m_pBoneHead->SetPlusRotation(0,0,angles.x);
				if(m_pBoneNeck)
					m_pBoneNeck->ResetPlusRotation();
				if(m_pBoneSpine1)
					m_pBoneSpine1->ResetPlusRotation();
				if(m_pBoneSpine2)
					m_pBoneSpine2->ResetPlusRotation();
			}
//			else if(m_aimLook)	//aiming
			else if(m_CurStance == eCrouch || m_CurStance == eStealth || m_CurStance == eStand)	
			{
			//force weapon (shoulders) be same direction as ent direstion
				//m_LegADeltaLimit = 60;
				m_LegADeltaLimit = m_pGame->pa_leg_limitaim->GetFVal();// 60;
				if(m_pBoneHead)
					m_pBoneHead->ResetPlusRotation();
				if(m_pBoneNeck)
					m_pBoneNeck->ResetPlusRotation();

				float	amountSpine = m_pGame->pa_spine->GetFVal();
				float	amountSpine1 = m_pGame->pa_spine1->GetFVal();
				float	amountSpine2;

				amountSpine2 = 1.0f - (amountSpine1 + amountSpine);

				if(m_pBoneSpine)
				{
					qtH.SetRotationAA( DEG2RAD(zAngle*amountSpine), Vec3(0.0f,0.0f,1.0f) );
					qtV.SetRotationAA( DEG2RAD(xAngle*.2f),         Vec3(1.0f,0.0f,0.0f) );
					qtR.SetRotationAA( DEG2RAD(leanBodyAngle*.2f),  Vec3(0.0f,1.0f,0.0f) );
//					qtTotal = qtR*qtV*qtH;
					qtParent = m_pBoneSpine->GetParentWQuat();
					qtParentCnj = qtParent;
					qtParentCnj.w = -qtParentCnj.w;
					qtTotal = qtParent*qtR*qtV*qtH*qtParentCnj;
					m_pBoneSpine->SetPlusRotation( qtTotal );
				}
				if(m_pBoneSpine1)
				{
					qtH.SetRotationAA( DEG2RAD(zAngle*amountSpine1), Vec3(0.0f,0.0f,1.0f) );
					qtV.SetRotationAA( DEG2RAD(xAngle*.4f),          Vec3(1.0f,0.0f,0.0f) );
					qtR.SetRotationAA( DEG2RAD(leanBodyAngle*.4f),   Vec3(0.0f,1.0f,0.0f) );
//					qtTotal = qtR*qtV*qtH;
					qtParent = m_pBoneSpine1->GetParentWQuat();
					qtParentCnj = qtParent;
					qtParentCnj.w = -qtParentCnj.w;
					qtTotal = qtParent*qtR*qtV*qtH*qtParentCnj;
					m_pBoneSpine1->SetPlusRotation( qtTotal );
				}
				if(m_pBoneSpine2)
				{
					qtH.SetRotationAA( DEG2RAD(zAngle*amountSpine2), Vec3(0.0f,0.0f,1.0f) );
					qtV.SetRotationAA( DEG2RAD(xAngle*.4f),          Vec3(1.0f,0.0f,0.0f) );
					qtR.SetRotationAA( DEG2RAD(leanBodyAngle*.4f),   Vec3(0.0f,1.0f,0.0f) );
//					qtTotal = qtR*qtV*qtH;
					qtParent = m_pBoneSpine2->GetParentWQuat();
					qtParentCnj = qtParent;
					qtParentCnj.w = -qtParentCnj.w;
					qtTotal = qtParent*qtR*qtV*qtH*qtParentCnj;
					m_pBoneSpine2->SetPlusRotation( qtTotal );
				}
			}
			else 
			{
				//m_LegADeltaLimit = 110;
				m_LegADeltaLimit = m_pGame->pa_leg_limitidle->GetFVal();
				if(m_pBoneHead)
				{
					qtH.SetRotationAA( DEG2RAD(zAngle*.3f), Vec3(0.0f, 0.0f, 1.0f) );
					qtV.SetRotationAA( DEG2RAD(xAngle*.5f), Vec3(1.0f, 0.0f, 0.0f) );
//					qtTotal = qtV*qtH;
					qtParent = m_pBoneHead->GetParentWQuat();
					qtParentCnj = qtParent;
					qtParentCnj.w = -qtParentCnj.w;
					qtTotal = qtParent*qtV*qtH*qtParentCnj;
					m_pBoneHead->SetPlusRotation( qtTotal );
				}
				if(m_pBoneNeck)
				{
					qtH.SetRotationAA( DEG2RAD(zAngle*.3f), Vec3(0.0f, 0.0f, 1.0f) );
					qtV.SetRotationAA( DEG2RAD(xAngle*.2f), Vec3(1.0f, 0.0f, 0.0f) );
//					qtTotal = qtV*qtH;
					qtParent = m_pBoneNeck->GetParentWQuat();
					qtParentCnj = qtParent;
					qtParentCnj.w = -qtParentCnj.w;
					qtTotal = qtParent*qtV*qtH*qtParentCnj;
					m_pBoneNeck->SetPlusRotation( qtTotal );
				}
				if(m_pBoneSpine)
				{
					qtH.SetRotationAA( DEG2RAD(zAngle*.1f),        Vec3(0.0f, 0.0f, 1.0f) );
					qtV.SetRotationAA( DEG2RAD(xAngle*.1f),        Vec3(1.0f, 0.0f, 0.0f) );
					qtR.SetRotationAA( DEG2RAD(leanBodyAngle*.1f), Vec3(0.0f, 1.0f, 0.0f) );
//					qtTotal = qtR*qtV*qtH;
					qtParent = m_pBoneSpine->GetParentWQuat();
					qtParentCnj = qtParent;
					qtParentCnj.w = -qtParentCnj.w;
					qtTotal = qtParent*qtR*qtV*qtH*qtParentCnj;
					m_pBoneSpine->SetPlusRotation( qtTotal );
				}
				if(m_pBoneSpine1)
				{
					qtH.SetRotationAA( DEG2RAD(zAngle*.1f),        Vec3(0.0f, 0.0f, 1.0f) );
					qtV.SetRotationAA( DEG2RAD(xAngle*.1f),        Vec3(1.0f, 0.0f, 0.0f) );
					qtR.SetRotationAA( DEG2RAD(leanBodyAngle*.5f), Vec3(0.0f, 1.0f, 0.0f) );
//					qtTotal = qtR*qtV*qtH;
					qtParent = m_pBoneSpine1->GetParentWQuat();
					qtParentCnj = qtParent;
					qtParentCnj.w = -qtParentCnj.w;
					qtTotal = qtParent*qtR*qtV*qtH*qtParentCnj;
					m_pBoneSpine1->SetPlusRotation( qtTotal );
				}
				if(m_pBoneSpine2)
				{
					qtH.SetRotationAA( DEG2RAD(zAngle*.2f),        Vec3(0.0f, 0.0f, 1.0f) );
					qtV.SetRotationAA( DEG2RAD(xAngle*.1f),        Vec3(1.0f, 0.0f, 0.0f) );
					qtR.SetRotationAA( DEG2RAD(leanBodyAngle*.4f), Vec3(0.0f, 1.0f, 0.0f) );
//					qtTotal = qtR*qtV*qtH;
					qtParent = m_pBoneSpine2->GetParentWQuat();
					qtParentCnj = qtParent;
					qtParentCnj.w = -qtParentCnj.w;
					qtTotal = qtParent*qtR*qtV*qtH*qtParentCnj;
					m_pBoneSpine2->SetPlusRotation( qtTotal );
				}
	//m_pBoneSpine1->SetPlusRotation(-m_LegDeltaAngle*.10f, -leanBodyAngle*.5f,
	//																	-xAngle*.1f);//GetAngleDifference360(0, -angles.x)*.5f);
			}
		}
	}
	else
	{
		ResetRotateHead();
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/*! 
*/
void CPlayer::ResetRotateHead()
{
		if(m_pBoneHead)
			m_pBoneHead->ResetPlusRotation();
		if(m_pBoneNeck)
			m_pBoneNeck->ResetPlusRotation();
		if(m_pBoneSpine)
			m_pBoneSpine->ResetPlusRotation();
		if(m_pBoneSpine1)
			m_pBoneSpine1->ResetPlusRotation();
		if(m_pBoneSpine2)
			m_pBoneSpine2->ResetPlusRotation();

}

///////////////////////////////////////////////
///////////////////////////////////////////////
/*! 
*/
void CPlayer::UpdateFireAnimations()
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	//fixme - when dead - needs different update
	if(!IsAlive())
		return;

	if( m_pGame->pa_blend1->GetFVal()<0.0f )
		return;

	// stop fire animation when
	// in relaxed stance
	// swimming
	// on ladder
	if(!m_AnimationSystemEnabled || m_CurStance == eRelaxed || m_bSwimming || m_stats.onLadder)	
	{
	//	m_pEntity->GetCurrentAnimation(0, 1)
		m_sPrevAniNameLayer1.clear();
		m_pEntity->StartAnimation(0, NULL, 1, .15f);
		return;
	}


	const char *weaponShoot = "_utshoot";		// two hands animations
	const char *weaponAim = "_utaim";
	char aniName[64];
	if (!GenerateAnimationName( aniName ))
	{
		return;
	}

	char aniName0[64];	// store the base layer0 animation name
	memcpy( aniName0, aniName, strlen(aniName) );


	// fixme - has to be changed for multyplayer
	//m_stats.weapon_busy<=0
	//check for melee attack/reloading/whatever

	CWeaponClass *pSelectedWeapon = GetSelectedWeapon();

	if(m_JumpStage==1)
	{
		m_pEntity->StartAnimation(0, NULL, 1, .15f);
//m_pEntity->StartAnimation(0, "aidle", 1, .15f);
		m_sPrevAniNameLayer1 = "";
		return;
	}
	if(m_JumpStage!=0)
		return;

	if(!pSelectedWeapon )								// no weapon in hands - use machete animations
	{
		weaponShoot = "_umshoot";
		weaponAim = "_umaim";
	}
	else if(pSelectedWeapon->m_HoldingType == 2)		// pistol animations
	{
		weaponShoot = "_upshoot";
		weaponAim = "_upaim";
	}
	else if(pSelectedWeapon->m_HoldingType == 3)		// machete animations
	{
		weaponShoot = "_umshoot";
		weaponAim = "_umaim";
	}
	else if(pSelectedWeapon->m_HoldingType == 4)		// building tool animations
	{
		weaponShoot = "_ubshoot";
		weaponAim = "_ubaim";
	}
	else if(pSelectedWeapon->m_HoldingType == 5)		// scouttool/healthpack animations
	{
		weaponShoot = "_usshoot";
		weaponAim = "_usaim";
	}

	if ( m_stats.firing_grenade)
		m_bGrenadeAnimation = true;


	if ( m_bGrenadeAnimation )
	{
		if(m_stats.weapon_busy<=0)
			m_bGrenadeAnimation = false;
		else
		{
		const char *grenadeAim = "grenade_mp";
			if(m_sPrevAniNameLayer1 != grenadeAim)
				if(m_pEntity->StartAnimation(0, grenadeAim, 1, .2f, true))	//.2f
					m_sPrevAniNameLayer1 = grenadeAim;
		}
	}
	else 
	if( m_aimLook && m_stats.canfire )
	{

		m_fShootAniLength -= m_pTimer->GetFrameTime();

		if(m_bWeaponJustFired)			// do fire animation
		{
			m_bWeaponJustFired = false;
			strcat( aniName,weaponShoot );
			m_fShootAniLength = m_pEntity->GetAnimationLength( aniName );
		}
		else if( m_fShootAniLength<=0 )	// fire animation over - do aim animation
		{
			strcat( aniName,weaponAim );
		}
		else	
		{
			// still playing fire animation 
			strcat( aniName,weaponShoot );
			//return;
		}

		if(m_sPrevAniNameLayer1 == aniName)
			return;

//		float	blendTime = m_pGame->pa_blend1->GetFVal();
//		GetBlendTime( aniName, blendTime );

		// blend time has to be the same as for base animation in layer 0 
//		float	blendTime = m_pGame->pa_blend1->GetFVal();
//		GetBlendTime( aniName0, blendTime );

		float	blendTime = m_pGame->pa_blend0->GetFVal();

		// if animation is shorter than blend time - it blands to base animation ( which is bad )
		float	aniLenght = m_pEntity->GetAnimationLength(aniName);
		if( blendTime>aniLenght )
			blendTime = aniLenght;	//0;


		if(m_pEntity->StartAnimation(0, aniName, 1, blendTime, true))	//.2f
			m_sPrevAniNameLayer1 = aniName;
	}
	else	// not shooting/aiming - weapon down animation
	{
		if(pSelectedWeapon && pSelectedWeapon->m_HoldingType == 2)		// pistol animations
		{
			strcat( aniName,"_upidle" );
			//  PETAR no animations for all stances - temporary HACK
			strcpy(aniName,"aidle_upidle");
			//--------------------
			if(m_sPrevAniNameLayer1 == aniName)
				return;
//			float	blendTime = m_pGame->pa_blend1->GetFVal();
//			GetBlendTime( aniName, blendTime );	

			// blend time has to be the same as for base animation in layer 0 
			float	blendTime = m_pGame->pa_blend0->GetFVal();
			GetBlendTime( aniName0, blendTime );

			// if animation is shorter than blend time - it blands to base animation ( which is bad )
			float	aniLenght = m_pEntity->GetAnimationLength(aniName);
			if( blendTime>aniLenght )
				blendTime = aniLenght;	//0;
			// try to start pistol idle for current stance/movement
			if(m_pEntity->StartAnimation(0, aniName, 1, blendTime, true))	
			{
				m_sPrevAniNameLayer1 = aniName;
				return;
			}
			strcpy( aniName,"aidle_upidle" );
			if(m_sPrevAniNameLayer1 == aniName)
				return;
			// if animation is shorter than blend time - it blands to base animation ( which is bad )
			aniLenght = m_pEntity->GetAnimationLength(aniName);
			if( blendTime>aniLenght )
				blendTime = aniLenght;	//0;

			// use aim aidle stance
			if(m_pEntity->StartAnimation(0, aniName, 1, blendTime, true))	
			{
				m_sPrevAniNameLayer1 = aniName;
				return;
			}
		}
		if(m_sPrevAniNameLayer1.empty())
			return;
		m_pEntity->StartAnimation(0, NULL, 1, .15f);
		m_sPrevAniNameLayer1 = "";
		m_fShootAniLength = 0.0f;
	}
}


///////////////////////////////////////////////
///////////////////////////////////////////////
/*! 
*/
void CPlayer::UpdateJumpAnimations()
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	if( m_pGame->pa_blend2->GetFVal()<0.0f )
		return;
	if(!m_AnimationSystemEnabled)
		return;

	const char *aniName;

//	if(m_FlyTime<.15f && !m_stats.landing)
	if( m_FlyTime<.35f )
	{
		if( m_sPrevAniNameLayer2 == "jump_air" )
		{
			m_pEntity->StartAnimation(0, NULL, 2, .1f);
			m_sPrevAniNameLayer2.clear();
		}
		return;
	}

	if( m_stats.flying )
		aniName = "jump_air";
	if(m_stats.landing)
		aniName = "jump_land";

	if(m_pEntity->GetCurrentAnimation(0, 2)<0)
		m_sPrevAniNameLayer2.clear();
	if (strlen(aniName) == 0 || m_sPrevAniNameLayer2 == aniName )
		return;

	float blendTime = m_pGame->pa_blend2->GetFVal();
	GetBlendTime( aniName, blendTime );
	if(m_pEntity->StartAnimation(0, aniName, 2, blendTime))
		m_sPrevAniNameLayer2 = aniName;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
/*! Updates walking-flags for the player so the correct animation can be played back 

*/
void CPlayer::UpdateCharacterAnimations( SPlayerUpdateContext &ctx )
{
	if(!IsAlive())	// if dead - deadBodyPhysics takes over
		return;

	if(m_pMountedWeapon)
	{
		UpdateCharacterAnimationsMounted( ctx );
		return;
	}
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	//float		MIN_WALK_SPEED				= m_WalkSpeed*.15f;
//	float		LEFT_RIGHT_COS	= .8f;
//	float		LEFT_RIGHT_COS_TRH	= 0.91f;
	float		MIN_CROUCH_SPEED			= m_CrouchSpeed*.17f;
	float		MIN_PRONE_SPEED				= m_ProneSpeed*.17f;

	//bool moving = false;
	bool flying = false;
	//bool running = false;
	bool forward = false;

	// New variables for script based version
	int nStanding = 0;	// 0 = Standing( combat ), 1 = Crouching, 2 = Prone, 3 = Stealth, 4 = Standing( relaxed )
	int nStrafe = 0;
	int nForward = 0;
	int nMode = 0;			// 0 = Idle, 1 = Walking, 2 = Running, 3 = Jump, 4 = Flying, 6 = Turning, 7 = sprinting

	// Create a new status object.  The fields are initialized for us
	pe_status_living &status = ctx.status;

	// Get a pointer to the physics engine
	IPhysicalEntity *physEnt = m_pEntity->GetPhysics();
	PhysicsVars *physvars = m_pGame->GetSystem()->GetIPhysicalWorld()->GetPhysVars();

	m_stats.landing = false;

	// If the physics says the player is flying, set the flag
	if (status.bFlying)
	{
		flying = true;
		m_FlyTime += m_pTimer->GetFrameTime();
		m_LandTime = 0;

		// Calculate our current Vertical speed - to use for falling damage
		Vec3 vel = status.vel;
		if (status.pGroundCollider)
			vel -= status.velGround;
		m_walkParams.vertSpeedFalling = vel.z;
	}
	else
	{
		// If we were flying previously, but have stopped, set the flag
		if (m_stats.flying)
		{
			if(m_FlyTime>.5f)	// if more than .5 sec in air - play land animation
				m_stats.landing = true;

//			m_pEntity->SendScriptEvent(ScriptEvent_AnimationKey,1);

			if(m_Dynamics.gravity!=0.0f)			// in 0 gravity landing is not possible
				if(m_walkParams.vertSpeedFalling<0.0f )	// falling down
					m_pEntity->SendScriptEvent(ScriptEvent_Land, (int)(-m_walkParams.vertSpeedFalling*100));
		}
		m_FlyTime = 0;
		m_LandTime += m_pTimer->GetFrameTime();
	}

	// Store the current flying and landing states
	m_stats.flying = flying;

	// [Anton] a piece of code moved to UpdatePhysics

	// here goes stance determination -stuff
	// Check if the player is stealth
	if (m_CurStance == eStealth)
	{
		nStanding = 3;

		
		// Check if the player is moving
		// This is true if the player is running, moving left, moving right, or moving at all
		if (m_stats.fVel > MIN_CROUCH_SPEED)
		{
			if (m_stats.running)
				nMode = 2;
			else
				nMode = 1;
		}
		
		
	}
	// Check if the player is crouching
	else if (m_CurStance == eCrouch)
	{
		nStanding = 1;

		// Check if the player is moving
		// This is true if the player is running, moving left, moving right, or moving at all
		if (m_stats.fVel > MIN_CROUCH_SPEED)
		{
			// Crouching movement.
			nMode = 1;
		}
	}
	else if(m_CurStance == eProne)
	{
		if (m_stats.fVel > MIN_PRONE_SPEED)
			nMode = 1;

		nStanding = 2;
	}
	else		// must be standing
	{
		if (m_CurStance == eRelaxed)		//Standing( relaxed ), otherwise - standing combat
			nStanding	= 4;		


		// Check if the player is moving
		// This is true if the player is running, moving left, moving right, or moving at all
		if (m_stats.moving)
		{
			// Check if the player is running
			if (m_stats.running)
			{
				nMode = 2;
				if( m_Sprinting )
					nMode = 7;
			}
			else
			{
				// Walking
				nMode = 1;
			}
		}
		else
		{
			nMode = 0;
		}
	}
	// stance stuff over
	// here goes movement direction/type determination -stuff
	if(m_stats.moving)
	{
		Ang3 playerLookAngles = GetActualAngles();
		Ang3 playerMoveAngles = (Ang3)status.vel;
		playerMoveAngles.normalize();
		playerMoveAngles=ConvertUnitVectorToCameraAngles(playerMoveAngles);
		playerMoveAngles.Snap180();

	//	float	moveDeltaAngle = playerLookAngles.DifferenceZ( playerMoveAngles );
		float	moveDeltaAngle = Snap_s180(playerLookAngles.z-playerMoveAngles.z);

		if(m_bSwimming)
		{
//			if( moveDeltaAngle<120 && moveDeltaAngle>-80 )
			if( Ffabs(moveDeltaAngle)<120 )
			{
				// forward 
				nForward = 1;
				forward = true;

				m_LegAngleDesired = Snap_s180(playerMoveAngles.z);
				m_LegAngleVel = m_LegAngleVelMoving;
			}
			else
			{
				// backwards
				m_LegAngleDesired = Snap_s180(playerMoveAngles.z-180);
				m_LegAngleVel = m_LegAngleVelMoving;
			}
		
		}
		else
		{
			if( moveDeltaAngle<120 && moveDeltaAngle>-80 )
	//		if( Ffabs(moveDeltaAngle)<120 )
			{
				// forward 
				nForward = 1;
				forward = true;

				m_LegAngleDesired = Snap_s180(playerMoveAngles.z);
				m_LegAngleVel = m_LegAngleVelMoving;
			}
			else
			{
				// backwards
				m_LegAngleDesired = Snap_s180(playerMoveAngles.z-180);
				m_LegAngleVel = m_LegAngleVelMoving;
			}
		}
	}
	else
	{
		if(m_stats.moving)	// if was moving - just stopped stop rotating legs in movement direction 
				m_LegAngleDesired = m_LegAngle;		
	}

	// [Anton] a piece of code moved to UpdatePhysics

	if(nMode==0)	// do rotation if needed
	{
		float	aDir = Snap_s180(m_LegAngle - m_LegAngleDesired);
		if( Ffabs(aDir)>.2 )
		{
			nMode = 6;
			if( aDir<0 )
				nStrafe  = 1;	// rotate left
			else
				nStrafe  = 2;	// rotate right
		}
	}

//if(speed>0.0f)
//m_pGame->m_pLog->LogToConsole("\001 %.2f %.2f ",speed, m_WalkSpeed);


	m_nStanding = nStanding;
	m_nMode = nMode;
	m_nStrafe = nStrafe;
	m_nForward = nForward;

	Vec3 vel = status.vel;
	if (status.pGroundCollider)
		vel -= status.velGround;
	float speed2d = Vec2(vel).len();

	if(m_AnimationSystemEnabled )	// if enabled 
	{
		if( m_stats.moving )
		{
			ScaleAnimationSpeed( speed2d );
		}
		else
		{
			m_pEntity->SetAnimationSpeed( 1.0f );
		}
		StartAnimation( ctx );
	}

	return;
}


/////////////////////////////////////////////////////////////////////////////////////
void CPlayer::ScaleAnimationSpeed( const float speed2d )
{
float curAniRefSpeed = -1;

		//if is jumping/flying use a normal anim speed
		if (m_stats.flying || m_JumpAniLenght>0)
		{		
			curAniRefSpeed = -1;
		}
		else if(m_stats.bIsLimping )
		{
			curAniRefSpeed = -1;
		}
		else if(m_nMode==7)	// run sprint - don't scale it now	
		{
			curAniRefSpeed = -1;
		}
		else if(m_nMode==3)	// jump
		{
			curAniRefSpeed = -1;
		}
		else if(m_nMode==2)	// run
		{
			if( m_nStanding==3 )	// stealth
			{
				if( m_nForward )
					curAniRefSpeed = m_AniSpeedXRun[0];
				else
					curAniRefSpeed = m_AniSpeedXRun[2];
			}
			else
			{
				if( m_nForward )
					curAniRefSpeed = m_AniSpeedRun[0];	
				else	
					curAniRefSpeed = m_AniSpeedRun[2];
			}
		}
		else if( m_nStanding==0 )	// walking 
		{
			if( m_CurStance == eRelaxed )
			{
				if( m_nForward )
					curAniRefSpeed = m_AniSpeedWalkRelaxed[0];
				else
					curAniRefSpeed = m_AniSpeedWalkRelaxed[2];
			}
			else
			{
				if( m_nForward )
					curAniRefSpeed = m_AniSpeedWalk[0];	
				else
					curAniRefSpeed = m_AniSpeedWalk[2];	
			}
		}
		else if( m_nStanding==3 )	// stealth
		{
			if( m_nForward )
				curAniRefSpeed = m_AniSpeedXWalk[0];	
			else
				curAniRefSpeed = m_AniSpeedXWalk[2];	
		}
		else if( m_nStanding==1 )	//crouch
		{
			if( m_nForward )
				curAniRefSpeed = m_AniSpeedCrouch[0];	
			else
				curAniRefSpeed = m_AniSpeedCrouch[2];	
		}
		else if( m_nStanding==4 )	// walking relaxed
		{
			if( m_nForward )
				curAniRefSpeed = m_AniSpeedWalkRelaxed[0];
			else
				curAniRefSpeed = m_AniSpeedWalkRelaxed[2];
		}


		if(curAniRefSpeed>0.0f)
			m_pEntity->SetAnimationSpeed( speed2d/curAniRefSpeed );
		else
			m_pEntity->SetAnimationSpeed( 1.0f );


}

/////////////////////////////////////////////////////////////////////////////////////

void CPlayer::StartAnimation( const SPlayerUpdateContext &ctx  )
{
	if( m_pGame->pa_blend0->GetFVal()<0.0f )
		return;

	if (!IsAlive())
		return;

	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	char aniName[32];
	if( m_stats.onLadder)
	{
		strcpy( aniName,"ladder_loop" );
		m_JumpStage=0;
		m_JumpAniLenght=0;

		//play ladder animation at a speed between 0.001 and 1, depending on the palyer velocity.
		//we cant use a speed = 0 because it would be impossible to rotate the head bone, fix this in the anim sys?
		float aspeed = max(0.001f,min(1.0f,fabs(ctx.status.vel.z)));
		m_pEntity->SetAnimationSpeed( aspeed );

		/*if( fabs(ctx.status.vel.z) > .1f )
			m_pEntity->SetAnimationSpeed( 1.0f );
		else
			m_pEntity->SetAnimationSpeed( 0.0f );*/
	}
	else if(m_bSwimming)
//		m_stats.fInWater>1.0f)
	{
		// the idle animation should depend on current depth 
 		float depth = m_p3DEngine->GetWaterLevel(m_pEntity) - m_p3DEngine->GetTerrainElevation(m_pEntity->GetPos().x, m_pEntity->GetPos().y);

		ray_hit hit;
		Vec3 pos = m_pEntity->GetPos();
		pos.z = m_p3DEngine->GetWaterLevel(m_pEntity);
		if (m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection( pos, vectorf(0,0,-1.5f), ent_all,
			rwi_stop_at_pierceable,&hit,1, GetEntity()->GetPhysics()))
			depth = hit.dist;

		if( ctx.status.vel.len2()>.3f || depth<1.5f)
		{
			if(GetSelectedWeapon())
				strcpy( aniName,"swim_weapon" );
			else
				strcpy( aniName,"swim" );

		}
		else
		{
			if(GetSelectedWeapon())
				strcpy( aniName,"swim_weapon_idle" );
			else
				strcpy( aniName,"swim_idle" );
		}
		m_JumpStage=0;
		m_JumpAniLenght=0;
		m_pEntity->SetAnimationSpeed( 1.0f );
	}
	else
	{

		GenerateAnimationName( aniName );
	
		if (!m_bIsAI)
		{

			if(m_JumpStage==2 && !m_stats.flying)
				m_JumpStage=0;

			if(m_JumpAniLenght>0)
			{
				m_JumpAniLenght-=m_pTimer->GetFrameTime();

				if(m_JumpStage==2)
				{
					return;
				}

				if(m_JumpAniLenght<=0)
				{
					if(m_JumpStage==1)
					{
						m_JumpStage=2;
					}
				}
				else if (m_JumpStage==1 || m_JumpStage==3)
					return;


			}

			if( m_stats.flying )
			{
				if(m_JumpStage==0 || m_JumpStage==1 
					|| m_JumpStage==3 )	// if jumping immidiately after prev jump - still in landing state
				{
					//strcat( aniName,"_jump_start" );
					if (m_nForward /*|| m_stats.fVel<0.1f*/)
						strcpy( aniName,"jump_forward" );
					else
						strcpy( aniName,"jump_back" );
     					
					m_JumpStage=1;
				}
				else//flying? dont play anything but the jump start animation. So return.
				{
					return;
					//strcat( aniName,"_jump_air" );
				}
			}
			
			if(m_stats.landing)
			{
				strcat( aniName,"_jump_land" );

				if (m_JumpStage!=3)
				{
					m_JumpStage=3;
					m_JumpAniLenght = 0;
				}
			}
		}
//		else
//			m_JumpStage=0;
	}

	if(m_sPrevAniName == aniName)
		return;

	if(m_stats.landing && !m_bIsAI)
	{
		if(m_JumpAniLenght>0)
			return;
		m_JumpAniLenght = m_pEntity->GetAnimationLength( aniName );
	}
	else if(m_JumpStage==1)
	{
		//we dont care anymore about the length of jumpin animations, because after this we keep the last frame until land.
		m_JumpAniLenght = 0;
		//m_JumpAniLenght = m_pEntity->GetAnimationLength( aniName );
	}
	else if(m_JumpAniLenght>0)
		return;	// don't start other anims before jump landing animation is over

//	float	blendTime = m_pGame->pa_blend0->GetFVal();
//	GetBlendTime( aniName, blendTime );

	float	blendTime = m_pGame->pa_blend0->GetFVal();

	//use a different bleding for the jump start anim
	if (m_JumpStage==1)
	{
		//when jump start reset current animations, so when the jump anim will finish player will keep its the last frame.
		m_pEntity->ResetAnimations(0);
		//blendTime = 0.15f;

		//set jumpstage directly to 2 (inair)
		m_JumpStage = 2;
	}

	if(m_JumpStage==3 && m_JumpAniLenght<=0)
	{
//		m_sPrevAniNameLayer1 = "";
		m_JumpStage=0;
		blendTime = 0.15f;
		//blendTime = 0.05f;
	}
	m_pEntity->StartAnimation(0, aniName, 0, blendTime );//.3f);
	m_sPrevAniName = aniName;
/*
	if(m_pEntity->StartAnimation(0, aniName, 0, blendTime ))//.3f))
	{
		m_sPrevAniName = aniName;
	//m_pGame->m_pLog->LogToConsole("\002 %s ",aniName.c_str());
	}
*/
}


/////////////////////////////////////////////////////////////////////////////////////
/*
-- 	generates string with appropriate animation name, according to naming convention
--	depending on parametrs
--
-- 	m_nForward: 	0 = Backward, 	1 = Forward
-- 	m_nStrafe: 	0 = None, 		1 = Left, 		2 = Right
-- 	m_nStanding: 	0 = Standing, 	1 = Crouching, 	2 = Prone,	3 = Stealth,
-- 	m_nMode: 		0 = Idle, 		1 = Walking, 	2 = Running, 	3 = Jump, 	4 = Flying, 	5 = Dead,	6 = Turning, 
*/

bool CPlayer::GenerateAnimationName( char *aniName )
{
	strcpy( aniName,"" );

//aniName = "aidle";
//return;

	if(m_stats.bIsLimping)
		strcpy( aniName,"l" );
	else
		switch(m_nStanding)
	{
		case	0:					// standing combat
			strcpy( aniName,"a" );
			break;
		case	1:					// crouching combat
			strcpy( aniName,"c" );
			break;
		case	2:					// proning combat
			strcpy( aniName,"p" );
			break;
		case	3:					// stealth combat
			strcpy( aniName,"x" );
			break;
		case	4:					// standing relaxed
			strcpy( aniName,"s" );
			break;
		default:
			return false;
	}
	switch(m_nMode)
	{
	case 0:
		strcat( aniName,"idle" );
		return true;
	case 1:
		strcat( aniName,"walk" );
		break;
	case 2:
		strcat( aniName,"run" );
		break;
	case 6:
		strcat( aniName,"rotate" );
		break;
	case 7:
		strcat( aniName,"sprint" );
		break;
	default:
		return true;
	}

	if(m_nStrafe)
	{
		if(m_nStrafe == 1)
			strcat( aniName,"left" );
		else
			strcat( aniName,"right" );
	}
	else if(m_nForward)
			strcat( aniName,"fwd" );
		else
			strcat( aniName,"back" );

	return true;
}



///////////////////////////////////////////////
/*! Let the player die by playing the animation and disabling physics
*/
/*
void CPlayer::Die()
{
	m_pEntity->KillTimer();
//	m_deathTime = GetCurrTime();
	

	IEntityCharacter *ichar = m_pEntity->GetCharInterface();
//	UpdateCharacterAnimations();
	
	ICryCharInstance *character = ichar->GetCharacter(PLAYER_MODEL_IDX);
	if (character)
	{
		// Hide weapon.
		character->EnableLastIdleAnimationRestart(0,false);
		character->EnableLastIdleAnimationRestart(1,false);
		character->AttachObjectToBone(NULL,NULL);

		if (m_bIsAI)
			character->ResetAnimations();
	}
	
	m_pEntity->EnableAI(false);

	if (m_pEntity->GetPhysics()->GetType()==PE_ARTICULATED)
		return;

	// Disable physics on this player.
	m_pEntity->EnablePhysics(false);

}
*/

///////////////////////////////////////////////
/*! Respawns the player by enabling physics and calling the script
*/
void CPlayer::Respawn()
{
	// Exit all areas to turn off water sound.
	//m_pGame->m_XAreaMgr.ExitAllAreas( this );

	m_walkParams.shakeDegree=0.0f;
	m_walkParams.leanAmount = 0.0f;
	m_walkParams.leanStart = 0.0f;
	m_walkParams.leanEnd = 0.0f;
	m_walkParams.leanFactor = 0.0f;

	// Call game rules and try to revive this player.
	CXServerRules *rules = m_pGame->GetRules();
	// If no server rules, ignore this function.
	if (rules)
	{
		rules->OnPlayerRespawn( m_pEntity );
	}

//	m_physicsEnabled = true;
	m_pEntity->EnablePhysics(true);
	m_pEntity->EnableAI(true);

	
//	SetTime2Respawn( -1 );

	m_bStayCrouch = false;
}

///////////////////////////////////////////////
/*! Retrieves if this player is mine
		@return returns true if it is my player, false otherwise
*/
bool CPlayer::IsMyPlayer() const
{
	if(!m_pGame->m_pClient) return false;
	return (m_pEntity->GetId()==m_pGame->m_pClient->GetPlayerId());
}

///////////////////////////////////////////////
/*! Retrieves if we are in 1st person view
		@return returns true if we are in 1st person view, false otherwise
*/
bool CPlayer::IsFirstPerson() const
{
	return m_bFirstPerson;
}

///////////////////////////////////////////////
/*! Writes all data which needs to be synchronized to the network-stream
		@param stream stream to write to
		@param cs state of the entity per serverslot
		@return true if the function succeeds, false otherwise
*/
bool CPlayer::Write( CStream &stream,EntityCloneState *cs)
{
	bool bLocalHostEntity=true;

	if(cs)
    bLocalHostEntity=cs->m_bLocalplayer;

	WRITE_COOKIE(stream);

  stream.Write(bLocalHostEntity);

	if(bLocalHostEntity)
	{
		assert(m_stats.health>=0);
		if(m_stats.health>255)
		{
			GameWarning(" Player health=%d is bigger than 255 (name %s). Change properties", m_stats.health,m_pEntity->GetName());
			m_stats.health = 255;
		}
//		assert(m_stats.health<=255);
		assert(m_stats.armor>=0);
		assert(m_stats.armor<=255);
		if(m_stats.numofgrenades<0)
		{
			GameWarning(" Player numofgrenades=%d is less than 0 (name %s). Change properties", m_stats.numofgrenades,m_pEntity->GetName());
			m_stats.numofgrenades = 0;
		}
		if(m_stats.numofgrenades>255)
		{
			GameWarning(" Player numofgrenades=%d is bigger than 255 (name %s). Change properties", m_stats.numofgrenades,m_pEntity->GetName());
			m_stats.numofgrenades = 255;
		}

		assert(m_stats.stamina>=0 && m_stats.stamina<255);
		stream.WritePkd( (BYTE)m_stats.stamina );
//		assert(m_stats.breath>=0 && m_stats.breath<255);
//		stream.WritePkd( (BYTE)m_stats.breath );

		stream.WritePkd( (BYTE)m_stats.health );
		stream.WritePkd( (BYTE)m_stats.armor );

		if((unsigned int)(m_stats.ammo_in_clip)>1023)
		{
			m_pGame->GetSystem()->GetILog()->LogError("Ammo in clip (%d) is more than 1023, Value will not be restored correctly",m_stats.ammo_in_clip);
			if(m_stats.ammo_in_clip>0)
				m_stats.ammo_in_clip=1023;
		}

		stream.WriteNumberInBits(m_stats.ammo_in_clip,10);
		
		if((unsigned int)(m_stats.ammo)>1023)
		{
			m_pGame->GetSystem()->GetILog()->LogError("Ammo (%d) is more than 1023, Value will not be restored correctly",m_stats.ammo);
			if(m_stats.ammo>0)
				m_stats.ammo=1023;
		}

		stream.WriteNumberInBits(m_stats.ammo,10);
		stream.WritePkd((BYTE)m_stats.numofgrenades);
		stream.WriteNumberInBits(m_stats.grenadetype, 4);
		stream.Write(m_stats.holding_breath);
		for(int i=0;i<sizeof(m_vWeaponSlots)/sizeof(int);i++)
		{
			stream.Write(m_vWeaponSlots[i]!=0);
			if(m_vWeaponSlots[i]!=0)
			{
				assert(m_vWeaponSlots[i]>=0);
				assert(m_vWeaponSlots[i]<=255);

				stream.WritePkd((BYTE)m_vWeaponSlots[i]);
			}
		}
	}
	else
	{
		stream.Write((bool)(m_stats.ammo_in_clip!=0));
		stream.Write((bool)(m_stats.ammo!=0));

		stream.Write( m_stats.firing );
		stream.Write(m_walkParams.fCurrLean!=0);
		if(m_walkParams.fCurrLean!=0)
		{
			stream.Write(m_walkParams.fCurrLean>0); //sign
			stream.WritePkd(((BYTE)(fabs(m_walkParams.fCurrLean)/(1.0f/255))));
		}

	}
	WRITE_COOKIE(stream);

	bool bSendFireGrenade = m_stats.firing_grenade;

	if((m_stats.grenadetype != 1 && m_stats.numofgrenades<=0))
		bSendFireGrenade = false;

	stream.Write(bSendFireGrenade);
	// [kirill]
	// this is needed to be able to keep PrevWeapon when doing quicksave/quickload 
	// while on ladder - so when player gets out of ladder after QuickLoad he has 
	// correct weapon
	//[filippo]
	//in MP we need to send always the right "m_stats.weapon" value, otherwise the ladders would not works correctly.
	if( m_stats.onLadder && !m_pGame->IsMultiplayer())
		stream.WritePkd((BYTE)m_PrevWeaponID);
	else
		stream.WritePkd((BYTE)m_stats.weapon);
	stream.WritePkd((BYTE)m_stats.firemode);
//	stream.Write( m_stats.onLadder );
	stream.WriteNumberInBits((unsigned int)m_CurStance,3);
	stream.Write( m_stats.jumping );

	// sync player random seed value to the clients
	{
		bool bSyncToClients=m_SynchedRandomSeed.GetSynchToClientsS();
		stream.Write(bSyncToClients);

		if(bSyncToClients)
			stream.Write(m_SynchedRandomSeed.GetStartRandomSeedS());
	}


	stream.Write( m_stats.reloading );

	BYTE acc=(BYTE)(m_stats.firing?m_stats.last_accuracy:m_stats.accuracy/(1.f/255.f));
	stream.WritePkd(acc);
	WRITE_COOKIE(stream);

	if (m_pRedirected && m_pRedirected->GetId()<50000)
	{
		stream.Write(true);
		stream.Write(m_pRedirected->GetId());
	}
	else
		stream.Write(false);

	stream.Write(m_bFirstPerson);

	m_bWriteOccured = true;

	return true;
}

///////////////////////////////////////////////
/*! Reads all data which needs to be synchronized from the network-stream
		@param stream stream to read from
		@return true if the function succeeds, false otherwise
*/
bool CPlayer::Read( CStream &stream )
{
	unsigned short dirty = 0;
	PlayerStats &stats=m_stats;
//	GetPlayerStats( stats );
	BYTE health,armor,weapon,firemode,staminaBuff;
	//int weaponid = stats.weaponid;
	bool bLocalHostEntity;

	VERIFY_COOKIE(stream);
	stream.Read(bLocalHostEntity);

	if(bLocalHostEntity)
	{
		stream.ReadPkd( staminaBuff );
		m_stats.stamina = staminaBuff;
//		stream.ReadPkd( staminaBuff );
//		m_stats.breath = staminaBuff;
		stream.ReadPkd( health );
		stats.health=health;
		stream.ReadPkd( armor );
		stats.armor=armor;
		unsigned int ammo_in_clip;
		stream.ReadNumberInBits(ammo_in_clip,10);
		stats.ammo_in_clip=ammo_in_clip;
		unsigned int ammo;
		stream.ReadNumberInBits(ammo,10);
		stats.ammo=ammo;
		BYTE b;
		stream.ReadPkd(b);
		stats.numofgrenades=b;
		stream.ReadNumberInBits(stats.grenadetype, 4);
		stream.Read(stats.holding_breath);
		
		bool isnotzero;
		for(int i=0;i<sizeof(m_vWeaponSlots)/sizeof(int);i++)
		{
			stream.Read(isnotzero);
			if(isnotzero)
			{
				stream.ReadPkd(b);
				m_vWeaponSlots[i]=b;
			}else
			{
				m_vWeaponSlots[i]=0;
			}
			
		}
	}
	else
	{
		bool val;
		stream.Read(val);
		stats.ammo_in_clip = val;
		stream.Read(val);
		stats.ammo = val;
		stream.Read(val);
		stats.firing = val;

		bool t;
		stream.Read(t);
		if(t)
		{
			bool sign;
			BYTE lean;
			stream.Read(sign);
			stream.ReadPkd(lean);
			m_walkParams.fCurrLean=sign?(lean*(1.0f/255)):-(lean*(1.0f/255));
		}
		else
		{
			m_walkParams.fCurrLean=0;
		}
	}
	VERIFY_COOKIE(stream);
	stream.Read(stats.firing_grenade);
	stream.ReadPkd(weapon);
	if (weapon!=stats.weapon)
	{
		SelectWeapon(weapon, false);
		stats.weapon = weapon;
		stats.firing = false;
	}
	stream.ReadPkd(firemode);
	if(firemode!=stats.firemode)
	{
		stats.firemode=firemode;
		SwitchFiremode(stats.firemode);
	}

	//stream.Read(stats.firing);
//	stream.Read(stats.onLadder);
	eStance stance;
	stream.ReadNumberInBits(*((unsigned int *)&stance),3);
	
	// no stances in vehicle
	if(stance!=m_CurStance && !m_pVehicle)
	{
		switch(stance)
		{
		case eRelaxed:
			GoRelaxed();
			break;
		case eStand:
			GoStand();
			break;
//		case eAimLook:
//			GoAim();
//			break;
		case eStealth:
			GoStealth();
			break;
		case eCrouch:
			GoCrouch();
			break;
		case eProne:
			GoProne();
			break;
		case eSwim:
			GoSwim();
			break;
		default:
			//DEBUG_BREAK;
			::OutputDebugString("INVALID STANCE\n");
			m_CurStance=(eStance)stance;
			break;
		}
	}
	stream.Read(stats.jumping);

	if(stream.GetStreamVersion()>=PATCH1_SAVEVERSION)					// to be backward compatible with old samegames
	{
		bool bSyncToAllClients;

		stream.Read(bSyncToAllClients);

		if(bSyncToAllClients)
		{
			uint8 ucStartRandomSeedCS;

			stream.Read(ucStartRandomSeedCS);

			m_SynchedRandomSeed.SetStartRandomSeedC(ucStartRandomSeedCS);
		}
	}


	//TRIGGER REALOAD ON CLIENT ONLY (think if can be done better)
	bool bReloading;
	stream.Read(bReloading);
	
	if(bReloading && (stats.reloading==false) && m_nSelectedWeaponID != -1)
	{
		CWeaponClass *pSelectedWeapon = GetSelectedWeapon();
		pSelectedWeapon->ScriptOnStopFiring(m_pEntity);
		pSelectedWeapon->ScriptReload(m_pEntity);
	}
	stats.reloading=bReloading;

	
	BYTE acc;
	stream.ReadPkd(acc);
	stats.accuracy=acc*(1.f/255.f);

	VERIFY_COOKIE(stream);


	if(stats.firing_grenade)
	{
		//invoke script event (client)
		m_bGrenadeAnimation = true;
		m_pEntity->SendScriptEvent(ScriptEvent_FireGrenade,0,NULL);
	}

	bool bRedirected;
	EntityId nID;
	stream.Read(bRedirected);
	if (bRedirected)
		stream.Read(nID);
	if ((m_pRedirected!=0)!=bRedirected || m_pRedirected && m_pRedirected->GetId()!=nID)
	{
		if (m_pRedirected)
		{
			m_pRedirected->OnUnBind(m_pEntity,0);
			m_pRedirected = 0;
		}
		if (bRedirected && (m_pRedirected=m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(nID)))
			m_pRedirected->OnBind(m_pEntity,0);
	}
	if (m_pRedirected && !bLocalHostEntity)
		m_pRedirected->SetAngles(m_pEntity->GetAngles());

	stream.Read(m_bFirstPersonLoaded);

	return true;
}


bool CPlayer::Save( CStream &stream)
{
	EntityCloneState cs;

	cs.m_bSyncYAngle = false;
	cs.m_bOffSync = false;
	cs.m_bLocalplayer = true;

	//[PETAR] Serialize flashlight for singleplayer
	stream.Write(m_bLightOn);
	bool bRet=Write(stream,&cs);

	ASSERT(bRet);
	
	return true;
}

bool CPlayer::Load( CStream &stream)
{
	bool bPrevLightStatus;
	stream.Read(bPrevLightStatus);
	if (bPrevLightStatus!=m_bLightOn)
		SwitchFlashLight(bPrevLightStatus);
	return Read(stream);
}

// these two functions save and load the player state which is NOT already saved using read/write

bool CPlayer::SaveGame(CStream &stm)
{
	for(PlayerWeaponsItor itor=m_mapPlayerWeapons.begin(); itor!=m_mapPlayerWeapons.end(); ++itor)
	{
		WeaponInfo &wi = itor->second;
    if(wi.owns)
    {
			stm.Write(itor->first);
			ASSERT(itor->first>=0 && itor->first<50);
			stm.Write(wi.iFireMode);
    }
	}
	stm.Write((int)0);
	for(int i=0;i<sizeof(m_vWeaponSlots)/sizeof(int);i++)
	{
		stm.Write(m_vWeaponSlots[i]);
	}

	Save(stm);

	return true;
};

bool CPlayer::LoadGame(CStream &stm)
{
	RemoveAllWeapons();

	// [Petar] preserve the original health in case we need to use it
	// to restore the health of special AI on a checkpoint
	int nStartHealth = m_stats.health;
	if (!m_bIsAI)
		DeselectWeapon();

	for(int cl = 0; (stm.Read(cl),cl); )
  {
    WeaponInfo &wi = m_mapPlayerWeapons[cl];
    wi.owns = true;
    stm.Read(wi.iFireMode);
  }
	for(int i=0;i<sizeof(m_vWeaponSlots)/sizeof(int);i++)
	{
		stm.Read(m_vWeaponSlots[i]);
	}

	Load(stm);


	SelectWeapon(m_stats.weapon);

	if (m_nSelectedWeaponID != -1)
	{
		WeaponInfo &wi = GetWeaponInfo();

		CWeaponClass *pSelectedWeapon = GetSelectedWeapon();
		pSelectedWeapon->ScriptOnStopFiring(m_pEntity);

		_SmartScriptObject pObj(m_pScriptSystem);
		pObj->SetValue( "firemode", m_stats.firemode);
		pObj->SetValue( "ignoreammo", true);

		bool bCanSwitch;
		m_pEntity->SendScriptEvent(ScriptEvent_FireModeChange, *pObj, &bCanSwitch);

		wi.iFireMode = m_stats.firemode;
	}

	if (m_bIsAI)
	{
		IPuppet *pPuppet=0;
		if (m_pEntity->GetAI()->CanBeConvertedTo(AIOBJECT_PUPPET,(void**)&pPuppet))
		{
			if (pPuppet->GetPuppetParameters().m_bSpecial)
			  m_stats.health = nStartHealth;
		}
	}	

	return true;
};

void CPlayer::SetScriptObject(IScriptObject *pObject)
{
	m_pScriptObject=pObject;
	m_pUpdateAnimation=NULL;
}
/*! Retrieves the ScriptObject of this container
		@return pointer to the ScriptObject
*/
IScriptObject *CPlayer::GetScriptObject()
{
	return m_pScriptObject;
}

int CPlayer::MakeWeaponAvailable(int nWeaponID, bool bAvailable)
{
	//make sure that we don't remove the currently selected weapon
	if (nWeaponID == m_nSelectedWeaponID && !bAvailable)
	{
		DeselectWeapon();
		SelectFirstWeapon();
	}

	CWeaponClass* pWC = GetGame()->GetWeaponSystemEx()->GetWeaponClassByID(nWeaponID);
	if (pWC && !pWC->IsLoaded())
	{
		pWC->Load();
	}

	PlayerWeaponsItor it;

	if ((it = m_mapPlayerWeapons.find(nWeaponID)) == m_mapPlayerWeapons.end())
	{
		TRACE("ERROR: Trying to make invalid weapon ID %i available", nWeaponID);
		return -1;
	}
	bool bWasOwning=(* it).second.owns;
	
	(* it).second.owns = bAvailable;	

	int slot=0;
	if(bAvailable && (!bWasOwning))
	{
		(* it).second.iFireMode = 0;
		slot=0;
		while(slot<PLAYER_MAX_WEAPONS){if(m_vWeaponSlots[slot]==nWeaponID)return slot;slot++;}
		slot=0;
		while(slot<PLAYER_MAX_WEAPONS && m_vWeaponSlots[slot]!=0) slot++;

		if(slot<PLAYER_MAX_WEAPONS && m_vWeaponSlots[slot]==0)
		{
			m_vWeaponSlots[slot]=nWeaponID;
			return slot;
		}
	}else if(!bAvailable && (bWasOwning))
	{
		while(slot<PLAYER_MAX_WEAPONS){if(m_vWeaponSlots[slot]==nWeaponID){m_vWeaponSlots[slot]=0;return -1;} slot;slot++;}
	}
	return -1;
}

CWeaponClass *CPlayer::DeselectWeapon()
{

	if (m_nSelectedWeaponID != -1)
	{
			CWeaponClass *pSelectedWeapon = GetSelectedWeapon();
			//this script event is called only for the local client
			if(IsMyPlayer() || m_pGame->IsMultiplayer())
			{
				pSelectedWeapon->ScriptOnDeactivate(m_pEntity);
			}

			// detach from bone ?
			if (!pSelectedWeapon->GetBindBone().empty())
				m_pEntity->DetachObjectToBone(pSelectedWeapon->GetBindBone().c_str());

			SetWeapon(-1);
			
			return pSelectedWeapon;
	}
	return 0;
}

/*ridirect the input of a player to another entity
	@param id the id of the entity that will receive the input
	@param angleDelta
*/
void CPlayer::RedirectInputToEntity(EntityId id, int angleDelta)
{
	if (!id)
	{
		if (m_pRedirected)
			m_pRedirected = NULL;
		if(IsMyPlayer())
			m_pGame->m_pClient->m_PlayerProcessingCmd.SetDeltaAngles(Vec3d(0.0f, 0.0f, 0.0f));
	}
	else
	{
		IEntity *pEntity = m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(id);
		if (pEntity)
		{
			m_pRedirected = pEntity;
			//[PETAR]
			//    m_pGame->m_pClient->m_PlayerProcessingCmd.SetDeltaAngles(pEntity->GetAngles());
			//[KIRILL] make turret point in right direction /statmounted - ONLY for local player
			if(IsMyPlayer())
			{
				if(angleDelta>=0)
					m_pGame->m_pClient->m_PlayerProcessingCmd.SetDeltaAngles(Vec3d(0.0f, 0.0f, (float)angleDelta));
				else
					m_pGame->m_pClient->m_PlayerProcessingCmd.SetDeltaAngles(pEntity->GetAngles(1));
			}
		}
	}
}

bool CPlayer::QueryContainerInterface(ContainerInterfaceType desired_interface, void **ppInterface )
{

	if (desired_interface == CIT_IPLAYER)
	{
		*ppInterface = (void *) this;
		return true;
	}
	else
	{
		*ppInterface = 0;
		return false;
	}
		
}

IEntity * CPlayer::GetRedirected()
{
	return m_pRedirected;
}

/*! set the offset of the camera from the player position.
	usually is the height of the player
*/
void CPlayer::SetCameraOffset(const Vec3d& Offset)
{
	m_pEntity->GetCamera()->SetCameraOffset(Offset);
}

/*! retreive the offset of the camera relative to the player position
	@param Offset the offset in meters
*/
void CPlayer::GetCameraOffset(Vec3d& Offset)
{
	m_pEntity->GetCamera()->GetCameraOffset(Offset);
}

void CPlayer::UpdateDrawAngles( )
{
	ICryCharInstance *pChar = m_pEntity->GetCharInterface()->GetCharacter(0);

	if (!pChar)
		return;

	float timeScale = 20.1f*m_pTimer->GetFrameTime();

	if (timeScale>1.0f)
		timeScale = 1.0f;
	
	Vec3d angles = GetActualAngles();
	Vec3d vAngles;

	//on ladder orient the player so he always look at the center of the ladder.
	if  (m_stats.onLadder) 
	{
		m_vLadderAngles = m_pEntity->GetPos() - m_vLadderPosition;
		m_vLadderAngles.z = 0;
		
		m_vLadderAngles.Normalize();

        m_vLadderAngles.z = -atan2(m_vLadderAngles.x,m_vLadderAngles.y)*(180.0f/gf_PI);
        m_vLadderAngles.x = 0;
        m_vLadderAngles.y = 0;
         
		vAngles = m_vLadderAngles;
	}
	else if (m_pEntity->IsBound())
		vAngles = angles;
	else if (m_CurStance == eProne )		
		vAngles = m_EnvTangent;
	else
	{
		if(m_LegRotation)
			vAngles = Vec3d(0, 0, m_LegAngle);
		else
			vAngles = Vec3d(0,0,angles.z);
	}

	//[kirill] smooth here Character angles (all but Z)
	// to make it not jurk when proning on some bumpy enviroment
	vAngles.x = Snap_s180(vAngles.x);
	vAngles.y = Snap_s180(vAngles.y);
	m_vCurEntAngle.x = Snap_s180(m_vCurEntAngle.x);
	m_vCurEntAngle.y = Snap_s180(m_vCurEntAngle.y);

	Vec3d delta = (vAngles - m_vCurEntAngle);
	delta.x = Snap_s180(delta.x);
	delta.y = Snap_s180(delta.y);
	
	m_vCurEntAngle += (delta*timeScale);
	m_vCurEntAngle.z = vAngles.z;
	m_vCharacterAngles = m_vCurEntAngle;

//	CheckIfNAN( vAngles );
//	m_vCharacterAngles = vAngles;
}
 
/*!drawn the player
	called by the engine when the player has to be drawn
*/
void CPlayer::OnDraw(const SRendParams & _RendParams)
{
	//return;

	OnDrawMountedWeapon( _RendParams );

	// if nRecursionLevel is not 0 - use only 3tp person view ( for reflections )
	int nRecursionLevel = (int)m_pGame->GetSystem()->GetIRenderer()->EF_Query(EFQ_RecurseLevel) - 1;

	// draw first person weapon
	if(m_bFirstPerson && !nRecursionLevel && m_stats.drawfpweapon	&& m_nSelectedWeaponID != -1)
	{
		CWeaponClass* pWeapon		= GetSelectedWeapon();
		ICryCharInstance *pInst	= pWeapon->GetCharacter();
		
		if (pInst && pInst->GetFlags()&CS_FLAG_DRAW_MODEL)
		{
			SRendParams RendParams      = _RendParams;
			RendParams.vPos             = pWeapon->GetPos();
			RendParams.vAngles          = pWeapon->GetAngles();

			if (RendParams.pShadowVolumeLightSource)
			{
				if(m_pEntity->GetRndFlags()&ERF_CASTSHADOWVOLUME && !IsMyPlayer())
					pInst->RenderShadowVolumes(&RendParams);
			}
			else {
				pInst->Draw(RendParams,m_pEntity->GetPos());
			}
		}

		return;
	}


	ICryCharInstance *pChar = m_pEntity->GetCharInterface()->GetCharacter(0);
	if (!pChar)
		return;

	if (!(pChar->GetFlags() & CS_FLAG_DRAW_MODEL) && !nRecursionLevel)
		return;

  SRendParams RendParams = _RendParams;
//  RendParams.vPos = m_pEntity->GetPos(); // position is not always entity position

	if(m_pVehicle )	// it' gunner in vehicle
	{
	// create the matrix here
		Matrix44 matParent;
		matParent.SetIdentity();

		matParent=Matrix44::CreateRotationZYX(-gf_DEGTORAD*m_pVehicle->GetEntity()->GetAngles())*matParent; //NOTE: angles in radians and negated 

		CryQuat cxquat = Quat( GetTransposed44(matParent) );

		CryQuat rxquat;
		Vec3d	gunnerAngle;
//		if(m_pMountedWeapon)	// it' gunner in vehicle
		if(m_stats.inVehicleState == PVS_GUNNER)	// it' gunner in vehicle
			gunnerAngle = GetEntity()->GetAngles(1);
		else					// it' driver/passenger
			gunnerAngle.Set(0,0,180);
		gunnerAngle.x=0;
		gunnerAngle.y=0;
		rxquat.SetRotationXYZ(DEG2RAD(gunnerAngle));

		CryQuat result = cxquat*rxquat;
		Vec3d finalangles = Ang3::GetAnglesXYZ(Matrix33(result));
		RendParams.vAngles = RAD2DEG(finalangles);
		m_pEntity->SetPhysAngles( RendParams.vAngles );
	}
	else
//	if (!m_pVehicle || m_bIsAI )
	{
		if( IsAlive())
			RendParams.vAngles = m_vCharacterAngles;
		else 
			RendParams.vAngles.Set(0, 0, m_pEntity->GetAngles().z);
		
	}
/*
	else
	{
		// if the player is bound to the vehicle and in 3rd person mode,
		// force the character's angles to the correct direction
		RendParams.vAngles = m_pVehicle->GetEntity()->GetAngles();
//		RendParams.vAngles.z+=180;
//		RendParams.vAngles.x=-RendParams.vAngles.x;
	}
*/

	if (_RendParams.pShadowVolumeLightSource)
	{	
		if(m_pEntity->GetRndFlags()&ERF_CASTSHADOWVOLUME)
			pChar->RenderShadowVolumes(&RendParams, (m_pEntity->GetRndFlags()&ERF_SELFSHADOW) ? 0 : 10);
	}
	else
		pChar->Draw(RendParams,m_pEntity->GetPos());
}


/*!sets the animation speed of the player - used to scale animation playback speed 
	depending on actuall movement speed (to awoid skeiting)
*/
void CPlayer::SetAnimationRefSpeedRunRelaxed(const float fwd, const float side, const float back )
{
	m_AniSpeedRunRelaxed[0] = fwd;
	m_AniSpeedRunRelaxed[1] = side;
	m_AniSpeedRunRelaxed[2] = back;
}
/*!sets the animation speed of the player - used to scale animation playback speed 
	depending on actuall movement speed (to awoid skeiting)
*/
void CPlayer::SetAnimationRefSpeedWalkRelaxed(const float fwd, const float side, const float back )
{
	m_AniSpeedWalkRelaxed[0] = fwd;
	m_AniSpeedWalkRelaxed[1] = side;
	m_AniSpeedWalkRelaxed[2] = back;
}

/*!sets the animation speed of the player - used to scale animation playback speed 
	depending on actuall movement speed (to awoid skeiting)
*/
void CPlayer::SetAnimationRefSpeedWalk(const float fwd, const float side, const float back )
{
	m_AniSpeedWalk[0] = fwd;
	m_AniSpeedWalk[1] = side;
	m_AniSpeedWalk[2] = back;
}

/*!sets the animation speed of the player - used to scale animation playback speed 
	depending on actuall movement speed (to awoid skeiting)
*/
void CPlayer::SetAnimationRefSpeedXRun(const float fwd, const float side, const float back )
{
	m_AniSpeedXRun[0] = fwd;
	m_AniSpeedXRun[1] = side;
	m_AniSpeedXRun[2] = back;
}


/*!sets the animation speed of the player - used to scale animation playback speed 
	depending on actuall movement speed (to awoid skeiting)
*/
void CPlayer::SetAnimationRefSpeedRun(const float fwd, const float side, const float back )
{
	m_AniSpeedRun[0] = fwd;
	m_AniSpeedRun[1] = side;
	m_AniSpeedRun[2] = back;
}

/*!sets the animation speed of the player - used to scale animation playback speed 
	depending on actuall movement speed (to awoid skeiting)
*/
void CPlayer::SetAnimationRefSpeedXWalk(const float fwd, const float side, const float back )
{
	m_AniSpeedXWalk[0] = fwd;
	m_AniSpeedXWalk[1] = side;
	m_AniSpeedXWalk[2] = back;
}

/*!sets the animation speed of the player - used to scale animation playback speed 
	depending on actuall movement speed (to awoid skeiting)
*/
void CPlayer::SetAnimationRefSpeedCrouch(const float fwd, const float side, const float back )
{
	m_AniSpeedCrouch[0] = fwd;
	m_AniSpeedCrouch[1] = side;
	m_AniSpeedCrouch[2] = back;
}



/*!sets the running speed of the player
	@param speed the running speed
*/
void CPlayer::SetRunSpeed(const float speed)
{
	m_RunSpeed = speed;
	if(!m_bIsAI)
//	if(IsMyPlayer())
		m_pGame->p_speed_run->Set(m_RunSpeed);
}	

/*!sets the walk speed of the player
	@param speed the walk speed
*/
void CPlayer::SetWalkSpeed(const float speed)
{
	m_WalkSpeed = speed;
	if(!m_bIsAI)
//	if(IsMyPlayer())
		m_pGame->p_speed_walk->Set(m_WalkSpeed);
}	

/*!sets the walk speed of the player when is croching
	@param speed the croching speed
*/
void CPlayer::SetCrouchSpeed(const float speed)
{
	m_CrouchSpeed = speed;
	if(!m_bIsAI)
//	if(IsMyPlayer())
		m_pGame->p_speed_crouch->Set(m_CrouchSpeed);
}	

/*!sets the walk speed of the player when is proning
	@param speed the proning speed
*/
void CPlayer::SetProneSpeed(const float speed)
{
	m_ProneSpeed = speed;
	if(!m_bIsAI)
//	if(IsMyPlayer())
		m_pGame->p_speed_prone->Set(m_ProneSpeed);
}	

/*!sets the walk speed of the player when is proning
	@param speed the proning speed
*/
void CPlayer::SetSwimSpeed(const float speed)
{
	m_SwimSpeed = speed;
}	


//----------------------------------------------------------------------------------------------------
/*!sets the camera bob
	@param pitch
	@param roll
	@param length
*/
void CPlayer::SetCameraBob(const float pitch, const float roll, const float length)
{
	m_walkParams.runRoll = roll;
	m_walkParams.runPitch = pitch;
	m_walkParams.stepLength = length;

	if (!m_bIsAI)
	{
		m_pGame->p_bob_pitch->Set(m_walkParams.runPitch);
		m_pGame->p_bob_roll->Set(m_walkParams.runRoll);
		m_pGame->p_bob_length->Set(m_walkParams.stepLength);
	}
}	

//----------------------------------------------------------------------------------------------------
//
/*!sets the weapon bob
*/
void CPlayer::SetWeaponBob(const float ampl)
{
	m_walkParams.weaponCycle = ampl;
	if (!m_bIsAI)
		m_pGame->p_bob_weapon->Set(m_walkParams.weaponCycle);
}	



//----------------------------------------------------------------------------------------------------
/*!sets the amount of force applied to the player when he jump
	@param force the amount of force
*/
void CPlayer::SetJumpForce(const float force)
{
	m_JumpForce = force;
	if (!m_bIsAI)
		m_pGame->p_jump_force->Set(m_JumpForce);
}	

//----------------------------------------------------------------------------------------------------
//
/*!set the player's lean angle
	@param lean angle in degrees
*/
void CPlayer::SetLean(const float lean)
{
	m_LeanDegree = lean;

//[KIRILL]	this console variable makes lean global - it has to be player's per-instance property
// to fix bug in MP with leaning
//	if (!m_bIsAI)
//		m_pGame->p_lean->Set(m_LeanDegree);
}	



//----------------------------------------------------------------------------------------------------
/*!sets the player dimension in stealth mode
	@param pDim struct containing the player dimensions
	@see pe_player_dimensions
*/
void CPlayer::SetDimStealth(const pe_player_dimensions* const pDim)
{
	if(!pDim)
	{
		m_PlayerDimStealth.heightEye = (m_PlayerDimNormal.heightEye + m_PlayerDimCrouch.heightEye)*.5f;
		m_PlayerDimStealth.heightCollider = (m_PlayerDimNormal.heightCollider + m_PlayerDimCrouch.heightCollider)*.5f;
		m_PlayerDimStealth.sizeCollider = (m_PlayerDimNormal.sizeCollider+m_PlayerDimCrouch.sizeCollider)*.5f;
		return;
	}
	m_PlayerDimStealth = *pDim;
	m_CurStance = eNone;
	GoStand( );
}	


//----------------------------------------------------------------------------------------------------
/*!sets the player dimension in normal(standing) mode
	@param pDim struct containing the player dimensions
	@see pe_player_dimensions
*/
void CPlayer::SetDimNormal(const pe_player_dimensions* const pDim)
{
	if(!pDim)
	{
		m_PlayerDimNormal.heightEye = 1.7f;
		m_PlayerDimNormal.heightCollider = 1.1f;
		m_PlayerDimNormal.sizeCollider.Set(0.4f,0.4f,0.7f);
		return;
	}
	m_PlayerDimNormal = *pDim;
	SetDimStealth();
	m_CurStance = eNone;
	GoStand( );

//	m_PlayerDimSwim = m_PlayerDimNormal;
//	m_PlayerDimSwim.sizeCollider.z = .55f;
//	m_PlayerDimSwim.heightEye = .65f;


//	m_PlayerDimSwim = m_PlayerDimNo;
//	m_PlayerDimSwim.sizeCollider.z = .55f;
//	m_PlayerDimSwim.heightEye = .65f;

}	
//----------------------------------------------------------------------------------------------------
/*!sets the player dimension in crouch mode
	@param pDim struct containing the player dimensions
	@see pe_player_dimensions
*/
void CPlayer::SetDimCrouch(const pe_player_dimensions* const pDim)
{
	if(!pDim)
	{
		m_PlayerDimCrouch.heightEye = 1.0f;
		m_PlayerDimCrouch.heightCollider = 0.6f;
		m_PlayerDimCrouch.sizeCollider.Set(0.4f,0.4f,0.5f);
		return;
	}
	m_PlayerDimCrouch = *pDim;
	SetDimStealth();
	m_CurStance = eNone;
	GoStand( );
}

//----------------------------------------------------------------------------------------------------
/*!sets the player dimension in prone mode
	@param pDim struct containing the player dimensions
	@see pe_player_dimensions
*/
void CPlayer::SetDimProne(const pe_player_dimensions* const pDim)
{
	if(!pDim)
	{
		m_PlayerDimProne.heightEye = 0.5f;
		m_PlayerDimProne.heightCollider = 0.3f;
		m_PlayerDimProne.sizeCollider.Set(0.4f,0.4f,0.2f);
		return;
	}
	m_PlayerDimProne = *pDim;
	m_PlayerDimSwim = m_PlayerDimProne;
	m_PlayerDimSwim.sizeCollider.z = .55f;
	m_PlayerDimSwim.heightCollider = .75f;
	m_PlayerDimSwim.heightEye = .70f;
	m_CurStance = eNone;
	GoStand( );
}

//////////////////////////////////////////////////////////////////////////
int CPlayer::GetBoneHitZone( int boneIdx ) const
{
	return  m_pEntity->GetBoneHitZone(boneIdx);
}

//////////////////////////////////////////////////////////////////////////
Vec3d CPlayer::CalcTangentOnEnviroment( const Vec3d	&angle )
{
Vec3d	forward = angle;
Vec3d tangent=forward;
	if(!m_pEntity->GetPhysics())
		return tangent;
	pe_status_living pStat;
	m_pEntity->GetPhysics()->GetStatus(&pStat);

//	Vec3d		forward = m_pEntity->GetAngles();
//	Vec3d		normal = (Vec3d)pStat.groundSlope;
	Vec3d		normal = m_vProneEnvNormal;

	forward=ConvertToRadAngles(forward);
	forward = normal.Cross( forward );
	forward = forward.Cross( normal );

	forward.Normalize();
	normal.Normalize();

	Vec3d vA[3];

	vA[1] = -forward;
	vA[0] = normal.Cross(forward);
	vA[2] = normal;


	matrix3x3RMf &mtrx((matrix3x3RMf&)(*(float*)vA));
	mtrx.Transpose();
	
	tangent = Ang3::GetAnglesXYZ(mtrx);

  Ang3 angles=tangent;
	angles.Rad2Deg();
  tangent=angles;

	return tangent;
}

//----------------------------------------------------------------------------------------------------
//
//! returns arm damage in percent 100 - (arm_health/max_arm_health)*100
int CPlayer::GetArmDamage(void) const
{
//TRACE("ARM damage %d", 100 - ((float)m_stats.armHealth/(float)m_stats.maxArmHealth)*100.0f);
	return 100 - (int)(((float)m_stats.armHealth/(float)m_stats.maxArmHealth)*100.0f);
}

//! returns leg damage in percent 100 - (leg_health/max_leg_health)*100
int CPlayer::GetLegDamage(void) const
{
	return 100 - (int)((m_stats.legHealth/m_stats.maxLegHealth)*100.0f);
}


// 
//!return true if the player is colliding with something
bool CPlayer::HasCollided( )
{
	IPhysicalEntity *pent = m_pEntity->GetPhysics();
	if(!pent)
		return false;

	if (pent->GetType()==PE_LIVING)
	{
		// Create a new status object.  The fields are initialized for us
		pe_status_living status;

		// Get new player status from physics engine
		pent->GetStatus(&status);
		if (!status.bFlying)
			return true;

		return false;
	}
	else 
	{
		pe_status_dynamics sd;
		pent->GetStatus(&sd);
		float fMassTot=sd.mass, fMassColl=0;
		int nColls,i,j;

		coll_history_item	item[8];
		pe_status_collisions sc;
		sc.pHistory = item;
		sc.len = 8;
		sc.age = 0.5f;
		nColls = pent->GetStatus(&sc);
		for(i=0; i<nColls; i++) 
		{
			for(j=0;j<i && item[j].partid[0]!=item[i].partid[0];j++);
			if (j==i) 
			{
				sd.partid = item[i].partid[0];
				pent->GetStatus(&sd);
				fMassColl += sd.mass;
			}
		}
		return fMassColl>fMassTot*0.3f;
	}
}


//-----------------------------------------------------------------------------
void CPlayer::StartDie( const Vec3d& hitImpuls, const Vec3d hitPoint, int hitpartid, const int deathType )
{
	//

	if(!m_pGame->IsMultiplayer())
	{
		float	deathTimer=m_pGame->p_deathtime->GetFVal();
		m_fDeathTimer = deathTimer;
	}	else
		m_pGame->m_DeadPlayers.push_back(this);

//*/

	if(IsMyPlayer())
	{

/*
		// if it was first person view - go to third person
		if(m_bFirstPerson)
			m_pGame->SetViewMode(true);

		Vec3d camPos = m_pEntity->GetCamera()->GetPos();
		float fWaterLevel=m_p3DEngine->GetWaterLevel(&camPos);
		if( fWaterLevel>m_pEntity->GetCamera()->GetPos().z )
		{
			camPos.z = fWaterLevel + 1;
            m_pEntity->GetCamera()->SetPos( camPos );
		}
*/

		InitCameraTransition( PCM_CASUAL );

		IAISystem *pAISystem =m_pGame->GetSystem()->GetAISystem();
		if (pAISystem)
		{
			if (pAISystem->GetAutoBalanceInterface())
				pAISystem->GetAutoBalanceInterface()->RegisterPlayerDeath();
		}
	}

	m_sPrevAniName.clear();
	m_sPrevAniNameLayer1.clear();
	m_sPrevAniNameLayer2.clear();
	m_pEntity->ResetAnimations( 0 );
	SwitchFlashLight( false );		// turn off flashlight

	IPhysicalEntity *pent = m_pEntity->GetPhysics();
	IEntityCharacter *ichar = m_pEntity->GetCharInterface();
	ICryCharInstance *character = ichar->GetCharacter(PLAYER_MODEL_IDX);

	// Hide weapon.
	DeselectWeapon();
	if (character)
	{
		hitpartid = character->TranslatePartIdToDeadBody(hitpartid);
	}
	
	if (pent)
	{
		if (pent->GetType()==PE_LIVING)
		{
			pe_action_move am;
			am.dir = hitImpuls/100.0f;
			am.iJump = 2;
			pent->Action(&am);
		}
		else if (hitpartid>=0)
		{
			pe_action_impulse ai;
			ai.partid = hitpartid;
			if (hitPoint.len2()>0)
				ai.point = hitPoint;
			ai.impulse = hitImpuls;
			pent->Action(&ai);
		}
	}

//	m_deathTime = GetCurrTime();
	//	UpdateCharacterAnimations();
	m_pEntity->EnableAI(false);

	m_pEntity->KillTimer();

/*
//	m_deathTime = GetCurrTime();
	

	IEntityCharacter *ichar = m_pEntity->GetCharInterface();
//	UpdateCharacterAnimations();
	
	ICryCharInstance *character = ichar->GetCharacter(PLAYER_MODEL_IDX);
	if (character)
	{
		// Hide weapon.
		character->EnableLastIdleAnimationRestart(0,false);
		character->EnableLastIdleAnimationRestart(1,false);
		character->AttachObjectToBone(NULL,NULL);

		if (m_bIsAI)
			character->ResetAnimations();
	}
	

	if (m_pEntity->GetPhysics()->GetType()==PE_ARTICULATED)
		return;

	// Disable physics on this player.
	m_pEntity->EnablePhysics(false);
*/



}

/*! sets the player dynamics parameters
	@param pDyn a structure containing the dynamics parameters
		air_control
		gravity
		swimming_gravity
		inertia
		swimming_inertia
*/
void CPlayer::SetDynamics(const PlayerDynamics *pDyn)
{
	if (pDyn->air_control!=1E10f) m_Dynamics.air_control = pDyn->air_control;
	if (pDyn->gravity!=1E10f) m_Dynamics.gravity = pDyn->gravity;
	if (pDyn->swimming_gravity!=1E10f) m_Dynamics.swimming_gravity = pDyn->swimming_gravity;
	if (pDyn->inertia!=1E10f) m_Dynamics.inertia = pDyn->inertia;
	if (pDyn->swimming_inertia!=1E10f) m_Dynamics.swimming_inertia = pDyn->swimming_inertia;
	if (pDyn->jump_gravity!=1E10f) m_Dynamics.jump_gravity = pDyn->jump_gravity;
}

void CPlayer::RemoveAllWeapons()
{
	// Takes away the owns flag of all weapons in the player's weapon map
	PlayerWeaponsItor it;
	for (it=m_mapPlayerWeapons.begin(); it!=m_mapPlayerWeapons.end(); it++)
		(* it).second.owns = false;
}

//
//-----------------------------------------------------------------------------------------------
//	returns true if can stand in given position
bool	CPlayer::CanStand( const Vec3& pos)
{
	bool result=true;
	IPhysicalEntity*	phys = m_pEntity->GetPhysics();

	pe_player_dynamics pd;
	phys->GetParams(&pd);
	bool bActive = pd.bActive;
	m_pEntity->ActivatePhysics( true );

	Vec3 curPos=m_pEntity->GetPos();
	pe_params_pos ppos;
	ppos.pos = pos;
	phys->SetParams(&ppos);

	if(!phys->SetParams( &m_PlayerDimCrouch ))
		result=false;
	if(result && !phys->SetParams( &m_PlayerDimNormal ))
		result=false;

	// restore active/position
	m_pEntity->ActivatePhysics( bActive );
	ppos.pos = curPos;
	phys->SetParams(&ppos);
	phys->SetParams( &m_PlayerDimNormal );
//	m_CurStance = eNone;
//	GoStand( );

	return result;
}



//
//-----------------------------------------------------------------------------------------------
//	returns true if switch to stance or already in stance, otherwise return false
bool	CPlayer::GoStand(bool ignoreSpam)
{
	//prevent prone-standing position spamming
	if (!ignoreSpam && m_fLastProneTime > m_pTimer->GetCurrTime())
		return false;

	IPhysicalEntity*	phys = m_pEntity->GetPhysics();
 	if ( phys && m_CurStance != eStand )		//m_currDimensions!=eDimNormal)	
	{
		// Use normal physics dimensions.
		if( m_CurStance == eProne )
			m_pEntity->SetPhysAngles( Vec3(0,0,0) );			// reset angle if proning


		if(phys->SetParams( &m_PlayerDimNormal ))
		{
			//InitCameraTransition( PCM_CASUAL ,true );
			m_AngleLimitBase.Set(0,0,0);
			if(m_pEntity->GetAI())
			{
				m_pEntity->GetAI()->SetEyeHeight(m_PlayerDimNormal.heightEye);
			}
			m_currDimensions = eDimNormal;
			m_PrevStance = m_CurStance;
			m_CurStance = eStand;

			m_stats.aim = false;
			m_stats.crouch = false;
			m_stats.prone = false;
			// [kirill] need this to use only z component of entity angles (same as for drawing)
			// when calculating BBox
			GetEntity()->SetFlags( ETY_FLAG_CALCBBOX_ZROTATE );

#if defined(LINUX64)
			m_pEntity->SendScriptEvent(ScriptEvent_StanceChange, 0);
#else
			m_pEntity->SendScriptEvent(ScriptEvent_StanceChange, NULL);
#endif
			m_bStayCrouch = false;

			m_fLastProneTime = m_pTimer->GetCurrTime() + 0.5f;

			return true;
		}

		// could not change - restore angle
		if( m_CurStance == eProne )			
			m_pEntity->SetPhysAngles( m_EnvTangent );					

		return false;
	}
	else		// was standing already
		m_PrevStance = eStand;

	m_bStayCrouch = false;

	return true;
}


//
//-----------------------------------------------------------------------------------------------
//	returns true if switch to stance or already in stance, otherwise return false
bool	CPlayer::GoStealth( )
{
	IPhysicalEntity*	phys = m_pEntity->GetPhysics();
	if ( phys && m_CurStance != eStealth )		//m_currDimensions!=eDimStealth)
	{
		if( m_CurStance == eProne )
			m_pEntity->SetPhysAngles( Vec3(0,0,0) );			// reset angle if proning

		// Use stealth physics dimensions.
		if(phys->SetParams( &m_PlayerDimStealth ))	//if can stealth here
		{
			//InitCameraTransition( PCM_CASUAL );
			m_AngleLimitBase.Set(0,0,0);
			if(m_pEntity->GetAI())
			{
				m_pEntity->GetAI()->SetEyeHeight(m_PlayerDimStealth.heightEye);
			}
			m_currDimensions = eDimStealth;
			m_PrevStance = m_CurStance;
			m_CurStance = eStealth;

			m_stats.aim = false;
			m_stats.crouch = false;
			m_stats.prone = false;
			// [kirill] need this to use only z component of entity angles (same as for drawing)
			// when calculating BBox
			GetEntity()->SetFlags( ETY_FLAG_CALCBBOX_ZROTATE );

			m_Running = false;
#if defined(LINUX64)
			m_pEntity->SendScriptEvent(ScriptEvent_StanceChange, 0);
#else
			m_pEntity->SendScriptEvent(ScriptEvent_StanceChange, NULL);
#endif
			return true;
		}

		// could not change - restore angle if proning
		if( m_CurStance == eProne )			
			m_pEntity->SetPhysAngles( m_EnvTangent );					
		return false;
	}
	return true;
}

//
//-----------------------------------------------------------------------------------------------
//	returns true if switch to stance or already in stance, otherwise return false
bool	CPlayer::GoCrouch( )
{
	//	don't go to crouch mode if using mounted weapon
	if(m_pMountedWeapon)
		return false;
	// don't change stance in water
	if( m_bSwimming )
		return false;

	IPhysicalEntity*	phys = m_pEntity->GetPhysics();
	if ( phys && m_CurStance != eCrouch	)		//m_currDimensions!=eDimCrouch)
	{
		if( m_CurStance == eProne )
			m_pEntity->SetPhysAngles( Vec3(0,0,0) );			// reset angle if proning

		// Use crouching physics dimensions.
		if(phys->SetParams( &m_PlayerDimCrouch ))
		{
			//InitCameraTransition( PCM_CASUAL, true );
			m_AngleLimitBase.Set(0,0,0);
			if(m_pEntity->GetAI())
			{
				m_pEntity->GetAI()->SetEyeHeight(m_PlayerDimCrouch.heightEye);
			}
			m_currDimensions = eDimCrouch;
			m_PrevStance = m_CurStance;
			m_CurStance = eCrouch;

			m_stats.aim = false;
			m_stats.crouch = true;
			m_stats.prone = false;
			// [kirill] need this to use only z component of entity angles (same as for drawing)
			// when calculating BBox
			GetEntity()->SetFlags( ETY_FLAG_CALCBBOX_ZROTATE );

			m_Running = false;
#if defined(LINUX64)
			m_pEntity->SendScriptEvent(ScriptEvent_StanceChange, 0);
#else
			m_pEntity->SendScriptEvent(ScriptEvent_StanceChange, NULL);
#endif
			return true;
		}
		// could not change - restore angle if proning
		if( m_CurStance == eProne )			
			m_pEntity->SetPhysAngles( m_EnvTangent );					
		return false;
	}
	return true;
}

//
//-----------------------------------------------------------------------------------------------
//	returns true if switch to stance or already in stance, otherwise returns false
bool	CPlayer::GoProne( )
{
	IPhysicalEntity*	phys = m_pEntity->GetPhysics();

	if(!CanProne(false))
		return false;

	if ( phys && m_CurStance != eProne)				//!=eDimProne)
	{

		// Use proning physics dimensions.
		// if can't change dimentions at current position - put player little up
		// and try again
		if(!phys->SetParams( &m_PlayerDimProne ))
		{
			Vec3d pos = m_pEntity->GetPos();
			pos.z += .5f;
			m_pEntity->SetPos( pos );
			if(!phys->SetParams( &m_PlayerDimProne ))
			{
				// restore position
				pos.z -= .5f;
				m_pEntity->SetPos( pos );
				return false;
			}
		}

		//
		Vec3d	ang = m_pEntity->GetAngles();
		m_EnvTangent = CalcTangentOnEnviroment( m_pEntity->GetAngles() );
		m_pEntity->SetPhysAngles( m_EnvTangent );			// set angels for phisics (body is flat on surfece tangent space)

		//InitCameraTransition( PCM_CASUAL, true );
		if(m_pEntity->GetAI())
		{
			m_pEntity->GetAI()->SetEyeHeight(m_PlayerDimProne.heightEye);
		}
		m_currDimensions = eDimProne;
		m_PrevStance = m_CurStance;
		m_CurStance = eProne;

		m_stats.aim = false;
		m_stats.crouch = false;
		m_stats.prone = true;
		// [kirill] when in prone - use all the rotations (same as for drawing)
		// when calculating BBox
		GetEntity()->ClearFlags( ETY_FLAG_CALCBBOX_ZROTATE );

		m_Running = false;
#if defined(LINUX64)
		m_pEntity->SendScriptEvent(ScriptEvent_StanceChange, 0);
#else
		m_pEntity->SendScriptEvent(ScriptEvent_StanceChange, NULL);
#endif
	}

	m_bStayCrouch = false;

	m_fLastProneTime = m_pTimer->GetCurrTime() + 0.5f;

	return true;
}

//
//-----------------------------------------------------------------------------------------------
//	returns true if switch to stance or already in stance, otherwise returns false
bool	CPlayer::GoSwim( )
{
	IPhysicalEntity*	phys = m_pEntity->GetPhysics();

	if ( phys && m_CurStance != eSwim)				//!=eDimProne)
	{
		//
//		Vec3d	ang = m_pEntity->GetAngles();
//		m_EnvTangent = CalcTangentOnEnviroment( m_pEntity->GetAngles() );
//		m_pEntity->SetPhysAngles( m_EnvTangent );			// set angels for phisics (body is flat on surfece tangent space)

		// Use proning physics dimensions for swimming.
		if(phys->SetParams( &m_PlayerDimSwim ))
		{

			InitCameraTransition( PCM_CASUAL, true );
			m_AngleLimitBase.Set(0,0,0);
			if(m_pEntity->GetAI())
			{
				m_pEntity->GetAI()->SetEyeHeight(m_PlayerDimProne.heightEye);
			}
			m_currDimensions = eDimProne;
			m_PrevStance = m_CurStance;
			m_CurStance = eSwim;

//			m_stats.aim = false;
//			m_stats.crouch = false;
//			m_stats.prone = true;

			m_stats.aim = false;
			m_stats.crouch = false;
			m_stats.prone = false;
			m_stats.prone = false;
			// [kirill] need this to use only z component of entity angles (same as for drawing)
			// when calculating BBox
			GetEntity()->SetFlags( ETY_FLAG_CALCBBOX_ZROTATE );

			m_Running = false;
#if defined(LINUX64)
			m_pEntity->SendScriptEvent(ScriptEvent_StanceChange, 0);
#else
			m_pEntity->SendScriptEvent(ScriptEvent_StanceChange, NULL);
#endif
			m_bStayCrouch = false;

			return true;
		}
		return false;
	}

	m_bStayCrouch = false;

	return true;
}

//
//-----------------------------------------------------------------------------------------------
//	returns true if slope angle is ok to be in prone (not too steep) AND not falling/jumping
bool	CPlayer::CanProne(bool ignoreSpam)
{
	float	slopeProneLimit=.7f;

	//	don't go to prone mode if using mounted weapon
	if(m_pMountedWeapon)
		return false;

	//prevent prone-standing position spamming
	if (!ignoreSpam && m_fLastProneTime > m_pTimer->GetCurrTime())
		return false;

	//	don't go to prone mode if falling/jumping
	if(m_FlyTime>.5f && m_CurStance!=eSwim)
		return false;

	if(!m_pEntity->GetPhysics())
		return false;

	pe_status_living pStat;
	m_pEntity->GetPhysics()->GetStatus(&pStat);
	m_vProneEnvNormal = (Vec3d)pStat.groundSlope;
	ray_hit hit;
	Vec3d pos=m_pEntity->GetPos();
	int hitCount=0;		
	float fHitCount=1.0f;
	if( m_vProneEnvNormal.z>slopeProneLimit )
		++hitCount;

	pos.z += .3f;
	pos.x += .1f;
	if (m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection( pos, vectorf(0,0,-1.5f), 
		ent_all-ent_independent, rwi_stop_at_pierceable | geom_colltype_player<<rwi_colltype_bit,&hit,
		1, GetEntity()->GetPhysics()))
	{
		m_vProneEnvNormal += hit.n;
		++fHitCount;
		if( hit.n.z>slopeProneLimit )
			++hitCount;
	}
	pos.x -= .2f;
	if (m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection( pos, vectorf(0,0,-1.5f), 
		ent_all-ent_independent, rwi_stop_at_pierceable | geom_colltype_player<<rwi_colltype_bit,&hit,
		1, GetEntity()->GetPhysics()))
	{
		m_vProneEnvNormal += hit.n;
		++fHitCount;
		if( hit.n.z>slopeProneLimit )
			++hitCount;
	}
	pos.x += .1f;
	pos.y += .1f;
	if (m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection( pos, vectorf(0,0,-1.5f), 
		ent_all-ent_independent, rwi_stop_at_pierceable | geom_colltype_player<<rwi_colltype_bit,&hit,
		1, GetEntity()->GetPhysics()))
	{
		m_vProneEnvNormal += hit.n;
		++fHitCount;
		if( hit.n.z>slopeProneLimit )
			++hitCount;
	}
	pos.y -= .2f;
	if (m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection( pos, vectorf(0,0,-1.5f), 
		ent_all-ent_independent, rwi_stop_at_pierceable | geom_colltype_player<<rwi_colltype_bit,&hit,
		1, GetEntity()->GetPhysics()))
	{
		m_vProneEnvNormal += hit.n;
		++fHitCount;
		if( hit.n.z>slopeProneLimit )
			++hitCount;
	}

	// there are no points with allowed slope around - can't prone here
	if( hitCount<1 )
		return false;

	m_vProneEnvNormal = m_vProneEnvNormal/fHitCount;

	// can't prone when standing on boat/car
	IPhysicalEntity *physEnt = m_pEntity->GetPhysics();
	if (physEnt)
	{
		pe_status_living status;
		if (physEnt->GetStatus(&status))
		{
			if(status.pGroundCollider)
			{
				IEntity* pEnt=(IEntity*)status.pGroundCollider->GetForeignData();
				if(pEnt && pEnt->GetContainer())
				{
					CVehicle* pVehicleBelow;
					if(pEnt->GetContainer()->QueryContainerInterface(CIT_IVEHICLE,(void**)&pVehicleBelow))
					{
						return false;
					}
				}
			}
		}
	}

	return true;
}


//
//-----------------------------------------------------------------------------------------------
//	returns true if switch to stance or already in stance, otherwise return false
bool	CPlayer::GoRelaxed( )
{
	if(!GoStand())
	{
		return false;
	}

	m_PrevStance = m_CurStance;
	m_CurStance = eRelaxed;

	return true;
}



//
//-----------------------------------------------------------------------------------------------
bool	CPlayer::RestorePrevStence()
{
	switch( m_PrevStance )
	{
		case eStand:
		case eSwim:
			return GoStand();
		case eStealth:
			return GoStealth();
		case eCrouch:
			return GoCrouch();
		case eProne:
			return GoProne();
	}
	return false;
}



//
//-----------------------------------------------------------------------------------------------
//	for debug/test purposes - 
//////////////////////////////////////////////////////////////////////////
void	CPlayer::StartFire()
{
	Vec3d	pos = m_pEntity->GetPos();	
}

//////////////////////////////////////////////////////////////////////////
/*void CPlayer::PlaySound(ISound * pSound, float fSoundScale, Vec3d &Offset)
{
	if(IsMyPlayer())
	{
		m_lstAttachedSounds.push_back(SAttachedSound(pSound, Offset));
		pSound->SetPosition(CalcSoundPos());
	}
	else
	{
		pSound->SetPosition(m_pEntity->GetPos());
	}
	pSound->Play(fSoundScale);
}*/

Vec3d CPlayer::CalcSoundPos()
{
	IEntityCamera *pEC;
	if (IsMyPlayer() && (pEC=m_pEntity->GetCamera()))
	{
		CCamera &cam=pEC->GetCamera();
		Vec3d vPos=cam.GetPos();
		vPos+=cam.m_vOffset;
		Vec3d vAngles=cam.GetAngles();
		Vec3d vTrans=vPos-cam.GetPos();
		Matrix44 vRot;
		vRot.SetIdentity();		
		//rotate the matrix ingnoring the Y rotation
		//vRot.RotateMatrix(Vec3d(vAngles.x,0,vAngles.z));		
		vRot=Matrix44::CreateRotationZYX(-Vec3d(vAngles.x,0,vAngles.z)*gf_DEGTORAD)*vRot; //NOTE: angles in radians and negated 
		vPos=vRot.TransformPointOLD(vTrans);
		//translate back
		vPos+=cam.GetPos();
		return vPos;
	}
	return m_pEntity->GetPos();
}


//	calculates vertical and horizontal speed to make player jump on dist and height
void CPlayer::CalcJumpSpeed( float dist, float height, float &horV, float &vertV  )
{
	assert(height>=0.0f);
//	vz = srtq(2*g*h) 
//	vx = len*g/(2*vz) 
	float fGravity;

	//we are using a different gravity for jump? use it to calculate jump speeds.
	if (m_Dynamics.jump_gravity!=1E10)
		fGravity = m_Dynamics.jump_gravity;
	else
		fGravity = m_Dynamics.gravity;

	fGravity *= m_pGame->p_gravity_modifier->GetFVal();

	vertV = cry_sqrtf( 2.0f*fGravity*height );
//	horV = dist*m_Dynamics.gravity/( 2.0f*vertV );

	if(height!=0.0f) horV=(dist*cry_sqrtf(fGravity)) / (2*cry_sqrtf(2*height));
		else horV=0.0f;
}

void CPlayer::SetViewMode(bool bThirdPerson)
{
	if(!IsMyPlayer())
		return;

	if( !bThirdPerson )
	{
		m_pEntity->DrawCharacter(0, 0);
		m_pEntity->NeedsUpdateCharacter(0, true);
	}
	else 
	{
		if (m_stats.bModelHidden)
			m_pEntity->DrawCharacter(0, 0);
		else
			m_pEntity->DrawCharacter(0, 1);
	}


//	if( !bThirdPerson && IsMyPlayer() )
//	{
//		m_pEntity->DrawCharacter(0, 0);
//		m_pEntity->NeedsUpdateCharacter(0, true);
//	}
}


/*
//	calcualtes aproximate point of lending for granade on terrain
//	firePos		- start point
//	dir				- normolized direction 
//	vel				- velocity
//	timeStep	- time interval for iteration
//	timeLimit	- time to stop iterations if no collision with terrain is detected
Vec3d	CPlayer::TraceGrenade( const Vec3d& firePos, const Vec3d& dir, const float vel, 
																const float timeStep, const float timeLimit)
{
Vec3d curPos;
float	time = 0.0f;
	curPos = firePos;
	while( curPos.z > m_p3DEngine->GetTerrainElevation(curPos.x, curPos.y) && time<timeLimit)
	{
		curPos.x = time*dir.x*vel + firePos.x;
		curPos.y = time*dir.y*vel + firePos.y;
		curPos.z = m_Dynamics.gravity*time*time/2.0f + dir.z*vel*time + firePos.z;
		time += timeStep;
	}

	curPos.z = m_p3DEngine->GetTerrainElevation(curPos.x, curPos.y);
	return curPos;
}
*/

void CPlayer::SwitchFiremode(int nforce)
{
	if (m_nSelectedWeaponID != -1)
	{	
		WeaponParams temp;
		bool bChanged = true, bCanSwitch;
		int iNewFireMode;
		CWeaponClass* pSelectedWeapon = GetSelectedWeapon();
		WeaponInfo &wi = GetWeaponInfo();

		if(nforce==-1)
		{
			// Check what the next firemode would be, bChanged will be false in case
			// we can't switch to another mode
			iNewFireMode = pSelectedWeapon->GetNextFireMode(wi.iFireMode, m_bIsAI);
		}
		else
		{
			iNewFireMode=nforce;
		}

		bChanged = (wi.iFireMode != iNewFireMode);

		if (bChanged && (!m_stats.reloading) && (!m_stats.weapon_busy))
		{
			// Call the script to find out if we can switch
			_SmartScriptObject pObj(m_pScriptSystem);
			pObj->SetValue( "firemode",iNewFireMode);
			pSelectedWeapon->ScriptOnStopFiring(m_pEntity);
			m_stats.firemode=iNewFireMode;
			m_pEntity->SendScriptEvent(ScriptEvent_FireModeChange, *pObj, &bCanSwitch);

			// Apply the change to the C++ state
			wi.iFireMode = iNewFireMode;
		}
	}
}

void	CPlayer::CounterAdd( const string name, const float timeScale )
{
	CountersMap::iterator	counter = m_UpdatedCounters.find( name );
	if( counter == m_UpdatedCounters.end() )
	{
		couterEntry newEntry;
		newEntry.scale = timeScale;
		m_UpdatedCounters[name] = newEntry;
	}
	else
	{
		(counter->second).scale = timeScale;	
	}
}

void	CPlayer::CounterIncrement( const string name, const float value )
{
	CountersMap::iterator	counter = m_UpdatedCounters.find( name );
	if( counter == m_UpdatedCounters.end() )
		return;
	(counter->second).value += value;
}

float CPlayer::CounterGetValue( const string name )
{
	CountersMap::iterator	counter = m_UpdatedCounters.find( name );
	if( counter == m_UpdatedCounters.end() )
		return 0.0f;

	return (counter->second).value;

}

void	CPlayer::CounterUpdateAll( const float dt )
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );
	CountersMap::iterator	counter = m_UpdatedCounters.begin( );
	for(; counter!=m_UpdatedCounters.end(); counter++ )
	{
		float	prevValue = (counter->second).value;
		(counter->second).value += (counter->second).scale*dt;

		if( prevValue<(counter->second).eventTrhld && (counter->second).value>(counter->second).eventTrhld )
		{
			IAIObject *pObject = m_pEntity->GetAI();
			if (pObject)
			{
				pObject->SetSignal(1, (counter->second).eventToCall.c_str());
			}
		}
		if((counter->second).value < 0.0f)
			(counter->second).value = 0.0f;
	}
}

void	CPlayer::CounterSetValue( const string name, const float value )
{
	CountersMap::iterator	counter = m_UpdatedCounters.find( name );
	if( counter == m_UpdatedCounters.end() )
		return;

	(counter->second).value = value;
}

void	CPlayer::CounterSetEvent( const string name, const float thrhld, const string eventName )
{
	CountersMap::iterator	counter = m_UpdatedCounters.find( name );
	if( counter == m_UpdatedCounters.end() )
		return;
	(counter->second).eventToCall = eventName;
	(counter->second).eventTrhld = thrhld;
}

// attaches weapon to weapon bone on character's back
void CPlayer::HolsterWeapon(void)
{
	SetWeaponPositionState(WEAPON_POS_HOLSTER);
}

// rebinds weapon to be bound to the weapon bone in the characters hands
void CPlayer::HoldWeapon(void)
{
	SetWeaponPositionState(WEAPON_POS_HOLD);
}

void CPlayer::SetWeaponPositionState(EWeaponPositionState weaponPositionState)
{
	assert (IsHeapValid());

	// attach to bone ?
	if (m_nSelectedWeaponID != -1 && m_weaponPositionState != weaponPositionState)
	{
		CWeaponClass* pSelectedWeapon = GetSelectedWeapon();
		m_weaponPositionState = weaponPositionState;
		ICryCharInstance *character = m_pEntity->GetCharInterface()->GetCharacter(PLAYER_MODEL_IDX);
		WeaponInfo &wi = GetWeaponInfo();

		assert (IsHeapValid());
		if (character)
		{
			wi.DetachBindingHandles(character);

			if (weaponPositionState == WEAPON_POS_UNDEFINED)
				return;

//			ValidateHeap();
			string sBone = pSelectedWeapon->GetBindBone();
//			ValidateHeap();

			if (!sBone.empty())
			{
//				ValidateHeap();
				if (weaponPositionState == WEAPON_POS_HOLSTER)
					sBone+="02";

//				ValidateHeap();
				wi.hBindInfo = character->AttachObjectToBone( pSelectedWeapon->GetObject(), sBone.c_str() );
//				ValidateHeap();

				// if there is an auxilary weapon bone, attach to that as well
				string sAuxBone = "aux_" + sBone;
				if (character->GetModel()->GetBoneByName(sAuxBone.c_str())>=0)
				{
					wi.hAuxBindInfo = character->AttachObjectToBone( pSelectedWeapon->GetObject(), sAuxBone.c_str() );
				}
			}
			else
			{
				character->DetachAll();
			}
		}
	}
}

//
//
void CPlayer::SetBlendTime(const char *sAniName, float fBlendTime)
{
	if(strlen(sAniName)<3)	// it has to be some name
		return;
	m_AniBlendTimes[sAniName] = fBlendTime;
}

void	CPlayer::GetBlendTime(const char *sAniName, float&fBlendTime)
{
	BlendTimesMap::iterator curAni=m_AniBlendTimes.find(sAniName);
	if( curAni == m_AniBlendTimes.end() )
		return;
	fBlendTime = (curAni->second);
}

void	CPlayer::UpdateCollisionDamage( )
{
	// Get a pointer to the physics engine
	IPhysicalEntity *physEnt = m_pEntity->GetPhysics();
	if (!physEnt)
		return;

	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	// Create a new status object.  
	pe_status_collisions status;
	coll_history_item		 history[4];
	memset( history, 0, sizeof(history) );
	status.pHistory = history;
	status.len = 4;
	status.bClearHistory = true;
	status.age = .1f;
	int	colN=0,imax;
	float	massLimitMin=50.0f;
	float	massLimitMax=80.0f;
	float	velLimit=3;//10;//6.5f;
	if ((m_timeDmgCollision+=m_pTimer->GetFrameTime())>2.0f)
	{
		m_timeDmgCollision = 0; m_pPrevDmgCollider = 0;
	}
	
	// Get new player status from physics engine
	if (colN=physEnt->GetStatus(&status))
	{
		float	damageValue = 0.0f;
		//find the most damaging collision in history

		for(imax=-1 ; colN>0; colN--)
		{
			float	curVel = max(0.0f,history[colN-1].v[1]*history[colN-1].n);
			if(history[colN-1].mass[1]<=massLimitMin || curVel<velLimit)
				continue;
			float	curDamage = (history[colN-1].mass[1]<massLimitMax ? history[colN-1].mass[1] : massLimitMax)*curVel;
			IPhysicalEntity *pCollider = m_pPhysicalWorld->GetPhysicalEntityById(history[colN-1].idCollider);
			if (pCollider==m_pPrevDmgCollider && curDamage<m_prevCollDamage*1.2f)
				continue;
			if(damageValue < curDamage)
				damageValue = curDamage, imax = colN-1;
		}
		//m_pGame->GetSystem()->GetILog()->Log( "\002 %d collision %s %.2f %.2f ", colN, GetName(), history[0].collMass, history[0].vrel.len() );
		if(damageValue>=1.0f)
		{
			_SmartScriptObject pTable(m_pScriptSystem,false);
			CScriptObjectVector oDir(m_pScriptSystem); oDir=history[imax].n;
			pTable->SetValue("dir",*oDir);
			pTable->SetValue("damage",damageValue);
			pTable->SetValue("collider_mass",history[imax].mass[1]);
			pTable->SetValue("collider_velocity",-history[imax].v[1]*history[imax].n);
			IPhysicalEntity *pCollider = m_pPhysicalWorld->GetPhysicalEntityById(history[imax].idCollider);
			if (pCollider)
			{
				IEntity *pEntCollider = (IEntity*)pCollider->GetForeignData();
				if (pEntCollider)
					pTable->SetValue("collider",pEntCollider->GetScriptObject());
			}

			m_pEntity->SendScriptEvent(	ScriptEvent_PhysCollision , pTable);
//m_pGame->GetSystem()->GetILog()->Log( "\002 collision %s %.1f %d %.1f ", GetName(), damageValue, m_stats.health, maxVel);
			m_pPrevDmgCollider = pCollider;
			m_prevCollDamage = damageValue;
			m_timeDmgCollision = 0;
		}
	}
}



void	CPlayer::SetSpeedMult( float run, float crouch, float prone, float xrun, float xwalk, float rrun, float rwalk )
{
	m_RunSpeedScale			= run;
	m_CrouchSpeedScale	= crouch;
	m_ProneSpeedScale		= prone;
	m_XRunSpeedScale		= xrun;		// stealth
	m_XWalkSpeedScale		= xwalk;	
	m_RRunSpeedScale		= rrun;		// relaxed
	m_RWalkSpeedScale		= rwalk;
}


//
//-----------------------------------------------------------------


void	CPlayer::OnDrawMountedWeapon( const SRendParams & RendParams )
{
	if(m_nSelectedWeaponID == -1)
		return;

	// if the localplayer is using the mounted weapon, only then do we disable the first person weapon drawing
	if( m_pMountedWeapon && IsMyPlayer())
	{
		m_pMountedWeapon->DrawObject(0,ETY_DRAW_NORMAL);
		m_pMountedWeapon->DrawCharacter(0,ETY_DRAW_NORMAL);

		Vec3d apos, aang;
		GetFirePosAngles( apos, aang );

		apos = m_pMountedWeapon->GetAngles(1);
	//	if(IsMyPlayer())
			m_pMountedWeapon->SetAngles( aang, false, false, true );
	//	else
	//		m_pMountedWeapon->SetAngles( m_pEntity->GetAngles(), false, false, true );

		m_pMountedWeapon->ForceCharacterUpdate(0);
		m_pMountedWeapon->DrawEntity(RendParams);

		m_pMountedWeapon->SetAngles( apos );
		m_pMountedWeapon->DrawObject(0,ETY_DRAW_NONE);		// we don't want to draw it (already drawn by player)
		m_pMountedWeapon->DrawCharacter(0,ETY_DRAW_NONE);
		m_pMountedWeapon->NeedsUpdateCharacter( 0, true);	// but we want to update animation for it

		GetEntity()->DrawCharacter(1, ETY_DRAW_NONE);
	}
	return;
}


void CPlayer::UpdateCharacterAnimationsMounted( SPlayerUpdateContext &ctx )
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	m_pEntity->SetAnimationSpeed( 1.0f );
	if( m_pVehicle )	// mounted weapon on vehicle - don't do any animation
		return;

	float	diff = m_pEntity->GetAngles().z - m_vPrevMntPos.z;
	float	vel = diff/m_pTimer->GetFrameTime();

//if(diff!=0)
//m_pGame->m_pLog->Log(" %.2f  %.2f  %.2f\n", diff, vel, m_pEntity->GetAngles().z );

	string	aniName;
/*
	if(fabsf(vel)>50.6f)	// too slow - no animations
	{
		m_RunningTime += m_pTimer->GetFrameTime();
	}
	else
	{
		m_RunningTime = 0;
	}
*/
//	if( m_RunningTime<.1f )
	if( fabsf(vel)<60.6f )
		aniName = "mount_fwd";
	else 
	{
		if( diff<0 )
			aniName = "mount_left";//"arotateright";
		else
//			aniName = "srotateleft";
			aniName = "mount_right";//"arotateleft";

//		float	speedScale = 1 + (fabsf(vel)-60)*.5f;
//		m_pEntity->SetAnimationSpeed( speedScale );
	}

	m_RunningTime += m_pTimer->GetFrameTime();
	if(m_sPrevAniName != aniName)
	{
		if(aniName == "mount_fwd")
		{
			if( m_MntWeaponStandTime>.3f )
			{
				if(m_pEntity->StartAnimation(0, aniName.c_str(), 0, .25f ))
					m_sPrevAniName = aniName;
//				m_RunningTime = 0;
			}
			m_MntWeaponStandTime += m_pTimer->GetFrameTime();
		}
		else
		{
			if(m_pEntity->StartAnimation(0, aniName.c_str(), 0, .25f ))
				m_sPrevAniName = aniName;
			m_MntWeaponStandTime = 0.0f;
		}
	}
	m_vPrevMntPos = m_pEntity->GetAngles();
	return;

/*




//m_pEntity->StartAnimation(0, "awalkback", 0, .1 );
//return;
Vec3d	diff = m_pEntity->GetPos() - m_vPrevMntPos;
//Ang3	playerDir = GetActualAngles();
Vec3d	playerDir = m_pEntity->GetAngles();

	playerDir = ConvertToRadAngles(playerDir);
	playerDir.z=0;
	playerDir.normalize();
	diff.z=0;
	float	dist = diff.len();
	float	vel = dist/m_pTimer->GetFrameTime();
	diff.normalize();

	float dotz = playerDir.x*diff.x + playerDir.y*diff.y;
	float crossz = playerDir.x * diff.y - playerDir.y *  diff.x;
	string	aniName;

	if(vel<.6f)	// too slow - no animations
	{
		m_NotRunTime += m_pTimer->GetFrameTime();
	}
	else
	{
		m_NotRunTime = 0;
	}


	if( m_NotRunTime>.2f )//|| fabsf(crossz)<.3 )
		aniName = "sidle";
	else if( crossz<0 )
		aniName = "awalkfwd";//"arotateright";
	else
		aniName = "srotateleft";
//		aniName = "swalkback";//"arotateleft";






	if(m_sPrevAniNameLayer1 != aniName)
		if(m_pEntity->StartAnimation(0, aniName.c_str(), 0, .1 ))
			m_sPrevAniNameLayer1 = aniName;

	m_vPrevMntPos = m_pEntity->GetPos();
	return;
*/
}

void	CPlayer::GiveBinoculars(bool val)
{
	if (m_stats.has_binoculars == val)
		return;

	m_stats.has_binoculars = val;

	// FIXME ... if the binoculars are take away, then we should reset the viewlayer (must be done 
	// on the script side
}

void CPlayer::PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime)
{
	int nRecursionLevel = (int)m_pGame->GetSystem()->GetIRenderer()->EF_Query(EFQ_RecurseLevel) - 1;
	if(m_bFirstPerson && !nRecursionLevel && m_stats.drawfpweapon	&& m_nSelectedWeaponID != -1)
		return;

	ICryCharInstance *pChar = m_pEntity->GetCharInterface()->GetCharacter(0);
	if (!pChar)
		return;

	if (!(pChar->GetFlags() & CS_FLAG_DRAW_MODEL) && !nRecursionLevel)
		return;

	float fDist = fPrevPortalDistance + vPrevPortalPos.GetDistance(m_pEntity->GetPos());
	pChar->PreloadResources(fDist, 1.f, 0);
}

void	CPlayer::SetStaminaTable( const StaminaTable& stTable )
{
	m_StaminaTable = stTable;
}


void	CPlayer::UpdateStamina( float dTime )
{
	// stamina (for sprint run) update
	// update breath (blue bar)
	if(m_stats.underwater>0.0f)			//we are under water - decrease breath level
	{
		if(m_stats.stamina>0)		// if there is some stamina - 
		{
			if((m_stats.stamina-=dTime*m_StaminaTable.BreathDecoyUnderwater)<0)
				m_stats.stamina = 0;
		}
	}
	else if( m_stats.holding_breath )	// we are sniping/holding breath - decrease breath level
	{
		if((m_stats.stamina-=dTime*m_StaminaTable.BreathDecoyAim)<1.0f)
			m_stats.stamina = 1.0f;
	}
	// update stamina (yellow bar)
	else if( m_Sprinting )
	{
		if((m_stats.stamina -= dTime*m_StaminaTable.DecoyRun)<1.0f)
			m_stats.stamina = 1.0f;
	}
	// restore stamina - player is resting
	else if(m_stats.stamina<100 && (!m_stats.flying || m_bSwimming&&m_stats.underwater<=0.0f ))
	{
		if(m_Running)
			m_stats.stamina += dTime*m_StaminaTable.RestoreRun;
		else
			m_stats.stamina += dTime*m_StaminaTable.RestoreWalk;
		if(m_stats.stamina>100)
			m_stats.stamina = 100;
	}
	// to indicate stamina on HUD 
	m_StaminaTable.StaminaHUD = m_stats.stamina*0.01f;
}

//////////////////////////////////////////////////////////////////////////
bool CPlayer::IsVisible() const
{
	if(m_pGame->IsServer() && m_pGame->IsMultiplayer())
		return true;

	if (m_pEntity)
	{
		// If was rendered recently.
		int nCurrRenderFrame = (int)m_pGame->GetSystem()->GetIRenderer()->GetFrameID();
		if (abs(nCurrRenderFrame-m_pEntity->GetDrawFrame(0)) < 3)
			return true;
	}
	return false;
}



//////////////////////////////////////////////////////////////////////////
void CPlayer::UpdateSwimState(bool bAlive)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );
	// check if the player goes underwater.
	//if (m_p3DEngine->GetWaterLevel(&m_vEyePos)>m_vEyePos.z)
	float fWaterLevel=m_p3DEngine->GetWaterLevel(m_pEntity);
	m_stats.fInWater=fWaterLevel-m_pEntity->GetPos().z;
	if (m_stats.fInWater<0.0f)
		m_stats.fInWater=0.0f;
	Vec3 vPos=m_vEyePos;
	if (!bAlive && !IsMyPlayer())
	{		
		SetEyePosBone();
		vPos=m_vEyePos;
		//vPos=m_pEntity->GetPos();		
	}
	
	if (fWaterLevel>vPos.z)
	{
		// if that is the case, start the EAX underwater effect
		// plus increase the underwater time
		m_stats.underwater+=m_pTimer->GetFrameTime();
		if (!m_bIsAI)
			if(m_pGame->GetSystem()->GetISoundSystem())
				 m_pGame->GetSystem()->GetISoundSystem()->SetEaxListenerEnvironment(EAX_PRESET_UNDERWATER); 

		// also call the script function to play the water splash sounds and
		// other stuff it this is the first time it goes underwater
		if (!m_bEyesInWater)
		{
			m_pEntity->SendScriptEvent(ScriptEvent_EnterWater,0);
			m_bEyesInWater=true;
		}		
	}
	else
	{
		// remove EAX underwater, if he was in the water before
		if ((m_stats.underwater>0.001f) && (!m_bIsAI)) // epsilon
      if(m_pGame->GetSystem()->GetISoundSystem())
			  m_pGame->GetSystem()->GetISoundSystem()->SetEaxListenerEnvironment(EAX_PRESET_OFF);
		m_stats.underwater=0.0f;
		m_bEyesInWater=false;
	}
	m_stats.fKWater = 0;
	// no need for anymore checks if dead - only eyesInWater updated
	if(!IsAlive())
	{
		m_bSwimming = false;
		return;
	}
	// ladder is priority to swim
	if(m_stats.onLadder)
	{
		m_bSwimming = false;
		return;
	}

	// in vehicle is priority to swim
	if(m_pVehicle)
	{
		m_bSwimming = false;
		return;
	}

	// not in water
	if(fWaterLevel < -500)
	{
		m_bSwimming = false;
		return;
	}


//	if(m_stats.fInWater<)
/*
//[kirill] 
//fixme 
//remove this
//the check is for cathching strange positon outorrange problem
//if( m_pGame->pa_blend2->GetFVal()>0.0f )
{
Vec3d pos = m_pEntity->GetPos();
if(pos.z<-100 || pos.z>500 ||
   pos.x<=0 || pos.x>10000 ||
   pos.y<=0 || pos.y>10000
   )
{
if(IsAlive())
m_pGame->m_pLog->Log("\006 alive player < %s > position is invalid %.2f %.2f %.2f \n", m_pEntity->GetName(), pos.x, pos.y, pos.z );
else
m_pGame->m_pLog->Log("\006 dead player < %s > position is invalid %.2f %.2f %.2f \n", m_pEntity->GetName(), pos.x, pos.y, pos.z );
}
}
//remove this over
*/
	
	// to use value depending on current stance
	float	depthOffset;// = GetEntity()->GetCamera()->GetCamOffset().z;
	//AIs don't have camera offset - use fixed value (could use eyeHeight)
	if( m_bIsAI )
		depthOffset = 1.5f;
	else
	{
		switch( m_CurStance )
		{
		case eProne:
		case eSwim:
			depthOffset = .45f;
			break;
//		case eStand:
		default:
			depthOffset = 1.5f;
			break;
		}
	}
	//check if we are standing on something
	pe_status_living status;
	IPhysicalEntity *physEnt = m_pEntity->GetPhysics();	
	if (!physEnt || !physEnt->GetStatus(&status))
	{
		GameWarning( "Bad Phyics Entity for Player %s",m_pEntity->GetName() );
		return;
	}
	if (status.pGroundCollider)
	{
		if( status.groundHeight+depthOffset > fWaterLevel )
		{
			m_bSwimming = false;
			return;
		}
	}
	//
	if(m_pEntity->GetEntityVisArea()==0)	// if outdoors - check how deep we are
	{
		Vec3 rPos = m_pEntity->GetPos();
		bool bChanged = false;
		if(_isnan(rPos.x))
		{
			bChanged = true;
			rPos.x = 0;
		}
		if(_isnan(rPos.y))
		{
			bChanged = true;
			rPos.y = 0;
		}
		if(_isnan(rPos.z))
		{
			bChanged = true;
			rPos.z = 0;
		}
		if(bChanged)
			m_pEntity->SetPos(rPos);

	float fTerrainLevel=m_p3DEngine->GetTerrainElevation(m_pEntity->GetPos().x, m_pEntity->GetPos().y);

		if(fWaterLevel - fTerrainLevel < depthOffset )//1.5f) 
		{
			m_bSwimming = false;
			return;
		}
	}
//	else 
		if(fWaterLevel - m_pEntity->GetPos().z < depthOffset )//1.5f) // check if it's deep enough to sweem
	{
	vectorf pos = m_pEntity->GetPos();
	pos.z = fWaterLevel;
	ray_hit hit;

		if (m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection( pos, vectorf(0,0,-1.5f), ent_all,
			rwi_stop_at_pierceable,&hit,1, GetEntity()->GetPhysics()))
		{
			m_bSwimming = false;
			return;
		}
	}
	m_stats.fKWater = min(1.0f,m_stats.fInWater*(1.0f/.60f));
	m_bSwimming = m_stats.fKWater>0.0f;
	m_stats.fKWater*=m_stats.fKWater;
}

//////////////////////////////////////////////////////////////////////////

void CPlayer::DampInputVector(vectorf &vec ,float speed ,float stopspeed ,bool only2d ,bool linear)
{
	if (speed<=0.0f)
		return;

	float spd = speed;
	float saveZ = vec.z;

	if (only2d) 
		vec.z = 0;

	float goallen = vec.len();

	if (goallen<0.001f)	
		spd = stopspeed;

	vectorf delta = vec - m_vLastMotionDir;

	if (linear)//if linear cap the delta to "ai_smoothvel"
	{
		float deltalen = delta.len();

		if (deltalen>spd) 
			delta = delta * (deltalen==0?0.001f:1.0f/deltalen) * spd;
	}

	float dt = m_fLastDeltaTime;//m_pTimer->GetFrameTime();//m_pGame->GetSystem()->GetITimer()->GetFrameTime();

	vec = m_vLastMotionDir + delta*min(dt*spd,1.0f);

	if (vec.len()<0.001f) 
		vec.Set(0,0,0);

	m_vLastMotionDir = vec;

	if (only2d)
		vec.z = saveZ;
}

void CPlayer::SaveAIState(CStream & stm, CScriptObjectStream & scriptStream)
{
	IScriptSystem *pScriptSystem = m_pGame->GetSystem()->GetIScriptSystem();
	HSCRIPTFUNCTION	saveOverallFunction=NULL;
	if( m_pEntity->GetScriptObject() && m_pEntity->GetScriptObject()->GetValue("OnSaveOverall", saveOverallFunction) )
	{
		pScriptSystem->BeginCall(saveOverallFunction);
		pScriptSystem->PushFuncParam(m_pEntity->GetScriptObject());
		pScriptSystem->PushFuncParam(scriptStream.GetScriptObject());
		pScriptSystem->EndCall();
	}

	if (!m_bIsAI)
		return;

	IAIObject *pObject = m_pEntity->GetAI();
		
    pObject->Save(stm);
	IPipeUser *pUser =0;

	// save weapon state
	int nGunOut = 0;
	m_pEntity->GetScriptObject()->GetValue("AI_GunOut",nGunOut);
	stm.Write(nGunOut);

	ICryCharInstance *pCharacter = m_pEntity->GetCharInterface()->GetCharacter(0);
	bool bAnimWritten = false;
	if (pCharacter)
	{
		int nAnimID = pCharacter->GetCurrentAnimation(3);
		if (nAnimID>=0)
		{
			ICryCharModel *pModel =  pCharacter->GetModel();
			if (pModel)
			{
				const char *szAnimName = pModel->GetAnimationSet()->GetName(nAnimID);
				stm.Write(szAnimName);
				bAnimWritten = true;
			}
		}
	}
	if (!bAnimWritten)
		stm.Write("NA");

	// save any conversation that this guy may be in
	_SmartScriptObject pCurrentConversation(m_pScriptSystem,true);
	if (m_pEntity->GetScriptObject()->GetValue("CurrentConversation",pCurrentConversation))
	{
		const char *szName;
		pCurrentConversation->GetValue("NAME",szName);
		stm.Write(szName);
		int iProgress,conv_id;
		pCurrentConversation->GetValue("CONV_ID",conv_id);
		stm.Write(conv_id);
		pCurrentConversation->GetValue("Progress",iProgress);
		stm.Write(iProgress);

		// save all the actors in the conversation
		_SmartScriptObject pActors(m_pScriptSystem,true);
		pCurrentConversation->GetValue("Actor",pActors);
		int i=1;
		stm.Write(pActors->Count());
		while (i<=pActors->Count())
		{
			_SmartScriptObject pActorEntity(m_pScriptSystem,true);
			if (pActors->GetAt(i,pActorEntity))
			{
				int Actor_ID;
				pActorEntity->GetValue("id",Actor_ID);
				stm.Write(Actor_ID);
			}
			i++;
		}
	}
	else
		stm.Write("NA");

}

void CPlayer::LoadAIState(CStream & stm, CScriptObjectStream & scriptStream)
{
	IScriptSystem *pScriptSystem = m_pGame->GetSystem()->GetIScriptSystem();
	HSCRIPTFUNCTION	loadOverallFunction=NULL;
	if( m_pEntity->GetScriptObject() && m_pEntity->GetScriptObject()->GetValue("OnLoadOverall", loadOverallFunction) )
	{
		pScriptSystem->BeginCall(loadOverallFunction);
		pScriptSystem->PushFuncParam(m_pEntity->GetScriptObject());
		pScriptSystem->PushFuncParam(scriptStream.GetScriptObject());
		pScriptSystem->EndCall();
	}

	if (!m_bIsAI)
		return;

	IAIObject *pObject = m_pEntity->GetAI();
	pObject->Load(stm);

	int nGunOut=0;
	stm.Read(nGunOut);
	if (nGunOut)
	{
		SetWeaponPositionState(WEAPON_POS_HOLD);
		m_pEntity->GetScriptObject()->SetValue("AI_GunOut",1);

		m_pScriptSystem->BeginCall("BasicAI","MakeAlerted");
		m_pScriptSystem->PushFuncParam(m_pEntity->GetScriptObject());
		m_pScriptSystem->EndCall();
	}

	char str[255];
	stm.Read(str,255);

	if (stricmp(str,"NA"))
	{
		ICryCharInstance *pCharacter = m_pEntity->GetCharInterface()->GetCharacter(0);
		if (pCharacter)
		{
			CryCharAnimationParams ccap;
			ccap.fBlendInTime = 0;
			ccap.fBlendOutTime = 0;
			ccap.nLayerID = 3;
			pCharacter->StartAnimation(str,ccap);
		}
	}


	stm.Read(str,255);
	if (stricmp(str,"NA"))
	{
		_SmartScriptObject pCurrentConversation(m_pScriptSystem,true);
		if (!m_pEntity->GetScriptObject()->GetValue("CurrentConversation",pCurrentConversation))
		{
			// get the saved conversation
			int conv_id;
			stm.Read(conv_id);
			_SmartScriptObject pNewConversation(m_pScriptSystem,true);
			_SmartScriptObject pConvManager(m_pScriptSystem,true);
			m_pScriptSystem->GetGlobalValue("AI_ConvManager",pConvManager);
			m_pScriptSystem->BeginCall("AI_ConvManager","GetSpecificConversation");
			m_pScriptSystem->PushFuncParam(pConvManager);
			m_pScriptSystem->PushFuncParam(str);
			m_pScriptSystem->PushFuncParam(conv_id);
			m_pScriptSystem->EndCall(pNewConversation);

			// remember how far we were
			stm.Read(conv_id);
			// go back one line
			if (conv_id>0)
				conv_id--;
			pNewConversation->SetValue("Progress",conv_id);

			// now loop through all the actors and make them join this conversation
			int nrActors,curr_actor=0;
			stm.Read(nrActors);
			while (curr_actor<nrActors)
			{
				int ActorID;
				stm.Read(ActorID);

				// get the entity
				IEntity *pActorEntity = m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(ActorID);
				if (pActorEntity)
				{
					IScriptObject *pActorScriptObject = pActorEntity->GetScriptObject();
					pActorScriptObject->SetValue("CurrentConversation",pNewConversation);
					unsigned int funcHandle;
					pNewConversation->GetValue("Join",funcHandle);
					m_pScriptSystem->BeginCall(funcHandle);
					m_pScriptSystem->PushFuncParam(pNewConversation);
					m_pScriptSystem->PushFuncParam(pActorEntity->GetScriptObject());
					m_pScriptSystem->EndCall();

				}
				curr_actor++;
			}

			// if joined is equal to amount of participants, continue the conversation
			int iJoined,iParticipants;
			pNewConversation->GetValue("Joined",iJoined);
			pNewConversation->GetValue("Participants",iParticipants);
			if (iJoined == iParticipants)
			{
					unsigned int funcHandle;
					pNewConversation->GetValue("Continue",funcHandle);
					m_pScriptSystem->BeginCall(funcHandle);
					m_pScriptSystem->PushFuncParam(pNewConversation);
					m_pScriptSystem->EndCall();
			}

		}
		else
		{
			int dummy;
			stm.Read(dummy); //conv_id
			stm.Read(dummy); //progress
			int nrActors;
			stm.Read(nrActors);
			while (nrActors--)
				stm.Read(dummy);
		}

	}

}


void CPlayer::LoadAIState_RELEASE(CStream & stm)
{
	if (!m_bIsAI)
		return;

	IAIObject *pObject = m_pEntity->GetAI();
//AIObject Load is empty in release version
//	pObject->Load(stm);

	int nGunOut=0;
	stm.Read(nGunOut);
	if (nGunOut)
	{
		SetWeaponPositionState(WEAPON_POS_HOLD);
		m_pEntity->GetScriptObject()->SetValue("AI_GunOut",1);

		m_pScriptSystem->BeginCall("BasicAI","MakeAlerted");
		m_pScriptSystem->PushFuncParam(m_pEntity->GetScriptObject());
		m_pScriptSystem->EndCall();
	}


/*
	if (!m_bIsAI)
		return;

	IAIObject *pObject = m_pEntity->GetAI();
	//pObject->Load(stm);

	IPipeUser *pUser =0;
	if (pObject->CanBeConvertedTo(AIOBJECT_PIPEUSER,(void**)&pUser))
	{
		int nId;
		stm.Read(nId);
		if (nId)
		{
			if (nId<0)
			{
				// player is the target
				IEntity *pMyPlayer = m_pGame->GetMyPlayer();
				if (pMyPlayer)
				{
					SAIEVENT event;
					event.bFuzzySight = false;
					event.fThreat = 1000000.f;
					event.pSeen = pMyPlayer->GetAI();
					pObject->Event(AIEVENT_ONVISUALSTIMULUS,&event);
				}
			}
			else
			{
				// some puppet or vehicle is the target
				IEntity *pMyTarget = m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(nId);
				if (pMyTarget)
				{
					SAIEVENT event;
					event.bFuzzySight = false;
					event.fThreat = 1000000.f;
					event.pSeen = pMyTarget->GetAI();
					pObject->Event(AIEVENT_ONVISUALSTIMULUS,&event);

				}
			}
			SetWeaponPositionState(WEAPON_POS_HOLD);
			m_pEntity->GetScriptObject()->SetValue("AI_GunOut",1);
		}
	}
*/
}


void CPlayer::LoadAIState_PATCH_1(CStream & stm)
{

	if (!m_bIsAI)
		return;

	IAIObject *pObject = m_pEntity->GetAI();
	pObject->Load_PATCH_1(stm);

	int nGunOut=0;
	stm.Read(nGunOut);
	if (nGunOut)
	{
		SetWeaponPositionState(WEAPON_POS_HOLD);
		m_pEntity->GetScriptObject()->SetValue("AI_GunOut",1);

		m_pScriptSystem->BeginCall("BasicAI","MakeAlerted");
		m_pScriptSystem->PushFuncParam(m_pEntity->GetScriptObject());
		m_pScriptSystem->EndCall();
	}
}


void CPlayer::OnEntityNetworkUpdate( const EntityId &idViewerEntity, const Vec3d &v3dViewer, uint32 &inoutPriority, 
	EntityCloneState &inoutCloneState) const
{
	inoutPriority+=100000;

	if(m_stats.moving)
		inoutPriority+=10000;

	if(m_stats.firing)
		inoutPriority+=20000;

//	if(m_stats.reloading)
//		inoutPriority-=10000;

	inoutCloneState.m_bSyncYAngle=false;
}


//
//
//-------------------------------------------------------------------------------------
bool CPlayer::LoadGame_PATCH_1(CStream &stm)
{
	RemoveAllWeapons();

	// [Petar] preserve the original health in case we need to use it
	// to restore the health of special AI on a checkpoint
	int nStartHealth = m_stats.health;
	if (!m_bIsAI)
		DeselectWeapon();

	for(int cl = 0; (stm.Read(cl),cl); )
	{
		WeaponInfo &wi = m_mapPlayerWeapons[cl];
		wi.owns = true;
		stm.Read(wi.iFireMode);
	}
	for(int i=0;i<sizeof(m_vWeaponSlots)/sizeof(int);i++)
	{
		stm.Read(m_vWeaponSlots[i]);
	}

	Load_PATCH_1(stm);


	SelectWeapon(m_stats.weapon);

	if (m_nSelectedWeaponID != -1)
	{
		WeaponInfo &wi = GetWeaponInfo();

		CWeaponClass *pSelectedWeapon = GetSelectedWeapon();
		pSelectedWeapon->ScriptOnStopFiring(m_pEntity);

		_SmartScriptObject pObj(m_pScriptSystem);
		pObj->SetValue( "firemode", m_stats.firemode);
		pObj->SetValue( "ignoreammo", true);

		bool bCanSwitch;
		m_pEntity->SendScriptEvent(ScriptEvent_FireModeChange, *pObj, &bCanSwitch);

		wi.iFireMode = m_stats.firemode;
	}

	if (m_bIsAI)
	{
		IPuppet *pPuppet=0;
		if (m_pEntity->GetAI()->CanBeConvertedTo(AIOBJECT_PUPPET,(void**)&pPuppet))
		{
			if (pPuppet->GetPuppetParameters().m_bSpecial)
				m_stats.health = nStartHealth;
		}
	}	

	return true;
};

bool CPlayer::Load_PATCH_1( CStream &stream)
{
//	bool bPrevLightStatus;
//	stream.Read(bPrevLightStatus);
//	if (bPrevLightStatus!=m_bLightOn)
//		SwitchFlashLight(bPrevLightStatus);
	return Read_PATCH_1(stream);
}



///////////////////////////////////////////////
/*! Reads all data which needs to be synchronized from the network-stream
@param stream stream to read from
@return true if the function succeeds, false otherwise
*/
bool CPlayer::Read_PATCH_1( CStream &stream )
{
	unsigned short dirty = 0;
	PlayerStats &stats=m_stats;
	//	GetPlayerStats( stats );
	BYTE health,armor,weapon,firemode,staminaBuff;
	//int weaponid = stats.weaponid;
	bool bHostEntity;

	VERIFY_COOKIE(stream);
	stream.Read(bHostEntity);

	if(bHostEntity)
	{
		stream.ReadPkd( staminaBuff );
		m_stats.stamina = staminaBuff;
		//		stream.ReadPkd( staminaBuff );
		//		m_stats.breath = staminaBuff;
		stream.ReadPkd( health );
		stats.health=health;
		stream.ReadPkd( armor );
		stats.armor=armor;
		unsigned int ammo_in_clip;
		stream.ReadNumberInBits(ammo_in_clip,10);
		stats.ammo_in_clip=ammo_in_clip;
		unsigned int ammo;
		stream.ReadNumberInBits(ammo,10);
		stats.ammo=ammo;
		BYTE b;
		stream.ReadPkd(b);
		stats.numofgrenades=b;
		stream.ReadNumberInBits(stats.grenadetype, 4);
		stream.Read(stats.holding_breath);

		bool isnotzero;
		for(int i=0;i<sizeof(m_vWeaponSlots)/sizeof(int);i++)
		{
			stream.Read(isnotzero);
			if(isnotzero)
			{
				stream.ReadPkd(b);
				m_vWeaponSlots[i]=b;
			}else
			{
				m_vWeaponSlots[i]=0;
			}

		}
	}
	else
	{
		bool val;
		stream.Read(val);
		stats.ammo_in_clip = val;
		stream.Read(val);
		stats.ammo = val;
		stream.Read(val);
		stats.firing = val;

		bool t;
		stream.Read(t);
		if(t)
		{
			bool sign;
			BYTE lean;
			stream.Read(sign);
			stream.ReadPkd(lean);
			m_walkParams.fCurrLean=sign?(lean*(1.0f/255)):-(lean*(1.0f/255));
		}
		else
		{
			m_walkParams.fCurrLean=0;
		}
	}
	VERIFY_COOKIE(stream);
	stream.Read(stats.firing_grenade);
	stream.ReadPkd(weapon);
	if (weapon!=stats.weapon)
	{
		SelectWeapon(weapon, false);
		stats.weapon = weapon;
		stats.firing = false;
	}
	stream.ReadPkd(firemode);
	if(firemode!=stats.firemode)
	{
		stats.firemode=firemode;
		SwitchFiremode(stats.firemode);
	}

	//stream.Read(stats.firing);
	//	stream.Read(stats.onLadder);
	eStance stance;
	stream.ReadNumberInBits(*((unsigned int *)&stance),3);

	// no stances in vehicle
	if(stance!=m_CurStance && !m_pVehicle)
	{
		switch(stance)
		{
		case eRelaxed:
			GoRelaxed();
			break;
		case eStand:
			GoStand();
			break;
			//		case eAimLook:
			//			GoAim();
			//			break;
		case eStealth:
			GoStealth();
			break;
		case eCrouch:
			GoCrouch();
			break;
		case eProne:
			GoProne();
			break;
		case eSwim:
			GoSwim();
			break;
		default:
			//DEBUG_BREAK;
			::OutputDebugString("INVALID STANCE\n");
			m_CurStance=(eStance)stance;
			break;
		}
	}
	stream.Read(stats.jumping);

	if(stream.GetStreamVersion()>=PATCH1_SAVEVERSION)					// to be backward compatible with old samegames
	{
		bool bSyncToAllClients;

		stream.Read(bSyncToAllClients);

		if(bSyncToAllClients)
		{
			uint8 ucStartRandomSeedCS;

			stream.Read(ucStartRandomSeedCS);

			//			if(!bHostEntity)
			m_SynchedRandomSeed.SetStartRandomSeedC(ucStartRandomSeedCS);

			//			GetISystem()->GetILog()->Log(">> Player Read %d %d",(int)(this->GetEntity()->GetId()),(int)ucStartRandomSeedCS);			// debug
		}
	}


	//TRIGGER REALOAD ON CLIENT ONLY (think if can be done better)
	bool bReloading;
	stream.Read(bReloading);

	if(bReloading && (stats.reloading==false) && m_nSelectedWeaponID != -1)
	{
		CWeaponClass *pSelectedWeapon = GetSelectedWeapon();
		pSelectedWeapon->ScriptOnStopFiring(m_pEntity);
		pSelectedWeapon->ScriptReload(m_pEntity);
	}
	stats.reloading=bReloading;


	BYTE acc;
	stream.ReadPkd(acc);
	stats.accuracy=acc*(1.f/255.f);

	VERIFY_COOKIE(stream);


	if(stats.firing_grenade)
	{
		//invoke script event (client)
		m_bGrenadeAnimation = true;
		m_pEntity->SendScriptEvent(ScriptEvent_FireGrenade,0,NULL);
	}

	bool bRedirected;
	EntityId nID;
	stream.Read(bRedirected);
	if (bRedirected)
		stream.Read(nID);
	if ((m_pRedirected!=0)!=bRedirected || m_pRedirected && m_pRedirected->GetId()!=nID)
	{
		if (m_pRedirected)
		{
			m_pRedirected->OnUnBind(m_pEntity,0);
			m_pRedirected = 0;
		}
		if (bRedirected && (m_pRedirected=m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(nID)))
			m_pRedirected->OnBind(m_pEntity,0);
	}
	if (m_pRedirected && !bHostEntity)
		m_pRedirected->SetAngles(m_pEntity->GetAngles());

	stream.Read(m_bFirstPersonLoaded);

	return true;
}
