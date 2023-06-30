#ifndef __ENTITY_SYSTEM_INTERFACES_H__
#define __ENTITY_SYSTEM_INTERFACES_H__

#pragma once

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the CRYENTITYDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// CRYENTITYDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#if !defined _XBOX && !defined(LINUX)
#ifdef CRYENTITYDLL_EXPORTS
#define CRYENTITYDLL_API __declspec(dllexport)
#else
#define CRYENTITYDLL_API __declspec(dllimport)
#endif
#else
#define CRYENTITYDLL_API
#endif

// !!! don't change the type !!!
typedef unsigned short EntityClassId;			//! unique identifier for the entity class (defined in ClassRegistry.lua)
typedef ULONG_PTR BoneBindHandle;

class CEntityDesc;
struct I3DEngine;

// Common
#include <Cry_Math.h>
#include <Cry_Camera.h>
#include "IEntityRenderState.h"

// Forward declarations.
class cAIBrain;
class IPhysicalEntity;
class CryCharInstance;
//class CImage;
struct IStatObj;
struct pe_cargeomparams;
struct pe_params_car;
struct IEntitySystemSink;

// Forward declare interfaces.
struct IEntitySystem;
struct IEntityCharacter;
struct IEntityContainer;
struct ISystem;
struct ISound;
struct IScriptSystem;
struct IEntityCamera;
struct ICryCharInstance;
struct AIObjectParameters;
struct IAIObject;
struct ILipSync;
struct IParticleEffect;
struct IServerSlot;


// all available events. if you want to add an event, dont forget to register a lua-constant in the EntitySystem:Init() !
enum EScriptEventId
{
	ScriptEvent_Activate = 0x00000001,
	ScriptEvent_Deactivate ,
	ScriptEvent_FireModeChange ,
	ScriptEvent_DropItem ,
	ScriptEvent_Reset		 ,
	ScriptEvent_Contact		 ,
	ScriptEvent_Enter		 ,
	ScriptEvent_Leave		 ,
	ScriptEvent_Timer		 ,
	ScriptEvent_StartAnimation		,
	ScriptEvent_AnimationKey		,
	ScriptEvent_EndAnimation		,
	ScriptEvent_Respawn		,
	ScriptEvent_ItemActivated ,
	ScriptEvent_Hit ,
	ScriptEvent_Fire ,
	ScriptEvent_WeaponReady ,
	ScriptEvent_StopFiring ,
	ScriptEvent_Reload ,
	ScriptEvent_Command ,
	ScriptEvent_FireGrenade ,
	ScriptEvent_Die ,
	ScriptEvent_ZoomToggle ,
	ScriptEvent_ZoomIn ,
	ScriptEvent_ZoomOut ,
	ScriptEvent_Land ,
	ScriptEvent_FireCancel ,
	ScriptEvent_GameDefinedEvent ,
	ScriptEvent_ViewModeChange ,
	ScriptEvent_SelectWeapon,
	ScriptEvent_Deafened,
	ScriptEvent_StanceChange,
	ScriptEvent_CycleGrenade,
	ScriptEvent_Use,
	ScriptEvent_MeleeAttack,
	ScriptEvent_PhysicalizeOnDemand,
	ScriptEvent_PhysCollision,
	ScriptEvent_FlashLightSwitch,
	ScriptEvent_EnterWater,
	ScriptEvent_CycleVehiclePos,
	ScriptEvent_AllClear,				// sent when the main player has no opposition around him
	ScriptEvent_Expression,
	ScriptEvent_InVehicleAnimation,
	ScriptEvent_InVehicleAmmo,
  ScriptEvent_ProcessCharacterEffects,
	ScriptEvent_Jump,//! jump event
};

//! Draw mode
enum eDrawMode
{
	ETY_DRAW_NONE	= 0,
	ETY_DRAW_NORMAL	= 1,
	ETY_DRAW_NEAR	= 2
};

//! Object info flags
enum eObjInfoFlags
{
	ETY_OBJ_INFO_DRAW = 1,
	ETY_OBJ_INFO_DRAW_NEAR = 2,
	ETY_OBJ_USE_MATRIX = 4,
	ETY_OBJ_IS_A_LINK = 8
};

//! Entity flags:
enum eEntityFlags
{
	ETY_FLAG_WRITE_ONLY	= 1,
	ETY_FLAG_NOT_REGISTER_IN_SECTORS = 2,
	ETY_FLAG_CALC_PHYSICS = 4,
	//ETY_FLAG_DRAW_MODEL		= 8,
	ETY_FLAG_CLIENT_ONLY	= 16,
	ETY_FLAG_NEED_UPDATE	= 32,
	ETY_FLAG_DESTROYABLE	= 64,
	//ETY_FLAG_DRAW_NEAR		= 64
	ETY_FLAG_RIGIDBODY	= 128,
	ETY_FLAG_CALCBBOX_USEALL	= 256,		// use character and objects in BBOx calculations
	ETY_FLAG_IGNORE_PHYSICS_UPDATE = 512,	// Used by Editor only, (dont set)
	ETY_FLAG_CALCBBOX_ZROTATE	= 1024		// use only z angle when calculation bbox
};

// Misc
enum eMiscEnum
{
	PLAYER_MODEL_IDX = 0,
	MAX_ANIMATED_MODELS = 2
};

// object types - bitmask 0-terrain 1-static, 2-sleeping, 3-physical, 4-living
enum PhysicalEntityFlag
{
	PHYS_ENTITY_STATIC = (1<<1),
	PHYS_ENTITY_DYNAMIC = (1<<2)|(1<<3),
	PHYS_ENTITY_LIVING = (1<<4),
	PHYS_ENTITY_ALL = (1<<1)|(1<<2)|(1<<3)|(1<<4)
};

struct IEntity;

/*! Wrapper class for geometry attached to an entity. The entity contains slots which contain one entity object each.
 These objects can be arbitrarily offseted and rotated with respect to the entity that contains them.
*/
class CEntityObject
{
public:
	CEntityObject()
	{
		flags = 0;
		pos(0,0,0);
		angles(0,0,0);
		scale(0,0,0);
		object = 0;

    mtx.SetIdentity();
	}

	int		flags;
	Vec3	pos;
	Vec3	angles;
	Vec3	scale;
	Matrix44 mtx;

	IStatObj*	object;
//## [kirill] character newer used here
//##	ICryCharInstance *m_pCryCharInstance;

	// flags for "spring" objects that are connected to 2 other entity parts (ETY_OBJ_IS_LINKED)
	int ipart0,ipart1;
	Vec3 link_start0,link_end0;
};

struct IScriptObject;

/*! Entity iterator interface. This interface is used to traverse trough all the entities in an entity system. In a way,
	this iterator works a lot like a stl iterator.
*/
struct IEntityIt
{
	virtual void AddRef() = 0;

/*! Deletes this iterator and frees any memory it might have allocated.
*/
	virtual void Release() = 0;

/*! Check whether current iterator position is the end position.
	@return True if iterator at end position.
*/
	virtual bool IsEnd() = 0;

/*! Retrieves next entity
	@return The entity that the iterator points to before it goes to the next
*/
	virtual IEntity * Next() = 0;

/*! Positions the iterator at the begining of the entity list
*/
	virtual void MoveFirst() = 0;
};

#include <ICryAnimation.h>

class CXServerSlot;

//
struct EntityCloneState
{
	//! constructor
	EntityCloneState()
	{
		m_bLocalplayer=false;
		m_bSyncYAngle=true;
		m_bSyncAngles=true;
		m_bSyncPosition=true;
		m_fWriteStepBack=0;
		m_bOffSync=true;
		m_pServerSlot=0;
	}

	//! destructor
	EntityCloneState(const EntityCloneState& ecs)
	{
		m_pServerSlot=ecs.m_pServerSlot;
		m_v3Angles=ecs.m_v3Angles;
		m_bLocalplayer=ecs.m_bLocalplayer;
		m_bSyncYAngle=ecs.m_bSyncYAngle;
		m_bSyncAngles=ecs.m_bSyncAngles;
		m_bSyncPosition=ecs.m_bSyncPosition;
		m_fWriteStepBack=ecs.m_fWriteStepBack;
		m_bOffSync=ecs.m_bOffSync;
	}

	// ------------------------------------------------------------------------------

	CXServerSlot *	m_pServerSlot;			//!< destination serverslot, 0 if not used
	Vec3						m_v3Angles;					//!<
	bool						m_bLocalplayer;			//!< say if this entity is the entity of the player
	bool						m_bSyncYAngle;			//!< can be changed dynamically (1 bit), only used if m_bSyncAngles==true, usually not used by players (roll)
	bool						m_bSyncAngles;			//!< can be changed dynamically (1 bit)
	bool						m_bSyncPosition;		//!< can be changed dynamically (1 bit)
	float						m_fWriteStepBack;		//!<
	bool						m_bOffSync;					//!<
};


// types of dependance of entity update from visibility
enum EEntityUpdateVisLevel
{
  eUT_Always=0,			//! Always update entity.
  eUT_InViewRange,	//! Only update entity if it is in view range.
  eUT_PotVisible,		//! Only update entity if it is potentially visible.
  eUT_Visible,			//! Only update entity if it is visible.
	eUT_Physics,			//! Only update entity if it is need to be updated due to physics.
	eUT_PhysicsVisible,	//! Only update entity if it is need to be updated due to physics or if it is visible.
  eUT_Never,				//! Never update entity.
	eUT_PhysicsPostStep, //! Update only when PostStep is called from the physics
	eUT_Unconditional  //! Update regardless of anything - this has to be explicitly set
};

/*! Entity Update context structure.
 */
struct SEntityUpdateContext
{
	//! ScriptObject with Update params for script.
	IScriptObject *pScriptUpdateParams;
	//! Current rendering frame id.
	int nFrameID;
	//! Current camera.
	CCamera *pCamera;
	//! Current system time.
	float fCurrTime;
	//! Delta frame time (of last frame).
	float fFrameTime;
	//! If set to true must profile entity update to log.
	bool bProfileToLog;
	//! Number of updated entities.
	int numUpdatedEntities;
	//! Number of visible and updated entities.
	int numVisibleEntities;
	//! Maximal view distance.
	float fMaxViewDist;
	//! Maximal view distance squared.
	float fMaxViewDistSquared;
	//! Camera source position.
	Vec3 vCameraPos;

	//! Initialization ctor.
	SEntityUpdateContext() : pScriptUpdateParams(NULL),nFrameID(0),pCamera(0),fCurrTime(0),
														bProfileToLog(false),numVisibleEntities(0),numUpdatedEntities(0),fMaxViewDist(1e+8) {};
};

/*! Entity interface.

		The public interface of the CEntity class. Contains functions for managing an entity, controlling its position/orientation
		in the world, loading geometry for the entity, physicalizing an entity, etc.
*/
struct IEntity :
public IEntityRender,
public ICharInstanceSink
{
public:
/*!	Retrieves the unique identifier of this entity assigned to it by the Entity System.
	@return The entity id as an unsigned short if succesfull. This function always suceeds if a pointer to the interface is acquired.
*/
	virtual	EntityId GetId() const = 0;

	// Description:
	//     Updates the Entitys internal structures once a frame.
	//		 Normally called from IEntitySystem::Update function.
	// See Also:
	//     IEntitySystem::Update, SEntityUpdateContext
	// Arguments:
	//     updateContext - Structure that contain general information needed to update entity,
	//                     Update time, Camera, etc..
	virtual void Update( SEntityUpdateContext &updateContext ) = 0;

	// Description:
	//     Reset entity to initial state.
	//     This function is used by Sandbox editor to restore the state of entity when going in or out of game mode.
	//     It will call also OnReset callback of entity script.
	virtual void	Reset() = 0;

	// Description:
	//     Retrieves the net presence state of the entity.
	//     Net presence detrermine if entity must be synchronized other network.
	// Returns:
	//     true - Entity should be present in network game.
	//     false - Entity should not be present in network game.
	virtual bool GetNetPresence() const = 0;

	// Description:
	//     Set the the net presence state of the entity.
	//     Net presence detrermine if entity must be synchronized other network.
	// Arguments:
	//     bPresent - True if the entity needs to be synchronized over network
	virtual void SetNetPresence( bool bPresent ) = 0;

	// Description:
	//     Changes entity name.
	//     Entity name does not have to be unique, but for the sake of easier finding entities by name it is better to not
	//     assign same name to different entities.
	// See Also:
	//     GetName
	// Arguments:
	//     name - New name for the entity.
	virtual void  SetName( const char *name ) = 0;

	// Description:
	//     Set class name of the entity.
	//     The class name is the name of the lua table that represents this entity in script.
	// See Also:
	//     GetClassName
	// Arguments:
	//     name - Name of the script table.
	virtual void  SetClassName( const char *name)  = 0;

	// Description:
	//     Get entity name.
	// See Also:
	//     SetName
	// Returns:
	//     Name of the entity.
	virtual const char *GetName() const = 0;

	// Description:
	//     Get class name of the entity.
	//     The class name is the name of the lua table that represents this entity in script
	// See Also:
	//     SetEntityClassName
	// Returns:
	//     Name of the entity class.
	virtual const char * GetEntityClassName() const = 0;

	// Description:
	//     Get description of entity in a CEntityDesc structure.
	//     This describes the entity class, entity id and other parameters.
	// See Also:
	//     Spawn, CEntityDesc
	// Arguments:
	//     desc - This parameter will be filled with entity description.
	virtual void GetEntityDesc( CEntityDesc &desc ) const = 0;

	// Description:
	//     Retrieves the position of a helper defined in geometry or character loaded in this entity.
	//     This function will search helper inside objects and chracters in all entity slots.
	// Arguments:
	//     helper	- The name of the helper
	//     pos	  - The returned position of the helper. If helper not found, this is a zero vector.
	//     objectspace - If true, paremeter pos return helper position in object space, otherwise in world space.
	virtual void GetHelperPosition(const char *helper, Vec3 &pos, bool objectspace = false) = 0;

	// Description:
	//     Retrieves the class id of the entity.
	//     Example: If there are five ammoboxes for bullets, they all have the same type (ammo for bullet)
	//     but they have different entity id's. The valid types are registered previously in the CEntityRegistry
	// Returns:
	//     Id of the entity class.
	// See Also:
	//     GetId, IEntityClassRegistry
	virtual EntityClassId GetClassId() = 0;

	// Description:
	//     Set the entity class id.
	// Arguments:
  //     ClassId - The desired entity type. This must be a valid class id registered previously in the IEntityClassRegistry.
	// See Also:
	//     IEntityClassRegistry
	virtual void SetClassId(const EntityClassId ClassId) = 0;

	// Description:
	//     Frees any memory that the entity class might be using. This method is called before an entity is destroyed,
	//     and provides a way for the entity to clean up after itself.
	virtual void ShutDown() = 0;

	// Description:
	//     Check if this entity was marked for deletion.
	//     If this function returns true, it will be deleted on next frame, and it is pointless to perform any operations on such entity.
	// Returns:
	//     True if entity marked for deletion, false otherwise.
	virtual bool IsGarbage() = 0;

	// Description:
	//     Get the flags of an entity sub object
	// Arguments:
	//     nSubObj - Index of the object slot to get the flags from.
	// Returns:
 	//     Flags for the object slot, or 0 if there`s no such slot.
	virtual int	GetObjectsFlags(int nSubObj) = 0;

	// Description:
	//     Sets the flags of an entity.
	// See Also:
	//     eEntityFlags
	// Arguments:
	//     flags - Flags to be set from eEntityFlags enum, they will be ORed with flags already in the entity (m_flags |= flags).
	virtual void	SetFlags( unsigned int flags ) = 0;

	// Description:
	//     Sets the flags of an entity.
	// See Also:
	//     eEntityFlags
	// Arguments:
	//     flags - Flags to be removed from eEntityFlags enum, they will be removed from the flags theat already in the entity (m_flags &= ~flags).
	virtual void ClearFlags( unsigned int flags ) = 0;

	// Description:
	//     Get the flags of an entity.
	// See Also:
	//     eEntityFlags
	// Returns:
	//     Entity flags, a combination of the flags from eEntityFlags enum.
	virtual unsigned int GetFlags() = 0;

	// Description:
	//     Get camera attached to this entity.
	//     Entity camera is an any class implementing IEntityCamera interface.
	// Returns:
	//     Pointer to an IEntityCamera interface that defines the camera.
	virtual IEntityCamera* GetCamera() const = 0;

	// Description:
	//     Attach a camera to this entity.
	// Arguments:
	//     cam - Pointer to an IEntityCamera interface that defines the camera.
	virtual void SetCamera( IEntityCamera *cam ) = 0;

	// Description:
	//     Get light attached to entity.
	// Returns:
	//     Pointer to CDLight structure that defines light parameters.
	virtual class CDLight*	GetLight( ) = 0;

	// Description:
	//     Initialize and attach light source to an entity.
	//     At the same time load projected texture and light shader if specified.
	// See Also:
	//     GetLight
	// Arguments:
	//     img - Filename of the projected texture, (DDS file) or NULL if no projected texture.
	//     shader - Name of the light shader, or NULL if not using light shader.
	//     bUseAsCube - When true, projected texture will be projected on all 6 sides arround light.
	//     fAnimSpeed - When using animated projected texture, this will specify animation speed for animated texture.
	//     nLightStyle - Style o light
	//     fCoronaScale - Scale of the light corona, if 0 light corona will not be visible.
	//                    Light corona is the flare image displayed at the position of the light source, that simulate glow of light.
	// Returns:
	//     True if light was successfully initialized and attched.
	virtual bool	InitLight( const char* img=NULL, const char* shader=NULL, bool bUseAsCube=false, float fAnimSpeed=0, int nLightStyle=0, float fCoronaScale=0 ) = 0;


	// Description:
	//     Fires an event in the entity script.
	//     This will call OnEvent(id,param) Lua function in entity script, so that script can handle this event.
	// See Also:
	//     EScriptEventId, IScriptObject
	virtual void SendScriptEvent(enum EScriptEventId Event, IScriptObject *pParamters, bool *pRet=NULL) = 0;
	virtual void SendScriptEvent(enum EScriptEventId Event, const char *str, bool *pRet=NULL )=0;
	virtual void SendScriptEvent(enum EScriptEventId Event, int nParam, bool *pRet=NULL )=0;

	// Description:
	//     Calls script defined event handler.
	//     Script defined event handler is any function within Lua entity table, that starts with "Event_" characters.
	//     After Event_ follows the name of the event itself.
	//     Ex: Door.Event_Open(sender) - Event name is Open.
	// Arguments:
	//     sEvent - Name of event, in script function Event_[sEvent] inside entity table will be called.
	virtual void CallEventHandler( const char *sEvent ) = 0;


	// Description:
	//     Get Physical Entity associated with this entity.
	//     Physical entity is an object instance within physical world with its own transformation.
	//     Physical entity can have associate geometry to perform collision detection with, they have thier physical properties (mass,velocity)
	//     They also have reference to thier originating Entity, not all Physical entities are created by entity system, there are physical entities
	//     that represent static geometry in the world like vegetation,brushes and so on.
	// See Also:
	//     SetPhysics, DestroyPhysics, IPhysicalEntity.
	// Returns:
	//	An interface to a IPhysicalEntity if one exists, NULL if it doesn't exist.
	virtual class IPhysicalEntity* GetPhysics() const = 0;

	// Description:
	//     Set Physical Entity to be used by this entity.
	// See Also:
	//     GetPhysics, DestroyPhysics
	// Arguments:
	//     physic	- A pointer to the IPhysicalEntity interface.
	virtual void SetPhysics( IPhysicalEntity* physic ) = 0;

/*! Destroys the physics for this entity.
*/
	virtual void DestroyPhysics() = 0;

/*! Creates physical entity on demand
	@param iForeignFlags - 'foreign flags' specified when creating placeholder, can specify entity subtype (main/character etc.)
	@return nonzero if successful
*/
	virtual int CreatePhysicalEntityCallback(int iForeignFlags) = 0;

/*! Notification upon dephysicalisation of a temporary physical entity (this function should save entity state if necessary)
	@param pent - pointer to physical entity
	@return nonzero if successful
*/
	virtual int DestroyPhysicalEntityCallback(IPhysicalEntity *pent) = 0;

	// Description:
  //     Enables or disables physic calculation for this entity.
	//     When physics for entity is disabled, it will also suspend physics entity to be simulated or collided with in physics engnie.
	//     When enable argument is false this function will only unregister entity from the physics engine so that it does not recognized
	//     and not simulated anymore, but not destroyed so enabling/disabling physics entity is relatively cheap.
	// Arguments:
	//     enable - If true physics will be calculated for this entity, if false physics entity will be deactivated.
	virtual void EnablePhysics( bool enable ) = 0;

	// Description:
	//     Adds physical impulse to the entity.
	//     Will only affect entities with dynamic physic entity (RigidBody,Chracter).
	// Arguments:
	//     ipart   - Part identifier, passed to physics with AddGeometry, usually identify chracter bone.
	//     pos     - Position in world space where to apply impulse.
	//     impulse - Impulse direction and magnitude vector specified in world space coordinates.
	//     bPos    - When true, pos parameter will be used and valid position must be specified.
	//     fAuxScale - Auxilary scale for impulse magnitude.
	virtual void AddImpulse(int ipart,Vec3 pos,Vec3 impulse,bool bPos=true,float fAuxScale=1.0f) = 0;

	// Description:
	//     Physicalize this entity as a rigid body.
	//     This function create rigid body physic object and associate it with this entity.
	// SeeAlso:
	//     GetPhysics,DestroyPhysics,EnablePhysics,CreateLivingEntity,CreateStaticEntity,CreateSoftEntity
	// Arguments:
	//     type - physical entity type (can be PE_RIGID or something else, say, PE_ARTICULATED)
	//     density - The density of matter of the rigid body (Object mass will be automatically calculted).
	//               if -1 then mass must be specified and desnity will be automatically calculated from geometry volume.
	//     mass - The mass of the rigid body, -1 if not specified, density must be specified in this case and mass will be calculated from geometry volume.
	//            if 0 it is considered as infinite mass rigid body, it will behave same was as static physical entity,
	//            (will not be simulated as rigid body but will participate in collision detection).
	//     surface_id - (Depricated) The identifier of the physical surface for this body.
	//     pInitialVelocity	- Pointer to an initial velocity vector, if NULL no initial velocity.
	//     slot - Make a rigid body from the object in this slot, if -1 chooses first non empty slot.
	//     bPermanent - For physics on demand (Leave on default).
	virtual bool CreateRigidBody(pe_type type, float density,float mass,int surface_id,Vec3* pInitialVelocity = NULL, int slot=-1,
		bool bPermanent=false) = 0;

	// Description:
	//     Physicalize this entity as a living entity (ex. human chracter, monster,...)
	//     This function create living physic object and associate it with this entity.
	//     Living entity usually contain 2 physical entities:
	//       * Bounding cylinder with 2 optional additional sphere at the top and bottom of cylinder, for collision with surounding geometry.
	//       * Articulated chracter with IK for precise collision detection when shooting rays that hit this chracter or applying impulses.
	// SeeAlso:
	//     GetPhysics,DestroyPhysics,EnablePhysics,CreateLivingEntity,CreateStaticEntity,CreateSoftEntity
	// Arguments:
	//     density - The density of matter of the rigid body (Object mass will be automatically calculted).
	//               if -1 then mass must be specified and desnity will be automatically calculated from geometry volume.
	//     mass - The mass of the rigid body, -1 if not specified, density must be specified in this case and mass will be calculated from geometry volume.
	//            if 0 it is considered as infinite mass rigid body, it will behave same was as static physical entity,
	//            (will not be simulated as rigid body but will participate in collision detection).
	//     eye_height - The distance from ground level to the eyes of this living entity.
	//     sphere_height - The distance from ground level to the center of the bounding sphere of this living entity
	//     radius	- The radius of the bounding cylinder.
	virtual bool CreateLivingEntity(float mass, float height, float eye_height, float sphere_height, float radius,int nSurfaceID, float fGravity , float fAirControl, bool collide=false)=0;

	// Description:
	//     Physicalize this entity as a static physic geometry.
	//     This entity can be moved explicitly, but physics will never try to move or rotate this object, and will treat it as an object with infinite mass.
	// SeeAlso:
	//     GetPhysics,DestroyPhysics,EnablePhysics,CreateLivingEntity,CreateStaticEntity,CreateSoftEntity
	// Arguments:
	//     mass - (Depricated) Not used.
	//     surface_idx - (Depricated) Identifier of the surface of this static object.
	//     slotToUse - Object from this entity slot will be used as a physics geometry, if -1 combined geometry from all loaded slots will be used.
	//     bPermanent - For physics on demand (Leave on default).
	virtual bool CreateStaticEntity(float mass, int surface_idx, int slotToUse=-1, bool bPermanent=false)=0;

/*! Physicalize entity as a static object.
	@param mass		Mass of soft object
	@param density Density of soft object
	@param bCloth	treat object as cloth (remove the longest internal spans from triangles)
	@param pAttachTo attach to this physical entity
	@param iAttachToPart attach to this part of pAttachTo
*/
	virtual bool CreateSoftEntity(float mass,float density, bool bCloth=true, IPhysicalEntity *pAttachTo=WORLD_ENTITY,int iAttachToPart=-1)=0;

/*! Load and create vehicle physics. This function actually loads a vehicle from a cgf file, and according to some helper data that it finds
 in the cgf file creates a physical representation of a vehicle.
	@param objfile	Name of cgf file
	@param pparts		An array of geometry parts of this vehicle.
	@param params   General parameters of this vehicle
	@see pe_cargeomparams
	@see pe_params_car
*/
	virtual bool LoadVehicle(const char *objfile, pe_cargeomparams *pparts, pe_params_car *params,bool bDestroy=false) = 0;

/*! Load and create boat physics. This function actually loads a boat from a cgf file, and according to some helper data that it finds
 in the cgf file creates a physical representation of a vehicle.
	@param objfile	Name of cgf file
	@param mass			mass of mass proxy.
*/
	virtual bool LoadBoat(const char *objfile, float mass, int surfaceID) = 0;

/*! damage <<FIXME>> remove it
	@param damage model number
*/
	virtual void SetDamage(const int dmg) = 0;

/*! Add geometry object to entity, which is loaded from a cgf file into an available slot in the entity. There is no limit on the number of
	cgf files that can be loaded into an entity.
	@param	slot	Identifier of the slot into which this object should be loaded. Note, if any object exists at this slot, it will be overwritten.
	@param	fileName	The filename of the cgf
	@param  scale	The desired scale of the object
	@return True upon successful loading of cgf file
*/
	virtual bool	LoadObject( unsigned int slot,const char *fileName,float scale, const char *geomName=NULL) = 0;
	virtual bool GetObjectPos(unsigned int slot,Vec3 &pos)=0;
	virtual bool SetObjectPos(unsigned int slot,const Vec3 &pos)=0;
	virtual bool GetObjectAngles(unsigned int slot,Vec3 &ang)=0;
	virtual bool SetObjectAngles(unsigned int slot,const Vec3 &ang)=0;
/*! Load a pre-broken object into the entity, piece by piece from the cgf-file. The pre broken pieces will be loaded in a separate slot
  each with a separate IStatObj pointer.
	@param	fileName	The filename of the cgf
	@see    IStatObj
*/
	virtual void	LoadBreakableObject( const char *fileName) = 0;

/*! Assign object to specified slot.
	@param slot Identifier of the slot.
	@param object The object to be put in this slot.
	@see CEntityObject
*/
	virtual bool	SetEntityObject( unsigned int slot,const CEntityObject &object ) = 0;

/*! Get object at specified slot.
	@param slot Identifier of the slot.
	@param object The returned object at this slot
	@see CEntityObject
*/
	virtual bool	GetEntityObject( unsigned int slot,CEntityObject &object ) = 0;
/*! Get number of attached objects.
	@return number of objects in this entity
*/
	virtual int		GetNumObjects() = 0;

/*! Get Static object interface from an object attached to an entity.
	@param pos	The slot at which this object is to be retrieved from
	@return Pointer to an IStatObj object interface
	@see IStatObj
*/
	virtual struct IStatObj *GetIStatObj(unsigned int pos) = 0;

/*! Play sound from entity position
	@param pSound Sound-handle
	@param fSoundScale Sound-scale factor
	@param Offset Offset to value returned by CalcSoundPos() in IEntityContainer or GetPos() if the former doesnt exist
*/
	virtual void PlaySound(ISound *pSound, float fSoundScale, Vec3 &Offset) = 0;

/*! Control draw method of an object at a specific slot. Mode can be ETY_DRAW_NORMAL, ETY_DRAW_NEAR or ETY_DRAW_NONE.
	NOTE: The static object slots are different than the character animated object slots.
	@param	pos	The slot of the object that we are modifying.
	@param	mode The desired drawing mode of this object
*/
	virtual void	DrawObject(unsigned int pos,int mode) = 0;

/*! Control draw method of all objects. Mode can be ETY_DRAW_NORMAL, ETY_DRAW_NEAR or ETY_DRAW_NONE.
	NOTE: The static object slots are different than the character animated object slots.
	@param	mode The desired drawing mode of this object
*/
	virtual void	DrawObject(int mode) = 0;

/*! Control drawing of character animated object at a specific slot.Mode can be ETY_DRAW_NORMAL, ETY_DRAW_NEAR or ETY_DRAW_NONE.
	NOTE: The character animated object slots are different than the static object slots.
	@param	pos	The slot of the character that we are modifying.
	@param	mode The desired drawing mode of this character
*/
	virtual void DrawCharacter(int pos, int mode) = 0;

/*! Control bones update of character animated object at a specific slot.
	NOTE: The character animated object slots are different than the static object slots.
	@param	pos	The slot of the character that we are modifying.
	@param	updt update switch
*/
	virtual void NeedsUpdateCharacter( int pos, bool updt ) = 0;


/*! Set Axis Aligned bounding box of entity.
	@param mins	Bottom left close corner of box
	@param maxs Top Right far corner of box
	@param bForcePhysicsCallback forces to create a physics object to check for contact
*/
	virtual void	SetBBox(const Vec3 &mins,const Vec3 &maxs) = 0;

/*! Get Axis Aligned bounding box of entity.
	@param mins The value that contains the bottom left close corner of box after function completion
	@param maxs The value that contains the Top Right far corner of box after function completion
*/
	virtual void	GetBBox( Vec3 &mins,Vec3 &maxs ) = 0;

/*! Get Axis Aligned bounding box of entity in local space.
	NOTE: this function do not hash local bounding box, so it must be calculated every time this
	function is called.
	@param mins The value that contains the bottom left close corner of box after function completion
	@param maxs The value that contains the Top Right far corner of box after function completion
*/
	virtual void GetLocalBBox( Vec3 &min,Vec3 &max ) = 0;

/*! Marks internal bbox invalid, it will be recalculated on next update.
 */
	virtual void InvalidateBBox() = 0;

/*! Enables tracking of physical colliders for this entity.
*/
	virtual void TrackColliders( bool bEnable ) = 0;

/*! Rendering of the entity.
*/
	virtual bool DrawEntity(const struct SRendParams & EntDrawParams) = 0;

/*! Physicalize this entity as a particle.

*/
	virtual bool CreateParticleEntity(float size,float mass, Vec3 heading, float acc_thrust=0,float k_air_resistance=0,
		float acc_lift=0,float gravity=-9.8, int surface_idx=0,bool bSingleContact=true) = 0;

//! Various accessors to entity internal stats
//@{
	virtual void SetPos(const Vec3 &pos, bool bWorldOnly = true) = 0;
	virtual const Vec3 & GetPos(bool bWorldOnly = true) const = 0;

	virtual void SetPhysAngles(const Vec3 &angl) = 0;
	virtual void SetAngles(const Vec3 &pos,bool bNotifyContainer=true,bool bUpdatePhysics=true,bool forceInWorld=false) = 0;
	virtual const Vec3 & GetAngles(int realA=0) const = 0;

	virtual void SetScale( float scale ) = 0;
	virtual float GetScale() const = 0;

	virtual void SetRadius( float r ) = 0;
	virtual float GetRadius() const = 0;
	virtual float	GetRadiusPhys() const = 0;	// gets radius from physics bounding box
	//@}

	//! Sets the entity in sleep mode on/off
	virtual void	SetSleep(bool bSleep) = 0;

/*! Control the updating of this entity. This can be used to specify that an entity does not need to be updated every frame.
	@param needUpdate		If true, entity will be updated every frame.
*/
	virtual void SetNeedUpdate( bool needUpdate ) = 0;
/*! return true if the entity must be updated
*/
	virtual bool NeedUpdate() = 0;
/*! Turn on/off registration of entity in sectors.
	@param needToRegister If true entity will be registered in terrain sectors.
*/
	virtual void SetRegisterInSectors( bool needToRegister ) = 0;

	//! Set radius in which entity will be always updated.
	virtual void SetUpdateRadius( float fUpdateRadius ) = 0;
	//! Get radius in which entity will be always updated.
	virtual float GetUpdateRadius() const = 0;

	//! If entity registered in sectors, force to reregister it again.
	virtual void ForceRegisterInSectors() = 0;

	//! Check if current position is different from previous update
  virtual bool IsMoving() const = 0;

	//! Check if entity is bound to another entity
	virtual bool IsBound() = 0;

	//! Bind another entity to this entity. Bounded entities always change relative to their parent.
	//! @param id The unique id of the entity which should be bounded to this entity.
	//! @param cBind
	//! @param bClientOnly true=don't send this message to the server again because it came from it
	virtual void Bind(EntityId id,unsigned char cBind=0, const bool bClientOnly=false, const bool bSetPos=false )=0;

	//! Unbind another entity from this entity.
	//! @param id The unique if of the entity which needs to be unbinded from this entity.
	//! @param bClientOnly true=don't send this message to the server again because it came from it
	virtual void Unbind(EntityId id,unsigned char cBind, const bool bClientOnly=false )=0;

/*! Forces the entity to act like a bind entity, when in reality it doesn't have to be
 */
	virtual void ForceBindCalculation(bool bEnable) = 0;

/*! Sets the parent space parameters (locale)
 */
	virtual void SetParentLocale(const Matrix44 &matParent)= 0;

/*! calculate world position / angles
 */
	virtual	void CalculateInWorld(void) = 0;


/*! Attach another entity to this entity bone. Bounded entities always change relative to their parent.
	@param id The unique if of the entity which needs to be attached.
	@bone name
*/
	virtual void AttachToBone(EntityId id, const char* boneName)=0;

/*! Attach object from SLOT to bone
	@param slot object slot idx
	@bone name
	@return ObjectBind handle.
*/
	virtual BoneBindHandle AttachObjectToBone(int slot,const char* boneName,bool bMultipleAttachments=false, bool bUseZOffset=false)=0;

/*! Detach object from bone
	@bone name
	@param objectBindHandle If negative remove all objects from bone.
*/
	virtual void DetachObjectToBone(const char* boneName,BoneBindHandle objectBindHandle=-1 )=0;

/*! Set the script object that describes this entity in the script.
	@param pObject	Pointer to the script object interface
	@see IScriptObject
*/
	virtual void SetScriptObject(IScriptObject *pObject) = 0;

/*! Get the script object that describes this entity in the script.
	@return	Pointer to the script object interface
	@see IScriptObject
*/
	virtual IScriptObject *GetScriptObject()= 0 ;

	virtual bool Write(CStream&,EntityCloneState *cs=NULL) = 0;
	/*!	read the object from a stream(network)
		@param stm the stream class that store the bitstream
		@param bNoUpdate if true, just fetch the data from the stream w/o applying it
		@return true if succeded,false failed
		@see CStream
	*/
	virtual bool Read(CStream&,bool bNoUpdate=false) = 0;
	/*!	is called for each entity after ALL entities are read (this inter-entity connections can be serialized)
	*/
	virtual bool PostLoad() = 0;
	/*! check if the object must be syncronized since the last serialization
		@return true must be serialized, false the object didn't change
	*/
	//virtual bool IsDirty() = 0;

	/*!	serialize the object to a bitstream(file persistence)
		@param stm the stream class that will store the bitstream
		@param pStream script wrapper for the stream(optional)
		@return true if succeded,false failed
		@see CStream
	*/
	virtual bool Save(CStream &stm,IScriptObject *pStream=NULL) = 0;
	/*!	read the object from a stream(file persistence)
		@param stm the stream class that store the bitstream
		@param pStream script wrapper for the stream(optional)
		@return true if succeded,false failed
		@see CStream
	*/
	virtual bool Load(CStream &stm,IScriptObject *pStream=NULL) = 0;
	/*!	read the object from a stream(file persistence) RELLEASE save version - for compatibility
	@param stm the stream class that store the bitstream
	@param pStream script wrapper for the stream(optional)
	@return true if succeded,false failed
	@see CStream
	*/
	virtual bool LoadRELEASE(CStream &stm,IScriptObject *pStream=NULL) = 0;

	/*!	read the object from a stream(file persistence) PATCH1 save version - for compatibility
	@param stm the stream class that store the bitstream
	@param pStream script wrapper for the stream(optional)
	@return true if succeded,false failed
	@see CStream
	*/
	virtual bool LoadPATCH1(CStream &stm,IScriptObject *pStream=NULL) = 0;

/*! Get the entity container. The concept of the container is explained in IEntityContainer.
	@return Pointer to the entity container if there is one, otherwise NULL
	@see IEntityContainer
*/
	virtual IEntityContainer* GetContainer() const = 0;
	virtual void SetContainer(IEntityContainer* pContainer) = 0;
/*! Get Character interface, if a character is loaded.
	@return Pointer to the character interface if one exists, otherwise NULL
	@see IEntityCharacter
*/
	virtual IEntityCharacter* GetCharInterface() const = 0;

/*! Start an animation of a character animated object in a specified slot.
	@param pos	The slot in which the character containing this animation is loaded.
	@param animname	The name of the animation that we want to start.
	@return True if animation started, otherwise false.
*/
	virtual bool StartAnimation(int pos, const char *animname,int iLayerID =0, float fBlendTime=1.5f, bool bStartWithLayer0Phase = false ) = 0;

  //! Set animations speed scale
	//! This is the scale factor that affects the animation speed of the character.
	//! All the animations are played with the constant real-time speed multiplied by this factor.
	//! So, 0 means still animations (stuck at some frame), 1 - normal, 2 - twice as fast, 0.5 - twice slower than normal.
	virtual void SetAnimationSpeed( const float scale=1.0f ) = 0;

	//! Returns the current animation in the layer or -1 if no animation is being played
	//! in this layer (or if there's no such layer)
  virtual int GetCurrentAnimation(int pos, int iLayerID) = 0;


	//! Return the length of the given animation in seconds; 0 if no such animation found
	virtual float GetAnimationLength( const char *animation ) = 0;

/*! check if there is animation
	@param pos	The slot in which the character containing this animation is loaded.
	@param animname	The name of the animation that we want to start.
	@return True if animation present, otherwise false.
*/
	virtual bool IsAnimationPresent( int pos,const char *animation ) = 0;

/*! Reset an animation of a character animated object in a specified slot.
	@param pos	The slot in which the character containing this animation is loaded.
*/
	virtual void ResetAnimations(int pos) = 0;

// sets the given aniimation to the given layer as the default
	virtual void SetDefaultIdleAnimation( int pos,const char * szAnimation=NULL ) =0;

/*force update character in slot pos
	This is required if you start some animations abruptly (i.e. without blendin time)
	after the character update has passed, and need the results to be on this frame,
	rather than on the next.
*/
	virtual void ForceCharacterUpdate( int pos ) = 0;

/*! Registers this entity in the ai system so that agents can see it and interact with it
*/
	virtual bool RegisterInAISystem(unsigned short type, const AIObjectParameters &params) = 0;

/*! Gets AI representation
*/
	virtual IAIObject *GetAI() =0;

/*! Enables or disables AI presence of this entity
*/
	virtual void EnableAI(bool enable) = 0;

/*! Enable disable the save of the entity when the level is saved on disk
*/
	virtual void EnableSave(bool bEnable) = 0;
/*! return true if the entity must be saved on disk
*/
	virtual bool IsSaveEnabled() = 0;

//! Retrieves the Trackable-flag
	virtual bool IsTrackable() = 0;

	//////////////////////////////////////////////////////////////////////////
	// State Managment public interface.
	//////////////////////////////////////////////////////////////////////////

/*!
	Change entity state to specified.
	@param sState Name of state table within entity script (case sensetive).
	@return true if state succesfully changed or false if state is unknown.
*/
	virtual bool GotoState( const char *sState ) = 0;
	virtual bool GotoState( int nState ) = 0;
/*!
	Check if entity is in specified state.
	@param sState Name of state table within entity script (case sensetive).
	@return true if entity is currently in this state.
*/
	virtual bool IsInState( const char *sState ) = 0;
	virtual bool IsInState( int nState ) = 0;
/*!
	Get name of currently active entity state.
	@return State's name.
*/
	virtual const char* GetState() = 0;
	virtual int GetStateIdx() = 0;
/*!
	Register a state for this entity
	@return State's name.
*/
	virtual void RegisterState(const char *sState) = 0;

	//! \return true=prevents error when state changes on the client and does not sync state changes to the client, false otherwise
	virtual bool IsStateClientside() const=0;
	//! \param bEnable true=prevents error when state changes on the client and does not sync state changes to the client, false otherwise
	virtual void SetStateClientside( const bool bEnable )=0;

	//! Calls script OnTimer callback in current state.
	//! @param nTimerId Id of timer set with SetTimer.
	virtual void OnTimer( int nTimerId ) = 0;
	//! Calls script OnAction callback in current state.
	//! @nKey code of action.
	//virtual void OnAction( int nAction ) = 0;
	//! Calls script OnDamage callback in current state.
	//! @pObj structure to describe the hit. - script object, to be passed to script OnDamage
		virtual void OnDamage( IScriptObject *pObj ) = 0;
	//! Calls script OnEnterArea callback in current state. Called when the entity enters area
	//! @entity wich enters the area
	//! @areaID id of the area the entity is in
	virtual void OnEnterArea( IEntity* entity, const int areaID )=0;

	//! Physics	callback called when an entity overlaps with this one
	//! @IEntity pOther entity which enters in contact with this one
	virtual void OnPhysicsBBoxOverlap(IEntity *pOther)=0;

	//! Physics	callback called when an entity physics change its simulation class (Awakes or goes to sleep)
	virtual void OnPhysicsStateChange( int nNewSymClass,int nPrevSymClass )=0;

	//! Assign recorded physical state to the entity.
	virtual void SetPhysicsState( const char *sPhysicsState ) = 0;;

	//! Calls script OnLeaveArea callback in current state. Called when the entity leavs area
	//! @entity wich leaves the area
	//! @areaID id of the area
	virtual void OnLeaveArea( IEntity* entity, const int areaID )=0;

	//! Calls script OnProceedFadeArea callback in current state. Called every frame when the entity is in area
	//! @entity wich leaves the area
	//! @areaID id of the area
	//! @fadeCoeff	[0, 1] coefficient
	virtual void OnProceedFadeArea( IEntity* entity, const int areaID, const float fadeCoeff )=0;

	//! Calls script OnBind callback in current state. Called on client only
	//! @entity wich was bound to this on server
	//! @par
	virtual void OnBind( IEntity* entity, const char par )=0;

	//! Calls script OnUnBind callback in current state. Called on client only
	//! @entity wich was unbound to this on server
	//! @par
	virtual void OnUnBind( IEntity* entity, const char par )=0;


	virtual void SetTimer(int msec) = 0;
	virtual void KillTimer() = 0;
	//! Sets rate of calls to script update function.
	virtual void SetScriptUpdateRate( float fUpdateEveryNSeconds ) = 0;

	virtual void ApplyForceToEnvironment(const float radius, const float force) = 0;

	virtual int GetSide(const Vec3& direction) = 0;

	//! Hide entity, making it invisible and not collidable. (Used by Editor).
	virtual void Hide(bool b) = 0;
	//! Check if entity is hidden. (Used by Editor mostly).
	virtual bool IsHidden() const = 0;

	/** Used by editor, if entity will be marked as garbage do not destroy it.
	*/
	virtual void SetDestroyable(bool b) = 0;
	virtual bool IsDestroyable() const = 0;
	virtual void SetGarbageFlag( bool bGarbage ) = 0;
	//////////////////////////////////////////////////////////////////////////

	virtual bool WasVisible() = 0;
	//virtual bool IsVisible() = 0;

	//! check wheteher this entity has changed
	virtual bool HasChanged() = 0;

	//! get the slot containing the steering wheel
	//! FIXME: This better be moved in the game code, together with the
	//! loadvehicle function (-1 if not found)
	virtual int	GetSteeringWheelSlot()=0;

/*! Get bone hit zone type (head, torso, arm,...) for boneIdx bone
*/
	virtual int		GetBoneHitZone( int boneIdx ) const = 0;
/*	hit parameters
		set by OnDamage()
		used by CPlayer::StartDeathAnimation()
*/
	virtual void GetHitParms( int &deathType, int &deathDir, int &deathZone ) const = 0;

  //! access to the entity rendering info/state (used by 3dengine)
//  virtual EntityRenderState & GetEntityRenderState() = 0;

	//! inits entity rendering properties ( tmp silution for particle spray entity )
	virtual void InitEntityRenderState() = 0;

	//!
	virtual void ActivatePhysics( bool activate ) = 0;

	/* it sets common callback functions for those entities
		 without a script, by calling a common script function
		 for materials / sounds etc.
		 Client side only.
	@param pScriptSystem pointer to script system
	*/
	virtual void SetCommonCallbacks(IScriptSystem *pScriptSystem)=0;

	//! Create particle emitter at specified slot and with specified params
	virtual int	CreateEntityParticleEmitter(int nSlotId, const ParticleParams & PartParams, float fSpawnPeriod,Vec3 vOffSet,Vec3 vDir,IParticleEffect *pEffect = 0,float fSize=1.0f ) = 0;
	//! Delete particle emitter at specified slot
	virtual void DeleteParticleEmitter(int nId) = 0;

	virtual void GetMemoryStatistics(ICrySizer *pSizer) = 0;

	//! gets/set water density for this object
	virtual void	SetWaterDensity(float fWaterDensity)=0;
	virtual float	GetWaterDensity()=0;

  //! set update type
  virtual void SetUpdateVisLevel(EEntityUpdateVisLevel nUpdateVisLevel) = 0;

  //! get update type
  virtual EEntityUpdateVisLevel GetUpdateVisLevel() = 0;


/*! Sets position for IK target for hands
	@param target	pointer to vec pos. If NULL - no hands IK
*/
	virtual void SetHandsIKTarget( const Vec3* target=NULL ) = 0;

	virtual void Remove() =0;

	//! Set custom shader parameters
	virtual void SetShaderFloat(const char *Name, float Val) = 0;

	//! enable/disable entity objects light sources
	virtual void	SwitchLights( bool bLights ) = 0;

	//
	//needed for MP - when client connects after some entity was bound (player to vehicle) - connected clent does not get OnBing
	//so this will send binding to the client
	virtual void	SinkRebind(IEntitySystemSink *pSink) = 0;

};

enum ContainerInterfaceType
{
	CIT_IPLAYER,
	CIT_IWEAPON,
	CIT_IVEHICLE,
	CIT_ICOMMANDER,
	CIT_ISPECTATOR,
	CIT_IADVCAMSYSTEM
};

//! Just an interface that Aggregate object who wants to aggregate entity must implement.
/*!
	This interface is used to extend the functionality of an entity, trough aggregation.
	Different entity types have containers - players,
	weapons,vehicles and flocks(birds,fishes etc.) . The entity will call
	its container (if one exists) on major events like Init,Update and will enable the container to
	perform certain specialization for the entity. Most of the entities will
	not have a container, since their specialization will be done over
	the script that is associated with them. So, classes that implement this
	interface actually "contain" an entity within them, and therefore are EntityContainers.

*/
struct IEntityContainer
{

/*! Container Init Callback. This function is called from the init of the entity contained within this container.
	@return True if initialization completed with no fatal errors, false otherwise.
*/
	virtual bool Init() = 0;

/*! Container update callback. This function is called from the update of the entity contained within this container.
*/
	virtual void Update() = 0;

/*! Sets the entity contained in this container.
	@param entity	The desired entity.
	@see IEntity
*/
	virtual void SetEntity( IEntity* entity ) = 0;

/*! called by the entity when the position is changed
	@param new postition of the entity.
	@see IEntity
*/
	virtual void OnSetAngles( const Vec3 &ang ) = 0;

/*! Get the interface which describes this container in script. Containers are not mapped to separate tables in script, but rather to a
  table which is a member of the sctipt table of the entity contained within the container. So this script object should not be used as
	a first parameter in a script call.
	@return The interface to the script object
	@see IScriptSystem
	@see IScriptSystem::BeginCall(const char*, const char *)
*/
	virtual bool Write(CStream &stm,EntityCloneState *cs=NULL) = 0;
	virtual bool Read(CStream &stm) = 0;

/*! Position for sound-sources attached to this entity object.
*/
	virtual Vec3 CalcSoundPos() = 0;

	virtual IScriptObject *GetScriptObject() = 0;
/*! Set the object that describes this container in script.
	@param object The desired script object.
*/
	virtual void SetScriptObject(IScriptObject *object) = 0;

	virtual void Release()=0;

	virtual bool QueryContainerInterface( ContainerInterfaceType desired_interface, void **pInterface) = 0;
/*!	Add description of container in a CEntityDesc structure. This describes the entity class, entity id and other parameters.
	@param desc This parameter will contain the filled in CEntityDesc structure.
	@see CEntityDesc
*/
	virtual void GetEntityDesc( CEntityDesc &desc ) const = 0;

	virtual void OnDraw(const SRendParams & EntDrawParams) = 0;

	//! start preloading render resoures
	virtual void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime) = 0;

	//! tells if the container should be serialized or not (checkpoints, savegame)
	virtual bool IsSaveable() = 0;

	//! return maximum radius of lightsources in container, used for vehicle now,
	//	todo: find better solution later like store lsourses in same place as other entity components - in entity
	virtual float GetLightRadius() { return 0; }

	//! called before the entity is synched over network - to calculate priority or neccessarity
	//! \param pXServerSlot must not be 0
	//! \param inoutPriority 0 means no update at all
	virtual void OnEntityNetworkUpdate( const EntityId &idViewerEntity, const Vec3d &v3dViewer, uint32 &inoutPriority,
		EntityCloneState &inoutCloneState ) const=0;
};

//////////////////////////////////////////////////////////////////////
/*! Interface for accessing character related entity functions.
 */
struct IEntityCharacter
{
/*! Load new character model at a specific slot in the entity.
	@param pos The number of the slot in which this character will be loaded
	@param name The filename of the character cid file
	@return True on successful load, false otherwise.
*/
	virtual bool LoadCharacter( int pos,const char *fileName ) = 0;

/*! Physicalize existing character model. The model has to be previously loaded in the appropriate slot.
	@param pos Number of the slot where character model is loaded
	@param mass Mass of character
	@param surface_idx	Surface identifier for the player.
	@param bInstant - true if character is to be physicalized instantly and permanenty, otherwise it will be physicalized on-demand
	@return True if successfully physicalized, otherwise false.
*/
	virtual bool PhysicalizeCharacter( int pos,float mass,int surface_idx,float stiffness_scale=1.0f,bool bInstant=false ) = 0;

/*! Retrieves character physics from animation system and assigns it as main physical entity
*/
	virtual void KillCharacter( int pos ) = 0;

/*! Assign character model to a slot.
	@param pos	Number of slot where this character needs to be put.
	@param character	Character object
	@see ICryCharInstance
*/
	virtual void SetCharacter( int pos,ICryCharInstance *character ) = 0;

/*! Retrieves  character model from specified slot.
	@param pos Number of slot from which to retrieve character.
	@return Character object interface
	@see ICryCharInstance
*/
	virtual ICryCharInstance* GetCharacter( int pos ) = 0;


/*! Enable/Disable drawing of character at a certain slot
	@param pos Number of slot which contains the character whose parameter is to be changed
	@param mode Desired drawing mode. This mode can be one of the following values: ETY_DRAW_NORMAL, ETY_DRAW_NEAR, ETY_DRAW_NONE.
*/
	virtual void DrawCharacter( int pos,int mode ) = 0;
/*! Reset all animations of the character at a specified slot.
	@param pos Number of slot which contains the character whose animation is to be reset
*/
	virtual void ResetAnimations( int pos ) = 0;
/*! Retrieves a lip-sync interface :)
*/
	virtual ILipSync* GetLipSyncInterface() = 0;
/*! Release the lip-sync interface and deallocate all resources.
*/
	virtual void ReleaseLipSyncInterface() = 0;
};

/*! A callback interface for a class that wants to be aware when new entities are being spawned or removed. A class that implements
	this interface will be called everytime a new entity is spawned, removed, or when an entity container is to be spawned.
*/
struct IEntitySystemSink
{
/*! This callback enables the class which implements this interface a way to spawn containers for the entity that is just in the
	process of spawning. Every entity class that has a container creates it here. NOTE: Although the container is being created here,
	it is not initialized yet (this will be done when the entity being spawned initializes itself).
	@param ed	Entity description of entity which would be contained in this container
	@param pEntity The entity that will hold this container
*/
	virtual void OnSpawnContainer( CEntityDesc &ed,IEntity *pEntity) = 0;

/*! This callback is called when this entity has finished spawning. The entity has been created and added to the list of entities,
 but has not been initialized yet.
	@param e The entity that was just spawned
*/
	virtual void OnSpawn( IEntity *e, CEntityDesc &ed  ) = 0;

/*! Called when an entity is being removed.
	@param e The entity that is being removed. This entity is still fully valid.
*/
	virtual void OnRemove( IEntity *e ) = 0;

	virtual void OnBind(EntityId id,EntityId child,unsigned char param)=0;

	virtual void OnUnbind(EntityId id,EntityId child,unsigned char param)=0;

};


/*! Interface to the system that manages the entities in the game, their creation, deletion and upkeep. The entities are kept in a map
 indexed by their uniqie entity ID. The entity system updates only unbound entities every frame (bound entities are updated by their
 parent entities), and deletes the entities marked as garbage every frame before the update. The entity system also keeps track of entities
 that have to be drawn last and with more zbuffer resolution.
*/
struct IEntitySystem
{

/*! Update entity system and all entities. This function executes once a frame.
*/
	virtual	void	Update() = 0;

/*! Retrieves the script system interface.
	@return Script System Interface
	@see IScriptSystem
*/
	virtual IScriptSystem * GetScriptSystem() = 0;

/*! Reset whole entity system, and destroy all entities.
*/
	virtual void	Reset() = 0;

/*! Spawns a new entity according to the data in the Entity Descriptor
	@param ed	Entity descriptor structure that describes what kind of entity needs to be spawned
	@param bAutoInit If true automatically initialize entity.
	@return The spawned entity if successfull, NULL if not.
	@see CEntityDesc
*/
	virtual IEntity* SpawnEntity( CEntityDesc &ed,bool bAutoInit=true ) = 0;

/*! Initialize entity if entity was spawned not initialized (with bAutoInit false in SpawnEntity)
		Used only by Editor, to setup properties & other things before initializing entity,
		do not use this directly.
		@param pEntity Pointer to just spawned entity object.
		@param ed	Entity descriptor structure that describes what kind of entity needs to be spawned.
		@return true if succesfully initialized entity.
*/
	virtual bool InitEntity( IEntity* pEntity,CEntityDesc &ed ) = 0;

/*! Retrieves entity from its unique id.
	@param id The unique ID of the entity required
	@return The entity if one with such an ID exists, and NULL if no entity could be matched with the id
*/
  virtual IEntity* GetEntity( EntityId id )  = 0;

/*! Set an entity as the player associated with this client, identified with the entity id
	@param id Unique Id of the entity which is to represent the player
*/
	//virtual	void SetMyPlayer( EntityId id ) = 0;

/*! Get the entity id of the entity that represents the player.
	@return The entity id as an unsigned short*/

	//virtual EntityId GetMyPlayer() const = 0;

/*! Find first entity with given name.
	@param name The name to look for
	@return The entity if found, 0 if failed
*/
	virtual IEntity* GetEntity(const char *sEntityName) =0;
//! obsolete
	virtual EntityId FindEntity( const char *name ) const = 0;

/*! Remove an entity by ID
	@param entity	The id of the entity to be removed
	@param bRemoveNode If true forces immidiate delete of entity, overwise will delete entity on next update.
*/
	virtual void	RemoveEntity( EntityId entity,bool bRemoveNow=false ) = 0;

/*! Get number of entities stored in entity system.
	@return The number of entities
*/
	virtual int		GetNumEntities() const = 0;


/*! Get entity iterator. This iterator interface can be used to traverse all the entities in this entity system.
	@return An entityIterator
	@see IEntityIt
*/
	virtual IEntityIt * GetEntityIterator() = 0;

/*! Get entity iterator of all visible entities. This iterator interface can be used to traverse all the visible entities in this entity system.
	bFromPrevFrame	- to get entities visible in previouse update
	@return An entityIterator
	@see IEntityIt
*/
	virtual IEntityIt * GetEntityInFrustrumIterator( bool bFromPrevFrame=false ) = 0;

/*! Get all entities in specified radius.
		 physFlags is one or more of PhysicalEntityFlag.
	 @see PhysicalEntityFlag
*/
#if defined(LINUX)
	#undef vector;//well this was previously defined in physics, so...
#endif
	virtual void	GetEntitiesInRadius( const Vec3 &origin, float radius, std::vector<IEntity*> &entities,int physFlags=PHYS_ENTITY_ALL ) const = 0;

/*! Add the sink of the entity system. The sink is a class which implements IEntitySystemSink.
	@param sink	Pointer to the sink
	@see IEntitySystemSink
*/
	virtual void	SetSink( IEntitySystemSink *sink ) = 0;

	virtual void	PauseTimers(bool bPause,bool bResume=false)=0;

/*! Remove listening sink from the entity system. The sink is a class which implements IEntitySystemSink.
	@param sink	Pointer to the sink
	@see IEntitySystemSink
*/
	virtual void RemoveSink( IEntitySystemSink *sink ) = 0;

/*! Creates an IEntityCamera that can be attached to an entity
	@return Pointer to a new entity camera.
	@see IEntityCamera
*/
	virtual IEntityCamera * CreateEntityCamera() = 0;
/*! Enable/Disable the Client-Side script calls
	@param bEnable true enable false disable
*/
	virtual void EnableClient(bool bEnable)=0;
/*! Enable/Disable the Server-Side script calls
	@param bEnable true enable false disable
*/
	virtual void EnableServer(bool bEnable)=0;

	//virtual IEntityClonesMgr *CreateEntityClonesMgr()=0;
/*! Destroys the entity system
*/
	virtual void Release()=0;

/*! Checks whether a given entity ID is already used
*/
	virtual bool IsIDUsed(EntityId nID)= 0;

//! Calls reset for every entity, to reset its state
	virtual void ResetEntities() = 0;

	//! Puts the memory statistics of the entities into the given sizer object
	//! According to the specifications in interface ICrySizer
	virtual void GetMemoryStatistics(ICrySizer *pSizer) = 0;

	//! if this is set to true (usually non editor mode)
	//! only dynamic EntityID s are created, no longer static ones
	virtual void SetDynamicEntityIdMode( const bool bActivate )=0;

	//! sets the default update level for the entities
	//! every entity spawned after this call will have the specified update level at creation time
	//! the entity can specify it differently later
	virtual void SetDefaultEntityUpdateLevel( EEntityUpdateVisLevel eDefault)=0;

	//! Set entity system mode into precaching of resources.
	virtual void SetPrecacheResourcesMode( bool bPrecaching ) = 0;

//	virtual CIDGenerator* GetIDGenerator() = 0;
	virtual bool	IsDynamicEntityId( EntityId id ) = 0;
	virtual void	MarkId( EntityId id ) = 0;
	virtual void	ClearId( EntityId id ) = 0;
};

/*struct IEntityClonesMgr
{
	virtual void Update(Vec3 v3Viever) = 0;
	virtual bool WriteEntity(CStream &stm) = 0;
	virtual void Release() = 0;
};*/

enum ThirdPersonMode {
	CAMERA_3DPERSON1,
	CAMERA_3DPERSON2
};

/*! Various camera parameters.
*/
struct EntityCameraParam
{
	float m_cam_dist;
	Vec3 m_cam_dir, m_1pcam_butt_pos,m_1pcam_eye_pos;
	float m_cam_kstiffness,m_cam_kdamping;
	float m_cam_angle_kstiffness,m_cam_angle_kdamping;
	float m_1pcam_kstiffness,m_1pcam_kdamping;
	float m_1pcam_angle_kstiffness,m_1pcam_angle_kdamping;
	float m_cur_cam_dist, m_cur_cam_dangle;
	float m_cur_cam_vel, m_cur_cam_dangle_vel;
	int   m_cam_angle_flags;
	Vec3 m_cur_cam_rotax;
	Vec3 m_camoffset;
	float m_viewoffset;
};

struct IEntityCamera
{
	virtual void Release() = 0;
	virtual void SetPos( const Vec3 &p ) = 0;
	virtual Vec3 GetPos() const = 0;
	virtual void SetAngles( const Vec3 &p ) = 0;
	virtual Vec3 GetAngles() const = 0;
	virtual void SetFov( const float &f, const unsigned int iWidth, const unsigned int iHeight ) = 0;
	virtual float GetFov() const = 0;
//	virtual void SetMatrix( const Matrix44& m ) = 0;
	virtual Matrix44 GetMatrix() const = 0;
	virtual void Update() = 0;
	virtual CCamera& GetCamera() = 0;
	virtual void SetCamera( const CCamera &cam ) = 0;
	virtual void SetParameters(const EntityCameraParam *pParam) = 0;
	virtual void GetParameters(EntityCameraParam *pParam) = 0;
	virtual void SetViewOffset(float f) = 0;
	virtual float GetViewOffset() = 0;
	virtual void SetCamOffset(Vec3 v) = 0;
	virtual Vec3& GetCamOffset() = 0;
	virtual void SetThirdPersonMode( const Vec3 &pos,const Vec3 &angles,int mode,float frameTime,float
		range,int dangleAmmount,IPhysicalEntity *physic, IPhysicalEntity *physicMore=NULL, I3DEngine* p3DEngine=NULL, float safe_range=0.0f) = 0;
	virtual void SetCameraMode(const Vec3 &lookat,const Vec3 &lookat_angles, IPhysicalEntity *physic)=0;
	virtual void SetCameraOffset(const Vec3 &offset) = 0;
	virtual void GetCameraOffset(Vec3 &offset) = 0;
};

extern "C"
{
	CRYENTITYDLL_API struct IEntitySystem * CreateEntitySystem(ISystem *pISystem);
}

typedef struct IEntitySystem * (* PFNCREATEENTITYSYSTEM) (ISystem *pISystem);

//////////////////////////////////////////////////////////////////////////

//#define GERMAN_GORE_CHECK

#ifdef GERMAN_GORE_CHECK

extern "C"
{
	CRYENTITYDLL_API struct IEntitySystem * CreateMainEntitySystem(ISystem *pISystem);
}

typedef struct IEntitySystem * (* PFNCREATEMAINENTITYSYSTEM) (ISystem *pISystem);

#endif



//DOC-IGNORE-BEGIN
#include "smartptr.h"
typedef _smart_ptr<IEntityIt> IEntityItPtr;
//DOC-IGNORE-END

#endif // __ENTITY_SYSTEM_INTERFACES_H__