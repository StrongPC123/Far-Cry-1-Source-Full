//////////////////////////////////////////////////////////////////////
//
//	Articulated Entity header
//	
//	File: articulatedentity.h
//	Description : CArticulatedEntity class declaration
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef articulatedentity_h
#define articulatedentity_h
#pragma once

struct featherstone_data {
	vectori qidx2axidx,axidx2qidx;
	vectorf Ya_vec[2];
	vectorf dv_vec[2];
	vectorf s_vec[3][2];
	vectorf Q;
	float qinv[9];
	float qinv_down[9];
	float s[18];
	DEFINE_ALIGNED_DATA( float, Ia[6][6], 16 );
	float Ia_s[18];
	DEFINE_ALIGNED_DATA( float, Ia_s_qinv_sT[6][6], 16 );
	DEFINE_ALIGNED_DATA( float, s_qinv_sT[6][6], 16 );
	DEFINE_ALIGNED_DATA( float, s_qinv_sT_Ia[6][6], 16 );
	float qinv_sT[3][6];
	float qinv_sT_Ia[3][6];
	DEFINE_ALIGNED_DATA( float, Iinv[6][6], 16 );
};

struct ae_joint {
	ae_joint() { 
		nChildren=nChildrenTree=0; iParent=-2; idbody=-1;
		q.zero(); qext.zero(); dq.zero(); dqext.zero(); ddq.zero();
		MARK_UNUSED dq_req.x,dq_req.y,dq_req.z; dq_limit.zero();
		bounciness.zero(); ks.zero(); kd.zero(); 
		qdashpot.zero(); kdashpot.zero();
		limits[0].Set(-1E10,-1E10,-1E10);
		limits[1].Set(1E10,1E10,1E10);
		flags=all_angles_locked; 
		quat0.SetIdentity();
		Pext.zero(); Lext.zero();
		Pimpact.zero(); Limpact.zero();
		//fs.Q.zero(); fs.Ya_vec[0].zero(); fs.Ya_vec[1].zero();
		//matrix3x3f(fs.qinv).identity();
		nActiveAngles = nPotentialAngles = 0;
		pivot[0].zero(); pivot[1].zero();
		iLevel = 0;
		iContacts = 0;
		iStartPart = nParts = 0;
		dv_body.zero(); dw_body.zero();
		selfCollMask = 0;
		fs = 0; fsbuf = 0;
		bQuat0Changed = 0;
	}
	~ae_joint() {
		if (fsbuf) delete fsbuf;
	}

	vectorf q;
	vectorf qext;
	vectorf dq;
	vectorf dqext;
	vectorf dq_req;
	vectorf dq_limit;
	vectorf ddq;
	quaternionf quat;

	vectorf prev_q,prev_dq;
	vectorf prev_pos,prev_v,prev_w;
	quaternionf prev_qrot;
	vectorf q0;

	unsigned int flags;
	quaternionf quat0;
	vectorf limits[2];
	vectorf bounciness;
	vectorf ks,kd;
	vectorf qdashpot,kdashpot;
	vectorf pivot[2];
	vectorf hingePivot[2];

	int iStartPart,nParts;
	int iParent;
	int nChildren,nChildrenTree;
	int iLevel;
	masktype selfCollMask;
	masktype iContacts;
	int bAwake;
	int bQuat0Changed;
	int bHasExtContacts;
	
	int idbody;
	RigidBody body;
	vectorf dv_body,dw_body;
	vectorf Pext,Lext;
	vectorf Pimpact,Limpact;
	int nActiveAngles,nPotentialAngles;
	vectorf rotaxes[3];
	matrix3x3f I;

	featherstone_data *fs;
	void *fsbuf;
};

struct ae_part_info {
	quaternionf q0;
	vectorf pos0;
	int iJoint;
	int idbody;
};


class CArticulatedEntity : public CRigidEntity, public IRigidBodyOwner {
 public:
	CArticulatedEntity(CPhysicalWorld *pworld);
	virtual ~CArticulatedEntity();
	virtual pe_type GetType() { return PE_ARTICULATED; }
	virtual void AlertNeighbourhoodND();

	virtual int AddGeometry(phys_geometry *pgeom, pe_geomparams* params,int id=-1);
	virtual void RemoveGeometry(int id);
	virtual int SetParams(pe_params *_params);
	virtual int GetParams(pe_params *_params);
	virtual int GetStatus(pe_status *_status);
	virtual int Action(pe_action*);

	virtual RigidBody *GetRigidBody(int ipart=-1);
	virtual void GetContactMatrix(const vectorf &pt, int ipart, matrix3x3f &K);
	virtual void AddImpulseAtContact(entity_contact *pcontact, int iop, const vectorf &dP) {};
	virtual vectorf GetVelocityAtContact(entity_contact *pcontact, int iop) { return vectorf(zero); };
	virtual int OnRegisterContact(entity_contact *pcontact, int iop);
	virtual void OnSolverEvent(int iEvent) {};

	virtual void GetMemoryStatistics(ICrySizer *pSizer);

	enum snapver { SNAPSHOT_VERSION = 6 };
	virtual int GetStateSnapshot(CStream &stm, float time_back=0,int flags=0);
	virtual int SetStateFromSnapshot(CStream &stm, int flags);

	virtual float GetMaxTimeStep(float time_interval);
	virtual int Step(float time_interval);
	virtual void StepBack(float time_interval);
	virtual int RegisterContacts(float time_interval,int nMaxPlaneContacts);
	virtual int Update(float time_interval, float damping);
	virtual float CalcEnergy(float time_interval);
	virtual float GetDamping(float time_interval);

	virtual int GetPotentialColliders(CPhysicalEntity **&pentlist);
	virtual int CheckSelfCollision(int ipart0,int ipart1);
	virtual int IsAwake(int ipart=-1);
	virtual void RecomputeMassDistribution(int ipart=-1,int bMassChanged=1);

	void SyncWithHost(int bRecalcJoints,float time_interval);
	void SyncBodyWithJoint(int idx, int flags=3);
	void SyncJointWithBody(int idx, int flags=1);
	void UpdateJointRotationAxes(int idx);
	void CheckForGimbalLock(int idx);
	int GetUnprojAxis(int idx, vectorf &axis);

	int StepJoint(int idx, float time_interval,masktype &contact_mask,int &bBounced, int bFlying);
	void JointListUpdated();
	int CalcBodyZa(int idx, float time_interval, vectornf &Za_change);
	int CalcBodyIa(int idx, matrixf& Ia_change);
	void CalcBodiesIinv(int bLockLimits);
	int CollectPendingImpulses(int idx,int &bNotZero);
	void PropagateImpulses(const vectorf &dv,int bLockLimits=0);
	void CalcVelocityChanges(float time_interval, const vectorf &dv,const vectorf &dw);
	void GetJointTorqueResponseMatrix(int idx, matrix3x3f &K);

	int IsChildOf(int idx, int iParent) { return isnonneg(iParent) & isneg(iParent-idx) & isneg(idx-iParent-m_joints[iParent].nChildrenTree-1); }
	entity_contact *CreateConstraintContact(int idx);

	ae_part_info *m_infos;
	ae_joint *m_joints;
	int m_nJoints, m_nJointsAlloc;
	vectorf m_posPivot, m_offsPivot;
	vectorf m_acc,m_wacc;
	matrix3x3f m_M0inv;
	vectorf m_Ya_vec[2];
	float m_simTime,m_simTimeAux;
	float m_scaleBounceResponse;
	int m_bGrounded;
	int m_bInheritVel;
	CPhysicalEntity *m_pHost;
	vectorf m_posHostPivot;
	int m_bCheckCollisions;
	int m_bCollisionResp;
	int m_bExertImpulse;
	int m_iSimType,m_iSimTypeLyingMode;
	int m_iSimTypeCur;
	int m_iSimTypeOverride;
	int m_bIaReady;
	int m_bPartPosForced;
	int m_bFastLimbs;
	int m_bExpandHinges;
	float m_maxPenetrationCur;
	int m_bUsingUnproj;
	vectorf m_prev_pos,m_prev_vel;
	int m_bUpdateBodies;
	int m_nDynContacts,m_bInGroup;

	int m_nCollLyingMode;
	vectorf m_gravityLyingMode;
	float m_dampingLyingMode;
	float m_EminLyingMode;
	int m_nContacts;

	CPhysicalEntity **m_pCollEntList;
	int m_nCollEnts;
};

#endif