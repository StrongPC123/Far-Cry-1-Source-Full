
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//	XVehicle.h 
//	This is the container class for the Vehicles in the game
//	created by Petar Kotevski
//////////////////////////////////////////////////////////////////////////

#ifndef _XVEHICLE_H_
#define _XVEHICLE_H_

#include "XPlayer.h"	//<<FIXME>> put CGameObject in a separate file and do not include this header
#include "ScriptObjectStream.h"
//#include "xsmthbuffer.h"

class CXGame;
class CScriptObjectVehicle;
class CXVehicleProxy;



enum VHC_TYPE {
	VHT_CAR,
	VHT_BOAT,
	VHT_BOATDEAD,
	VHT_PARAGLIDER
};

typedef struct WATER_VEHICLE_PARAMS {

	// water vehicles parameters
	float m_fBtDumpRot;				//slow down rate for rotation when stop turning
	float m_fBtDumpV;					//damping slow down rate for speed in forward/back dorection
	float m_fBtDumpVSide;				//damping slow down rate for speed in sideways direction
	float m_fBtDumpVH;					//damping slow down rate for vertical speed 
	float m_fBtDumpW;					//damping slow down rate for waves momentum 
	float m_fBtTurn;					//turning impuls
	float m_fBtTurnMin;					//turning impuls minimal
	float m_fBtTurnSpeedScale;			// scale to lower turning impuls with velosity
	float m_fBtSpeedV;					//movement impuls
	float m_fBtSpeedTurnMin;		//min speed allowing to turn
	float m_fBtStand;				//forsing to normal vertical position impuls
	float m_fBtWave;				//wave momentum
	float m_fBtTitlTurn;			//tilt momentum when turning
	float m_fBtTitlSpd;				//tilt momentum when speeding up
	float m_fBtTitlSpdA;			//tilt momentum when speeding up (acceleration thrhld)
	float m_fBtTitlSpdMinV;			//tilt momentum when speeding up (min speed to tilt when not accelerating)
	float m_fBtTitlSpdMinVTilt;		//tilt momentum when speeding up (how much to tilt when not accelerating)

	float m_fBoatGravity;			//!< gravity of the boat when out of water
	float m_fBtStandAir;			//!< forsing to normal vertical position impuls, used when in air.

	float m_fCameraDist;

	// paraglider specific
	float m_fPgDownRate;
	float m_fPgBackUpRate;
	float m_fPgBackUpSlowRate;
	float m_fPgUpSpeedThrhld;

	WATER_VEHICLE_PARAMS()
	{
		memset( this, 0, sizeof(WATER_VEHICLE_PARAMS) );
		m_fCameraDist = 7.0f;
	}

} WATER_VEHICLE_PARAMS;

/*! Vehicle entity container
	this class is aggregated to an entity to specialize 
	it as vehicle
*/

class CVehicle : public CGameObject
{
	friend class CPlayer;	// need this to access m_pVehicle->m_BoatParams.m_fCameraDist;
	friend class CScriptObjectVehicle;
	friend class CXVehicleProxy;

	// map of entityIds - weaponIds
	typedef std::list		< int > UsersList;

	UsersList					m_UsersList;					//!<
	int								m_WeaponUser;					//!<

	IScriptObject *		m_pScriptObject;			//!<
	CXGame *					m_pGame;							//!<

	Vec3							m_vShakeCameraPos;		//!<

public: // -------------------------------------------------------------------------------

	//! constuctor
	CVehicle(CXGame *);
	//! destructor
	~CVehicle();

	// interface IEntityContainer ----------------------------------------------------------

	virtual bool Init();
	virtual void Update();
	virtual bool Write(CStream&,EntityCloneState *cs=NULL);
	virtual bool Read(CStream&);
	virtual IScriptObject *GetScriptObject() { return m_pScriptObject; }
	virtual void SetScriptObject(IScriptObject *pObject) { m_pScriptObject=pObject; }
	virtual bool QueryContainerInterface( ContainerInterfaceType desired_interface, void **ppInterface);
	virtual void GetEntityDesc( CEntityDesc &desc ) const{}
	virtual void OnDraw(const SRendParams & RendParams);
	virtual void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime);
	virtual float GetLightRadius();
	virtual void OnEntityNetworkUpdate( const EntityId &idViewerEntity, const Vec3d &v3dViewer, uint32 &inoutPriority, 
		EntityCloneState &inoutCloneState) const;

	// -------------------------------------------------------------------------------------

	virtual void UpdatePhysics(float fTimeStep);
	void UpdateBoat(float fTimeStep);
	void UpdateParaglider(float fTimeStep);
	//! process the input
	void ProcessMovement(CXEntityProcessingCmd &ProcessingCmd);
	void ProcessMovementBoat(CXEntityProcessingCmd &ProcessingCmd);
	void ProcessMovementBoat2(CXEntityProcessingCmd &ProcessingCmd, float velScale = -1.0f);
	void ProcessMovementParaglider(CXEntityProcessingCmd &ProcessingCmd);
	
	void GetWheelStatus(int nWheel, pe_status_wheel *pStatus);

	bool IsDirty(void) {return false;}
	bool Save( CStream &stream,IScriptObject *pStream=NULL ){return false;}
	bool Load( CStream &stream,IScriptObject *pStream=NULL ){return false;}

	bool HasAccelerated() { return m_bAcceleratedLastUpdate && m_bAcceleratedFlagSetLastFrame; };
	bool IsBreaking() { return m_bIsBreaking; };

	//[filippo]
	//! added handbrakingvalue
	void SetDrivingParams(float pedalspeed,float steerspeed,float v0maxsteer,float kvmaxsteer,float v0steerrelax,float kvsteerrelax,
		float brake_vel_threshold,float brake_axle_friction,float steerspeedVelScale, float steerSpeedMin, float maxSteeringPedal, float pedalLimitSpeed, 
		float fhandbrakingvalue,float fmaxbrakingfrictionnodriver,float fhandbrakingvaluenodriver,float fstabilizejump,
		float fsteerspeedscale,float fsteerspeedscalemin);

	void	AddUser(int entId);
	void	RemoveUser( int entId );		
	void	SetWeaponUser(int entId);
	void	ReleaseWeaponUser( bool bDeselectWeapon=false );

	bool	HasDriver();

	class CXGame *GetGame() { return m_pGame; }
	void SetWaterVehicleParameters(	WATER_VEHICLE_PARAMS &wvpar, bool bFlyingVehicle);
//	void SetWaterVehicleParameters(float fBSpeed,float fBTurn);
	void SetEngineHealth(float fEngineHealth, bool bScheduled=false);

	float	m_fEngineHealth;
	float m_fDeathTimer;

	// for boat
	float	m_btVelocity;
	bool	m_btInWater;
	float m_fWaterlevelLimit;//!< this is the limit to check if the boat is in water or not.

	int		m_DriverID;

//	float	m_fTimeDelay;		// startup delay - after entering, before vehicle can drive
	// for camera smoothing
	//CXSmthBuffer	m_zAngBuffer;

	bool	IsBoat()	{ return (m_Type == VHT_BOAT); }
	int		GetType() { return (m_Type); }
	void	SetType( VHC_TYPE type ) { m_Type = type; }

	void SaveAIState(CStream & stm, CScriptObjectStream & scriptStream);
	void LoadAIState(CStream & stm, CScriptObjectStream & scriptStream);

	void	InitHeadLight( const char* sImg, const char* sShader );
	void	InitFakeLight( CDLight** light, const char* sShader );
	void	SwitchLights( int light );
	void	GetFirePosAngles(Vec3d& firePos, Vec3d& fireAngles);
	const string& GetWeaponName( CPlayer::eInVehiclestate state );
	void	UpdateWeaponPosAngl( );
	CPlayer*	GetUserInState( CPlayer::eInVehiclestate state );
	CPlayer*	GetWeaponUser( );

	void ResetCamera(bool bUpdateCamera=false,const char *pHelperName=0);
	void UpdateCamera(float fTimeStep, bool bSynchronizing=false);
	Vec3 GetCamPos() { return m_vCamPos; }
	void SetCameraParams(Vec3 vCamStiffness[2],Vec3 vCamLimits[2],float fCamDamping,float fMaxCamTimestep,float fCamSnapDist,float fCamSnapVel);

	void CVehicle::WeaponState(int userId=-1, bool shooting=false, int fireMode=0 );

	bool	CrossOnScreen() { return m_bCrossOnScreen; }

	static string	m_sNoWeaponName;
protected:

	// sets current movement type to be used to play animations for users (turning/breaking/movingBack)
	void	UpdateMovementStatus();
	void	UpdateLights( );
	void	UpdateFakeLight( CDLight* light, const char* sHelper );

	void	WakeupPhys( );
	bool	AnglesToLimit( Vec3& angl);
	void	UpdateWeaponLimitRotation( Vec3& curPos, bool bClamped);

	bool m_bAcceleratedLastUpdate;
	bool m_bAcceleratedFlagSetLastFrame;
	bool m_bIsBreaking;
	float m_fPedalSpeed;
	float m_fSteerSpeed;
	float m_fSteerSpeedVelSCale;
	float m_fSteerSpeedMin;
	float m_fv0MaxSteer,m_fkvMaxSteer;
	float m_fv0SteerRelaxation,m_fkvSteerRelaxation;
	float m_fAxleFriction;
	float m_fBrakeVelThreshold,m_fBrakeAxleFriction;
	float m_fMaxSteeringPedal,m_fPedalLimitSpeed;
	CScriptObjectStream m_ScriptStream;

	VHC_TYPE	m_Type;

	//the place where to apply the impulse from
	Vec3	m_vEnginePos;

	// water vehicles parameters
	WATER_VEHICLE_PARAMS	m_BoatParams;

//	float m_fBoatSpeed;
//	float m_fBoatTurnSpeed;

	float m_fBtSpeedVCurrent;					//movement impuls
	float m_fBtSpeedVCurrentRev;					//movement impuls

	ICVar *m_pGliderGravity;
	ICVar *m_pGliderBackImpulse;
	ICVar *m_pGliderDamping;
	ICVar *m_pGliderStartGravity;

	float m_fPrevFwvSpeedLen;
	float m_fPrevFwvTilt;

	//!	attached dynamic light source 
	CDLight	*m_pHeadLight;
	CDLight	*m_pHeadLightLeft;
	CDLight	*m_pHeadLightRight;
	CDLight	*m_pBreakLightLeft;
	CDLight	*m_pBreakLightRight;

	string	m_HeadLightHelper;
	string	m_HeadLightHelperLeft;
	string	m_HeadLightHelperRight;
	string	m_BackLightHelperLeft;
	string	m_BackLightHelperRight;

	bool	m_bAutoLights;		// headlights on/off depending on environment lightening
	bool	m_bAIDriver;		// 
	bool	m_bHeadLightsOn;
	bool	m_bBreakLightsOn;
	int m_iPrevMoveTime;
	float m_fSimTime;

	bool	m_bPhysAwaike;

	bool	m_bBrakeOnNodriver;
	float	m_fNoDriverTime;

	string	m_sAutoWeaponName;
	string	m_sMountedWeaponName;

	//angles limitation
	bool	m_AngleLimitVFlag;
	bool	m_AngleLimitHFlag;
	Vec3	m_AngleLimitBase;
	float	m_MinVAngle;
	float	m_MaxVAngle;
	float	m_MinHAngle;
	float	m_MaxHAngle;

	Vec3	m_vWpnPos;
	Vec3	m_vWpnAng;
	Vec3	m_vWpnAngNoSnap;
	Vec3	m_vWpnAngDelta;
//	Vec3	m_vWpnAngNoSnap;
	Vec3	m_vCross3D;
	Vec3	m_vCrossScreen;
	Vec3	m_vCrossScreenSmoothed;
	bool	m_bTargetIsLocked;
	bool	m_bCrossOnScreen;

	// inertial camera stuff
	Vec3 m_vPrevCamTarget;
	Vec3 m_vCamPos,m_vCamVel;
	Vec3 m_vCamHelperPos;
	Vec3 m_vCamStiffness[2];
	Vec3 m_vCamLimits[2];
	float m_fCamDamping;
	float m_fMaxCamTimestep;
	float m_fCamSnapDist,m_fCamSnapVel;
	bool m_bUpdateCamera;

	CXEntityProcessingCmd m_cmdLastMove;

	float	m_fBackoffTime;		// duration of backward movement -- needed for animating driver
	int		m_TurnState;		// needed for animating driver/users

	//float	m_btDump;			// dump coeff for momentum					"water friction"
	//float	m_btDumpV;		// dump coeff for linear velocity in horizontal plane		"water friction"
	//float	m_btdumpVH;		// dump coeff for linear velocity	in vertical direction	"water friction, gravity"
	//float	m_btStand;		// straightening force
	//float	m_btTurn;			// turning momentum - turn radius
	//float	m_btTilt;			// pitch up when speeding
	//float	m_btSpeedV;		// impulse when moving 
	//float	m_btFloat;		// floating coeff - Gravity = (WaterLevel-BoatZ)*m_btFloat
	// waves parametrs. Wave is a sinusoid
	//float	m_btWscale;			//	world to wave scale
	//float	m_btWscalew;		//	time coefficient
	//float	m_btWmomentum;	//	max momentum - strength of waves
	
	//[filippo]
private:
	
	//! This function apply some extra physics to the wheeled vehicles.
	void AdditionalPhysics(IPhysicalEntity *pcar,float fdelta,bool bforcebreaking);

	//! (meter/sec / sec) this force the handbraking velocity to a certain value, 
	//! necessary when max braking friction is low to make cars sliding.
	float m_fhandbraking_value;		
	float m_fhandbraking_value_nodriver; //!< applied when no driver

	float m_fMaxBrakingFriction; //!< this is the friction to use normally
	float m_fMaxBrakingFrictionNoDriver;//!< this is the friction to use when there is no driver

	bool m_bUsingNoDriverFriction; //!< 

	float m_DirVelDotProduct; //!< dotproduct between direction and velocity of the vehicle, to get if it is going backward or not

	float m_fstabilizejump; //!< how much the car angle will be corrected/stabilized while it is in air.

	float m_flastwheelrotation; //!< what is the last angle of the steering wheel? to make smooth rotation.
	float m_fsteerspeed_scale; //!< the steering speed scale at max speed
	float m_fsteerspeed_scale_min;//!< the steering speed scale at min speed

	bool	m_bForceHandBreak;		// breake on every CVehicle::UpdatePhysics - needed for AI cars 
};

#endif // _XVEHICLE_H_







