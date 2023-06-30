//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XEntityPlayer.h
//  Description: Entity player class.
//
//  History:
//  - August 16, 2001: Created by Alberto Demichelis
//	- September 2001: Taken Over by Petar Kotevski
//
//////////////////////////////////////////////////////////////////////

#ifndef __GAME_PLAYER_H__
#define __GAME_PLAYER_H__ 

#if _MSC_VER > 1000
#pragma once
#endif

#include "GameObject.h"
#include <IEntitySystem.h>
#include <ISound.h>
#include "ScriptObjectStream.h"
#include "ScriptObjectVector.h"
#include "FireType.h"
#include "SynchedRandomSeed.h"			// CSynchedRandomSeed

//////////////////////////////////////////////////////////////////////////
#define BITMASK_PLAYER				1 
#define BITMASK_WEAPON				2				// both 1st and 3rd person weapon
#define BITMASK_OBJECT				4

// Foraward declarations.
class CXPuppetProxy;
class CWeaponClass;
class CXEntityProcessingCmd;
class CXGame;
class CWeaponSystemEx;
class CVehicle;
struct WeaponParams;
class	CXArea;
struct IAIObject;

/*!
 *	Base class for all game objects.
 *  Implements IEntityContainer interface.
 *	
 */

//! Current weapon description structure.
struct WeaponInfo
{
	bool	owns;																					//!< have this weapon in possesion.
	int		maxAmmo;																			//!< Max ammo allowed for this weapon.
	bool	reloading;																		//!< Is weapon is reloading now.
	float fireFirstBulletTime;													//!< first bullet in a burst
	float fireTime;																			//!< Last time we fired.
	float fireLastShot;																	//!< Last succeded shot
	int		iFireMode;																		//!< firemode
  ICryCharInstance::ObjectBindingHandle	hBindInfo, hAuxBindInfo;		//!<  auxillary bind info for two weapon shooting and binding
	
	WeaponInfo()
	{
		ZeroStruct(*this);
		hBindInfo			= ICryCharInstance::nInvalidObjectBindingHandle;
		hAuxBindInfo	= ICryCharInstance::nInvalidObjectBindingHandle;
	}

	void DetachBindingHandles(ICryCharInstance *pCharacter)
	{
		assert (IsHeapValid());
		if (hBindInfo)
		{
			pCharacter->Detach(hBindInfo);
			hBindInfo = ICryCharInstance::nInvalidObjectBindingHandle;
		}
		if (hAuxBindInfo)
		{
			pCharacter->Detach(hAuxBindInfo);
			hAuxBindInfo = ICryCharInstance::nInvalidObjectBindingHandle;
		}
		assert (IsHeapValid());
	}
};

struct PlayerDynamics 
{
	PlayerDynamics() { gravity=swimming_gravity=inertia=swimming_inertia=air_control=jump_gravity=1E10f; }

	float gravity;						//!< any number to override or 1E10f otherwise
	float swimming_gravity;		//!< any number to override or 1E10f otherwise
	float inertia;						//!< any number to override or 1E10f otherwise
	float swimming_inertia;		//!< any number to override or 1E10f otherwise
	float air_control;				//!< any number to override or 1E10f otherwise
	float jump_gravity;				//!< we could have to use a different gravity when jumping (to feel better) ; any number to override or 1E10f otherwise
};

//	all stamina-related values are here (exept for current value - it's m_stats.stamina)
struct StaminaTable
{
	float			StaminaHUD;				// this is exposed to script to be indicated on HUD
	float			BreathHUD;				// this is exposed to script to be indicated on HUD
	float			BreathDecoyUnderwater;
	float			BreathDecoyAim;
	float			BreathRestore;
	float			RunSprintScale;
	float			SwimSprintScale;
	float			RestoreRun;
	float			RestoreWalk;
	float			RestoreIdle;
	float			DecoyRun;
	float			DecoyJump;
	float			DecoyUnderWater;
	StaminaTable()
	{
		ZeroStruct(*this);

		StaminaHUD		= 1.0f;
		BreathHUD		= 1.0f;

		BreathDecoyUnderwater	= 15.0f;
		BreathDecoyAim			= 10.0f;
		BreathRestore			= 9.0f;

		RunSprintScale	= 1.4f;
		SwimSprintScale	= 1.4f;
		RestoreRun		= 1.5f;
		RestoreWalk		= 8.0f;
		RestoreIdle		= 10.0f;
		DecoyRun		= 30.0f;
		DecoyJump		= 10.0f;
		DecoyUnderWater = 35.0f;
	}
};

//////////////////////////////////////////////////////////////////////////
struct SPlayerUpdateContext
{
	pe_status_living status;
	bool bPlayerVisible;
};

/*! Implement the entity container for the player entity
*/

class CPlayer : public CGameObject
{
public:
enum eInVehiclestate
{
	PVS_OUT=0,
	PVS_DRIVER,
	PVS_GUNNER,
	PVS_PASSENGER,
};


	friend class CScriptObjectPlayer;
	friend class CXPuppetProxy;
	//! Current state of player.
	struct PlayerStats
	{
		//{{NETWORK SYNCHRONIZED STATS
		int	health;					//!< Players current health. 0..255 because of network packet, int because of LUA, 0=dead
		int	armor;					//!< Players current armor. 0..255 because of network packet, int because of LUA
		int ammo_in_clip;
		int ammo;
		int numofgrenades;
		int grenadetype;
		int	weapon;					//!< Index of selected weapon.
		int firemode;			
		int score;					//!< Players score in the game.
		int deaths;					//!< Players deaths count
		bool firing;
		bool firing_grenade;
		bool crosshairOnScreen;
		bool canfire;
		bool reloading;
		bool melee_attack;
		//<<FIXME>> sync this value
		bool aiming;
		bool has_flashlight;
		bool has_binoculars;
		bool fire_rate_busy;
		BYTE random_seed;
		BYTE last_accuracy;
		//}}END OF NETWORK SYNCHRONIZED STATS

		float accuracy;
		float	melee_distance;	//!< range of the melee attack (used in UpdateMelee) default 2.0 set in BasicPlayer.lua

		bool crouch;			//!< Player is crouching.
		bool prone;			//!< Player is proning.

		int		maxHealth;	//!< Players maximal allowed health.
		int		legHealth;	//!< Players LEG HIT ZONE current health.
		int		maxLegHealth;	//!< Players LEG HIT ZONE max health.
		int		armHealth;	//!< Players ARM HIT ZONE current health.
		int		maxArmHealth;	//!< Players ARM HIT ZONE max health.
		int		maxArmor;		//!< Players maximal allowed armor.

		float	dmgFireAccuracy;	// to decreace accuracy with arm damage initial value = 100
		float	dmgSpeedCoeff;		// to solw down with leg damage initial value = 100
		eFireType LastFiringType;     // How we fired during the last update
		eFireType FiringType;

		bool cancelFireFlag;
		bool aim;				//!< Player is aiming.
		float	weapon_busy;
//		bool isDriver;	//!< Player is in vehicle and drivin.
		eInVehiclestate	inVehicleState;	
//		bool			inVehicleActive;	// if player actually entered vehicle - gettingIn animations over
		bool moving;			//!< Player is moving.
		bool running;			//!< Player is running.
		bool jumping;		//!< Player is jumping - (JUMP pressed). 
		bool flying;			//!< Player is flying - not touching ground right now.
		bool jumping_in_air;	//!< Player is flying - not touching ground for some time.
		bool landing;			// was jumping_in_air - now on ground
		bool concentration; //!< player is concentration (listening)
		int	climbing;			//!< Player is climbing (int to avoid overrides when this parameter is not specified through script-calls)
		bool injump;
		float fSpeedScale;
		// For internal use.
		bool moveLeft;
		bool moveRight;
		bool use_pressed;		
		bool holding_breath;
		bool back_pressed;
		bool bForceWalk;
		bool drawfpweapon;
		bool lock_weapon; //if true the player cannot switch weapon
		float fInWater;	// player is fInWater world units under water (measured from entity pos; not eyepos !)
		float fKWater;	// [0,1] - gravity scale when swimming
		float underwater;	// player is completly under water for this amount of time (seconds)
		float swimTime;		// player is swiming for this amount of time (seconds)
		float fVel;
		float	curBlindingValue;	// how much is blinded by flashlights
		bool	bIsBlinded;			// 
		bool	bIsLimping;
		float	stamina;			// used for SPRINT RUN/jump/sprintSwimming
///		float	breath;				// for underwater/aiming
		bool	onLadder;
		bool	bModelHidden;
	};

	Vec3 m_vDEBUGAIFIREANGLES;
	
	float m_fRecoilXDelta;
	float m_fRecoilZDelta;
	float m_fRecoilXUp;
	float m_fRecoilZUp;
	float m_fRecoilX;
	float m_fRecoilZ;
	
	typedef std::map<int, WeaponInfo> PlayerWeapons;
	typedef PlayerWeapons::iterator PlayerWeaponsItor;

public:
	float m_fGrenadeTimer;
	CPlayer(CXGame *);
	virtual ~CPlayer();
	

	virtual void UpdatePhysics(float fDeltaTime);
	void UpdateCamera();

	bool IsAlive() const { return (m_stats.health>0); }
	
	//! Process player commands.
	virtual void ProcessCmd(unsigned int nPing,CXEntityProcessingCmd &ProcessingCmd);

	//! Local for players.
	virtual void ProcessAngles(CXEntityProcessingCmd &ProcessingCmd);
	virtual void ProcessMovements(CXEntityProcessingCmd &ProcessingCmd, bool bScheduled=false);
	virtual void ProcessWeapons(CXEntityProcessingCmd &ProcessingCmd);

	virtual void FireGrenade(const Vec3 &origin, const Vec3 &angles, IEntity *pIShooter);
//	virtual Vec3	TraceGrenade( const Vec3& firePos, const Vec3& dir, const float vel, 
//															const float timeStep=.1f, const float timeLimit=30.0f );
	void SetFiring(bool bIsFiring);

	//! Set new character model for this player.
	void	SetPlayerModel( const string &model );
	
	//! Get character model of this player.
//	string GetPlayerModel() const { return m_strModel; };

	//! Set new character color for this player.
	void SetColor( const Vec3 &invColor ){ m_vColor=invColor; }
	
	//! Get character color of this player.
	Vec3 GetColor() const { return m_vColor; };

	/*! Sets the swaying-amplitude
			@param fAmp amplitude
	*/
	void SetSwayAmp(float fAmp) { m_walkParams.swayAmp=fAmp; }

	/*! Sets the swaying-frequency
			@param fFreq frequency
	*/
	void SetSwayFreq(float fFreq) { m_walkParams.swayFreq=fFreq; }



	/*! sets base angle for players angels restriction 
			uses current entity angle for restriction base
	*/
	void SetAngleLimitBase( ) { 
								m_AngleLimitBase = GetEntity()->GetAngles();
//							m_AngleLimitBase.ConvertToRadAngles();
														}

	void SetAngleLimitBase( const Vec3& base ) { 
								m_AngleLimitBase = base;
//							m_AngleLimitBase.ConvertToRadAngles();
														}


	/*! sets base angle for players angels restriction 
			uses current camera angle for restriction base
	*/
	void SetAngleLimitBaseOnCamera( ) { m_AngleLimitBase = m_pEntity->GetAngles(); }	
	/*! Limits the vertical player angles - min value
			@param fLimit - min angle value
	*/
	void SetMinAngleLimitV( float const fLimit) { m_MinVAngle=fLimit; }
	/*! Limits the vertical player angles - max value
			@param fLimit - max angle value
	*/
	void SetMaxAngleLimitV( float const fLimit) { m_MaxVAngle=fLimit; }
	/*! Enables vertical player angles restriction
			@param enabled
	*/
	void EnableAngleLimitV( bool const enabled ) { m_AngleLimitVFlag=enabled; }
	/*! Limits the horizontal player angles - min value
			@param fLimit - min angle value
	*/
	void SetMinAngleLimitH( float const fLimit) { m_MinHAngle=fLimit; }
	/*! Limits the horizontal player angles - max value
			@param fLimit - max angle value
	*/
	void SetMaxAngleLimitH( float const fLimit) { m_MaxHAngle=fLimit; }
	/*! Enables horizontal player angles restriction
			@param enabled
	*/
	void EnableAngleLimitH( bool const enabled ) { m_AngleLimitHFlag=enabled; }

 	bool IsAngleLimitVOn( ) const { return m_AngleLimitVFlag; }
	bool IsAngleLimitHOn( ) const { return m_AngleLimitHFlag; }	


	Vec3 CalcTangentOnEnviroment( const Vec3	&forward );

	//
	//deathType - head/body hit
	void StartDie( const Vec3& hitImpuls, const Vec3 hitPoint, int partid, const int deathType );

	bool HasCollided();

	// binoculars
	bool	HasBinoculars() const	{ return m_stats.has_binoculars;	}
	void	GiveBinoculars( bool on );


	// flashlight
	void	SwitchFlashLight( bool on );
	bool	InitLight( const char* img=NULL, const char* shader=NULL );
	bool	HasFlashLight() const	{	return m_stats.has_flashlight;	}
	void	GiveFlashLight( bool on );

	// interface IEntityContainer ------------------------------------------------------------------

	virtual bool Init();
	virtual	void Update();
	virtual	void OnSetAngles( const Vec3 &ang );
	virtual	bool Write( CStream &stream,EntityCloneState *cs=NULL);
	virtual	bool Read( CStream &stream );
	virtual	bool Read_PATCH_1( CStream &stream );
	virtual bool QueryContainerInterface( ContainerInterfaceType desired_interface, void **ppInterface);
	virtual float GetLightRadius();
	virtual void OnEntityNetworkUpdate( const EntityId &idViewerEntity, const Vec3d &v3dViewer, uint32 &inoutPriority,
		EntityCloneState &inoutCloneState ) const;

	// ---------------------------------------------------------------------------------------------

	/*! Sets camera shaking-parameters
			@param shakeAxis axis to shake around
			@param shakeDegree degree of shaking
			@param shakeFreq frequency if shaking
			@param shakeTime time in seconds of shaking
	*/
	void SetShake(const Vec3& shakeAxis, float shakeDegree, float shakeFreq, float shakeTime) 
	{ 
		m_walkParams.shakeAxis=shakeAxis; m_walkParams.shakeDegree=shakeDegree; 
		m_walkParams.shakeFreq=shakeFreq; m_walkParams.shakeTime=shakeTime; 
		m_walkParams.shakeOffset=0.0f; m_walkParams.shakeElapsedTime=0.0f; 
	}
	void SetShakeL(const Vec3& shakeAmpl, const Vec3& shakeFreq, const float shakeTime) 
			{ m_walkParams.shakeLAmpl=shakeAmpl; m_walkParams.shakeLFreq=shakeFreq; m_walkParams.shakeLTime=shakeTime; m_walkParams.shakeLElapsedTime=0.0f; }
	void SetShakeL2(const Vec3& shakeAmpl, const Vec3& shakeFreq, const float shakeTime) 
			{ m_walkParams.shakeLAmpl=shakeAmpl; m_walkParams.shakeLFreq=shakeFreq; 
					if(m_walkParams.shakeLElapsedTime<m_walkParams.shakeLTime)
						m_walkParams.shakeLTime += shakeTime;
					else
					{	m_walkParams.shakeLTime=shakeTime; m_walkParams.shakeLElapsedTime=0.0f;} }


	void SetCameraOffset(const Vec3& Offset);
	void GetCameraOffset(Vec3& Offset);

	//! Get statistics information for selected weapon of this player.
	//! If nWeaponIndex is negative WeaponInfo of selected weapon is returned.	
	WeaponInfo& GetWeaponInfo( int nWeaponIndex = -1 );
	//! Get weapon parameters of current weapon
	void GetCurrentWeaponParams(WeaponParams& wp);

	bool SelectWeapon( int weapon, bool bCheckForAvailability = true );
	void SelectNextWeapon();
	void SelectPrevWeapon();
	int CountAvaliableWeapons(){int count=0;for(int n=0;n<PLAYER_MAX_WEAPONS;n++){if(m_vWeaponSlots[n]!=0)count++;} return count;}

	int MakeWeaponAvailable(int nWeaponID, bool bAvailable);

	CWeaponClass* GetSelectedWeapon() const;

	/*! Retrieves the id of the currently selected weapon
			@return weapon-id
	*/
	int GetSelectedWeaponId() { return m_nSelectedWeaponID; }

	void	InitWeapons();
	bool	IsAI() const { return m_bIsAI; }
	bool	IsMyPlayer() const;
	bool	IsFirstPerson() const;
	void	UpdateSwimState(bool bAlive);
	bool	IsSwimming() 
	{ 
		return(m_bSwimming && !m_pEntity->IsBound()); 
	} 

	Vec3 m_vDEBUG_POS;

	// used for Load/Save and Network syncronization
	bool Save( CStream &stream);
	bool Load( CStream &stream);
	bool Load_PATCH_1( CStream &stream);

	bool SaveGame(CStream &stm);
	bool LoadGame(CStream &stm);
	bool LoadGame_PATCH_1(CStream &stm);

	void	UpdateDrawAngles( );

	void OnDraw(const SRendParams & RendParams);

	/*! Retrieves the game associated with this container
			@return pointer to the game
	*/
	CXGame *GetGame() const { return m_pGame; };

	IScriptObject *GetScriptObject();
	
	/*! Attaches a ScriptObject to this container
			@param pObject pointer to the ScriptObject
	*/
	void SetScriptObject(IScriptObject *pObject);

	/*! Overrides the calculated gravity-setting for the next frame
			@param fNewGravity new gravity to set
	*/
	void SetGravityOverride(float fNewGravity) { m_fGravityOverride=fNewGravity; }
	void GetEntityDesc( CEntityDesc &desc ) const
	{
		desc.sModel=m_strModel.c_str();
		desc.vColor=GetColor();
	}
	void EnterVehicle( CVehicle *pVehicle, eInVehiclestate state, const char *szHelperName);
	void LeaveVehicle();
	CVehicle* GetVehicle() { return m_pVehicle; };

//	void Die();

	void SetDimNormal(const pe_player_dimensions* const pDim=NULL );
	void SetDimStealth(const pe_player_dimensions* const pDim=NULL );
	void SetDimCrouch(const pe_player_dimensions* const pDim=NULL );
	void SetDimProne(const pe_player_dimensions* const pDim=NULL );

//	void SetAnimationRefSpeed(const float run=-1.0f, const float walk=-1.0f, const float crouch=-1.0f, const float prone=-1.0f );
	void SetAnimationRefSpeedRun(const float fwd=-1.0f, const float side=-1.0f, const float back=-1.0f );
	void SetAnimationRefSpeedWalk(const float fwd=-1.0f, const float side=-1.0f, const float back=-1.0f );
	void SetAnimationRefSpeedRunRelaxed(const float fwd=-1.0f, const float side=-1.0f, const float back=-1.0f );
	void SetAnimationRefSpeedWalkRelaxed(const float fwd=-1.0f, const float side=-1.0f, const float back=-1.0f );
	void SetAnimationRefSpeedXWalk(const float fwd=-1.0f, const float side=-1.0f, const float back=-1.0f );
	void SetAnimationRefSpeedXRun(const float fwd=-1.0f, const float side=-1.0f, const float back=-1.0f );
	void SetAnimationRefSpeedCrouch(const float fwd=-1.0f, const float side=-1.0f, const float back=-1.0f );
//	void SetAnimationRefSpeedRun(const float fwd=-1.0f, const float side=-1.0f, const float back=-1.0f );
	void SetRunSpeed(const float speed);
	void SetWalkSpeed(const float speed);
	void SetCrouchSpeed(const float speed);
	void SetProneSpeed(const float speed);
	void SetSwimSpeed(const float speed);
	void SetJumpForce(const float force);
	void SetLean(const float lean);
	void SetCameraBob(const float pitch, const float roll, const float length);
	void SetWeaponBob(const float ampl);

	void SetDynamics(const PlayerDynamics *pDyn);

	void Respawn();	
	int  GetBoneHitZone( int boneIdx ) const;

	// set a set of properties for the heat vision
	void SetHeatVisionValues(int dwFlags,const char *szName,float fValue,float fFadingValue);

	// returns ACTUAL plaerys direction angle - the one it's shooting/looking at
	Vec3 GetActualAngles();	

	//! Check if this player is visible.
	bool IsVisible() const;
protected:
	void UpdateLean();	
	Vec3 CalcLeanOffset(float leanAngle);
	bool IsLeaning();
	void SetEyePos();
	void SetEyePosDead();
	void SetEyePosBone();
	void SetEyePosOffset();
	void	UpdateDead( SPlayerUpdateContext &ctx );
	void	UpdateRotateHead();
	void	ResetRotateHead();
	void UpdateWeapon();
	void UpdateBoatCamera();
	void UpdateFirstPersonView();
	void UpdateThirdPersonView();
	void UpdateBonesRotation( );
	bool UpdateBonesPtrs( );
	void UpdateMelee();
	void UpdateCharacterAnimations( SPlayerUpdateContext &ctx );
	void UpdateCharacterAnimationsMounted( SPlayerUpdateContext &ctx );
	void UpdateFireAnimations();
	void UpdateJumpAnimations();
	void ScaleAnimationSpeed( const float speed2d );
	void StartAnimation( const SPlayerUpdateContext &ctx );
	bool GenerateAnimationName( char *sAnimName );

	void	UpdateAutoCenter();
	void	StartAutoCenter(bool forward);
	void	ResetAutoCenter();

	void	ProcessVehicleMovements(CXEntityProcessingCmd &ProcessingCmd);

	void SetWeapon(int iClsID);

	/*! Retrieves the current time
			@return elapsed time in seconds
	*/
	float GetCurrTime() const { return m_pTimer->GetCurrTime(); };

	Vec3 m_vPrevMntPos;	// used when at mounted weapon

	// camera pos/angles used for vehicles FPV to mace smooth camera

public:
	// player's camera mode
	enum e_PCM{
		PCM_ENTERINGVEHICLE,
		PCM_LEAVINVEHICLE,
		PCM_INVEHICLE,
		PCM_OUTVEHICLE,
		PCM_CASUAL,	// casual transition - on/out of water, ladders

	};	
	void	InitCameraTransition( e_PCM mode, bool OnlyZtransition = false );

	Vec3 m_vCurCamposVhcl;
	Vec3 m_vCurAngleVhcl;
	Vec3 m_vCurAngleParent;
	e_PCM	m_CameraMode;

	//filippo
	//! this specify if we want the camera transition work only on the Z pos of the view:
	//! true - smooth only Z , false - smooth view angles and whole view pos.
	bool m_bCameraTransitionOnlyZ; 
	Vec3 m_vDeltaEyeVehicle; //!< needed to shift camera view accordingly with vehicle position while changing sit position.
	bool m_bLastDeltaEyeVehicle; //!< this tell us if m_vDeltaEyeVehicle must be used. in case we are switching sit position inside the same vehicle.

protected:
	float	m_fCameraTime;
	float	m_fCameraSpdV;
	float	m_fCameraSpdA;
	float m_fLastCamUpdateTime;

	void	UpdateCameraTransition( const Vec3& vEyePos  );

public:
	struct SWalkParams
	{
		float runRoll;
		float runPitch;

		float swayAmp;
		float swayFreq;
		float swayOfs;

		float leanDegree;
		float leanCur;

		float leanAmount;
		float leanStart;
		float leanEnd;
		float leanFactor;
		float leanSpeed;

		float fLeanTarget;
		float fCurrLean;


		Vec3 shakeAxis;
		float shakeDegree;
		float shakeFreq;
		float shakeTime;
		float shakeOffset;
		float shakeElapsedTime;

		Vec3	shakeAOffset;
		Vec3	shakeLOffset;
		Vec3	shakeLAmpl;
		Vec3	shakeLFreq;
		float shakeLTime;
		float shakeLElapsedTime;

		float fWaterPitchPhase;
		float fWaterRollPhase;

		float weaponPos;
		float weaponCycle;
		float	weaponCycleSpeed;

		Vec3 dir;

		float	speed;
		float	vertSpeed;
		float	vertSpeedFalling;
		float	curLength;
		float	stepLength;
		float	flyCoeff;
		float	prevF;
	};
	//////////////////////////////////////////////////////////////////////////
	// Fields.
	//////////////////////////////////////////////////////////////////////////
	//! Player parameters.
public:
	Vec3 m_vShake;
	PlayerStats m_stats;
	// Walking params.
	SWalkParams m_walkParams;


	string m_strModel;					//!< Name of player model.
	bool m_bFirstPerson;						//!< True when in first person mode. 
	bool m_bFirstPersonLoaded;					//!< True when was saved in first person mode - set on loading. 

	bool m_bAlwaysRun;
	bool m_bWeaponJustFired;

	bool	m_aimLook;	
	bool	m_bEyesInWater;
  
	//-- A flag to indicate if the animation system is enabled or not
	//-- When a special animation sequence is needed, this flag should 
	//-- be set to 0 so that the normal animation system does not override it.
	//-- Otherwise this flag should be set to 1
	int	m_AnimationSystemEnabled;	// 

	void RemoveAllWeapons();

	Vec3 m_vEyePos;
	Vec3 m_vEyeAngles;

protected:

//	Vec3 m_vLeanPos;
//	Vec3 m_vLeanAngles;

	// angular offset added of something player stands on (i.e. shaking boat)
	Vec3 m_vEyeAnglesBaseOffset;


	IAIObject *m_pLightTarget;

	bool m_bLightOn;
	ITimer	*m_pTimer;
	CXGame		*m_pGame;
	HSCRIPTFUNCTION m_pUpdateAnimation;
	

	pe_player_dimensions m_PlayerDimNormal;
	pe_player_dimensions m_PlayerDimStealth;
	pe_player_dimensions m_PlayerDimCrouch;
	pe_player_dimensions m_PlayerDimProne;
	pe_player_dimensions m_PlayerDimSwim;
	pe_player_dimensions m_PlayerDimDead;
	
	CScriptObjectVector m_ssoHitPosVec;
	CScriptObjectVector m_ssoHitDirVec;
	CScriptObjectVector m_ssoHitNormVec;	

	Vec3 m_vColor;					//<! Color of model. used for team coloring, default: 1,0,1 (means color wasn't set)


	IEntity *m_pRedirected;

	//! Time when player died.
	//float m_deathTime;

	//! When physics is enabled on player.
	//bool m_physicsEnabled;
	
	//! vehicle that this player is driving.
	CVehicle *m_pVehicle;
	// name of helper in vehicle to set camera position 
	string	m_sVehicleEyeHelper;

	//! Player dimention
	enum ePlayerDimension
	{
		eDimNone,
		eDimNormal,
		eDimStealth,
		eDimCrouch,
		eDimProne
	};
	unsigned char m_currDimensions;

	IScriptObject *m_pScriptObject;
	IScriptSystem *m_pScriptSystem;
	IPhysicalWorld *m_pPhysicalWorld;
	I3DEngine *m_p3DEngine;
	
	enum EWeaponPositionState
	{
		WEAPON_POS_UNDEFINED=0,
		WEAPON_POS_HOLSTER,
		WEAPON_POS_HOLD,
	};

	//! Info about weapons used by player.
	PlayerWeapons m_mapPlayerWeapons;
	int m_nSelectedWeaponID;
	int m_vWeaponSlots[PLAYER_MAX_WEAPONS];

	//! True if this is an AI player
	bool m_bIsAI;

	struct pe_status_living m_PreviousLivingStatus;

	//angles limitation
	bool	m_AngleLimitVFlag;
	bool	m_AngleLimitHFlag;
	Vec3	m_AngleLimitBase;
	float	m_MinVAngle;
	float	m_MaxVAngle;
	float	m_MinHAngle;
	float	m_MaxHAngle;

	Vec3	m_EnvTangent;

	//character rotation - first rotate head, than legs	
	float	m_LegADeltaLimit;			// delta (legs/view) to start rotation
	float	m_LegADeltaLimitForce;		// delta (legs/view) to force legs angle without rotation
	float	m_LegAIdleTimeLimit;	// idle time to start rotation
public:
	float	m_LegAngle;
	EWeaponPositionState m_weaponPositionState;
protected:
	float	m_LegDeltaAngle;			
	float	m_LegAngleIdleTime;
	float	m_LegAngleDesired;
	float	m_LegAngleVel;
	float	m_LegAngleVelMoving;		// angle velocity when moving
	float	m_LegAngleVelTimed;			// angle velocity when standing and streightning body pos
	bool	m_LegRotation;
	bool	m_LegNeedsForceAngle;
	bool	m_bSwimming;		// true if underwater/water volume
//	bool	m_bSwimmingMoving; // true if underwater/water volume and moving


	bool	m_bGrenadeAnimation;
	char	m_JumpStage;		// 0-nojump 1-jumpStart 2-jumpAir 3-jumpLand
	float	m_JumpAniLenght;	// landing animation duration - to dealy stating of other anims
	float	m_LandTime;
	float	m_FlyTime;
	float	m_NotRunTime,m_NotRunTimeClient;
	float	m_RunningTime,m_RunningTimeClient;		// how long is running
	float	m_WalkingTime,m_WalkingTimeClient;		// how long is walking (moving)
	float	m_MntWeaponStandTime;			// mounted weapon gunner not mowing left/right time	
	float m_staminaClient;
//	float m_breathClient;
	//threshold times for different jumps ( jump when standing, walking, running )
	float	m_JTWalk;
	float	m_JTRun;

	//djump measures for stand, walk and run jump
	float	m_JumpDist[3];
	float	m_JumpHeight[3];
	//	calculates vertical and horizontal speed to make player jump on dist and heigt
	void	CalcJumpSpeed( float dist, float height, float &horV, float &vertV  );

	//player's animations speed
	//fwd side back 
	float	m_AniSpeedRun[3];
	float	m_AniSpeedWalk[3];
	float	m_AniSpeedXRun[3];
	float	m_AniSpeedXWalk[3];
	float	m_AniSpeedCrouch[3];
	float	m_AniSpeedWalkRelaxed[3];
	float	m_AniSpeedRunRelaxed[3];

	//the player's velocity
	float	m_RunSpeed;
	float	m_WalkSpeed;
	float	m_StealthSpeed;
	float	m_CrouchSpeed;
	float	m_ProneSpeed;
	float	m_SwimSpeed;
	//the player's velocity - strafe
	float	m_RunSpeedStrafe;
	float	m_WalkSpeedStrafe;
	float	m_StealthSpeedStrafe;
	float	m_CrouchSpeedStrafe;
	float	m_ProneSpeedStrafe;
	float	m_SwimSpeedStrafe;
	//the player's velocity - back
	float	m_RunSpeedBack;
	float	m_WalkSpeedBack;
	float	m_StealthSpeedBack;
	float	m_CrouchSpeedBack;
	float	m_ProneSpeedBack;
	float	m_SwimSpeedBack;

	float	m_JumpForce;
	float	m_LeanDegree;

	Vec3	m_ValidAngle;
	Vec3	m_ValidPos;

	PlayerDynamics m_Dynamics;

	ICryBone * m_pBoneHead;
	ICryBone * m_pBoneNeck;
	ICryBone * m_pBoneSpine;
	ICryBone * m_pBoneSpine1;
	ICryBone * m_pBoneSpine2;

	//! Not to be used directly...
	ICryCharInstance *m_pLastUsedCharacter;

/*
	//	hit parameters
	//	set by RememberHit()
	//	used by StartDeathAnimation()
	int	m_DeathType;				//what kind of weapon you died from	
													//		DTExplosion = 0,
													//		DTSingleP = 1,
													//		DTSingle = 2,
													//		DTRapid = 3, 
	int m_DeathDirection;		//from what direction was the fatal hit
													//		fronf			1	
													//		back			2
	int	m_DeathZone;				//what body part was hit
													//		general		0
													//		head			1
													//		chest			2
*/


public:

	IEntity * GetRedirected();
	void RedirectInputToEntity(EntityId id, int angleDelta);
	/*! Retrieves the current walk-stats
			@return walk-stats
	*/
	CPlayer::SWalkParams GetWalkParams() { return m_walkParams; }
	CScriptObjectStream m_stmScriptStream;
private:
	// These variables contain the animation state information
	// They are used to reduce calls to the player script function
	// "UpdateAnimation" by only calling if it detects a state changes
	int m_nStanding;
	int m_nMode;
	int m_nStrafe;
	int m_nForward;
	int	m_nAimLook;

	// heat vision variables, to deal with heat
	// vision, dead bodies etc.
	float	m_fHeatBodyFadingSpeed,m_fHeatBodyDesiredValue,m_fHeatBodyCurrValue;
	float	m_fHeatWeaponsFadingSpeed,m_fHeatWeaponsDesiredValue,m_fHeatWeaponsCurrValue;

	enum eStance
	{
		eNone = 0,
		eStand = 0x01,
//		eAimLook = 0x02,
		eStealth = 0x03,
		eCrouch = 0x04,
		eProne = 0x05,
		eRelaxed = 0x06,
		eSwim = 0x07,
	};

	eStance			m_PrevStance;						//!<
	eStance			m_CurStance;						//!<
	bool			m_Running;							//!<
	bool			m_Sprinting;						//!<

	StaminaTable	m_StaminaTable;
//	float			m_SprintStamina;				// this is exposed to script to be indicated on HUD
//	float			m_RunSprintScale;
//	float			m_RunSprintRestoreScaleRun;
//	float			m_RunSprintRestoreScaleIdle;
//	float			m_RunSprintDecoyScale;



	//char*		m_PrevAniName;
	string	m_sPrevAniName;					//!< layer 0 is used for movement/stance animations (lower body)
	string	m_sPrevAniNameLayer1;		//!< layer 1 is used for aiming/shooting animations
	string	m_sPrevAniNameLayer2;		//!< layer 2 is used for jump/land animations
	float		m_fShootAniLength;

public:
	void GetFirePosAngles( Vec3& firePos, Vec3& fireAngles );
	float CalculateAccuracyFactor(float accuracy);

	void	AutoAiming();
	IEntity	*m_pLastAiming;
	float		m_fLastTime;

	// returns arm damage in percent 100 - (arm_health/max_arm_health)*100
	int GetArmDamage( void ) const;
	// returns arm damage in percent 100 - (leg_health/max_leg_health)*100
	int GetLegDamage( void ) const;

	// changing stances -------------------------------------
	bool GoStand( bool ignoreSpam = true );
	bool GoStealth( void );
	bool GoCrouch( void );
	bool GoProne( void );
	bool GoRelaxed( void );
	bool GoSwim( void );
	bool RestorePrevStence( void );

	bool CanStand( const Vec3& pos );
	bool CanProne( bool ignoreSpam = true );

	// BEGIN - Weapon-related functions
	void SelectFirstWeapon();
	CWeaponClass *DeselectWeapon();
	void DrawThirdPersonWeapon(bool bDraw);
	//
	//	for debug/test purposes
	void StartFire( void );
	void SwitchFiremode(int nforce=-1);
	// attaches weapon to weapon bone on character's back
	void HolsterWeapon(void);
	// rebinds weapon to be bound to the weapon bone in the characters hands
	void HoldWeapon(void);
	// rebinds weapon to be bound to the weapon bone in the characters hands
	void SetWeaponPositionState(EWeaponPositionState weaponPositionState);

	// END   - Weapon-related functions

//	std::vector<int>	m_HostedAreasIdx;
	CXAreaUser	m_AreaUser;

	Vec3 m_vCharacterAngles;

	//void PlaySound(ISound * pSound,float fSoundScale, Vec3 &Offset);
	virtual Vec3 CalcSoundPos();
	void SetViewMode(bool bThirdPerson);

	unsigned MemStats( void );

	void	CounterAdd( const string name, const float timeScale );
	void	CounterSetEvent( const string name, const float thrhld, const string eventName );
	void	CounterIncrement( const string name, const float value );
	float CounterGetValue( const string name );
	void	CounterSetValue( const string name, const float value );
protected:
	void	CounterUpdateAll( const float dt );

	struct  couterEntry 
	{
		float	value;
		float	scale;
		float	eventTrhld;
		string	eventToCall;

		couterEntry()
		{
			scale = 1.0f;
		}
	};

	typedef std::map< string, couterEntry >	CountersMap;
	
	CountersMap							m_UpdatedCounters;				//!<

	


	typedef std::map< string, float >	BlendTimesMap;
	BlendTimesMap		m_AniBlendTimes;
	void	UpdateCollisionDamage();
	void	ProceedFLight( );
	Vec3	GetFLightPos( );
	void	UpdateLightBlinding( );

	void	OnDrawMountedWeapon(const SRendParams & RendParams);

	//!	attached dynamic light source - flashlight
	CDLight	*m_pDynLight;

	IPhysicalEntity *m_pPrevDmgCollider;
	float m_timeDmgCollision;
	float m_prevCollDamage;

	//	// countdown to remove entity
	float	m_fDeathTimer;

	bool	m_bRotateHead;
	bool	m_bInertiaOverride;
	bool	m_bIsFish;
	char	m_AutoCenter;	//0 -off, 1 -going forward, 2 -goung backward
	float	m_fNoChangeangleTime;
	float	m_fProcessTime;
	bool	m_bMouseMoved;	// flag used to count time in autocentering

public:
	void	SetBlendTime(const char *sAniName, float fBlendTime);
	void	GetBlendTime(const char *sAniName, float&fBlendTime);
	void  PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime);

	float m_fGravityOverride;

	int		m_PrevWeaponID;

//	typedef std::vector<Vec3> VectorOfVectors;
//	VectorOfVectors	m_vBlindingPosList;
//	std::vector<Vec3> 	m_vBlindingPosList;
typedef std::map<CPlayer*, Vec3> BlindingList;
	BlindingList 	m_vBlindingList;
	BlindingList::const_iterator m_LastUsed;
//	Vec3	m_vBlindingPos;
	float	m_vBlindingValueFade;

	float	m_RunSpeedScale;
	float	m_CrouchSpeedScale;
	float	m_ProneSpeedScale;
	float	m_XRunSpeedScale;		// stealth
	float	m_XWalkSpeedScale;
	float	m_RRunSpeedScale;		// relaxed
	float	m_RWalkSpeedScale;
	float	m_fAccuracyMod;
	float	m_fAccuracy;

	// sets speeds multiplyers for AI puppet proxyis
	void	SetSpeedMult( float run, float crouch, float prone, float xrun, float xwalk, float rrun, float rwalk );

	void	SetStaminaTable( const StaminaTable& stTable );
	void	UpdateStamina(float fDeltaTime);

//	bool	Get3DCrosshairPosition( Vec3& pos3D, Vec3& posScr );

	bool GetWaitForFireRelease() const		{	return m_bWaitForFireRelease; }
	void SetWaitForFireRelease(bool bVal)	{	m_bWaitForFireRelease = bVal; }

	IEntity *m_pMountedWeapon;
	// here we store last angles of player at mounted weapon when no intersection 
	// with any objects would happen
	Vec3d	m_vSafeAngAtMountedWeapon;
	
	//! DampInputVector make change the input vector of the player in a smooth way, it is implemented for AIs but can be used also for players.
	void DampInputVector(vectorf &vec ,float speed ,float stopspeed ,bool only2d ,bool linear);

	float m_input_accel;//!<the acceleration for the input to reach the desired input vector, 0 means no input smoothing.
	float m_input_stop_accel;//!<input acceleration used when player stops, usually more than the acceleration.

	//! indoor special input acceleration values, for AI only.
	float m_input_accel_indoor;
	float m_input_stop_accel_indoor;

private:

	bool									m_bStayCrouch;//!< if true the player must stay crouched.

	vectorf	 						 	m_vLastMotionDir;					//!< used by DampInput function, keep track of the last vector to smooth the new input.
	float  								m_fLastDeltaTime;

	bool  								m_bHasJumped;							//!< player jumped?
	//! variables for apply a downward negative impulse to the player when he reach the max height of the jump.
	float  								m_fLastGroundHeight;			//!< the ground height when the player was on ground

	//! variables used for the lazy weapon movement
	Vec3  								m_vDeltaCamAngles;
	Vec3  								m_vDeltaCamPos;
	Vec3  								m_vPrevCamAngles;
	Vec3  								m_vPrevCamPos;
	Vec3  								m_vWeaponAngles;
	bool  								m_bWaitForFireRelease;

	Vec3 									m_vProneEnvNormal;
	Vec3 									m_vCurEntAngle;

	bool									m_bWriteOccured;					//!< flag to tell the player update that his state has been written over a network stream


public: // ----------------------------------------------------

	CSynchedRandomSeed		m_SynchedRandomSeed;			//!< random seed helper (randomize the weapon bullet shooting)

	//!
	void SaveAIState(CStream & stm, CScriptObjectStream & scriptStream);
	//!
	void LoadAIState(CStream & stm, CScriptObjectStream & scriptStream);
	//!
	void LoadAIState_RELEASE(CStream & stm);
	//!
	void LoadAIState_PATCH_1(CStream & stm);


	friend class CSynchedRandomSeed;

	Vec3 m_vLadderPosition; //!< when player use a ladder we have to know the ladder position to orient the player model to the ladder.
	Vec3 m_vLadderAngles; //!< the player model orientation when using the ladder, useful to limit the head rotation.

private:

	Ang3 m_vHeadAngles; //!< the actual player head bone angle , used for smooth the head rotation.

	float m_fLastProneTime; //!< needed to cap the prone-standing position spamming
}; 

#endif // __GAME_PLAYER_H__
