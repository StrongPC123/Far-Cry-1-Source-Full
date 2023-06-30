//////////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Entity.h
//
//	History:
//	-Feb 14,2001:Originally created by Marco Corbetta
//	-: modified by everyone
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef ENTITY_H
#define ENTITY_H

#if _MSC_VER > 1000
# pragma once   
#endif

#define ENTITY_PROFILER FUNCTION_PROFILER_FAST( m_pISystem,PROFILE_ENTITY,g_bProfilerEnabled );
#define ENTITY_PROFILER_NAME(str) FRAME_PROFILER_FAST( str,m_pISystem,PROFILE_ENTITY,g_bProfilerEnabled );

#define SCRIPT_INIT				"OnInit"
#define SCRIPT_UPDATE			"OnUpdate"
#define SCRIPT_SHUTDOWN		"OnShutDown"
#define SCRIPT_ONCONTACT	"OnContact"

//! On reset callback, mostly called when Editor wants to Reset the state
#define SCRIPT_ONRESET		"OnReset"

//////////////////////////////////////////////////////////////////////////

#include "EntityCamera.h"
#include "EntityDesc.h"
#include <IScriptSystem.h>
#include <IEntitySystem.h>
#include <IStatObj.h>
#include <ICryAnimation.h>
#include <IAgent.h>
#include <ISound.h>
#include <I3DEngine.h>

#if defined(LINUX)
	#include "Tarray.h"
#endif

// Forward class declarations.
class cAIBrain;
struct CStatObj;
class CSound;
class PhysicalEntity;
class CImage;
struct ILipSync;
struct pe_cargeomparams;
struct pe_params_car;
struct IScriptSystem;
struct ICryCharInstance;
struct IShader;
class CREShadowMap;
class CCObject;
struct AIObjectParameters;

//! States's standart script functions.
enum	EScriptStateFunctions
{
	ScriptState_OnBeginState,
	ScriptState_OnEndState,
	ScriptState_OnUpdate,
	ScriptState_OnContact,
	ScriptState_OnTimer,
	ScriptState_OnEvent,
	ScriptState_OnDamage,
	ScriptState_OnEnterArea,
	ScriptState_OnLeaveArea,
	ScriptState_OnProceedFadeArea,
	ScriptState_OnBind,
	ScriptState_OnUnBind,
	ScriptState_OnMove, //!< OnMove script callback called when an entity moves.
	ScriptState_OnCollide,
	ScriptState_OnStopRollSlideContact,
	ScriptState_Last,
};

//////////////////////////////////////////////////////////////////////////

struct SAttachedSound
{
	SAttachedSound(ISound *_pSound, Vec3d &_Offset)
	{
		pSound=_pSound;
		Offset=_Offset;
	}
	_smart_ptr<ISound> pSound;
	Vec3d Offset;
};

typedef std::list<SAttachedSound> SoundsList;
typedef SoundsList::iterator SoundsListItor;

//! Structure that define current state of entity.
//! Contains pointer to script functions that implement state behaivor.
struct SScriptState
{
	// Pointers to script state functions.
	HSCRIPTFUNCTION pFunction[ScriptState_Last];		//!< Called when entity is in contact with another entity.

	//! Must provide copy operator and constructor.
	SScriptState()
	{
		memset( pFunction,0,sizeof(pFunction) );
	}
	SScriptState( const SScriptState& s ) { *this = s; }
	void operator =( const SScriptState& s )
	{
		memcpy( pFunction,s.pFunction,sizeof(pFunction) );
	}
};
 
//////////////////////////////////////////////////////////////////////
#define ENTITY_MAX_OBJECTS	16

typedef std::map<string,unsigned char> EntityStateMap;
typedef EntityStateMap::iterator EntityStateMapItor;
//////////////////////////////////////////////////////////////////////
typedef std::multimap<int,int> IntToIntMap;

//////////////////////////////////////////////////////////////////////
class CEntity : 
public IEntity, 
public IEntityCharacter
{
public:
	void GetMemoryStatistics(ICrySizer *pSizer);
	//! Flags that tell what parameters of entity are dirty.
	
	// not protected because some time the creation of dummy entities is required
	CEntity (class CEntitySystem *pEntitySystem, ISystem *pISystem,IScriptSystem *pSS);
	//! destructor
	virtual ~CEntity();
	
	//! Shortcut to get entity id.
	EntityId GetId() const { return m_nID; };
	
	virtual bool Init( CEntityDesc &ed );
	virtual	void Update( SEntityUpdateContext &ctx );
	virtual void ShutDown();
	virtual void Reset();

	virtual bool IsStatic() const { return false; }			// Entities are dynamic by definition

	virtual EntityClassId GetClassId() {return m_ClassId;}
	virtual void SetClassId(const EntityClassId ClassId) { m_ClassId = ClassId;}
	
	//! Called when entity is drawn.
	
	void SetID(int id)	{m_nID = id;}
	
	bool GetNetPresence() const { return m_netPresence; };

	void SetNetPresence(bool ispresent) { m_netPresence = ispresent;}
	
	ILipSync* GetLipSyncInterface();
	void ReleaseLipSyncInterface();

	int	GetObjectsFlags(int nSubObj)
	{
		CEntityObject *pObj=&m_objects[nSubObj];	
		return (pObj->flags);
	}


	void	SinkRebind(IEntitySystemSink *pSink);

	void	SetWaterDensity(float fWaterDensity) { m_fWaterDensity=fWaterDensity; }
	float	GetWaterDensity() { return(m_fWaterDensity); }
	
	void SetFlags( uint flags ) { m_flags |= flags; };
	uint GetFlags() { return m_flags; };
	void ClearFlags( uint flags ) { m_flags &= ~flags; };
	bool CheckFlags( uint flags ) const { return (m_flags&flags) == flags; };
	
	//! Set name of the entity.
	//! Hides same member in base class.
	void  SetName( const char *name );
	const char *GetName() const { return m_name.c_str(); };

	void  SetClassName( const char *name ) 
	{ 
		if (m_sClassName.empty())
			m_sClassName = name;	
	}
	const char * GetEntityClassName() const { return m_sClassName.c_str(); };

	//! Get description of entity.
	void GetEntityDesc( CEntityDesc &desc ) const;

	void GetHelperPosition(const char *helper, Vec3d &pos, bool objectspace);

	// garbage stuff maybe needs to be removed later
	bool IsGarbage() { return m_bGarbage;}	
	void SetGarbageFlag( bool bGarbage );
	void MarkAsGarbage();
	void SetDestroyable(bool b);
	bool IsDestroyable() const;

	void SetScriptObject(IScriptObject *pObject);
	IScriptObject *GetScriptObject() { return m_pScriptObject;}
	
	void Bind(EntityId id,unsigned char cBind, const bool bClientOnly, const bool bSetPos );
	void Unbind(EntityId id,unsigned char cBind, const bool bClientOnly);
	void AttachToBone(EntityId id,const char* boneName);
	BoneBindHandle AttachObjectToBone(int slot,const char* boneName,bool bMultipleAttachments=false, bool bUseZOffset=false);
	void DetachObjectToBone(const char* boneName,BoneBindHandle objectBindHandle=-1 );

	//////////////////////////////////////////////////////////////////////////
	void SetSleep( bool bSleep )
	{
		m_bSleeping = bSleep;
	}

	/////////////////////////////////////////////////////////////////////////
	//! Access to attached camera.
	//////////////////////////////////////////////////////////////////////////
	//! Check if camera present.
	bool HaveCamera() const { return GetCamera() != 0; }
	//! Get Attached camera.
	IEntityCamera* GetCamera() const;
	//! Set new camera.
	void SetCamera( IEntityCamera *pCam ) { m_bUpdateCamera = true; m_pCamera = pCam; }
	// Set third person mode for camera.
	void SetThirdPersonCameraMode( const Vec3d &center,const Vec3d &angles,int mode,float frameTime,float range,int dangleAmmount,IPhysicalEntity *physic );


	/////////////////////////////////////////////////////////////////////////
	//! Access to attached dynamic light.
	//////////////////////////////////////////////////////////////////////////
	//! Get Attached light.
	CDLight*	GetLight( );
	//! create light, load proj. texture and shader
	bool	InitLight( const char* img=NULL, const char* shader=NULL, bool bUseAsCube=false, float fAnimSpeed=0, int nLightStyle=0, float fCoronaScale=0 );


	//////////////////////////////////////////////////////////////////////////
	//! Intersection testing.
	//////////////////////////////////////////////////////////////////////////
	//! Check if entity inside other entity.
	
	//! Check if entity interesct radius.
	
	//////////////////////////////////////////////////////////////////////////
	//! Physics.
	//////////////////////////////////////////////////////////////////////////
	//! Check if physics entity exist.
	bool HavePhysics() const { return m_physic!=0 || m_physPlaceholder!=0; };
	//! Get physics entity.
	IPhysicalEntity* GetPhysics() const {
		return m_physPlaceholder ? m_physPlaceholder : m_physic;
	};
	//! Change physical entity.
	void SetPhysics( IPhysicalEntity* physic );
	void DestroyPhysics();
	int CreatePhysicalEntityCallback(int iForeignFlags);
	int DestroyPhysicalEntityCallback(IPhysicalEntity *pent);
	void EnablePhysics( bool enable );
	void AddImpulse(int ipart,Vec3d pos,Vec3d impulse,bool bPos=true,float fAuxScale=1.0f);
	
	//! Create general rigid body attached to entity.
	bool CreateRigidBody(pe_type type, float density,float mass,int surface_id, Vec3d* pInitialVelocity = NULL, int slot=-1, bool bPermanent=false);
	bool CreateLivingEntity(float mass, float height, float eye_height, float sphere_height, float radius,int nSurfaceID,float fGravity,float fAirControl, bool collide=false);

	bool CreateStaticEntity(float mass, int surface_idx, int slotToUse=-1, bool bPermanent=false);
	bool CreateSoftEntity(float mass,float density, bool bCloth, IPhysicalEntity *pAttachTo=WORLD_ENTITY,int iAttachToPart=-1);

	//! AI stuff
	bool RegisterInAISystem(unsigned short type, const AIObjectParameters &params);
	void EnableAI(bool enabled) {  if (m_pAIObject) m_pAIObject->IsEnabled(enabled); }


	//! Create vehichle physics.
	bool LoadVehicle(const char *objfile, pe_cargeomparams *pparts, pe_params_car *params,bool bDestroy=false);
	bool LoadBoat(const char *objfile, float mass, int surfaceID);

	void SetDamage(const int dmg);
	
	//////////////////////////////////////////////////////////////////////////
	//! Geometry related.
	//////////////////////////////////////////////////////////////////////////
	//! Add geometry object to entity.
	bool	LoadObject(unsigned int pos,const char *fileName,float scale, const char *geomName=NULL);
	bool GetObjectPos(unsigned int slot,Vec3d &pos);
	bool SetObjectPos(unsigned int slot,const Vec3d &pos);
	bool GetObjectAngles(unsigned int slot,Vec3d &ang);
	bool SetObjectAngles(unsigned int slot,const Vec3d &ang);
	//! Load image to entity.
	//! Check if attached object at specified slot loaded.
	virtual bool	IsObjectLoaded( unsigned int slot );
	//! Assign object to specified slot.
	virtual bool	SetEntityObject( unsigned int slot,const CEntityObject &object );
	//! Get object at specified slot.
	virtual bool	GetEntityObject( unsigned int slot,CEntityObject &object );
	//! Get number of attached objects.
	virtual int		GetNumObjects() { return m_objects.size(); };

	//! Draw spcific geometry slot.
	void	DrawObject(unsigned int pos,int mode);
	// Draw all geometry slots.
	void	DrawObject(int mode);
	
	
	void SendScriptEvent(enum EScriptEventId Event, struct IScriptObject *pParamters, bool *pRet=NULL);
	//Timur[2/1/2002] ? Remove it.....(alberto)no
	void SendScriptEvent(enum EScriptEventId Event, const char *str, bool *pRet=NULL );
	void SendScriptEvent(enum EScriptEventId Event, int nParam, bool *pRet=NULL );
	
	//! Set bounding box of entity.
	void	SetBBox(const Vec3d &mins,const Vec3d &maxs);
	//! Get bounding box of entity.
	void	GetBBox( Vec3d &mins,Vec3d &maxs );
	//! Enable tracking of physical colliders for this entity.
	void TrackColliders( bool bEnable );

	//! Get bounding box of entity including all render components
	void GetRenderBBox(Vec3d &mins, Vec3d &maxs);

	//! Get radius of entity including all render components
	float GetRenderRadius() const;

	//! Spawn particle system attached to entity.
	bool CreateParticleEntity(float size,float mass, Vec3d heading, float acc_thrust=0,float k_air_resistance=0, 
		float acc_lift=0,float gravity=-9.8, int surface_idx=0, bool bSingleContact=true);
	
	//////////////////////////////////////////////////////////////////////////
	//! Entity modification methods.
	//////////////////////////////////////////////////////////////////////////
	void	SetPos(const Vec3d &pos, bool bWorldOnly = true);
	const Vec3d & GetPos(bool bWorldOnly = true) const;
	
	void	SetPhysAngles(const Vec3d &angl);
	void	SetAngles(const Vec3d &pos,bool bNotifyContainer=false,bool bUpdatePhysics=true,bool forceInWorld=false);
	const Vec3d & GetAngles(int realA=0) const;
		
	void SetScale( float scale );
	float GetScale() const { return m_fScale; };
	
	void SetRadius( float r ) { m_fRadius = r; SetBBox(Vec3d(-r,-r,-r),Vec3d(r,r,r)); }
	float GetRadius() const { return m_fRadius; };
	float	GetRadiusPhys() const;				// gets radius from physics bounding box			 

	virtual void SetUpdateRadius( float fUpdateRadius ) { m_fUpdateRadius = fUpdateRadius; };
	virtual float GetUpdateRadius() const { return m_fUpdateRadius; };
	
	/*determens on each side if entity direction is
		@param direction 1-front 2-back 3-left 4- right
	*/
	int GetSide(const Vec3d& direction);
	
	//! Turn on/off update calls.
	void SetNeedUpdate( bool needUpdate ) 
	{ 
		m_bUpdate = needUpdate; 
		if (needUpdate) m_bSleeping = false; 
	}
	bool NeedUpdate(){ return m_bUpdate;}

	bool IsBound() { return m_bIsBound;}

	void SetRegisterInSectors( bool needToRegister );
	
  bool IsMoving() const 
  { 
    return 
      (!IsEquivalent(m_center,m_vPrevDrawCenter,0.0025f)) || 
      (!IsEquivalent(m_angles,m_vPrevDrawAngles,0.0025f)) ||
      (m_fScale!=m_fPrevDrawScale); 
  };	
	
	//////////////////////////////////////////////////////////////////////////
	//! Implement IEntityCharacter interface.
	//@{
	virtual bool LoadCharacter( int pos,const char *fileName );
	virtual bool PhysicalizeCharacter( int pos,float mass,int surface_idx,float stiffness_scale=1.0f,bool bInstant=false );
	virtual void KillCharacter( int pos );
	virtual void SetCharacter( int pos,ICryCharInstance *character );
	virtual ICryCharInstance* GetCharacter( int pos );
	virtual bool StartAnimation( int pos,const char *animation, int iLayerID, float fBlendTime, bool bStartWithLayer0Phase = false );
	virtual void SetAnimationSpeed( const float scale=1.0f );
	//! Returns the current animation in the layer or -1 if no animation is being played 
	//! in this layer (or if there's no such layer)
  virtual int GetCurrentAnimation(int pos, int iLayerID);
	//! Return the length of the given animation in seconds; 0 if no such animation found
	virtual float GetAnimationLength( const char *animation );
	virtual bool IsAnimationPresent( int pos,const char *animation );
	virtual void DrawCharacter( int pos,int mode );
	virtual void NeedsUpdateCharacter( int pos, bool updt );
	virtual void ResetAnimations( int pos );
	virtual void SetDefaultIdleAnimation( int pos,const char * szAnimation=NULL );
	virtual void ForceCharacterUpdate( int pos );


	//@}
	//////////////////////////////////////////////////////////////////////////
	//! 
	bool Write(CStream&,EntityCloneState *cs=NULL);
	virtual bool Read( CStream& stream, bool bNoUpdate=false );
	virtual bool PostLoad();
	
	virtual bool Save( CStream& stream,IScriptObject *pStream=NULL );
	virtual bool Load( CStream& stream,IScriptObject *pStream=NULL );
	virtual bool LoadRELEASE( CStream& stream,IScriptObject *pStream=NULL );
	virtual bool LoadPATCH1( CStream& stream,IScriptObject *pStream=NULL );

	IEntityContainer* GetContainer() const { return m_pContainer; }
	void SetContainer(IEntityContainer* pContainer);
	virtual IEntityCharacter* GetCharInterface() const { return (IEntityCharacter*)this; }
	
	void EnableSave(bool bEnable){
		m_bSave=bEnable;
	}
	bool IsSaveEnabled(){
		return m_bSave;
	}
	//ICharInstanceSink
	void OnStartAnimation(const char *sAnimation);
  void OnAnimationEvent(const char *sAnimation, AnimSinkEventData UserData);
  void OnEndAnimation(const char *sAnimation);

	// Calls handler Event_##name in script.
	void CallEventHandler( const char *sEvent );

	bool IsTrackable() { return m_bTrackable; }

	void SetTimer(int msec);
	void KillTimer();
	void SetScriptUpdateRate( float fUpdateEveryNSeconds ) { m_fScriptUpdateRate = fUpdateEveryNSeconds; };

	//////////////////////////////////////////////////////////////////////////
	// State Managment public interface.
	//////////////////////////////////////////////////////////////////////////
	//! Change entity state to specified.
	//! @param sState Name of state table within entity script (case sensetive).
	//! @return true if state succesfully changed or false if state is unknown.
	virtual bool GotoState( const char *sState );
	virtual bool GotoState( int nState );
	//! Check if entity is in specified state.
	//! @param sState Name of state table within entity script (case sensetive).
	//! @return true if entity is currently in this state.
	virtual bool IsInState( const char *sState );
	virtual bool IsInState( int nState );
	virtual const char* GetState();
	virtual int GetStateIdx();

	virtual void RegisterState(const char *sState);

	virtual bool IsStateClientside() const { return m_bStateClientside; }
	virtual void SetStateClientside( const bool bEnable ){ m_bStateClientside=bEnable; }

	virtual void Hide( bool bHide );
	virtual bool IsHidden() const { return m_bHidden; };

	virtual bool WasVisible(){return m_bVisible;}
	//virtual bool IsVisible(){return m_bVisible;}

	//////////////////////////////////////////////////////////////////////////

	void SetHasEnvLighting( const bool bEnable ) { m_bHasEnvLighting = bEnable; }
	
	//////////////////////////////////////////////////////////////////////////
	// State Callbacks.
	//////////////////////////////////////////////////////////////////////////
	//! Calls script OnTimer callback in current state.
	//! @param nTimerId Id of timer set with SetTimer.
	virtual void OnTimer( int nTimerId );
	//! Calls script OnDamage callback in current state.
	//! @param pObj structure to describe the hit.- script object, to be passed to script OnDamage
	virtual void OnDamage( IScriptObject *pObj );
	
	//! Calls script OnEnterArea callback in current state. Called when the entity enters area
	//! @param entity wich enters the area
	//! @param areaID id of the area the entity is in
	virtual void OnEnterArea( IEntity* entity, const int areaID );

	//! Calls script OnLeaveArea callback in current state. Called when the entity leavs area
	//! @param entity wich leaves the area
	//! @param areaID id of the area 
	virtual void OnLeaveArea( IEntity* entity, const int areaID );

	//! Calls script OnProceedFadeArea callback in current state. Called every frame when the entity is in area 
	//! @param entity wich leaves the area
	//! @param areaID id of the area 
	//! @param fadeCoeff	[0, 1] coefficient 
	virtual void OnProceedFadeArea( IEntity* entity, const int areaID, const float fadeCoeff );

	//! Calls script OnBind callback in current state. Called on client only
	//! @entity wich was bound to this on server
	//! @param par 
	virtual void OnBind( IEntity* entity, const char par );

	//! Calls script OnUnBind callback in current state. Called on client only
	//! @entity wich was unbound to this on server
	//! @param par 
	virtual void OnUnBind( IEntity* entity, const char par );

	//! get the slot containing the steering wheel
	//! FIXME: This better be moved in the game code, together with the
	//! loadvehicle function (-1 if not found)
	int	GetSteeringWheelSlot()
	{
		return (m_nSteeringWheelSlot);
	}

	/* it sets common callback functions for those entities
		 without a script, by calling a common script function
		 for materials / sounds etc. 
		 Client side only.
	@param pScriptSystem pointer to script system
	*/
	void SetCommonCallbacks(IScriptSystem *pScriptSystem);

	// add more callbacks here...

	void LoadBreakableObject(const char *pFileName);
	IAIObject * GetAI();
	void RotateTo(const Vec3d &angles, bool bUpdatePhysics = true);
	IStatObj * GetIStatObj(unsigned int pos);

	Vec3d GetSoundPos();
	void PlaySound(ISound *pSound, float fSoundScale, Vec3d &Offset);

	bool DrawEntity(const SRendParams & EntDrawParams);
	void DrawEntityDebugInfo(const SRendParams & rParms);
	virtual float GetMaxViewDist();

	void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime);

	void	ApplyForceToEnvironment(const float radius, const float force);

	// returns true if the entity changed in any way trough moving or animation
	bool HasChanged(void);

	void CalculateInWorld(void);
	void ForceBindCalculation(bool bEnable);
	void SetParentLocale(const Matrix44 & matParent);

	int		GetBoneHitZone( int boneIdx ) const;
	void	GetHitParms( int &deathType, int &deathDir, int &deathZone ) const;

	//! Check if entitiy of type StaticEntity needs Update method to be called.
	//! Update must be called if: its have RigidBody physics or it have child entities and 
	//! this entity itself doesn't have parent entity.
	bool IsUpdateStatic() const
	{
		return ((!m_lstBindings.empty() || (m_flags&ETY_FLAG_RIGIDBODY)) && (!m_bIsBound)) || m_bEntityHasLights || m_pCryCharInstance[0] != NULL;
	};
	void UpdateHierarchy( SEntityUpdateContext &ctx );
	void ProcessEntityLightSources();
	void CheckEntityLightSourcesInEntityObjects();

	//! return pointer to object and it's transformation
	virtual IStatObj * GetEntityStatObj( unsigned int nSlot, Matrix44 * pMatrix = NULL, bool bReturnOnlyVisible = false);
	//! set entity object and it's transformation
	void SetEntityStatObj( unsigned int nSlot, IStatObj * pStatObj, Matrix44 * pMatrix = NULL );
	//! return pointer to object and it's transformation
	virtual ICryCharInstance* GetEntityCharacter( unsigned int nSlot, Matrix44 * pMatrix = NULL );

	//! inits structure needed for rendering
	void InitEntityRenderState();

	//! return true if entity has some not hiden components
	bool IsEntityHasSomethingToRender();

	//! 
	void ActivatePhysics( bool activate );

	void CheckBBox(void);

	// Particle emmiters implementation
	// todo: mov into 3dengine
	virtual int	CreateEntityParticleEmitter(int nSlotId, const ParticleParams & PartParams, float fSpawnPeriod,Vec3d vOffSet,Vec3d vDir,IParticleEffect *pEffect = 0,float fScale=1.0f);
	virtual void DeleteParticleEmitter(int nId);
	void PlayParticleSoundEffects( IParticleEffect *pEffect );

	// vis areas
	virtual bool IsEntityAreasVisible();

	bool CheckUpdateVisLevel( SEntityUpdateContext &ctx,EEntityUpdateVisLevel eUpdateVisLevel );

	//////////////////////////////////////////////////////////////////////////
	// custom/editable materials support
	virtual void SetMaterial( IMatInfo *pMatInfo );
	virtual IMatInfo* GetMaterial() const;
	//////////////////////////////////////////////////////////////////////////

	void SetHandsIKTarget( const Vec3d* target=NULL );

	void SetUpdateVisLevel(EEntityUpdateVisLevel nUpdateVisLevel);
	EEntityUpdateVisLevel GetUpdateVisLevel() { return m_eUpdateVisLevel; }

	// 
//	void	SetDeathTimer(float time) { m_fDeathTimer = time; }
	// if returns TRUE - entity wants to die
//	bool	UpdateDeathTimer( float dTime );
	void	Remove();

protected:
	//! Protected function that actually change current state.
	//! @param sState Name of state table within entity script (case sensetive).
	//! @return true if state succesfully changed or false if state is unknown.
	void InitializeStateTable( IScriptObject *pStateTable,SScriptState &scriptState );
	void ReleaseStateTable(SScriptState &scriptState );

	void OnCollide(float fDeltaTime);	

	//////////////////////////////////////////////////////////////////////////
	// Specific update functions. All called from CEntity::Update
	//////////////////////////////////////////////////////////////////////////
	void UpdateSounds( SEntityUpdateContext &ctx );
	void UpdateAIObject( bool bEntityVisible );
	//! resolve entity/entity  bbox collisions and call OnContact
	void ResolveCollision();
	void UpdatePhysics( SEntityUpdateContext &ctx );
	void UpdatePhysPlaceholders( SEntityUpdateContext &ctx );
	void UpdateCharacterPhysicsAndIK( SEntityUpdateContext &ctx );
	void UpdateLipSync( SEntityUpdateContext &ctx );
	void UpdateParticleEmitters( SEntityUpdateContext &ctx );
	void UpdateCharacters( SEntityUpdateContext &ctx );
	//////////////////////////////////////////////////////////////////////////

	//! Called from set position.
	void MoveTo( const Vec3d &pos,bool moveObjects=true,bool bUpdatePhysics=true );

	void CalcWholeBBox();

	//Timur[8/6/2002]
	// Calculates local bounding box of entity.
	void GetLocalBBox( Vec3d &min,Vec3d &max );
	void InvalidateBBox();

	void	CreatePhysicsBBox();

	//! Put entity into terrain sector.
	void RegisterInSector();
	//! Remove entity from terrain sector.
	void UnregisterInSector();
	void ForceRegisterInSectors();
	void ShutDownScript();

	//////////////////////////////////////////////////////////////////////////
	// For Triggers, handaling colliding entities.
	//////////////////////////////////////////////////////////////////////////
	//! Add collider to list.
	void AddCollider( EntityId id );
	//! Remove collider from list.
	void RemoveCollider( EntityId id );
	//! Check with physics if current colliders list is valid.
	//! Called by Update.
	void CheckColliders();
	//! Called when entity becomes visible or invisible.
	void OnVisibilityChange( bool bVisible );
	// Physics callback.
	virtual void OnPhysicsBBoxOverlap( IEntity *pOther );
	virtual void OnPhysicsStateChange( int nNewSymClass,int nPrevSymClass );
	virtual void SetPhysicsState( const char *sPhysicsState );
	//////////////////////////////////////////////////////////////////////////

	// set custom shader params for entity
	void SetShaderFloat(const char *Name, float Val);

	//////////////////////////////////////////////////////////////////////////
	// State functions helpers.
	//////////////////////////////////////////////////////////////////////////
	//! Check if state function have been implemented.
	bool IsStateFunctionImplemented( EScriptStateFunctions function );
	//! Call script state function.
	void CallStateFunction( EScriptStateFunctions function );
	template <class T1>
	void CallStateFunction( EScriptStateFunctions function,T1 param1 );
	template <class T1,class T2>
	void CallStateFunction( EScriptStateFunctions function,T1 param1,T2 param2 );
	template <class T1,class T2,class T3>
	void CallStateFunction( EScriptStateFunctions function,T1 param1,T2 param2,T3 param3 );
	//////////////////////////////////////////////////////////////////////////

	void	SwitchLights( bool bLights ) // enable/disable entity objects light sources
	{
		m_bEntityLightsOn = bLights;
	}

private:
	//////////////////////////////////////////////////////////////////////////
	// VARIABLES.
	//////////////////////////////////////////////////////////////////////////
	friend class CEntitySystem;
	
	//////////////////////////////////////////////////////////////////////////
	// Flags first (Reduce cache misses on access to entity data).
	//////////////////////////////////////////////////////////////////////////
	unsigned int m_bUpdate : 1;
	unsigned int m_bSleeping : 1;
	unsigned int m_bGarbage : 1;
	unsigned int m_bIsBound : 1;
//	unsigned int m_bForceBBox : 1; // moved into IEntityRender
	unsigned int m_bRecalcBBox : 1;
	unsigned int m_bInitialized : 1;						//!< If this entity already Initialized.
	unsigned int m_netPresence : 1;							//!< Where entity should be present.
	unsigned int m_bHidden : 1;
	unsigned int m_bTrackable : 1;							//!< Trackable in MotionTracker...
	unsigned int m_bHandIK : 1;
	unsigned int m_bForceBindCalculation : 1;
	unsigned int m_bSave : 1;										//!< Should be saved on disk(when the level is saved) by default is true
	unsigned int m_bEntityHasLights : 1;				//!< entity objects has light sources
	unsigned int m_bEntityLightsOn : 1;					//!< if the entity objects light sources are enabled
	unsigned int m_bTrackColliders : 1;					//!< If entity want to track collider and generate Enter/Leave events.
	unsigned int m_bUpdateSounds : 1;						//!< If true will update attached sounds.
	unsigned int m_bUpdateAI : 1;								//!< If set will update AI objects.
	unsigned int m_bUpdateEmitters : 1;					//!< Particle emitters present and active.
	unsigned int m_bUpdateScript : 1;						//!< True if script update function should be called.
	unsigned int m_bUpdateContainer : 1;				//!< True if container must be updated.
	unsigned int m_bUpdateCharacters : 1;				//!< True if characters must be updated.
	unsigned int m_bUpdateCamera : 1;						//!< True if must update entity camera.
	unsigned int m_bUpdateBinds : 1;						//!< True if must update binded entities.
	unsigned int m_bUpdateOnContact : 1;				//!< True if must check for OnContact collisions.
	unsigned int m_bIsADeadBody : 1;						//!< True is entity is a dead body (set in KillCharacter)
	unsigned int m_bVisible : 1;								//!< Remembers visibility state from the last update
	unsigned int m_bWasVisible : 1;							//!< Remembers visibility state from the update before the last one
	unsigned int m_bHasEnvLighting : 1;					//!< 
	unsigned int m_bStateClientside : 1;				//!< prevents error when state changes on the client and does not sync state changes to the client 

	//////////////////////////////////////////////////////////////////////////
	//! As long as this counter is not 0, entity will be forced to be updated.
	uint m_awakeCounter;
	// Update level for the entity.
	EEntityUpdateVisLevel m_eUpdateVisLevel; // defines in which case Update() function will be called
	//! entity timer used by SetTimer()
	int m_nTimer;
	//! time when the last SetTimer() was called
	//int m_nStartTimer;
	//! Physical entity attached to us.
	IPhysicalEntity	*m_physic;
	//! Physical placeholder attached.
	IPhysicalEntity *m_physPlaceholder;
	//! Pointer to system
	ISystem *m_pISystem;

	//////////////////////////////////////////////////////////////////////////
	EntityId m_nID;
	EntityClassId m_ClassId;
	string m_name;
	string m_sClassName;
	uint m_flags;

	ILipSync *m_pLipSync;

	_HScriptFunction m_pSaveFunc;
	_HScriptFunction m_pLoadFunc;
	_HScriptFunction m_pLoadRELEASEFunc;
	_HScriptFunction m_pLoadPATCH1Func;

	// common callbacks
	_HScriptFunction m_pOnCollide;
	_HScriptFunction m_pOnStopRollSlideContact;
	
	//! Name of current state.
	string	m_sStateName;
	EntityStateMap m_mapStates;
	unsigned char m_cLastStateID;

	unsigned int m_nLastVisibleFrameID;
	//@FIXME: In Development!!!
	SScriptState*	m_pClientState;
	SScriptState*	m_pServerState;
	//@FIXME: In Development!!!
		
	//! Attached camera.
	IEntityCamera *m_pCamera;

	// Flags.
	uint m_registeredInSector : 1; //when we get entity & script from server
//	uint m_static : 1; // when Entity is dtatic or moving object.
	uint m_physicEnabled : 1; // when Entity have physics enabled.
	
	//uint m_dirtyFlags;
	
	//! Time when entity was last collided.
	float m_fLastCollideTime;	
	//! Time when entity was last generated splash.
	float m_fLastSplashTime;
	
	IScriptObject			*m_pScriptObject;
	IScriptSystem			*m_pScriptSystem;

	//////////////////////////////////////////////////////////////////////////
	// collide tables

	_SmartScriptObject m_pObjectCollide;
	_SmartScriptObject m_vObjPosCollide;
	_SmartScriptObject m_vObjVelCollide;
	_SmartScriptObject m_pSplashList;
	//_SmartScriptObject m_vNormDirCollide;

	IPhysicalEntity		*m_pBBox; // for fast bbox detection and callback

	//! List of entities that are bound to this entity
	typedef std::list<EntityId> BINDLIST;
	typedef BINDLIST::iterator BINDLISTItor;
	BINDLIST m_lstBindings;

	// creation data for physics-on-demand creation
	struct RigidBodyData {
		pe_type type;
		float mass,density;
		int surface_idx;
		int slot;
	};
	struct StaticData {
		int surface_idx;
		int slot;
	};
	union PhysData {
		RigidBodyData RigidBody;
		StaticData Static;
	};
	enum phystype { PHYS_NONE=0,PHYS_STATIC=1,PHYS_RIGID=2 };
	int m_iPhysType;
	PhysData m_PhysData;
	unsigned char *m_pPhysState;
	int m_iPhysStateSize;

	CEntitySystem *m_pEntitySystem;

	IntToIntMap		m_mapSlotToPhysicalPartID;

	//! Pointer to a container object.
	IEntityContainer *m_pContainer;

	//! Not animated objects.
	std::vector<CEntityObject> m_objects;
	std::vector<IStatObj*> m_auxObjects;
	
	//! Number of animated characters.
	int m_nMaxCharNum;
	ICryCharInstance *m_pCryCharInstance[MAX_ANIMATED_MODELS];
	IPhysicalEntity *m_pCharPhysPlaceholders[MAX_ANIMATED_MODELS];
	struct charphysdata {
		float mass;
		int surface_idx;
		float stiffness_scale;
	};
	charphysdata m_charPhysData[MAX_ANIMATED_MODELS];

	//damaged model
	int	m_DmgModel;

	//! The representation of this object in AI
	IAIObject	*m_pAIObject;
	ICryBone	*m_pHeadBone;

	int				m_nIndoorArea;

	//!	attached dynamic light source
	CDLight	*m_pDynLight;

	//! Override material.
  _smart_ptr<IMatInfo> m_pMaterial;

	//!	hit parameters
	//!	set by OnDamage()
	//!	used by CPlayer::StartDeathAnimation()
	int	m_DeathType;				//!< what kind of weapon you died from	
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


	// Attached sounds.
	SoundsList m_lstAttachedSounds;

	// prev vertical velosity - used for falling damage for cars
	float		m_PrevVertVel;
	Vec3		m_vPrevVel;

	//////////////////////////////////////////////////////////////////////////
	// Definition of particle emitters associated with entity.
	//////////////////////////////////////////////////////////////////////////
	struct EntPartEmitter
	{
		Vec3d vOffset, vDir;
		float fSpawnPeriod, fLastSpawnTime;
		float fScale;
		IParticleEffect_AutoPtr pEffect;
		IParticleEmitter_AutoPtr pEmitter;
		// Constructor.
		EntPartEmitter() : vOffset(0,0,0),vDir(0,0,0),fSpawnPeriod(0),fLastSpawnTime(0),fScale(1) {};
		~EntPartEmitter();
	};
	typedef std::vector<EntPartEmitter> PatricleEmitters;
	PatricleEmitters *m_pParticleEmitters;
	//////////////////////////////////////////////////////////////////////////

	_SmartScriptObject m_pAnimationEventParams;
	float	m_fWaterDensity;

	Vec3d	m_vHandIKTarget;
	float m_fCharZOffsetCur,m_fCharZOffsetTarget;
	int m_nFlyingFrames;

	//! physics-sounds
	float m_fTimeRolling,m_fTimeNotRolling;
	float m_fRollTimeout;
	float m_fSlideTimeout;
	int m_bWasAwake;
	//////////////////////////////////////////////////////////////////////////

	int		m_nSteeringWheelSlot;

	// the bind matrix
	Matrix44 m_matParentMatrix;

	Vec3d	m_center, m_vPrevDrawCenter, m_vPrevDrawAngles;
	Ang3 m_angles;

	Vec3	m_realcenter;
	Ang3 m_realangles;
	EntityId m_idBoundTo;	// these are transmitted by the server to clients
	unsigned char m_cBind;

	Vec3	m_vBoxMin, m_vBoxMax;
	Vec3  m_vForceBBoxMin,m_vForceBBoxMax;
	float	m_fRadius; //from bbox
	//! Radius in which entity will always be update for potentially visible entities.
	float	m_fUpdateRadius;
	float m_fScriptUpdateRate; //!< How often to call update function in script (in seconds. 0 mean every frame).
	float m_fScriptUpdateTimer;

	float m_fScale, m_fPrevDrawScale;

	float m_fLastSubMergeFracion;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//! List of colliding entities, used by all triggers.
	//! When entity is first added to this list it is considered as enetring
	//! to proximity, when it erased from it it is leaving proximity.
	typedef std::set<EntityId> Colliders;
	Colliders *m_pColliders;

//	// countdown to remove entity
//	float	m_fDeathTimer;

	//! Dynamically adjustable shader parameters (like rotor rotation speed)
	TArray <struct SShaderParam> m_arrShaderParams;
};


//////////////////////////////////////////////////////////////////////////
// INLINE METHODS.
//////////////////////////////////////////////////////////////////////////

template <class T1>
inline void CEntity::CallStateFunction( EScriptStateFunctions function,T1 param1 )
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
		m_pScriptSystem->PushFuncParam(param1);
		m_pScriptSystem->EndCall();
	}
	if (funcServer)
	{
		m_pScriptSystem->BeginCall(funcServer);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam(param1);
		m_pScriptSystem->EndCall();
	}
}
//////////////////////////////////////////////////////////////////////////
template <class T1,class T2>
inline void CEntity::CallStateFunction( EScriptStateFunctions function,T1 param1,T2 param2 )
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
		m_pScriptSystem->PushFuncParam(param1);
		m_pScriptSystem->PushFuncParam(param2);
		m_pScriptSystem->EndCall();
	}
	if (funcServer)
	{
		m_pScriptSystem->BeginCall(funcServer);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam(param1);
		m_pScriptSystem->PushFuncParam(param2);
		m_pScriptSystem->EndCall();
	}
}
//////////////////////////////////////////////////////////////////////////
template <class T1,class T2,class T3>
inline void CEntity::CallStateFunction( EScriptStateFunctions function,T1 param1,T2 param2,T3 param3 )
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
		m_pScriptSystem->PushFuncParam(param1);
		m_pScriptSystem->PushFuncParam(param2);
		m_pScriptSystem->PushFuncParam(param3);
		m_pScriptSystem->EndCall();
	}
	if (funcServer)
	{
		m_pScriptSystem->BeginCall(funcServer);
		m_pScriptSystem->PushFuncParam(m_pScriptObject);
		m_pScriptSystem->PushFuncParam(param1);
		m_pScriptSystem->PushFuncParam(param2);
		m_pScriptSystem->PushFuncParam(param3);
		m_pScriptSystem->EndCall();
	}
}

#endif //Entity