////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   flock.h
//  Version:     v1.00
//  Created:     5/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __flock_h__
#define __flock_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "GameObject.h"

class CFlock;

enum EFlockType
{
	EFLOCK_BIRDS,
	EFLOCK_FISH,
	EFLOCK_BUGS,
};

struct SBoidContext
{
	Vec3 playerPos;
	Vec3 flockPos;

	//! Some integer for various behaviors.
	int behavior;

	float fSpawnRadius;
	float fBoidRadius;

	float fBoidMass;
	float fGravity;

	float terrainZ;
	float waterLevel;

	// Flock constants.
	float MinHeight;
	float MaxHeight;

	// Attraction distances.
	float MaxAttractDistance;
	float MinAttractDistance;

	// Speed.
	float MaxSpeed;
	float MinSpeed;

	// AI factors.
	// Group behavior factors.
	float factorAlignment;
	float factorCohesion;
	float factorSeparation;
	// Other behavior factors.
	float factorAttractToOrigin;
	float factorKeepHeight;
	float factorAvoidLand;
	//! Cosine of boid field of view angle.
	//! Other boids which are not in field of view of this boid not considered as a neightboards.
	float cosFovAngle;

	//! Maximal Character animation speed.
	float MaxAnimationSpeed;

	// Settings.
	bool followPlayer;
	bool avoidObstacles;
	bool noLanding;

	//! Max visible distane of flock from player.
	float maxVisibleDistance;

	//! Size of boid.
	float boidScale;

	I3DEngine *engine;
	IPhysicalWorld *physics;
	IEntity* entity;
};

/*!	This is flock creation context passed to flock Init method.
*/
struct SBoidsCreateContext
{
	int boidsCount;						//! Number of boids in flock.
	std::vector<string> models;	//! Geometry models (Static or character) to be used for flock.
	string characterModel;	//! Character model.
	string animation;	//! Looped character animation.
};

//////////////////////////////////////////////////////////////////////////
// BoidObject.
//////////////////////////////////////////////////////////////////////////
/*! Single Boid object.
 */
class CBoidObject
{
public:
	CBoidObject( SBoidContext &bc );
	virtual ~CBoidObject();

	virtual void Update( float dt,SBoidContext &bc ) {};
	virtual void Physicalize( SBoidContext &bc );

	//! Kill this boid object.
	//! @param force Force vector applyed on dying boid (shot vector).
	virtual void Kill( const Vec3 &hitPoint,const Vec3 &force,string &surfaceName ) {};

	virtual void OnFlockMove( SBoidContext &bc ) {};

	virtual void Render( SRendParams &rp,CCamera &cam,SBoidContext &bc );
	void CalcFlockBehavior( SBoidContext &bc,Vec3 &vAlignment,Vec3 &vCohesion,Vec3 &vSeparation );
	void CalcMovement( float dt,SBoidContext &bc,bool banking );

	void CalcMatrix( Matrix44 &mtx );
	void CreateRigidBox( SBoidContext &bc,const Vec3 &size,float density );
	void CreateArticulatedCharacter( SBoidContext &bc,const Vec3 &size,float density );

public:
	//////////////////////////////////////////////////////////////////////////
	friend class CFlock;
	IPhysicalEntity	 *m_pPhysics;
	CFlock *m_flock;	//!< Flock of this boid.
	Vec3 m_pos;			//!< Boid position.
	Vec3 m_heading;	//!< Current heading direction.
	Vec3 m_accel;		//!< Current acceleration vector.
	float m_speed;		//!< Speed of bird at heading direction.
	float m_banking;	//!< Amount of banking to apply on boid.
	ICryCharInstance *m_object;	//< Geometry of this boid.
	float m_alignHorizontally; // 0-1 to align bird horizontally when it lands.

	// Flags.
	unsigned m_dead : 1;			//! Boid is dead, do not update it.
	unsigned m_dying : 1;			//! Boid is dying.
	unsigned m_physicsControlled : 1;	//! Boid is controlled by physics.
	unsigned m_inwater : 1;		//! When boid falls in water.
	unsigned m_nodraw : 1;		//! Do not draw this boid.
};

//////////////////////////////////////////////////////////////////////////
class CBoidBird : public CBoidObject
{
public:
	CBoidBird( SBoidContext &bc );
	virtual ~CBoidBird();

	virtual void Update( float dt,SBoidContext &bc );
	virtual void Kill( const Vec3 &hitPoint,const Vec3 &force,string &surfaceName );
	virtual void OnFlockMove( SBoidContext &bc );

	//void Render( CCamera &cam,SBoidContext &bc );
	void Think( SBoidContext &bc );

	void TakeOff( SBoidContext &bc );

private:
	float m_flightTime;	//!< Time flying after take off.
	float m_lastThinkTime; //! Time of last think operation.
	float m_maxFlightTime; // Time this bird can be in flight.
	float m_desiredHeigh; // Deisred height this birds want to fly at.
	Vec3 m_birdOriginPos;

	// Flags.
	unsigned m_onGround : 1;	//! True if stand on ground.
	unsigned m_landing : 1;		//! True if bird wants to land.
	unsigned m_takingoff : 1;	//! True if bird is just take-off from land.
};

//////////////////////////////////////////////////////////////////////////
//! Boid object with fish behavior.
//////////////////////////////////////////////////////////////////////////
class CBoidFish : public CBoidObject
{
public:
	CBoidFish( SBoidContext &bc );
	~CBoidFish();

	virtual void Update( float dt,SBoidContext &bc );
	virtual void Kill( const Vec3 &hitPoint,const Vec3 &force,string &surfaceName );

private:
	void SpawnBubble( const Vec3 &pos,SBoidContext &bc );

	float m_dyingTime; // Deisred height this birds want to fly at.
	CScriptObjectVector vec_Bubble;
	HSCRIPTFUNCTION m_pOnSpawnBubbleFunc;
};

//! Structure passed to CFlock::RayTest method, filled with intersection parameters.
struct SFlockHit {
	//! Hit object.
	CBoidObject *object;
	//! Distance from ray origin to the hit distance.
	float dist;
};

//////////////////////////////////////////////////////////////////////////
class CFlockManager;
/*!
 *	Define flock of boids, where every boid share common properties and recognize each other.
 */
class CFlock : public CGameObject//IEntityContainer
{
public:
	CFlock( int id,CFlockManager *mgr );
	virtual ~CFlock();

	//! Create boids in flock.
	//! Must be overriden in derived specialized flocks.
	virtual void CreateBoids( SBoidsCreateContext &ctx ) {};

	void SetName( const char *name );
	const char* GetName() const { return m_name; };

	int GetId() const { return m_id; };
	EFlockType GetType() const { return m_type; };

	void SetPos( const Vec3& pos );
	Vec3 GetPos() const { return m_origin; };

	void AddBoid( CBoidObject *boid );
	int GetBoidsCount() { return m_boids.size(); }
	CBoidObject* GetBoid( int index ) { return m_boids[index]; }

	float GetMaxVisibilityDistance() const { return m_bc.maxVisibleDistance; };

	//! Retrieve general boids settings in this flock.
	void GetBoidSettings( SBoidContext &bc ) { bc = m_bc; };
	//! Set general boids settings in this flock.
	void SetBoidSettings( SBoidContext &bc );

	bool IsFollowPlayer() const { return m_bc.followPlayer; };

	void ClearBoids();

	//! Check ray to flock intersection.
	bool RayTest( Vec3 &raySrc,Vec3 &rayTrg,SFlockHit &hit );

	const char* GetModelName() const { return m_model; };

	//! Static function that initialize defaults of boids info.
	static void GetDefaultBoidsContext( SBoidContext &bc );

	//! Enable/Disable Flock to be updated and rendered.
	void SetEnabled( bool bEnabled );
	//! True if this flock is enabled, and must be updated and rendered.
	bool IsEnabled() const { return m_bEnabled; }

	//! Set how much percent of flock is visible.
	//! value 0 - 100.
	void SetPercentEnabled( int percent );

	//! See if this flock must be active now.
	bool IsFlockActive() const;

	//! flock's container should not be saved
	bool IsSaveable() { return(false); }

	//! Get entity owning this flock.
	IEntity* GetEntity() const { return m_pEntity; }

	//////////////////////////////////////////////////////////////////////////
	// IEntityContainer implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual bool Init() { return true; };
	virtual void Update();
	virtual void SetEntity( IEntity* entity );
	virtual void OnSetAngles( const Vec3 &ang ) {};
	virtual bool Write(CStream &stm,EntityCloneState *cs=NULL) { return true; };
	virtual bool Read(CStream &stm) { return true; };
	virtual IScriptObject *GetScriptObject() { return 0; };
	virtual void SetScriptObject(IScriptObject *object) {};
	virtual void Release() {};
	virtual bool QueryContainerInterface( ContainerInterfaceType desired_interface, void **pInterface) { return false; };
	virtual void GetEntityDesc( CEntityDesc &desc ) const {};
	virtual void OnDraw(const SRendParams & EntDrawParams);
	virtual void PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime);
	//////////////////////////////////////////////////////////////////////////
	

public:
	friend class CFlockManager;
	//! Manager that owns flock.
	CFlockManager *m_flockMgr;

protected:
	typedef std::vector<CBoidObject*> Boids;
	Boids m_boids;
	Vec3 m_origin;

	// Bonding box.
	AABB m_bounds;

	//! All boid parameters.
	SBoidContext m_bc;

	//! Uniq id of this flock, assigned by flock manager at creation.
	int m_id;

	//! Name of this flock.
	EFlockType m_type;
	
	char m_name[64];
	char m_model[64];

	// Pointer to entity who created this flock.
	IEntity* m_pEntity;

	bool m_bEnabled;
	int m_updateFrameID;
	int m_percentEnabled;
};

//////////////////////////////////////////////////////////////////////////
// Sepcialized flocks for birds and fish.
//////////////////////////////////////////////////////////////////////////
class CBirdsFlock : public CFlock
{
public:
	CBirdsFlock( int id,CFlockManager *mgr ) : CFlock( id,mgr ) {};
	virtual void CreateBoids( SBoidsCreateContext &ctx );
};

//////////////////////////////////////////////////////////////////////////
class CFishFlock : public CFlock
{
public:
	CFishFlock( int id,CFlockManager *mgr ) : CFlock( id,mgr ) {};
	virtual void CreateBoids( SBoidsCreateContext &ctx );
};


//////////////////////////////////////////////////////////////////////////
class CFlockManager
{
public:
	CFlockManager( ISystem *system );
	~CFlockManager();

	CFlock* CreateFlock( EFlockType type );
	void RemoveFlock( CFlock *flock );

	CFlock* GetFlock( int id );
	CFlock* FindFlock( const char *sFlockName );

	//! Check ray to flock intersection.
	//! @param raySrc Source point of ray.
	//! @param rayTrg Target point of ray.
	//! @param hit Output structure filled if ray intersected any boid.
	//! @param onlyVisible If true ray hit will only consider currently visible flocks.
	//! @return true if any boid object was hit, overwise false.
	bool RayTest( Vec3 &raySrc,Vec3 &rayTrg,SFlockHit &hit,bool onlyVisible=true );

	ISystem *GetSystem() { return m_system; };

	void Update( float dt,Vec3 &playerPos );
	void Render();
	void ClearFlocks();

	Vec3 GetPlayerPos() const { return m_playerPos; }

	bool IsFlockVisible( CFlock *flock );
	bool IsFlocksEnabled() const { return m_e_flocks != 0; };
public:
	typedef std::vector<CFlock*> Flocks;
	Flocks m_flocks;
	ISystem *m_system;

	IStatObj *m_object;

	Vec3 m_playerPos;
	int m_lastFlockId;

	static int m_e_flocks;
	static int m_e_flocks_hunt; // Hunting mode...
};


#endif // __flock_h__
