//////////////////////////////////////////////////////////////////////
//
//	Physics System Interface
//
//	File: physinterface.h
//	Description : declarations of all physics interfaces and structures
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef physinterface_h
#define physinterface_h

#if defined(LINUX)
	#include "Stream.h"
	#include "validator.h"
#endif


enum pe_type { PE_NONE=0, PE_STATIC=1, PE_LIVING=2, PE_RIGID=3, PE_WHEELEDVEHICLE=4, PE_PARTICLE=5, PE_ARTICULATED=6, PE_ROPE=7, PE_SOFT=8 };
enum sim_class { SC_STATIC=0, SC_SLEEPING_RIGID=1, SC_ACTIVE_RIGID=2, SC_LIVING=3, SC_INDEPENDENT=4, SC_TRIGGER=6, SC_DELETED=7 };
class IGeometry;
class IPhysicalEntity;
class IGeomManager;
class IPhysicalWorld;
class ICrySizer;
struct ILog;
IPhysicalEntity *const WORLD_ENTITY = (IPhysicalEntity*)-10;


/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// IPhysicsStreamer Interface /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class IPhysicsStreamer {
public:
	virtual int CreatePhysicalEntity(void *pForeignData,int iForeignData,int iForeignFlags) = 0;
	virtual int DestroyPhysicalEntity(IPhysicalEntity *pent) = 0;
	virtual const char *GetForeignName(void *pForeignData,int iForeignData,int iForeignFlags) = 0;
};


class CMemStream { // for "fastload" serialization; hopefully it can be made global for the project
public:
	CMemStream() {
		m_pBuf = new char[m_nSize=0x1000]; m_iPos=0; bDeleteBuf=true;
	}
	CMemStream(void *pbuf,int sz) {
		m_pBuf=(char*)pbuf; m_nSize=sz; m_iPos=0; bDeleteBuf=false;
	}
	~CMemStream() {
		if (bDeleteBuf)
			delete[] m_pBuf;
	}

	void *GetBuf() { return m_pBuf; }
	int GetUsedSize() { return m_iPos; }
	int GetAllocatedSize() { return m_nSize; }

	template<class ftype> void Write(const ftype &op) { Write(&op, sizeof(op)); }
	void Write(const void *pbuf, int sz) {
		if (m_iPos+sz>m_nSize)
			GrowBuf(sz);
		if (sz==4)
			*(int*)m_pBuf = *(int*)pbuf;
		else
			memcpy(m_pBuf+m_iPos,pbuf,(unsigned int)sz);
		m_iPos += sz;
	}

	#ifdef _WIN32
	_declspec(noinline) //changed by ivo
	#endif
	void GrowBuf(int sz) {
		int prevsz = m_nSize; char *prevbuf = m_pBuf;
		m_pBuf = new char[m_nSize = (m_iPos+sz-1 & ~0xFFF)+0x1000];
		memcpy(m_pBuf, prevbuf, (unsigned int)prevsz);
	}

	template<class ftype> void Read(ftype &op) { Read(&op, sizeof(op)); }
	void Read(void *pbuf, int sz) {
		if (sz==4)
			*(int*)pbuf = *(int*)m_pBuf;
		else
			memcpy(pbuf,(m_pBuf+m_iPos),(unsigned int)sz);
		m_iPos += sz;
	}

	char *m_pBuf;
	int m_iPos,m_nSize;
	bool bDeleteBuf;
};

// in physics interface [almost] all parameters are passed via structures
// this allows having stable interface methods and flexible default arguments system

////////////////////////// Params structures /////////////////////

////////// common params
struct pe_params {
	int type;
};

struct pe_params_pos : pe_params { //	Sets postion and orientation of entity
	enum entype { type_id=0 };
	pe_params_pos() {
		type=type_id; MARK_UNUSED pos,scale,q,iSimClass; pMtx3x3=0;pMtx3x3T=0;pMtx4x4=0;pMtx4x4T=0; bRecalcBounds=1;
	}

	vectorf pos;
	quaternionf q;
	float scale;
	float *pMtx3x3;	// optional orientation via 3x3 matrix
	float *pMtx3x3T;	// optional orientation via 3x3 column-major matrix
	float *pMtx4x4;	// optional position and orientation via 4x4 matrix
	float *pMtx4x4T;	// optional position and orientation via 4x4 column-major matrix
	int iSimClass;
	int bRecalcBounds;

	VALIDATORS_START
		VALIDATOR(pos)
		VALIDATOR_NORM_MSG(q,"(perhaps non-uniform scaling was used?)",pos)
		VALIDATOR(scale)
	VALIDATORS_END
};

struct pe_params_bbox : pe_params {
	enum entype { type_id=14 };
	pe_params_bbox() { type=type_id; MARK_UNUSED BBox[0],BBox[1]; }
	vectorf BBox[2];

	VALIDATORS_START
		VALIDATOR(BBox[0])
		VALIDATOR(BBox[1])
	VALIDATORS_END
};

// If entity represents an interior volume this allows to set outer entity, which will be skipped during tests against
// objects that are inside this entity
struct pe_params_outer_entity : pe_params {
	enum entype { type_id=9 };
	pe_params_outer_entity() { type=type_id; pOuterEntity=0; pBoundingGeometry=0; }

	IPhysicalEntity *pOuterEntity; //	outer entity for this one (outer entities can form chains)
	IGeometry *pBoundingGeometry;	// optional geometry to test containment
};

struct pe_params_part : pe_params {	// Sets geometrical parameters of entity part
	enum entype { type_id=6 };
	pe_params_part() {
		type=type_id; MARK_UNUSED pos,q,scale,partid,ipart,mass,density,pPhysGeom,pPhysGeomProxy;
		pMtx3x3=0;pMtx3x3T=0;pMtx4x4=0;pMtx4x4T=0;
		bRecalcBBox=1; flagsOR=flagsColliderOR=0; flagsAND=flagsColliderAND=-1;
	}

	int partid;	// partid identifier of part
	int ipart; // optionally, part slot number
	int bRecalcBBox; // whether entity's bounding box should be recalculated
	vectorf pos;
	quaternionf q;
	float scale;
	float *pMtx3x3;	// optional orientation via 3x3 matrix
	float *pMtx3x3T;
	float *pMtx4x4;	// optional position and orientation via 4x4 matrix
	float *pMtx4x4T;
	unsigned int flagsOR,flagsAND; // new flags = (flags & flagsAND) | flagsOR
	unsigned int flagsColliderOR,flagsColliderAND;
	float mass;
	float density;
	struct phys_geometry *pPhysGeom,*pPhysGeomProxy;

	VALIDATORS_START
		VALIDATOR(pos)
		VALIDATOR_NORM(q)
		VALIDATOR(scale)
	VALIDATORS_END
};

struct pe_params_sensors : pe_params { // Attaches optional sensors to entity (sensors raytrace enviroment around entity)
	enum entype { type_id=7 };
	pe_params_sensors() { type=type_id; nSensors=0; pOrigins=0; pDirections=0; }

	int nSensors;	// nSensors number of sensors
	const vectorf *pOrigins; // pOrigins sensors origins in entity CS
	const vectorf *pDirections;	// pDirections sensors directions (dir*ray length) in entity CS
};

struct pe_simulation_params : pe_params { // Sets gravity and maximum time step
	enum entype { type_id=10 };
	pe_simulation_params() { type=type_id; MARK_UNUSED maxTimeStep,gravity,minEnergy,damping,iSimClass,
		softness,softnessAngular,dampingFreefall,gravityFreefall,mass,density; }

	int iSimClass;
	float maxTimeStep; // maximum time step that entity can accept (larger steps will be split)
	float minEnergy; // minimun of kinetic energy below which entity falls asleep (divided by mass!)
	float damping;
	vectorf gravity;
	float dampingFreefall; // damping and gravity used when there are no collisions,
	vectorf gravityFreefall; // NOTE: if left unused, gravity value will be substituted (if provided)
	float mass;
	float density;
	float softness,softnessAngular;
	float softnessGroup,softnessAngularGroup;
};

struct pe_params_foreign_data : pe_params {
	enum entype { type_id=11 };
	pe_params_foreign_data() { type=type_id; MARK_UNUSED pForeignData,iForeignData,iForeignFlags; }

	void *pForeignData;
	int iForeignData;
	int iForeignFlags;
};

struct pe_params_buoyancy : pe_params {
	enum entype { type_id=12 };
	pe_params_buoyancy() {
		type=type_id; MARK_UNUSED waterDensity,waterDamping,waterPlane.n,waterPlane.origin,waterEmin,waterResistance,waterFlow;
	};

	float waterDensity;
	float waterDamping;
	float waterResistance;
	vectorf waterFlow;
	primitives::plane waterPlane;
	float waterEmin;
};

enum phentity_flags {
	particle_single_contact=0x01,particle_constant_orientation=0x02,particle_no_roll=0x04,particle_no_path_alignment=0x08,particle_no_spin=0x10,
	lef_push_objects=0x20, lef_push_players=0x40,	lef_snap_velocities=0x80,	lef_loosen_stuck_checks=0x100,
	pef_pushable_by_players=0x200,	pef_traceable=0x400, particle_traceable=0x400, pef_update=0x800,
	pef_monitor_state_changes=0x1000, pef_monitor_collisions=0x2000, pef_monitor_impulses=0x4000, pef_never_affect_triggers=0x8000,
	pef_checksum_received=0x10000, pef_checksum_outofsync=0x20000, pef_fixed_damping=0x40000,	pef_custom_poststep=0x80000,
	pef_always_notify_on_deletion=0x100000,
	rope_collides=0x200000, rope_traceable=0x400, rope_collides_with_terrain=0x400000,
	se_skip_longest_edges=0x800000,
	ref_use_simple_solver=0x1000000
};

struct pe_params_flags : pe_params {
	enum entype { type_id=15 };
	pe_params_flags() { type=type_id; MARK_UNUSED flags,flagsOR,flagsAND; }
	unsigned int flags;
	unsigned int flagsOR;
	unsigned int flagsAND;
};

////////// articulated entity params
enum joint_flags { angle0_locked=1, all_angles_locked=7, angle0_limit_reached=010, angle0_auto_kd=0100, joint_no_gravity=01000,
									 joint_isolated_accelerations=02000, joint_expand_hinge=04000, angle0_gimbal_locked=010000 };

struct pe_params_joint : pe_params {
	enum entype { type_id=5 };
	pe_params_joint() {
		type=type_id;
		for(int i=0;i<3;i++)
			MARK_UNUSED limits[0][i],limits[1][i],qdashpot[i],kdashpot[i],bounciness[i],q[i],qext[i],ks[i],kd[i];
		bNoUpdate=0; pMtx0=pMtx0T=0;
		MARK_UNUSED flags,q0,pivot,ranimationTimeStep,nSelfCollidingParts;
		animationTimeStep = 0.01f;
	}

	unsigned int flags; // should be a combination of angle0,1,2_locked, angle0,1,2_auto_kd, joint_no_gravity
	vectorf pivot; // joint pivot in entity CS
	quaternionf q0;	// orientation of child in parent coordinates that corresponds to angles (0,0,0)
	float *pMtx0; // same as 3x3 row major matrix
	float *pMtx0T; // same as 3x3 column major matrix
	vectorf limits[2];	// limits for each angle
	vectorf bounciness; // bounciness for each angle (applied when limit is reached)
	vectorf ks,kd; // stiffness and damping koefficients for each angle angular spring
	vectorf qdashpot; // limit vicinity where joints starts resisting movement
	vectorf kdashpot; // when dashpot is activated, this is roughly the angular speed, stopped in 2 sec
	vectorf q;	// angles values
	vectorf qext; // additional angles values (angle[i] = q[i]+qext[i]; only q[i] is taken into account
								 // while calculating spring torque
	int op[2]; // body identifiers of parent and child respectively
	int nSelfCollidingParts,*pSelfCollidingParts; // part ids of only parts that should be checked for self-collision
	int bNoUpdate; // omit recalculation of body parameters after changing this joint
	float animationTimeStep; // used to calculate joint velocities of animation
	float ranimationTimeStep;	// 1/animation time step, can be not specified (specifying just saves extra division operation)

	VALIDATORS_START
		VALIDATOR(pivot)
		VALIDATOR_NORM(q0)
		VALIDATOR(q)
		VALIDATOR(qext)
	VALIDATORS_END
};

struct pe_params_articulated_body : pe_params {
	enum entype { type_id=8 };
	pe_params_articulated_body() {
		type=type_id;
		MARK_UNUSED bGrounded,bInheritVel,bCheckCollisions,bCollisionResp,bExpandHinges;
		MARK_UNUSED bGrounded,bInheritVel,bCheckCollisions,bCollisionResp, a,wa,w,v,pivot, scaleBounceResponse,posHostPivot;
		MARK_UNUSED bAwake,pHost,nCollLyingMode, gravityLyingMode,dampingLyingMode,minEnergyLyingMode,iSimType,iSimTypeLyingMode;
		bApply_dqext=0;	bRecalcJoints=1;
	}

	int bGrounded; // whether body's pivot is firmly attached to something or free
	int bCheckCollisions;
	int bCollisionResp;
	vectorf pivot; // attachment position for grounded bodies
	vectorf a; // acceleration of ground for grounded bodies
	vectorf wa; // angular acceleration of ground for grounded bodies
	vectorf w; // angular velocity of ground for grounded bodies
	vectorf v;
	float scaleBounceResponse; // scales impulsive torque that is applied at a joint that has just reached its limit
	int bApply_dqext;	// adds current dqext to joints velocities. dqext is the speed of external animation and is calculated each time
										// qext is set for joint (as difference between new value and current value, multiplied by inverse of animation timestep)
	int bAwake;

	IPhysicalEntity *pHost;
	vectorf posHostPivot;
	int bInheritVel;

	int nCollLyingMode;
	vectorf gravityLyingMode;
	float dampingLyingMode;
	float minEnergyLyingMode;
	int iSimType;
	int iSimTypeLyingMode;
	int bExpandHinges;

	int bRecalcJoints;
};

////////// living entity params

struct pe_player_dimensions : pe_params {
	enum entype { type_id=1 };
	pe_player_dimensions() { type=type_id; MARK_UNUSED sizeCollider,heightPivot,heightCollider,heightEye,heightHead,headRadius; }

	float heightPivot; // offset from central ground position that is considered entity center
	float heightEye; // vertical offset of camera
	vectorf sizeCollider; // collision cylinder dimensions
	float heightCollider;	// vertical offset of collision geometry center
	float headRadius;
	float heightHead;

	VALIDATORS_START
		VALIDATOR(heightPivot)
		VALIDATOR(heightEye)
		VALIDATOR_RANGE2(sizeCollider,0,100)
	VALIDATORS_END
};

struct pe_player_dynamics : pe_params {
	enum entype { type_id=4 };
	pe_player_dynamics() {
		type=type_id; MARK_UNUSED kInertia,kAirControl,gravity,nodSpeed,mass, bSwimming,surface_idx,bActive;
		MARK_UNUSED minSlideAngle,maxClimbAngle,maxJumpAngle,minFallAngle,kAirResistance,bNetwork,maxVelGround; }

	float kInertia;	// inertia koefficient, the more it is, the less inertia is; 0 means no inertia
	float kAirControl; // air control koefficient 0..1, 1 - special value (total control of movement)
	float kAirResistance;
	float gravity; // gravity vector
	float nodSpeed;
	int bSwimming; // whether entity is swimming (is not bound to ground plane)
	float mass;	// mass (in kg)
	int surface_idx; // surface identifier for collisions
	float minSlideAngle; // if surface slope is more than this angle, player starts sliding (angle is in radians)
	float maxClimbAngle; // player cannot climb surface which slope is steeper than this angle
	float maxJumpAngle; // player is not allowed to jump towards ground if this angle is exceeded
	float minFallAngle;	// player starts falling when slope is steeper than this
	float maxVelGround; // player cannot stand of surfaces that are moving faster than this
	int bNetwork;
	int bActive;
};

////////// particle entity params

struct pe_params_particle : pe_params {
	enum entype { type_id=3 };
	pe_params_particle() {
		type=type_id;
		MARK_UNUSED mass,size,thickness,wspin,accThrust,kAirResistance,kWaterResistance, velocity,heading,accLift,accThrust,gravity,waterGravity;
		MARK_UNUSED surface_idx, normal,q0,minBounceVel, flags,pColliderToIgnore, iPierceability;
	}

	unsigned int flags; // see entity flags
	float mass;
	float size; // pseudo-radius
	float thickness; // thickness when lying on a surface (if left unused, size will be used)
	vectorf heading; // direction of movement
	float velocity;	// velocity along "heading"
	float kAirResistance; // air resistance koefficient, F = kv
	float kWaterResistance; // same for water
	float accThrust; // acceleration along direction of movement
	float accLift; // acceleration that lifts particle with the current speed
	int surface_idx;
	vectorf wspin; // angular velocity
	vectorf gravity;
	vectorf waterGravity;
	vectorf normal;
	quaternionf q0;	// initial orientation (zero means x along direction of movement, z up)
	float minBounceVel;
	IPhysicalEntity *pColliderToIgnore;	// physical entity to ignore during collisions
	int iPierceability;

	VALIDATORS_START
		VALIDATOR(mass)
		VALIDATOR(size)
		VALIDATOR(thickness)
		VALIDATOR_NORM(heading)
		VALIDATOR_NORM(normal)
		VALIDATOR_NORM(q0)
	VALIDATORS_END
};

////////// vehicle entity params

struct pe_params_car : pe_params {
	enum entype { type_id=2 };
	pe_params_car() {
		type=type_id;
		MARK_UNUSED engineMaxRPM,iIntegrationType,axleFriction,enginePower,maxSteer,maxTimeStep,minEnergy,damping,brakeTorque;
		MARK_UNUSED engineMinRPM,engineShiftUpRPM,engineShiftDownRPM,engineIdleRPM,engineStartRPM,clutchSpeed,nGears,gearRatios,kStabilizer;
		MARK_UNUSED slipThreshold,gearDirSwitchRPM,kDynFriction,maxBrakingFriction,steerTrackNeutralTurn;
	}

	float axleFriction; // friction torque at axes divided by mass of vehicle
	float enginePower; // power of engine (about 10,000 - 100,000)
	float maxSteer;	// maximum steering angle
	float engineMaxRPM;	// engine torque decreases to 0 after reaching this rotation speed
	float brakeTorque;
	int iIntegrationType; // for suspensions; 0-explicit Euler, 1-implicit Euler
	float maxTimeStep; // maximum time step when vehicle has only wheel contacts
	float minEnergy; // minimum awake energy when vehicle has only wheel contacts
	float damping; // damping when vehicle has only wheel contacts
	float maxBrakingFriction; // limits the the tire friction when handbraked
	float kStabilizer; // stabilizer force, as a multiplier for kStiffness of respective suspensions
	int nWheels; // the number of wheels
	float engineMinRPM;
	float engineShiftUpRPM;
	float engineShiftDownRPM;
	float engineIdleRPM;
	float engineStartRPM;
	float clutchSpeed;
	int nGears;
	float *gearRatios;
	float slipThreshold;
	float gearDirSwitchRPM;
	float kDynFriction;
	float steerTrackNeutralTurn;
};

struct pe_params_wheel : pe_params {
	enum entype { type_id=16 };
	pe_params_wheel() {
		type=type_id; iWheel=0; MARK_UNUSED bDriving,iAxle,suspLenMax,minFriction,maxFriction,surface_idx;
	}

	int iWheel;
	int bDriving;
	int iAxle;
	float suspLenMax;
	float minFriction;
	float maxFriction;
	int surface_idx;
};

////////// rope entity params

struct pe_params_rope : pe_params {
	enum entype { type_id=13 };
	pe_params_rope() {
		type=type_id; MARK_UNUSED length,mass,bCheckCollisions,collDist,surface_idx,friction,nSegments,pPoints,pVelocities;
		MARK_UNUSED pEntTiedTo[0],ptTiedTo[0],idPartTiedTo[0],pEntTiedTo[1],ptTiedTo[1],idPartTiedTo[1];
	}

	float length;
	float mass;
	int bCheckCollisions;
	float collDist;
	int surface_idx;
	float friction;
	int nSegments;
	vectorf *pPoints;
	vectorf *pVelocities;
	int iStride; // used in GetParams only

	IPhysicalEntity *pEntTiedTo[2];
	vectorf ptTiedTo[2];
	int idPartTiedTo[2];
};

////////// soft entity params

struct pe_params_softbody : pe_params {
	enum entype { type_id=17 };
	pe_params_softbody() { type=type_id; MARK_UNUSED thickness,maxSafeStep,ks,kdRatio,airResistance,wind,nMaxIters,
		accuracy,friction,impulseScale,explosionScale,collisionImpulseScale,maxCollisionImpulse,collTypes; }

	float thickness;
	float maxSafeStep;
	float ks;
	float kdRatio;
	float friction;
	float airResistance;
	vectorf wind;
	int nMaxIters;
	float accuracy;
	float impulseScale;
	float explosionScale;
	float collisionImpulseScale;
	float maxCollisionImpulse;
	int collTypes;
};


////////////////////////// Action structures /////////////////////

////////// common actions
struct pe_action {
	int type;
};

struct pe_action_impulse : pe_action {
	enum entype { type_id=2 };

	pe_action_impulse() { type=type_id; impulse.Set(0,0,0); MARK_UNUSED point,momentum,partid,ipart; iApplyTime=2; iSource=0; }


	vectorf impulse;
	vectorf momentum;	// r x impulse, optional
	vectorf point; // point of application, in world CS, optional
	int partid;	// receiver part identifier
	int ipart; // alternatively, part index can be used
	int iApplyTime; // 0-apply immediately, 1-apply before the next time step, 2-apply after the next time step
	int iSource; // reserved for internal use

	VALIDATORS_START
		VALIDATOR_RANGE2(impulse,0,1E8f)
		VALIDATOR_RANGE2(momentum,0,1E8f)
		VALIDATOR_RANGE2(point,0,1E6f)
		VALIDATOR_RANGE(ipart,0,10000)
	VALIDATORS_END
};

struct pe_action_reset : pe_action { // Resets dynamic state of an entity
	enum entype { type_id=4 };
	pe_action_reset() { type=type_id; }
};

enum constrflags { local_frames=1, world_frames=2 };

struct pe_action_add_constraint : pe_action {
	enum entype { type_id=5 };
	pe_action_add_constraint() {
		type=type_id; pBuddy=0; MARK_UNUSED pt[0],pt[1],partid[0],partid[1],qframe[0],qframe[1],xlimits[0],yzlimits[0],pConstraintEntity; flags=world_frames;
	}

	IPhysicalEntity *pBuddy;
	vectorf pt[2];
	int partid[2];
	quaternionf qframe[2];
	float xlimits[2];
	float yzlimits[2];
	unsigned int flags;
	IPhysicalEntity *pConstraintEntity;
};

struct pe_action_remove_constraint : pe_action {
	enum entype { type_id=6 };
	pe_action_remove_constraint() { type=type_id; MARK_UNUSED idConstraint; }
	int idConstraint;
};

struct pe_action_register_coll_event : pe_action {
	enum entype { type_id=7 };
	pe_action_register_coll_event() { type=type_id; }

	vectorf pt;
	vectorf n;
	vectorf v;
	float collMass;
	IPhysicalEntity *pCollider;
	int partid[2];
	int idmat[2];
};

struct pe_action_awake : pe_action {
	enum entype { type_id=8 };
	pe_action_awake() { type=type_id; bAwake=1; }
	int bAwake;
};

struct pe_action_remove_all_parts : pe_action {
	enum entype { type_id=9 };
	pe_action_remove_all_parts() { type=type_id; }
};

struct pe_action_set_velocity : pe_action {
	enum entype { type_id=10 };
	pe_action_set_velocity() { type=type_id; MARK_UNUSED ipart,partid,v,w; }
	int ipart;
	int partid;
	vectorf v,w;
};

////////// living entity actions

struct pe_action_move : pe_action { // movement request for living entities
	enum entype { type_id=1 };
	pe_action_move() { type=type_id; iJump=0; dt=0; MARK_UNUSED dir; }

	vectorf dir; // dir requested velocity vector
	int iJump; // jump mode - 1-instant velocity change, 2-just adds to current velocity
	float dt;	// time interval for this action

	VALIDATORS_START
		VALIDATOR_RANGE2(dir,0,1000)
		VALIDATOR_RANGE(dt,0,2)
	VALIDATORS_END
};

////////// vehicle entity actions

struct pe_action_drive : pe_action {
	enum entype { type_id=3 };
	pe_action_drive() { type=type_id; MARK_UNUSED pedal,dpedal,steer,dsteer,bHandBrake,clutch,iGear; }

	float pedal; // engine pedal absolute value
	float dpedal; // engine pedal delta
	float steer; // steering angle absolute value
	float dsteer; // steering angle delta
	float clutch;
	int bHandBrake;
	int iGear;
};

////////// soft entity actions

struct pe_action_attach_points : pe_action {
	enum entype { type_id=11 };
	pe_action_attach_points() { type=type_id; MARK_UNUSED partid,points; nPoints=1; }

	IPhysicalEntity *pEntity;
	int partid;
	int *piVtx;
	vectorf *points;
	int nPoints;
};

////////////////////////// Status structures /////////////////////

////////// common statuses
struct pe_status {
	int type;
};

enum status_pos_flags { status_local=1 };

struct pe_status_pos : pe_status {
	enum entype { type_id=1 };
	pe_status_pos() { type=type_id; ipart=partid=-1; flags=0; pMtx3x3=0;pMtx3x3T=0;pMtx4x4=0;pMtx4x4T=0; iSimClass=0; timeBack=0; }

	int partid; // part identifier, -1 for entire entity
	int ipart; // optionally, part slot index
	unsigned int flags; // status_local if part coordinates should be returned in entity CS rather than world CS
	unsigned int flagsOR; // boolean OR for all parts flags of the object (or just flags for the selected part)
	unsigned int flagsAND; // boolean AND for all parts flags of the object (or just flags for the selected part)
	vectorf pos; // position of center
	vectorf BBox[2]; // bounding box relative to pos (bbox[0]-min, bbox[1]-max)
	quaternionf q;
	float scale;
	int iSimClass;
	float *pMtx3x3;	// optional 3x3 matrix buffer that receives transformation
	float *pMtx3x3T;	// optional 3x3 column-major matrix buffer that receives transformation
	float *pMtx4x4;	// optional 4x4 matrix buffer that receives transformation
	float *pMtx4x4T;	// optional 4x4 column-major matrix buffer that receives transformation
	IGeometry *pGeom,*pGeomProxy;
	float timeBack; // can retrieve pervious position; only supported by rigid entities; pos and q; one step back
};

struct pe_status_sensors : pe_status { // Requests status of attached to the entity sensors
	enum entype { type_id=18 };
	pe_status_sensors() { type=type_id; }

	vectorf *pPoints;	// pointer to array of points where sensors touch environment (assigned by physical entity)
	vectorf *pNormals; // pointer to array of surface normals at points where sensors touch environment
	unsigned int flags; // bitmask of flags, bit==1 - sensor touched environment
};

struct pe_status_dynamics : pe_status {
	enum entype { type_id=8 };
	pe_status_dynamics() : v(zero),w(zero),a(zero),wa(zero),centerOfMass(zero) {
		MARK_UNUSED partid,ipart; type=type_id; time_interval=0; submergedFraction=0; waterResistance=0;
	}

	int partid;
	int ipart;
	vectorf v; // velocity
	vectorf w; // angular velocity
	vectorf a; // linear acceleration
	vectorf wa; // angular acceleration
	vectorf centerOfMass;
	float submergedFraction;
	float waterResistance;
	float mass;
	float time_interval;
};

struct coll_history_item {
	vectorf pt; // collision area center
	vectorf n; // collision normal in entity CS
	vectorf v[2]; // velocities of contacting bodies at the point of impact
	float mass[2]; // masses of contacting bodies
	float age; // age of collision event
	int idCollider; // id of collider (not a pointer, since collider can be destroyed before history item is queried)
	int partid[2];
	int idmat[2];	// 0-this body material, 1-collider material
};

struct pe_status_collisions : pe_status {
	enum entype { type_id=9 };
	pe_status_collisions() { type=type_id; age=0; len=1; pHistory=0; bClearHistory=0; }

	coll_history_item *pHistory; // pointer to a user-provided array of history items
	int len; // length of this array
	float age; // maximum age of collision events (older events are ignored)
	int bClearHistory;
};

struct pe_status_id : pe_status {
	enum entype { type_id=10 };
	pe_status_id() { type=type_id; ipart=partid=-1; bUseProxy=1; }

	int ipart;
	int partid;
	int iPrim;
	int iFeature;
	int bUseProxy;
	int id; // usually id means material
};

struct pe_status_timeslices : pe_status {
	enum entype { type_id=11 };
	pe_status_timeslices() { type=type_id; pTimeSlices=0; sz=1; precision=0.0001f; MARK_UNUSED time_interval; }

	float *pTimeSlices;
	int sz;
	float precision; // time surplus below this threshhold will be discarded
	float time_interval; // if unused, time elapsed since the last action will be used
};

struct pe_status_nparts : pe_status {
	enum entype { type_id=12 };
	pe_status_nparts() { type=type_id; }
};

struct pe_status_awake : pe_status {
	enum entype { type_id=7 };
	pe_status_awake() { type=type_id; }
};

struct pe_status_contains_point : pe_status {
	enum entype { type_id=13 };
	pe_status_contains_point() { type=type_id; }
	vectorf pt;
};

struct pe_status_placeholder : pe_status {
	enum entype { type_id=16 };
	pe_status_placeholder() { type=type_id; }
	IPhysicalEntity *pFullEntity;
};

struct pe_status_sample_contact_area : pe_status {
	enum entype { type_id=19 };
	pe_status_sample_contact_area() { type=type_id; }
	vectorf ptTest;
	vectorf dirTest;
};

struct pe_status_caps : pe_status {
	enum entype { type_id=20 };
	pe_status_caps() { type=type_id; }
	unsigned int bCanAlterOrientation; // can change orientation that is explicitly set from outside
};

////////// living entity statuses

struct pe_status_living : pe_status {
	enum entype { type_id=2 };
	pe_status_living() { type=type_id; }

	int bFlying; // whether entity has no contact with ground
	float timeFlying; // for how long the entity was flying
	vectorf camOffset; // camera offset
	vectorf vel; // actual velocity (as rate of position change)
	vectorf velUnconstrained; // 'physical' movement velocity
	vectorf velRequested;	// velocity requested in the last action
	vectorf velGround; // velocity of the object entity is standing on
	float groundHeight; // position where the last contact with the ground occured
	vectorf groundSlope;
	int groundSurfaceIdx;
	IPhysicalEntity *pGroundCollider;
	int iGroundColliderPart;
	float timeSinceStanceChange;
	int bOnStairs;
};

////////// vehicle entity statuses

struct pe_status_vehicle : pe_status {
	enum entype { type_id=4 };
	pe_status_vehicle() { type=type_id; }

	float steer; // current steering angle
	float pedal; // current engine pedal
	int bHandBrake;	// nonzero if handbrake is on
	float footbrake; // nonzero if footbrake is pressed (range 0..1)
	vectorf vel;
	int bWheelContact; // nonzero if at least one wheel touches ground
	int iCurGear;
	float engineRPM;
	float clutch;
	int nActiveColliders;
};

struct pe_status_wheel : pe_status {
	enum entype { type_id=5 };
	pe_status_wheel() { type=type_id; iWheel=0; }
	int iWheel;

	int bContact;	// nonzero if wheel touches ground
	vectorf ptContact; // point where wheel touches ground
	float w; // rotation speed
	int bSlip;
	vectorf velSlip; // slip velocity
	int contactSurfaceIdx;
	float suspLen; // current suspension spring length
	float suspLenFull; // relaxed suspension spring length
	float suspLen0; // initial suspension spring length
};

struct pe_status_vehicle_abilities : pe_status {
	enum entype { type_id=15 };
	pe_status_vehicle_abilities() { type=type_id; MARK_UNUSED steer; }

	float steer; // should be set to requested steering angle
	vectorf rotPivot;	// returns turning circle center
	float maxVelocity; // calculates maximum velocity of forward movement along a plane (steer is ignored)
};

////////// articulated entity statuses

struct pe_status_joint : pe_status {
	enum entype { type_id=6 };
	pe_status_joint() { type=type_id; idChildBody=-1; }

	int idChildBody; // requested joint is identified by child body id
	unsigned int flags; // joint flags
	vectorf q;	// current joint angles (controlled by physics)
	vectorf qext; // external angles (from animation)
	vectorf dq; // current joint angular velocities
	quaternionf quat0;
};

////////// rope entity statuses

struct pe_status_rope : pe_status {
	enum entype { type_id=14 };
	pe_status_rope() { type=type_id; pPoints=pVelocities=0; }

	int nSegments;
	vectorf *pPoints;
	vectorf *pVelocities;
};

////////// soft entity statuses

struct pe_status_softvtx : pe_status {
	enum entype { type_id=17 };
	pe_status_softvtx() { type=type_id; pVtx=pNormals=0; }

	int nVtx;
	strided_pointer<vectorf> pVtx;
	strided_pointer<vectorf> pNormals;
};

////////////////////////// Geometry structures /////////////////////

////////// common geometries
enum geom_flags { geom_colltype0=0x0001, geom_colltype1=0x0002, geom_colltype2=0x0004, geom_colltype3=0x0008, geom_colltype4=0x0010,
									geom_colltype5=0x0020, geom_colltype6=0x0040, geom_colltype7=0x0080, geom_colltype8=0x0100, geom_colltype9=0x0200,
									geom_colltype10=0x0400,geom_colltype11=0x0800,geom_colltype12=0x1000,geom_colltype13=0x2000,geom_colltype14=0x4000,
									geom_colltype_ray=0x8000, geom_collides=0xFFFF, geom_floats=0x10000, geom_has_thin_parts=0x20000,
									geom_colltype_player=geom_colltype1, geom_colltype_explosion=geom_colltype2, geom_proxy=0x40000 };

struct pe_geomparams {
	enum entype { type_id=0 };
	pe_geomparams() {
		type=type_id; density=mass=0; pos.Set(0,0,0); q.SetIdentity(); bRecalcBBox=1;
		flags = geom_collides|geom_floats; flagsCollider = geom_colltype0;
		pMtx3x3=0;pMtx3x3T=0;pMtx4x4=0;pMtx4x4T=0; scale=1.0f;
		MARK_UNUSED surface_idx,minContactDist;
	}

	int type;
	float density; // 0 if mass is used
	float mass; // 0 if density is used
	vectorf pos; // offset from object's geometrical pivot
	quaternionf q; // orientation relative to object
	float scale;
	float *pMtx3x3;	// optional 3x3 orintation+scale matrix
	float *pMtx3x3T;	// optional 3x3 column-major orintation+scale matrix
	float *pMtx4x4;	// optional 4x4 transformation matrix
	float *pMtx4x4T;	// optional 4x4 column-major transformation matrix
	int surface_idx; // surface identifier (used if corresponding CGeometry does not contain materials)
	unsigned int flags,flagsCollider;
	float minContactDist;
	int bRecalcBBox;

	VALIDATORS_START
		VALIDATOR_RANGE(density,-1E8,1E8)
		VALIDATOR_RANGE(mass,-1E8,1E8)
		VALIDATOR(pos)
		VALIDATOR_NORM(q)
	VALIDATORS_END
};

////////// articulated entity geometries

struct pe_articgeomparams : pe_geomparams {
	enum entype { type_id=2 };
	pe_articgeomparams() { type=type_id; idbody=0; }
	pe_articgeomparams(pe_geomparams &src) {
		type=type_id;	density=src.density; mass=src.mass;
		pos=src.pos; q=src.q; scale=src.scale; surface_idx=src.surface_idx;
		pMtx3x3=src.pMtx3x3; pMtx3x3T=src.pMtx3x3T; pMtx4x4=src.pMtx4x4; pMtx4x4T=pMtx4x4T;
		idbody=0;	minContactDist=src.minContactDist; bRecalcBBox=src.bRecalcBBox;
	}
	int idbody; // id of the subbody this geometry is attached to, the 1st add geometry specifies frame CS of this subbody
};

////////// vehicle entity geometries

const int NMAXWHEELS = 16;
struct pe_cargeomparams : pe_geomparams {
	enum entype { type_id=1 };
	pe_cargeomparams() : pe_geomparams() { type=type_id; MARK_UNUSED bDriving,minFriction,maxFriction; bCanBrake=1; }
	pe_cargeomparams(pe_geomparams &src) {
		type=type_id;	density=src.density; mass=src.mass;
		pos=src.pos; q=src.q; surface_idx=src.surface_idx;
		MARK_UNUSED bDriving,minFriction,maxFriction; bCanBrake=1;
	}
	int bDriving;	// whether wheel is driving, -1 - geometry os not a wheel
	int iAxle; // wheel axle, currently not used
	int bCanBrake; // whether the wheel is locked during handbrakes
	vectorf pivot; // upper suspension point in vehicle CS
	float lenMax;	// relaxed suspension length
	float lenInitial; // current suspension length (assumed to be length in rest state)
	float kStiffness; // suspension stiffness, if 0 - calculate from lenMax, lenInitial, and vehicle mass and geometry
	float kDamping; // suspension damping, if <0 - calculate as -kdamping*(approximate zero oscillations damping)
	float minFriction,maxFriction; // additional friction limits for tire friction
};


/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// IGeometry Interface ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

struct geom_world_data {
	geom_world_data() {
		v.Set(0,0,0);
		w.Set(0,0,0);
		offset.Set(0,0,0);
		R.SetIdentity();
		centerOfMass.Set(0,0,0);
		scale=1.0f; iStartNode=0;
	}
	vectorf offset;
	matrix3x3f R;
	float scale;
	vectorf v,w;
	vectorf centerOfMass;
	int iStartNode;
};

struct intersection_params {
	intersection_params() {
		iUnprojectionMode=0;
		vrel_min=1E-6f;
		time_interval=100.0f;
		maxSurfaceGapAngle=1.0f*float(pi/180);
		pGlobalContacts=0;
		minAxisDist=0;
		bSweepTest=false;
		centerOfRotation.Set(0,0,0);
		axisContactNormal.Set(0,0,1);
		unprojectionPlaneNormal.Set(0,0,0);
		axisOfRotation.Set(0,0,0);
		bKeepPrevContacts=false;
		bStopAtFirstTri=false;
		ptOutsidePivot[0].Set(1E11f,1E11f,1E11f);
		ptOutsidePivot[1].Set(1E11f,1E11f,1E11f);
		maxUnproj=1E10f;
		bNoAreaContacts=false;
		bNoBorder=false;
		bNoIntersection=0;
	}
	int iUnprojectionMode;
	vectorf centerOfRotation;
	vectorf axisOfRotation;
	float time_interval;
	float vrel_min;
	float maxSurfaceGapAngle;
	float minAxisDist;
	vectorf unprojectionPlaneNormal;
	vectorf axisContactNormal;
	float maxUnproj;
	vectorf ptOutsidePivot[2];
	bool bSweepTest;
	bool bKeepPrevContacts;
	bool bStopAtFirstTri;
	bool bNoAreaContacts;
	bool bNoBorder;
	int bNoIntersection;
	int bBothConvex;
	geom_contact *pGlobalContacts;

};

struct phys_geometry {
	IGeometry *pGeom;
	vectorf Ibody;
	quaternionf q;
	vectorf origin;
	float V;
	int nRefCount;
	int surface_idx;
};

enum geomtypes { GEOM_TRIMESH=0,GEOM_HEIGHTFIELD=1,GEOM_CYLINDER=2,GEOM_RAY=3,GEOM_SPHERE=4,GEOM_BOX=5 };

class IGeometry {
public:
	virtual int GetType() = 0;
	virtual void Release() = 0;
	virtual void GetBBox(primitives::box *pbox) = 0;
	virtual int CalcPhysicalProperties(phys_geometry *pgeom) = 0;
	virtual int PointInsideStatus(const vectorf &pt) = 0;
	virtual int Intersect(IGeometry *pCollider, geom_world_data *pdata1,geom_world_data *pdata2, intersection_params *pparams, geom_contact *&pcontacts) = 0;
	virtual int FindClosestPoint(geom_world_data *pgwd, int &iPrim,int &iFeature, const vectorf &ptdst0,const vectorf &ptdst1,
		vectorf *ptres, int nMaxIters=10) = 0;
	virtual void CalcVolumetricPressure(geom_world_data *gwd, const vectorf &epicenter,float k,float rmin,
		const vectorf &centerOfMass, vectorf &P,vectorf &L) = 0;
	virtual float CalculateBuoyancy(const primitives::plane *pplane, const geom_world_data *pgwd, vectorf &submergedMassCenter) = 0;
	virtual void CalculateMediumResistance(const primitives::plane *pplane, const geom_world_data *pgwd, vectorf &dPres,vectorf &dLres) = 0;
	virtual void DrawWireframe(void (*DrawLineFunc)(float*,float*), geom_world_data *gwd, int iLevel) = 0;
	virtual int GetPrimitiveId(int iPrim,int iFeature) = 0;
	virtual int GetPrimitive(int iPrim, primitives::primitive *pprim) = 0;
	virtual int GetFeature(int iPrim,int iFeature, vectorf *pt) = 0;
	virtual int IsConvex(float tolerance) = 0;
	virtual void PrepareForRayTest(float raylen) = 0;
	virtual float BuildOcclusionCubemap(geom_world_data *pgwd, int iMode, int *pGrid0[6],int *pGrid1[6],int nRes, float rmin,float rmax, int nGrow) = 0;
	virtual void GetMemoryStatistics(ICrySizer *pSizer) = 0;
	virtual void Save(CMemStream &stm) = 0;
	virtual void Load(CMemStream &stm) = 0;
	virtual void RemapFaceIds(short *pMap,int sz) = 0;
	virtual int GetPrimitiveCount() = 0;
};


/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// IGeometryManager Interface /////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

enum meshflags { mesh_OBB=1, mesh_AABB=2, mesh_SingleBB=4, mesh_multicontact0=8, mesh_multicontact1=16, mesh_multicontact2=32,
								 mesh_approx_cylinder=0x100, mesh_approx_box=0x200, mesh_approx_sphere=0x400, mesh_uchar_ids=0x1000 };

class IGeomManager {
public:
	virtual void InitGeoman() = 0;
	virtual void ShutDownGeoman() = 0;

	virtual IGeometry *CreateMesh(strided_pointer<const vectorf> pVertices,index_t *pIndices,const short *pIds,int nTris, int flags,bool bCopyTriangles=true,
		bool bCopyVertices=true, float approx_tolerance=0.05f, int nMinTrisPerNode=2,int nMaxTrisPerNode=4, float favorAABB=1.0f) = 0;
	virtual IGeometry *CreatePrimitive(int type, const primitives::primitive *pprim) = 0;
	virtual void DestroyGeometry(IGeometry *pGeom) = 0;

	// defSurfaceIdx will be used (until overwritten in entity part) if the geometry doesn't have per-face materials
	virtual phys_geometry *RegisterGeometry(IGeometry *pGeom,int defSurfaceIdx=0) = 0;
	virtual int AddRefGeometry(phys_geometry *pgeom) = 0;
	virtual int UnregisterGeometry(phys_geometry *pgeom) = 0;

	virtual void SaveGeometry(CMemStream &stm, IGeometry *pGeom) = 0;
	virtual IGeometry *LoadGeometry(CMemStream &stm) = 0;
	virtual void SavePhysGeometry(CMemStream &stm, phys_geometry *pgeom) = 0;
	virtual phys_geometry *LoadPhysGeometry(CMemStream &stm) = 0;
	virtual void RemapPhysGeometryFaceIds(phys_geometry *pgeom,short *pMap,int sz) = 0;
};


/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// IPhysUtils Interface /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class IPhysUtils {
public:
	virtual int BreakPolygon(vector2df *ptSrc,int nPt, int nCellx,int nCelly, int maxPatchTris, vector2df *&ptout,int *&nPtOut,
													 float jointhresh=0.5f,int seed=-1) = 0;
	virtual int CoverPolygonWithCircles(strided_pointer<vector2df> pt,int npt,bool bConsecutive, const vector2df &center,
		vector2df *&centers,float *&radii, float minCircleRadius) = 0;
	virtual void DeletePointer(void *pdata) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// IPhysicalEntity Interface //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

enum snapshot_flags { ssf_compensate_time_diff=1, ssf_checksum_only=2, ssf_no_update=4 };

class IPhysicalEntity {
public:
	/*! Retrieves entity type
		@returb entity type enum
	*/
	virtual pe_type GetType() = 0;

	/*! Sets parameters
		@param params pointer to parameters structure
		@return nonzero if success
	*/
	virtual int SetParams(pe_params* params) = 0;
	virtual int GetParams(pe_params* params) = 0;
	/*! Retrieves status
		@param status pointer to status structure
		@retuan nonzero if success
	*/
	virtual int GetStatus(pe_status* status) = 0;
	/*! Performes action
		@param action pointer to action structure
		@return nonzero if success
	*/
	virtual int Action(pe_action*) = 0;

	/*! Adds geometry
		@param pgeom geometry identifier (obtained from RegisterXXX function)
		@param params pointer to geometry parameters structure
		@param id requested geometry id, if -1 - assign automatically
		@return geometry id (0..some number), -1 means error
	*/
	virtual int AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id=-1) = 0;
	/*! Removes geometry
		@param params pointer to parameters structure
		@return nonzero if success
	*/
	virtual void RemoveGeometry(int id) = 0;

	/*! Retrieves foreign data passed during creation (can be pointer to the corresponding engine entity, for instance)
		@param iforeigndata requested foreign data type
		@return foreign data (void*) if itype==iforeigndata of this entity, 0 otherwise
	*/
	virtual void *GetForeignData(int itype=0) = 0;

	/*! Retrieves iforeigndata of the entity (usually it will be a type identifier for pforeign data
		@return iforeigndata
	*/
	virtual int GetiForeignData() = 0;

	/*! Writes state into snapshot
		@param stm stream
		@param time_back requests previous state (only supported by living entities)
		@params flags a combination of snapshot_flags
		@return non0 if successful
	*/
	virtual int GetStateSnapshot(class CStream &stm, float time_back=0, int flags=0) = 0;
	/*! Reads state from snapshot
		@param stm stream
		@return size of snapshot
	*/
	virtual int SetStateFromSnapshot(class CStream &stm, int flags=0) = 0;
	virtual int PostSetStateFromSnapshot() = 0;
	virtual int GetStateSnapshotTxt(char *txtbuf,int szbuf, float time_back=0) = 0;
	virtual void SetStateFromSnapshotTxt(const char *txtbuf,int szbuf) = 0;
	virtual unsigned int GetStateChecksum() = 0;
	/*! StartStep should be called before step with the full time interval (if the step will be split into substeps)
	*/
	virtual void StartStep(float time_interval) = 0;
	/*! Evolves entity in time. Normally this is called from PhysicalWorld::TimeStep
		@param time_interval time step
	*/
	virtual int Step(float time_interval) = 0;
	/*! Restores previous entity state that corresponds to time -time_interval from now, interpolating when
			necessary. This can be used for	manual client-server synchronization. Outside of PhysicalWorld::TimeStep
			should be called only for living entities
		@param time_interval time to trace back
	*/
	virtual void StepBack(float time_interval) = 0;
	/*! Returns physical world this entity belongs to
		@return physical world interface
	*/
	virtual IPhysicalWorld *GetWorld() = 0;

	virtual void GetMemoryStatistics(ICrySizer *pSizer) = 0;
};


/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// IPhysicsEventClient Interface //////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

class IPhysicsEventClient {
public:
	virtual void OnBBoxOverlap(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData,
		IPhysicalEntity *pCollider, void *pColliderForeignData,int iColliderForeignData) = 0;
	virtual void OnStateChange(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, int iOldSimClass,int iNewSimClass) = 0;
	virtual void OnCollision(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, coll_history_item *pCollision) = 0;
	virtual int OnImpulse(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, pe_action_impulse *impulse) = 0;
	virtual void OnPostStep(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, float dt) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// IPhysicalWorld Interface //////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

enum draw_helper_flags { pe_helper_collisions=1, pe_helper_geometry=2, pe_helper_bbox=4 };
enum surface_flags { sf_pierceable_mask=0x0F, sf_pierceable0=0, sf_pierceable=1, sf_max_pierceable=0x0F, sf_important=0x200 };
enum rwi_flags { rwi_ignore_terrain_holes=0x20, rwi_ignore_noncolliding=0x40, rwi_ignore_back_faces=0x80, rwi_any_hit=0x100,
								 rwi_pierceability_mask=0x0F, rwi_pierceability0=0, rwi_stop_at_pierceable=0x0F, rwi_separate_important_hits=sf_important,
								 rwi_colltype_bit=16 };
enum entity_query_flags { ent_static=1, ent_sleeping_rigid=2, ent_rigid=4, ent_living=8, ent_independent=16, ent_deleted=128, ent_terrain=0x100,
													ent_all=ent_static | ent_sleeping_rigid | ent_rigid | ent_living | ent_independent | ent_terrain,
													ent_flagged_only = pef_update, ent_skip_flagged = pef_update*2, ent_triggers = 64,
													ent_ignore_noncolliding = 0x10000, ent_sort_by_mass = 0x20000, ent_allocate_list = 0x40000 };

struct phys_profile_info {
	IPhysicalEntity *pEntity;
	int nTicks;
	int nCalls;
	int id;
	const char *pName;
};

struct SolverSettings {
	int nMaxStackSizeMC;							// def 8
	float maxMassRatioMC;							// def 50
	int nMaxMCiters;									// def 1400
	int nMaxMCitersHopeless;					// def 400
	float accuracyMC;									// def 0.005
	float accuracyLCPCG;							// def 0.005
	int nMaxContacts;									// def 150
	int nMaxPlaneContacts;            // def 7
	int nMaxPlaneContactsDistress;		// def 4
	int nMaxLCPCGsubiters;						// def 120
	int nMaxLCPCGsubitersFinal;				// def 250
	int nMaxLCPCGmicroiters;
	int nMaxLCPCGmicroitersFinal;
	int nMaxLCPCGiters;								// def 5
	float minLCPCGimprovement;        // def 0.1
	int nMaxLCPCGFruitlessIters;			// def 4
	float accuracyLCPCGnoimprovement;	// def 0.05
	float minSeparationSpeed;					// def 0.02
	float maxvCG;
	float maxwCG;
	float maxvUnproj;
};

struct PhysicsVars : SolverSettings {
  int bFlyMode;
	int iCollisionMode;
	int bSingleStepMode;
	int bDoStep;
	float fixedTimestep;
	float timeGranularity;
	float maxWorldStep;
	int iDrawHelpers;
	float maxContactGap;
	float maxContactGapPlayer;
	float minBounceSpeed;
	int bProhibitUnprojection;
	int bUseDistanceContacts;
	float unprojVelScale;
	float maxUnprojVel;
	int bEnforceContacts;
	int nMaxSubsteps;
	int nMaxSurfaces;
	vectorf gravity;
	int nGroupDamping;
	float groupDamping;
	int bBreakOnValidation;
	int bLogActiveObjects;
	int bMultiplayer;
	int bProfileEntities;
	int nGEBMaxCells;
	float maxVel;
	float maxVelPlayers;
	float maxContactGapSimple;
	float penaltyScale;
	int bSkipRedundantColldet;
	int bLimitSimpleSolverEnergy;
};

struct ray_hit {
	float dist;
	IPhysicalEntity *pCollider;
	int ipart;
	int partid;
	int surface_idx;
	vectorf pt;
	vectorf n;
	int bTerrain;
};

class IPhysicalWorld {
public:
	/*! Inits world
		@param pconsole pointer of IConsole interace
	*/
	virtual void Init() = 0;

	virtual IGeomManager* GetGeomManager() = 0;
	virtual IPhysUtils* GetPhysUtils() = 0;

	/*! Shuts the world down
	*/
	virtual void Shutdown(int bDeleteGeometries = 1) = 0;
	/*! Destroys the world
	*/
	virtual void Release() = 0;

	/*! Initializes entity hash grid
		@param axisx id of grid x axis (0-world x,1-world y,2-world z)
		@param axisy id of grid y axis (0-world x,1-world y,2-world z)
		@param origin grid (0,0) in world CS
		@param nx number of cells in grid x direciton
		@param ny number of cells in grid y direciton
		@param stepx cell x dimension
		@param stepy cell y dimension
	*/
	virtual void SetupEntityGrid(int axisz, vectorf org, int nx,int ny, float stepx,float stepy) = 0;
	/*! Sets heightfield data
		@param phf
	*/
	virtual void SetHeightfieldData(const primitives::heightfield *phf) = 0;
	/*! Retrieves heightfield data
		@param phf
		@return 0 if no global heightfield, 1 otherwise
	*/
	virtual int GetHeightfieldData(primitives::heightfield *phf) = 0;
	/*! Retrieves pointer to physvars structure
		@return pointer to physvar structure
	*/
	virtual PhysicsVars *GetPhysVars() = 0;

	/*! Creates physical entity
		@param type entity type
		@param params initial params (as in entity SetParams)
		@param pforeigndata entity foreign data
		@param iforeigndata entity foreign data type identifier
		@return pointer to physical entity interface
	*/
	virtual IPhysicalEntity* CreatePhysicalEntity(pe_type type, pe_params* params=0, void *pforeigndata=0, int iforeigndata=0, int id=-1) = 0;
	virtual IPhysicalEntity* CreatePhysicalEntity(pe_type type, float lifeTime, pe_params* params=0, void *pForeignData=0,int iForeignData=0,
		int id=-1,IPhysicalEntity *pHostPlaceholder=0) = 0;
	virtual IPhysicalEntity *CreatePhysicalPlaceholder(pe_type type, pe_params* params=0, void *pForeignData=0,int iForeignData=0, int id=-1) = 0;
	/*! Destroys physical entity
		@param pent entity
		@param mode 0-normal destroy, 1-suspend, 2-restore from suspended state
		@return nonzero if success
	*/
	virtual int DestroyPhysicalEntity(IPhysicalEntity *pent, int mode=0) = 0;

	virtual int SetPhysicalEntityId(IPhysicalEntity *pent, int id, int bReplace=1) = 0;
	virtual int GetPhysicalEntityId(IPhysicalEntity *pent) = 0;
	virtual IPhysicalEntity* GetPhysicalEntityById(int id) = 0;

	/*! Sets surface parameters
		@param surface_idx surface identifier
		@param bounciness restitution coefficient (for pair of surfaces k = sum of their coefficients, clamped to [0..1]
		@param friction friction coefficient (for pair of surfaces k = sum of their coefficients, clamped to [0..inf)
		@param flags bitmask (see surface_flags enum)
		@return nonzero if success
	*/
	virtual int SetSurfaceParameters(int surface_idx, float bounciness,float friction, unsigned int flags=0) = 0;
	virtual int GetSurfaceParameters(int surface_idx, float &bounciness,float &friction, unsigned int &flags) = 0;

	/*! Perfomes a time step
		@param time_interval time interval
		@param flags entity types to update (ent_..; ent_deleted to purge deletion physics-on-demand state monitoring)
	*/
	virtual void TimeStep(float time_interval,int flags=ent_all|ent_deleted) = 0;
	/*! Returns current time of the physical world
		@return current time
	*/
	virtual float GetPhysicsTime() = 0;
	virtual int GetiPhysicsTime()  = 0;
	/*! Sets current time of the physical world
		@param time new time
	*/
	virtual void SetPhysicsTime(float time) = 0;
	virtual void SetiPhysicsTime(int itime) = 0;
	/*! Sets physical time that corresponds to the following server state snapshot
		@param time_snapshot physical time of the following server snapshot
	*/
	virtual void SetSnapshotTime(float time_snapshot,int iType=0) = 0;
	virtual void SetiSnapshotTime(int itime_snapshot,int iType=0) = 0;

	/*! Retrives list of entities that fall into a box
		@param ptmix,ptmax - box corners
		@param pList returned pointer to entity list
		@param objtypes	bitmask 0-static, 1-sleeping, 2-physical, 3-living
		@return number of entities
	*/
	virtual int GetEntitiesInBox(vectorf ptmin,vectorf ptmax, IPhysicalEntity **&pList, int objtypes) = 0;

	/*! Shoots ray into world
		@param origin origin
		@param dir direction*(ray length)
		@param objtypes	bitmask 0-terrain 1-static, 2-sleeping, 3-physical, 4-living
		@param flags a combination of rwi_flags
		@param hits destination hits array
		@param nmaxhits size of this array
		@param pskipent entity to skip
		@return number of collisions
	*/
	virtual int RayWorldIntersection(vectorf org,vectorf dir, int objtypes, unsigned int flags, ray_hit *hits,int nMaxHits,
		IPhysicalEntity *pSkipEnt=0,IPhysicalEntity *pSkipEntAux=0) = 0;

	/*! Freezes (resets velocities of) all physical, living, and detached entities
	*/
	virtual void ResetDynamicEntities() = 0;
	/*! Immediately destroys all physical, living, and detached entities; flushes the deleted entities
		All subsequent calls to DestroyPhysicalEntity for non-static entities are ignored until the next
		non-static entity is created
	*/
	virtual void DestroyDynamicEntities() = 0;
	/*! Forces deletion of all entities marked as deleted
	*/
	virtual void PurgeDeletedEntities() = 0;

	/*! Simulates physical explosion with k/(r^2) pressure distribution
		@param epicenter epicenter used for building the occlusion map
		@param epicenterImp epicenter used for applying impulse
		@param rmin all r<rmin are set to rmin to avoid singularity in center
		@param rmax clamps entities father than rmax
		@param r radius at which impulsive pressure is spesified
		@param impulsive_pressure_at_r impulsive pressure at r
		@param nOccRes resolution of occulision cubemap (0 to skip occlusion test)
		@param nGrow inflate dynamic objects' rasterized image by this amount
		@params rmin_occ subtract cube with this size (half length of its side) during rasterization
		@params pSkipEnts pointer to array of entities to skip
		@params nSkipEnts number of entities to skip
	*/
	virtual void SimulateExplosion(vectorf epicenter,vectorf epicenterImp, float rmin,float rmax, float r,float impulsive_pressure_at_r,
		int nOccRes=0,int nGrow=0,float rmin_occ=0.1f, IPhysicalEntity **pSkipEnts=0,int nSkipEnts=0,
		int iTypes=ent_rigid|ent_sleeping_rigid|ent_living|ent_independent) = 0;

	/*! Returns fraction of pent (0-1) that was exposed to the last explosion
	*/
	virtual float IsAffectedByExplosion(IPhysicalEntity *pent) = 0;

	virtual void DrawPhysicsHelperInformation(void (*DrawLineFunc)(float*,float*)) = 0;

	virtual int CollideEntityWithBeam(IPhysicalEntity *_pent, vectorf org,vectorf dir,float r, ray_hit *phit) = 0;
	virtual int RayTraceEntity(IPhysicalEntity *pient, vectorf origin,vectorf dir, ray_hit *pHit, pe_params_pos *pp=0) = 0;

	virtual void GetMemoryStatistics(ICrySizer *pSizer) = 0;

	virtual void SetPhysicsStreamer(IPhysicsStreamer *pStreamer) = 0;
	virtual void SetPhysicsEventClient(IPhysicsEventClient *pEventClient) = 0;
	virtual float GetLastEntityUpdateTime(IPhysicalEntity *pent) = 0;
	virtual int GetEntityProfileInfo(phys_profile_info *&pList) = 0;

	virtual int SerializeWorld(const char *fname, int bSave) = 0;
	virtual int SerializeGeometries(const char *fname, int bSave) = 0;
};

#endif